#pragma once
#include "basic.h"
#include <stdint.h>

// =============================================================================
//                                    Inst.h
// =============================================================================
// An implementation of a fake assembly language. An instruction has an
// op-code (operation-code) and up to 3 arguments. Each part is exactly 1 byte,
// making instruction a total of 4 bytes. Arguments can either
//
// The instructions run on a virtual machine with 16 registers with 64 bit
// words.
// -----------------------------------------------------------------------------

#define INST_N_ARGS 3
#define INST_N_REGS 16

// ------ Op-Code --------------------------------------------------------------

typedef enum InstArgType : uint8_t {
    /// An immediate.
    INST_ARG_TYPE_IMM,
    /// An input register.
    INST_ARG_TYPE_INP_REG,
    /// An output register.
    INST_ARG_TYPE_OUT_REG,
    /// A register that is both input and output.
    INST_ARG_TYPE_INP_OUT_REG,
} InstArgType;

#define OP_CODE_X_TABLE(X)                                                     \
    /* op-code, name, suffix, arg-1 type, arg-2 type, arg-3 type */            \
    /* We don't have a dedicated no-op op-code, but the zero-instruction acts  \
     * as a no-op. */                                                          \
    X(ADD_RI, "add", "_ri", OUT_REG, INP_REG, IMM)                             \
    X(ADD_RR, "add", "_rr", OUT_REG, INP_REG, INP_REG)

typedef enum InstOpCode : uint8_t {
#define X(OP_CODE, ...) INST_OP_CODE_##OP_CODE,
    OP_CODE_X_TABLE(X)
#undef X
} InstOpCode;

constexpr InstOpCode inst_op_code_all[] = {
#define X(OP_CODE, ...) INST_OP_CODE_##OP_CODE,
    OP_CODE_X_TABLE(X)
#undef X
};

/// One day this will be 256.
constexpr uint16_t N_INST_OP_CODES = ARRAY_LEN(inst_op_code_all);
/// One day this will be 255.
constexpr InstOpCode INST_OP_CODE_MAX = ARRAY_LEN(inst_op_code_all) - 1;

const char* inst_op_code_short_name(InstOpCode);
const char* inst_op_code_full_name(InstOpCode);

// clang-format off
constexpr InstArgType inst_op_code_arg_type_table[N_INST_OP_CODES][INST_N_ARGS] = {
#define X(OP_CODE, SHORT_NAME, FULL_NAME, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE)     \
    { INST_ARG_TYPE_##ARG1_TYPE, INST_ARG_TYPE_##ARG2_TYPE, INST_ARG_TYPE_##ARG3_TYPE },
    OP_CODE_X_TABLE(X)
#undef X
};
// clang-format on

// ------ The Rest -------------------------------------------------------------

typedef uint8_t InstArg;

typedef struct Inst {
    InstOpCode op_code;
    InstArg args[INST_N_ARGS];
} Inst;

typedef struct InstMachine {
    uint64_t regs[INST_N_REGS];
} InstMachine;

InstMachine inst_machine_new(void);
uint64_t inst_machine_reg(const InstMachine*, uint8_t);
void inst_machine_set_reg(InstMachine*, uint8_t, uint64_t);
void inst_machine_free(InstMachine*);

void inst_run(Inst, InstMachine*);

char* inst_to_str(Inst);

#define inst(OP, ...) ((Inst) { INST_OP_CODE_##OP, { __VA_ARGS__ } })
