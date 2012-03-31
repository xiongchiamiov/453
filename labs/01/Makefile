#  CPE 453
#  -------------------
#
#  @author James Pearson
CC=gcc

# The _POSIX_* symbols only come into play on systems that are POSIX
# but not SUS.
SUS3=-D_POSIX_SOURCE -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600
HARDEN=-D_FORTIFY_SOURCE
CFLAGS=-Wall -g -std=c99 -pedantic $(SUS3) $(HARDEN)
LDFLAGS=

ALL=pipeit

all:	$(ALL)

pipeit:	pipeit.o
	$(CC) $(LDFLAGS) -o $@ $^

pipeit.o: pipeit.c pipeit.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf core* *.o *.gch $(ALL)
