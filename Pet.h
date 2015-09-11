/*=============== P e t . h ===============*/

/*
BY:     Sudha Shunmugam
        Student ID : 00805861
        Embedded Real Time Systems
        Computer Engineering Dept.
        UMASS Lowell
*/
/*
PURPOSE
Interface to Framer.c.

CHANGES
04-12-2003 - Created
*/

#ifndef PET_H
#define PET_H

#define MAX_PETS 10

void CreatePetTask(INT8U petNum);

void SendPetQ(Payload *payload);
void SendPetInvalid(Payload *payload);
void SendPetNotDefined(Payload *oBfr);
INT8U IsPetValid(INT8U petNum);
INT8U IsPetNotDefined(INT8U petNum);
#endif
