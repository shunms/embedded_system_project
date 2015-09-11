/*--------------- a s s e r t . c ---------------*/

/*
BY:	George Cheney
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
*/

/*
PURPOSE
Define function AssertFailure() to support macro "assert()."

CHANGES
11-21-2001 gpc -	Created.
04-16-2002 gpc - 	Moved infinite loop out of AssertFailure() and
						into the assert() macro.
*/		

/*
assert.c: Copyright ©2001, 2002 by George Cheney, UMASS Lowell ECE Departmnent
Permission is granted to copy and/or modify this software, provided
that existing copyright notices are retained in all copies and that this
notice is included verbatim in any distributions.
*/

#include <string.h>

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

#define MaxFileName	30    /* -- Length of file name buffer */

/*----- g l o b a l    v a r i a b l e s -----*/

char		fileName[MaxFileName];  /* -- File name buffer */
int		lineNumber;             /* -- Line number buffer */

/*---------- A s s e r t F a i l u r e ( ) ----------*/
/*
PURPOSE
When an assertion fails, capture the file name and line number
of the offending code. Then hang in a loop until killed from
visionCLICK.
*/

void AssertFailure(char *file, int line)
{
   /* Capture source file name. */
   
	strncpy(fileName, file, MaxFileName);
	
	/* Capture source line number. */
	lineNumber = line;	/* <-- INSERT BREAKPOINT HERE. */
}