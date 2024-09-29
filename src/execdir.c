/* CJIT https://dyne.org/cjit
 *
 * Copyright (C) 2024 Dyne.org foundation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <microtar.h>

// from io.c
extern void _err(const char *fmt, ...);

// embedded_headers.c
extern char *tinycc_headers;
extern unsigned int tinycc_headers_len;

static int embed_read(mtar_t *tar, void *data, unsigned size) {
  memcpy(data, ((char*)tar->stream)+tar->pos, size);
  return(MTAR_ESUCCESS);
}

static int embed_seek(mtar_t *tar, unsigned offset) {
  if(offset >= tar->max) return MTAR_ESEEKFAIL;
  tar->pos = offset;
  return MTAR_ESUCCESS;
}

static int embed_close(mtar_t *tar) {
  (void)tar;
  return MTAR_ESUCCESS;
}

bool tar_x_embedded_buffer(unsigned char src[], unsigned int len) {
  mtar_t tar;
  mtar_header_t h;
  tar.stream = src;
  tar.pos = 0;
  tar.max = len;
  tar.read = embed_read;
  tar.seek = embed_seek;
  tar.close = embed_close;
  tar.write = NULL;
  /* Print all file names and sizes */
  while ( (mtar_read_header(&tar, &h)) != MTAR_ENULLRECORD ) {
    _err("microtar_x: %s (%d bytes)\n", h.name, h.size);
    mtar_next(&tar);
  }
  return true;
}
