#include <string.h>

#include "minget.h"

int main(int argc, char* argv[]) {
    int optind,
        part = -1,
        subpart = -1,
        currentInode;
    unsigned long offset;
    bool verbose = false;
    char* imagefile = NULL,
        * path = NULL,
        * pathComponent,
        * pathSeparator = "/";
    FILE* destination,
        * diskImage;
    partition partitionTable[4];
    superblock superBlock;
    inode* inodeList;
    directory* fileList;
    
    optind = parse_flagged_arguments(argc, argv, &verbose, &part, &subpart);
    if (optind == -1) {
        show_help_and_exit();
    }
    
    /* Check for our imagefile, path, and optional destination path. */
    if ((argc - optind) == 2) {
        imagefile = argv[optind];
        path = argv[optind + 1];
        destination = stdout;
    } else if ((argc - optind) == 3) {
        imagefile = argv[optind];
        path = argv[optind + 1];
        destination = fopen(argv[optind + 2], "w+");
        if (destination == NULL) {
            fprintf(stderr, "Cannot write to file %s.\n", argv[optind + 2]);
            show_help_and_exit();
        }
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
    /* Actually, we want to put relative paths relative to /, automatically.
    if (path[0] != '/') {
        fprintf(stderr, "Path %s is not absolute.\n", path);
        exit(1);
    }
    */
    
    /* (partition) -> sector -> block -> zone */
    offset = fetch_partition_offset(part, diskImage, verbose);
    build_superblock(&superBlock, diskImage, offset, verbose);
    inodeList = build_inode(&superBlock, diskImage, offset, verbose);
    
    /*printf("%s:\n", path);*/
    
    /* Do special-case stuff for the root (/) */
    /* Inode 1 is the root */
    currentInode = 1;
    /* Get file list */
    fileList = read_zone(inodeList[currentInode - 1].zones[0], diskImage,
                         offset, &superBlock);
    pathComponent = strtok(path, pathSeparator);
    
    /* Walk on down the tree while there are still branches to walk down */
    while (pathComponent != NULL) {
        /*printf("path is %s\n", pathComponent);*/
        
        /* Get inode for this part of the path. */
        currentInode = search_for_inode(pathComponent, fileList,
                                        inodeList[currentInode-1].numLinks + 2);
        if (currentInode == 0) {
            fprintf(stderr, "File %s not found!\n", path);
            exit(1);
        }
        
        /* Is this a directory? */
        if (inodeList[currentInode - 1].mode & 040000) {
            /*printf("%s is a directory\n", pathComponent);*/
            /* (Yes) Get list of contained files */
            fileList = read_zone(inodeList[currentInode - 1].zones[0],
                                 diskImage, offset, &superBlock);
        } else {
            /*printf("%s is a file\n", pathComponent);*/
            /* (No) Copy the sucker over */
            copy_file_and_exit(inodeList + currentInode - 1, diskImage,
                               offset, destination, &superBlock);
        }
        /* Get the next branch of the path-tree, if there is one. */
        pathComponent = strtok(NULL, pathSeparator);
    }
    
    return 0;
}

void show_help_and_exit() {
    fprintf(stderr, "Usage: minget [-v] [-p part [-s subpart]] imagefile ");
    fprintf(stderr, "srcpath [dstpath]\n");
    exit(EXIT_FAILURE);
}

void copy_file_and_exit(inode* inode, FILE* diskImage, unsigned long offset,
                        FILE* destination, superblock* superBlock) {
    int zonesize,
        zone,
        i;
    void* buffer;
    FILE* zeros;
    
    zonesize = superBlock->s_block_size << superBlock->s_log_zone_size;
    buffer = malloc(zonesize);
    zeros = fopen("/dev/zero", "rb");
    if (zeros == NULL) {
        fprintf(stderr, "Uh oh, I can't read /dev/zero!\n");
        exit(1);
    }
    
    for (i = 0; i < 7; i++) {
        zone = inode->zones[i];
        /* Zone 0 indicates that we should just pretend it's all zeros. */
        if (zone == 0) {
            fread(buffer, zonesize, 1, zeros);
        } else {
            fseek(diskImage, (zone * zonesize) + offset, SEEK_SET);
            fread(buffer, zonesize, 1, diskImage);
        }
        fwrite(buffer, zonesize, 1, destination);
    }
}

