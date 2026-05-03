#include <stdbool.h>
#include <string.h>

/// Does the first c-string begin with the second?
bool str_starts_with(const char *str, const char *prefix);
/// If the last character of a c-string is a newline, removes it.
void str_remove_newline(char *str);
/// Shorthand for `strcmp(..) == 0`
#define str_eq(A, B) (strcmp(A, B) == 0)
/// Duplicate the string. Must be freed!
char* str_clone(const char *str);
/// Takes a c-string, owned, and returns it as a character array. The string
/// becomes owned by the array.
struct CharArray str_to_array(char* str);
/// Format a string using `printf`-style formatting. The returned string must be
/// freed.
char* str_format(const char* fmt, ...);
