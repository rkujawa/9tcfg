/* Ninetails configuration program */

#include <stdio.h>
#include <stdlib.h>

#include <exec/types.h>
#include <exec/execbase.h>
#include <workbench/startup.h>

#include <proto/dos.h>
#include <proto/exec.h>

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

static const STRPTR version = "\0$VER: 9tcfg 0.7 (19.01.2014)\0";
static const STRPTR id = "\0$Id$\0";

static LONG *argArray;	/* arguments passed on the command line */

BOOL debug = FALSE;

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

	r0 = cfgreg_read(CFG_R0_OFFSET);
	r1 = cfgreg_read(CFG_R1_OFFSET);
	r2 = cfgreg_read(CFG_R2_OFFSET);

	printf(" ==================== CPU / Memory options ==================== \n");

	printf("\tEnable MC68000 after reset: ");
	status_print_reg(r0, CFG_R0_68KMODE);

	printf("\tEnable 16bit FastRAM after reset: ");
	status_print_reg(r0, CFG_R0_68KMEMORYMODE);

	printf("\tMC68000 status: ");
	status_print_reg(r2, CFG_R2_68KMODE_STATUS);

	printf("\tPCMCIA Friendly (4MB RAM) after reset: ");
	status_print_reg(r0, CFG_R0_PCMCIA);

	printf("\tInstruction cache: ");
	status_print_reg_inv(r1, CFG_R1_INSTCACHEOFF);
	
	printf(" ========================= ROM options ======================== \n");

	printf("\tMAPROM at next reset: ");
	status_print_reg(r1, CFG_R1_MAPROM);

	printf("\tMAPROM status: ");
	status_print_reg(r2, CFG_R2_MAPROM_STATUS);

	printf("\tShadow ROM: ");
	status_print_reg(r1, CFG_R1_SHADOWROM);
}

void
help(void) 
{


	printf("9tcfg     - Ninetails accelerator config tool by R.Kujawa \n\n");
	printf("M68K      - turn off accelerator (on/off) \n");
	printf("M68KMEM   - enable 5.5MB 16bit FastRam, available when M68K is ON (ON/OFF)\n");
	printf("PCMCIA    - sacrifice 4MB of fastram (600000-9FFFFF) for PCMCIA sake (ON/OFF)\n");
	printf("SHADOWROM - copy onboard kickstart to reserved RAM for shadowing (ON/OFF) \n");
	printf("MAPROM    - enable mapping of loaded kickstart file, use with LOADROM (ON/OFF)  \n");
	printf("LOADROM   - load kickstart file to reserved RAM (1MB is supported) \n");
	printf("MOREMEM   - add more memory to system pool (A80000-B7FFFF, F00000-F7FFFF)  \n");
	printf("INSTCACHE - disable/enable instruction cache for MC68EC020 (ON/OFF)\n");
	printf("REBOOT    - die and rise from ashes \n");
	printf("DEBUG     - display informations useful only for developers \n");
	printf(" \n\n");
	printf("Example: \n");
	printf("9tcfg maprom on loadrom=ks3.9.rom moremem pcmcia on reboot \n");
	printf(" \n");
	
	


}

BOOL
arg_switch_isempty(UBYTE argNo)
{
	if ( ((LONG) argArray[argNo] != 0))
		return 0;

	return 1;
}

BOOL
arg_toggle_val(UBYTE argNo)
{
#define TOGGLE_EMPTY	-2
#define TOGGLE_FALSE	0x0
#define TOGGLE_TRUE	0xFFFFFFFF
	if ((LONG) argArray[argNo] == TOGGLE_TRUE)
		return 1;
	else if ((LONG) argArray[argNo] == TOGGLE_FALSE)
		return 0;
/*	else
		// I wonder if we'll observe one of these, duh. 
		printf("DEBUG: toggle neither TRUE nor FALSE, this should not happen!\n");
*/

	return 0;
}

BOOL
arg_key_isempty(UBYTE argNo)
{
	if ((LONG) argArray[argNo] == 0)
		return 1;
	else
		return 0;
}

