/*=============== R e g i o n . h ===============*/

/*
BY:     Sudha Shunmugam
        Student ID : 00805861
        Embedded Real Time Systems
        Computer Engineering Dept.
        UMASS Lowell
*/
/*
PURPOSE
Interface to Region.c.

CHANGES
04-20-2003 - Created
*/

#include "Globals.h"

#ifndef REGION_H
#define REGION_H

#define MAX_REGION 11

extern OS_EVENT	*regionQueue[MAX_REGION];	

void CreateRegionTask(INT8U regionNum);
void SendRegionLocation(INT8U regionNum);

void SendRegionQ(Payload *payload);
INT8U IsRegionValid(INT8U regionNum);
INT8U IsRegionNotDefined(INT8U regionNum);
void SendRegionInvalid(Payload *oBfr);
void SendRegionNotDefined(Payload *oBfr);
void SendRegionAlreadyDefined(Payload *oBfr);

void SendToHeadRegion(Payload *payload);
#endif
