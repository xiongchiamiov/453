typedef struct inode_t {
	small mode;               /* File type and rwx bits */
	small numLinks;           /* Directory entries for this file */
	small uid;
	small gid;
	large fileSize;           /* Number of bytes in the file */
	large accessTime;         /* \                                 */
	large modificationTime;   /*  | All in seconds since the epoch */
	large statusChangeTime;   /* /                                 */
	large zones[7];
	large indirectZone;       /* \                                     */
	large doubleIndirectZone; /*  | Used for files larger than 7 zones */
	large tripleIndirectZone; /* /  <- (unused)                        */
} inode;

