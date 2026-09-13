#include <stdio.h>

static void f(int x)
{
     printf("f(%d)\n", x);
}

int main(void)
{
     int count = 0, i = 0;
     for (; i < 3; ++i) {
         printf("%d\n", i);
         (void)(i || (f(i), ++count));
     }
     printf("count %d\n", count);
     return count == 1 ? 0 : 1;
}
