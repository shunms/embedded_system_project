/*=============== S M C 1 D r i v e r . h ===============*/

/*
BY:	George Cheney
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell

PURPOSE
Interface to SMC1Driver.c

CHANGES
04-07-2003 gpc - Release to 16.472 / 16.572 class.
04-07-2003 gpc - Add assert to timeout Pends.
04-08-2003 gpc - Remove typing dependent timeouts.
03-28-2005 gpc - Split into separate modules.
03-29-2006 gpc - Update for S2006
*/

#ifndef SMC1DRIVER_H
#define SCM1DRIVER_H
extern OS_EVENT	*txDone;		/* Output waits until transmit done */
extern OS_EVENT *rxDone;		/* Input waits until receive done */

/*----- f u n c t i o n    p r o t o t y p e s -----*/

void InitSMC1Driver(void);

void OutByte(int c);
int InByte(void);
#endif