#include <stdio.h>

int main(int argc, char** argv) {
	FILE* in;
	int c;
	int limit;
	
	printf("Testing read from /dev/8ball...\n");

	in = fopen("/dev/8ball", "r");
	if (!in) {
		printf("Error opening /dev/8ball.\n");
		return 1;
	}

	/*
	for (limit=30; (c=fgetc(in)) != EOF && limit != 0; limit--) {
		printf("%c", c);
	}
	*/

	fclose(in);
	return 0;
}

