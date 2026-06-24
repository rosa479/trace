#include <stdio.h>

int add(int a, int b) {
    return a + b;
}

int main() {
    int x = 5, y = 3;
    int z = add(x, y);
    printf("result: %d\n", z);
    return 0;
}
