int printf(const char *s, ...);

float fx(x)
float x;
{
    return 2.0 * x;
}

void func(float a);

void func3(struct p { int a; int b; } *q) {
}

void func4(q)
 struct p { int a; int b; int c; } *q;
{
}

struct p { int a; int b; int c; int d; };

int
main(void)
{
    float fy();

    printf("%g %g\n", fx(2.0), fy(10.0));
    printf("%g %g\n", fx(2.0f), fy(10.0f));
    func(1);
}

float fy(x)
float x;
{
    return 3.0 * x;
}

void func(a)
float a;
{
    printf("%g\n", a);
}
