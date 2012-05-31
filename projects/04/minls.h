#ifndef _MINLS
#define _MINLS

#include "min.h"

void show_help_and_exit();
directory* read_zone(int zone, FILE* diskImage, superblock* superBlock);
char* generate_permission_string(small mode);

#endif

