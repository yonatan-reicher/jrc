#include "super_optimizer.h"
#include "array.h"
#include "inst.h"

void test_empty_program(void) {
    const uint8_t regs[] = { 0, 1 };
    const uint8_t imms[] = { 0 };
    SuperOptimizer s = super_optimizer_new(&(SuperOptimizerConfig) {
        // enumeration config
        {
            // immediates
            slice_of_fixed_array(imms),
            // registers
            slice_of_fixed_array(regs),
        },
        // empty program
        slice_empty(),
    });
    const SuperOptimizerResult r = super_optimizer_step(&s);
    EXPECT(
        r.type == SUPER_OPTIMIZER_RESULT_TYPE_FOUND,
        "Expected the super-optimizer to find the empty program"
    );
    EXPECT(r.data.found.len == 0, "Expected the found program to be empty");
}

void test_len_one_solution(void) {
    const uint8_t regs[] = { 0, 1 };
    const uint8_t imms[] = { 0, 1 };
    SuperOptimizer s = super_optimizer_new(&(SuperOptimizerConfig) {
        // enumeration config
        {
            // immediates
            slice_of_fixed_array(imms),
            // registers
            slice_of_fixed_array(regs),
        },
        // program that adds 1 to register 0 and stores it in register 1
        slice(((Inst[]) { inst(ADD_RI, 1, 0, 1), inst(ADD_RI, 1, 0, 1) }), 1),
    });
    const SuperOptimizerResult r = super_optimizer_step(&s);
    EXPECT(
        r.type == SUPER_OPTIMIZER_RESULT_TYPE_FOUND,
        "Expected the super-optimizer to find a solution"
    );
    EXPECT(
        r.data.found.len == 1, "Expected the found program to have length 1"
    );
    EXPECT(
        r.data.found.ptr[0].op_code == INST_OP_CODE_ADD_RI,
        "Expected the found instruction to be ADD_RI"
    );
    EXPECT(
        r.data.found.ptr[0].args[0] == 1, "Expected the output register to be 1"
    );
    EXPECT(
        r.data.found.ptr[0].args[1] == 0, "Expected the input register to be 0"
    );
    EXPECT(r.data.found.ptr[0].args[2] == 1, "Expected the immediate to be 1");
}

int main(void) {
    test_empty_program();
    test_len_one_solution();
    return 0;
}
