#ifndef __LDSCRIPT_H__
#define __LDSCRIPT_H__

struct LDState {
	int cc;
	int fd;
	int new_undef_sym;
	int static_link;
};
typedef struct LDState LDState;

int cjit_load_ldscript(LDState *s1, char *path);

#endif
