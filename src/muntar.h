/* muntar, part of CJIT
 *
 * Copyright (C) 2024 Dyne.org foundation
 *              and maintained by Jaromil
 *
 * based on microtar (C) 2016 rxi
 *   and on minitar  (C) 2019 Bruno Costa
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

#ifndef MUNTAR_H
#define MUNTAR_H

#include <stdint.h>

#define TMAGIC   "ustar"
#define TVERSION "00"

// used by extract_assets(char *tmpdir)
int muntar_to_path(const char *path,
		  const uint8_t *buf, const unsigned int len);
#if !defined(NOGUNZIP)
int muntargz_to_path(const char *path,
		    const uint8_t *buf, const unsigned int len);
#endif

enum {
	MTAR_ESUCCESS     =  0,
	MTAR_EFAILURE     = -1,
	MTAR_EOPENFAIL    = -2,
	MTAR_EREADFAIL    = -3,
	MTAR_EWRITEFAIL   = -4,
	MTAR_ESEEKFAIL    = -5,
	MTAR_EBADCHKSUM   = -6,
	MTAR_ENULLRECORD  = -7,
	MTAR_ENOTFOUND    = -8,
    MTAR_EINVALIDMODE = -9
};

enum {
	MTAR_TREG   = '0',
	MTAR_TLNK   = '1',
	MTAR_TSYM   = '2',
	MTAR_TCHR   = '3',
	MTAR_TBLK   = '4',
	MTAR_TDIR   = '5',
	MTAR_TFIFO  = '6'
};

typedef enum {
    MTAR_TSUID   = 04000, /* set user ID on execution   */
    MTAR_TSGID   = 02000, /* set group ID on execution  */
    MTAR_TSVTX   = 01000, /* reserved                   */
    MTAR_TUREAD  = 00400, /* read by owner              */
    MTAR_TUWRITE = 00200, /* write by owner             */
    MTAR_TUEXEC  = 00100, /* execute or search by owner */
    MTAR_TGREAD  = 00040, /* read by group              */
    MTAR_TGWRITE = 00020, /* write by group             */
    MTAR_TGEXEC  = 00010, /* execute or search by group */
    MTAR_TOREAD  = 00004, /* read by others             */
    MTAR_TOWRITE = 00002, /* write by others            */
    MTAR_TOEXEC  = 00001  /* execute or search by other */
} mtar_perm_t;

typedef enum {
    MTAR_READ = 0,
    MTAR_WRITE = 1
} mtar_mode_t;

typedef uint64_t mtar_size_t;

typedef struct {
	uint32_t mode;
	uint32_t uid;
	uint32_t gid;
	mtar_size_t size;
	uint32_t mtime;
	uint32_t type;
	uint32_t devminor;
	uint32_t devmajor;
	char name[100 + 1];
	char path[155 + 1];
	char linkname[100 + 1];
} mtar_header_t;

struct mtar_iterator_t
{
    mtar_header_t header;
    mtar_size_t offset; /* header position */
    mtar_size_t cursor; /* content cursor  */
};

typedef struct mtar_t {
	const uint8_t *buffer;
	const char *name;
	size_t position;
	size_t max;
	struct mtar_iterator_t iterator;
} mtar_t;

int mtar_load(mtar_t *tar, const char *name,
	      const uint8_t *buf, size_t size);
int mtar_next(mtar_t *tar);
int mtar_header(mtar_t *tar, const mtar_header_t **header);
int mtar_eof(mtar_t *tar);
int mtar_entry_eof(mtar_t *tar);
int mtar_entry_read(mtar_t *tar, void *ptr, int size);

#endif
