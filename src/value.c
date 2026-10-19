#include "value.h"
#include "basic.h"
#include "str.h"
#include <inttypes.h>

// =============================================================================
//                                     Func
// =============================================================================

typedef ValueFunc Func;
typedef ValueFuncCapture Capture;
typedef ValueFuncCaptureSlice CaptureSlice;
typedef ConstValueFuncCaptureSlice ConstCaptureSlice;

/// The parameter name and capture names are copied, the ast is just referenced
/// and the captured values are moved.
static Func* func_alloc(
    const char* param_name,
    const Ast* ast,
    size_t n_captured,
    const char* const* captured_names,
    Value* captured_values
) {
    // count
    size_t param_name_len = strlen(param_name);
    size_t n_chars = param_name_len + 1;
    for (size_t i = 0; i < n_captured; i++)
        n_chars += strlen(captured_names[i]) + 1;
    // allocate
    size_t size = sizeof(Func) + n_captured * sizeof(ValueFuncCapture) +
                  n_chars * sizeof(char);
    Func* ret = malloc(size);
    // initialize
    ValueFuncCapture* ret_captured = (ValueFuncCapture*)(ret + 1);
    char* ret_param_name = (char*)(ret_captured + n_captured);
    *ret = (Func) { 0, ret_param_name, ast, n_captured };
    char* next_name = ret_param_name + param_name_len + 1;
    for (size_t i = 0; i < n_captured; i++) {
        ret->captured[i].var_name = next_name;
        ret->captured[i].value = captured_values[i];
    }
    return ret;
}

static CaptureSlice func_captured(Func* f) {
    return (CaptureSlice)slice_new(f->captured, f->n_captured);
}

static ConstCaptureSlice func_const_captured(const Func* f) {
    return (ConstCaptureSlice)slice_new(f->captured, f->n_captured);
}

static void func_free(Func* f) {
    // TODO: How is the AST managed? who frees it?
    // I think the idea is basically that we want to use macros to allow us to
    // manage the ast in different ways? based on some allocator API
    // An allocator should be a thing that can allocate and free memory, and
    // have it's behaviour passed as parameters to functions.
    // Something like
    // typedef struct Allocator {
    //     Alloc* alloc;
    //     Free* free;
    //     void* arg;
    // } Allocator;
    // Maybe that arg field should be given differently... it sure would make
    // some sense to save it along with the functions.
    // Maybe instead the function should just carry it's own destructor function
    // and let that take care of managing the ownership of the AST and decide
    // when it's freed or whatever.
    //
    // For now the AST is just going to be managed by the parser and the
    // function will take it's existence for granted.
    if (f == NULL) return;
    CaptureSlice captured = func_captured(f);
    SLICE_FOREACH(&captured, c) value_dec_rc(c->value);
    *f = (Func) { 0 };
    free(f);
}

static char* func_to_str(const Func* f) {
    // Make a string showing the substituted variables
    CharPtrArray subst;
    bool first = false;
    size_t subst_str_size = 1;
    array_init(&subst, f->n_captured);
    ARRAY_FOREACH(&subst, s) {
        const char* start = first ? "" : ",";
        const Capture* c = &f->captured[i_s];
        char* value_str = value_to_str(c->value);
        *s = str_format("%s%s:=%s", start, c->var_name, value_str);
        free(value_str);
        subst_str_size += strlen(*s);
    }
    char* subst_str = malloc(subst_str_size);
    size_t i = 0;
    ARRAY_FOREACH(&subst, s) {
        size_t l = strlen(*s);
        memcpy(&subst_str[i], *s, l);
        i += l;
        free(*s);
    }
    array_free(&subst);
    // Now finish
    char* ast_str = ast_to_str(f->body);
    char* ret = str_format("(%s => %s)[%s]", f->param_name, ast_str, subst_str);
    free(ast_str);
    free(subst_str);
    return ret;
}

// =============================================================================
//                                     Value
// =============================================================================

void value_inc_rc(Value x) {
    switch (x.kind) {
        case VALUE_NULL: break;
        case VALUE_INT: break;
        case VALUE_FUNC: x.data.f->rc++; break;
    }
}

void value_dec_rc(Value x) {
    switch (x.kind) {
        case VALUE_NULL: break;
        case VALUE_INT: break;
        case VALUE_FUNC:
            x.data.f->rc--;
            if (x.data.f->rc == -1ul) func_free(x.data.f);
            break;
    }
}

Value value_int(int64_t i) {
    Value value = { VALUE_INT, { .i = i } };
    return value;
}

Value value_null(void) {
    return (Value) { VALUE_NULL, {} };
}

Value value_func_new(
    const char* param_name,
    Ast* body,
    size_t n_captured,
    const char* const captured_variable_names[],
    Value captured_values[]
) {
    Func* f = func_alloc(
        param_name,
        body,
        n_captured,
        captured_variable_names,
        captured_values
    );
    return (Value) { VALUE_FUNC, { .f = f } };
}

ValueFuncCaptureSlice value_func_captured(Value v) {
    EXPECT(v.kind == VALUE_FUNC, "must be a function");
    return func_captured(v.data.f);
}

char* value_to_str(Value v) {
    switch (v.kind) {
        case VALUE_NULL: return str_clone("null");
        case VALUE_INT: return str_format("%" PRId64, v.data.i);
        case VALUE_FUNC: return func_to_str(v.data.f);
    }
    PANIC("invalid value kind %d", v.kind);
}
