/*=============== R e g i o n . c ===============*/

/*
BY:     Sudha Shunmugam
        Student ID : 00805861
        Embedded Real Time Systems
        Computer Engineering Dept.
        UMASS Lowell
*/

/*
PURPOSE
Region Task to handle the region definition and reporting


CHANGES
04-20-2003 sudha - created
*/


#include "assert.h"
#include "Globals.h"

#include "SMC1Driver.h"
#include "MemMgr.h"

#include "Region.h"
#include "Framer.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/



/* Allocate Framer Task stack space */
OS_STK RegionStk[MAX_REGION][TASK_STK_SIZE];

/* Message queues */
void *RegionMsg[PoolSize];    /* -- Space for output queue messages */


INT8U gRegionNum;


struct RegionDetails {
    INT8U regionNum;
    INT8U xLoc;
    INT8U yLoc;
    INT8U width;
    INT8U height;
    OS_EVENT *regionQueue;
    struct RegionDetails *nextRegion;
};

struct RegionDetails gRegionDetail[MAX_REGION]= {{0}};
struct RegionDetails *head = NULL;

/*----- f u n c t i o n    p r o t o t y p e s -----*/

void Region (void *data);
INT8U SendRegionAck(INT8U cmdType);
void AddToLinkedList(INT8U regionNum);
void SendToNextRegion(INT8U regionNum, Payload *payload);
void SendPetViolationAlarm(INT8U regionNum, Payload *regionBfr);
INT8U PetVoilatesMyRegion(INT8U regionNum, Payload *regionBfr);

/*--------------- C r e a t e F r a m e r T a s k ( ) ---------------*/

/*
PURPOSE
Create and initialize the Framer task.

GLOBALS
*/
void CreateRegionTask(INT8U regionNum)
{
    INT32U idx;    /* -- Index to task stacks */
    INT8U osErr;/* -- uCos Error Code */
    /*
     * Fill stacks with known values; good for seeing how much stack space
     * is used.
     * */
    for (idx = 0; idx < TASK_STK_SIZE; idx++)
      RegionStk[regionNum][idx] = 0xE0E1E2E3;
        
    /* Create the region queue. */
    gRegionDetail[regionNum].regionQueue = OSQCreate(RegionMsg, PoolSize);
    assert(gRegionDetail[regionNum].regionQueue != NULL);
    
    gRegionNum = regionNum;
    
    /* Add the newly created task to the correct place in linked list */
    AddToLinkedList(regionNum);

    /* Create Region task. */    
    osErr = OSTaskCreate (Region,
            &gRegionNum,
            (void *)&RegionStk[regionNum][TASK_STK_SIZE],
            RegionPrio+regionNum);
    assert(osErr==OS_NO_ERR);
}
   
/*--------------- F r a m e r ( ) ---------------*/

/*
PURPOSE
Get buffers from the output quese, and empty them to the SMC1 tx.

INPUT PARAMETERS
data         --    task data pointer (not used)

GLOBALS
*/
void Region (void *data)
{
    INT8U osErr;            /* -- uCos Error Code */  
    Payload *regionBfr = NULL;    /* -- Current Region buffer */        
    INT8U regionNum;
        
    regionNum = *((INT8U *)data);
    
    /* Task runs forever, or until preempted or suspended. */
    for(;;)
    {
        /* If no output buffer is assigned, get one from the output queue. */
        if (regionBfr == NULL)
        {
            regionBfr = OSQPend(gRegionDetail[regionNum].regionQueue, 0, 
                    &osErr);            
            assert(osErr==OS_NO_ERR);
        }

        switch(regionBfr->cmdType) {
            case CmdDefRegion:
                gRegionDetail[regionBfr->dataPart.regionNum].xLoc = 
                    regionBfr->dataPart.regionDetails[1];
                gRegionDetail[regionBfr->dataPart.regionNum].yLoc = 
                    regionBfr->dataPart.regionDetails[2];
                gRegionDetail[regionBfr->dataPart.regionNum].width = 
                    regionBfr->dataPart.regionDetails[3];
                gRegionDetail[regionBfr->dataPart.regionNum].height = 
                    regionBfr->dataPart.regionDetails[4];

                SendRegionAck(regionBfr->cmdType);
                Free(regionBfr);
                break;
            case CmdReportRegion:
                SendRegionAck(regionBfr->cmdType);
                SendRegionLocation(regionBfr->dataPart.regionNum);
                Free(regionBfr);
                break;
            case CmdPetLocation:
                /* If this is a petlocation forwarded by pet check if alarm
                 * needs to be raised */
                if(PetVoilatesMyRegion(regionNum, regionBfr)){
                    SendPetViolationAlarm(regionNum, regionBfr);
                }
                /* Forward this to the next region */
                SendToNextRegion(regionNum, regionBfr);
                break;
            default:
                break;
        }        
        regionBfr = NULL;    
    }
}


INT8U SendRegionAck(INT8U cmdType)
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

void SendRegionLocation(INT8U regionNum)
{
    Payload *oBfr = NULL;

    if (oBfr == NULL)
        oBfr = Allocate();

    oBfr->cmdType = RegionLocationRep;
    oBfr->dataLength = RegionLocationRepLen;
    oBfr->dataPart.regionDetails[0] = regionNum;
    oBfr->dataPart.regionDetails[1] = gRegionDetail[regionNum].xLoc;
    oBfr->dataPart.regionDetails[2] = gRegionDetail[regionNum].yLoc;
    oBfr->dataPart.regionDetails[3] = gRegionDetail[regionNum].width;
    oBfr->dataPart.regionDetails[4] = gRegionDetail[regionNum].height;

    SendFramerQ(oBfr);
    oBfr = NULL;
    
    return;    
}

