# Get a Minix System

## Approach

Follow the instructions on [the Minix wiki][0] for installing Minix on
VirtualBox.  I chose Minix 3.1.8 because it was the latest stable in the 3.1.x
branch and you mentioned that Minix versions don't follow semantic versioning
(in regards to 3.1 vs. 3.2, at least).

[0]: http://wiki.minix3.org/en/UsersGuide/RunningMinixOnVirtualBox

## Problems Encountered

I already know my way around VirtualBox, and it's not that difficult anyways.

## Solutions

None needed.

# Log In

## Approach

Use root:[blank] as instructed.

## Problems Encountered

When I was first booting, I forgot what VirtualBox's escape key was set to, so
I pressed Ctrl before properly hitting Left-Cmd.  This keypress got included
(transparently) into the username field on login, so my first login failed.

## Solutions

I just tried logging in again.

## Lessons Learned

Apparently ctrl by itself is a valid character in Minix's login system.

# Installation

## Approach

This section isn't actually described in the lab, but I figured it was a good
idea to actually install Minix to the (virtual) hard drive.

## Problems

I tried to use `shutdown -hP now` like I do in Linux, but Minix's version of
shutdown doesn't have `-P`.

## Solutions

Use plain ol' `shutdown` and then power off the machine via VirtualBox's menus
once we return to the boot loader.

## Lessons Learned

I can't directly power off the machine from inside Minix.

# Create a User Account

## Approach

Run `adduser`.  Run `adduser pearson operator /home/pearson`.

## Problems

After installing `sudo`, I set up access for my new user using `visudo`, `su`ed
to them, and attempted to use `sudo -s` to obtain a root shell (so I could lock
the root account).  This caused a segfault.  The same thing happened with `sudo
su`.

Also, attempting to launch `zsh` (the best shell) caused my host machine's CPU
usage to spike and stay fully-utilized until I interrupted the process.

## Solutions

After playing around a bit, I gave up on having a remotely secure VM, since,
after all, it's a local VM on which there is nothing important.  Also, resigned
myself to `bash`.

## Lessons Learned

Minix package maintainers suck.

# Create a Minix Floppy

This is a boring way to transfer data, so I skipped it and set up ssh instead.

# Set Up SSH

## Approach

Installed ssh using `pkgin`.  Started it with `/usr/pkg/etc/rc.d/sshd`.

## Problems

I couldn't find the init-script until I remembered that (when doing shell
configuration) I discovered Minix puts user-installed programs in `/usr/pkg`.
Sure enough, there's an `etc` in there.

I also had to set up port forwarding in VirtualBox.  The instructions on the
Minix wiki indicated forwarding 22 -> 22, but I already run SSH on this
machine, so that would be silly.

## Solutions

Forward 2200 locally to 22 on the guest machine.

## Lessons Learned

Networking support is a lot easier than you made it sound.

