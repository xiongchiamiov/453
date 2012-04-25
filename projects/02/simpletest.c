#include "lwp.h"

#include <stdio.h>

unsigned long * sp;
unsigned long * bp;

static void foo(void *);
static void foo2(void *);

int main(int argc, char *argv[]){
    /* Returns immediately. */
    lwp_start();

    /* Prints "Hallo thar!" once and exits. */
    new_lwp(foo, (void *)212, 256);
    lwp_start();

    printf("---------------------------------\n");
    
    /* Returns immediately. */
    lwp_start();

    /* Prints "Hallo thar!" once and exits. */
    new_lwp(foo2, (void *)212, 256);
    lwp_start();

    return 0;
}

static void foo(void * bar) {
    GetSP(sp);
    GetBP(bp);
    printf("Hallo thar!\n");
    lwp_exit();
}

static void foo2(void * bar) {
    GetSP(sp);
    GetBP(bp);
    printf("Hallo thar!\n");
}

