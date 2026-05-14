#pragma once
#include "array.h"
#include "inst.h"
#include "inst_enumerator.h"
#include "slice.h"

// =============================================================================
//                               super_optimizer.h
// =============================================================================
// A super-optimizer is an algorithm that, given a specification, finds the
// optimal program that satisfies it, under some search space. The specification
// is for now, just a reference program, and the search just searches for
// programs that match it on some arbitrary set of inputs. The programs are all
// straight-line instructions from [inst.h]. The algorithm for now is also just
// a dumb brute-force breadth-first search.
// -----------------------------------------------------------------------------

DECLARE_ARRAY(Inst, InstArray);
DECLARE_ARRAY(InstArray, InstArrayArray);
DECLARE_ARRAY(InstMachine, InstMachineArray);
DECLARE_SLICE(const Inst, ConstInstSlice);

typedef struct SuperOptimizerConfig {
    InstEnumeratorConfig inst_enumerator_config;
    ConstInstSlice ref_prog;
} SuperOptimizerConfig;

typedef struct CounterExample {
    InstMachine inp;
    InstMachine out;
} CounterExample;

DECLARE_ARRAY(CounterExample, CounterExampleArray);

typedef struct SuperOptimizer {
    // ------ Things that change ------
    InstEnumerator inst_enumerator;
    InstArrayArray prefixes;
    uint16_t prefix_len;
    InstArrayArray next_prefixes;
    size_t i_prefix;
    CounterExampleArray counter_examples;
    // ------ Things that don't ------
    InstEnumeratorConfig inst_enumerator_config;
    ConstInstSlice ref_prog;
} SuperOptimizer;

typedef enum SuperOptimizerResultType {
    /// The super-optimizer found a program that matches the specification. The
    /// program can be read from the super-optimizer itself.
    SUPER_OPTIMIZER_RESULT_TYPE_FOUND,
    /// The super-optimizer searched the entire space and found no more matching
    /// programs.
    SUPER_OPTIMIZER_RESULT_TYPE_DONE,
} SuperOptimizerResultType;

/// The result of searching for the next matching program by a super-optimizer.
typedef struct SuperOptimizerResult {
    SuperOptimizerResultType type;
    union {
        /// This reference is valid only as long as the super-optimizer is not
        /// modified.
        ConstInstSlice found;
    } data;
} SuperOptimizerResult;

SuperOptimizer super_optimizer_new(const SuperOptimizerConfig* c);

void super_optimizer_free(SuperOptimizer* s);

SuperOptimizerResult super_optimizer_step(SuperOptimizer* s);
