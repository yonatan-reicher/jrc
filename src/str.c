#include "string.h"
#include <stdbool.h>

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
