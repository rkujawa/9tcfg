/* Various CPU/memory options */

#include <stdio.h>

#include <exec/types.h>

#include "config.h"
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
pcmciamode_enable()
{
	cfgreg_set(CFG_R0_OFFSET, CFG_R0_PCMCIA);
}

void
pcmciamode_disable()
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

void 
instcache_reset_enable()
{
	cfgreg_set(CFG_R1_OFFSET, CFG_R1_INSTCACHERESET);
}

void 
instcache_reset_disable()
{
	cfgreg_unset(CFG_R1_OFFSET, CFG_R1_INSTCACHERESET);
}

