/* Test CFLAGS env parsing
 * run me successfully only with:
 *    CFLAGS="-DALLOWED=1" cjit test/cflags.c
 */

#if !defined(ALLOWED) || (ALLOWED != 1)

#error Running this program is not allowed. Please compile with -DALLOWED=1

#else

#include <stdio.h>

int main(void) 
{
    printf("Success.\n");
    return 0;
}
#endif
