/* muntar
 *
 * Copyright (C) 2024 Dyne.org foundation
 *                  maintained by Jaromil
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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <muntar.h>

static int mtar_read_header(mtar_t *tar, mtar_header_t *h);

typedef struct
{
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char checksum[8];
	char type;
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char path[155];
	char padding[12];
} mtar_raw_header_t;

static unsigned checksum(const mtar_raw_header_t* rh)
{
	unsigned i;
	unsigned char *p = (unsigned char*) rh;
	unsigned res = 256;
	for (i = 0; i < offsetof(mtar_raw_header_t, checksum); i++)
		res += p[i];
	for (i = offsetof(mtar_raw_header_t, type); i < sizeof(*rh); i++)
		res += p[i];
	return res;
}

/**
 * Decode an octal field.
 */
static int decodeTarOctal(const char *data, size_t size, uint64_t *value)
{
	uint64_t sum = 0;
	size_t i = 0;
	while (i < size && (data[i] == ' ' || data[i] == '\0')) i++;
	for (; i < size && data[i] != ' ' && data[i] != '\0'; i++) {
		unsigned char digit = (unsigned char)data[i];
		if (digit < '0' || digit > '7' || sum > (UINT64_MAX - (digit - '0')) / 8)
			return MTAR_EINVALIDMODE;
		sum = sum * 8 + (digit - '0');
	}
	while (i < size) {
		if (data[i] != ' ' && data[i] != '\0') return MTAR_EINVALIDMODE;
		i++;
	}
	*value = sum;
	return MTAR_ESUCCESS;
}

static int raw_to_header(mtar_header_t *h, const mtar_raw_header_t *rh)
{
	unsigned chksum1;
	uint64_t chksum2, value;

	/* if the checksum starts with a null byte we assume the record is NULL */
	if (*rh->checksum == '\0') return MTAR_ENULLRECORD;

	/* validate header fields */
	chksum1 = checksum(rh);
	if (decodeTarOctal(rh->checksum, sizeof(rh->checksum), &chksum2) != MTAR_ESUCCESS)
		return MTAR_EINVALIDMODE;
	if (chksum1 != chksum2) return MTAR_EBADCHKSUM;

	if (strncmp(rh->magic, TMAGIC, sizeof(rh->magic)) != 0) return MTAR_ENULLRECORD;

	if (decodeTarOctal(rh->mode, sizeof(rh->mode), &value) != MTAR_ESUCCESS) return MTAR_EINVALIDMODE;
	h->mode = (uint32_t)value;
	if (decodeTarOctal(rh->uid, sizeof(rh->uid), &value) != MTAR_ESUCCESS) return MTAR_EINVALIDMODE;
	h->uid = (uint32_t)value;
	if (decodeTarOctal(rh->gid, sizeof(rh->gid), &value) != MTAR_ESUCCESS) return MTAR_EINVALIDMODE;
	h->gid = (uint32_t)value;
	if (decodeTarOctal(rh->size, sizeof(rh->size), &h->size) != MTAR_ESUCCESS) return MTAR_EINVALIDMODE;
	if (decodeTarOctal(rh->mtime, sizeof(rh->mtime), &value) != MTAR_ESUCCESS) return MTAR_EINVALIDMODE;
	h->mtime = (uint32_t)value;
	if (decodeTarOctal(rh->devmajor, sizeof(rh->devmajor), &value) != MTAR_ESUCCESS) return MTAR_EINVALIDMODE;
	h->devmajor = (uint32_t)value;
	if (decodeTarOctal(rh->devminor, sizeof(rh->devminor), &value) != MTAR_ESUCCESS) return MTAR_EINVALIDMODE;
	h->devminor = (uint32_t)value;

	h->type = (uint32_t) rh->type;
	strncpy(h->name, rh->name, sizeof(h->name));
	h->name[ sizeof(h->name) - 1 ] = 0;
	strncpy(h->linkname, rh->linkname, sizeof(h->linkname));
	h->linkname[ sizeof(h->linkname) - 1 ] = 0;
	strncpy(h->path, rh->path, sizeof(h->path));
	h->path[ sizeof(h->path) - 1 ] = 0;

	return MTAR_ESUCCESS;
}

