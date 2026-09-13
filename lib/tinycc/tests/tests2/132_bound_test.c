#include <stdio.h>
#include <float.h>

union ieee_double_extract
{
    struct {
        unsigned int manl:32;
        unsigned int manh:20;
        unsigned int exp:11;
        unsigned int sig:1;
    } s;
    double d;
};

double scale(double d)
{
    union ieee_double_extract x;

    x.d = d;
    x.d *= 1000;
    return x.d;
}

void mul(double *p)
{
    *p *= 2.0;
}

int
main(void)
{
    double d = 4.0;
    printf("%g\n", scale(42));
    mul(&d);
    printf("%g\n", d);
    return 0;
}
