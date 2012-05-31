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

directory* read_zone(int zone, FILE* diskImage, superblock* superBlock) {
	int zonesize;
	directory* fileList;
	
	zonesize = superBlock->s_block_size << superBlock->s_log_zone_size;
	fseek(diskImage, zone * zonesize, SEEK_SET);
	fileList = malloc(zonesize);
	fread(fileList, zonesize, 1, diskImage);
	
	return fileList;
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

