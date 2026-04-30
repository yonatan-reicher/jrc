#pragma once
#include "slice.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/// A platform-agnostic file-system path.
typedef struct Path {
    /// An array of path parts.
    char **parts;
    uint16_t n_parts;
} Path;

DEFINE_PTR_SLICE(char);

/// Returns a new empty path.
Path path_empty(void);
bool path_is_empty(const Path *path);
/// Free a path.
void path_free(Path *path);
/// Replaces a path with an empty one.
Path path_move(Path *path);
/// Replace a path with a new path.
void path_assign(Path *dest, Path src);
/// Adds a single part to the right of the string. Takes that part by reference.
void path_add(Path *dest, char *part);
/// Adds a path to another path by reference. Destroys the path given.
void path_append(Path *dest, Path src);
/// Adds a path to another, destroying both.
Path path_concat(Path lhs, Path rhs);
/// Builds a path from a slice of parts, as c-strings.
Path path_of_slice(const PtrSlice(char) parts);
/// Clone a path.
Path path_clone(const Path *p);
bool path_eq(const Path *a, const Path *b);
/// Parses a path from a c-string. The string is as you would write a regular
/// path, with the separators being either '/' '\', both are accepted. Whether
/// the path ends with a separator of not (`/usr/local/` vs `/usr/local`) does
/// not matter, and so does whether it begins with one or not. `.` and `..` are
/// respected and stored in the path.
Path path_parse(const char *path_str);
/// Parses a path from a length-ed string. Look at `path_parse`
Path path_parse_n(const char *path_str, size_t len);
/// Returns a new path to the current working directory.
Path path_cwd(void);
/// Returns a new path to the user's home directory.
Path path_home(void);
/// Turn a string into a system representation.
char* path_to_str(const Path *p);
/// Takes a path, expands '~', expands '.', and returns it.
Path path_expand(Path src);


#define path(...) \
    path_of_slice( ptr_slice(char, ARRAY_LEN((char*[]) { __VA_ARGS__ }), (char*[]) { __VA_ARGS__ } ) )
