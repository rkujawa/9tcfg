/* Various CPU/memory options */

#include <stdio.h>

#include <exec/types.h>

#include "hardware.h"
#include "cfgreg.h"
#include "cpu.h"

void
cpu_68k_enable()
{
	cfgreg_set(CFG_R0_OFFSET, CFG_R0_68KMODE);
}

void
cpu_68k_disable()
{
	cfgreg_unset(CFG_R0_OFFSET, CFG_R0_68KMODE);
}

BOOL
cpu_68k_get()
{
	return cfgreg_read(CFG_R0_OFFSET) & CFG_R0_68KMODE;
}

void
cpu_68kfast_enable()
{
	cfgreg_set(CFG_R0_OFFSET, CFG_R0_68KMEMORYMODE);
}

void
cpu_68kfast_disable()
{
	cfgreg_unset(CFG_R0_OFFSET, CFG_R0_68KMEMORYMODE);
}

void
pcmcia_enable()
{
	cfgreg_set(CFG_R0_OFFSET, CFG_R0_PCMCIA);
}

void
pcmcia_disable()
{
	cfgreg_unset(CFG_R0_OFFSET, CFG_R0_PCMCIA);
}

void 
instcache_disable()
{
	cfgreg_set(CFG_R1_OFFSET, CFG_R1_INSTCACHEOFF);
}

void 
instcache_enable()
{
	cfgreg_unset(CFG_R1_OFFSET, CFG_R1_INSTCACHEOFF);
}

