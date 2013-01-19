/* Ninetails configuration program */

#include <stdio.h>
#include <stdlib.h>
#ifndef __VBCC__
#include <stdint.h>
#include <stdbool.h>
#endif
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <getopt.h>

#include <exec/types.h>
#include <exec/execbase.h>
#include <proto/exec.h>

#define DEBUG		1	/* print debug messages */

/* -- hardware registers -- */

#define CFG_ADDRESS		0xBE0000
#define CFG_R0_OFFSET		0
#define CFG_R1_OFFSET		4
#define CFG_R2_OFFSET		8

#define CFG_R0_DISABLE		0x80	/* 1xxxxxxx - disable 68020 at next reset */
#define CFG_R0_PCMCIA		0x40	/* x1xxxxxx - PCMCIA mode */
#define CFG_R0_CACHEOFF		0x20	/* xx1xxxxx - instruction cache off */
#define CFG_R0_WRITELOCKOFF	0x10	/* xxx1xxxx - disable write lock to banks */

#define CFG_R1_MAPROM		0x80	/* 1xxxxxxx - MAPROM at next reset */
#define CFG_R1_SHADOWROM	0x40	/* x1xxxxxx - Shadow ROM */
#define CFG_R1_BANKBIT0		0x20	/* xx1xxxxx */
#define CFG_R1_BANKBIT1		0x10	/* xxx1xxxx */

#define CFG_R2_68KMODE		0x80	/* 1xxxxxxx - current 68k mode */
#define CFG_R2_MAPROM_STATUS	0x40	/* x1xxxxxx - current MAPROM mode */

#define MAPROM_BANK_ADDRESS	0xB80000

#define ADDMEM_0_BASE		0xA80000
#define ADDMEM_1_BASE		0xF00000
#define ADDMEM_PRI		0

/* -- data types and structs used in the program -- */

#ifdef __VBCC__
typedef uint8_t bool;
#endif

struct flags_to_regs {
	int enable_flag;	/* flag to enable something was set */
	int disable_flag;	/* flag to disable something was set */
	bool one_means_off;	/* setting this bit means it disables something */
	uint8_t reg_offset;	/* offset to register */
	uint8_t bit;		/* which bit to set/unset in this register */
};

/* -- function prototypes -- */

void bank_select(uint8_t banknum);
uint8_t bank_bits2num(uint8_t r1);
void bank_copy(uint32_t address);

uint8_t cfgreg_read(uint8_t offset);
void cfgreg_write(uint8_t offset, uint8_t value);
void cfgreg_set(uint8_t offset, uint8_t bits);
void cfgreg_unset(uint8_t offset, uint8_t bits);
void cfgreg_display(void);

void memory_add(void);
bool memory_check_added(uint32_t address);

void flag_toggle(void);

size_t file_load(char *path, char *filebuf);
void loadrom(char *path);
void shadow_activate(void);

void reboot(void);

void usage(void);

/* -- global variables -- */

static const STRPTR version = "\0$VER: 9tcfg 0.1.1 (19.01.2013)";

/* pointer to base address of the card */
uint8_t *cardaddr = CFG_ADDRESS;

/* simple on/off flags, this struct is meh */
struct flags_to_regs toggles[] = {
	{ 0, 0, 1, CFG_R0_OFFSET, CFG_R0_DISABLE }, /* enable/disable020 */
	{ 0, 0, 0, CFG_R0_OFFSET, CFG_R0_PCMCIA },  /* pcmciamodeon/pcmciamodeoff */
	{ 0, 0, 1, CFG_R0_OFFSET, CFG_R0_CACHEOFF }, /* instcacheon/instcacheoff */
	{ 0, 0, 1, CFG_R0_OFFSET, CFG_R0_WRITELOCKOFF }, /* instcacheon/instcacheoff */
	{ 0, 0, 0, CFG_R1_OFFSET, CFG_R1_MAPROM }, /* mapromon/mapromoff */
	{ 0, 0, 0, CFG_R1_OFFSET, CFG_R1_SHADOWROM }, /* shadowromon/shadowromoff */
	{ -1, -1, 0, 0, 0 } /* end */
};

/* -- implementation -- */

/* display program usage information */
void
usage(void) 
{
	printf("usage: 9tcfg [--disable020|--enable020] [--instcacheoff|--instcacheon] [--pcmciamodeoff|--pcmciamodeon] [--writelockoff|--writelockon] [--mapromoff|--mapromon] [--mapromload=file.rom] [--shadowromoff|--shadowromon] [--shadowromactivate] [--customaddress=0xADDRESS] [--copytobank=0xADDRESS] [--memoryadd] [--reboot]\n");	
}

/* read register at offset */
uint8_t
cfgreg_read(uint8_t offset) 
{
	uint8_t v;
	uint8_t *ptr;

	ptr = cardaddr + offset;
	v = *ptr;
#ifdef DEBUG
	printf("DEBUG: read %x from %p\n", (int) v, (void*) ptr);
#endif /* DEBUG */

	return v;
}

/* write register at offset */
void
cfgreg_write(uint8_t offset, uint8_t value)
{
	uint8_t *ptr;

	ptr = cardaddr + offset;
#ifdef DEBUG
	printf("DEBUG: write %x to %p\n", (int) value, (void*) ptr);
#endif /* DEBUG */
	*ptr = value;
}

/* set bit in register at offset */
void
cfgreg_set(uint8_t offset, uint8_t bits)
{
	uint8_t v;
	
	v = cfgreg_read(offset) | bits;
	cfgreg_write(offset, v);
}

