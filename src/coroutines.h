#ifndef COROUTINES
#define COROUTINES

#include <assert.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct coroutine_state_t {
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rip;
    uint64_t rsp;
    uint64_t rbx;
    uint64_t rbp;
    uint16_t fpu_cw;
    uint32_t mxcsr;
} coroutine_state_t;

typedef struct coroutine_t {
    coroutine_state_t state;
    uint8_t* stack;
    size_t stack_size;
} coroutine_t;

typedef void (*coroutine_func_t)(void *arg);

void coroutine_crash(void);
coroutine_t *coroutine_init(size_t stack_size, coroutine_func_t func, void *arg);
void coroutine_deinit(coroutine_t *coroutine);
void coroutine_switch(coroutine_t *next);
void switch_context(coroutine_state_t *from, coroutine_state_t *to);
coroutine_t *coroutine_get_current();
coroutine_t *coroutine_init_main(void);


#endif // !COROUTINES
