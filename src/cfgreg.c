#include <stdio.h>
#include <stdlib.h>

#include <exec/types.h>

#include "hardware.h"
#include "config.h"

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

