// https://lists.nongnu.org/archive/html/tinycc-devel/2024-12/msg00019.html
// x86_64/i386 void gfunc_call(int nb_args), when push struct args, need fetch cpu flag before generating any code

#include <stdio.h>

struct string {
	char *str;
	int len;
};

void dummy(struct string fpath, int dump_arg) {
}

int main() {
	int a = 1;
	struct string x;
	x.str = "gg.v";
	x.len = 4;
	dummy(x, a == 0);
	printf("done\n");
	return 0;
}

