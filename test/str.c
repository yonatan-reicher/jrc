#include "../src/str.h"
#include "../src/basic.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    // --- str_starts_with -----------------------------------------------------
    if (!str_starts_with("Hello World", "Hell"))
        PANIC("!!");
    if (str_starts_with("Hello World", "hell"))
        PANIC("!!");
    if (!str_starts_with("Hello World", "Hello World"))
        PANIC("!!");
    if (!str_starts_with("Hello\0World", "Hello\0Netanya"))
        PANIC("As these are C-strings, zero terminator marks ends of strings "
              "and should be respected.");
    // --- str_remove_newline --------------------------------------------------
#define MY_STR "hello world with a two newlines!\n\n"
    char str[] = MY_STR;
    str_remove_newline(str);
    if (str_eq(str, MY_STR))
    {
    }
    return 0;
}
