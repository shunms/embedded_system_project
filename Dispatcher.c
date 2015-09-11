/*=============== D i s p a t c h e r . c ===============*/

/*
BY:     Sudha Shunmugam
        Student ID : 00805861
        Embedded Real Time Systems
        Computer Engineering Dept.
        UMASS Lowell
*/

/*
PURPOSE
Dispatches the parsers ouput to corresponding modules


CHANGES
04-12-2003 - Created
*/


#include "assert.h"
#include "Globals.h"

#include "SMC1Driver.h"
#include "MemMgr.h"

#include "PktParser.h"
#include "Framer.h"
#include "Pet.h"
#include "Region.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

#define CmdAck      0x04
#define CmdAckLen   7

/* Allocate Framer Task stack space */
OS_STK            DispatcherStk[TASK_STK_SIZE];

/* Message queues */
void *dispatcherMsg[PoolSize];    /* -- Space for output queue messages */

/* Message Queues */
OS_EVENT    *dispatcherQueue;    /* -- Passes buffers from processing to output */

/*----- f u n c t i o n    p r o t o t y p e s -----*/

void Dispatcher (void *data);

/*--------------- C r e a t e D i s p a t c h e r T a s k ( ) ---------------*/

/*
PURPOSE
Create and initialize the output task.

GLOBALS
outputQueue    -- Provides full buffers from the input task.
bfrAvail        -- Semaphore to allow buffer requests when pool is not empty
*/
void CreateDispatcherTask(void)
{
    INT32U    idx;    /* -- Index to task stacks */
    INT8U        osErr;/* -- uCos Error Code */   

    /*
    Fill stacks with known values; good for seeing how much stack space
    is used.
    */
    for (idx = 0; idx < TASK_STK_SIZE; idx++)
        DispatcherStk[idx] = 0xE0E1E2E3;
        
    /* Create the output queue. */
    dispatcherQueue = OSQCreate(dispatcherMsg, PoolSize);
    assert(dispatcherQueue != NULL);
    
    /* Create Framer task. */    
    osErr = OSTaskCreate (Dispatcher,
                                    NULL,
                                    (void *)&DispatcherStk[TASK_STK_SIZE],
                                    DispatcherPrio);
    assert(dispatcherQueue != NULL);
}
   
/*--------------- D i s p a t c h e r ( ) ---------------*/

/*
PURPOSE
Get buffers from the parser task and sends it to the corresponding module.

INPUT PARAMETERS
data         --    task data pointer (not used)

GLOBALS
dispatcherQueue    -- Provides full buffers from the input task.
*/
void Dispatcher (void *data)
{
    INT8U        osErr;      /* -- uCos Error Code */    
    Payload *oBfr = NULL;    /* -- Current output buffer */
    Payload *dBfr = NULL;
    
    /* Task runs forever, or until preempted or suspended. */
    for(;;) {
        /* If no output buffer is assigned, get one from the output queue. */
        if (dBfr == NULL) {
            dBfr = OSQPend(dispatcherQueue, 0, &osErr);
            assert(osErr==OS_NO_ERR);
        }
        
        if (oBfr == NULL)
            oBfr = Allocate();

        *oBfr = *dBfr;
            
        switch(dBfr->cmdType) {
          case CmdPetLocation:
              /* If pet valid and not defined create it now */
              if(IsPetValid(oBfr->dataPart.petNum)) {
                  if(IsPetNotDefined(oBfr->dataPart.petNum))
                      CreatePetTask(oBfr->dataPart.petNum);
                  SendPetQ(oBfr);
              }else 
                  SendPetInvalid(oBfr);
            oBfr = NULL;
            break;
          case CmdFindPet:
            if(!IsPetValid(oBfr->dataPart.petNum)) 
                SendPetInvalid(oBfr);
            else if(IsPetNotDefined(oBfr->dataPart.petNum)) 
                SendPetNotDefined(oBfr);
            else
                SendPetQ(oBfr);
            oBfr = NULL;
            break;
          case CmdDefRegion:
            /* If region valid and not defined then create it now */
            if(IsRegionValid(oBfr->dataPart.regionNum)) {
                if(IsRegionNotDefined(oBfr->dataPart.regionNum)) {
                    CreateRegionTask(oBfr->dataPart.regionNum);
                    SendRegionQ(oBfr);
                } else {
                    SendRegionAlreadyDefined(oBfr);
                }              
            } else
                SendRegionInvalid(oBfr);
            oBfr = NULL;
            break;
          case CmdReportRegion:
            if(!IsRegionValid(oBfr->dataPart.regionNum))
                SendRegionInvalid(oBfr);
            else if(IsRegionNotDefined(oBfr->dataPart.regionNum))
                SendRegionNotDefined(oBfr);
            else
                SendRegionQ(oBfr);
            oBfr = NULL;
            break;
          default:
            oBfr->cmdType = Ack;
            oBfr->dataLength = AckLen;
            oBfr->dataPart.regionNum = dBfr->cmdType;
            SendFramerQ(oBfr);
            oBfr = NULL;            
        }
        
        Free(dBfr);
        dBfr = NULL;
        
    }
}

void SendDispatcherQ(Payload *payload)
{
    INT8U osErr;            /* -- OS Error code */
  
    osErr = OSQPost(dispatcherQueue, payload);
    assert(osErr==OS_NO_ERR);
    
    return;
}
