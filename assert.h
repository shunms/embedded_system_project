/*--------------- a s s e r t . h ---------------*/

/*
BY:	George Cheney
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
*/

/*
PURPOSE
Define assert macro for MPC860 / visionCLICK

CHANGES
11-21-2001 gpc -	Created.
04-16-2002 gpc -	Move infinite loop from function AssertFailure() to
						the assert() macro. On assert failure the program
						will now hang in the offending function rather than
						in AssertFailure().
*/		

/*
assert.h: Copyright ©2001 by George Cheney, UMASS Lowell ECE Departmnent
Permission is granted to copy and/or modify this software, provided
that existing copyright notices are retained in all copies and that this
notice is included verbatim in any distributions.
*/

#ifndef ASSERT_H
#define ASSERT_H

#ifndef NDEBUG
#define assert(cond) \
if (!(cond))\
{\
	AssertFailure(__FILE__, __LINE__);\
	asm(" b .");\
}
#else
#define assert(cond) 
#endif

void AssertFailure(char *file, int line);
#endif