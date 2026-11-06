#pragma once
// =============================================================================
//          macro.h
// =============================================================================
// This file contains advanced macros for building readable APIs.
// -----------------------------------------------------------------------------

// This file is gonna teach you some macro magic. But first, some paperwork.
//
// ==== STYLE ====
//
// In this file specifically, macro arguments are lower-case, because otherwise
// this whole file would have been upper-case.
//
// ==== EXPANSION ====
//
// What exactly are the semantics of expansion? When are macros *not* expanded,
// and what's the order?
//
// The entire source code sent through a root expansion. That expansion triggers
// nested expansions, which trigger more and so on. Systems are in place to make
// this always terminating.
//
// When an expansion sees the name of a registered macro next to parenthesis, it
// starts expanding it (and if the macro does not take arguments, it is expanded
// even when there aren't parenthesis). When a macro is expanded, it's arguments
// are first expanded. Arguments are expanded separately (see example below).
// After that, the macro body is expanded, with the expansions of the arguments
// bound to the parameter names. Finally, the macro call is replaced with it's
// expanded body.
//
// One more not about expansion: it has a system to stop recursion. The
// preprocessor keeps a list of macro names it is currently expanding, and any
// macro calls of those names will not be expanded. More than just that - the
// tokens will be marked, and will never expand, even in other expansions.
// Luckily, this marking is only done when the macro is applied to arguments (if
// it takes any). See example below.
//
// ==== EXAMPLES ====
//
// Arguments are expanded separately:
// ```c
// #define HELLO(a, ...) first argument is a
// #define HI() this, has, commas
// HELLO(HI())
// ```
// Despite `HI()` expanding to tokens with commas, `this, has, commas` is
// treated in it's entirety as the first argument to `HELLO`. Thus, the final
// expansion of the code is `first argument is this, has, commas`. Step by step:
// - `HELLO(HI())` - Expand arguments.
// - `HI()` - Expand arguments. No arguments, expand body.
// - `this, has, commas` - Nothing to do.
// - `first argument is a` - `this, has, commas` was bound to `a`.
// - `first argument is this, has, commas` - Final expansion.
//
// Macro marking:
// ```c
// #define EMPTY()
// #define EVAL(...) ... // evaluates it's arguments many times
//
// #define REC() hi REC()
// EVAL(REC()) // expands to `hi REC()`
//
// #define REC_2() hi REC_2_AUX EMPTY () ()
// #define REC_2_AUX() REC_2 EMPTY () ()
// EVAL(REC_2()) // expands to `hi hi hi hi hi ...`
// ```
//
// Expansion continues to token after:
// ```c
// #define MY_TEST() TEST
// #define TEST() great success
// #define DELAY
// MY_TEST() () // expands to `great success`
// MY_TEST() DELAY // expands to `TEST ()`
// ```

// ------ Expanding `#` and `##` -----------------------------------------------

/// Turn a macro expansion to string (`#`).
#define STR(x) _STR(x)
#define _STR(x) #x

/// Concatenate two expansions together (`##`).
#define CAT_2(a, b) _CAT_2(a, b)
#define _CAT_2(a, b) a##b

#define CAT_3(a, b, c) _CAT_3(a, b, c)
#define _CAT_3(a, b, c) a##b##c

#define CAT_4(a, b, c, d) _CAT_4(a, b, c, d)
#define _CAT_4(a, b, c, d) a##b##c##d

#define CAT_5(a, b, c, d, e) _CAT_5(a, b, c, d, e)
#define _CAT_5(a, b, c, d, e) a##b##c##d##e

// ------ Some Primitives ------------------------------------------------------

/// Expands to nothing when applied to `()`.
#define EMPTY()

/// `IF(cond)(a)(b)` expands to `a` if `cond` is `1` and `b` if it is `0`.
/// If the condition is neither 0 nor 1, things break.
#define IF(cond) CAT_2(_IF_, cond)
#define _IF_1(...) __VA_ARGS__ _IF_1_ELSE
#define _IF_1_ELSE(...)
#define _IF_0(...) _IF_0_ELSE
#define _IF_0_ELSE(...) __VA_ARGS__

/// Get the first argument.
#define ARG_1(a, ...) a
/// Get the second argument.
#define ARG_2(a, b, ...) b
/// Get the second argument.
#define ARG_3(a, b, c, ...) c
// clang-format off
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
// clang-format on

// clang-format off
/// Expands to 1 if given more than 1 argument, else 0.
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
// clang-format on

