/* Gleefully stolen from the assignment specs. */

#ifndef _PARTITION
#define _PARTITION

#define PARTITION_TABLE_OFFSET 0x1BE
#define MAGIC_BYTE_ONE 0x55
#define MAGIC_BYTE_ONE_ADDRESS 510
#define MAGIC_BYTE_TWO 0xAA
#define MAGIC_BYTE_TWO_ADDRESS 511
#define MINIX_FILESYSTEM_TYPE 0x81
#define SECTOR_SIZE 512

#include "types.h"

typedef struct partition_t {
	u_int8_t  bootind;    /* Boot magic number (0x80 if bootable) */
	u_int8_t  start_head; /* Start of partition in CHS */
	u_int8_t  start_sec;
	u_int8_t  start_cyl;
	u_int8_t  type;       /* Type of partition (0x81 is MINIX) */
	u_int8_t  end_head;   /* End of partition in CHS */
	u_int8_t  end_sec;
	u_int8_t  end_cyl;
	u_int32_t lFirst;     /* First sector (LBA addressing) */
	u_int32_t size;       /* Size of partition (in sectors) */
} partition;

#endif /* _PARTITION */
