/*=============== F r a m e r . c ===============*/

/*
BY:     Sudha Shunmugam
        Student ID : 00805861
        Embedded Real Time Systems
        Computer Engineering Dept.
        UMASS Lowell
*/

/*
PURPOSE
Frames the output character and sends it to the PC


CHANGES
04-07-2003 gpc - Release to 16.472 / 16.572 class.
04-07-2003 gpc - Add assert to timeout Pends.
04-08-2003 gpc - Remove typing dependent timeouts.
03-28-2005 gpc - Split into separate modules.
03-29-2006 gpc - Update for S2006
*/


#include "assert.h"
#include "Globals.h"

#include "SMC1Driver.h"
#include "MemMgr.h"

#include "PktParser.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/


#define Err      0x05
#define CmdErr      6

/* Allocate Framer Task stack space */
OS_STK			FramerStk[TASK_STK_SIZE];

/* Message queues */
void *framerMsg[PoolSize];	/* -- Space for output queue messages */

/* Message Queues */
OS_EVENT	*framerQueue;	/* -- Passes buffers from processing to output */

/*----- f u n c t i o n    p r o t o t y p e s -----*/

void Framer (void *data);

/*--------------- C r e a t e F r a m e r T a s k ( ) ---------------*/

/*
PURPOSE
Create and initialize the Framer task.

GLOBALS
framerQueue	-- Provides full buffers from the input task.
*/
void CreateFramerTask(void)
{
   INT32U	idx;	/* -- Index to task stacks */
	INT8U		osErr;/* -- uCos Error Code */   

	/*
	Fill stacks with known values; good for seeing how much stack space
	is used.
	*/
	for (idx = 0; idx < TASK_STK_SIZE; idx++)
		FramerStk[idx] = 0xE0E1E2E3;
		
	/* Create the output queue. */
	framerQueue = OSQCreate(framerMsg, PoolSize);
	assert(framerQueue != NULL);
	
	/* Create Framer task. */	
	osErr = OSTaskCreate (	Framer,
									NULL,
									(void *)&FramerStk[TASK_STK_SIZE],
									FramerPrio);
	assert(framerQueue != NULL);
}
   
/*--------------- F r a m e r ( ) ---------------*/

/*
PURPOSE
Get buffers from the output quese, and empty them to the SMC1 tx.

INPUT PARAMETERS
data 		--	task data pointer (not used)

GLOBALS
framerQueue	-- Provides full buffers from the input task.
*/
void Framer (void *data)
{
    INT8U		osErr;			/* -- uCos Error Code */  
    
    char c = 0;            /* -- The next character to output */
	
    Payload	*oBfr = NULL;	/* -- Current output buffer */
    
    /* Task runs forever, or until preempted or suspended. */
    for(;;)
    {
        /* If no output buffer is assigned, get one from the output queue. */
        if (oBfr == NULL)
        {
          oBfr = OSQPend(framerQueue, 0, &osErr);  /* 04-08-2003 gpc */
          assert(osErr==OS_NO_ERR);
        }
		
        /* Framer character to tx. */
        OutByte((INT8U)PREAMBLE_BYTE1);
        /* Framer character to tx. */
        OutByte((INT8U)PREAMBLE_BYTE2);
        /* Framer character to tx. */
        OutByte((INT8U)PREAMBLE_BYTE3);
        switch(oBfr->cmdType) {
            case CmdErr:
                /* Calculate checksum */
                c = (INT8U)PREAMBLE_BYTE1^PREAMBLE_BYTE2^PREAMBLE_BYTE3^
                  AckLen ^ Err ^ oBfr->dataLength;
                OutByte(c);
                OutByte(AckLen);
                OutByte(Err);
                c = oBfr->dataLength;
                OutByte(c);
              break;
            case Ack:
                /* Calculate checksum */
                c = (INT8U)PREAMBLE_BYTE1^PREAMBLE_BYTE2^PREAMBLE_BYTE3^
                  oBfr->dataLength ^ oBfr->cmdType ^ oBfr->dataPart.regionNum;
                OutByte(c);
                OutByte(oBfr->dataLength);
                OutByte(oBfr->cmdType);
                c = oBfr->dataPart.regionNum;
                OutByte(c);
                break;
            case PetLocationRep:
                c = (INT8U)PREAMBLE_BYTE1^PREAMBLE_BYTE2^PREAMBLE_BYTE3^
                  oBfr->dataLength ^ oBfr->cmdType ^ 
                  oBfr->dataPart.petDetails[0] ^
                  oBfr->dataPart.petDetails[1] ^
                  oBfr->dataPart.petDetails[2] ;
                OutByte(c);
                OutByte(oBfr->dataLength);
                OutByte(oBfr->cmdType);
                OutByte(oBfr->dataPart.petDetails[0]);                
                OutByte(oBfr->dataPart.petDetails[1]);                
                OutByte(oBfr->dataPart.petDetails[2]);                
                break;
            case RegionLocationRep:
                c = (INT8U)PREAMBLE_BYTE1^PREAMBLE_BYTE2^PREAMBLE_BYTE3^
                  oBfr->dataLength ^ oBfr->cmdType ^ 
                  oBfr->dataPart.regionDetails[0] ^
                  oBfr->dataPart.regionDetails[1] ^
                  oBfr->dataPart.regionDetails[2] ^
                  oBfr->dataPart.regionDetails[3] ^
                  oBfr->dataPart.regionDetails[4];
                OutByte(c);
                OutByte(oBfr->dataLength);
                OutByte(oBfr->cmdType);
                OutByte(oBfr->dataPart.regionDetails[0]);
                OutByte(oBfr->dataPart.regionDetails[1]);
                OutByte(oBfr->dataPart.regionDetails[2]);
                OutByte(oBfr->dataPart.regionDetails[3]);
                OutByte(oBfr->dataPart.regionDetails[4]);
                    
                break;
            case RegionViolationRep:
                 c = (INT8U)PREAMBLE_BYTE1^PREAMBLE_BYTE2^PREAMBLE_BYTE3^
                  oBfr->dataLength ^ oBfr->cmdType ^ 
                  oBfr->dataPart.regionDetails[0] ^
                  oBfr->dataPart.regionDetails[1] ^
                  oBfr->dataPart.regionDetails[2] ^
                  oBfr->dataPart.regionDetails[3] ;
                 OutByte(c);
                OutByte(oBfr->dataLength);
                OutByte(oBfr->cmdType);
                OutByte(oBfr->dataPart.regionDetails[0]);
                OutByte(oBfr->dataPart.regionDetails[1]);
                OutByte(oBfr->dataPart.regionDetails[2]);
                OutByte(oBfr->dataPart.regionDetails[3]);
                 
                break;
            default:
                
              break;
        }
        Free(oBfr);
        oBfr = NULL;
    }
}

void SendFramerQ(Payload *payload)
{
    INT8U osErr;            /* -- OS Error code */
  
    osErr = OSQPost(framerQueue, payload);
    assert(osErr==OS_NO_ERR);
    
    return;
}
