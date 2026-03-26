#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

/** Mark a variable as unused */
#define UNUSED(X) (void)(X)

/** Turn a macro argument to string */
#define _STR(X) #X
#define STR(X) _STR(X)

/** Get length of an array variable (does not work on function parameters!) */
#define ARRAY_LEN(A) (sizeof(A) / sizeof( (A)[0] ))

/** Hard failure with an error message */
#define PANIC(...) \
    do { \
        fprintf(stderr, "[Error] " __FILE__ ":" STR(__LINE__) " %s: ", __func__); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
        exit(1); \
    } while (0)

/** Soft failure that just prints a message to the screen */
#define WARNING(...) \
    do { \
        fprintf(stderr, "[Warning] " __FILE__ ":" STR(__LINE__) " %s: ", __func__); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } while (0)

/** Do an expression and panic if it returns false. */
#define EXPECT(E, ...) \
    do { \
        if (!(E)) PANIC(__VA_ARGS__); \
    } while (0)

/** Do an expression and panic if it returns false, but also print the error
 * from the error number. */
#define EXPECT_ERRNO(E) \
    do { \
        if (!(E)) \
            PANIC("'" #E "' failed - %s (errno %d)", strerror(errno), errno); \
    } while (0)

/** Make a system call and check for -1, print based on the error number. */
#define SYS(E) EXPECT_ERRNO((E) != -1)