BOOL
arg_toggle_isempty(UBYTE argNo)
{
	if ((LONG) argArray[argNo] != TOGGLE_EMPTY)
		return 0;	
	else
		return 1;
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
	    "M68K/T,M68KMEM/T,PCMCIA/T,MAPROM/T,SHADOWROM/T,LOADROM/K,MOREMEM/S,INSTCACHE/T,REBOOT/S,DEBUG/S,HELP/S";
#define ARGNUM		12

#define MODE68K_ARG	0
#define MODE68KMEMORY_ARG 1
#define PCMCIA_ARG	2
#define MAPROM_ARG	3
#define SHADOWROM_ARG	4	
#define LOADROM_ARG	5	
#define MOREMEM_ARG	6
#define INSTCACHE_ARG	7
#define REBOOT_ARG	8
#define DEBUG_ARG	9
#define HELP_ARG	10

	argArray = AllocVec(ARGNUM*sizeof(LONG), MEMF_ANY|MEMF_CLEAR);

	argArray[MODE68K_ARG] = TOGGLE_EMPTY;
	argArray[MODE68KMEMORY_ARG] = TOGGLE_EMPTY;
	argArray[PCMCIA_ARG] = TOGGLE_EMPTY;
	argArray[MAPROM_ARG] = TOGGLE_EMPTY;
	argArray[SHADOWROM_ARG] = TOGGLE_EMPTY;
	argArray[INSTCACHE_ARG] = TOGGLE_EMPTY;

	result = ReadArgs(argTemplate, argArray, NULL);

	if (!arg_switch_isempty(DEBUG_ARG)) {
		debug = TRUE; 
	}

	/* 
	 * Some RURUs for correct usage of this program...
	 */
	if ((!arg_toggle_isempty(MAPROM_ARG)) &&
	   (!arg_toggle_isempty(SHADOWROM_ARG))) {
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

	/* Some options are forbidden in 68000 mode. */
	if (cpu_68k_get()) {
		if (!arg_switch_isempty(MOREMEM_ARG)) {
			printf("MOREMEM cannot be used in 68000 mode!\n");
			cfgreg_lock();
			return EXIT_SYNTAX_ERROR;	
		}
		if (!arg_toggle_isempty(PCMCIA_ARG)) {
			printf("PCMCIA is always available in 68000 mode!\n");
			cfgreg_lock();
			return EXIT_SYNTAX_ERROR;
		}
		if (!arg_toggle_isempty(MAPROM_ARG)) {
			printf("MAPROM cannot be used in 68000 mode!\n");
			cfgreg_lock();
			return EXIT_SYNTAX_ERROR;
		}
		if (!arg_toggle_isempty(SHADOWROM_ARG)) {
			printf("SHADOWROM cannot be used in 68000 mode!\n");
			cfgreg_lock();
			return EXIT_SYNTAX_ERROR;
		}	
	}

	/* MAPROM ON only if LOADROM passed. */
	if (!arg_toggle_isempty(MAPROM_ARG)) {
		if (arg_toggle_val(MAPROM_ARG)) 
			if (arg_key_isempty(LOADROM_ARG))
				printf("MAPROM ON must be used with LOADROM!\n");
			else 
				maprom_enable((STRPTR) argArray[LOADROM_ARG]);
		else
			maprom_disable();
	}

	if (!arg_toggle_isempty(SHADOWROM_ARG)) {
		if (arg_toggle_val(SHADOWROM_ARG)) 
			shadowrom_enable();
		else
			shadowrom_disable();
	}

	if (!arg_switch_isempty(MOREMEM_ARG))
	{
/*
		printf("DEBUG: MOREMEM arugment passed\n");
*/
		/* only if not running in 68000 mode */	
		memory_add_misc();
	}

	if (!arg_toggle_isempty(MODE68K_ARG)) {
		if (arg_toggle_val(MODE68K_ARG)) 
			cpu_68k_enable();
		else 
			cpu_68k_disable();
	}

	if (!arg_toggle_isempty(MODE68KMEMORY_ARG)) {
		if (arg_toggle_val(MODE68KMEMORY_ARG)) 
			cpu_68kfast_enable();
		else
			cpu_68kfast_disable();
	}

	if (!arg_toggle_isempty(PCMCIA_ARG)) {
		if (arg_toggle_val(PCMCIA_ARG)) 
			pcmcia_enable();
		else
			pcmcia_disable();
	}

	if (!arg_toggle_isempty(INSTCACHE_ARG)) {
		if (arg_toggle_val(INSTCACHE_ARG)) 
			instcache_enable();
		else
			instcache_disable();
	}

	if (!arg_switch_isempty(REBOOT_ARG)) {
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

