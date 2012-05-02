James Pearson (jmpearso)  
CPE 453-09
Lab 03

1. Bad Things.  I assume the filesystem will issue some sort of signal
indicating that it can't read the file.

2. Multiprogramming is particularly useful when processes are in wait, usually
because of I/O; it allows other processes to run while the disk/network/scanner
is spinning/downloading/scanning.

3. 1 000 000 000 / 2 / 1 000 = 500 000 system calls per second.

4. A race condition is when the precise timing affects the behavior of
processes.  Like memory allocation issues, race conditions usually manifest
themselves unpredictably.

5. The busy-waiting system will work just as well on a shared-memory
multiprocessor as on a single-processor system, since the turn variable is
changed *after* the critical region.

6. The OS should disable interrupts at the start of UP(s) and DOWN(s) and
enable them again at the end, making those operations atomic.

7. If a process occurs twice in a round-robin queue, it will be run twice as
many times as processes that only occur once.  Extending this to being in the
list n times allows the scheduler to give certain processes a larger chunk of
CPU time; in essence, this is adding priority levels.

8. a) C:13, D:18, E:23, B:27, A:30  Mean: 22.2
   b) B: 7, E:13, A:23, C:26, D:30  Mean: 19.8
   c) A:10, B:17, C:20, D:24, E:30  Mean: 20.2
   d) C: 3, D: 7, E:13, B:20, A:30  Mean: 14.6

10. 5 swaps are needed, as calculated using Python:

    >>> swaps = 0
    >>> quantas = 30
    >>> while quantas > 0:
    ...    quantas -= 2**swaps
    ...    swaps += 1
    ... 
    >>> swaps
    5

