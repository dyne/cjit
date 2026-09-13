int printf(const char*, ...);

#define myassert(x) \
    printf("%s:%d: %s : \"%s\"\n", __FILE__,__LINE__,(x)?"ok":"error",#x)

enum{ in = 0};

int main_1(){
    {
        myassert(!in);
        if(sizeof(enum{in=1})) myassert(in);
        myassert(!in); //OOPS
    }
    {
        myassert(!in);
        switch(sizeof(enum{in=1})) { default: myassert(in); }
        myassert(!in); //OOPS
    }
    {
        myassert(!in);
        while(sizeof(enum{in=1})) { myassert(in); break; }
        myassert(!in); //OOPS
    }
    {
        myassert(!in);
        do{ myassert(!in);}while(0*sizeof(enum{in=1}));
        myassert(!in); //OOPS
    }

    {
        myassert(!in);
        for(sizeof(enum{in=1});;){ myassert(in); break; }
        myassert(!in); //OK
    }
    {
        myassert(!in);
        for(;;sizeof(enum{in=1})){ myassert(in); break; }
        myassert(!in); //OK
    }
    {
        myassert(!in);
        for(;sizeof(enum{in=1});){ myassert(in); break; }
        myassert(!in); //OK
    }
    return 0;
}

/* --------------------------------------------- */
int main_2()
{
  char c = 'a';
  void func1(char c); /* param 'c' must not shadow local 'c' */
  func1(c);
  return 0;
}

void func1(char c)
{
    myassert(c == 'a');
}

struct st { int a; };

/* --------------------------------------------- */
int main_3()
{
  struct st func(void);
  struct st st = func(); /* not an 'incompatible redefinition' */
  myassert(st.a == 10);
  return 0;
}

struct st func(void)
{
  struct st st = { 10 };
  return st;
}

/* --------------------------------------------- */
/* func* 'md' must not be shadowed by param 'md' */
static void f4(char *(*md)(char *md))
{
   (*md)("test");
}
static char *a4(char *a)
{
    int strcmp();
    myassert(!strcmp(a, "test"));
    return a;
}

int main_4()
{
    f4(a4);
    return 0;
}

/* --------------------------------------------- */
int a5[3], b5[];
int f5(void);
int main_5()
{
    extern int a5[3], b5[3];
    a5[2]=10, b5[2]=4;
    myassert(f5() == 10 + 4);
    return 0;
}
int f5(void)
{
    return a5[2]+b5[2];
}
int b5[3];

/* --------------------------------------------- */
static int f6(int);
int i6 = 11;

int main_6()
{
    {
        int i6 = 33, f6 = 44;
        myassert(i6 == 33 && f6 == 44);
        {
            int f6(int);
            extern int i6;
            myassert(i6 == 11 && f6(22) == 22);
        }
        myassert(i6 == 33 && f6 == 44);
    }
    myassert(i6 == 11 && f6(22) == 22);
    return 0;
}

int f6(int x)
{
    return x;
}

/* --------------------------------------------- */

#if defined __TINYC__ \
    ? !defined __leading_underscore \
    : !(defined __APPLE__ || defined _WIN32)
# define _
#else
# define _ "_"
#endif

struct xx7 { int a, b; };

void f7()
{
    struct xx7 { int c; } x;
    {
        extern struct xx7 { int a, b; } x __asm__(_"z7");
        x.a = 12;
        struct xx7 y = { 0,0 };
    }
    struct xx7 y = { 90 };
    x.c = 78;
    printf("xx7 (1) : %d %d\n", x.c, y.c);
}

int main_7()
{
    f7();
    extern struct xx7 y __asm__(_"z7");
    printf("xx7 (2) : %d %d\n", y.a, y.b);
    return 0;
}


struct xx7 z7 = { 0, 34 };

/* --------------------------------------------- */
int main()
{
    main_1();
    main_2();
    main_3();
    main_4();
    main_5();
    main_6();
    main_7();
    return 0;
}
