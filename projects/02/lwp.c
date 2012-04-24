#include "lwp.h"

#include <stdlib.h>
#include <unistd.h>

schedfun gScheduler = NULL;
void * gStackPointer;
unsigned int nextPid = 0;

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
int new_lwp(lwpfun function, void * argument, size_t stacksize) {
	size_t stackSizeInBytes = stacksize * sizeof(int);
	unsigned long * stackPointer;
	
	/* For simplicity's sake, we limit ourselves to a set number of LWPs. */
	if (lwp_procs == LWP_PROC_LIMIT) {
		return -1;
	}
	/* lwp_running is 0-based, so set it *before* incrementing lwp_procs. */
	lwp_running = lwp_procs;
	lwp_procs++;
	
	lwp_ptable[lwp_procs].pid = nextPid++;
	lwp_ptable[lwp_procs].stack = malloc(stackSizeInBytes);
	lwp_ptable[lwp_procs].stacksize = stacksize;
	stackPointer = lwp_ptable[lwp_procs].stack + stackSizeInBytes;
	
	/* Build up our stack in preparation for lwp_yield. */
	*stackPointer = (unsigned long)argument;
	stackPointer -= sizeof(int);
	*stackPointer = (unsigned long)lwp_exit;
	stackPointer -= sizeof(int);
	*stackPointer = (unsigned long)function;
	stackPointer -= sizeof(int);
	*stackPointer = (unsigned long)lwp_exit;
	stackPointer -= sizeof(int);
	/* Space for the 7 registers SAVE_STATE saves. */
	stackPointer -= 7 * sizeof(int);
	
	lwp_ptable[lwp_procs].sp = stackPointer;
	
	return lwp_running;
}

/**
 * @return the pid of the calling LWP.  Undefined if not called by a LWP.
 */
int lwp_getpid() {
	return lwp_ptable[lwp_running].pid;
}

/**
 * Yields control to another LWP.  Which one depends on the scheduler.  Saves
 * the current LWP's context, picks the next one, restores that thread's
 * context, and returns.
 */
void lwp_yield() {
	/* Save context for later. */
	SAVE_STATE();
	
	/* Save stack pointer for later. */
	GetSP(lwp_ptable[lwp_running].sp);
	
	/* Pick a thread to run. */
	if (gScheduler == NULL) {
		/* Round-robin. */
		lwp_running++;
		/* Go back to the start of the list if we've reached the end. */
		if (lwp_running == lwp_procs) {
			lwp_running = 0;
		}
	} else {
		lwp_running = gScheduler();
	}
	SetSP(lwp_ptable[lwp_running].sp);
	
	/* And run it. */
	RESTORE_STATE();
}

/**
 * Terminates the current LWP, frees its resources, and moves all the others up
 * in the process table.  If there are no other threads, calls `lwp_stop()`.
 */
void lwp_exit() {
	int i;
	
	/* Return thread's stack to system. */
	free(lwp_ptable[lwp_running].stack);
	
	/* Shift the process table. */
	for (i=lwp_running+1; i<lwp_procs; i++) {
		lwp_ptable[i-1] = lwp_ptable[i];
	}
	
	/* Stop if we don't have any more threads. */
	lwp_procs--;
	if (lwp_procs == 0) {
		lwp_stop();
	}
}

/**
 * Starts the LWP system.  Saves the original context and stack pointer (for
 * `lwp_stop()` to use later), picks a LWP and start it running.  If there are
 * no LWPs, returns immediately.
 */
void lwp_start() {
	if (lwp_procs == 0) {
		return;
	}
	
	/* Save context for later. */
	SAVE_STATE();
	
	/* Save stack pointer for later. */
	GetSP(gStackPointer);
	
	/* Pick a thread to run. */
	if (gScheduler == NULL) {
		/* Round-robin. */
		/* Let's start at the very beginning, a very good place to start. */
		lwp_running = 0;
	} else {
		lwp_running = gScheduler();
	}
	SetSP(lwp_ptable[lwp_running].sp);
	
	/* And run it. */
	RESTORE_STATE();
}

/**
 * Stops the LWP system, restores the original stack pointer and returns to
 * that context (wherever `lwp_start()` was called from).  Does not destory any
 * existing contexts, and thread processing will be restarted by a call to
 * `lwp_start()`.
 */
void lwp_stop() {
	/* Save context for later. */
	SAVE_STATE();
	
	/* Reset stack pointer to original context. */
	SetSP(gStackPointer);
	
	/* Continue on our way. */
	RESTORE_STATE();
}

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

