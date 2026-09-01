#include "backtrace.h"
#include "array.h"

#ifdef NDEBUG

void backtrace_push(const char* func, const char* file, unsigned line) {
    UNUSED(func);
    UNUSED(file);
    UNUSED(line);
}

void backtrace_pop() {
}

void backtrace_print_to_file(FILE* f) {
    UNUSED(f);
}

void backtrace_print() {
}

#else

typedef struct BacktraceEntry {
    const char* func;
    const char* file;
    unsigned line;
    void* stack_address;
} BacktraceEntry;

DECLARE_ARRAY(BacktraceEntry, BacktraceEntryArray);

BacktraceEntryArray backtrace = array_empty();

bool exit_handler_initialized = false;

static void exit_handler() {
    if (backtrace.len > 0) {
        fprintf(stderr, "BACKTRACE\n");
        backtrace_print();
    } else {
        fprintf(stderr, "BACKTRACE - empty\n");
    }
}

static void init_exit_handler() {
    EXPECT(
        !exit_handler_initialized,
        "should not initialize backtrace exit handler twice"
    );
    int ret = atexit(exit_handler);
    if (ret == 0) exit_handler_initialized = true;
}

void backtrace_push(const char* func, const char* file, unsigned line) {
    if (!exit_handler_initialized) init_exit_handler();

    int top = 0;
    array_push(&backtrace, (BacktraceEntry) { func, file, line, &top });
}

void backtrace_pop() {
    array_pop(&backtrace);
}

void backtrace_print_to_file(FILE* f) {
    ARRAY_FOREACH(&backtrace, x) {
        fprintf(f, "%s:%u %s\n", x->file, x->line, x->func);
    }
}

void backtrace_print() {
    backtrace_print_to_file(stderr);
}

#endif
