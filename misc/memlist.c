#include <stdio.h>

#include <exec/types.h>
#include <exec/execbase.h>

#include <proto/exec.h>

int
main(void)
{
	struct MemHeader *m, *nm;

	for (m  = (void *) SysBase->MemList.lh_Head;
	    nm = (void *) m->mh_Node.ln_Succ; m = nm) {
		printf("%p to %p\n", (void*) m->mh_Lower, (void*) m->mh_Upper);
	}

	return 0;
}

