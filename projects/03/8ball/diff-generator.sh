#!/bin/sh

diff() {
	/usr/bin/diff -u $1.orig $1
}

diff /usr/src/drivers/Makefile > diffs/drivers_Makefile.diff
diff /etc/system.conf > diffs/system.conf.diff
diff /etc/rc > diffs/rc.diff

