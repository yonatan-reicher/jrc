#include "plot.h"
#include "array.h"
#include "basic.h"
#include "memory.h"
#include "slice.h"
#include <math.h>
#include <wchar.h>

DECLARE_SLICE(const float, ConstFloatSlice);
DECLARE_SLICE(float, FloatSlice);
DECLARE_ARRAY(float, FloatArray);

static double sample_data(ConstFloatSlice data, double fractional_index) {
    fractional_index = MAX(0, MIN(fractional_index, (double)(data.len - 1)));
    float a = *slice_get(&data, (size_t)floor(fractional_index));
    float b = *slice_get(&data, (size_t)ceil(fractional_index));
    double t = fractional_index - floor(fractional_index);
    return a * (1 - t) + b * t;
}

/// Takes data from the input array and writes to the output array by sampling
/// between or something like that.
static void adjust_data(ConstFloatSlice inp, FloatSlice out) {
    EXPECT(inp.len > 0, "must have at least one input");
    double max = 0;
    SLICE_FOREACH(&inp, i) max = MAX(max, *i);
    // A fractional index into the input array.
    double i_inp = 0;
    SLICE_FOREACH(&out, o) {
        double i_out_percent = (double)(i_o + 1) / (double)out.len;
        double i_inp_next = i_out_percent * (double)inp.len;
        double sum = 0;
        double area = i_inp_next - i_inp;
        while (i_inp < i_inp_next) {
            // Two cases: either both fractional indices are between the same
            // elements, or there is some element between them.
            if (floor(i_inp) == floor(i_inp_next)) { /* same */
                double i = 0.5 * (i_inp + i_inp_next);
                sum += sample_data(inp, i) * (i_inp_next - i_inp);
                break;
            }
            sum += sample_data(inp, i_inp + 0.5);
            i_inp += 1;
        }
        i_inp = i_inp_next;
        double avg = sum / area;
        *o = (float)(avg / max);
    }
}

void plot_bar(
    ConstFloatSlice data,
    unsigned int width,
    unsigned int height,
    const wchar_t* x_right_tick_label,
    const wchar_t* y_top_tick_label
) {
    plot_bar_to_file(
        stdout, data, width, height, x_right_tick_label, y_top_tick_label
    );
}

void plot_bar_to_file(
    FILE* f,
    ConstFloatSlice data,
    unsigned int width,
    unsigned int height,
    const wchar_t* x_right_tick_label,
    const wchar_t* y_top_tick_label
) {
    EXPECT(width > 0, "width must be positive");
    EXPECT(height > 2, "height must be at least three");
    size_t buf_size = (size_t)width * (size_t)height * sizeof(wchar_t);
    wchar_t* buf = malloc(buf_size);
    plot_bar_to_buf(
        buf, data, width, height, x_right_tick_label, y_top_tick_label
    );
    // Print.
    for (unsigned i = 0; i < height; i++)
        fwprintf(f, L"%.*ls\n", (int)width, &buf[i * (size_t)width]);
    free(buf);
}

static void write_to_buf(unsigned x, unsigned y, wchar_t c, void* arg) {
    struct {
        wchar_t* buf;
        unsigned width;
    }* a = arg;
    a->buf[(size_t)x + (size_t)y * (size_t)a->width] = c;
}

void plot_bar_to_buf(
    wchar_t* buf,
    ConstFloatSlice data,
    unsigned int width,
    unsigned int height,
    const wchar_t* x_right_tick_label,
    const wchar_t* y_top_tick_label
) {
    struct {
        wchar_t* buf;
        unsigned width;
    } arg = { buf, width };
    plot_bar_to_func(
        write_to_buf,
        &arg,
        data,
        width,
        height,
        x_right_tick_label,
        y_top_tick_label
    );
}

void plot_bar_to_func(
    void f(unsigned x, unsigned y, wchar_t, void*),
    void* f_arg,
    ConstFloatSlice data,
    unsigned int width,
    unsigned int height,
    const wchar_t* x_right_tick_label,
    const wchar_t* y_top_tick_label
) {
    EXPECT(width > 0, "width must be positive");
    EXPECT(height > 2, "height must be at least three");
    // Start with all spaces.
    for (unsigned i = 0; i < height; i++)
        for (unsigned j = 0; j < width; j++)
            f(j, i, L' ', f_arg);
    // Y ticks.
    unsigned y_label_len = (unsigned)wcslen(y_top_tick_label);
    unsigned x_label_len = (unsigned)wcslen(x_right_tick_label);
    EXPECT(
        y_label_len < width,
        "The y tick label should be less than the total width of the image"
    );
    unsigned n_y_tick_columns = MAX(1, y_label_len);
    for (unsigned i = 0; i < y_label_len; i++)
        f(i, 0, y_top_tick_label[i], f_arg);
    f(n_y_tick_columns - 1, height - 2, L'0', f_arg);
    // X ticks.
    f(n_y_tick_columns + 1 /* padding */, height - 1, L'0', f_arg);
    EXPECT(n_y_tick_columns + 1 + x_label_len < width, "not wide enough");
    for (unsigned i = 0; i < x_label_len; i++)
        f(width - x_label_len + i, height - 1, x_right_tick_label[i], f_arg);
    // Border.
    f(n_y_tick_columns + 1 /* padding */, 0, L'┌', f_arg);
    f(n_y_tick_columns + 1 /* padding */, height - 2, L'└', f_arg);
    f(width - 1, 0, L'┐', f_arg);
    f(width - 1, height - 2, L'┘', f_arg);
    for (unsigned i = n_y_tick_columns + 2; i < width - 1; i++) {
        f(i, 0, L'─', f_arg);
        f(i, height - 2, L'─', f_arg);
    }
    for (unsigned i = 1; i < height - 2; i++) {
        f(n_y_tick_columns + 1, i, L'│', f_arg);
        f(width - 1, i, L'│', f_arg);
    }
    // Data.
    size_t inner_width =
        width - 2 /* border */ - 1 /* padding */ - n_y_tick_columns;
    FloatArray heights;
    array_init(&heights, inner_width);
    adjust_data(data, (FloatSlice)slice_of_array(&heights));
    size_t max_height = height - 2 /* border */ - 1 /* x ticks */;
    ARRAY_FOREACH(&heights, h) {
        unsigned x =
            n_y_tick_columns + 1 /* padding */ + 1 /* border */ + (unsigned)i_h;
        *h = MAX(0, MIN(*h, 1));
        unsigned i = 0;
        for (i = 0; (double)i < (double)max_height * *h; i++) {
            unsigned y = height - 1 - 1 /* x ticks */ - 1 /* border */ - i;
            f(x, y, L'█', f_arg);
        }
        // Half height!
        if ((double)i <
            MIN((double)max_height * *h + 0.5, (double)max_height)) {
            unsigned y = height - 1 - 1 /* x ticks */ - 1 /* border */ - i;
            f(x, y, L'▄', f_arg);
        }
    }
}
