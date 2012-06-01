/* This file is modified from the version in Minix 3.1.8. */

#ifndef __MFS_SUPER_H__
#define __MFS_SUPER_H__

#define SUPER_BLOCK_OFFSET 1024
#define MINIX_MAGIC 0x4D5A

/* Super block table.  The root file system and every mounted file system
 * has an entry here.  The entry holds information about the sizes of the bit
 * maps and inodes.  The s_ninodes field gives the number of inodes available
 * for files and directories, including the root directory.  Inode 0 is 
 * on the disk, but not used.  Thus s_ninodes = 4 means that 5 bits will be
 * used in the bit map, bit 0, which is always 1 and not used, and bits 1-4
 * for files and directories.  The disk layout is:
 *
 *    Item        # blocks
 *    boot block      1
 *    super block     1    (offset 1kB)
 *    inode map     s_imap_blocks
 *    zone map      s_zmap_blocks
 *    inodes        (s_ninodes + 'inodes per block' - 1)/'inodes per block'
 *    unused        whatever is needed to fill out the current zone
 *    data zones    (s_zones - s_firstdatazone) << s_log_zone_size
 *
 * A super_block slot is free if s_dev == NO_DEV. 
 */

typedef struct super_block {
  large s_ninodes;        /* # usable inodes on the minor device */
  small  s_nzones;        /* total device size, including bit maps etc */
  small s_imap_blocks;        /* # of blocks used by inode bit map */
  small s_zmap_blocks;        /* # of blocks used by zone bit map */
  small s_firstdatazone;    /* number of first data zone (small) */
  small s_log_zone_size;    /* log2 of blocks/zone */
  small s_pad;            /* try to avoid compiler-dependent padding */
  large s_max_size;    /* maximum file size on this device */
  large s_zones;        /* number of zones (replaces s_nzones in V2) */
  small s_magic;        /* magic number to recognize super-blocks */

  /* The following items are valid on disk only for V3 and above */

  /* The block size in bytes. Minimum MIN_BLOCK SIZE. SECTOR_SIZE
   * multiple. If V1 or V2 filesystem, this should be
   * initialised to STATIC_BLOCK_SIZE.
   */
  small s_pad2;            /* try to avoid compiler-dependent padding */
  small s_block_size;    /* block size in bytes. */
  small s_disk_version;        /* filesystem format sub-version */

  /* The following items are only used when the super_block is in memory. */
  
  struct inode *s_isup;    /* inode for root dir of mounted file sys */
  struct inode *s_imount;   /* inode mounted on */
  small s_inodes_per_block;    /* precalculated from magic number */
  small s_dev;    /* whose super block is this? */
  small s_rd_only;        /* set to 1 iff file sys mounted read only */
  small s_native;            /* set to 1 iff not byte swapped file system */
  small s_version;        /* file system version, zero means bad magic */
  small s_ndzones;        /* # direct zones in an inode */
  small s_nindirs;        /* # indirect zones per indirect block */
  small s_isearch;        /* inodes below this bit number are in use */
  small s_zsearch;        /* all zones below this bit number are in use*/
} superblock;

#define IMAP        0    /* operating on the inode bit map */
#define ZMAP        1    /* operating on the zone bit map */

#endif

