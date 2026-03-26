#include "path.h"
#include "str.h"
#include <stdlib.h>
#include <unistd.h>

// --- Some helpers to help with the implementation -------------

#define IS_SEP(X) ((X) == '\\' || (X) == '/')

// -------------------------------------------------------------

Path path_empty(void) {
    return (Path) { .parts = NULL, .n_parts = 0 };
}

bool path_is_empty(const Path *p) {
    return p->parts == NULL;
}

void path_free(Path *p) {
    if (path_is_empty(p)) return;
    for (uint16_t i = 0; i < p->n_parts; i++) free(p->parts[i]);
    free(p->parts);
    *p = path_empty();
}

Path path_move(Path *p) {
    Path ret = *p;
    *p = path_empty();
    return ret;
}

void path_add(Path *dest, const char *part) {
    const size_t new_size = (dest->n_parts + 1) * sizeof(char*);
    dest->parts = realloc(dest->parts, new_size);
    dest->parts[dest->n_parts] = str_clone(part);
    dest->n_parts++;
}

void path_append(Path *dest, Path src) {
    const uint16_t new_n_parts = dest->n_parts + src.n_parts;
    if (new_n_parts < dest->n_parts) PANIC("integer overflow");
    // Update
    dest->parts = realloc(dest->parts, new_n_parts * sizeof(char*));
    memcpy(&dest->parts[dest->n_parts], src.parts, new_n_parts);
    dest->n_parts = new_n_parts;
    // Free
    free(src.parts);
}

Path path_concat(Path lhs, Path rhs) {
    path_append(&lhs, rhs);
    return path_move(&lhs);
}

Path path_of_slice(const PtrSlice(char) parts) {
    Path ret = path_empty();
    for (uint16_t i = 0; i < parts.len; i++) path_add(&ret, parts.ptr[i]);
    return ret;
}

Path path_clone(const Path *src) {
    Path ret;
    ret.parts = malloc(src->n_parts * sizeof(char*));
    for (uint16_t i = 0; i < src->n_parts; i++)
        ret.parts[i] = str_clone(src->parts[i]);
    ret.n_parts = src->n_parts;
    return ret;
}

bool path_eq(const Path *a, const Path *b) {
    if (a->n_parts != b->n_parts) return false;
    for (uint16_t i = 0; i < a->n_parts; i++)
        if (!str_eq(a->parts[i], b->parts[i])) return false;
    return true;
}

Path path_parse(const char *path_str) {
    return path_parse_n(path_str, strlen(path_str));
}

Path path_parse_n(const char *path_str, size_t len) {
    // Declare some variables.
    Path ret = path_empty();
    char part[2000] = "";
    size_t part_len = 0;
#define PART_MAX_LEN (ARRAY_LEN(part))
#define PART_TOO_BIG ("path part was too big - max size is " STR(PART_MAX_LEN))
    // Actual code.
    for (size_t i = 0; i < len; i++) {
        const char c = path_str[i];
        if (IS_SEP(c)) {
            part[part_len] = 0; // Finish the string
            path_add(&ret, part); // Add it
            part_len = 0; // Reset it
        } else {
            part[part_len++] = c;
            if (part_len == ARRAY_LEN(part)) PANIC(PART_TOO_BIG);
        }
    }
    // Finish the final part, if there is one!
    if (part_len > 0) {
        part[part_len] = 0; // Finish the string
        path_add(&ret, part); // Add it
    }
    return ret;
}

Path path_cwd(void) {
    char *cwd_str;
    EXPECT_ERRNO(cwd_str = getcwd(NULL, 0)); // Calling with NULL means allocating.
    // TODO: What if this crashes?
    Path ret = path_parse(cwd_str);
    free(cwd_str);
    return ret;
}

char* path_to_str(const Path *p) {
    // Just in case, allocate 5 + Σ (n + 5) for every part of length n.
    size_t size = 5;
    for (uint16_t i = 0; i < p->n_parts; i++) {
        size += strlen(p->parts[i]);
    }
    char *buf = malloc(size * sizeof(char));
    size_t i = 0;
    for (uint16_t i_part = 0; i_part < p->n_parts; i_part++) {
        const char *part = p->parts[i_part];
        size_t len = strlen(part);
        memcpy(&buf[i], part, len);
        i += len;
        buf[i++] = i_part == p->n_parts - 1 ? 0 : '/';
    }
    return buf;
}
