#include <stdio.h>
#include <stdlib.h>

#include <exec/types.h>

#include "hardware.h"
#include "config.h"

#pragma dontwarn 113

static UBYTE *cardaddr = CFG_ADDRESS;

/* read register at offset */
UBYTE
cfgreg_read(UBYTE offset) 
{
	UBYTE v;
	UBYTE *ptr;

	ptr = cardaddr + offset;
	v = *ptr;
#ifdef DEBUG
	printf("DEBUG: read %x from %p\n", (int) v, (void*) ptr);
#endif /* DEBUG */

	return v;
}

/* write register at offset */
void
cfgreg_write(UBYTE offset, UBYTE value)
{
	UBYTE *ptr;

	ptr = cardaddr + offset;
#ifdef DEBUG
	printf("DEBUG: write %x to %p\n", (int) value, (void*) ptr);
#endif /* DEBUG */
	*ptr = value;
}

/* set bit in register at offset */
void
cfgreg_set(UBYTE offset, UBYTE bits)
{
	UBYTE v;
	
	v = cfgreg_read(offset) | bits;
	cfgreg_write(offset, v);
}

/* unset bit in register at offset */
void
cfgreg_unset(UBYTE offset, UBYTE bits)
{
	UBYTE v;

	v = cfgreg_read(offset) & ~bits;
	cfgreg_write(offset, v);
}

void
cfgreg_unlock(void) 
{
#ifdef DEBUG
	printf("DEBUG: unlocking registers\n");
#endif /* DEBUG */
	/* reset the FSM */
	cfgreg_write(CFG_LOCK0_OFFSET, 0);

	/* sequence to unlock registers */
	cfgreg_write(CFG_LOCK3_OFFSET, 0x60);
	cfgreg_write(CFG_LOCK2_OFFSET, 0xA0);
	cfgreg_write(CFG_LOCK1_OFFSET, 0x50);
	cfgreg_write(CFG_LOCK0_OFFSET, 0x90);
}

void
cfgreg_lock(void)
{
#ifdef DEBUG
	printf("DEBUG: locking registers\n");
#endif /* DEBUG */
	cfgreg_write(CFG_LOCK0_OFFSET, 0);
}

BYTE
ninetails_detect(void)
{
	UBYTE l0, l1, l2, l3;
#ifdef DEBUG
	printf("DEBUG: running detection procedure\n");
#endif /* DEBUG */
	l0 = cfgreg_read(CFG_LOCK0_OFFSET) & CFG_LOCK_ID_MASK;	
	l1 = cfgreg_read(CFG_LOCK1_OFFSET) & CFG_LOCK_ID_MASK;	
	l2 = cfgreg_read(CFG_LOCK2_OFFSET) & CFG_LOCK_ID_MASK;	
	l3 = cfgreg_read(CFG_LOCK3_OFFSET) & CFG_LOCK_ID_MASK;	

	if ((l0 == CFG_LOCK0_ID_REV0) &&
	    (l1 == CFG_LOCK1_ID_REV0) &&
	    (l2 == CFG_LOCK2_ID_REV0) &&
	    (l3 == CFG_LOCK3_ID_REV0))
		return 0;

	return -1;
}

