James Pearson (jmpearso)
CPE 453-09
Assignment 3

# Overall Architecture

## How will the driver be initialized?

The `8ball` executable gets run by the reincarnation server (RS).  RS is told
to do this either by a manual call to `service` (`service up /usr/sbin/8ball
-dev /dev/8ball`) or the `boot_image_priv_table` data structure.

`8ball` itself runs an initialization routine before entering the
message-receiving infinite loop.

## How will the driver be started?

"Starting" and "initializing" are synonyms, as far as I know, so the answer to
this is the same as the previous section.

## How will it randomize answers?

Since this seemed to be a less important part of the assignment, I left it
until later (and never got to it).

`stdlib.h` and `time.h` provide standard randomization functions; there is also
`/dev/random`, which could be utilized with a small amount of work (read a
character and use it modulo 20 as the index into the answer array).

## How will it enforce the requirement that it not answer without being asked a question?

There exists a global state variable that records the number of queued answers;
that is, it starts at 0, increments every time a question comes in, and
decrements every time we output an answer.

Initially, a read request when there were no queued answers would receive the
EIO status, indicating that it should sleep while input was buffered.  However,
this didn't appear to be the behavior of the model, so instead `8ball` returns
a NUL character (`\0`) to indicate that there is no more to be read.

## How will it behave to not bring the rest of the operating system to a screeching halt?

`8ball` uses the non-blocking `nbsend` variant of the `send` system call when
responding to messages and attempts to avoid blocking calls elsewhere.

# Detailed Description

## Development Environment

Minix 10.0.2.15 3.1.8-pearson (stock kernel with my modifications for this
assignment) running on VirtualBox 4.0.6.

## Modified Files

### include/minix/com.h

	--- /usr/src/include/minix/com.h.orig	Sun May 20 22:28:39 2012
	+++ /usr/src/include/minix/com.h	Sun May 20 22:46:57 2012
	@@ -74,7 +74,8 @@
	 #define VM_PROC_NR   ((endpoint_t) 8)   /* memory server */
	 #define PFS_PROC_NR  ((endpoint_t) 9)  /* pipe filesystem */
	 #define SCHED_PROC_NR ((endpoint_t) 10)	/* scheduler */
	-#define LAST_SPECIAL_PROC_NR	11	/* An untyped version for
	+#define EIGHTBALL_PROC_NR  ((endpoint_t) 19)	/* terminal (TTY) driver */
	+#define LAST_SPECIAL_PROC_NR	12	/* An untyped version for
												computation in macros.*/
	 #define INIT_PROC_NR ((endpoint_t) LAST_SPECIAL_PROC_NR)  /* init
															 -- goes multiuser */

To include `8ball` in the list of drivers started at boot by RS, `TODO/table.c`
needed to know about the major device number associated with `/dev/8ball`.
Also, `TODO/table.c` performs a compile-time sanity check to ensure that a few
tables are the sizes it expects, and thus we needed to bump up the number of
"special processes" by one.

### drivers/Makefile

	--- /usr/src/drivers/Makefile.orig	Sat May 19 16:23:01 2012
	+++ /usr/src/drivers/Makefile	Sat May 19 16:23:16 2012
	@@ -8,7 +8,7 @@
	 SUBDIR = acpi
	 .endif
	 
	-SUBDIR+= ahci amddev atl2 at_wini audio bios_wini dec21140A dp8390 dpeth \
	+SUBDIR+= 8ball ahci amddev atl2 at_wini audio bios_wini dec21140A dp8390 dpeth \
	 	e1000 filter floppy fxp hello lance log orinoco pci printer \
	 	random readclock rtl8139 rtl8169 sb16 ti1225 tty \
	 	.WAIT ramdisk .WAIT memory

We want to compile `8ball` when doing an all-drivers build.

