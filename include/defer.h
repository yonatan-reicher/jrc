#pragma once
#include "macro.h"

/// This macro takes at least one expression as an argument, and a trailing
/// statement as a body. It runs the body, and then the argument expressions
/// after. Useful for putting cleanup next to setup. Note that cleanup will not
/// run if `return` or `break` are used, use a `goto` instead, to the end of the
/// body.
#define DEFER(...) for (bool _FLAG = true; _FLAG; (__VA_ARGS__, _FLAG = false))
#define _FLAG UNIQUE_VAR(flag)
