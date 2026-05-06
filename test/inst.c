#include "inst.h"
#include <assert.h>

void test_instruction_size(void) {
    static_assert(sizeof(Inst) == 4, "");
}

void test_add_r(void) {
    InstMachine m = inst_machine_new();
    inst_machine_set_reg(&m, 0, 7);
    inst_machine_set_reg(&m, 5, 14);
    inst_run(inst(ADD_RI, 5, 0, 0), &m);
    assert(inst_machine_reg(&m, 0) == 7);
    assert(inst_machine_reg(&m, 5) == 21);
    inst_machine_free(&m);
}

int main(void) {
    test_instruction_size();
    test_add_r();
    return 0;
}
