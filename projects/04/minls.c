#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "minls.h"

int main(int argc, char *argv[]) {
	int c,
	    i,
	    currentInode,
	    nonDeletedFiles,
	    numExtraFiles;
	extern char* optarg;
	extern int optopt, optind;
	bool verbose = false;
	char* part = NULL,
	    * subpart = NULL,
	    * imagefile = NULL,
	    * path = NULL,
	    * pathComponent,
	    * pathSeparator = "/";
	FILE* diskImage;
	partition partitionTable[4];
	superblock superBlock;
	inode* inodeList;
	directory* fileList;
	directory file;
	char filename[61];
	
	/* This is modified from Wikipedia's getopt example:
	 * https://en.wikipedia.org/w/index.php?title=Getopt&oldid=489461319 */
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
	if (path[0] != '/') {
		fprintf(stderr, "Path %s is not absolute.\n", path);
		exit(1);
	}
	
	/* (partition) -> sector -> block -> zone */
	if (part) {
		build_partition(partitionTable, diskImage, verbose);
	}
	build_superblock(&superBlock, diskImage, verbose);
	inodeList = build_inode(&superBlock, diskImage, verbose);
	
	printf("%s:\n", path);
	
	/* Do special-case stuff for the root (/) */
	/* Inode 1 is the root */
	currentInode = 1;
	/* . and .. won't be accounted for in the inode's link counter. */
	numExtraFiles = 2;
	/* Get file list */
	fileList = read_zone(inodeList[currentInode - 1].zone0, diskImage,
	                     &superBlock);
	pathComponent = strtok(path, pathSeparator);
	
	/* Walk on down the tree while there are still branches to walk down */
	while (pathComponent != NULL) {
		printf("path is %s\n", pathComponent);
		
		/* Get inode for this part of the path. */
		currentInode = search_for_inode(pathComponent, fileList,
		                                inodeList[currentInode - 1].numLinks + 2);
		if (currentInode == 0) {
			fprintf(stderr, "File %s not found!\n", path);
			exit(1);
		}
		
		/* Is this a directory? */
		if (inodeList[currentInode - 1].mode & 040000) {
			printf("%s is a directory\n", pathComponent);
			/* (Yes) Get list of contained files */
			fileList = read_zone(inodeList[currentInode - 1].zone0, diskImage,
			                     &superBlock);
			/* . and .. won't be accounted for in the inode's link counter. */
			numExtraFiles = 2;
		} else {
			printf("%s is a file\n", pathComponent);
			/* (No) Filter down filelist to just this file */
			fileList = filter_file_list(fileList, pathComponent);
			/* We don't care about . or .. for a file. */
			numExtraFiles = 0;
		}
		/* Get the next branch of the path-tree, if there is one. */
		pathComponent = strtok(NULL, pathSeparator);
	}
	
	/* Print out all of our files.  Since we don't actually know how long the
	 * list is, rely upon the inode's recorded number of links. */
	for (i = 0, nonDeletedFiles = 0;
	     nonDeletedFiles < inodeList[currentInode - 1].numLinks + numExtraFiles;
	     i++) {
		file = fileList[i];
		
		/* Skip deleted files. */
		if (file.inode == 0) {
			if (verbose) {
				fprintf(stderr, "File %s deleted.\n", file.name);
			}
			continue;
		}
		nonDeletedFiles++;
		
		/* If the name is 60 characters, it's not null-terminated.  So, make
		 * sure there's a null at the end. */
		strncpy(filename, (char *)(file.name), 60);
		filename[61] = '\0';
		
		if (verbose) {
			fprintf(stderr, "File with name: %s\n", filename);
			print_inode(inodeList + file.inode - 1);
		}
		 
		printf("%s %9d %s\n",
		       /* inodes are 1-indexed */
		       generate_permission_string(inodeList[file.inode - 1].mode),
		       inodeList[file.inode - 1].fileSize,
		       filename);
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

directory* read_zone(int zone, FILE* diskImage, superblock* superBlock) {
	int zonesize;
	directory* fileList;
	
	zonesize = superBlock->s_block_size << superBlock->s_log_zone_size;
	fseek(diskImage, zone * zonesize, SEEK_SET);
	fileList = malloc(zonesize);
	fread(fileList, zonesize, 1, diskImage);
	
	return fileList;
}

/**
 * Find the directory listing that has name `pathComponent` and return its
 * corresponding inode number.
 */
int search_for_inode(char* pathComponent, directory* fileList, int limit) {
	int i,
	    nonDeletedFiles;
	
	for (i = 0, nonDeletedFiles = 0;
	     nonDeletedFiles < limit;
	     i++) {
		/* Skip deleted files. */
		if (fileList[i].inode == 0) {
			continue;
		}
		nonDeletedFiles++;
		
		/* Did we find a match? */
		if (strcmp((char *)(fileList[i].name), pathComponent) == 0) {
			return fileList[i].inode;
		}
	}
	return 0;
}

/**
 * Cut down `fileList` to only the file that has name `name`.  Unspecified
 * behavior if `name` is not found.  Frees `fileList` when done and returns a
 * pointer to a newly-allocated `directory`.
 */
directory* filter_file_list(directory* fileList, char* name) {
	int i,
	    j;
	directory* file;
	
	file = malloc(sizeof(directory));
	
	for (i = 0; ; i++) {
		if (strcmp(name, (char *)(fileList[i].name)) == 0) {
			file->inode = fileList[i].inode;
			/* Copy name. */
			for (j = 0; j < 60; j++) {
				file->name[j] = fileList[i].name[j];
			}
			break;
		}
	}
	
	free(fileList);
	return file;
}

char* generate_permission_string(small mode) {
	char* permissions;
	
	permissions = malloc(10 * sizeof(char));
	strcpy(permissions, "----------");
	/*printf("Generating permission string from mode %07o...\n", mode);*/
	
	if (mode & 040000) {
		permissions[0] = 'd';
	}
	if (mode & 0400) {
		permissions[1] = 'r';
	}
	if (mode & 0200) {
		permissions[2] = 'w';
	}
	if (mode & 0100) {
		permissions[3] = 'x';
	}
	if (mode & 0040) {
		permissions[4] = 'r';
	}
	if (mode & 0020) {
		permissions[5] = 'w';
	}
	if (mode & 0010) {
		permissions[6] = 'x';
	}
	if (mode & 0004) {
		permissions[7] = 'r';
	}
	if (mode & 0002) {
		permissions[8] = 'w';
	}
	if (mode & 0001) {
		permissions[9] = 'x';
	}
	
	return permissions;
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