### /etc/rc

	--- /etc/rc.orig	Sat May 19 16:47:12 2012
	+++ /etc/rc	Sun May 20 22:01:47 2012
	@@ -182,6 +182,7 @@
	 	edit vfs
	 	edit ds
	 	edit tty
	+	edit 8ball
	 	edit memory
	 	edit -p log
	 	edit -c pfs

`/etc/rc` is run at startup; under normal circumstances, it performs minor
edits to the services known to RS using the `service` command.  Since we're
planning on having `8ball` start on boot, it should be included in this list.

### servers/rs/table.c

	--- /usr/src/servers/rs/table.c.orig	Sun May 20 23:41:23 2012
	+++ /usr/src/servers/rs/table.c	Sun May 20 23:43:07 2012
	@@ -21,6 +21,7 @@
	 {VFS_PROC_NR,  "vfs",    SRV_F  },
	 {DS_PROC_NR,   "ds",     SRV_F  },
	 {TTY_PROC_NR,  "tty",    SRV_F  },
	+{EIGHTBALL_PROC_NR,"8ball",SRV_F},
	 {MEM_PROC_NR,  "memory", SRV_F  },
	 {LOG_PROC_NR,  "log",    SRV_F  },
	 {MFS_PROC_NR,"fs_imgrd", SRV_F  },

This is the list of service RS knows about (and starts) at boot.