/// Returns `1` if the argument expands exactly to `0`. Otherwise, returns `0`.
#define NOT(x) _NOT_AUX_0_DETECTOR(CAT_2(_NOT_AUX_, x)())
#define _NOT_AUX_0() ~, 1
#define _NOT_AUX_0_DETECTOR(...) ARG_2(__VA_ARGS__, 0)

#define AND(a, b) _AND(a, b)
#define _AND(a, b) _AND_##a##b()
#define _AND_11() 1
#define _AND_10() 0
#define _AND_01() 0
#define _AND_00() 0

#define OR(a, b) _OR(a, b)
#define _OR(a, b) _OR_##a##b()
#define _OR_11() 1
#define _OR_10() 1
#define _OR_01() 1
#define _OR_00() 0

#define AND_4(a, b, c, d) _AND_4(a, b, c, d)
#define _AND_4(a, b, c, d) _AND_4_##a##b##c##d()
#define _AND_4_1111() 1
#define _AND_4_0000() 0
#define _AND_4_0001() 0
#define _AND_4_0010() 0
#define _AND_4_0011() 0
#define _AND_4_0100() 0
#define _AND_4_0101() 0
#define _AND_4_0110() 0
#define _AND_4_0111() 0
#define _AND_4_1000() 0
#define _AND_4_1001() 0
#define _AND_4_1010() 0
#define _AND_4_1011() 0
#define _AND_4_1100() 0
#define _AND_4_1101() 0
#define _AND_4_1110() 0

#define OR_4(a, b, c, d) _OR_4(a, b, c, d)
#define _OR_4(a, b, c, d) _OR_4_##a##b##c##d()
#define _OR_4_0000() 0
#define _OR_4_0001() 1
#define _OR_4_0010() 1
#define _OR_4_0011() 1
#define _OR_4_0100() 1
#define _OR_4_0101() 1
#define _OR_4_0110() 1
#define _OR_4_0111() 1
#define _OR_4_1000() 1
#define _OR_4_1001() 1
#define _OR_4_1010() 1
#define _OR_4_1011() 1
#define _OR_4_1100() 1
#define _OR_4_1101() 1
#define _OR_4_1110() 1
#define _OR_4_1111() 1

/// Returns `0` if the argument is exactly `0`, otherwise returns `1`.
#define BOOL(...) NOT(NOT(__VA_ARGS__))

// TODO: This doesn't work in the case where the first argument expands to
// empty.
#define HAS_ARGS(...) BOOL(ARG_1(_HAS_ARGS_AUX __VA_ARGS__)())
#define _HAS_ARGS_AUX() 0

// ------ Computations ---------------------------------------------------------

/// This macro expands it's arguments many times.
#define EVAL(...) EVAL_512(__VA_ARGS__)
// Each `EVAL_N` macro below expands it's arguments N times.
#define EVAL_2048(...) EVAL_512(EVAL_512(__VA_ARGS__))
#define EVAL_1024(...) EVAL_512(EVAL_512(__VA_ARGS__))
#define EVAL_512(...) EVAL_256(EVAL_256(__VA_ARGS__))
#define EVAL_256(...) EVAL_128(EVAL_128(__VA_ARGS__))
#define EVAL_128(...) EVAL_64(EVAL_64(__VA_ARGS__))
#define EVAL_64(...) EVAL_32(EVAL_32(__VA_ARGS__))
#define EVAL_32(...) EVAL_16(EVAL_16(__VA_ARGS__))
#define EVAL_16(...) EVAL_8(EVAL_8(__VA_ARGS__))
#define EVAL_8(...) EVAL_4(EVAL_4(__VA_ARGS__))
#define EVAL_4(...) EVAL_2(EVAL_2(__VA_ARGS__))
#define EVAL_2(...) EVAL_1(EVAL_1(__VA_ARGS__))
#define EVAL_1(...) __VA_ARGS__

/// This macro defers the expansion of it's arguments once.
#define DEFER_1(m) m EMPTY()
#define DEFER_2(m) m EMPTY EMPTY()()
#define DEFER_3(m) m EMPTY EMPTY EMPTY()()()

#define MAP(m, first, ...)                                                     \
    m(first)                                                                   \
    IF(HAS_ARGS(__VA_ARGS__))(                                                 \
        DEFER_2(_MAP)()(m, __VA_ARGS__)                                        \
    )(                                                                         \
        /* Do nothing, just terminate */                                       \
    )
