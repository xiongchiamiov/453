typedef struct inode_t {
	small mode;               /* File type and rwx bits */
	small numLinks;           /* Directory entries for this file */
	small uid;
	small gid;
	large fileSize;           /* Number of bytes in the file */
	large accessTime;         /* \                                 */
	large modificationTime;   /*  | All in seconds since the epoch */
	large statusChangeTime;   /* /                                 */
	large zone0;
	large zone1;
	large zone2;
	large zone3;
	large zone4;
	large zone5;
	large zone6;
	large indirectZone;       /* \                                     */
	large doubleIndirectZone; /*  | Used for files larger than 7 zones */
	large tripleIndirectZone; /* /  <- (unused)                        */
} inode;

