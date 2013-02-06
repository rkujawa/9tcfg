/* Ninetails configuration registers */
#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#define CFG_ADDRESS		0xBE0000
#define CFG_R0_OFFSET		0x0
#define CFG_R1_OFFSET		0x4
#define CFG_R2_OFFSET		0x8
#define CFG_R3_OFFSET		0xC

#define CFG_R0_68KMODE		0x80	/* 1xxxxxxx - disable 68020 at next reset */
#define CFG_R0_PCMCIA		0x40	/* x1xxxxxx - PCMCIA mode */
#define CFG_R0_68KMEMORYMODE	0x20	/* xx1xxxxx - 68000 + 5.5MB fast expansion */

#define CFG_R1_MAPROM		0x80	/* 1xxxxxxx - MAPROM at next reset */
#define CFG_R1_SHADOWROM	0x40	/* x1xxxxxx - Shadow ROM now */
#define CFG_R1_INSTCACHEOFF	0x20	/* xx1xxxxx - instruction cache off now */
#define CFG_R1_INSTCACHERESET	0x10	/* xxx1xxxx - instruction cache off after reset */

#define CFG_R2_68KMODE_STATUS	0x80	/* 1xxxxxxx - 68020 is disabled */
#define CFG_R2_MAPROM_STATUS	0x40	/* x1xxxxxx - MAPROM is enabled */
#define CFG_R2_UNLCKBIT0_STATUS 0x20
#define CFG_R2_UNLCKBIT1_STATUS 0x10

#define CFG_R3_MAPROMBANKBIT0	0x80
#define CFG_R3_MAPROMBANKBIT1	0x40
#define CFG_R3_MAPROMBANKON	0x20

#define MAPROM_BANK_ADDRESS	0xB80000

#define ADDMEM_0_BASE		0xA80000
#define ADDMEM_1_BASE		0xF00000
#define ADDMEM_PRI		0

#define S256K			256*1024
#define S512K			512*1024

#endif /* _HARDWARE_H_ */

