#include "coroutines.h"

#define Iterations 500000000

// Global variables
coroutine_t *co_main;
coroutine_t *co_bench;
volatile int counter;

void coroutine_bench(void* arg) {
    for (;;) {
        counter++;
        coroutine_switch(co_main);
    }
}

void function_bench(void) {
    counter++;
}

int main() {
    printf("context-switching timing test\n\n");
    time_t start, end;
    int i, t1, t2;

    start = clock();
    for (counter = 0, i = 0; i < Iterations; i++) {
        function_bench();
    }
    end = clock();

    t1 = (int)difftime(end, start);
    printf("%2.3f seconds per 50 million subroutine calls (%d iterations)\n", (float)t1 / CLOCKS_PER_SEC, counter);

    co_main = coroutine_init_main();
    co_bench = coroutine_init(4096, coroutine_bench, NULL);

    start = clock();
    for (counter = 0, i = 0; i < Iterations; i++) {
        coroutine_switch(co_bench);
    }
    end = clock();

    coroutine_deinit(co_main);
    coroutine_deinit(co_bench);

    t2 = (int)difftime(end, start);
    printf("%2.3f seconds per 100 million co_switch calls (%d iterations)\n", (float)t2 / CLOCKS_PER_SEC, counter);

    printf("switch overhead skew = %fx\n", (double)t2 / (double)t1);

    return 0;
}

