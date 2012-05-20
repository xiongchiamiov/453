#!/bin/sh

# This uses a re-aliased version of diff that compares the
# given filename to its .orig counterpart.
diff /usr/src/drivers/Makefile > diff/drivers_Makefile.diff
diff /etc/system.conf > diff/system.conf.diff
diff /etc/rc > diff/rc.diff

