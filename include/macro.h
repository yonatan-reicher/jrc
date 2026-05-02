#pragma once
// =============================================================================
//          macro.h
// =============================================================================
// This file contains advanced macros for building readable APIs.
// -----------------------------------------------------------------------------

/// Returns the 101th argument. This is useful for looking into `__VA_ARGS__` of
/// a calling macro.
#define ARG_101( _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14,  \
        _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28,  \
        _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42,  \
        _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56,  \
        _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70,  \
        _71, _72, _73, _74, _75, _76, _77, _78, _79, _80, _81, _82, _83, _84,  \
        _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96, _97, _98,  \
        _99, _100, _101, ...) _101

/// Expands to 1 if given more than 1 arguments, else 0.
#define ARGS_HAS_COMMA(...) \
    ARG_101( \
        __VA_ARGS__, \
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   \
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   \
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   \
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   \
        1, 1, 1, 1, 1, 1, 1,                                                   \
        0                                                                      \
    )

/// Expands to comma when given any arguments.
#define _TRIGGER_PARENTHESIS(...) ,

#define ARGS_IS_EMPTY(...)                                                     \
    _ARGS_IS_EMPTY_START(                                                      \
          /* Empty if the trigger is triggered by the following parenthesis */ \
          ARGS_HAS_COMMA(_TRIGGER_PARENTHESIS __VA_ARGS__ ()),                 \
          /* Unless the arguments start themselves with parenthesis... */      \
          ARGS_HAS_COMMA(_TRIGGER_PARENTHESIS __VA_ARGS__),                    \
          /* Or they react to the parenthesis themselves... */                 \
          ARGS_HAS_COMMA(__VA_ARGS__ ()),                                      \
          /* Or they have a comma and something weird happens. */              \
          ARGS_HAS_COMMA(__VA_ARGS__)                                          \
      )
// No we just need to "pattern match" on the results.
#define _ARGS_IS_EMPTY_START(_1,_2,_3,_4) \
  ARGS_HAS_COMMA( _NAME5( _ARGS_IS_EMPTY_RESULT_, _1, _2, _3, _4 ) )
#define _ARGS_IS_EMPTY_RESULT_1000 ,
#define _NAME5(A,B,C,D,E)          _NAME5_HELPER( A, B, C, D, E )
#define _NAME5_HELPER(A,B,C,D,E)   A ## B ## C ## D ## E
