#include "lwp.h"

#include <unistd.h>

schedfun gScheduler = NULL;

/**
 * Creates a new lightweight process which calls the given function with the
 * given argument.  The new process's stack will be `stacksize` words.  The
 * LWP's process table entry will include:
 *    `pid`        a unique integer process id
 *    `stack`      a pointer to the memory region for this thread's stack
 *    `stacksize`  the size of this thread's stack in words
 *    `sp`         this thread's current stack pointer (top of stack)
 * @return the lightweight process id of the new thread on success
 *         -1 if more than LWP_PROC_LIMIT threads already exist
 */
int new_lwp(lwpfun function, void * argument, size_t stacksize);

/**
 * @return the pid of the calling LWP.  Undefined if not called by a LWP.
 */
int lwp_getpid();

/**
 * Yields control to another LWP.  Which one depends on the scheduler.  Saves
 * the current LWP's context, picks the next one, restores that thread's
 * context, and returns.
 */
void lwp_yield();

/**
 * Terminates the current LWP, frees its resources, and moves all the others up
 * in the process table.  If there are no other threads, calls `lwp_stop()`.
 */
void lwp_exit();

/**
 * Starts the LWP system.  Saves the original context and stack pointer (for
 * `lwp_stop()` to use later), picks a LWP and start it running.  If there are
 * no LWPs, returns immediately.
 */
void lwp_start();

/**
 * Stops the LWP system, restores the original stack pointer and returns to
 * that context (wherever `lwp_start()` was called from).  Does not destory any
 * existing contexts, and thread processing will be restarted by a call to
 * `lwp_start()`.
 */
void lwp_stop();

/**
 * Causes the LWP package to use the function `scheduler` to choose the next
 * process to run.  `(*scheduler)()` must return an integer in the range
 * 0...lwp_procs - 1, representing an index into `lwp_ptable`, or -1 if there
 * is no thread to schedule.
 * If `scheduler` is null or has never been set, the scheduler should do
 * round-robin scheduling.
 */
void lwp_set_scheduler(schedfun scheduler) {
	gScheduler = scheduler;
}

