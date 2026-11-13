#pragma once
#include "macro.h"

/// This macro takes at least one expression as an argument, and a trailing
/// statement as a body. It runs the body, and then the argument expressions
/// after. Useful for putting cleanup next to setup. Use `break` to exit the
/// block while still running the cleanup code, but do not use `return`!!
#define DEFER(...)                                                             \
    for (bool _FLAG1 = true; _FLAG1; (__VA_ARGS__, _FLAG1 = false))            \
        /* Having a second loop here let's us use `break` */                   \
        for (bool _FLAG2 = true; _FLAG2; _FLAG2 = false)
#define _FLAG1 UNIQUE_VAR(flag_1)
#define _FLAG2 UNIQUE_VAR(flag_2)
