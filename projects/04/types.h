/* Type definitions extracted out of Minix. */

#ifndef _TYPES
#define _TYPES

/* These were the types used in Minix, but apparently longs and shorts differ
 * on my Mac from the VirtualBox Minix VM.  So instead we use specific byte
 * sizes implied by T&W Figure 5-35. */
/*typedef unsigned long ino_t;*/
/*typedef unsigned long bit_t;*/
/*typedef unsigned long zone_t;*/
/*typedef unsigned short zone1_t;*/
typedef u_int16_t small;
typedef u_int32_t large;

typedef unsigned char  uint8_t;
typedef unsigned long uint32_t;

#endif

