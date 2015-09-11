/*--------------- PktParser.c ---------------*/
/*
BY:     Sudha Shunmugam
        Student ID : 00805861
        Embedded Real Time Systems
        Computer Engineering Dept.
        UMASS Lowell
*/

/*
PURPOSE
Generic parser which continuously reads a single byte and changes state 
according to the algorithm specified[Ref - State Diagram]

CHANGES
01-27-2006 Created.
02-03-2006 Added CheckSum() to avoid redundant code
02-07-2006 Added SetErrorState() to decrease the parser function size
02-09-2006 Added some comments
01-15-2006 Did changes for running in the MPC860 Board
02-20-2006 Made changes for Prog3
02-25-2006 Corrected the errors from Prog2
03-08-2006 Changes for Prog4 - Interrupt IO
04-12-2006 Using this for Project - Using InByte to read a character from SMC1
*/

/*----- i n c l u d e s -----*/
#include "PktParser.h"

#include "assert.h"
#include "Globals.h"
#include "MemMgr.h"
#include "SMC1Driver.h"

#include "Framer.h"
#include "Dispatcher.h"

/*----- t y p e    d e f i n i t i o n s -----*/

typedef enum {PREAMBLE1_ERR=1, PREAMBLE2_ERR, PREAMBLE3_ERR,CHECKSUM_ERR, 
              LENGTH_ERR} ParserErrors;


/*----- c o n s t a n t s -----*/

#define LEN_CUTOFF      6
#define MaxCommands 5



/* Stack space for Process task stack */
OS_STK ParserStk[TASK_STK_SIZE];

/* To avoid structures Byte alignment problem */
#pragma pack (1)

/* Packet Buffer Structure */
typedef struct
{
    INT8U dataLen;
    INT8U pktType;
    INT8U data[1];
}PktBfr;


/*----- L o c a l  f u n c t i o n  d e f i n i t i o n s -----*/
INT8U CheckSum(PktBfr *pktBuf);
void SetErrorState(ParserState *parserState, ParserErrors *parserErrors,
    INT8U errVal);
INT8U SetState(PktBfr *pktBuf,ParserState *parserState, 
    ParserErrors *parserErrors, INT8U csVal);

void Parser(void *data);


/*--------------- C r e a t e P a r s e r T a s k ( ) ---------------*/

/*
PURPOSE
Create and initialize the Process task.

*/
void CreateParserTask(void)
{
   INT32U idx;	/* -- Index to task stacks */
	INT8U osErr;/* -- OS Error code */

	/*
	Fill stacks with known values; good for seeing how much stack space
	is used.
	*/
	for (idx = 0; idx < TASK_STK_SIZE; idx++)
		ParserStk[idx] = 0xE0E1E2E3;

	/* Create Input task. */	
	osErr =	OSTaskCreate (	Parser, NULL, (void *)&ParserStk[TASK_STK_SIZE],
                            ParserPrio);
	assert(osErr==OS_NO_ERR);
}


/*--------------- Parser ( ) ---------------*/
/*
PURPOSE
Calls the ParsePkt function to parse a command and 
displays the output in user readable format

*/
void Parser(void *data)
{
    Payload *payloadBfr = NULL;     /* Payload Buffer to store command detail */
        
    for(;;)
    {
        if(payloadBfr == NULL) 
            payloadBfr = Allocate();

        ParsePkt((void *)payloadBfr);                

        /* Check if there is any invalid command */
        if(payloadBfr->cmdType > MaxCommands) {            
            continue;
        }

#if 0
				/* Send an ack to the command */
  	    payloadBfr->dataPart.regionNum = payloadBfr->cmdType;
				payloadBfr->cmdType = Ack;
				payloadBfr->dataLength = AckLen;
#endif
        
        SendDispatcherQ(payloadBfr);
        payloadBfr = NULL;
                     
       
    }
}

