#include "../src/path.h"

int main(void) {
    Path p = path_cwd();
    Path q = path_parse("/Users/jr/projects/jrc/");
    if (!path_eq(&p, &q)) {
        PANIC(
            "The given paths were not equal! \n'%s'\n'%s'",
            path_to_str(&p),
            path_to_str(&q)
        );
    }
    path_free(&p);
    path_free(&q);
    if (!path_is_empty(&p) || !path_is_empty(&q))
        PANIC("paths should become empty after free");
    return 0;
}