### /etc/system.conf

	--- /etc/system.conf.orig	Sun May 20 12:24:27 2012
	+++ /etc/system.conf	Sun May 20 14:53:08 2012
	@@ -2,6 +2,14 @@
	 # Boot system services in the boot image
	 #
	 
	+service 8ball
	+{
	+	uid     0;
	+	ipc	ALL;		# ALL ipc targets allowed
	+	system	ALL;		# ALL kernel calls allowed
	+	io	ALL;		# ALL I/O ranges allowed
	+};
	+
	 service rs
	 {
	 	uid     0;

`service` gets unhappy dealing with `8ball` unless it knows a few details about
it.  This tells RS to run `8ball` as the root user and to give it a few
necessary (and a few unnecessary) permissions.

### kernel/table.c

	--- /usr/src/kernel/table.c.orig	Sun May 20 22:24:22 2012
	+++ /usr/src/kernel/table.c	Sun May 20 22:46:31 2012
	@@ -84,6 +84,7 @@
	 {MEM_PROC_NR,  BVM_F, 0,     "memory"},
	 {LOG_PROC_NR,  BVM_F, 0,     "log"   },
	 {TTY_PROC_NR,  BVM_F, 0,     "tty"   },
	+{EIGHTBALL_PROC_NR,BVM_F, 0,     "8ball" },
	 {MFS_PROC_NR,  BVM_F, 0,     "mfs"   },
	 {VM_PROC_NR,       0, 0,     "vm"    },
	 {PFS_PROC_NR,  BVM_F, 0,     "pfs"   },

Another table listing at-boot drivers; if the one in `servers/rs/table.c` is
modified and this isn't, the kernel gets rather unhappy at boot and `panic`s
about the inconsistency.

### include/minix/types.h

	--- /usr/src/include/minix/types.h.orig	Sun May 20 22:49:36 2012
	+++ /usr/src/include/minix/types.h	Sun May 20 22:49:38 2012
	@@ -36,7 +36,7 @@
	 typedef u32_t block_t;	   /* block number */
	 typedef u32_t bit_t;	   /* bit number in a bit map */
	 typedef u16_t zone1_t;	   /* zone number for V1 file systems */
	-typedef u16_t bitchunk_t; /* collection of bits in a bitmap */
	+typedef u32_t bitchunk_t; /* collection of bits in a bitmap */
	 
	 /* ANSI C makes writing down the promotion of unsigned types very messy.  When
	  * sizeof(short) == sizeof(int), there is no promotion, so the type stays

Adding another element to `boot_image_priv_table` increases the number of bits
needed for non-overlapping bitmasking.

## Magic * Ball(R) Driver

	#include <minix/drivers.h>
	#include <minix/driver.h>
	
	FORWARD _PROTOTYPE( void init, (void)					);
	FORWARD _PROTOTYPE( void do_cancel, (message *m_ptr)			);
	FORWARD _PROTOTYPE( void do_ioctl, (message *m_ptr)			);
	FORWARD _PROTOTYPE( void do_open, (message *m_ptr)			);
	FORWARD _PROTOTYPE( void do_close, (message *m_ptr)			);
	FORWARD _PROTOTYPE( void do_read, (message *m_ptr)			);
	FORWARD _PROTOTYPE( void do_write, (message *m_ptr)			);
	FORWARD _PROTOTYPE( void do_select, (message *m_ptr)			);
	FORWARD _PROTOTYPE( void reply, (int replyAddress, int process, int status));
	
	int eightBall_queuedResponses;
	
	/*===========================================================================*
	 *				tty_task				     *
	 *===========================================================================*/
	PUBLIC int main(void)
	{
		/* Main routine of the terminal task. */
	
		message eightBallMessage;		/* buffer for all incoming messages */
		int ipc_status;
		int r;
	
		init();
	
		while (TRUE) {
			printf("Waiting for a message...\n");
			/* Get a request message. */
			r= driver_receive(ANY, &eightBallMessage, &ipc_status);
			if (r != 0)
				panic("driver_receive failed with: %d", r);
	
			printf("8ball driver received message!\n");
			/* Execute the requested device driver function. */
			/* These constants are defined in minix/com.h . */
			switch (eightBallMessage.m_type) {
				/*case CANCEL:        do_cancel(&eightBallMessage);  break;*/
				case DEV_OPEN:      do_open(&eightBallMessage);    break;
				case DEV_CLOSE:     do_close(&eightBallMessage);   break;
				/*case TTY_SETPGRP:   break;*/
				/*case TTY_EXIT:      break;*/
				/*case DEV_SELECT:    do_select(&eightBallMessage);  break;*/
				/*case DEV_STATUS:    break;*/
				/*case DEV_REOPEN:    break;*/
				case DEV_READ_S:    do_read(&eightBallMessage);    break;
				case DEV_WRITE_S:   do_write(&eightBallMessage);   break;
				/*case DEV_SCATTER_S: break;*/
				/*case DEV_GATHER_S:  break;*/
				/*case DEV_IOCTL_S:   do_ioctl(&eightBallMessage);   break;*/
				/*case DEV_MMAP_S:    break;*/
				default:
					printf("Warning, 8ball got unexpected request %d from %d\n",
					       eightBallMessage.m_type-DEV_RQ_BASE,
					       eightBallMessage.m_source);
					reply(eightBallMessage.m_source, eightBallMessage.IO_ENDPT, EINVAL);
			}
		}
	
		return 0;
	}
	
	PRIVATE void init()
	{
		printf("Shaking the Magic 8 Ball(TM)...");
		sef_startup();
		eightBall_queuedResponses = 0;
		printf("ready.\n");
	}
	
	PRIVATE void reply(endpoint_t replyAddress, int process, int status)
	{
		message response;
		response.m_type = TASK_REPLY;
		response.REP_ENDPT = process;
		response.REP_STATUS = status;
		
		printf("8ball: replying to message\n");
		
		status = sendnb(replyAddress, &response);
		if (status != OK) {
			printf("reply: send to %d failed: %s (%d)\n",
			       replyAddress, strerror(status), status);
		}
	}
	
	PRIVATE void do_open(message* message)
	{
		printf("do_open called from process %d\n", message->IO_ENDPT);
		reply(message->m_source, message->IO_ENDPT, OK);
	}
	
	PRIVATE void do_close(message* message)
	{
		printf("do_close called from process %d\n", message->IO_ENDPT);
		reply(message->m_source, message->IO_ENDPT, DEV_CLOSE_REPL);
	}
	
	PRIVATE void do_read(message* message)
	{
		int r;
		int destinationProcess = message->IO_ENDPT;
		vir_bytes destinationAddress = (vir_bytes)(message->ADDRESS);
		phys_bytes bytesToCopy;
		char* response = (char*)malloc(1024);
		size_t responseSize = strlen(response);
		
		printf("do_read called from process %d\n", message->IO_ENDPT);
		
		/*if (eightBall_queuedResponses == 0) {
			printf("No queued 8ball responses!\n");
			r = EIO;
		} else if (message->COUNT <= 0) { */
		if (message->COUNT <= 0) {
			r = EINVAL;
		} else {
			if (eightBall_queuedResponses == 0) {
				strcpy(response, "\0");
			} else {
				strcpy(response, "Hello from the 8ball!");
				eightBall_queuedResponses--;
			}
			bytesToCopy = (message->COUNT > responseSize)
			              ? responseSize
			              : message->COUNT;
			printf("Copying the first %d bytes of %s to %d in %d...\n",
			       bytesToCopy, (vir_bytes)response, destinationAddress,
			       destinationProcess);
			/* minix/syslib.h */
			/* The lab instructions say to use sys_datacopy, but I couldn't get
			 * that to do anything useful.  So, we use sys_safecopyto, which is
			 * what tty.c uses.                                               */
			sys_safecopyto(
				destinationProcess,
				destinationAddress,
				0,
				(vir_bytes)response,
				bytesToCopy,
				D
			);
			r = bytesToCopy;
			printf("%d responses left\n", eightBall_queuedResponses);
		}
		
		reply(message->m_source, message->IO_ENDPT, r);
	}
	
	PRIVATE void do_write(message* message)
	{
		int r;
		
		printf("do_write called from process %d\n", message->IO_ENDPT);
		
		if (message->COUNT <= 0) {
			r = EINVAL;
		} else {
			/* TODO: Increase per newline, not per write. */
			eightBall_queuedResponses++;
		}
		
		reply(message->m_source, message->IO_ENDPT, r);
	}
	
	/*********************************************************************************
	                                    UNUSED
	*********************************************************************************/
	
	PRIVATE void do_ioctl(message* message)
	{
	}
	
	PRIVATE void do_select(message* message)
	{
	}
	
	PRIVATE void do_cancel(message* message)
	{
	}

## Behavior

With all of the modifications described above, the kernel refuses to boot,
providing the helpfully-reverse-truncated message "vilege structure: -22".

Booting off a working kernel and manually starting `8ball` using `service`,
there are some basic read/write capabilities:

	# alias service8
	alias service8='service up /usr/sbin/8ball -dev /dev/8ball '
	# service8
	# ./a.out
	        Testing read from /dev/8ball...
	        Starting read on file descriptor 11272
	
	# echo 'one' > /dev/8ball
	# ./a.out
	        Testing read from /dev/8ball...
	        Starting read on file descriptor 11272
	        He
	# ./a.out
	        Testing read from /dev/8ball...
	        Starting read on file descriptor 11272
	
	# echo 'one' > /dev/8ball
	# echo 'two' > /dev/8ball
	# ./a.out
	        Testing read from /dev/8ball...
	        Starting read on file descriptor 11272
	        HeHe
	# ./a.out
	        Testing read from /dev/8ball...
	        Starting read on file descriptor 11272
	
	# 

## Other Problems Encountered

Trying to compile gives a `(warning) main is a function; cannot be formal`
unless `<minix/driver.h>` is included.

Trying to use `service up` told me I needed to modify `/etc/system.conf`.
Then, it kept hanging until I added a call to `sef_startup()`.

Attempting to attach the driver to `/dev/8ball` kept failing because there
already was a driver using the major id 18; I had to remove `/dev/8ball` and
recreate it (using `mknod`) with major id 19.  `dev2name` reports that
`/dev/uds` is what's using 18-0.

