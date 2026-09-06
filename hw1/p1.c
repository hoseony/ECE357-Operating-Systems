#include <stdio.h>

int foo(void) {
    return 0;
}

int main(void) {
    printf("foo");
    foo();

    printf("return");
    return 0;
}
