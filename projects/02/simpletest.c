#include "lwp.h"

#include <stdio.h>

static void foo(void *);

int main(int argc, char *argv[]){
    /* Should return immediately. */
    /* lwp_start(); */

    new_lwp(foo, (void *)212, 256);

    lwp_start();

    return 0;
}

static void foo(void * bar) {
    printf("Hallo thar!\n");
    lwp_exit();
}

