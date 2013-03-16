#include <exec/types.h>

#ifndef _CFGREG_H_
#define _CFGREG_H_

UBYTE cfgreg_read(UBYTE offset);
void cfgreg_write(UBYTE offset, UBYTE value);
void cfgreg_set(UBYTE offset, UBYTE bits);
void cfgreg_unset(UBYTE offset, UBYTE bits);

void cfgreg_unlock(void);
void cfgreg_lock(void);

BYTE ninetails_detect(void);

#endif /* _CFGREG_H_ */

