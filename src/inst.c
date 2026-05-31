#include "inst.h"
#include "str.h"
#include <assert.h>

InstMachine inst_machine_new(void) {
    return (InstMachine) { {} };
}

#define VALIDATE_REG(R) assert((R) < INST_N_REGS)

uint64_t inst_machine_reg(const InstMachine* m, uint8_t r) {
    VALIDATE_REG(r);
    return m->regs[r];
}

void inst_machine_set_reg(InstMachine* m, uint8_t r, uint64_t v) {
    VALIDATE_REG(r);
    m->regs[r] = v;
}

void inst_machine_free(InstMachine* m) {
    (void)m;
}

static uint64_t* inst_run_get_reg(
    Inst inst, InstMachine* m, uint8_t arg_index
) {
    assert(arg_index < 3);
    const InstArg arg = inst.args[arg_index];
    // TODO: Make sure  i's argument type at the index is a register.
    VALIDATE_REG(arg);
    return &m->regs[arg];
}

static uint64_t inst_run_get_imm(Inst inst, uint8_t arg_index) {
    assert(arg_index < 3);
    const InstArg arg = inst.args[arg_index];
    return arg;
}

void inst_run(Inst inst, InstMachine* m) {
#define reg(R) *inst_run_get_reg(inst, m, R)
#define imm(I) inst_run_get_imm(inst, I)
    switch (inst.op_code) {
        case INST_OP_CODE_ADD_RI: reg(0) += reg(1) + imm(2); break;
        case INST_OP_CODE_ADD_RR: reg(0) += reg(1) + reg(2); break;
    }
#undef reg
#undef imm
}

const char* inst_op_code_short_name(InstOpCode op_code) {
#define X(CASE, SHORT_NAME, ...)                                               \
    case INST_OP_CODE_##CASE: return SHORT_NAME;
    switch (op_code) {
        // NOLINTNEXTLINE(bugprone-branch-clone)
        OP_CODE_X_TABLE(X)
    }
#undef X
    PANIC("Bad InstOpCode: %d", op_code);
}

const char* inst_op_code_full_name(InstOpCode op_code) {
#define X(CASE, SHORT_NAME, SUFFIX, ...)                                       \
    case INST_OP_CODE_##CASE: return SHORT_NAME SUFFIX;
    switch (op_code) {
        // NOLINTNEXTLINE(bugprone-branch-clone)
        OP_CODE_X_TABLE(X)
    }
#undef X
    PANIC("Bad InstOpCode: %d", op_code);
}

char* inst_to_str(Inst inst) {
    return str_format(
        "%s %d %d %d",
        inst_op_code_full_name(inst.op_code),
        inst.args[0],
        inst.args[1],
        inst.args[2]
    );
}
