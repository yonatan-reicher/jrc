#ifdef BIN

#include "basic.h"
#include "path.h"
#include "str.h"
#include "stream.h"
#include <dirent.h>

Path get_header_dir_path() {
    Path p = path_concat(path_cwd(), path_parse("include"));
    char* s = path_to_str(&p);
    EXPECT(path_exists(&p), "'%s' exists", s);
    free(s);
    return p;
}

bool check_header_file(const char* file_path_str, Stream s) {
    Stream pragma_once = stream_of_str("#pragma once");
    bool has_pragma_once = stream_starts_with(stream_clone(&s), pragma_once);
    if (!has_pragma_once) {
        LOG("Error", "file '%s' is missing '#pragma once'", file_path_str);
        return false;
    }
    return true;
}

int main(void) {
    Path header_dir_path = get_header_dir_path();
    char* header_dir_path_str = path_to_str(&header_dir_path);
    DIR* dir;
    struct dirent* ent;
    EXPECT_ERRNO(
        dir = opendir(header_dir_path_str),
        "open directory '%s'",
        header_dir_path_str
    );
    bool err = false;
    // go through all headers files and check them
    while ((ent = readdir(dir)) != NULL) {
        if (str_eq(ent->d_name, ".") || str_eq(ent->d_name, "..")) continue;
        Path p = path_clone(&header_dir_path);
        path_add(
            &p, str_of_slice((CharSlice)slice_new(ent->d_name, ent->d_namlen))
        );
        Stream s = path_open_stream(&p);
        char* p_str = path_to_str(&p);
        path_free(&p);
        err |= !check_header_file(p_str, s);
        free(p_str);
    }
    closedir(dir);
    return err ? EXIT_FAILURE : EXIT_SUCCESS;
}

#else
int placeholder = 0;
#endif
