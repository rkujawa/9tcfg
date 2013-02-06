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

/* -- function prototypes -- */

void reboot(void);
void usage(void);
void status_display(void);

/* -- global variables -- */

static const STRPTR version = "\0$VER: 9tcfg 0.2.3 (06.02.2013)";
static const STRPTR id = "\0$Id$";

/* -- implementation -- */

/* display program usage information */
/*void
usage(void) 
{
	printf("usage: 9tcfg [--68kmode=0|1] [--instcache=0|1] [--pcmciamode=0|1] [--maprom=0|1] [--mapromload=file.rom] [--shadowrom=0|1] [--memoryadd] [--reboot]\n");	
}
*/
/* read and display configuration registers */
/*void
status_display(void) 
{
	uint8_t r0, r1, r2;

	r0 = cfgreg_read(CFG_R0_OFFSET);
	r1 = cfgreg_read(CFG_R1_OFFSET);
	r2 = cfgreg_read(CFG_R2_OFFSET);

	printf("\t68020: ");
	if (r0 & CFG_R0_DISABLE)
		printf("disabled\n");
	else
		printf("enabled\n");

	printf("\tPCMCIA mode: ");
	if (r0 & CFG_R0_PCMCIA)
		printf("enabled\n");
	else
		printf("disabled\n");

	printf("\tInstruction cache: ");
	if (r0 & CFG_R0_CACHEOFF)
		printf("disabled\n");
	else
		printf("enabled\n");
	
	printf("\tB80000-BC0000 write lock: ");
	if (r0 & CFG_R0_WRITELOCKOFF)
		printf("disabled\n");
	else
		printf("enabled\n");

	printf("\tMAPROM: ");
	if (r1 & CFG_R1_MAPROM)
		printf("enabled\n");
	else
		printf("disabled\n");

	printf("\tMAPROM bank: %d (bits: ", (int) bank_bits2num(r1));
	if (r1 & CFG_R1_BANKBIT0)
		printf("1");
	else
		printf("0");
	if (r1 & CFG_R1_BANKBIT1)
		printf("1");
	else
		printf("0");
	printf(")\n");

	printf("\tShadow ROM: ");
	if (r1 & CFG_R1_SHADOWROM)
		printf("enabled\n");
	else
		printf("disabled\n");

	printf("\t68000 mode: ");
	if (r2 & CFG_R2_68KMODE)
		printf("enabled\n");
	else
		printf("disabled\n");

	printf("\tCurrent MAPROM state: ");
	if (r2 & CFG_R2_MAPROM_STATUS)
		printf("enabled\n");
	else
		printf("disabled\n");

}
*/
int
main(int argc, char *argv[])
{

	/*
	 * AmigaOS ReadArgs-style argument parsing is an inconsistent shit.
	 * Pile of shit. Period.
	 */
	struct RDArgs *result;
	CONST_STRPTR argTemplate =
	    "MAPROM/T,SHADOWROM/T,LOADROM/K,MEMORYADD/S,REBOOT/S";
#define ARGNUM		5
#define TOGGLE_EMPTY	-2
#define TOGGLE_FALSE	0x0
#define TOGGLE_TRUE	0xFFFFFFFF

#define MAPROM_ARG	0
#define SHADOWROM_ARG	1
#define LOADROM_ARG	2
#define MEMORYADD_ARG	3
#define REBOOT_ARG	4

	LONG *argArray;
	argArray = AllocVec(ARGNUM*sizeof(LONG), MEMF_ANY|MEMF_CLEAR);

	argArray[MAPROM_ARG] = TOGGLE_EMPTY;
	argArray[SHADOWROM_ARG] = TOGGLE_EMPTY;

	result = ReadArgs(argTemplate, argArray, NULL);

	/* 
	 * Some RURUs for correct usage of this program...
	 */
	printf("%p %x\n", argArray[MAPROM_ARG], *argArray);

	if ( ((LONG) argArray[MAPROM_ARG] != TOGGLE_EMPTY) &&
	   ((LONG) argArray[SHADOWROM_ARG] != TOGGLE_EMPTY) ) {
		printf("MAPROM and SHADOWROM can't be used together!\n");
		return 10;
	}

	if ( ((LONG) argArray[MAPROM_ARG] != TOGGLE_EMPTY))
	{
#ifdef DEBUG
		printf("DEBUG: MAPROM arugment passed\n");
#endif /* DEBUG */
		/* MAPROM ON only if LOADROM passed. */
		if ((LONG) argArray[MAPROM_ARG] == TOGGLE_TRUE) {
			if ((LONG) argArray[LOADROM_ARG] == 0) {
				printf("MAPROM ON must be used with LOADROM!\n");
				return 10;
			}
			maprom_enable((STRPTR) argArray[LOADROM_ARG]);
		} /* else
			maprom_disable(); */
	}
	if ( ((LONG) argArray[SHADOWROM_ARG] != TOGGLE_EMPTY))
	{
#ifdef DEBUG
		printf("DEBUG: SHADOWROM arugment passed\n");
#endif /* DEBUG */
		if ((LONG) argArray[SHADOWROM_ARG] == TOGGLE_TRUE) {
			shadowrom_enable();
		} else {
			/* shadowrom_disable(); */
		}

	}
	if ( ((LONG) argArray[MEMORYADD_ARG] != 0))
	{
#ifdef DEBUG
		printf("DEBUG: SHADOWROM arugment passed\n");
#endif /* DEBUG */
	}

	if ((LONG) argArray[REBOOT_ARG] != 0) {
		reboot();
	}

	/* TODO: Display status... */

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

