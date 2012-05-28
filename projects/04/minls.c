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
	FILE* diskImage;
	superblock superBlock;
	
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
	diskImage = fopen(imagefile, "rb");
	if (diskImage == NULL) {
		fprintf(stderr, "Cannot read file %s.\n", imagefile);
		show_help_and_exit();
	}
	
	build_superblock(&superBlock, diskImage);
}

void show_help_and_exit() {
	fprintf(stderr,
	       "Usage: minls [-v] [-p part [-s subpart]] imagefile [path]\n");
	exit(EXIT_FAILURE);
}

void build_superblock(superblock* superBlock, FILE* diskImage) {
	fseek(diskImage, SUPER_BLOCK_OFFSET, SEEK_SET);
	fread(superBlock, sizeof(superblock), 1, diskImage);
	print_superblock(superBlock);
}

/**
 * Wouldn't it be nice if C had facilities for printing out structures?
 */
void print_superblock(superblock* superBlock) {
	printf("superblock at %p:\n", superBlock);
	printf("\ts_ninodes: %lu\n", superBlock->s_ninodes);
	printf("\ts_nzones: %d\n", superBlock->s_nzones);
	printf("\ts_imap_blocks: %d\n", superBlock->s_imap_blocks);
	printf("\ts_zmap_blocks: %d\n", superBlock->s_zmap_blocks);
	printf("\ts_firstdatazone: %d\n", superBlock->s_firstdatazone);
	printf("\ts_log_zone_size: %d\n", superBlock->s_log_zone_size);
	printf("\ts_pad: %d\n", superBlock->s_pad);
	printf("\ts_max_size: %lld\n", superBlock->s_max_size);
	printf("\ts_zones: %lu\n", superBlock->s_zones);
	printf("\ts_magic: %d\n", superBlock->s_magic);
	/* V3 and higher filesystems only. */
	printf("\n");
	printf("\ts_pad2: %d\n", superBlock->s_pad2);
	printf("\ts_block_size: %d\n", superBlock->s_block_size);
	printf("\ts_disk_version: %d\n", superBlock->s_disk_version);
	/* The following items are only used when the super_block is in memory. */
	printf("\n");
	printf("\ts_isup: %p\n", superBlock->s_isup);
	printf("\ts_imount: %p\n", superBlock->s_imount);
	printf("\ts_inodes_per_block: %d\n", superBlock->s_inodes_per_block);
	printf("\ts_dev: %d\n", superBlock->s_dev);
	printf("\ts_rd_only: %d\n", superBlock->s_rd_only);
	printf("\ts_native: %d\n", superBlock->s_native);
	printf("\ts_version: %d\n", superBlock->s_version);
	printf("\ts_ndzones: %d\n", superBlock->s_ndzones);
	printf("\ts_nindirs: %d\n", superBlock->s_nindirs);
	printf("\ts_isearch: %lu\n", superBlock->s_isearch);
	printf("\ts_zsearch: %lu\n", superBlock->s_zsearch);
}

