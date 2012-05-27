#  CPE 453
#  -------------------
#
#  @author James Pearson
CC=gcc

# The _POSIX_* symbols only come into play on systems that are POSIX
# but not SUS.
SUS3=-D_POSIX_SOURCE -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_DARWIN_C_SOURCE
HARDEN=-D_FORTIFY_SOURCE
CFLAGS=-Wall -g -std=c89 -pedantic $(SUS3) $(HARDEN)
LDFLAGS=

ALL=minls

all:	$(ALL)

minls: minls.o
	$(CC) $(LDFLAGS) -o $@ $^

minls.o: minls.c minls.h
	$(CC) $(CFLAGS) -c $<

handin:
	handin pn-cs453 asgn4 README Makefile *.c *.h

longlines:
	~pnico/bin/longlines.pl *.c *.h

clean:
	rm -rf core* *.o *.gch $(ALL)
