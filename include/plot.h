#pragma once
#include <stdio.h>
#include <wchar.h>

struct ConstFloatSlice;

/// Example:
/// 50 ┌───────────────────────────────────┐
///    │                                   │
///    │                                   │
///    │          ██████████               │
///    │    ██████████████████             │
///    │   ████████████████████            │
///    │ ███████████████████████           │
///    │██████████████████████████       ██│
///    │████████████████████████████   ████│
///  0 └───────────────────────────────────┘
///    0                                 100
void plot_bar(
    FILE* f,
    struct ConstFloatSlice data,
    unsigned int width,
    unsigned int height,
    const wchar_t* x_right_tick_label,
    const wchar_t* y_top_tick_label
);
