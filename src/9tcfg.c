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

static const STRPTR version = "\0$VER: 9tcfg 0.2 (03.02.2013)";

/* -- implementation -- */

/* display program usage information */
void
usage(void) 
{
	printf("usage: 9tcfg [--68kmode=0|1] [--instcache=0|1] [--pcmciamode=0|1] [--maprom=0|1] [--mapromload=file.rom] [--shadowrom=0|1] [--memoryadd] [--reboot]\n");	
}

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
	int ch;

	bool flag_maprombank = 0;	uint8_t maprombank_number;
	bool flag_customaddress = 0;	uint32_t customaddress = 0;
	bool flag_copytobank = 0;	uint32_t copytobank_address = 0;
	bool flag_loadrom = 0;		char loadrom_path[256];
	bool flag_memoryadd = 0;
	bool flag_shadowromactivate = 0;
	bool flag_shadowromactivateself = 0;
	bool flag_reboot = 0;

	extern char *optarg;
	extern int optind;

	static struct option longopts[] = {
		{ "disable020",		no_argument,	&toggles[0].disable_flag,	'd' },
		{ "enable020",		no_argument,	&toggles[0].enable_flag,	'e' },
		{ "pcmciamodeoff",	no_argument,	&toggles[1].disable_flag,	'p' },
		{ "pcmciamodeon",	no_argument,	&toggles[1].enable_flag,	'P' },
		{ "instcacheoff",	no_argument,	&toggles[2].disable_flag,	'I' },
		{ "instcacheon",	no_argument,	&toggles[2].enable_flag,	'i' },
		{ "writelockoff",	no_argument,	&toggles[3].disable_flag,	'u' },
		{ "writelockon",	no_argument,	&toggles[3].enable_flag,	'l' },
		{ "mapromoff",		no_argument,	&toggles[4].disable_flag,	'M' },
		{ "mapromon",		no_argument,	&toggles[4].enable_flag,	'm' },
		{ "shadowromoff",	no_argument,	&toggles[5].disable_flag,	'S' },
		{ "shadowromon",	no_argument,	&toggles[5].enable_flag,	's' },
		{ "shadowromactivate",	no_argument,	NULL, 'o' },	
		{ "shadowromactivateself",no_argument,	NULL, 'O' },	
		{ "maprombank",		required_argument, NULL, 'b' },
		{ "customaddress",	required_argument, NULL, 'a' },
		{ "copytobank",		required_argument, NULL, 'c' },
		{ "mapromload",		required_argument, NULL, 'f' },
		{ "memoryadd",		no_argument,	NULL, 'r' },
		{ "reboot",		no_argument,	NULL, 'R' },
		{ NULL,			0,		NULL,	0 }
	};

	while ((ch = getopt_long(argc, argv, "depPIiulMmSsbahcro?:", longopts, NULL)) != -1) {
		switch (ch) {
		case 'b':
			flag_maprombank = 1;
			maprombank_number = atoi(optarg);
			break;
		case 'a':
			flag_customaddress = 1;
			customaddress = strtoul(optarg, NULL, 16);
			break;
		case 'c':
			flag_copytobank = 1;
			copytobank_address = strtoul(optarg, NULL, 16); 
			break;
		case 'r':
			flag_memoryadd = 1;
			break;
		case 'R':
			flag_reboot = 1;
			break;
		case 'o':
			flag_shadowromactivate = 1;
			break;
		case 'O':
			flag_shadowromactivateself = 1;
			break;
		case 'f':
			flag_loadrom = 1;
			strncpy(loadrom_path, optarg, 256);
			break;
		case 0:
			break;
		case '?':
		case 'h':
		default: /* means that some weird args were passed */
			usage();
			exit(EXIT_FAILURE); /* exit in this case */
		}

	}
	argc -= optind;
	argv += optind;

	if (flag_customaddress)
		cardaddr = (void*) customaddress;

	if (flag_maprombank)
		bank_select(maprombank_number);

	if (flag_copytobank)
		bank_copy(copytobank_address);

	if (flag_memoryadd)
		memory_add();

	if (flag_loadrom)
		loadrom(loadrom_path);

	if (flag_shadowromactivateself)
		shadow_activate_self();

	if (flag_shadowromactivate)
		shadow_activate();

	flag_toggle();

	cfgreg_display();

	if (flag_reboot)
		reboot();
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

