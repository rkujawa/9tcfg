/* Ninetails configuration program */

#include <stdio.h>
#include <stdlib.h>

#include <exec/types.h>
#include <exec/execbase.h>
#include <workbench/startup.h>

#include <proto/dos.h>
#include <proto/exec.h>

#include "config.h"
#include "hardware.h"
#include "addmem.h"
#include "cfgreg.h"
#include "rom.h"
#include "cpu.h"

#define EXIT_SYNTAX_ERROR	10
#define EXIT_HARDWARE_ERROR	20 

/* -- function prototypes -- */

void reboot(void);
void status_display(void);
void status_print_reg(UBYTE reg, UBYTE bit);
void status_print_reg_inv(UBYTE reg, UBYTE bit);

/* -- global variables -- */

static const STRPTR version = "\0$VER: 9tcfg 0.3 (06.02.2013)\0";
static const STRPTR id = "\0$Id$\0";

/* -- implementation -- */

void
status_print_reg_inv(UBYTE reg, UBYTE bit)
{
	if (reg & bit)
		printf("disabled\n");
	else
		printf("enabled\n");
}

void
status_print_reg(UBYTE reg, UBYTE bit)
{
	if (reg & bit)
		printf("enabled\n");
	else
		printf("disabled\n");
}

/* read and display configuration registers */
void
status_display(void) 
{
	UBYTE r0, r1, r2;

	r0 = cfgreg_read(CFG_R0_OFFSET) & CFG_LOCK_ID_MASK;
	r1 = cfgreg_read(CFG_R1_OFFSET) & CFG_LOCK_ID_MASK;
	r2 = cfgreg_read(CFG_R2_OFFSET) & CFG_LOCK_ID_MASK;

	printf(" ==================== CPU / Memory options ==================== \n");

	printf("\t68000 mode at next reset: ");
	status_print_reg(r0, CFG_R0_68KMODE);

	printf("\t5.5MB Fast if 68000 active at next reset: ");
	status_print_reg(r0, CFG_R0_68KMEMORYMODE);

	printf("\t68000 mode status: ");
	status_print_reg(r2, CFG_R2_68KMODE_STATUS);

	printf("\tPCMCIA2RAM mode (add 4MB RAM, disable PCMCIA): ");
	status_print_reg(r0, CFG_R0_PCMCIA2RAM);

	printf("\tInstruction cache: ");
	status_print_reg_inv(r1, CFG_R1_INSTCACHEOFF);
	
	printf(" ========================= ROM options ======================== \n");

	printf("\tMAPROM at next reset: ");
	status_print_reg(r1, CFG_R1_MAPROM);

	printf("\tMAPROM status: ");
	status_print_reg(r2, CFG_R2_MAPROM_STATUS);

	printf("\tShadow ROM: ");
	status_print_reg(r1, CFG_R1_SHADOWROM);
	
/*	printf("\tB80000-BC0000 write lock: ");
	if (r0 & CFG_R0_WRITELOCKOFF)
		printf("disabled\n");
	else
		printf("enabled\n");*/
/*	printf("\tMAPROM bank: %d (bits: ", (int) bank_bits2num(r1));
	if (r1 & CFG_R1_BANKBIT0)
		printf("1");
	else
		printf("0");
	if (r1 & CFG_R1_BANKBIT1)
		printf("1");
	else
		printf("0");
	printf(")\n");
*/

}

