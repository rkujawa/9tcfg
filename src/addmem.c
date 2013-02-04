#include <stdio.h>
#include <stdlib.h>

#include <exec/types.h>
#include <exec/execbase.h>
#include <proto/exec.h>

#include "hardware.h"
#include "config.h"
#include "addmem.h"

/* add non-autoconfiguring memory to the system */
void
memory_add(void)
{
	if (!memory_check_added(ADDMEM_0_BASE))
		AddMemList(1*1024*1024, MEMF_FAST, ADDMEM_PRI, ADDMEM_0_BASE, "9T A8 RAM");

	if (!memory_check_added(ADDMEM_1_BASE))
		AddMemList(512*1024, MEMF_FAST, ADDMEM_PRI, ADDMEM_1_BASE, "9T F0 RAM");
}

/* check if memory at given address is already added to system memory list */
BOOL
memory_check_added(ULONG address)
{
	struct MemHeader *m, *nm;

	for (m  = (void *) SysBase->MemList.lh_Head;
	    nm = (void *) m->mh_Node.ln_Succ; m = nm) {
		if (address == ( (ULONG) m->mh_Lower & 0xFFFF00)) {
#ifdef DEBUG
			printf("DEBUG: memory at address %p already added\n", (void*) address);
#endif /* DEBUG */
			return 1;
		}
	}
	return 0;
}

