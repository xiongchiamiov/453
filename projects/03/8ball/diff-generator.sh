#!/bin/sh

diff() {
	/usr/bin/diff -u $1.orig $1
}

diff /usr/src/drivers/Makefile > diffs/drivers_Makefile.diff
diff /etc/system.conf > diffs/system.conf.diff
diff /etc/rc > diffs/rc.diff
diff /usr/src/kernel/table.c > diffs/table.c.diff
diff /usr/src/include/minix/com.h > diffs/com.h.diff
diff /usr/src/include/minix/types.h > diffs/types.h

