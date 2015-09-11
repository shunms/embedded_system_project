/*=============== S M C 1 D r i v e r . c ===============*/

/*
BY:	George Cheney
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell

PURPOSE
Handles byte I/O for SMC1.

DEMONSTRATES
Multiple SMC Events

CHANGES
04-07-2003 gpc - Release to 16.472 / 16.572 class.
04-07-2003 gpc - Add assert to timeout Pends.
04-08-2003 gpc - Remove typing dependent timeouts.
03-28-2005 gpc - Split into separate modules.
03-29-2006 gpc - Update for S2006
*/

#include <stdlib.h>

#include "includes.h"
#include "860types.h"
#include "buffdesc.h"
#include "mpc860.h"
#include "860Init.h"
#include "860SMC.h"
#include "masks860.h"
#include "assert.h"
#include "Globals.h"

/*----- g l o b a l s -----*/

OS_EVENT	*txDone;		/* Output waits until transmit done */
OS_EVENT *rxDone;		/* Input waits until receive done */

/*--------------- I n i t S M C 1 D r i v e r ( ) ---------------*/

/*
PURPOSE
Initialize the SMC1 I/O driver..
*/
void InitSMC1Driver(void)
{
	txDone = OSSemCreate(1);			/* -- No xmit yet; indicate ready. */
	rxDone = OSSemCreate(0);			/* -- No receive data yet; not ready. */  
}   

/*--------------- E x t I n t e r r u p t ( ) ---------------*/

/*
PURPOSE
External interrupt exception handler. 
Signal Output on tx interrupt to indicate tx ready for data,
and signal Input to inddicate the presence of received data.
*/
void ExtInterrupt(void)
{
	INT8U		osErr;			/* -- OS Error code */

	/* Increment interrupt disable count, since interrupts are off. */
	++disableCnt;

	/* Tell kernel we're in an ISR. */
	OSIntEnter();
	
	/* Handle the O/S clock tick interrupt from timer 2. */
	if (IMMR->timer_ter2)
		{
		/* Tell the O/S. */
		OSTimeTick();
		
		/* Clear the timer event. */
		IMMR->timer_ter2 = (TER2_REF | TER2_CAP);
		}
	
	/* Check for a tx event */
	if (IMMR->smc_regs[SMC1_REG].smc_smce & SMCE1_TX)
		{
	
		/* Signal the Output that the tx is done. */
		osErr = OSSemPost(txDone);
		assert(osErr == OS_NO_ERR);
			
	   /* Clear the SMC1 TX Event. */
	   IMMR->smc_regs[SMC1_REG].smc_smce = SMCE1_TX;  
	   }
	   
	/* Check for an rx event. */
	if (IMMR->smc_regs[SMC1_REG].smc_smce & SMCE1_RX)
		{
		/* Signal the Input that the rx is ready. */
		osErr = OSSemPost(rxDone);
		assert(osErr == OS_NO_ERR);
	
	   /* Clear the SMC1 RX Event. */
	   IMMR->smc_regs[SMC1_REG].smc_smce = SMCE1_RX;  
		}
   
	/* Clear the in service bit. */      
   IMMR->cpmi_cisr = CISR_SMC1;
	   
	/*
	Reduce interrupt disable count,
	since interrupts are reenabled on "rfi."
	*/
   --disableCnt;

	/* Give the O/S a chance to swap tasks. */
	OSIntExit ();
}


/*--------------- O u t B y t e ( ) ---------------*/

/*
PURPOSE
Output 1 character to SMC1.

INPUT PARAMETERS
c		-- the character to transmit

GLOBALS
txBD		-- Pointer to tx buffer descriptor
txDone	-- Output waits.
*/
void OutByte(int c)
{
	INT8U		osErr;		/* -- Semaphore error code */

	/* Wait for xmit done and buffer not empty. */
	OSSemPend(txDone, SuspendTimeout, &osErr);

	assert(osErr==OS_NO_ERR);

	/* Write the character to the tx. */
	*txBD->bd_addr = c;
	txBD->bd_length = 1;
	txBD->bd_cstatus |= BDReady;
}

/*--------------- I n B y t e ( ) ---------------*/

/*
PURPOSE
Input one character from SMC1.

RETURN VALUE
	-- the received character

GLOBALS
rxBD		-- Pointer to tx buffer descriptor
rxDone	--	Input waits for received data.
*/
int InByte(void)
{
	char	rxChar;		/* -- Received character */
	INT8U	osErr;				/* -- Semaphore error code */
	
	/* Wait for the rx to get a character. */
	OSSemPend(rxDone, 0, &osErr);	/* 04-08-2003 gpc */

	assert(osErr==OS_NO_ERR);

	/* Read the character from the rx. */
   rxChar = *(rxBD->bd_addr);   
   rxBD->bd_cstatus |= BDEmpty; 
   
   /* Return the character. */
   return rxChar;  
}