static int mtar_read(mtar_t *tar, uint8_t *dest, size_t size) {
	if(size > tar->max - tar->position) return(MTAR_EREADFAIL);
	memcpy(dest, &tar->buffer[tar->position],(size_t)size);
	tar->position += size;
	return(MTAR_ESUCCESS);
}

static int mtar_seek(mtar_t *tar, size_t pos) {
	if(pos != UINT64_MAX) {
		if(pos > tar->max) return(MTAR_ESEEKFAIL);
		tar->position = pos;
	} else {
		tar->position = tar->max;
	}
	return(MTAR_ESUCCESS);
}

static int mtar_rewind(mtar_t *tar) {
	int err;
	tar->iterator.offset = UINT64_MAX;
	tar->iterator.cursor = UINT64_MAX;
	err = mtar_seek(tar, 0);
	if (err != MTAR_ESUCCESS) return err;
	err = mtar_read_header(tar, &(tar->iterator.header));
	if (err != MTAR_ESUCCESS) return err;
	tar->iterator.offset = 0;
	tar->iterator.cursor = 512;
	return(MTAR_ESUCCESS);
}
///////////////////
// PUBLIC FUNCTIONS
///////////////////


#if defined(_WIN32) || defined(WINDOWS)
#include <windows.h>
#define makedir(path) CreateDirectory(path, NULL)
#else
#include <sys/stat.h>
#define makedir(path) mkdir(path,0755)
#endif

