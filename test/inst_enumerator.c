#include "inst_enumerator.h"
#include "basic.h"
#include "inst.h"
#include <inttypes.h>

void test_count(void) {
    const uint8_t immediates[] = { 1, 2, 5, 7, 11 };
    const uint8_t registers[] = { 0, 7, 3 };
    uint64_t expected = 0;
    for (InstOpCode op_code = 0; op_code < N_INST_OP_CODES; op_code++) {
        const InstArgType arg_types[] = {
            inst_op_code_arg_type_table[op_code][0],
            inst_op_code_arg_type_table[op_code][1],
            inst_op_code_arg_type_table[op_code][2],
        };
        uint64_t n_insts_for_op_code = 1;
        for (uint8_t i = 0; i < 3; i++) {
            switch (arg_types[i]) {
                case INST_ARG_TYPE_INP_REG:
                case INST_ARG_TYPE_OUT_REG:
                case INST_ARG_TYPE_INP_OUT_REG:
                    n_insts_for_op_code *= ARRAY_LEN(registers);
                    break;
                case INST_ARG_TYPE_IMM:
                    n_insts_for_op_code *= ARRAY_LEN(immediates);
                    break;
            }
        }
        expected += n_insts_for_op_code;
    };
    const InstEnumeratorConfig config = {
        slice(immediates, ARRAY_LEN(immediates)),
        slice(registers, ARRAY_LEN(registers)),
    };
    InstEnumerator enumerator = inst_enumerator_new_ready(&config);
    uint64_t count = 0;
    while (!inst_enumerator_is_done(&enumerator)) {
        inst_enumerator_advance(&enumerator);
        count++;
    }
    EXPECT(
        count == expected,
        "Expected to enumerate %" PRIu64 " instructions, but got %" PRIu64,
        expected,
        count
    );
}

int main(void) {
    test_count();
    return 0;
}
