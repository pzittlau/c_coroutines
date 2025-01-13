#include <assert.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "coroutines.h"

coroutine_t *co_a;
coroutine_t *co_b;
coroutine_t *co_main;

void coroutine_a(void *arg) {
    int id = *(int *)arg;
    for (int i = 0; i < 5; ++i) {
        printf("Coroutine A [%d]: %d\n", id, i);
        coroutine_switch(co_b);
    }
    printf("Coroutine A [%d] finished\n", id);
    coroutine_switch(co_b);
}

void coroutine_b(void *arg) {
    int id = *(int *)arg;
    for (int i = 0; i < 5; ++i) {
        printf("Coroutine B [%d]: %d\n", id, i);
        coroutine_switch(co_a);
    }
    printf("Coroutine B [%d] finished\n", id);
    coroutine_switch(co_main);
}

int main() {
    co_main = coroutine_init_main();
    if (!co_main) {
        return 1;
    }

    int id_a = 0;
    int id_b = 1;

    co_a = coroutine_init(4096, coroutine_a, &id_a);
    co_b = coroutine_init(4096, coroutine_b, &id_b);
    if (!co_a || !co_b){
        return 1;
    }

    coroutine_switch(co_a);

    printf("Main finished\n");

    return 0;
}
