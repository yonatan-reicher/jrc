#include <stdio.h>

/// This macro adds the current function to the backtrace. Use it at the top of
/// the function. Does not do anything outside of debug builds
#define BACKTRACE_HEY                                                          \
    do {                                                                       \
        backtrace_push(__func__, __FILE__, __LINE__);                          \
    } while (0)

#define BACKTRACE_BYE                                                          \
    do {                                                                       \
        backtrace_pop();                                                       \
    } while (0)

void backtrace_push(const char* func, const char* file, unsigned line);
void backtrace_pop();

void backtrace_print_to_file(FILE* f);

/// Prints the current stack trace.
void backtrace_print();
