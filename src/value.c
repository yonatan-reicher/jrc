#include "value.h"
#include "str.h"
#include <inttypes.h>

Value value_int(int64_t i) {
    Value value = { VALUE_INT, { .i = i } };
    return value;
}

Value value_null(void) {
    return (Value) { VALUE_NULL, {} };
}

char* value_to_str(Value v) {
    switch (v.kind) {
        case VALUE_NULL: return str_clone("null");
        case VALUE_INT: return str_format("%" PRId64, v.data.i);
    }
}
