#ifndef _MINGET
#define _MINGET

#include "min.h"

void show_help_and_exit();
void copy_file_and_exit(inode* inode, FILE* diskImage, FILE* destination,
                        superblock* superBlock);

#endif

