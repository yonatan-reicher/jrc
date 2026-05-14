#include "inst_enumerator.h"
#include <assert.h>

/* On the implementation:
 * The enumerator keeps a current op-code and indices for each of the 3
 * arguments. The indices index into the slices given in the configuration. When
 * we advance, we advance the arguments before the op-code. When an argument is
 * done, we reset it and advance the next. When the last is done, we advance the
 * op-code instead. Notice that the fact that I'm describing this here means
 * this is all an implementation detail, and the order may change.
 */

typedef InstEnumerator This;
#define IE(NAME) InstEnumerator##NAME

static void validate_config(const IE(Config) * c) {
    EXPECT(c->immediates.len <= 255, "Expected at most 255 immediates");
    EXPECT(c->registers.len <= INST_N_REGS, "Expected at most 16 registers");
    // Check uniqueness of registers and immediates. Also validate registers.
    for (size_t i = 0; i < c->immediates.len; i++) {
        const uint8_t x = c->immediates.ptr[i];
        for (size_t j = 0; j < i; j++) {
            const uint8_t y = c->immediates.ptr[j];
            EXPECT(
                x != y,
                "Expected immediates to be unique, but '%d' appears more than "
                "once",
                x
            );
        }
    }
    for (size_t i = 0; i < c->registers.len; i++) {
        const uint8_t x = c->registers.ptr[i];
        EXPECT(x < INST_N_REGS, "'%d' is not a valid register number", x);
        for (size_t j = 0; j < i; j++) {
            const uint8_t y = c->registers.ptr[j];
            EXPECT(
                x != y,
                "Expected registers to be unique, but '%d' appears more than "
                "once",
                x
            );
        }
    }
}

This inst_enumerator_new(const IE(Config) * config) {
    validate_config(config);
    return (This) {
        0,
        { 0, 0, 0 },
        false,
        *config,
    };
}

This inst_enumerator_new_ready(const IE(Config) * config) {
    This ret = inst_enumerator_new(config);
    if (!inst_enumerator_is_done(&ret) && !inst_enumerator_has_current(&ret)) {
        inst_enumerator_advance(&ret);
    }
    return ret;
}

bool inst_enumerator_has_current(const This* this) {
    // For now, this is always true. If we allow filtering by specific op-codes,
    // this may become more conditional. This is just in case we want to add
    // that later.
    UNUSED(this);
    return true;
}

bool inst_enumerator_is_done(const This* this) {
    return this->is_done;
}

static bool try_advance_op_code(This* this) {
    if (this->op_code >= INST_OP_CODE_MAX) return false;
    this->op_code++;
    return true;
}

InstArgType arg_type(const This* this, uint8_t which_arg) {
    return inst_op_code_arg_type_table[this->op_code][which_arg];
}

static uint8_t arg_max_index(const This* this, uint8_t which_arg) {
    switch (arg_type(this, which_arg)) {
        case INST_ARG_TYPE_INP_REG:
        case INST_ARG_TYPE_OUT_REG:
        case INST_ARG_TYPE_INP_OUT_REG:
            return (uint8_t)(this->config.registers.len - 1);
        case INST_ARG_TYPE_IMM:
            return (uint8_t)(this->config.immediates.len - 1);
    }
}

static bool try_advance_arg(This* this, uint8_t which_arg) {
    uint8_t* arg_index = &this->arg_indices[which_arg];
    const uint8_t max_index = arg_max_index(this, which_arg);
    if (*arg_index >= max_index) return false;
    (*arg_index)++;
    return true;
}

static void reset_arg(This* this, uint8_t which_arg) {
    this->arg_indices[which_arg] = 0;
}

static uint8_t arg_current(const This* this, uint8_t which_arg) {
    const uint8_t arg_index = this->arg_indices[which_arg];
    switch (arg_type(this, which_arg)) {
        case INST_ARG_TYPE_INP_REG:
        case INST_ARG_TYPE_OUT_REG:
        case INST_ARG_TYPE_INP_OUT_REG:
            return this->config.registers.ptr[arg_index];
        case INST_ARG_TYPE_IMM: return this->config.immediates.ptr[arg_index];
    }
}

Inst inst_enumerator_current(const This* this) {
    assert(!inst_enumerator_is_done(this));
    return (Inst) {
        this->op_code,
        {
            arg_current(this, 0),
            arg_current(this, 1),
            arg_current(this, 2),
        },
    };
}

bool inst_enumerator_advance(This* this) {
    if (this->is_done) return false;
    if (try_advance_arg(this, 0)) return true;
    reset_arg(this, 0);
    if (try_advance_arg(this, 1)) return true;
    reset_arg(this, 1);
    if (try_advance_arg(this, 2)) return true;
    reset_arg(this, 2);
    if (try_advance_op_code(this)) return true;
    // We couldn't advance anything!
    this->is_done = true;
    return false;
}

void inst_enumerator_reset(This* this) {
    this->op_code = 0;
    for (uint8_t i = 0; i < INST_N_ARGS; i++) reset_arg(this, i);
    this->is_done = false;
    // Get it ready!
    if (!inst_enumerator_is_done(this) && !inst_enumerator_has_current(this))
        inst_enumerator_advance(this);
}
