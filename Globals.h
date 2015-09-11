/*=============== G l o b a l s . h ===============*/

/*
BY:     Sudha Shunmugam
        Student ID : 00805861
        Embedded Real Time Systems
        Computer Engineering Dept.
        UMASS Lowell
*/

/*
PURPOSE
Define PetMon globals

CHANGES
04-12-2006 - Created.
*/

#ifndef GLOBALS_H
#define GLOBALS_H

/*----- i n c l u d e s -----*/

#include <ctype.h>
#include <stdlib.h>

#include "includes.h"
#include "860types.h"
#include "buffdesc.h"
#include "mpc860.h"
#include "860Init.h"
#include "860SMC.h"
#include "masks860.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

#define SuspendTimeout 100	/* -- Timeout suspended task if not awakened. */

/* Size of the stacks for all the task */
#define	TASK_STK_SIZE     2048 
/*
Task priorities
*/
enum t_prio
{
	InitPrio = 10,		/* -- Initialization task */
	FramerPrio = 11,	/* -- Framer task */
	ParserPrio = 12,		/* -- PktParser task */
    DispatcherPrio = 13,    /* Dispatcher task */
	PetPrio = 14,
    RegionPrio = 23
		
};

#define Preamable1Err     1
#define Preamble2Err      2
#define Preamble3Err      3
#define ChecksumErr       4
#define UnKnownPacketErr  5
#define InvalidLenErr     6

#define InvalidDefineRegErr     11
#define RegionAlreadyDefErr     12

#define InvalidReportRegErr     31
#define RegionNotDefErr         32

#define InvalidPetErr     41
#define InvalidFPErr      51
#define PetNotDefErr      52

#define CmdDefRegion    1
#define CmdDeleteRegion 2
#define CmdReportRegion 3
#define CmdPetLocation  4
#define CmdFindPet      5
#define CmdErr          6

#define Ack      0x04
#define AckLen   7

#define PetLocationRep          0x02
#define PetLocationRepLen       9

#define RegionLocationRep       0x01
#define RegionLocationRepLen	11

#define RegionViolationRep      0x03
#define RegionViolationRepLen   10

#endif
