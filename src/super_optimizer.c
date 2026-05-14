#include "super_optimizer.h"
#include "basic.h"

typedef SuperOptimizer This;

static void add_counter_example(This* this, const InstMachine* inp) {
    InstMachine out = *inp;
    ARRAY_FOREACH(&this->ref_prog, inst) inst_run(*inst, &out);
    array_push(&this->counter_examples, (CounterExample) { *inp, out });
}

This super_optimizer_new(const SuperOptimizerConfig* c) {
    This this = {
        inst_enumerator_new_ready(&c->inst_enumerator_config),
        array_empty(),
        0,
        array_empty(),
        0,
        array_empty(),
        c->inst_enumerator_config,
        c->ref_prog,
    };
    // And add some arbitrary counter-examples.
    add_counter_example(&this, &(InstMachine) { .regs = {} });
    return this;
}

void super_optimizer_free(This* this) {
    ARRAY_FOREACH(&this->prefixes, prefix) array_free(prefix);
    array_free(&this->prefixes);
    ARRAY_FOREACH(&this->next_prefixes, prefix) array_free(prefix);
    array_free(&this->next_prefixes);
}

static bool tried_empty_program(const This* this) {
    return this->prefixes.len > 0;
}

static bool try_empty_program(This* this, SuperOptimizerResult* out) {
    const InstArray empty_prog = array_empty();
    bool matches = true;
    ARRAY_FOREACH(&this->counter_examples, ce) {
        const InstMachine* inp = &ce->inp;
        const InstMachine* expected_out = &ce->out;
        InstMachine actual_out = *inp;
        if (memcmp(&actual_out, expected_out, sizeof(InstMachine)) != 0) {
            matches = false;
            goto break_loop;
        }
    }
break_loop:
    if (matches) {
        *out = (SuperOptimizerResult) {
            SUPER_OPTIMIZER_RESULT_TYPE_FOUND,
            { .found = slice_of_array(&empty_prog) },
        };
        return true;
    } else {
        return false;
    }
}

static void after_trying_empty_program(This* this) {
    array_push(&this->prefixes, (InstArray)array_empty());
}

SuperOptimizerResult super_optimizer_step(This* this) {
    SuperOptimizerResult ret;
    if (!tried_empty_program(this)) {
        const bool was_solution = try_empty_program(this, &ret);
        after_trying_empty_program(this);
        if (was_solution) return ret;
    }
again:
    // Check if we are done
    if (this->prefix_len == this->ref_prog.len &&
        inst_enumerator_is_done(&this->inst_enumerator)) {
        return (SuperOptimizerResult) {
            SUPER_OPTIMIZER_RESULT_TYPE_DONE,
            {},
        };
    }
    // Check if this layer is done
    if (inst_enumerator_is_done(&this->inst_enumerator)) {
        this->prefix_len++;
        SWAP(this->prefixes, this->next_prefixes);
        ARRAY_FOREACH(&this->next_prefixes, a) array_clear(a);
        array_clear(&this->next_prefixes);
        inst_enumerator_reset(&this->inst_enumerator);
    }
    // Make a program
    const ConstInstSlice prefix =
        slice_of_array(array_get(&this->prefixes, this->i_prefix));
    const Inst inst = inst_enumerator_current(&this->inst_enumerator);
    InstArray new_prog = array_empty();
    array_extend(&new_prog, prefix.ptr, prefix.len);
    array_push(&new_prog, inst);
    printf("Trying program of length %zu:\n", new_prog.len);
    ARRAY_FOREACH(&new_prog, i) {
        char* inst_str = inst_to_str(*i);
        printf("  %s\n", inst_str);
        free(inst_str);
    }
    // Check if it matches the specification
    bool matches = true;
    ARRAY_FOREACH(&this->counter_examples, ce) {
        const InstMachine* inp = &ce->inp;
        const InstMachine* expected_out = &ce->out;
        InstMachine actual_out = *inp;
        ARRAY_FOREACH(&new_prog, i) inst_run(*i, &actual_out);
        if (memcmp(&actual_out, expected_out, sizeof(InstMachine)) != 0) {
            matches = false;
            goto break_loop;
        }
    }
break_loop:
    if (matches) {
        // Found a matching program!
        return (SuperOptimizerResult) {
            SUPER_OPTIMIZER_RESULT_TYPE_FOUND,
            { .found = slice_of_array(&new_prog) },
        };
    } else {
        array_push(&this->next_prefixes, new_prog);
        this->i_prefix++;
        if (this->i_prefix >= this->prefixes.len) {
            this->i_prefix = 0;
            inst_enumerator_advance(&this->inst_enumerator);
        }
        goto again;
    }
}
