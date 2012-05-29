#ifndef _MINLS
#define _MINLS

#include "types.h"
#include "super.h"
#include "partition.h"
#include "inode.h"

/* If we were using C99, we could use stdbool.h.  But we aren't.
 * The lowercased 'true' and 'false' are used because my setup of Vim has them
 * already specially-colored. */
typedef int bool;
#define true 1
#define false 0

void show_help_and_exit();
void build_superblock(superblock* superBlock, FILE* diskImage, bool verbose);
void build_partition(partition* partition, FILE* diskImage, bool verbose);
void print_superblock(superblock* superBlock);
void print_partition(partition* partition);
void print_inode(inode* inode);

#endif

