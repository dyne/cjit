#ifndef __ELFLINKER_H__
#define __ELFLINKER_H__

#include <array.h>

struct LDState {
	int cc;
	int fd;
	int new_undef_sym;
	int static_link;
	xarray_t *libs;
};
typedef struct LDState LDState;

bool read_ldsoconf(xarray_t *dest, char *path);
bool read_ldsoconf_dir(xarray_t *dest, const char *directory);
int resolve_libs(CJITState *cjit);

#endif
