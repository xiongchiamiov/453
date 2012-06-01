/**
 * `hello` prints out the classic hello-world greeting.  This is the entire
 * program.  Yep, right here.
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	int r;
	
	r = printf("Hello, world!\n");
	if (r < 0) {
		fprintf(stderr, "printf failed with status %d!\n", r);
		exit(EXIT_FAILURE);
	}
	exit(0);
}

