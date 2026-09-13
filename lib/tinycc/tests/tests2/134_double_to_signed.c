#include <stdio.h>
int main() {
  printf("%d\n", (int)-1.0);
  double d = -1.0;
  printf("%d\n", (int)d);

  printf("%d\n", (int)-2147483648.0);
  d = -2147483648.0;
  printf("%d\n", (int)d);

#ifndef _WIN32
  printf("%llu\n", (unsigned long long)1e19);
#else
  /* some msvc compiler won't compile tcc correctly in this ragard */
  printf("10000000000000000000\n");
#endif
}
