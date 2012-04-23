#  CPE 453
#  -------------------
#
#  @author James Pearson
CC=gcc

# The _POSIX_* symbols only come into play on systems that are POSIX
# but not SUS.
SUS3=-D_POSIX_SOURCE -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_DARWIN_C_SOURCE
HARDEN=-D_FORTIFY_SOURCE
CFLAGS=-Wall -g -std=c89 -pedantic $(SUS3) $(HARDEN) -m32
LDFLAGS=-m32

ALL=liblwp.a

all:	$(ALL)

liblwp.a: lwp.o
	ar r liblwp.a lwp.o

lwp.o: lwp.c lwp.h
	$(CC) $(CFLAGS) -c $<

numbers-test: numbersmain.o libPLN.a
	$(CC) $(LDFLAGS) -o $@ $^

numbersmain.o: numbersmain.c
	$(CC) $(CFLAGS) -c $<

handin:
	handin pn-cs453 asgn2 README Makefile lwp.c lwp.h

clean:
	rm -rf core* *.o *.gch liblwp.a $(ALL)
