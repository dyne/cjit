/* muntarfs, part of CJIT
 *
 * Copyright (C) 2024 Dyne.org foundation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

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
