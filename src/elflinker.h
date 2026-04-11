#ifndef __ELFLINKER_H__
#define __ELFLINKER_H__

typedef struct StringList StringList;

struct LDState {
	int cc;
	int fd;
	int new_undef_sym;
	int static_link;
	StringList *libs;
	StringList *libpaths;
};
typedef struct LDState LDState;

bool read_ldsoconf(StringList *dest, char *path);
bool read_ldsoconf_dir(StringList *dest, const char *directory);
int posix_resolve_libs(CJITState *cjit);

#endif
