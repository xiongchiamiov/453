#include <stdio.h>

int main(int argc, char** argv) {
	FILE* in;
	int c;
	int limit;
	
	printf("\tTesting read from /dev/8ball...\n");

	in = fopen("/dev/8ball", "r");
	if (!in) {
		printf("\tError opening /dev/8ball.\n");
		return 1;
	}
	
	printf("\tStarting read on file descriptor %d\n", in);

	for (limit=30; (c=fgetc(in)) != EOF && limit != 0; limit--) {
		printf("\t(%c)\n", c);
	}

	fclose(in);
	return 0;
}

