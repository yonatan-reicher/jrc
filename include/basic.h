#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/** Mark a variable as unused */
#define UNUSED(X) (void)(X)

/// Returns the maximum of two values. May evaluate arguments twice.
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))

/** Get length of an array variable (does not work on function parameters!) */
#define ARRAY_LEN(A) (sizeof(A) / sizeof((A)[0]))

#define STR(X) _STR(X)
#define _STR(X) #X

#define PANIC_PREFIX(FILE, LINE, FUNC)                                         \
    "[Error] " FILE ":" STR(LINE) " " FUNC ": "

/** Hard failure with an error message */
#define PANIC(...)                                                             \
    (fprintf(stderr, PANIC_PREFIX(__FILE__, __LINE__, "%s"), __func__),        \
     fprintf(stderr, __VA_ARGS__),                                             \
     fprintf(stderr, "\n"),                                                    \
     exit(1),                                                                  \
     0)

#define TODO PANIC("not implemented yet")

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

/// Like `EXPECT`, but only id `NDEBUG` is not defined.
#ifdef NDEBUG
#define ASSERT(...)
#else
#define ASSERT(...) EXPECT(__VA_ARGS__)
#endif

/** Do an expression and panic if it returns false, but also print the error
 * from the error number. */
#define EXPECT_ERRNO(E)                                                        \
    (!(E) ? PANIC("'" #E "' failed - %s (errno %d)", strerror(errno), errno)   \
          : 0)

/** Make a system call and check for -1, print based on the error number. */
#define SYS(E) EXPECT_ERRNO((E) != -1)

/// For pthread-like functions - functions that return 0 on success and an error
/// number on failure.
#define EXPECT_PTHREAD(E)                                                      \
    (/* Note that `errno` is thread-local. */                                  \
     errno = (E),                                                              \
     errno != 0                                                                \
         ? PANIC("'" #E "' failed - %s (errno %d)", strerror(errno), errno)    \
         : 0                                                                   \
    )

#define SWAP(A, B)                                                             \
    do {                                                                       \
        typeof((A)) _tmp = (A);                                                \
        (A) = (B);                                                             \
        (B) = _tmp;                                                            \
    } while (0)