#define _MAP() MAP

/// Just like `MAP`, but puts commas between the results.
#define MAP_COMMA(m, first, ...) \
    IF(HAS_ARGS(__VA_ARGS__)) \
    (m(first), DEFER_2(_MAP_COMMA)()(m, __VA_ARGS))

// ------ Naturals -------------------------------------------------------------

#define INC(n) INC_##n
#define INC_0 1
#define INC_1 2
#define INC_2 3
#define INC_3 4
#define INC_4 5
#define INC_5 6
#define INC_6 7
#define INC_7 8
#define INC_8 9
#define INC_9 10
#define INC_10 11
#define INC_11 12
#define INC_12 13
#define INC_13 14
#define INC_14 15
#define INC_15 16
#define INC_16 17
#define INC_17 18
#define INC_18 19
#define INC_19 20
#define INC_20 21
#define INC_21 22
#define INC_22 23
#define INC_23 24
#define INC_24 25
#define INC_25 26
#define INC_26 27
#define INC_27 28
#define INC_28 29
#define INC_29 30
#define INC_30 31
#define INC_31 32
#define INC_32 33
#define INC_33 34
#define INC_34 35
#define INC_35 36
#define INC_36 37
#define INC_37 38
#define INC_38 39
#define INC_39 40
#define INC_40 41
#define INC_41 42
#define INC_42 43
#define INC_43 44
#define INC_44 45
#define INC_45 46
#define INC_46 47
#define INC_47 48
#define INC_48 49
#define INC_49 50
#define INC_50 51
#define INC_51 52
#define INC_52 53
#define INC_53 54
#define INC_54 55
#define INC_55 56
#define INC_56 57
#define INC_57 58
#define INC_58 59
#define INC_59 60
#define INC_60 61
#define INC_61 62
#define INC_62 63
#define INC_63 64
#define INC_64 65
#define INC_65 66
#define INC_66 67
#define INC_67 68
#define INC_68 69
#define INC_69 70
#define INC_70 71
#define INC_71 72
#define INC_72 73
#define INC_73 74
#define INC_74 75
#define INC_75 76
#define INC_76 77
#define INC_77 78
#define INC_78 79
#define INC_79 80
#define INC_80 81
#define INC_81 82
#define INC_82 83
#define INC_83 84
#define INC_84 85
#define INC_85 86
#define INC_86 87
#define INC_87 88
#define INC_88 89
#define INC_89 90
#define INC_90 91
#define INC_91 92
#define INC_92 93
#define INC_93 94
#define INC_94 95
#define INC_95 96
#define INC_96 97
#define INC_97 98
#define INC_98 99
#define INC_99 100
#define INC_100 101
#define INC_101 102
#define INC_102 103
#define INC_103 104
#define INC_104 105
#define INC_105 106
#define INC_106 107
#define INC_107 108
#define INC_108 109
#define INC_109 110
#define INC_110 111
#define INC_111 112
#define INC_112 113
#define INC_113 114
#define INC_114 115
#define INC_115 116
#define INC_116 117
#define INC_117 118
#define INC_118 119
#define INC_119 120
#define INC_120 121
#define INC_121 122
#define INC_122 123
#define INC_123 124
#define INC_124 125
#define INC_125 126
#define INC_126 127
#define INC_127 128
#define INC_128 129
#define INC_129 130
#define INC_130 131
#define INC_131 132
#define INC_132 133
#define INC_133 134
#define INC_134 135
#define INC_135 136
#define INC_136 137
#define INC_137 138
#define INC_138 139
#define INC_139 140
#define INC_140 141
#define INC_141 142
#define INC_142 143
#define INC_143 144
#define INC_144 145
#define INC_145 146
#define INC_146 147
#define INC_147 148
#define INC_148 149
#define INC_149 150
#define INC_150 151
#define INC_151 152
#define INC_152 153
#define INC_153 154
#define INC_154 155
#define INC_155 156
#define INC_156 157
#define INC_157 158
#define INC_158 159
#define INC_159 160
#define INC_160 161
#define INC_161 162
#define INC_162 163
#define INC_163 164
#define INC_164 165
#define INC_165 166
#define INC_166 167
#define INC_167 168
#define INC_168 169
#define INC_169 170
#define INC_170 171
#define INC_171 172
#define INC_172 173
#define INC_173 174
#define INC_174 175
#define INC_175 176
#define INC_176 177
#define INC_177 178
#define INC_178 179
#define INC_179 180
#define INC_180 181
#define INC_181 182
#define INC_182 183
#define INC_183 184
#define INC_184 185
#define INC_185 186
#define INC_186 187
#define INC_187 188
#define INC_188 189
#define INC_189 190
#define INC_190 191
#define INC_191 192
#define INC_192 193
#define INC_193 194
#define INC_194 195
#define INC_195 196
#define INC_196 197
#define INC_197 198
#define INC_198 199
#define INC_199 200
#define INC_200 201
#define INC_201 202
#define INC_202 203
#define INC_203 204
#define INC_204 205
#define INC_205 206
#define INC_206 207
#define INC_207 208
#define INC_208 209
#define INC_209 210
#define INC_210 211
#define INC_211 212
#define INC_212 213
#define INC_213 214
#define INC_214 215
#define INC_215 216
#define INC_216 217
#define INC_217 218
#define INC_218 219
#define INC_219 220
#define INC_220 221
#define INC_221 222
#define INC_222 223
#define INC_223 224
#define INC_224 225
#define INC_225 226
#define INC_226 227
#define INC_227 228
#define INC_228 229
#define INC_229 230
#define INC_230 231
#define INC_231 232
#define INC_232 233
#define INC_233 234
#define INC_234 235
#define INC_235 236
#define INC_236 237
#define INC_237 238
#define INC_238 239
#define INC_239 240
#define INC_240 241
#define INC_241 242
#define INC_242 243
#define INC_243 244
#define INC_244 245
#define INC_245 246
#define INC_246 247
#define INC_247 248
#define INC_248 249
#define INC_249 250
#define INC_250 251
#define INC_251 252
#define INC_252 253
#define INC_253 254
#define INC_254 255
#define INC_255 256

