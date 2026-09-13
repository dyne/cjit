int printf(const char*, ...);

int main()
{
    char a;
    short b;

    printf("sizeof a : %d %d\n", sizeof(char), sizeof(a));
    printf("sizeof b : %d %d\n", sizeof(short), sizeof(b));

    int ii[] = {}; /* gnu extension, size = 0 */
    printf("sizeof ii : %d\n", sizeof ii);

    int kk[] = { 1 };
    printf("sizeof kk : %d\n", sizeof kk);

    char cc[] = "12";
    printf("sizeof cc : %d\n", sizeof cc);

    __WCHAR_TYPE__ ll[] = L"12345";
    printf("len-of ll : %d\n", sizeof ll / sizeof ll[0]);

    static struct {
        int a,b,c;
        int d[];
    } ss[] = {{ 1, 2, 3, {} }};
    printf("sizeof ss : %d\n", sizeof ss);

    return 0;
}
