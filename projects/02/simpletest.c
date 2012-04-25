#include "lwp.h"

#include <stdio.h>

unsigned long * sp;
unsigned long * bp;

static void test(void *);
static void test_without_exit(void *);
static void test_param(void *);
static void test_pid(void *);
static void test_proc_count(void *);

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

    printf("---------------------------------\n");
    
    printf("Expected: PID is 3\n----\n");
    new_lwp(test_pid, (void *)212, 256);
    lwp_start();

    printf("---------------------------------\n");
    
    printf("Expected: There are 1 running threads.\n----\n");
    new_lwp(test_proc_count, (void *)212, 256);
    lwp_start();

    printf("---------------------------------\n");
    
    printf("Expected: Hallow thar! x 2\n----\n");
    new_lwp(test, (void *)212, 256);
    new_lwp(test, (void *)213, 256);
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

static void test_pid(void * bar) {
    GetSP(sp);
    GetBP(bp);
    printf("PID is %d\n", lwp_getpid());
}

static void test_proc_count(void * bar) {
    GetSP(sp);
    GetBP(bp);
    printf("There are %d threads running.\n", lwp_procs);
}

