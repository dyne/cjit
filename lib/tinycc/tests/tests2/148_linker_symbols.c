int printf(const char *, ...);

#define assert(x) ((x) \
    ? (printf(" 1 (%s)\n",#x),1) \
    : (printf("%s:%d: error: (%s)\n",__FILE__,__LINE__,#x),0))

#define O 100
#define SO 200

/* ------------------------------------------------------ */
#if TEST-O == 1
extern char etext[], _etext[];
extern char edata[], _edata[];
extern char end[], _end[];
int check_linker_symbols(void)
{
    int errors = 0;
    errors += !assert(etext == _etext);
    errors += !assert(edata == _edata);
    errors += !assert(end == _end);
    return errors;
}

#elif TEST-MAIN == 1
int check_linker_symbols(void);
int main(void)
{
    printf("linker symbols test\n");
    assert(check_linker_symbols() == 0);
}

/* ------------------------------------------------------ */
#elif TEST-SO == 2
extern char etext[], edata[], end[];
void get_dso_end(char **p)
{
    p[0] = etext;
    p[1] = edata;
    p[2] = end;
}

#elif TEST-MAIN == 2
extern char _etext[], _edata[], _end[];
extern void get_dso_end(char **);
static int fill_data = 123;
static int fill_bss;

int main(void)
{
    char *p[3];
    printf("DSO linker symbols test\n");
    get_dso_end(p);
    assert(p[0] == _etext) || printf("-- %p %p\n", p[0], _etext);
    assert(p[1] == _edata) || printf("-- %p %p\n", p[1], _edata);
    assert(p[2] == _end)   || printf("-- %p %p\n", p[2], _end);
}

/* ------------------------------------------------------ */
#elif TEST-MAIN == 3

int etext(void) { return 1; }
int edata(void) { return 2; }
int end(void) { return 3; }
int main(void)
{
    printf("user definition test\n");
    assert(etext() + edata() + end() == 6);
}

/* ------------------------------------------------------ */
#elif TEST-SO == 4
unsigned char copy_reloc_data[257];

#elif TEST-MAIN == 4
extern unsigned char copy_reloc_data[257];
extern char end[], _end[];

int main(void)
{
    unsigned long data_end;
    printf("end copy relocation test\n");
    copy_reloc_data[256] = 42;
    data_end = (unsigned long)(copy_reloc_data + sizeof copy_reloc_data);
    assert(end == _end);
    assert((unsigned long)end >= data_end);
    assert(copy_reloc_data[256] == 42);
}

/* ------------------------------------------------------ */
#elif TEST-O == 5
void __attribute__((section(".text.linker_test"), noinline, used))
linker_test_text(void)
{
}
__attribute__((section(".bss.linker_test"), used))
unsigned char linker_test_bss[257];

__attribute__((section(".data.linker_test"), used))
unsigned char linker_test_data[23] = { 1 };

#elif TEST-MAIN == 5
extern void linker_test_text(void);
extern unsigned char linker_test_data[23];
extern unsigned char linker_test_bss[257];
extern char etext[], _etext[];
extern char edata[], _edata[];
extern char end[], _end[];

int main(void)
{
    char* text_addr = (void*)linker_test_text;
    char* data_end = linker_test_data + sizeof linker_test_data;
    char* bss_end = linker_test_bss + sizeof linker_test_bss;

    printf("linker boundary test\n");
    linker_test_data[22] = 42;
    linker_test_bss[256] = 42;
    assert(linker_test_bss > linker_test_data);
    assert(etext == _etext);
    assert(edata == _edata);
    assert(end == _end);
    assert(etext >= text_addr);
    assert(edata >= data_end);
    assert(end >= bss_end);
    assert(linker_test_data[22] == 42);
    assert(linker_test_bss[256] == 42);
}

/* ------------------------------------------------------ */
#elif TEST-SO == 6
int end;
int eight = 8;
int nine;
int dso_add() { return end + eight + nine; }

#elif TEST-MAIN == 6
int end = 7;
int eight;
int nine = 9;
int dso_add();
int main()
{
    int sum;
    printf("DSO initialized data test\n");
    sum = dso_add();
    assert(end == 7);
    assert(eight == 8);
    assert(nine == 9);
    assert(sum == 24) || printf("-- %d + %d + %d = %d\n", end, eight, nine, sum);
}

/* ------------------------------------------------------ */
#endif
