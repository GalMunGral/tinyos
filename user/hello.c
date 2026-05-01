#include "lib.h"

void main(long pid, long c) {
    for (;;) {
        printf("[process (%d)]: %c ", pid, (char)c);
        sleep(100);
    }
}