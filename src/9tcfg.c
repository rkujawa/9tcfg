/* Ninetails configuration program */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <getopt.h>

#include <exec/types.h>

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

static const STRPTR version = "\0$VER: 9tcfg 0.2.1 (03.02.2013)";

/* -- implementation -- */

/* display program usage information */
/*void
usage(void) 
{
	printf("usage: 9tcfg [--68kmode=0|1] [--instcache=0|1] [--pcmciamode=0|1] [--maprom=0|1] [--mapromload=file.rom] [--shadowrom=0|1] [--memoryadd] [--reboot]\n");	
}
*/
/* read and display configuration registers */
void
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

int
main(int argc, char *argv[])
{
	struct RDArgs *result;
	CONST_STRPTR argTemplate = "MAPROM/T,LOADROM/K,SHADOWROM/T,REBOOT/S,MEMORYADD/S";
	LONG argArray[] = { 0, 0, 0, 0, 0 };

	result = IDOS->ReadArgs(argTemplate, argArray, NULL);

	/* ... */

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

