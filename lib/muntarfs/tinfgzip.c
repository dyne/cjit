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

#include "tinf.h"

typedef enum {
	FTEXT    = 1,
	FHCRC    = 2,
	FEXTRA   = 4,
	FNAME    = 8,
	FCOMMENT = 16
} tinf_gzip_flag;

static unsigned int read_le16(const unsigned char *p)
{
	return ((unsigned int) p[0])
	     | ((unsigned int) p[1] << 8);
}

static unsigned int read_le32(const unsigned char *p)
{
	return ((unsigned int) p[0])
	     | ((unsigned int) p[1] << 8)
	     | ((unsigned int) p[2] << 16)
	     | ((unsigned int) p[3] << 24);
}

static const unsigned int tinf_crc32tab[16] = {
	0x00000000, 0x1DB71064, 0x3B6E20C8, 0x26D930AC, 0x76DC4190,
	0x6B6B51F4, 0x4DB26158, 0x5005713C, 0xEDB88320, 0xF00F9344,
	0xD6D6A3E8, 0xCB61B38C, 0x9B64C2B0, 0x86D3D2D4, 0xA00AE278,
	0xBDBDF21C
};


/**
 * Compute CRC32 checksum of `length` bytes starting at `data`.
 *
 * @param data pointer to data
 * @param length size of data
 * @return CRC32 checksum
 */
static unsigned int tinf_crc32(const void *data, unsigned int length)
{
	const unsigned char *buf = (const unsigned char *) data;
	unsigned int crc = 0xFFFFFFFF;
	unsigned int i;

	if (length == 0) {
		return 0;
	}

	for (i = 0; i < length; ++i) {
		crc ^= buf[i];
		crc = tinf_crc32tab[crc & 0x0F] ^ (crc >> 4);
		crc = tinf_crc32tab[crc & 0x0F] ^ (crc >> 4);
	}

	return crc ^ 0xFFFFFFFF;
}

// from tinflate.c
extern int tinf_uncompress(void *dest, unsigned int *destLen,
                           const void *source, unsigned int sourceLen);

int tinf_gzip_uncompress(void *dest, unsigned int *destLen,
                         const void *source, unsigned int sourceLen)
{
	const unsigned char *src = (const unsigned char *) source;
	unsigned char *dst = (unsigned char *) dest;
	const unsigned char *start;
	unsigned int dlen, crc32;
	int res;
	unsigned char flg;

	/* -- Check header -- */

	/* Check room for at least 10 byte header and 8 byte trailer */
	if (sourceLen < 18) {
		return TINF_DATA_ERROR;
	}

	/* Check id bytes */
	if (src[0] != 0x1F || src[1] != 0x8B) {
		return TINF_DATA_ERROR;
	}

	/* Check method is deflate */
	if (src[2] != 8) {
		return TINF_DATA_ERROR;
	}

	/* Get flag byte */
	flg = src[3];

	/* Check that reserved bits are zero */
	if (flg & 0xE0) {
		return TINF_DATA_ERROR;
	}

	/* -- Find start of compressed data -- */

	/* Skip base header of 10 bytes */
	start = src + 10;

	/* Skip extra data if present */
	if (flg & FEXTRA) {
		unsigned int xlen = read_le16(start);

		if (xlen > sourceLen - 12) {
			return TINF_DATA_ERROR;
		}

		start += xlen + 2;
	}

	/* Skip file name if present */
	if (flg & FNAME) {
		do {
			if (start - src >= sourceLen) {
				return TINF_DATA_ERROR;
			}
		} while (*start++);
	}

	/* Skip file comment if present */
	if (flg & FCOMMENT) {
		do {
			if (start - src >= sourceLen) {
				return TINF_DATA_ERROR;
			}
		} while (*start++);
	}

	/* Check header crc if present */
	if (flg & FHCRC) {
		unsigned int hcrc;

		if (start - src > sourceLen - 2) {
			return TINF_DATA_ERROR;
		}

		hcrc = read_le16(start);

		if (hcrc != (tinf_crc32(src, start - src) & 0x0000FFFF)) {
			return TINF_DATA_ERROR;
		}

		start += 2;
	}

	/* -- Get decompressed length -- */

	dlen = read_le32(&src[sourceLen - 4]);

	if (dlen > *destLen) {
		return TINF_BUF_ERROR;
	}

	/* -- Get CRC32 checksum of original data -- */

	crc32 = read_le32(&src[sourceLen - 8]);

	/* -- Decompress data -- */

	if ((src + sourceLen) - start < 8) {
		return TINF_DATA_ERROR;
	}

	res = tinf_uncompress(dst, destLen, start,
	                      (src + sourceLen) - start - 8);

	if (res != TINF_OK) {
		return TINF_DATA_ERROR;
	}

	if (*destLen != dlen) {
		return TINF_DATA_ERROR;
	}

	/* -- Check CRC32 checksum -- */

	if (crc32 != tinf_crc32(dst, dlen)) {
		return TINF_DATA_ERROR;
	}

	return TINF_OK;
}
