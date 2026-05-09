#pragma once

/// Mask just the first set bit of an integer.
#define FIRST_BIT(I) ((I) & -(I)) /* it's fine to negate unsigned values. */
/// Mask all but the first set bit of an integer.
#define ALL_BUT_FIRST_BIT(I) ((I) & ((I) - 1))

#define IS_EXACTLY_ONE_BIT(I) ((I) != 0 && FIRST_BIT(I) == (I))