INT8U PetVoilatesMyRegion(INT8U regionNum, Payload *regionBfr)
{
    /* Check if pet voilates the given region */
    INT8U petXLoc = regionBfr->dataPart.petDetails[1];
    INT8U petYLoc = regionBfr->dataPart.petDetails[2];
    INT8U regionXLoc = gRegionDetail[regionNum].xLoc;
    INT8U regionYLoc = gRegionDetail[regionNum].yLoc;
    INT8U regionWidth = gRegionDetail[regionNum].width;
    INT8U regionHeight = gRegionDetail[regionNum].height;
   
    if (regionNum == 0) {
      if( (petXLoc > (regionXLoc + regionWidth)) || 
          (petYLoc > (regionYLoc + regionHeight))) {
          /* Yes there is a violation */
          return(1);
      }
    }
    else {
      if(((petXLoc >= regionXLoc) && (petXLoc <= (regionXLoc + regionWidth)))
              && 
       ((petYLoc >= regionYLoc) && (petYLoc <= (regionYLoc + regionHeight)))) {
          /* Yes there is a violation */
          return(1);
      }
    }
    return(0);
}

void SendPetViolationAlarm(INT8U regionNum, Payload *regionBfr)
{
    Payload *oBfr = NULL;

    if (oBfr == NULL)
        oBfr = Allocate();

    oBfr->cmdType = RegionViolationRep;
    oBfr->dataLength = RegionViolationRepLen;
    oBfr->dataPart.regionDetails[0] = regionBfr->dataPart.petDetails[0];
    oBfr->dataPart.regionDetails[1] = regionBfr->dataPart.petDetails[1];
    oBfr->dataPart.regionDetails[2] = regionBfr->dataPart.petDetails[2];
    oBfr->dataPart.regionDetails[3] = regionNum;

    SendFramerQ(oBfr);
    oBfr = NULL;
    
    return;    
}

INT8U IsRegionValid(INT8U regionNum)
{
    /* Check if region num is greater than the max region num */
    if(regionNum > MAX_REGION)
      return(0);
    return(1);
}

INT8U IsRegionNotDefined(INT8U regionNum)
{
    /* If region not defined send 1 else send 0 */
    if(gRegionDetail[regionNum].regionQueue == NULL)
        return(1);
    return(0);
}

void SendRegionInvalid(Payload *oBfr)
{
    /* First sends ack for the command and then sends the error */
     SendRegionAck(oBfr->cmdType);
     
     if(oBfr->cmdType == CmdDefRegion)
         oBfr->dataLength = InvalidDefineRegErr;
     else
         oBfr->dataLength = InvalidReportRegErr; 

     oBfr->cmdType = CmdErr;

     SendFramerQ(oBfr);
     return;
}

void SendRegionNotDefined(Payload *oBfr)
{
    /* First sends ack for the command and then sends the error */
    SendRegionAck(oBfr->cmdType);

    oBfr->cmdType = CmdErr;
    oBfr->dataLength = RegionNotDefErr;

    SendFramerQ(oBfr);
    
    return;    
}

void SendRegionAlreadyDefined(Payload *oBfr)
{
    /* First sends ack for the command and then sends the error */
    SendRegionAck(oBfr->cmdType);

    oBfr->cmdType = CmdErr;
    oBfr->dataLength = RegionAlreadyDefErr;

    SendFramerQ(oBfr);
    
    return;    
}

void SendRegionQ(Payload *payload)
{
    INT8U osErr;            /* -- OS Error code */
    INT8U regionNum;

    regionNum = payload->dataPart.regionNum;
  
    /* Post the message to the corresponding Pet Q */
    osErr = OSQPost(gRegionDetail[regionNum].regionQueue, payload);
    assert(osErr==OS_NO_ERR);
    
    return;
}

void AddToLinkedList(INT8U regionNum)
{
    struct RegionDetails *temp = NULL;
    
    /* If head is NULL this is the first region task getting created */
    if(head == NULL) {
        head = &gRegionDetail[regionNum];
    } else {
        /* Find the correct place to add this task */
        temp = head;
        while(temp->nextRegion != NULL && 
            temp->nextRegion->regionNum < regionNum) {
            temp = temp->nextRegion;
        }
        gRegionDetail[regionNum].nextRegion = temp->nextRegion;
        temp->nextRegion = &gRegionDetail[regionNum];        
    }    
}

void SendToHeadRegion(Payload *payload)
{
     INT8U osErr;            /* -- OS Error code */
    /* Do this only if there is atleast one region task defined */
    if(head != NULL) {        
          /* Post the message to the head region Q */
        osErr = OSQPost(head->regionQueue, payload);
        assert(osErr==OS_NO_ERR); 
    }

}

void SendToNextRegion(INT8U regionNum, Payload *payload)
{
     INT8U osErr;            /* -- OS Error code */

    /*If there is a next region send it to the next region */
    if(gRegionDetail[regionNum].nextRegion != NULL) {
        /* Post the message to the corresponding Pet Q */
        osErr = OSQPost(gRegionDetail[regionNum].nextRegion->regionQueue, 
                payload);
        assert(osErr==OS_NO_ERR);        
    } else {
        /* free the buffer */
        Free(payload);
        payload = NULL;
    }

    return;
}


