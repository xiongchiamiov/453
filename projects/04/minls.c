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
	partition partition;
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
	
	build_partition(&partition, diskImage);
	build_superblock(&superBlock, diskImage);
}

void show_help_and_exit() {
	fprintf(stderr,
	       "Usage: minls [-v] [-p part [-s subpart]] imagefile [path]\n");
	exit(EXIT_FAILURE);
}

void build_partition(partition* partition, FILE* diskImage) {
	uint8_t byte[2];
	fseek(diskImage, MAGIC_BYTE_ONE_ADDRESS, SEEK_SET);
	fread(byte, 1, 1, diskImage);
	printf("First magic byte is %x (%x expected)\n", byte[0], MAGIC_BYTE_ONE);
	fseek(diskImage, MAGIC_BYTE_TWO_ADDRESS, SEEK_SET);
	fread(byte+1, 1, 1, diskImage);
	printf("Second magic byte is %x (%x expected)\n", byte[1], MAGIC_BYTE_TWO);
	
	if (byte[0] != MAGIC_BYTE_ONE
	 || byte[1] != MAGIC_BYTE_TWO) {
		fprintf(stderr, "Not a valid partition table!\n");
		exit(EXIT_FAILURE);
	}
	
	fseek(diskImage, PARTITION_TABLE_OFFSET, SEEK_SET);
	fread(partition, sizeof(partition), 1, diskImage);
	print_partition(partition);
}

void build_superblock(superblock* superBlock, FILE* diskImage) {
	fseek(diskImage, SUPER_BLOCK_OFFSET, SEEK_SET);
	fread(superBlock, sizeof(superblock), 1, diskImage);
	print_superblock(superBlock);
}

/******************************************************************************\
|                           Print Utility Functions                            |
\******************************************************************************/

/**
 * Wouldn't it be nice if C had facilities for printing out structures?
 */
void print_superblock(superblock* superBlock) {
	printf("superblock at %p:\n", superBlock);
	printf("\ts_ninodes:          %16lu", superBlock->s_ninodes);
	printf("  Usable inodes on the minor device\n");
	printf("\ts_nzones:           %16d", superBlock->s_nzones);
	printf("  Total device size, including bit maps etc\n");
	printf("\ts_imap_blocks:      %16d", superBlock->s_imap_blocks);
	printf("  # of blocks used by inode bit map\n");
	printf("\ts_zmap_blocks:      %16d", superBlock->s_zmap_blocks);
	printf("  # of blocks used by zone bit map\n");
	printf("\ts_firstdatazone:    %16d", superBlock->s_firstdatazone);
	printf("  number of first data zone (small)\n");
	printf("\ts_log_zone_size:    %16d", superBlock->s_log_zone_size);
	printf("  log2 of blocks/zone\n");
	printf("\ts_pad:              %16d", superBlock->s_pad);
	printf("  try to avoid compiler-dependent padding\n");
	printf("\ts_max_size:         %16lld", superBlock->s_max_size);
	printf("  maximum file size on this device\n");
	printf("\ts_zones:            %16lu", superBlock->s_zones);
	printf("  number of zones (replaces s_nzones in V2)\n");
	printf("\ts_magic:            %16d", superBlock->s_magic);
	printf("  magic number to recognize super-blocks\n");
	/* V3 and higher filesystems only. */
	printf("\n");
	printf("\ts_pad2:             %16d", superBlock->s_pad2);
	printf("  try to avoid compiler-dependent padding\n");
	printf("\ts_block_size:       %16d", superBlock->s_block_size);
	printf("  block size in bytes\n");
	printf("\ts_disk_version:     %16d", superBlock->s_disk_version);
	printf("  filesystem format sub-version\n");
	/* The following items are only used when the super_block is in memory. */
	printf("\n");
	printf("\ts_isup:             %16p", superBlock->s_isup);
	printf("  inode for root dir of mounted file sys\n");
	printf("\ts_imount:           %16p", superBlock->s_imount);
	printf("  inode mounted on\n");
	printf("\ts_inodes_per_block: %16d", superBlock->s_inodes_per_block);
	printf("  precalculated from magic number\n");
	printf("\ts_dev:              %16d", superBlock->s_dev);
	printf("  whose super block is this?\n");
	printf("\ts_rd_only:          %16d", superBlock->s_rd_only);
	printf("  set to 1 iff file sys mounted read only\n");
	printf("\ts_native:           %16d", superBlock->s_native);
	printf("  set to 1 iff not byte swapped file system\n");
	printf("\ts_version:          %16d", superBlock->s_version);
	printf("  file system version, zero means bad magic\n");
	printf("\ts_ndzones:          %16d", superBlock->s_ndzones);
	printf("  # direct zones in an inode\n");
	printf("\ts_nindirs:          %16d", superBlock->s_nindirs);
	printf("  # indirect zones per indirect block\n");
	printf("\ts_isearch:          %16lu", superBlock->s_isearch);
	printf("  inodes below this bit number are in use\n");
	printf("\ts_zsearch:          %16lu", superBlock->s_zsearch);
	printf("  all zones below this bit number are in use\n");
}

void print_partition(partition* partition) {
	printf("partition at %p:\n", partition);
	printf("\tbootind:    %16x", partition->bootind);
	printf("  Boot magic number (0x80 if bootable)\n");
	printf("\tstart_head: %16d", partition->start_head);
	printf("  \n");
	printf("\tstart_sec:  %16d", partition->start_sec);
	printf("  \n");
	printf("\tstart_cyl:  %16d", partition->start_cyl);
	printf("  \n");
	printf("\ttype:       %16x", partition->type);
	printf("  Type of partition (0x81 is MINIX)\n");
	printf("\tend_head:   %16d", partition->end_head);
	printf("  End of partition in CHS\n");
	printf("\tend_sec:    %16d", partition->end_sec);
	printf("  \n");
	printf("\tend_cyl:    %16d", partition->end_cyl);
	printf("  \n");
	printf("\tlFirst:     %16lu", partition->lFirst);
	printf("  First sector (LBA addressing)\n");
	printf("\tsize:       %16lu", partition->size);
	printf("  Size of partition (in sectors)\n");
}

