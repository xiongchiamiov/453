James Pearson (jmpearso)  
CPE 453-09
Lab 04

# Back Up Kernel

## Approach

Read `boot(8)`.  Copy /boot/image to /boot/image.bak.

## Problems Encountered

Attempting to boot now caused a hang after networking started.  Attempting to
boot from the backup image also didn't work.

## Solutions

After playing around a bit with a fresh install, I found that shutting down
over SSH hung up the VirtualBox forwarding in some weird way; thankfully, I
took VM snapshots this time around.

The solution is easy enough - shut it down from the virtual console in
VirtualBox or use a time parameter to shutdown that allows enough time to close
the SSH connection.

After I figured out this (unrelated) problem, booting from the alternate image
worked just fine.

# Modifying the Kernel

## Approach

To make sure I was running a modified version of the kernel, I thought I'd
change the OS name as outputted by `uname`; after digging through source files
for half an hour, I decided to give that particular pursuit up.

All of the kernel source appeared to be in `/usr/src/kernel`, so I went in
there and grepped for "sched", then "idle", which lead me to the `idle()`
function in `proc.c`.  I added my `kprintf` line in there.

## Problems Encountered

`kprintf` was not defined in my version of Minix; a little searching around
found `/usr/src/lib/libsys/kprintf.c`, which only contained a function
definition for `printf`.

## Solutions

Used `printf` instead of `kprintf`, as hinted at by the lab footnote.

It was easy to tell I was using the right kernel because it printed an annoying
"@" all the time.

