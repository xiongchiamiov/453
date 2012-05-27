#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "minls.h"

int main(int argc, char *argv[]) {
	/* This is modified from Wikipedia's getopt example:
	 * https://en.wikipedia.org/w/index.php?title=Getopt&oldid=489461319 */
	int c;
	extern char* optarg;
	extern int optopt, optind;
	bool verbose = false;
	char* part,
	    * subpart,
	    * imagefile,
	    * path;
	
	while ((c = getopt(argc, argv, "vp:s:h")) != -1) {
		switch (c) {
			case 'v':
				printf("Verbose mode.\n");
				verbose = true;
				break;
			case 'p':
				printf("Part %s selected.\n", optarg);
				part = optarg;
				break;
			case 's':
				printf("Subpart %s selected.\n", optarg);
				subpart = optarg;
				break;
			case 'h':
				show_help_and_exit();
			case '?':
				fprintf(stderr, "Unknown arg %c\n", optopt);
				show_help_and_exit();
				break;
		}
	}
	
	/* Check for our imagefile and optional path. */
	if ((argc - optind) == 1) {
		imagefile = argv[optind];
		path = "/";
	} else if ((argc - optind) == 2) {
		imagefile = argv[optind];
		path = argv[optind + 1];
	} else {
		fprintf(stderr, "Wrong number of positional arguments.\n");
		show_help_and_exit();
	}
	
	printf("Imagefile: %s\n", imagefile);
	printf("Path:      %s\n", path);
}

void show_help_and_exit() {
	fprintf(stderr,
	       "Usage: minls [-v] [-p part [-s subpart]] imagefile [path]\n");
	exit(EXIT_FAILURE);
}

