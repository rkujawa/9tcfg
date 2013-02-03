#include <stdio.h>
#include <stdlib.h>

#include <exec/types.h>

#include "config.h"
#include "rom.h"
#include "file.h"

/*
void bank_select(uint8_t banknum);
uint8_t bank_bits2num(uint8_t r1);
void bank_copy(uint32_t address);
*/
/*
void shadow_activate(void);
*/
/*
uint8_t
bank_bits2num(uint8_t r1)
{
	uint8_t num;

	num = ((r1 & CFG_R1_BANKBIT0) >> 5) | ((r1 & CFG_R1_BANKBIT1) >> 3);

	return num;
}

void
bank_select(uint8_t banknum)
{
#ifdef DEBUG
	printf("DEBUG: changing to bank %d\n", (int) banknum);
#endif 

	// XXX: this could be implemented with one set/unset operation 

	if (banknum & 0x1)
		cfgreg_set(CFG_R1_OFFSET, CFG_R1_BANKBIT0);
	else
		cfgreg_unset(CFG_R1_OFFSET, CFG_R1_BANKBIT0);

	if (banknum & 0x2)
		cfgreg_set(CFG_R1_OFFSET, CFG_R1_BANKBIT1);
	else
		cfgreg_unset(CFG_R1_OFFSET, CFG_R1_BANKBIT1);
		
}

void
bank_copy(uint32_t address)
{
#ifdef DEBUG
	printf("DEBUG: copying 256kB block from %x to %x\n", address, MAPROM_BANK_ADDRESS);
#endif 

	memcpy((void*) MAPROM_BANK_ADDRESS, (void*) address, 256*1024);	
}
*/
/* copy rom over itself */
void
shadow_activate_self(void)
{
	
	uint8_t r1, r2;

	r1 = cfgreg_read(CFG_R1_OFFSET);
	r2 = cfgreg_read(CFG_R2_OFFSET);

	if ( (r1 & CFG_R1_MAPROM) || (r2 & CFG_R2_MAPROM_STATUS)) {
		printf("Cannot enable Shadow ROM if MAPROM enabled or currently active!\n");
		return;
	}

	cfgreg_set(CFG_R0_OFFSET, CFG_R0_WRITELOCKOFF); 

	memcpy((void*) 0xE00000, (void*) 0xE00000, 512*1024);	
	memcpy((void*) 0xF80000, (void*) 0xF80000, 512*1024);	

	cfgreg_unset(CFG_R0_OFFSET, CFG_R0_WRITELOCKOFF); 
	cfgreg_set(CFG_R1_OFFSET, CFG_R1_SHADOWROM); 
}

/* activate shadow rom functionality */
/*
void
shadow_activate(void)
{
	int i;
	uint8_t r1, r2;

	r1 = cfgreg_read(CFG_R1_OFFSET);
	r2 = cfgreg_read(CFG_R2_OFFSET);

	if ( (r1 & CFG_R1_MAPROM) || (r2 & CFG_R2_MAPROM_STATUS)) {
		printf("Cannot enable Shadow ROM if MAPROM enabled or currently active!\n");
		return;
	}
	
	cfgreg_set(CFG_R0_OFFSET, CFG_R0_WRITELOCKOFF); 

	bank_select(0);
	bank_copy(0xE00000);
	bank_select(1);
	bank_copy(0xE40000);
	bank_select(2);
	bank_copy(0xF80000);
	bank_select(3);
	bank_copy(0xFC0000);

	cfgreg_unset(CFG_R0_OFFSET, CFG_R0_WRITELOCKOFF); 
	cfgreg_set(CFG_R1_OFFSET, CFG_R1_SHADOWROM); 
	
}
*/

void
rom_load(char *path)
{
	char *rombuf;
	size_t romsize;
	uint8_t r1, r2;

	r1 = cfgreg_read(CFG_R1_OFFSET);
	r2 = cfgreg_read(CFG_R2_OFFSET);

	/* do some sanity checks first */	
	if (r1 & CFG_R1_SHADOWROM) {
		printf("Cannot use MAPROM if Shadow ROM enabled. Please disable Shadow ROM and reboot first!\n");
		return;
	}

	if (r2 & CFG_R2_MAPROM_STATUS) {
		printf("Cannot load new ROM if MAPROM currently active. Please disable MAPROM and reboot first!\n");
		return;
	}

#ifdef DEBUG
	printf("DEBUG: will load ROM from %s\n", path);
#endif /* DEBUG */

	romsize = file_size(path);

#ifdef DEBUG
	printf("DEBUG: m'kay so apparanetly loaded ROM has size: %x\n", (unsigned int) romsize);
#endif /* DEBUG */

	rombuf = (char*) malloc(romsize);

#ifdef DEBUG
	printf("DEBUG: allocated %x bytes at address %p, ready to load data\n", (unsigned int) romsize, (void*) rombuf);
#endif /* DEBUG */

	file_load(path, rombuf, romsize);

	cfgreg_set(CFG_R0_OFFSET, CFG_R0_WRITELOCKOFF); 
	switch (romsize) {
	case 262144:
		bank_select(0);
		bank_copy((uint32_t) rombuf);
		bank_select(1);
		bank_copy((uint32_t) rombuf);
		bank_select(2);
		bank_copy((uint32_t) rombuf);
		bank_select(3);
		bank_copy((uint32_t) rombuf);
		break;
	case 524288:
		bank_select(0);
		bank_copy((uint32_t) rombuf);
		bank_select(1);
		bank_copy((uint32_t) rombuf + 256*1024);
		bank_select(2);
		bank_copy((uint32_t) rombuf);
		bank_select(3);
		bank_copy((uint32_t) rombuf + 256*1024);
		break;
	case 1048576:
		bank_select(0);
		bank_copy((uint32_t) rombuf);
		bank_select(1);
		bank_copy((uint32_t) rombuf + 256*1024);
		bank_select(2);
		bank_copy((uint32_t) rombuf + 512*1024);
		bank_select(3);
		bank_copy((uint32_t) rombuf + 768*1024);
		break;
	default:
		printf("Unsupported ROM size %x\n, ROM must be exactly 256kB, 512kB or 1MB\n", (unsigned int) romsize);
		/* bail out */
		/*free(rombuf);*/
		cfgreg_unset(CFG_R0_OFFSET, CFG_R0_WRITELOCKOFF); 
		return;
		break;
	}

	/*free(rombuf);*/
	cfgreg_unset(CFG_R0_OFFSET, CFG_R0_WRITELOCKOFF); 

	printf("Your Amiga should be restarted now...\n");
}

