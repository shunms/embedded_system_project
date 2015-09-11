/*=============== M e m M g r . c ===============*/

/*
BY:	George Cheney
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell

PURPOSE
Allocate and deallocate buffers from a pool.

DEMONSTRATES
Memory Management
Buffer Pool

CHANGES
04-07-2003 gpc - Release to 16.472 / 16.572 class.
04-07-2003 gpc - Add assert to timeout Pends.
04-08-2003 gpc - Remove typing dependent timeouts.
03-28-2005 gpc - Split into separate modules.
03-29-2006 gpc - Update for S2006
04-12-006 Sudha - Moved mpc include files to Globals.h
*/


#include "assert.h"
#include "Memmgr.h"

/*----- g l o b a l s -----*/

/* Space for "PoolSize" buffers */
Payload	bfrSpace[PoolSize];

/* Buffer Pool to manage the buffer space */
OS_MEM *bfrPool;

/* Semaphores */
OS_EVENT *bfrAvail;	/* Number of free buffers remaining in the pool */


/*--------------- I n i t M e m M g r ( ) ---------------*/

/*
PURPOSE
Initialize the meory manager.

GLOBALS
bfrPool		-- Pool of free buffers
bfrAvail		-- Semaphore to allow buffer requests when pool is not empty
*/
void InitMemMgr(void)
{
	INT8U		osErr;/* -- uCos Error Code */   
   
	/* Create and initialize semaphores. */
	bfrAvail = OSSemCreate(PoolSize);/* -- Initial buffer pool size. */
	assert(bfrAvail != NULL);

	/* Create a pool of "PoolSize" buffers. */
	bfrPool = OSMemCreate(bfrSpace, PoolSize, sizeof(Payload), &osErr);
	
	assert(osErr == OS_NO_ERR);
}

/*--------------- A l l o c a t e ( ) ---------------*/

/*
PURPOSE
Allocate a buffer from the buffer pool - block if none available.

GLOBALS
bfrPool		-- Pool of free buffers
bfrAvail		-- Semaphore to allow buffer requests when pool is not empty

RETURNS
The allocated buffer address.
*/
Payload *Allocate(void)
{
	INT8U		osErr;/* -- uCos Error Code */   
   
   Payload *bfr;
   
	/* Wait until there is an available buffer. */
	OSSemPend(bfrAvail, SuspendTimeout, &osErr);

	assert(osErr==OS_NO_ERR);
	
	/* Get the buffer from the pool. */
	bfr = (Payload*) OSMemGet(bfrPool, &osErr);
	assert(osErr==OS_NO_ERR);
	
	/* Initialize the buffer pointer. */
	//InitBfr(bfr);
	
	return bfr;
}   	

/*--------------- F r e e ( ) ---------------*/

/*
PURPOSE
Return a buffer to the pool.

INPUT PARAMETERS
bfr   -- the address of the buffer to be returned

GLOBALS
bfrPool		-- Pool of free buffers
bfrAvail		-- Semaphore to allow buffer requests when pool is not empty
*/
void Free(Payload *bfr)
{
	INT8U		osErr;/* -- uCos Error Code */   

   /* Return the buffer. */
	osErr = OSMemPut(bfrPool, bfr);
	assert(osErr==OS_NO_ERR);
	
	/* Indicate that one more buffer is available. */
	osErr = OSSemPost(bfrAvail);
	assert(osErr==OS_NO_ERR);
}   
