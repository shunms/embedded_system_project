#ifndef PktParser_H
#define PktParser_H

/*=============== PktParser.h ===============*/

/*
BY:   Sudha Shunmugam
      Student ID : 00805861
      16.572 Embedded Real Time Systems
      Computer Engineering Dept.
      UMASS Lowell
*/

/*
PURPOSE
Exporting Parser functions.

CHANGES
01-26-2006  Base version
02-02-2006  Added the File Header
02-09-2006 Added some comments
01-15-2006 Did changes for running in the MPC860 Board
02-20-2006 Made changes for Prog3
02-25-2006 Corrected the errors from Prog2
03-08-2006 Changes for Prog4 - Interrupt IO
04-12-2006 Using this for Project
           Added CreateParerTask()
*/    


/*----- i n c l u d e s -----*/
#include "860types.h"

#define PREAMBLE_BYTE1  0xff
#define PREAMBLE_BYTE2  0x00
#define PREAMBLE_BYTE3  0xA5

#ifndef BfrSize
#define BfrSize 4
#endif

#define RegionPktLen 5
#define PetPktLen 3

typedef enum {P1, P2, P3, C, L, T, D, ER} ParserState;

typedef struct
{
  INT8U dataLength;
  INT8U cmdType;    
    union
    {
        INT8U regionNum;
        INT8U petNum;
        INT8U regionDetails[RegionPktLen];
		INT8U petDetails[PetPktLen];
    }dataPart;
} Payload;


/*----- f u n c t i o n    d e f i n i t i o n s -----*/
void ParsePkt(void* payloadBfr);

void CreateParserTask(void);
#endif