static int make_directory(const char *path)
{
	if (makedir(path)) return MTAR_ESUCCESS;
#if defined(_WIN32) || defined(WINDOWS)
	{
		DWORD attributes = GetFileAttributes(path);
		return attributes != INVALID_FILE_ATTRIBUTES &&
			!(attributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
			(attributes & FILE_ATTRIBUTE_DIRECTORY)
			? MTAR_ESUCCESS : MTAR_EWRITEFAIL;
	}
#else
	{
		struct stat status;
		return lstat(path, &status) == 0 && S_ISDIR(status.st_mode)
			? MTAR_ESUCCESS : MTAR_EWRITEFAIL;
	}
#endif
}

/* Refuse to traverse an existing symbolic link or Windows reparse point. */
static int path_contains_link(const char *path, size_t trusted_prefix_length,
			      int include_leaf)
{
	char checked[1024];
	size_t length = strlen(path);
	size_t i;

	if (length >= sizeof(checked)) return 1;
	strcpy(checked, path);
	for (i = trusted_prefix_length; i <= length; i++) {
		int boundary = checked[i] == '/' || checked[i] == '\\' || checked[i] == '\0';
		char saved;
		if (!boundary || (!include_leaf && i == length)) continue;
		if (i == 2 && checked[1] == ':') continue;
		saved = checked[i];
		checked[i] = '\0';
#if defined(_WIN32) || defined(WINDOWS)
		{
			DWORD attributes = GetFileAttributes(checked);
			if (attributes != INVALID_FILE_ATTRIBUTES &&
			    (attributes & FILE_ATTRIBUTE_REPARSE_POINT)) return 1;
		}
#else
		{
			struct stat status;
			if (lstat(checked, &status) == 0 && S_ISLNK(status.st_mode)) return 1;
		}
#endif
		checked[i] = saved;
	}
	return 0;
}

/* Archive entry names are untrusted.  Keep every output below destination. */
static int append_safe_component(char *out, size_t capacity, size_t *length,
				 const char *component)
{
	const char *p = component;
	if (!component || component[0] == '/' || component[0] == '\\' || strchr(component, '\\'))
		return MTAR_EINVALIDMODE;
	while (*p) {
		const char *end = strchr(p, '/');
		size_t part = end ? (size_t)(end - p) : strlen(p);
		if (part == 0 || (part == 1 && p[0] == '.') ||
		    (part == 2 && p[0] == '.' && p[1] == '.')) return MTAR_EINVALIDMODE;
		if (*length + 1 + part >= capacity) return MTAR_EOPENFAIL;
		out[(*length)++] = '/';
		memcpy(out + *length, p, part);
		*length += part;
		out[*length] = '\0';
		if (!end) break;
		p = end + 1;
	}
	return MTAR_ESUCCESS;
}

static int entry_path(char *out, size_t capacity, const char *destination,
			  const mtar_header_t *header)
{
	size_t length;
	if (!destination || !*destination || strlen(destination) >= capacity) return MTAR_EOPENFAIL;
	strcpy(out, destination);
	length = strlen(out);
	if (header->path[0] && append_safe_component(out, capacity, &length, header->path) != MTAR_ESUCCESS)
		return MTAR_EINVALIDMODE;
	return append_safe_component(out, capacity, &length, header->name);
}

// used by extract_assets(char *tmpdir)
int muntar_to_path(const char *path, const uint8_t *buf,
		  const unsigned int len) {
	int res;
	mtar_t tar;
	char tpath[1024];
	const size_t pathlen = path ? strlen(path) : 0;
	if(!path || !buf || !len || pathlen >= sizeof(tpath)) return(MTAR_EFAILURE);
	const mtar_header_t *header = NULL;
	strcpy(tpath, path);
	res = mtar_load(&tar, path, buf, len);
	if(res != MTAR_ESUCCESS) return(MTAR_EOPENFAIL);
	// first create extract dir if doesn't exist
	if (make_directory(tpath) != MTAR_ESUCCESS)
		return MTAR_EWRITEFAIL;
	while(!mtar_eof(&tar)) {
		// then create every other subdir
		if (mtar_header(&tar, &header) != MTAR_ESUCCESS) return MTAR_EREADFAIL;
		switch(header->type) {
		case MTAR_TDIR:
			if (entry_path(tpath, sizeof(tpath), path, header) != MTAR_ESUCCESS) return MTAR_EINVALIDMODE;
			if (path_contains_link(tpath, pathlen, 1) || make_directory(tpath) != MTAR_ESUCCESS)
				return MTAR_EWRITEFAIL;
			break;
		}
		res = mtar_next(&tar);
		if (res != MTAR_ESUCCESS && res != MTAR_ENULLRECORD) return res;
	}
	mtar_rewind(&tar);
	while(!mtar_eof(&tar)) {
		// and at last create the files
		if (mtar_header(&tar, &header) != MTAR_ESUCCESS) return MTAR_EREADFAIL;
		switch(header->type) {
		case MTAR_TREG:
			if (entry_path(tpath, sizeof(tpath), path, header) != MTAR_ESUCCESS) return MTAR_EINVALIDMODE;
			if (path_contains_link(tpath, pathlen, 0)) return MTAR_EWRITEFAIL;
			/* Do not silently replace a file supplied by an earlier entry. */
			{
				FILE *existing = fopen(tpath, "rb");
				if (existing) {
					fclose(existing);
					return MTAR_EWRITEFAIL;
				}
			#if !defined(_WIN32) && !defined(WINDOWS)
				{
					struct stat status;
					if (lstat(tpath, &status) == 0 || errno != ENOENT)
						return MTAR_EWRITEFAIL;
				}
			#endif
			}
			FILE *fp = fopen(tpath,"wb");
			if(!fp) {
				fprintf(stderr,
					"Error open file for write: %s\n",
					tpath);
				perror("Reason: ");
				return(MTAR_EWRITEFAIL);
			}
			{
				size_t written = fwrite(&tar.buffer[tar.iterator.cursor], 1, header->size, fp);
				int close_result = fclose(fp);
				if (written != header->size || close_result != 0) return MTAR_EWRITEFAIL;
			}
			break;
		}
		res = mtar_next(&tar);
		if (res != MTAR_ESUCCESS && res != MTAR_ENULLRECORD) return res;
	}
	return(MTAR_ESUCCESS);
}

#if !defined(NOGUNZIP)
// gunzip and untar all in one
#include <tinf.h>
#define DECOMPRESSED_SIZE_RATIO 10 // raise this on errors
int muntargz_to_path(const char *path, const uint8_t *buf,
		    const unsigned int len) {
	unsigned int attempts = 0;
	unsigned int destlen;
	uint8_t *dest = NULL;
	if(!buf) {
		fprintf(stderr,"%s: called with NULL buffer\n",
			__func__);
		return(-1);
	}
	if(!len) {
		fprintf(stderr,"%s: called with zero length\n",
			__func__);
		return(-1);
	}
	if (len > UINT_MAX / DECOMPRESSED_SIZE_RATIO) return TINF_BUF_ERROR;
	destlen = len * DECOMPRESSED_SIZE_RATIO;
	for(attempts = 0; attempts < 8; attempts++) {
		unsigned int outlen = destlen;
		int res;
		dest = malloc(destlen);
		if(!dest) {
			fprintf(stderr,"%s: out of memory\n", __func__);
			return(-1);
		}
		res = tinf_gzip_uncompress(dest, &outlen, buf, len);
		if(res == TINF_OK) {
			res = muntar_to_path(path, dest, outlen);
			free(dest);
			return(res);
		}
		free(dest);
		dest = NULL;
		if(res != TINF_BUF_ERROR) {
			fprintf(stderr,"Error in gunzip decompression (untargz_to_path)\n");
			return(res);
		}
		if (destlen > UINT_MAX / 2) return TINF_BUF_ERROR;
		destlen *= 2;
	}
	fprintf(stderr,"Error in gunzip decompression (untargz_to_path)\n");
	return(TINF_BUF_ERROR);
}
#endif

int mtar_load(mtar_t *tar, const char *name,
	      const uint8_t *buf, size_t size) {
	int err = 0;
	memset(tar, 0, sizeof(mtar_t));
	tar->name = name;
	tar->buffer = buf;
	tar->position = 0;
	tar->max = size;
	tar->iterator.offset = UINT64_MAX;
	tar->iterator.cursor = UINT64_MAX;
	err = mtar_seek(tar, 0);
	if (err != MTAR_ESUCCESS) return err;
	err = mtar_read_header(tar, &(tar->iterator.header));
	if (err != MTAR_ESUCCESS) return err;
	tar->iterator.offset = 0;
	tar->iterator.cursor = 512;
	return MTAR_ESUCCESS;
}

int mtar_eof(mtar_t *tar)
{
	if (tar->iterator.offset == UINT64_MAX) return 1;
	return 0;
}

int mtar_entry_eof(mtar_t *tar)
{
	size_t end = 0;
	end = tar->iterator.offset + 512 + tar->iterator.header.size;
	if (tar->iterator.cursor >= end) return 1;
	return 0;
}

int mtar_next(mtar_t *tar)
{
	int err = 0;
	size_t position = 0;

	if (tar->iterator.offset == UINT64_MAX) return MTAR_ENULLRECORD;

	if (tar->iterator.header.size > tar->max - tar->iterator.offset - 512)
		return MTAR_EREADFAIL;
	position = tar->iterator.offset + 512 + tar->iterator.header.size;
	position = (size_t) ( (uint64_t)(position + 511) & (uint64_t) (~0x01FF) );
	err = mtar_seek(tar, position);
	if (err != MTAR_ESUCCESS) goto ESCAPE;
	err = mtar_read_header(tar, &tar->iterator.header);
	if (err != MTAR_ESUCCESS) goto ESCAPE;
	tar->iterator.offset = position;
	tar->iterator.cursor = position + 512;
	return MTAR_ESUCCESS;
ESCAPE:
	tar->iterator.offset = UINT64_MAX;
	tar->iterator.cursor = UINT64_MAX;
	return err;
}

int mtar_header(mtar_t *tar, const mtar_header_t **header)
{
	if (tar->iterator.offset == UINT64_MAX) return MTAR_ENULLRECORD;
	*header = &tar->iterator.header;
	return MTAR_ESUCCESS;
}

static int mtar_read_header(mtar_t *tar, mtar_header_t *h)
{
	int err;
	mtar_raw_header_t rh;

	err = mtar_read(tar, (uint8_t*) &rh, sizeof(rh));
	if (err != MTAR_ESUCCESS) {
		fprintf(stderr,"Error reading header of tar buffer: %s\n",tar->name);
		return -1;
	}
	return raw_to_header(h, &rh);
}

int mtar_entry_read(mtar_t *tar, void *ptr, int size)
{
	int err = 0;
	size_t end = 0;

	if (size < 0) return MTAR_EREADFAIL;

	end = tar->iterator.offset + 512 + tar->iterator.header.size;
	if (tar->iterator.cursor >= end) return MTAR_EREADFAIL;

	if ((size_t)size > end - tar->iterator.cursor)
		size = (int) (end - tar->iterator.cursor);

	err = mtar_read(tar, ptr, (size_t) size);
	if (err < 0) return err;
	tar->iterator.cursor += (size_t) size;
	return size;
}
