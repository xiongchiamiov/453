#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <alloca.h>

int main(int argc, char** argv) {
	FILE* in;
	int c;
	int limit;
	char* string = malloc(10);
	
	printf("\tTesting read from /dev/8ball...\n");

	in = fopen("/dev/8ball", "r");
	if (!in) {
		printf("\tError opening /dev/8ball.\n");
		return 1;
	}
	
	printf("\tStarting read on file descriptor %d\n", in);

	/*
	for (limit=1; (c=fgetc(in)) != EOF && limit != 0; limit--) {
		printf("\t%c (%d)\n", c, c);
	}
	if (limit == 0) {
		printf("\tAborted due to limit reached.\n");
	}
	*/
	printf("\t%s\n", fgets(string, 10, in));

	fclose(in);
	return 0;
}

