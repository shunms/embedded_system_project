/*=============== P e t . c ===============*/

/*
BY:     Sudha Shunmugam
        Student ID : 00805861
        Embedded Real Time Systems
        Computer Engineering Dept.
        UMASS Lowell
*/

/*
PURPOSE
To Maintain a pet


CHANGES
04-20-2003 sudha - created
*/


#include "assert.h"
#include "Globals.h"

#include "SMC1Driver.h"
#include "MemMgr.h"

#include "Pet.h"
#include "Framer.h"
#include "Region.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/



/* Allocate Framer Task stack space */
OS_STK            PetStk[MAX_PETS][TASK_STK_SIZE];

/* Message queues */
void *PetMsg[PoolSize];    /* -- Space for output queue messages */

INT8U gPetNum;

typedef struct {
    INT8U xLoc;
    INT8U yLoc;
    OS_EVENT *petQueue;
    INT8U petNum;
}PetStatus;

PetStatus gPetStatus[MAX_PETS]= {{0}};

/*----- f u n c t i o n    p r o t o t y p e s -----*/

void Pet (void *data);
INT8U SendAck(INT8U cmdType);
void SendPetLocation(INT8U petNum);

/*--------------- C r e a t e F r a m e r T a s k ( ) ---------------*/

/*
PURPOSE

Create and initialize the Framer task.

GLOBALS
framerQueue    -- Provides full buffers from the input task.
*/
void CreatePetTask(INT8U petNum)
{
    INT32U    idx;    /* -- Index to task stacks */
    INT8U osErr;/* -- uCos Error Code */  
    INT8U petIndex = petNum-1;     
    
    /*
    Fill stacks with known values; good for seeing how much stack space
    is used.
    */    
    for (idx = 0; idx < TASK_STK_SIZE; idx++)
        PetStk[petIndex][idx] = 0xE0E1E2E3;
        
    /* Create the output queue. */
    gPetStatus[petIndex].petQueue = OSQCreate(PetMsg, PoolSize);
    assert(gPetStatus[petIndex].petQueue != NULL);
    
    gPetNum = petNum;
    /* Create Framer task. */    
    osErr = OSTaskCreate (Pet, &gPetNum, 
            (void *)&PetStk[petIndex][TASK_STK_SIZE], 
            PetPrio+petIndex);
    assert(osErr==OS_NO_ERR);
}
   
/*--------------- P e t ( ) ---------------*/

/*
PURPOSE
Get buffers from the output quese, and empty them to the SMC1 tx.

INPUT PARAMETERS
data         --    task data pointer (not used)

GLOBALS
framerQueue    -- Provides full buffers from the input task.
*/
void Pet (void *data)
{
    INT8U        osErr;            /* -- uCos Error Code */  
    Payload        *petBfr = NULL;    /* -- Current pet buffer */        
    INT8U petNum;
        
    petNum = *((INT8U *)data);
    
    /* Task runs forever, or until preempted or suspended. */
    for(;;)
    {
        /* If no output buffer is assigned, get one from the output queue. */
        if (petBfr == NULL)
        {
            petBfr = OSQPend(gPetStatus[petNum-1].petQueue, 0, &osErr);  
            assert(osErr==OS_NO_ERR);
        }

        switch(petBfr->cmdType) {
            case CmdPetLocation:
                SendAck(petBfr->cmdType);
                gPetStatus[petBfr->dataPart.petNum-1].xLoc = petBfr->dataPart.petDetails[1];
                gPetStatus[petBfr->dataPart.petNum-1].yLoc = petBfr->dataPart.petDetails[2];
                /*Forward the same petBfr to head region task */
                SendToHeadRegion(petBfr);
                break;
            case CmdFindPet:
                SendAck(petBfr->cmdType);
                SendPetLocation(petBfr->dataPart.petNum);
                Free(petBfr);
                break;
        }       
        petBfr = NULL;
        
    }
}

INT8U SendAck(INT8U cmdType)
{
    Payload *oBfr = NULL;
    if (oBfr == NULL)
        oBfr = Allocate();

    oBfr->cmdType = Ack;
    oBfr->dataLength = AckLen;
    oBfr->dataPart.regionNum = cmdType;
        SendFramerQ(oBfr);
    oBfr = NULL;
    
    return(1);    
}

void SendPetLocation(INT8U petNum)
{
    Payload *oBfr = NULL;

    if (oBfr == NULL)
        oBfr = Allocate();

    oBfr->cmdType = PetLocationRep;
    oBfr->dataLength = PetLocationRepLen;
    oBfr->dataPart.petDetails[0] = petNum;
    oBfr->dataPart.petDetails[1] = gPetStatus[petNum-1].xLoc;
    oBfr->dataPart.petDetails[2] = gPetStatus[petNum-1].yLoc;

    SendFramerQ(oBfr);
    oBfr = NULL;
    
    return;    
}

INT8U IsPetNotDefined(INT8U petNum)
{
    if(gPetStatus[petNum-1].petQueue == NULL)
        return(1);
    return(0);
}

void SendPetQ(Payload *payload)
{
    INT8U osErr;            /* -- OS Error code */
    INT8U petNum;

    petNum = payload->dataPart.petNum;
  
    /* Post the message to the corresponding Pet Q */
    osErr = OSQPost(gPetStatus[petNum-1].petQueue, payload);
    assert(osErr==OS_NO_ERR);
    
    return;
}

INT8U IsPetValid(INT8U petNum)
{
    /* Check if pet num is greater than the max pet num */
    if(petNum > MAX_PETS || petNum <= 0)
      return(0);
    return(1);
}

void SendPetInvalid(Payload *oBfr)
{
    /* First sends ack for the command and then sends the error */
     SendAck(oBfr->cmdType);
     
     if(oBfr->cmdType == CmdPetLocation)
         oBfr->dataLength = InvalidPetErr;
     else
         oBfr->dataLength = InvalidFPErr;
     
     oBfr->cmdType = CmdErr;
     
     SendFramerQ(oBfr);
     return;
}

void SendPetNotDefined(Payload *oBfr)
{
    /* First sends ack for the command and then sends the error */
    SendAck(oBfr->cmdType);

    oBfr->cmdType = CmdErr;
    oBfr->dataLength = PetNotDefErr;

    SendFramerQ(oBfr);
    
    return;    
}
