#include "str.h"
#include "array.h"
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

bool str_starts_with(const char *str, const char *prefix)
{
    while (prefix[0] != 0)
    {
        if (str[0] != prefix[0])
            return false;
        str++;
        prefix++;
    }
    return true;
}

void str_remove_newline(char *str)
{
    for (; *str != 0; str++)
    {
        if (*str == '\n')
        {
            *str = 0;
            return;
        }
    }
}

void* malloc(size_t s);
char* str_clone(const char *s) {
    size_t len = strlen(s);
    char* ret = malloc(len + 1);
    if (ret == NULL) return NULL;
    memcpy(ret, s, len + 1);
    return ret;
}

CharArray str_to_array(char* str) {
    size_t len = strlen(str);
    CharArray arr = { .ptr = str, .len = len, .cap = len };
    return arr;
}

char* str_format(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char* ret = str_format_va_list(fmt, args);
    va_end(args);
    return ret;
}

char* str_format_va_list(const char* fmt, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);
    // Get the length of the formatted string.
    int len = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);
    if (len < 0) return NULL;
    // Allocate a buffer for the formatted string.
    char* buf = malloc(len + 1);
    if (buf == NULL) return NULL;
    // Write the formatted string to the buffer.
    vsnprintf(buf, len + 1, fmt, args);
    return buf;
}
