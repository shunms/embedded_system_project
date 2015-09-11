/*=============== F r a m e r . h ===============*/

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

#ifndef FRAMER_H
#define FRAMER_H

void CreateFramerTask(void);
void SendFramerQ(Payload *payload);
#endif
