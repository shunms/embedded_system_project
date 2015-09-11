##################### m a k e f i l e . ######################

#	by:	George Cheney
#			16.472 /16.572 Embedded Real Time Systems
#			Electrical and Computer Engineering Dept.
#			UMASS Lowell

# PURPOSE
# Build Polled I/O Demo executable from source files.
# This is a generalized makefile which can be changed to
# build other projects by changing the definition of only the
# four variables FILES, ESTDIR, and LSCRIPT.

# Copyright ©2000 by George Cheney, UMASS Lowell ECE Departmnent
# Permission is granted to copy and/or modify this software, provided
# that existing copyright notices are retained in all copies and that this
# notice is included verbatim in any distributions.

# CHANGES
# 02-14-2000 gpc	--	Created.
# 11-18-2000 gpc  -- Remove references to disk drive and make
#							directory paths relative.
# 03-28-2005 gpc  -- Split into separate modules.
# 03-29-2006 gpc	-- make recursive

################################################################
#                    I N S T R U C T I O N S

# 1. DEFINING THE PROJECT FILES
# Set the variable FILES equal to the list of source file names
# separated by spaces. The first name in the list will also be 
# the name of the project and the linked executable.

FILES = PetMon PktParser Framer Dispatcher Pet Region MemMgr SMC1Driver assert

#         E N D    O F   I N S T R U C T I O N S

################################################################

# Define symbols representing ucos-ii OS files and ucos-ii MPC860 port files.

UD = ../source/
UF = $(UD)os_core $(UD)os_mbox $(UD)os_mem $(UD)os_q $(UD)os_sem \
  $(UD)os_task $(UD)os_time
UDD = ..\source

MD = ../mpc860/
MDD = ..\mpc860

CFILES = $(FILES) $(MD)bseppcc $(UF) $(MD)860init $(MD)860smc
AFILES = $(MD)bseppca $(MD)vectors

# Define symbols representing the project name, the list of
# source files, and the list of object files.

PROJECT = $(word 1, $(CFILES))
CSRCFILES = $(addsuffix .c,$(CFILES))
ASRCFILES = $(addsuffix .s,$(AFILES))
OBJECTS = $(addsuffix .o,$(CFILES)) $(addsuffix .o,$(AFILES))

# Set the variable LSCRIPT equal to the the file name of the
# linker script file.

LSCRIPT = $(MD)860Basic.ld

# Define symbols needed in implicit C compile rules.

CC = gcc
CFLAGS = -Wall -MMD -O1 -g  -b powerpc-eabi -mcpu=860 -msdata\
  -I../source \
  -I../mpc860 \
  -I.
 
AS=$(CC)
ASFLAGS= -c -b powerpc-eabi
 
# Define symbols used in the link process.

LFLAGS = -Wl,-Map,$(PROJECT).map -b powerpc-eabi -mcpu=860 -msdata

# Rule to make $(PROJECT).bdx and $(PROJECT).ab from $(PROJECT).elf
$(PROJECT).bdx: $(PROJECT).elf
	$(ESTDIR)convert  -g -b -c -m gnu $(PROJECT).elf -q 
	
# Rule to make prog2.elf by linking object files.	
$(PROJECT).elf: $(OBJECTS)
	$(GCCDIR)gcc  $(LFLAGS) $(OBJECTS)  -T $(LSCRIPT) -o $(PROJECT).elf

# Apply implicit rules for compiling source accoding to dependency files.	
-include $(CSRCFILES:.c=.d)

# Rules to remove compile/link output files and .bak files

.PHONY: clean wayclean force

clean:
	-del *.o
	-del *.elf
	-del *.bdx
	-del *.ab*
	-del *.map
	-del *.d
	-del $(UDD)\*.o
	-del $(MDD)\*.o
	
wayclean:
	make clean
	-del *.bak
	-del $(UDD)\*.bak
	-del $(MDD)\*.bak

force:
	make clean
	make	
