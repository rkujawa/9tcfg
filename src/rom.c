#include <stdio.h>

#include <exec/types.h>

#include "rom.h"
#include "hardware.h"
#include "file.h"

#define EXIT_SYNTAX_ERROR	10
#define EXIT_HARDWARE_ERROR	20 

extern BOOL debug;

BOOL rom_copy_self(BYTE *rombuf, ULONG romsize);

void
shadowrom_disable(void)
{
	cfgreg_unset(CFG_R1_OFFSET, CFG_R1_SHADOWROM); 
}

/* copy rom over itself */
void
shadowrom_enable(void)
{
	
	UBYTE r1, r2;

	r1 = cfgreg_read(CFG_R1_OFFSET);
	r2 = cfgreg_read(CFG_R2_OFFSET);

	if (r2 & CFG_R2_68KMODE_STATUS) {
		printf("Cannot use SHADOWROM if running on 68000! Please reenable 68020 and reboot first.\n");
		return;
	}

	if ( (r1 & CFG_R1_MAPROM) || (r2 & CFG_R2_MAPROM_STATUS)) {
		printf("Cannot enable Shadow ROM if MAPROM enabled or currently active!\n");
		return;
	}
	
	if (r1 & CFG_R1_SHADOWROM) {
		printf("SHADOWROM is already active!\n");
		return;
	}

/*	cfgreg_set(CFG_R0_OFFSET, CFG_R0_WRITELOCKOFF);  */

	memcpy((void*) 0xE00000, (void*) 0xE00000, 512*1024);	
	memcpy((void*) 0xF80000, (void*) 0xF80000, 512*1024);	

/*	cfgreg_unset(CFG_R0_OFFSET, CFG_R0_WRITELOCKOFF);  */
	cfgreg_set(CFG_R1_OFFSET, CFG_R1_SHADOWROM); 
}

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
	if (debug)
		printf("DEBUG: changing to bank %d\n", (int) banknum);

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
	if (debug)
		printf("DEBUG: copying 256kB block from %x to %x\n", address, MAPROM_BANK_ADDRESS);
#endif 

	memcpy((void*) MAPROM_BANK_ADDRESS, (void*) address, 256*1024);	
}
*/

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
maprom_disable()
{
	cfgreg_unset(CFG_R1_OFFSET, CFG_R1_MAPROM); 
}

unsigned char
maprom_enable(BYTE *path)
{
	BYTE *rombuf;
	ULONG romsize;
	UBYTE r1, r2;

	r1 = cfgreg_read(CFG_R1_OFFSET);
	r2 = cfgreg_read(CFG_R2_OFFSET);

	/* do some sanity checks first */	
	if (r1 & CFG_R1_SHADOWROM) {
		printf("Cannot use MAPROM if Shadow ROM enabled. Please disable Shadow ROM and reboot first!\n");
		return EXIT_SYNTAX_ERROR;
	}

	if (r2 & CFG_R2_MAPROM_STATUS) {
		printf("Cannot load new ROM if MAPROM currently active. Please disable MAPROM and reboot first!\n");
		return EXIT_SYNTAX_ERROR;
	}

	if (r2 & CFG_R2_68KMODE_STATUS) {
		printf("Cannot use MAPROM if running on 68000! Please reenable 68020 and reboot first.\n");
		return EXIT_SYNTAX_ERROR;
	}

	if (debug)
		printf("DEBUG: will load ROM from %s\n", path);

	romsize = file_size(path);

	if (debug)
		printf("DEBUG: m'kay so apparanetly loaded ROM has size: %lx\n", romsize);

	rombuf = (char*) malloc(romsize);

	if (debug)
		printf("DEBUG: allocated %x bytes at address %p, ready to load data\n", (unsigned int) romsize, (void*) rombuf);

	file_load(path, rombuf, romsize);

	rom_copy_self(rombuf, romsize);
	/*rom_copy_bank(rombuf, romsize);*/

	cfgreg_set(CFG_R1_OFFSET, CFG_R1_MAPROM); 

	free(rombuf);
	printf("Your Amiga should be restarted now...\n");
	
	return 0;
}

BOOL
rom_copy_self(BYTE *rombuf, ULONG romsize)
{
	switch (romsize) {
	case 262144:
		memcpy((void*) 0xE00000, (void*) rombuf, S256K);	
		memcpy((void*) (0xE00000+S256K), (void*) rombuf, S256K);	
		memcpy((void*) 0xF80000, (void*) rombuf, S256K);	
		memcpy((void*) (0xF80000+S256K), (void*) rombuf, S256K);	
		break;
	case 524288:
		memcpy((void*) 0xE00000, (void*) rombuf, S512K);	
		memcpy((void*) 0xF80000, (void*) rombuf, S512K);	
		break;
	case 1048576:
		memcpy((void*) 0xE00000, (void*) rombuf, S512K);	
		memcpy((void*) 0xF80000, (void*) (rombuf + S512K), S512K);	
		break;
	default:
		printf("Unsupported ROM size %x\n, ROM must be exactly 256kB, 512kB or 1MB\n", (unsigned int) romsize);
		return 0;
		break;
	}

	return 1;	
}
/*
void
rom_copy_bank(char *rombuf, ULONG romsize)
{
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
		// bail out 
		cfgreg_unset(CFG_R0_OFFSET, CFG_R0_WRITELOCKOFF); 
		return;
		break;
	}
	cfgreg_unset(CFG_R0_OFFSET, CFG_R0_WRITELOCKOFF); 
}
*/
