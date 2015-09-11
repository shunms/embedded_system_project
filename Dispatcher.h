/*=============== O u t p u t . h ===============*/

/*
BY:     Sudha Shunmugam
        Student ID : 00805861
        Embedded Real Time Systems
        Computer Engineering Dept.
        UMASS Lowell
*/
/*
PURPOSE
Interface to Output.c.

CHANGES
04-12-2006 - Interface to Dispatcher task
*/

#ifndef DISPATCHER_H
#define DISPATCHER_H

void CreateDispatcherTask(void);
void SendDispatcherQ(Payload *payload);
#endif
