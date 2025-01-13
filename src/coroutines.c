#include "coroutines.h"

__thread coroutine_t *current_coroutine = NULL;

__attribute__((naked))
void switch_context(coroutine_state_t *from, coroutine_state_t *to) {
    __asm__ volatile (
        /*
            0x00                  -->                  0xff
            r12 r13 r14 r15 rip rsp rbx rbp fpucw16 mxcsr32
            0   8   10  18  20  28  30  38  40      44
        */
        // rdi - from_co | rsi - to_co
        "movq    (%%rsp), %%rdx      \n"  // retaddr
        "leaq    8(%%rsp), %%rcx            \n"  // rsp
        "movq    %%r12, 0x0(%%rdi)   \n"
        "movq    %%r13, 0x8(%%rdi)   \n"
        "movq    %%r14, 0x10(%%rdi)  \n"
        "movq    %%r15, 0x18(%%rdi)  \n"
        "movq    %%rdx, 0x20(%%rdi)  \n"  // retaddr
        "movq    %%rcx, 0x28(%%rdi)  \n"  // rsp
        "movq    %%rbx, 0x30(%%rdi)  \n"
        "movq    %%rbp, 0x38(%%rdi)  \n"
        "fnstcw  0x40(%%rdi)         \n"
        "stmxcsr 0x44(%%rdi)        \n"

        "movq    0x0(%%rsi), %%r12   \n"
        "movq    0x8(%%rsi), %%r13   \n"
        "movq    0x10(%%rsi), %%r14  \n"
        "movq    0x18(%%rsi), %%r15  \n"
        "movq    0x20(%%rsi), %%rax  \n"  // retaddr
        "movq    0x28(%%rsi), %%rcx  \n"  // rsp
        "movq    0x30(%%rsi), %%rbx   \n"
        "movq    0x38(%%rsi), %%rbp  \n"
        "fldcw   0x40(%%rsi)         \n"
        "ldmxcsr 0x44(%%rsi)        \n"
        "movq    %%rcx, %%rsp       \n"
        "jmp     *%%rax             \n"
        : /* No output operands */
        : /* Input operands are passed via registers */
        : "memory", "rax", "rcx", "rdx" , "r12", "r13", "r14", "r15" // clobbered registers
    );
}

void coroutine_crash(void) {
  assert(0);  /* called only if coroutine entrypoint returns */
}

coroutine_t *coroutine_init(size_t stack_size, coroutine_func_t func, void *arg) {
    coroutine_t *co = malloc(sizeof(coroutine_t));
    if (!co) {
        perror("Failed to allocate coroutine");
        return NULL;
    }

    co->stack_size = stack_size;
    co->stack = malloc(stack_size);
    if (!co->stack) {
        perror("Failed to allocate stack");
        free(co);
        return NULL;
    }

    // Align stack pointer to 16-byte boundary
    uintptr_t stack_top = (uintptr_t)co->stack + stack_size;
    stack_top -= stack_top % 16;
    uint64_t *stack_ptr = (uint64_t *)stack_top;

    // Simulate a call by pushing the entry point and the argument
    *(--stack_ptr) = (uint64_t)arg; // Argument to the coroutine function
    *(--stack_ptr) = (uint64_t)coroutine_crash; // Return address (in case the coroutine returns)
    *(--stack_ptr) = (uint64_t)func;  // Coroutine function address

    memset(&co->state, 0, sizeof(coroutine_state_t));
    co->state.rsp = (uint64_t)stack_ptr;
    co->state.rip = (uint64_t)func; // Set instruction pointer for initial start

    return co;
}

void coroutine_deinit(coroutine_t *co) {
    if (co) {
        free(co->stack);
        free(co);
    }
}

void coroutine_switch(coroutine_t *next) {
    assert(current_coroutine != NULL);
    if (current_coroutine == next) {
        return;
    }

    coroutine_t *previous = current_coroutine;
    current_coroutine = next;
    switch_context(&previous->state, &next->state);
}

// A helper function to get the current coroutine
coroutine_t *coroutine_get_current() {
    return current_coroutine;
}

coroutine_t *coroutine_init_main(void) {
    coroutine_t *co = malloc(sizeof(coroutine_t));
    if (!co) {
        perror("Failed to allocate main coroutine");
        return NULL;
    }

    // We don't need to allocate a separate stack for the main coroutine,
    // it's already using the main thread's stack.
    co->stack = NULL;
    co->stack_size = 0;

    // Initialize the state. Crucially, the rsp should point to the
    // current stack. We can get an approximate value.
    // The rip can be a placeholder, as we won't be "starting" this coroutine.
    memset(&co->state, 0, sizeof(coroutine_state_t));

    // Get the current stack pointer (approximate)
    uint64_t current_sp;
    __asm__ volatile ("mov %%rsp, %0" : "=r" (current_sp));
    co->state.rsp = current_sp;

    // Set the current coroutine to this "main" coroutine
    current_coroutine = co;

    return co;
}