int
main(int argc, char *argv[])
{
	BYTE hwrev; /* hardware revision */

	/*
	 * AmigaOS ReadArgs-style argument parsing is an inconsistent shit.
	 * Pile of shit. Period.
	 */
	struct RDArgs *result;
	CONST_STRPTR argTemplate =
	    "MODE68K/T,MODE68KMEMORY/T,PCMCIA2RAM/S,MAPROM/T,SHADOWROM/T,LOADROM/K,MOREMEM/S,INSTCACHE/T,REBOOT/S";
#define ARGNUM		10	
#define TOGGLE_EMPTY	-2
#define TOGGLE_FALSE	0x0
#define TOGGLE_TRUE	0xFFFFFFFF

#define MODE68K_ARG	0
#define MODE68KMEMORY_ARG 1
#define PCMCIA2RAM_ARG	2
#define MAPROM_ARG	3
#define SHADOWROM_ARG	4	
#define LOADROM_ARG	5	
#define MOREMEM_ARG	6
#define INSTCACHE_ARG	7
#define REBOOT_ARG	8

	LONG *argArray;
	argArray = AllocVec(ARGNUM*sizeof(LONG), MEMF_ANY|MEMF_CLEAR);

	argArray[MODE68K_ARG] = TOGGLE_EMPTY;
	argArray[MODE68KMEMORY_ARG] = TOGGLE_EMPTY;
	argArray[MAPROM_ARG] = TOGGLE_EMPTY;
	argArray[SHADOWROM_ARG] = TOGGLE_EMPTY;
	argArray[INSTCACHE_ARG] = TOGGLE_EMPTY;

	result = ReadArgs(argTemplate, argArray, NULL);

	/* 
	 * Some RURUs for correct usage of this program...
	 */
	if ( ((LONG) argArray[MAPROM_ARG] != TOGGLE_EMPTY) &&
	   ((LONG) argArray[SHADOWROM_ARG] != TOGGLE_EMPTY) ) {
		printf("MAPROM and SHADOWROM can't be used together!\n");
		return EXIT_SYNTAX_ERROR;
	}


	hwrev = ninetails_detect();

	if (hwrev == -1) {
		printf("Ninetails board not detected!\n");
		return EXIT_HARDWARE_ERROR;
	} else {
		printf("Ninetails revision %d\n", hwrev);
	}

	cfgreg_unlock();

	/* MAPROM ON only if LOADROM passed. */
	if ((LONG) argArray[MAPROM_ARG] == TOGGLE_TRUE) {
		if ((LONG) argArray[LOADROM_ARG] == 0) {
			printf("MAPROM ON must be used with LOADROM!\n");
		} else 
			maprom_enable((STRPTR) argArray[LOADROM_ARG]);
	} else if ((LONG) argArray[MAPROM_ARG] == TOGGLE_FALSE) {
		maprom_disable();
	}

	if ((LONG) argArray[SHADOWROM_ARG] == TOGGLE_TRUE) {
		shadowrom_enable();
	} else if ((LONG) argArray[SHADOWROM_ARG] == TOGGLE_FALSE) {
		shadowrom_disable();
	}

	if ( ((LONG) argArray[MOREMEM_ARG] != 0))
	{
#ifdef DEBUG
		printf("DEBUG: MOREMEM arugment passed\n");
#endif /* DEBUG */
	if(r2 & CFG_R2_68KMODE_STATUS) printf("MOREMEM is not allowed in 68000 mode"); else memory_add_misc();
	}

	if ((LONG) argArray[MODE68K_ARG] == TOGGLE_TRUE) {
		cpu_68k_enable();
	} else if ((LONG) argArray[MODE68K_ARG] == TOGGLE_FALSE) {
		cpu_68k_disable();
	}

	if ((LONG) argArray[MODE68KMEMORY_ARG] == TOGGLE_TRUE) {
		cpu_68kfast_enable();
	} else if ((LONG) argArray[MODE68KMEMORY_ARG] == TOGGLE_FALSE) {
		cpu_68kfast_disable();
	}

	if ((LONG) argArray[PCMCIA2RAM_ARG] != 0) {
		if(r2 & CFG_R2_68KMODE_STATUS)	{
			printf("PCMCIA2RAM is not allowed in 68000 mode");
			}
		else {
			pcmcia2ram_enable();
			memory_add_4m();
		}
	}

	if ((LONG) argArray[INSTCACHE_ARG] == TOGGLE_TRUE) {
		instcache_enable();
	} else if ((LONG) argArray[INSTCACHE_ARG] == TOGGLE_FALSE) {
		instcache_disable();
	}

	if ((LONG) argArray[REBOOT_ARG] != 0) {
		reboot();
	}

	status_display();

	cfgreg_lock();

	/* Free everything and return to OS. */
	FreeArgs(result);
	FreeVec(argArray);

	return 0;
}

/* reboot the machine */
void
reboot(void)
{
	/* think about opening graphics.library, 
	   then LoadView(); 2x WaitTOF(); to work around V39 bug */

	/* for now let's just call Exec's ColdReboot()... */
	ColdReboot();
}

