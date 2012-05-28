/* Gleefully stolen from the assignment specs. */

#ifndef _PARTITION
#define _PARTITION

#define PARTITION_TABLE_OFFSET 0x1BE
#define MAGIC_BYTE_ONE 0x55
#define MAGIC_BYTE_ONE_ADDRESS 510
#define MAGIC_BYTE_TWO 0xAA
#define MAGIC_BYTE_TWO_ADDRESS 511

#include "types.h"

typedef struct partition_t {
	uint8_t  bootind;    /* Boot magic number (0x80 if bootable) */
	uint8_t  start_head; /* Start of partition in CHS */
	uint8_t  start_sec;
	uint8_t  start_cyl;
	uint8_t  type;       /* Type of partition (0x81 is MINIX) */
	uint8_t  end_head;   /* End of partition in CHS */
	uint8_t  end_sec;
	uint8_t  end_cyl;
	uint32_t lFirst;     /* First sector (LBA addressing) */
	uint32_t size;       /* Size of partition (in sectors) */
} partition;

#endif /* _PARTITION */
