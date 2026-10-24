#include "path.h"
#include "str.h"
#include <limits.h>
#include <unistd.h>

void test_empty_path_to_str(void) {
    Path p = path_empty();
    char* s = path_to_str(&p);
    EXPECT(
        str_eq(s, ""),
        "expected the empty path to return an empty string, but it was '%s'",
        s
    );
    free(s);
}

void test_root_has_one_part(void) {
    Path p = path_root();
    EXPECT(p.n_parts == 1, "Expected root path to have exactly one part");
    path_free(&p);
}

void test_root_path_to_str(void) {
    Path p = path_root();
    char* s = path_to_str(&p);
    path_free(&p);
    EXPECT(str_eq(s, "/"), "`path_to_str(&path_root())` != \"/\", was '%s'", s);
    free(s);
}

void test_root_is_parse_slash(void) {
    Path p1 = path_parse("/");
    Path p2 = path_root();
    char* s = path_to_str(&p1);
    EXPECT(path_eq(&p1, &p2), "Expected `path_parse(\"/\")` to be '/' but was '%s'", s);
    free(s);
    path_free(&p1);
    path_free(&p2);
}

void test_os_stuff(void) {
    Path p = path_cwd();
    EXPECT(path_exists(&p), "the current directory should exist...");
    path_free(&p);
}

int main(void) {
    // Check the result of `path-cwd`
    Path p = path_cwd();
    char* cwd_from_path = path_to_str(&p);
    path_free(&p);
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    if (!str_eq(cwd_from_path, cwd)) {
        PANIC(
            "The given paths were not equal! \n'%s'\n'%s'", cwd_from_path, cwd
        );
    }
    free(cwd_from_path);
    // Assert that after the path has been cleared, it is empty.
    if (!path_is_empty(&p)) PANIC("paths should become empty after free");

    test_empty_path_to_str();
    test_os_stuff();

    return 0;
}
