/*--------------- PetMon.c ---------------*/
/*
BY:     Sudha Shunmugam
        Student ID : 00805861
        Embedded Real Time Systems
        Computer Engineering Dept.
        UMASS Lowell
*/

/*
PURPOSE
Pet Monitor project to process the commands and send corresponsing replies 
and alarms.

CHANGES
04-12-2006 - Created.
*/

/*----- i n c l u d e s -----*/

#include "assert.h"
#include "Globals.h"
#include "MemMgr.h"
#include "SMC1Driver.h"

#include "Framer.h"
#include "PktParser.h"
#include "Dispatcher.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

/* Size of the stacks for each task */
#define	TASK_STK_SIZE     2048 

/* Stack space for four tasks */
OS_STK			InitStk[TASK_STK_SIZE];

/*------- f u n c t i o n    p r o t o t y p e s -----*/

void Init (void * data);


/*--------------- m a i n ( ) ---------------*/

int main (void)
{
	INT32S idx;       /* -- Index to task stacks */
	INT8U osErr;      /* -- OS Error code */

	/*
	Fill stacks with known values; good for seeing how much stack space
	is used.
	*/
	for (idx = 0; idx < TASK_STK_SIZE; idx++)
		InitStk[idx] = 0xE0E1E2E3;

	/* Initialize the UCOS-II Kernel. */
	OSInit ();

	/* Create the Task Control Block (TCB) for the startup task. */
	osErr = OSTaskCreate (	Init, NULL, (void *)&InitStk[TASK_STK_SIZE],
                            InitPrio);
	assert(osErr==OS_NO_ERR);

	/*	Start multi-tasking. */
	OSStart();
    
	/* No, it never returns... */ 	
	return 0;
}


/*--------------- I n i t ( ) ---------------*/

/*
PURPOSE
Perform O/S and application initialization.
Create OS resources, create tasks, and then go away.

INPUT PARAMETERS
data		-- pointer to task data (not used)

*/
void Init (void *data)
{
	INT8U osErr;			/* -- OS Error code */

	/* Start the O/S clock tick. */
	ticker_start();

	/* Initialize the O/S statistics task. */
#if OS_TASK_STAT_EN
	OSStatInit();
#endif

    InitMemMgr();
    InitSMC1Driver();

    /* Create Parser and Dispatcher and Framer Tasks */
    CreateFramerTask();
    CreateParserTask();
    CreateDispatcherTask();

    /* This task is done, so it deletes itself. */
    osErr = OSTaskDel(InitPrio);
    assert(osErr == OS_NO_ERR);
}

