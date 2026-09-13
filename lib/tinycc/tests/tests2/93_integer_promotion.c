/* integer promotion */

int printf(const char*, ...);
#define promote(s) printf(" %ssigned : %s\n", (s) - 100 < 0 ? "  " : "un", #s);

int main (void)
{
    struct {
        unsigned u3:3;
        unsigned u31:31;
        unsigned u32:32;
        unsigned long ul31:31;
        unsigned long ul32:32;
        unsigned long long ull31:31;
        unsigned long long ull32:32;
        unsigned long long ull33:33;
        unsigned long long ull64:64;
        unsigned char c;
    } s = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

    promote(s.u3);
    promote(s.u31);
    promote(s.u32);
    promote(s.ul31);
    promote(s.ul32);
    promote(s.ull31);
    promote(s.ull32);
    promote(s.ull33);
    promote(s.ull64);
    promote(s.c);
    printf("\n");

    promote((1 ? s.u3 : 1));
    promote((1 ? s.u31 : 1));
    promote((1 ? s.u32 : 1));
    promote((1 ? s.ul31 : 1));
    promote((1 ? s.ul32 : 1));
    promote((1 ? s.ull31 : 1));
    promote((1 ? s.ull32 : 1));
    promote((1 ? s.ull33 : 1));
    promote((1 ? s.ull64 : 1));
    promote((1 ? s.c : 1));
    printf("\n");

    promote(s.u3 << 1);
    promote(s.u31 << 1);
    promote(s.u32 << 1);
    promote(s.ul31 << 1);
    promote(s.ul32 << 1);
    promote(s.ull31 << 1);
    promote(s.ull32 << 1);
    promote(s.ull33 << 1);
    promote(s.ull64 << 1);
    promote(s.c << 1);
    printf("\n");

    promote(+s.u3);
    promote(+s.u31);
    promote(+s.u32);
    promote(+s.ul31);
    promote(+s.ul32);
    promote(+s.ull31);
    promote(+s.ull32);
    promote(+s.ull33);
    promote(+s.ull64);
    promote(+s.c);
    printf("\n");

    promote(-s.u3);
    promote(-s.u31);
    promote(-s.u32);
    promote(-s.ul31);
    promote(-s.ul32);
    promote(-s.ull31);
    promote(-s.ull32);
    promote(-s.ull33);
    promote(-s.ull64);
    promote(-s.c);
    printf("\n");

    promote(~s.u3);
    promote(~s.u31);
    promote(~s.u32);
    promote(~s.ul31);
    promote(~s.ul32);
    promote(~s.ull31);
    promote(~s.ull32);
    promote(~s.ull33);
    promote(~s.ull64);
    promote(~s.c);
    printf("\n");

    promote(!s.u3);
    promote(!s.u31);
    promote(!s.u32);
    promote(!s.ul31);
    promote(!s.ul32);
    promote(!s.ull31);
    promote(!s.ull32);
    promote(!s.ull33);
    promote(!s.ull64);
    promote(!s.c);
    printf("\n");

    promote(+(unsigned)s.u3);
    promote(-(unsigned)s.u3);
    promote(~(unsigned)s.u3);
    promote(!(unsigned)s.u3);

    return 0;
}
