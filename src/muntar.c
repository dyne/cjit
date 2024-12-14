#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <muntar.h>

// from io.c
extern void _err(const char *fmt, ...);


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
static uint64_t decodeTarOctal(
	const char* data,
	size_t size )
{
    uint8_t* currentPtr = (uint8_t*) data + size;
    uint64_t sum = 0;
    uint64_t currentMultiplier = 1;

	/* find then last NUL or space */
    uint8_t* checkPtr = currentPtr;
    for (; checkPtr >= (uint8_t*) data; checkPtr--)
	{
        if (*checkPtr == 0 || *checkPtr == ' ') currentPtr = checkPtr - 1;
    }
	/* decode the octal number */
    for (; currentPtr >= (uint8_t*) data; currentPtr--)
	{
        sum += (uint64_t) ((*currentPtr) - 48) * currentMultiplier;
        currentMultiplier *= 8;
    }
    return sum;
}

static int raw_to_header(mtar_header_t *h, const mtar_raw_header_t *rh)
{
	unsigned chksum1, chksum2;

	/* if the checksum starts with a null byte we assume the record is NULL */
	if (*rh->checksum == '\0') return MTAR_ENULLRECORD;

	/* validate header fields */
	chksum1 = checksum(rh);
	chksum2 = (uint32_t) decodeTarOctal(rh->checksum, sizeof(rh->checksum));
	if (chksum1 != chksum2) return MTAR_EBADCHKSUM;

	if (strncmp(rh->magic, TMAGIC, sizeof(rh->magic)) != 0) return MTAR_ENULLRECORD;

	h->mode     = (uint32_t) decodeTarOctal(rh->mode, sizeof(rh->mode));
	h->uid      = (uint32_t) decodeTarOctal(rh->uid, sizeof(rh->uid));
	h->gid      = (uint32_t) decodeTarOctal(rh->gid, sizeof(rh->gid));
	h->size     = decodeTarOctal(rh->size, sizeof(rh->size));
	h->mtime    = (uint32_t) decodeTarOctal(rh->mtime, sizeof(rh->mtime));
	h->devmajor = (uint32_t) decodeTarOctal(rh->devmajor, sizeof(rh->devmajor));
	h->devminor = (uint32_t) decodeTarOctal(rh->devminor, sizeof(rh->devminor));

	h->type = (uint32_t) rh->type;
	strncpy(h->name, rh->name, sizeof(rh->name));
	h->name[ sizeof(h->name) - 1 ] = 0;
	strncpy(h->linkname, rh->linkname, sizeof(rh->linkname));
	h->linkname[ sizeof(h->linkname) - 1 ] = 0;
	strncpy(h->path, rh->path, sizeof(rh->path));
	h->path[ sizeof(h->path) - 1 ] = 0;

	return MTAR_ESUCCESS;
}

static int mtar_read(mtar_t *tar, uint8_t *dest, size_t size) {
	if(tar->position + size > tar->max) return(MTAR_EREADFAIL);
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

///////////////////
// PUBLIC FUNCTIONS
///////////////////

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
		_err("Error reading header of tar buffer: %s",tar->name);
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
