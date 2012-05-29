#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "minls.h"

int main(int argc, char *argv[]) {
	/* This is modified from Wikipedia's getopt example:
	 * https://en.wikipedia.org/w/index.php?title=Getopt&oldid=489461319 */
	int c,
	    i;
	extern char* optarg;
	extern int optopt, optind;
	bool verbose = false;
	char* part = NULL,
	    * subpart = NULL,
	    * imagefile = NULL,
	    * path = NULL;
	FILE* diskImage;
	partition partitionTable[4];
	superblock superBlock;
	inode* inodeList;
	directory* fileList;
	
	while ((c = getopt(argc, argv, "vp:s:h")) != -1) {
		switch (c) {
			case 'v':
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
	
	if (verbose) {
		printf("Imagefile: %s\n", imagefile);
		printf("Path:      %s\n", path);
	}
	diskImage = fopen(imagefile, "rb");
	if (diskImage == NULL) {
		fprintf(stderr, "Cannot read file %s.\n", imagefile);
		show_help_and_exit();
	}
	
	/* (partition) -> sector -> block -> zone */
	if (part) {
		build_partition(partitionTable, diskImage, verbose);
	}
	build_superblock(&superBlock, diskImage, verbose);
	inodeList = build_inode(&superBlock, diskImage, verbose);
	
	fileList = read_zone(inodeList[0].zone0, diskImage, inodeList[0].numLinks,
	                     &superBlock);
	if (verbose) {
		/* Go two larger to account for . and .. */
		for (i = 0; i < inodeList[0].numLinks + 2; i++) {
			fprintf(stderr, "File with name: %s\n", fileList[i].name);
			print_inode(inodeList + fileList[i].inode);
		}
	}
}

void show_help_and_exit() {
	fprintf(stderr,
	       "Usage: minls [-v] [-p part [-s subpart]] imagefile [path]\n");
	exit(EXIT_FAILURE);
}

void build_partition(partition partitionTable[], FILE* diskImage, bool verbose) {
	int i;
	uint8_t byte[2];
	
	fseek(diskImage, MAGIC_BYTE_ONE_ADDRESS, SEEK_SET);
	fread(byte, 1, 1, diskImage);
	printf("First magic byte is %x (%x expected)\n", byte[0], MAGIC_BYTE_ONE);
	fseek(diskImage, MAGIC_BYTE_TWO_ADDRESS, SEEK_SET);
	fread(byte+1, 1, 1, diskImage);
	printf("Second magic byte is %x (%x expected)\n", byte[1], MAGIC_BYTE_TWO);
	
	if (verbose
	 && (byte[0] != MAGIC_BYTE_ONE
	  || byte[1] != MAGIC_BYTE_TWO)) {
		fprintf(stderr, "Not a valid partition table!\n");
		exit(EXIT_FAILURE);
	}
	
	fseek(diskImage, PARTITION_TABLE_OFFSET, SEEK_SET);
	fread(partitionTable, sizeof(partition), 4, diskImage);
	if (verbose) {
		for (i = 0; i < 4; i++) {
			print_partition(partitionTable+i);
		}
	}
}

void build_superblock(superblock* superBlock, FILE* diskImage, bool verbose) {
	fseek(diskImage, SUPER_BLOCK_OFFSET, SEEK_SET);
	fread(superBlock, sizeof(superblock), 1, diskImage);
	if (verbose) {
		print_superblock(superBlock);
	}
}

inode* build_inode(superblock* superBlock, FILE* diskImage, bool verbose) {
	int inodeOffset,
	    i;
	inode* inodeList;
	
	inodeList = malloc(sizeof(inode) * superBlock->s_ninodes);
	inodeOffset = (2 + superBlock->s_imap_blocks + superBlock->s_zmap_blocks)
	            * superBlock->s_block_size;
	fseek(diskImage, inodeOffset, SEEK_SET);
	fread(inodeList, sizeof(inode) * superBlock->s_ninodes,
	      superBlock->s_ninodes, diskImage);
	if (verbose) {
		/*
		for (i = 0; i < superBlock->s_ninodes; i++) {
			print_inode(inodeList+i);
		}
		*/
		/* Nevermind, just print the root inode. */
		print_inode(inodeList);
	}
	
	return inodeList;
}

directory* read_zone(int zone, FILE* diskImage, int numFiles,
                     superblock* superBlock) {
	int zonesize;
	directory* fileList;
	
	/* We'll also be reading in . and .. */
	numFiles += 2;
	
	zonesize = superBlock->s_block_size << superBlock->s_log_zone_size;
	fseek(diskImage, zone * zonesize, SEEK_SET);
	fileList = malloc(numFiles * sizeof(directory));
	fread(fileList, sizeof(directory), numFiles, diskImage);
	
	return fileList;
}

/******************************************************************************\
|                           Print Utility Functions                            |
\******************************************************************************/

/**
 * Wouldn't it be nice if C had facilities for printing out structures?
 */
void print_superblock(superblock* superBlock) {
	fprintf(stderr, "superblock at %p:\n", superBlock);
	fprintf(stderr, "\ts_ninodes:          %16u", superBlock->s_ninodes);
	fprintf(stderr, "  Usable inodes on the minor device\n");
	fprintf(stderr, "\ts_nzones:           %16d", superBlock->s_nzones);
	fprintf(stderr, "  Total device size, including bit maps etc\n");
	fprintf(stderr, "\ts_imap_blocks:      %16d", superBlock->s_imap_blocks);
	fprintf(stderr, "  # of blocks used by inode bit map\n");
	fprintf(stderr, "\ts_zmap_blocks:      %16d", superBlock->s_zmap_blocks);
	fprintf(stderr, "  # of blocks used by zone bit map\n");
	fprintf(stderr, "\ts_firstdatazone:    %16d", superBlock->s_firstdatazone);
	fprintf(stderr, "  number of first data zone (small)\n");
	fprintf(stderr, "\ts_log_zone_size:    %16d", superBlock->s_log_zone_size);
	fprintf(stderr, "  log2 of blocks/zone\n");
	fprintf(stderr, "\ts_pad:              %16d", superBlock->s_pad);
	fprintf(stderr, "  try to avoid compiler-dependent padding\n");
	fprintf(stderr, "\ts_max_size:         %16u", superBlock->s_max_size);
	fprintf(stderr, "  maximum file size on this device\n");
	fprintf(stderr, "\ts_zones:            %16u", superBlock->s_zones);
	fprintf(stderr, "  number of zones (replaces s_nzones in V2)\n");
	fprintf(stderr, "\ts_magic:            %16x", superBlock->s_magic);
	fprintf(stderr, "  magic number to recognize super-blocks (%x expected)\n",
	        MINIX_MAGIC);
	/* V3 and higher filesystems only. */
	fprintf(stderr, "\n");
	fprintf(stderr, "\ts_pad2:             %16d", superBlock->s_pad2);
	fprintf(stderr, "  try to avoid compiler-dependent padding\n");
	fprintf(stderr, "\ts_block_size:       %16d", superBlock->s_block_size);
	fprintf(stderr, "  block size in bytes\n");
	fprintf(stderr, "\ts_disk_version:     %16d", superBlock->s_disk_version);
	fprintf(stderr, "  filesystem format sub-version\n");
	/* The following items are only used when the super_block is in memory. */
	fprintf(stderr, "\n");
	fprintf(stderr, "\ts_isup:             %16p", superBlock->s_isup);
	fprintf(stderr, "  inode for root dir of mounted file sys\n");
	fprintf(stderr, "\ts_imount:           %16p", superBlock->s_imount);
	fprintf(stderr, "  inode mounted on\n");
	fprintf(stderr, "\ts_inodes_per_block: %16d", superBlock->s_inodes_per_block);
	fprintf(stderr, "  precalculated from magic number\n");
	fprintf(stderr, "\ts_dev:              %16d", superBlock->s_dev);
	fprintf(stderr, "  whose super block is this?\n");
	fprintf(stderr, "\ts_rd_only:          %16d", superBlock->s_rd_only);
	fprintf(stderr, "  set to 1 iff file sys mounted read only\n");
	fprintf(stderr, "\ts_native:           %16d", superBlock->s_native);
	fprintf(stderr, "  set to 1 iff not byte swapped file system\n");
	fprintf(stderr, "\ts_version:          %16d", superBlock->s_version);
	fprintf(stderr, "  file system version, zero means bad magic\n");
	fprintf(stderr, "\ts_ndzones:          %16d", superBlock->s_ndzones);
	fprintf(stderr, "  # direct zones in an inode\n");
	fprintf(stderr, "\ts_nindirs:          %16d", superBlock->s_nindirs);
	fprintf(stderr, "  # indirect zones per indirect block\n");
	fprintf(stderr, "\ts_isearch:          %16u", superBlock->s_isearch);
	fprintf(stderr, "  inodes below this bit number are in use\n");
	fprintf(stderr, "\ts_zsearch:          %16u", superBlock->s_zsearch);
	fprintf(stderr, "  all zones below this bit number are in use\n");
}

void print_partition(partition* partition) {
	fprintf(stderr, "partition at %p:\n", partition);
	fprintf(stderr, "\tbootind:    %16x", partition->bootind);
	fprintf(stderr, "  Boot magic number (0x80 if bootable)\n");
	fprintf(stderr, "\tstart_head: %16u", partition->start_head);
	fprintf(stderr, "  \n");
	fprintf(stderr, "\tstart_sec:  %16u", partition->start_sec);
	fprintf(stderr, "  \n");
	fprintf(stderr, "\tstart_cyl:  %16u", partition->start_cyl);
	fprintf(stderr, "  \n");
	fprintf(stderr, "\ttype:       %16x", partition->type);
	fprintf(stderr, "  Type of partition (0x81 is MINIX)\n");
	fprintf(stderr, "\tend_head:   %16u", partition->end_head);
	fprintf(stderr, "  End of partition in CHS\n");
	fprintf(stderr, "\tend_sec:    %16u", partition->end_sec);
	fprintf(stderr, "  \n");
	fprintf(stderr, "\tend_cyl:    %16u", partition->end_cyl);
	fprintf(stderr, "  \n");
	fprintf(stderr, "\tlFirst:     %16lu", partition->lFirst);
	fprintf(stderr, "  First sector (LBA addressing)\n");
	fprintf(stderr, "\tsize:       %16lu", partition->size);
	fprintf(stderr, "  Size of partition (in sectors)\n");
}

void print_inode(inode* inode) {
	fprintf(stderr, "inode at %p:\n", inode);
	fprintf(stderr, "\tmode:               %16x", inode->mode);
	fprintf(stderr, "  File type and rwx bits\n");
	fprintf(stderr, "\tnumLinks:           %16u", inode->numLinks);
	fprintf(stderr, "  \n");
	fprintf(stderr, "\tuid:                %16u", inode->uid);
	fprintf(stderr, "  \n");
	fprintf(stderr, "\tgid:                %16u", inode->gid);
	fprintf(stderr, "  \n");
	fprintf(stderr, "\tfileSize:           %16u", inode->fileSize);
	fprintf(stderr, "  Number of bytes in the file\n");
	fprintf(stderr, "\taccessTime:         %16u", inode->accessTime);
	fprintf(stderr, "  \\\n");
	fprintf(stderr, "\tmodificationTime:   %16u", inode->modificationTime);
	fprintf(stderr, "   | All in seconds since the epoch\n");
	fprintf(stderr, "\tstatusChangeTime:   %16u", inode->statusChangeTime);
	fprintf(stderr, "  /\n");
	fprintf(stderr, "\tzone0:              %16u", inode->zone0);
	fprintf(stderr, "  \n");
	fprintf(stderr, "\tzone1:              %16u", inode->zone1);
	fprintf(stderr, "  \n");
	fprintf(stderr, "\tzone2:              %16u", inode->zone2);
	fprintf(stderr, "  \n");
	fprintf(stderr, "\tzone3:              %16u", inode->zone3);
	fprintf(stderr, "  \n");
	fprintf(stderr, "\tzone4:              %16u", inode->zone4);
	fprintf(stderr, "  \n");
	fprintf(stderr, "\tzone5:              %16u", inode->zone5);
	fprintf(stderr, "  \n");
	fprintf(stderr, "\tzone6:              %16u", inode->zone6);
	fprintf(stderr, "  \n");
	fprintf(stderr, "\tindirectZone:       %16u", inode->indirectZone);
	fprintf(stderr, "  \\\n");
	fprintf(stderr, "\tdoubleIndirectZone: %16u", inode->doubleIndirectZone);
	fprintf(stderr, "   | Used for files larger than 7 zones\n");
	fprintf(stderr, "\ttripleIndirectZone: %16u", inode->tripleIndirectZone);
	fprintf(stderr, "  / <- (unused)\n");
}

