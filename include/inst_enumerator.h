#pragma once
#include "inst.h"
#include "slice.h"
#include <stdint.h>
#include <stdbool.h>

// =============================================================================
//                               InstEnumerator.h
// =============================================================================
// Enumerates over a family of instructions, defined by an enumeration
// configuration. First create an enumerator with `inst_enumerator_new`, then
// call `inst_enumerator_has_current` to check if it has a current instruction.
// If not, ask for it by calling `inst_enumerator_advance`. Read it by
// `inst_enumerator_current`. Finally, check if it's done by
// `inst_enumerator_is_done`.
// -----------------------------------------------------------------------------

typedef struct InstEnumeratorConfig {
    ConstUInt8Slice immediates;
    ConstUInt8Slice registers;
} InstEnumeratorConfig;

typedef struct InstEnumerator {
    InstOpCode op_code;
    uint8_t arg_indices[3];
    bool is_done;
    InstEnumeratorConfig config;
} InstEnumerator;

/// Keeps a pointer to the config.
InstEnumerator inst_enumerator_new(const InstEnumeratorConfig*);

/// Like `inst_enumerator_new`, but also advances to the first instruction if it
/// needs to. Keeps a pointer to the config.
InstEnumerator inst_enumerator_new_ready(const InstEnumeratorConfig*);

/// If does not have, you need to ask for the next first. Only happens before
/// calling the first next.
bool inst_enumerator_has_current(const InstEnumerator*);

bool inst_enumerator_is_done(const InstEnumerator*);

Inst inst_enumerator_current(const InstEnumerator* enumerator);

bool inst_enumerator_advance(InstEnumerator* enumerator);
