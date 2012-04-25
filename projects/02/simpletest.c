#include "lwp.h"

#include <stdio.h>

unsigned long * sp;
unsigned long * bp;

static void test(void *);
static void test_without_exit(void *);
static void test_param(void *);

int main(int argc, char *argv[]){
    /* Returns immediately. */
    lwp_start();

    /* Prints "Hallo thar!" once and exits. */
    printf("Expected: Hallo thar!\n----\n");
    new_lwp(test, (void *)212, 256);
    lwp_start();

    printf("---------------------------------\n");
    
    /* Returns immediately. */
    lwp_start();

    /* Prints "Hallo thar!" once and exits. */
    printf("Expected: Hallo thar!\n----\n");
    new_lwp(test_without_exit, (void *)212, 256);
    lwp_start();

    printf("---------------------------------\n");
    
    printf("Expected: Bar is 212\n----\n");
    new_lwp(test_param, (void *)212, 256);
    lwp_start();
    
    return 0;
}

static void test(void * bar) {
    GetSP(sp);
    GetBP(bp);
    printf("Hallo thar!\n");
    lwp_exit();
}

static void test_without_exit(void * bar) {
    GetSP(sp);
    GetBP(bp);
    /* This line should not be necessary. */
    /*bp[1] = (unsigned long)lwp_exit;*/
    printf("Hallo thar!\n");
}

static void test_param(void * bar) {
    GetSP(sp);
    GetBP(bp);
    printf("Bar is %d\n", (int)bar);
}