/*--------------- ParsePkt ( ) ---------------*/
/*
PURPOSE
Generic parser module to read byte by byte from the serial input
and change to the corresponding state so that it can parse and 
return command packets to the Process function

INPUT PARAMETERS
void* payloadBfr - pointer sent by the Process function

RETURN VALUES
0  - On Successful parsing of a command

*/
void ParsePkt(void* payloadBfr)
{
    ParserState parserState = P1;               /* Initialise parser state */
    ParserErrors parserErrors = PREAMBLE1_ERR;  /* To store the Error Number */
    INT16S c = 0;                               /* Variable to read a byte */
    INT16U i = 0;                               /* Variable to use in loop */ 
    INT8U csVal = 0;                            /* check sum in the command */ 
    
    PktBfr *pktBuf = (PktBfr *)payloadBfr;
    for(;;) {
        c = InByte();

        switch(parserState) {
            case P1:        /* Preamble State 1 */
                if(c == PREAMBLE_BYTE1)
                    parserState = P2;                
                else
                    SetErrorState(&parserState, &parserErrors, PREAMBLE1_ERR);  
                break;
            case P2:        /* Preamble State 2 */
                if(c == PREAMBLE_BYTE2)
                    parserState = P3;
                else
                    SetErrorState(&parserState, &parserErrors, PREAMBLE2_ERR);
                break;
            case P3:        /* Preamble State 3 */
                if(c == PREAMBLE_BYTE3)
                    parserState = C;
                else
                    SetErrorState(&parserState, &parserErrors, PREAMBLE3_ERR);
                break;               
            case C:         /* Check sum state */                
                 csVal = SetState(NULL, &parserState, NULL, c);   
                break;
            case L:         /* Length State */
                if(c >= LEN_CUTOFF) {
                    parserState = T;
                    pktBuf->dataLen = (INT8U)(c - LEN_CUTOFF);
                }
                else
                    SetErrorState(&parserState, &parserErrors, LENGTH_ERR);                     break;
            case T:        /* Packet Type State */                
                pktBuf->pktType = (INT8U)c;		/* Store the Packet Type */
                if(!SetState(pktBuf,&parserState, &parserErrors, csVal)) 
                    return;
                i = 0; 
                break;
            case D:           /* Packet Data State */
                pktBuf->data[i++] = (INT8U)c;             
                if(i >= pktBuf->dataLen)
                    if(!SetState(pktBuf,&parserState,&parserErrors,csVal)) 
                        return;
                break;
            case ER:          /* Error State */
                if(c == PREAMBLE_BYTE1)   /* go to P2 only if preamble 1 */
                    parserState = P2;
                break;
        }
    }
}


/*--------------- CheckSum ( ) ---------------*/
/*
PURPOSE
To Calculate the checksum of the command except the checksum byte

INPUT PARAMETERS
PktBfr* payloadBfr - pointer of the packet buffer sent by the Process function

RETURN VALUES
temp - Calculated checksum

*/
INT8U CheckSum(PktBfr *pktBuf)
{
    INT8U tempCs = 0;   /* Temporary check sum variable */
    INT8U i = 0;        /* Loop variable */
    
    /* Need to add the LEN_CUTOFF to get the actual length came in the packet */
    tempCs = (INT8U)PREAMBLE_BYTE1^PREAMBLE_BYTE2^PREAMBLE_BYTE3^
                    (pktBuf->dataLen+LEN_CUTOFF)^pktBuf->pktType;
    
    /* This will only execute if there is data in the packet */
    for(i = 0; i < pktBuf->dataLen;i++) {
        tempCs ^= pktBuf->data[i];
    }

    return(tempCs);
}


/*--------------- SetErrorState ( ) ---------------*/
/*
PURPOSE
To Calculate the checksum of the command except the checksum byte

INPUT PARAMETERS
ParserState *parserState - pointer of the ParserState
INT8U errVal - The error value that occured recently

RETURN VALUES
errVal - The recent Error value to be stored
*/
void SetErrorState(ParserState *parserState, ParserErrors *parserErrors,INT8U errVal)
{
    Payload *errPkt = NULL;
    
    /* Set the parser state to Error State */
    *parserState = ER;
    *parserErrors = errVal;
    /* Send the error to framer task */
    if(errPkt == NULL)
      errPkt = Allocate();
    
    errPkt->cmdType = CmdErr;
    errPkt->dataLength = errVal;
    SendFramerQ(errPkt);
    
    errPkt = NULL;
    return;
}

/*--------------- SetState ( ) ---------------*/
/*
PURPOSE
Single function to set the parser state and error state. This is to reduce the parser functions length 

INPUT PARAMETERS
PktBfr *pktBuf - The packet buffer
ParserState *parserState - The parser state
ParserErrors *parserErrors - Errors
INT8U csVal - checksum value

RETURN VALUES
0 -  Only when the checksum is correct
1 - Otherwise

*/
INT8U SetState(PktBfr *pktBuf,ParserState *parserState, ParserErrors *parserErrors, INT8U csVal)
{
    /* If state is checksum set next state as Length and return */
    if(*parserState == C)
    {
        *parserState = L;
        return(csVal);
    }
    
    if(*parserState == T && pktBuf->dataLen != 0)
        *parserState = D;
    else{
        /* Need to calculate checksum */
        if(CheckSum(pktBuf) == csVal) {
            *parserState = P1;
            return(0);
        }
        else 
            SetErrorState(parserState,parserErrors, CHECKSUM_ERR);
    }
    return(1);
}

