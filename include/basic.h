#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/** Mark a variable as unused */
#define UNUSED(X) (void)(X)

/** Turn a macro argument to string */
#define _STR(X) #X
#define STR(X) _STR(X)

/// Returns the maximum of two values. May evaluate arguments twice.
#define MAX(A, B) ((A) > (B) ? (A) : (B))

/** Get length of an array variable (does not work on function parameters!) */
#define ARRAY_LEN(A) (sizeof(A) / sizeof((A)[0]))

#define PANIC_PREFIX(FILE, LINE, FUNC)                                         \
    "[Error] " FILE ":" STR(LINE) " " FUNC ": "

/** Hard failure with an error message */
#define PANIC(...)                                                             \
    (fprintf(stderr, PANIC_PREFIX(__FILE__, __LINE__, "%s"), __func__),        \
     fprintf(stderr, __VA_ARGS__),                                             \
     fprintf(stderr, "\n"),                                                    \
     exit(1))

/** Soft failure that just prints a message to the screen */
#define WARNING(...)                                                           \
    do {                                                                       \
        fprintf(                                                               \
            stderr, "[Warning] " __FILE__ ":" STR(__LINE__) " %s: ", __func__  \
        );                                                                     \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
    } while (0)

/** Do an expression and panic if it returns false. */
#define EXPECT(E, ...)                                                         \
    do {                                                                       \
        if (!(E)) PANIC(__VA_ARGS__);                                          \
    } while (0)

/** Do an expression and panic if it returns false, but also print the error
 * from the error number. */
#define EXPECT_ERRNO(E)                                                        \
    do {                                                                       \
        if (!(E))                                                              \
            PANIC("'" #E "' failed - %s (errno %d)", strerror(errno), errno);  \
    } while (0)

/** Make a system call and check for -1, print based on the error number. */
#define SYS(E) EXPECT_ERRNO((E) != -1)

#define SWAP(A, B)                                                             \
    do {                                                                       \
        typeof((A)) _tmp = (A);                                                \
        (A) = (B);                                                             \
        (B) = _tmp;                                                            \
    } while (0)
