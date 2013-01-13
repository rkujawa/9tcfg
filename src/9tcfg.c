/* Ninetails configuration program */

#include <stdio.h>
#include <stdlib.h>
#ifndef __VBCC__
#include <stdint.h>
#include <stdbool.h>
#endif

#include <getopt.h>

#define CFG_ADDRESS		0xDE0000
#define CFG_R0_OFFSET		0
#define CFG_R1_OFFSET		4

#define CFG_R0_DISABLE		0x80	/* 1xxxxxxx */
#define CFG_R0_PCMCIA		0x40	/* x1xxxxxx */
#define CFG_R0_CACHEOFF		0x20	/* xx1xxxxx */
#define CFG_R0_WRITELOCKOFF	0x10	/* xxx1xxxx */

#define CFG_R1_MAPROM		0x80	/* 1xxxxxxx */
#define CFG_R1_SHADOWROM	0x40	/* x1xxxxxx */
#define CFG_R1_BANKBIT0		0x20	/* xx1xxxxx */
#define CFG_R1_BANKBIT1		0x10	/* xxx1xxxx */

#define DEBUG		1	/* print debug messages */
/*#define FAKECARD	1	 fake the card in memory as normal variable */

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

#ifdef FAKECARD
uint64_t fakecardreg = 0x0;
/*uint64_t fakecardreg = 0xFFFFFFFFFFFFFF; */
void *cardaddr = &fakecardreg;  
#else
uint8_t *cardaddr = CFG_ADDRESS;
#endif /* FAKECARD */

struct flags_to_regs toggles[] = {
	{ 0, 0, 1, CFG_R0_OFFSET, CFG_R0_DISABLE }, /* enable/disable020 */
	{ 0, 0, 0, CFG_R0_OFFSET, CFG_R0_PCMCIA },  /* pcmciamodeon/pcmciamodeoff */
	{ 0, 0, 1, CFG_R0_OFFSET, CFG_R0_CACHEOFF }, /* instcacheon/instcacheoff */
	{ 0, 0, 1, CFG_R0_OFFSET, CFG_R0_WRITELOCKOFF }, /* instcacheon/instcacheoff */
	{ 0, 0, 0, CFG_R1_OFFSET, CFG_R1_MAPROM }, /* mapromon/mapromoff */
	{ 0, 0, 0, CFG_R1_OFFSET, CFG_R1_SHADOWROM }, /* shadowromon/shadowromoff */
	{ -1, -1, 0, 0, 0 } /* end */
};


void
usage(void) 
{
	printf("usage: 9tcfg [--disable020|--enable020] [--instcacheoff|--instcacheon] [--pcmciamodeoff|--pcmciamodeon] [--writelockoff|--writelockon] [--mapromoff|--mapromon] [--shadowromoff|--shadowromon] [--customaddress=0xADDRESS]\n");
}

uint8_t
cfgreg_read(uint8_t offset) 
{
	uint8_t v;
	uint8_t *ptr;

	ptr = cardaddr + offset;
	v = *ptr;
#ifdef DEBUG
	printf("DEBUG: read %x from %p\n", v, ptr);
#endif /* DEBUG */

	return v;
}

void
cfgreg_write(uint8_t offset, uint8_t value)
{
	uint8_t *ptr;

	ptr = cardaddr + offset;
#ifdef DEBUG
	printf("DEBUG: write %x to %p\n", value, ptr);
#endif /* DEBUG */
	*ptr = value;
}

void
cfgreg_set(uint8_t offset, uint8_t bits)
{
	uint8_t v;
	
	v = cfgreg_read(offset) | bits;
	cfgreg_write(offset, v);
}

void
cfgreg_unset(uint8_t offset, uint8_t bits)
{
	uint8_t v;

	v = cfgreg_read(offset) & ~bits;
	cfgreg_write(offset, v);
}

void
cfgreg_display(void) 
{
	uint8_t r0, r1;

	r0 = cfgreg_read(CFG_R0_OFFSET);
	r1 = cfgreg_read(CFG_R1_OFFSET);

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

	printf("\tShadow ROM: ");
	if (r1 & CFG_R1_SHADOWROM)
		printf("enabled\n");
	else
		printf("disabled\n");

	printf("\tBank bits: ");
	if (r1 & CFG_R1_BANKBIT0)
		printf("1");
	else
		printf("0");
	if (r1 & CFG_R1_BANKBIT1)
		printf("1");
	else
		printf("0");
	printf("\n");

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

int
main(int argc, char *argv[])
{
	int ch;

	bool flag_maprombank = 0; uint8_t maprombank_number;
	bool flag_customaddress = 0; uint32_t customaddress = 0;

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
		{ "maprombank",		required_argument, NULL, 'b' },
		{ "customaddress",	required_argument, NULL, 'a' },
		{ NULL,			0,		NULL,	0 }
	};

	while ((ch = getopt_long(argc, argv, "depPIiulMmSsbah?:", longopts, NULL)) != -1) {
		switch (ch) {
		case 'b':
			flag_maprombank = 1;
			maprombank_number = atoi(optarg);
			break;
		case 'a':
			flag_customaddress = 1;
			customaddress = atoi(optarg); /* XXX: strtoul*/
			break;
		case 0:
			break;
		case '?':
		case 'h':
		default:
			usage();
		}

	}
	argc -= optind;
	argv += optind;

	if (flag_customaddress)
		cardaddr = (void*) customaddress;

	flag_toggle();

	cfgreg_display();
}

