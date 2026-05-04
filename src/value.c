#include "value.h"
#include "str.h"
#include <inttypes.h>

Value value_int(int64_t i) {
    Value value = { VALUE_INT, { .i = i } };
    return value;
}

char* value_to_str(Value v) {
    switch (v.kind) {
        case VALUE_INT: return str_format("%" PRId64, v.data.i);
    }
}
