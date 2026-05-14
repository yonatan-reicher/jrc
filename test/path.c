#include "path.h"
#include "str.h"
#include <limits.h>
#include <unistd.h>

int main(void) {
    // Check the result of `path-cwd`
    Path p = path_cwd();
    char *cwd_from_path = path_to_str(&p);
    path_free(&p);
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    if (!str_eq(cwd_from_path, cwd)) {
        PANIC(
            "The given paths were not equal! \n'%s'\n'%s'",
            cwd_from_path,
            cwd
        );
    }
    free(cwd_from_path);
    // Assert that after the path has been cleared, it is empty.
    if (!path_is_empty(&p)) PANIC("paths should become empty after free");
    return 0;
}
