/*
 * tinfgz - tiny inflate library for gzip only
 *
 * Copyright (C) 2024 Dyne.org foundation
 *                  Maintained by Jaromil
 *
 * based on a stripped down tinf
 * Copyright (c) 2003-2019 Joergen Ibsen
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
 */

#ifndef TINF_H_INCLUDED
#define TINF_H_INCLUDED

/**
 * Status codes returned.
 *
 * @see tinf_uncompress, tinf_gzip_uncompress
 */
typedef enum {
	TINF_OK         = 0,  /**< Success */
	TINF_DATA_ERROR = -3, /**< Input error */
	TINF_BUF_ERROR  = -5  /**< Not enough room for output */
} tinf_error_code;

/**
 * Decompress `sourceLen` bytes of gzip data from `source` to `dest`.
 *
 * The variable `destLen` points to must contain the size of `dest` on entry,
 * and will be set to the size of the decompressed data on success.
 *
 * Reads at most `sourceLen` bytes from `source`.
 * Writes at most `*destLen` bytes to `dest`.
 *
 * @param dest pointer to where to place decompressed data
 * @param destLen pointer to variable containing size of `dest`
 * @param source pointer to compressed data
 * @param sourceLen size of compressed data
 * @return `TINF_OK` on success, error code on error
 */
int tinf_gzip_uncompress(void *dest, unsigned int *destLen,
			 const void *source, unsigned int sourceLen);

#endif /* TINF_H_INCLUDED */
