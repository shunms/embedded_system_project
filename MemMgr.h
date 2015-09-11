/*=============== M e m M g r . h ===============*/

/*
BY:	George Cheney
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell

PURPOSE
Interface to MemMgr.c.

CHANGES
04-07-2003 gpc - Release to 16.472 / 16.572 class.
04-07-2003 gpc - Add assert to timeout Pends.
04-08-2003 gpc - Remove typing dependent timeouts.
03-28-2005 gpc - Split into separate modules.
03-29-2006 gpc - Update for S2006
*/

#ifndef MEMMGR_H
#define MEMMGR_H

#include "PktParser.h"
#include "Globals.h"
/*----- c o n s t a n t    d e f i n i t i o n s -----*/

/* Total number of buffers in buffer pool */
#define PoolSize	25

/*----- g l o b a l s -----*/

/* The buffer pool */
extern OS_MEM *bfrPool;

/*----- f u n c t i o n    p r o t o t y p e s -----*/

void InitMemMgr(void);

Payload *Allocate(void);

void Free(Payload *bfr);

#endif