/* unset bit in register at offset */
void
cfgreg_unset(uint8_t offset, uint8_t bits)
{
	uint8_t v;

	v = cfgreg_read(offset) & ~bits;
	cfgreg_write(offset, v);
}

/* convert bank bits into bank number */
uint8_t
bank_bits2num(uint8_t r1)
{
	uint8_t num;

	num = ((r1 & CFG_R1_BANKBIT0) >> 5) | ((r1 & CFG_R1_BANKBIT1) >> 3);

	return num;
}

/* read and display configuration registers */
void
cfgreg_display(void) 
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

void
flag_toggle(void) 
{
	int i;
	struct flags_to_regs *curr_flag;

	i = 0;
	curr_flag = &toggles[i];

	while (curr_flag->enable_flag != -1) {

		/*printf("this flag has enable %d, disable %d\n", curr_flag->enable_flag, curr_flag->disable_flag);*/

		if ((curr_flag->enable_flag) && (curr_flag->disable_flag))
			printf("WARNING: enable and disable for the same option used together!\n");

		if (curr_flag->one_means_off) {
			if (curr_flag->enable_flag)
				cfgreg_unset(curr_flag->reg_offset, curr_flag->bit);
			if (curr_flag->disable_flag)
				cfgreg_set(curr_flag->reg_offset, curr_flag->bit);
		} else {
			if (curr_flag->enable_flag)
				cfgreg_set(curr_flag->reg_offset, curr_flag->bit);
			if (curr_flag->disable_flag)
				cfgreg_unset(curr_flag->reg_offset, curr_flag->bit);
		}

		i++;
		curr_flag = &toggles[i];
	}
}

/* switch bank */
void
bank_select(uint8_t banknum)
{
#ifdef DEBUG
	printf("DEBUG: changing to bank %d\n", (int) banknum);
#endif /* DEBUG */

	/* XXX: this could be implemented with one set/unset operation */

	if (banknum & 0x1)
		cfgreg_set(CFG_R1_OFFSET, CFG_R1_BANKBIT0);
	else
		cfgreg_unset(CFG_R1_OFFSET, CFG_R1_BANKBIT0);

	if (banknum & 0x2)
		cfgreg_set(CFG_R1_OFFSET, CFG_R1_BANKBIT1);
	else
		cfgreg_unset(CFG_R1_OFFSET, CFG_R1_BANKBIT1);
		
}

/* copy memory to bank */
void
bank_copy(uint32_t address)
{
#ifdef DEBUG
	printf("DEBUG: copying 256kB block from %x to %x\n", address, MAPROM_BANK_ADDRESS);
#endif /* DEBUG */

	memcpy((void*) MAPROM_BANK_ADDRESS, (void*) address, 256*1024);	
}

/* add non-autoconfiguring memory to the system */
void
memory_add(void)
{
	if (!memory_check_added(ADDMEM_0_BASE))
		AddMemList(1*1024*1024, MEMF_FAST, ADDMEM_PRI, ADDMEM_0_BASE, "9T A8 RAM");

	if (!memory_check_added(ADDMEM_1_BASE))
		AddMemList(512*1024, MEMF_FAST, ADDMEM_PRI, ADDMEM_1_BASE, "9T F0 RAM");
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

	if (flag_shadowromactivate)
		shadow_activate();

	flag_toggle();

	cfgreg_display();

	if (flag_reboot)
		reboot();
}

/* activate shadow rom functionality */
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

/* check if memory at given address is already added to system memory list */
bool
memory_check_added(uint32_t address)
{
	struct MemHeader *m, *nm;

	for (m  = (void *) SysBase->MemList.lh_Head;
	    nm = (void *) m->mh_Node.ln_Succ; m = nm) {
		if (address == ( (uint32_t) m->mh_Lower & 0xFFFF00)) {
#ifdef DEBUG
			printf("DEBUG: memory at address %p already added\n", (void*) address);
#endif /* DEBUG */
			return 1;
		}
	}
	return 0;
}

/* load file to memory buffer */
size_t
file_load(char *path, char *filebuf)
{
	int fd;
	struct stat statbuf;
	size_t filesize;

	if ((fd = open(path, O_RDONLY)) == -1)  {	
		perror("Error openinig file");
		return 0;
	}

	fstat(fd, &statbuf);

	filebuf = (char*) malloc(statbuf.st_size);
#ifdef DEBUG
	printf("DEBUG: loading %ld bytes long file at %p\n", (long) statbuf.st_size, (void*) filebuf);
#endif /* DEBUG */

	if (read(fd, filebuf, statbuf.st_size) == -1) {
		perror("Error reading file");
		return 0;
	}

	filesize = statbuf.st_size;

	return filesize;
}

void
loadrom(char *path)
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

	romsize = file_load(path, rombuf);

#ifdef DEBUG
	printf("DEBUG: m'kay so apparanetly loaded ROM has size: %x\n", (unsigned int) romsize);
#endif /* DEBUG */

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
		free(rombuf);
		cfgreg_unset(CFG_R0_OFFSET, CFG_R0_WRITELOCKOFF); 
		return;
		break;
	}

	free(rombuf);
	cfgreg_unset(CFG_R0_OFFSET, CFG_R0_WRITELOCKOFF); 

	printf("Your Amiga should be restarted now...\n");
}

/* reboot the machine */
void
reboot(void)
{
	/* for now let's just call Exec's ColdReboot()... */
	ColdReboot();
}

