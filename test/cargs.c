#include <stdio.h>

int main (int n, char** args) {
    for(int i=0; i<n; ++i) {
        printf("%d: %s\n",i, *args);
        ++args;
    }
}
