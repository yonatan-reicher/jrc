#include "plot.h"
#include "array.h"
#include "basic.h"
#include "memory.h"
#include "slice.h"
#include <locale.h>
#include <math.h>
#include <wchar.h>

DECLARE_SLICE(const float, ConstFloatSlice);
DECLARE_SLICE(float, FloatSlice);
DECLARE_ARRAY(float, FloatArray);

wchar_t heights[] = {
    L'█',
    L'▄',
    L'_',
};

static double sample_data(ConstFloatSlice data, double fractional_index) {
    fractional_index = MAX(0, MIN(fractional_index, data.len - 1));
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
        double i_out_percent = (double)i_o / (double)out.len;
        double i_inp_next = i_out_percent * inp.len;
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
    FILE* f,
    ConstFloatSlice data,
    unsigned int width,
    unsigned int height,
    const wchar_t* x_right_tick_label,
    const wchar_t* y_top_tick_label
) {
    EXPECT(width > 0, "width must be positive");
    EXPECT(height > 2, "height must be at least three");
    size_t buf_size = ((size_t)width + 1) * (size_t)height * sizeof(wchar_t);
    wchar_t* buf = malloc(buf_size);
#define AT(X, Y) (buf[(size_t)(X) + (size_t)(Y) * ((size_t)width + 1)])
    // Start with all spaces.
    for (size_t i = 0; i < buf_size / sizeof(wchar_t); i++) buf[i] = L' ';
    // Newlines.
    for (size_t i = 0; i < height; i++) AT(width, i) = L'\n';
    // Y ticks.
    size_t y_label_len = wcslen(y_top_tick_label);
    size_t x_label_len = wcslen(x_right_tick_label);
    EXPECT(
        y_label_len < width,
        "The y tick label should be less than the total width of the image"
    );
    size_t n_y_tick_columns = MAX(1, y_label_len);
    memcpy(buf, y_top_tick_label, y_label_len * sizeof(wchar_t));
    AT(n_y_tick_columns - 1, height - 2) = L'0';
    // X ticks.
    AT(n_y_tick_columns + 1 /* padding */, height - 1) = L'0';
    EXPECT(n_y_tick_columns + 1 + x_label_len < width, "not wide enough");
    memcpy(
        &AT(width - x_label_len, height - 1),
        x_right_tick_label,
        x_label_len * sizeof(wchar_t)
    );
    // Border.
    AT(n_y_tick_columns + 1 /* padding */, 0) = L'┌';
    AT(n_y_tick_columns + 1 /* padding */, height - 2) = L'└';
    AT(width - 1, 0) = L'┐';
    AT(width - 1, height - 2) = L'┘';
    for (size_t i = n_y_tick_columns + 2; i < width - 1; i++) {
        AT(i, 0) = L'─';
        AT(i, height - 2) = L'─';
    }
    for (size_t i = 1; i < height - 2; i++) {
        AT(n_y_tick_columns + 1, i) = L'│';
        AT(width - 1, i) = L'│';
    }
    // Data.
    size_t inner_width =
        width - 2 /* border */ - 1 /* padding */ - n_y_tick_columns;
    FloatArray heights;
    array_init(&heights, inner_width);
    adjust_data(data, (FloatSlice)slice_of_array(&heights));
    size_t max_height = height - 2 /* border */ - 1 /* x ticks */;
    ARRAY_FOREACH(&heights, h) {
        size_t x = n_y_tick_columns + 1 /* padding */ + 1 /* border */ + i_h;
        *h = MAX(0, MIN(*h, 1));
        size_t i = 0;
        for (i = 0; i < max_height * *h; i++) {
            size_t y = height - 1 - 1 /* x ticks */ - 1 /* border */ - i;
            AT(x, y) = L'█';
        }
        // Half height!
        if (i < MIN(max_height * *h + 0.5, max_height)) {
            size_t y = height - 1 - 1 /* x ticks */ - 1 /* border */ - i;
            AT(x, y) = L'▄';
        }
    }
    // Print.
    setlocale(LC_ALL, "");
    char buf2[BUFSIZ] = { };
    setbuffer(f, buf2, sizeof(buf2));
    fwprintf(f, L"%.*ls", buf_size / sizeof(*buf), buf);
    fflush(f);
    setlinebuf(f);
#undef AT
    free(buf);
}
