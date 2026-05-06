#pragma once
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

typedef enum InstOpCode : uint8_t {
    /// Does nothing.
    INST_OP_CODE_NOP,
    /// Adds a input register and immediate and writes to an output register.
    INST_OP_CODE_ADD_RI,
    /// Adds two input registers and writes to an output register.
    INST_OP_CODE_ADD_RR,
} InstOpCode;

typedef uint8_t InstArg;

typedef struct Inst {
    InstOpCode op_code;
    InstArg args[3];
} Inst;

#define INST_N_REGS 16

typedef struct InstMachine {
    uint64_t regs[INST_N_REGS];
} InstMachine;

InstMachine inst_machine_new(void);
uint64_t inst_machine_reg(const InstMachine*, uint8_t);
void inst_machine_set_reg(InstMachine*, uint8_t, uint64_t);
void inst_machine_free(InstMachine*);

void inst_run(Inst, InstMachine*);

#define inst(OP, ...) ((Inst) { INST_OP_CODE_##OP, { __VA_ARGS__ } })
