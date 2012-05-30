#ifndef _MINLS
#define _MINLS

#include "types.h"
#include "super.h"
#include "partition.h"
#include "inode.h"
#include "directory.h"

/* If we were using C99, we could use stdbool.h.  But we aren't.
 * The lowercased 'true' and 'false' are used because my setup of Vim has them
 * already specially-colored. */
typedef int bool;
#define true 1
#define false 0

void show_help_and_exit();
void build_superblock(superblock* superBlock, FILE* diskImage, bool verbose);
void build_partition(partition* partition, FILE* diskImage, bool verbose);
inode* build_inode(superblock* superBlock, FILE* diskImage, bool verbose);
directory* read_zone(int zone, FILE* diskImage, superblock* superBlock);
char* generate_permission_string(small mode);
void print_superblock(superblock* superBlock);
void print_partition(partition* partition);
void print_inode(inode* inode);

#endif

