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

ALL=malloc.o

all:	$(ALL)

run: build
	./test-malloc

test: run

build: test-malloc

libmalloc.a: malloc.o
	ar r libmalloc.a malloc.o

malloc.o: malloc.c malloc.h
	$(CC) $(CFLAGS) -c $<

test-malloc:	test.o libmalloc.a
	$(CC) $(LDFLAGS) -o $@ $^

test.o: test.c test.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf core* *.o *.gch libmalloc.a test-malloc $(ALL)
