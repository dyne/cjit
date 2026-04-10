#include "muntarfs.h"

#include "../../src/muntar.h"

int muntarfs_extract_tar_to_path(const char *destination_path,
                                 const uint8_t *tar_data,
                                 unsigned int tar_length)
{
    return muntar_to_path(destination_path, tar_data, tar_length);
}

int muntarfs_extract_targz_to_path(const char *destination_path,
                                   const uint8_t *targz_data,
                                   unsigned int targz_length)
{
    return muntargz_to_path(destination_path, targz_data, targz_length);
}
