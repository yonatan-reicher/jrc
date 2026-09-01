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
    struct ConstFloatSlice data,
    unsigned int width,
    unsigned int height,
    const wchar_t* x_right_tick_label,
    const wchar_t* y_top_tick_label
);

void plot_bar_to_file(
    FILE* f,
    struct ConstFloatSlice data,
    unsigned int width,
    unsigned int height,
    const wchar_t* x_right_tick_label,
    const wchar_t* y_top_tick_label
);

/// The buffer must have width times height wide characters.
void plot_bar_to_buf(
    wchar_t* buf,
    struct ConstFloatSlice data,
    unsigned int width,
    unsigned int height,
    const wchar_t* x_right_tick_label,
    const wchar_t* y_top_tick_label
);

void plot_bar_to_func(
    void f(unsigned x, unsigned y, wchar_t, void*),
    void* f_arg,
    struct ConstFloatSlice data,
    unsigned int width,
    unsigned int height,
    const wchar_t* x_right_tick_label,
    const wchar_t* y_top_tick_label
);
