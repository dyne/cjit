#include "support/source_files.h"

#include <stdbool.h>
#include <stddef.h>

#include "cwalk.h"

int cjit_classify_source_path(const char *path)
{
    char *extension;
    size_t extension_length;
    bool is_source;

    is_source = cwk_path_get_extension(path, (const char **)&extension, &extension_length);
    if (!is_source) {
        return 0;
    }
    if (extension_length == 2 && (extension[1] == 'c' || extension[1] == 'C')) {
        return 1;
    }
    if (extension_length == 3
        && (extension[1] == 'c' || extension[1] == 'C')
        && (extension[2] == 'c' || extension[2] == 'C')) {
        return 1;
    }
    if (extension_length == 4
        && (extension[1] == 'c' || extension[1] == 'C')
        && (extension[2] == 'x' || extension[2] == 'X')
        && (extension[3] == 'x' || extension[3] == 'X')) {
        return 1;
    }
    return -1;
}
