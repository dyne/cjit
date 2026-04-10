#ifndef MUNTARFS_H
#define MUNTARFS_H

#include <stdint.h>

/**
 * Extract a tar bundle into the destination directory.
 *
 * Returns 0 on success and a non-zero library-specific error code on failure.
 */
int muntarfs_extract_tar_to_path(const char *destination_path,
                                 const uint8_t *tar_data,
                                 unsigned int tar_length);

/**
 * Extract a tar.gz bundle into the destination directory.
 *
 * Returns 0 on success and a non-zero library-specific error code on failure.
 */
int muntarfs_extract_targz_to_path(const char *destination_path,
                                   const uint8_t *targz_data,
                                   unsigned int targz_length);

#endif