#define DEC(n) DEC_##n
#define DEC_1 0
#define DEC_2 1
#define DEC_3 2
#define DEC_4 3
#define DEC_5 4
#define DEC_6 5
#define DEC_7 6
#define DEC_8 7
#define DEC_9 8
#define DEC_10 9
#define DEC_11 10
#define DEC_12 11
#define DEC_13 12
#define DEC_14 13
#define DEC_15 14
#define DEC_16 15
#define DEC_17 16
#define DEC_18 17
#define DEC_19 18
#define DEC_20 19
#define DEC_21 20
#define DEC_22 21
#define DEC_23 22
#define DEC_24 23
#define DEC_25 24
#define DEC_26 25
#define DEC_27 26
#define DEC_28 27
#define DEC_29 28
#define DEC_30 29
#define DEC_31 30
#define DEC_32 31
#define DEC_33 32
#define DEC_34 33
#define DEC_35 34
#define DEC_36 35
#define DEC_37 36
#define DEC_38 37
#define DEC_39 38
#define DEC_40 39
#define DEC_41 40
#define DEC_42 41
#define DEC_43 42
#define DEC_44 43
#define DEC_45 44
#define DEC_46 45
#define DEC_47 46
#define DEC_48 47
#define DEC_49 48
#define DEC_50 49
#define DEC_51 50
#define DEC_52 51
#define DEC_53 52
#define DEC_54 53
#define DEC_55 54
#define DEC_56 55
#define DEC_57 56
#define DEC_58 57
#define DEC_59 58
#define DEC_60 59
#define DEC_61 60
#define DEC_62 61
#define DEC_63 62
#define DEC_64 63
#define DEC_65 64
#define DEC_66 65
#define DEC_67 66
#define DEC_68 67
#define DEC_69 68
#define DEC_70 69
#define DEC_71 70
#define DEC_72 71
#define DEC_73 72
#define DEC_74 73
#define DEC_75 74
#define DEC_76 75
#define DEC_77 76
#define DEC_78 77
#define DEC_79 78
#define DEC_80 79
#define DEC_81 80
#define DEC_82 81
#define DEC_83 82
#define DEC_84 83
#define DEC_85 84
#define DEC_86 85
#define DEC_87 86
#define DEC_88 87
#define DEC_89 88
#define DEC_90 89
#define DEC_91 90
#define DEC_92 91
#define DEC_93 92
#define DEC_94 93
#define DEC_95 94
#define DEC_96 95
#define DEC_97 96
#define DEC_98 97
#define DEC_99 98
#define DEC_100 99
#define DEC_101 100
#define DEC_102 101
#define DEC_103 102
#define DEC_104 103
#define DEC_105 104
#define DEC_106 105
#define DEC_107 106
#define DEC_108 107
#define DEC_109 108
#define DEC_110 109
#define DEC_111 110
#define DEC_112 111
#define DEC_113 112
#define DEC_114 113
#define DEC_115 114
#define DEC_116 115
#define DEC_117 116
#define DEC_118 117
#define DEC_119 118
#define DEC_120 119
#define DEC_121 120
#define DEC_122 121
#define DEC_123 122
#define DEC_124 123
#define DEC_125 124
#define DEC_126 125
#define DEC_127 126
#define DEC_128 127
#define DEC_129 128
#define DEC_130 129
#define DEC_131 130
#define DEC_132 131
#define DEC_133 132
#define DEC_134 133
#define DEC_135 134
#define DEC_136 135
#define DEC_137 136
#define DEC_138 137
#define DEC_139 138
#define DEC_140 139
#define DEC_141 140
#define DEC_142 141
#define DEC_143 142
#define DEC_144 143
#define DEC_145 144
#define DEC_146 145
#define DEC_147 146
#define DEC_148 147
#define DEC_149 148
#define DEC_150 149
#define DEC_151 150
#define DEC_152 151
#define DEC_153 152
#define DEC_154 153
#define DEC_155 154
#define DEC_156 155
#define DEC_157 156
#define DEC_158 157
#define DEC_159 158
#define DEC_160 159
#define DEC_161 160
#define DEC_162 161
#define DEC_163 162
#define DEC_164 163
#define DEC_165 164
#define DEC_166 165
#define DEC_167 166
#define DEC_168 167
#define DEC_169 168
#define DEC_170 169
#define DEC_171 170
#define DEC_172 171
#define DEC_173 172
#define DEC_174 173
#define DEC_175 174
#define DEC_176 175
#define DEC_177 176
#define DEC_178 177
#define DEC_179 178
#define DEC_180 179
#define DEC_181 180
#define DEC_182 181
#define DEC_183 182
#define DEC_184 183
#define DEC_185 184
#define DEC_186 185
#define DEC_187 186
#define DEC_188 187
#define DEC_189 188
#define DEC_190 189
#define DEC_191 190
#define DEC_192 191
#define DEC_193 192
#define DEC_194 193
#define DEC_195 194
#define DEC_196 195
#define DEC_197 196
#define DEC_198 197
#define DEC_199 198
#define DEC_200 199
#define DEC_201 200
#define DEC_202 201
#define DEC_203 202
#define DEC_204 203
#define DEC_205 204
#define DEC_206 205
#define DEC_207 206
#define DEC_208 207
#define DEC_209 208
#define DEC_210 209
#define DEC_211 210
#define DEC_212 211
#define DEC_213 212
#define DEC_214 213
#define DEC_215 214
#define DEC_216 215
#define DEC_217 216
#define DEC_218 217
#define DEC_219 218
#define DEC_220 219
#define DEC_221 220
#define DEC_222 221
#define DEC_223 222
#define DEC_224 223
#define DEC_225 224
#define DEC_226 225
#define DEC_227 226
#define DEC_228 227
#define DEC_229 228
#define DEC_230 229
#define DEC_231 230
#define DEC_232 231
#define DEC_233 232
#define DEC_234 233
#define DEC_235 234
#define DEC_236 235
#define DEC_237 236
#define DEC_238 237
#define DEC_239 238
#define DEC_240 239
#define DEC_241 240
#define DEC_242 241
#define DEC_243 242
#define DEC_244 243
#define DEC_245 244
#define DEC_246 245
#define DEC_247 246
#define DEC_248 247
#define DEC_249 248
#define DEC_250 249
#define DEC_251 250
#define DEC_252 251
#define DEC_253 252
#define DEC_254 253
#define DEC_255 254
#define DEC_256 255

#define EQ_0(n) NOT(n)
#define EQ_1(n) EQ_0(DEC(n))

/// `EVAL(N_RANGE(N))` expands to `0, ..., N-1`
#define N_RANGE(n) IF(EQ_0(n))()(IF(EQ_1(n))(0)(DEFER_3(_N_RANGE)()(DEC(n)), DEC(n)))
#define _N_RANGE() N_RANGE

// ------ Old Code -------------------------------------------------------------

// clang-format off
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
// clang-format on

// clang-format off
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
// clang-format on

/// Expands to comma when given any arguments.
#define _TRIGGER_PARENTHESIS(...) ,

// clang-format off
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
// clang-format on
