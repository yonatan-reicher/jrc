#include "idx_file_format.h"
#include "endianness.h"
#include <stdlib.h>

static_assert(ARRAY_LEN(idx_item_type_magic_byte_table) == N_IDX_ITEM_TYPES);

const char* idx_read_err_to_str(IdxReadErr e) {
    switch (e) {
        case IDX_ERR_NONE: return NULL;
        case IDX_ERR_NO_MAGIC: return "idx file magic byte is incorrect";
        case IDX_ERR_ZERO_DIMENSIONS:
            return "idx file data dimensions was zero";
        case IDX_ERR_NOT_ENOUGN_SIZES:
            return "idx file did not have enough sizes";
        case IDX_ERR_NOT_ENOUGH_DATA: return "idx file was missing some data";
    }
}

static uint8_t idx_item_type_size(IdxItemType t) {
    switch (t) {
        case IDX_ITEM_TYPE_U8: return 1;
        case IDX_ITEM_TYPE_I8: return 1;
        case IDX_ITEM_TYPE_I16: return 2;
        case IDX_ITEM_TYPE_I32: return 4;
        case IDX_ITEM_TYPE_F32: return 4;
        case IDX_ITEM_TYPE_F64: return 8;
    }
}

void idx_read_result_free(IdxReadResult* r) {
    if (r == NULL) return;
    array_free(&r->data);
    array_free(&r->shape);
}

#define ERR(NAME)                                                              \
    do {                                                                       \
        idx_read_result_free(&ret);                                            \
        ret.err = IDX_ERR_##NAME;                                              \
        return ret;                                                            \
    } while (0)

IdxReadResult idx_file_read(FILE* f) {
    IdxReadResult ret = {};
    ret.data = (typeof(ret.data))array_empty();
    ret.shape = (typeof(ret.shape))array_empty();
    // The format starts with a "magic number". This format's magic number
    // starts with 2 zero bytes, followed by a byte specifying the datatype and
    // then a number of dimensions.
    uint8_t magic[4];
    unsigned long n_read = fread(magic, 1, 4, f);
    if (n_read != 4 || magic[0] != 0 || magic[1] != 0) ERR(NO_MAGIC);
    bool item_type_found = false;
    for (IdxItemType t = 0; t < N_IDX_ITEM_TYPES; t++) {
        if (magic[2] == idx_item_type_magic_byte_table[t]) {
            ret.item_type = t;
            item_type_found = true;
            break;
        }
    }
    if (!item_type_found) ERR(NO_MAGIC);
    uint8_t n_dimensions = magic[3];
    if (n_dimensions == 0) ERR(ZERO_DIMENSIONS);
    // Done! Now comes the shape
    array_resize(&ret.shape, n_dimensions);
    n_read = fread(ret.shape.ptr, sizeof(uint32_t), n_dimensions, f);
    if (n_read != n_dimensions) ERR(NOT_ENOUGN_SIZES);
    //      (Convert shape endianness.)
    for (uint8_t i = 0; i < n_dimensions; i++) {
        uint32_t* x = array_get(&ret.shape, i);
        uint32_t n = *x;
        UInt8Slice s = slice_new((uint8_t*)&n, sizeof(n));
        endianness_of_big(s);
        *x = n;
    }
    // and now the data.
    size_t n_items = 1;
    size_t item_size = idx_item_type_size(ret.item_type);
    for (uint8_t i = 0; i < n_dimensions; i++) n_items *= ret.shape.ptr[i];
    array_resize(&ret.data, item_size * n_items);
    n_read = fread(ret.data.ptr, item_size, n_items, f);
    if (n_read != n_items) ERR(NOT_ENOUGH_DATA);
    //      (also convert some endianness.)
    for (size_t i = 0; i < n_items; i++) {
        void* item = array_get(&ret.data, i * item_size);
        UInt8Slice s = slice_new(item, item_size);
        endianness_of_big(s);
    }
    return ret;
}

DECLARE_ARRAY(float, FloatArray);

IdxReadErr idx_file_read_mnist_images(FILE* f, struct FloatArray* out_data) {
    IdxReadErr ret = 0;
    IdxReadResult r = idx_file_read(f);
    UInt8Array* data = &r.data;
    UInt32Array* shape = &r.shape;
    if (r.err) {
        idx_read_result_free(&r);
        return r.err;
    }
    EXPECT(
        shape->len == 3,
        "this .idx file is supposed to have 3 dimensions, but had a shape with "
        "%zu dimensions",
        shape->len
    );
    EXPECT(
        *array_get(shape, 1) == 28 && *array_get(shape, 2) == 28,
        "this .idx is supposed to be something by 28 by 28, but was %d by %d "
        "by %d",
        *array_get(shape, 0),
        *array_get(shape, 1),
        *array_get(shape, 2)
    );
    EXPECT(
        r.item_type == IDX_ITEM_TYPE_U8,
        "this .idx file is supposed to have bytes."
    );
    float* a = malloc(sizeof(float) * data->len);
    *out_data = (FloatArray) { a, data->len, data->len };
    for (size_t i = 0; i < data->len; i++) {
        uint8_t x = *array_get(data, i * idx_item_type_size(r.item_type));
        a[i] = (float)x / 255.0f;
    }
    return ret;
}
