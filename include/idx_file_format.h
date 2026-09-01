#pragma once

#include "array.h"
#include "mat.h"
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/// An error occurring when reading an .idx file.
typedef enum IdxReadErr : uint8_t {
    IDX_ERR_NONE,
    IDX_ERR_NO_MAGIC,
    IDX_ERR_ZERO_DIMENSIONS,
    IDX_ERR_NOT_ENOUGN_SIZES,
    IDX_ERR_NOT_ENOUGH_DATA,
} IdxReadErr;

const char* idx_read_err_to_str(IdxReadErr);

typedef enum IdxItemType : uint8_t {
    IDX_ITEM_TYPE_U8,
    IDX_ITEM_TYPE_I8,
    IDX_ITEM_TYPE_I16,
    IDX_ITEM_TYPE_I32,
    IDX_ITEM_TYPE_F32,
    IDX_ITEM_TYPE_F64,
} IdxItemType;
#define N_IDX_ITEM_TYPES 6

/// Indexable by `IdxItemType`, a table of matching magic bytes.
static uint8_t idx_item_type_magic_byte_table[] = {
    0x08, // U8
    0x09, // I8
    0x0B, // I16
    0x0C, // I32
    0x0D, // F32
    0x0E, // F64
};

typedef struct IdxReadResult {
    UInt8Array data;
    UInt32Array shape;
    IdxItemType item_type;
    IdxReadErr err;
} IdxReadResult;

void idx_read_result_free(IdxReadResult*);

/// Reads a file in the IDX file format. This is the format used by the MNIST
/// dataset.
/// On error, returns false and sets the error.
IdxReadResult idx_file_read(FILE* f);

struct FloatArray;
IdxReadErr idx_file_read_mnist_images(FILE* f, struct FloatArray* out_data);
