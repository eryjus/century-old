#####################################################################################################################
##                                                                                                                 ##
##  Makefile -- This file contains the core build information for building Century OS (with all architectures)     ##
##                                                                                                                 ##
##  OK, so after reading the outstanding paper by Peter Miller (http://aegis.sourceforge.net/auug97.pdf), I have   ##
##  removed the recursion from this make script and have changed the endire build method.  Initially, I wanted to  ##
##  be able to add a new module and have it automatically picked up in the build.  However, after reading the      ##
##  above white paper, I know why I am having trouble with my builds.  So, it will be changed.                     ##
##                                                                                                                 ##
## --------------------------------------------------------------------------------------------------------------- ##
##                                                                                                                 ##
##     Date     Tracker  Pgmr  Description                                                                         ##
##  ----------  -------  ----  ----------------------------------------------------------------------------------  ##
##  2015-07-19  Initial  Adam  Initial version                                                                     ##
##                                                                                                                 ##
#####################################################################################################################


.SILENT:

#
# == The architecture is expected to be passed in; if it is not then let's assume that we are building for i686
#    ==========================================================================================================
ifndef ARCH
ARCH := i686
endif

include dirs.mk
include targets.mk

#
# -- here are some variables where the subprojects will add strings
#    --------------------------------------------------------------
TOOLS :=
LIBS :=
MODULES :=
INCLUDES :=


#
# -- Define some build programs
#    --------------------------
#
CC := $(ARCH)-elf-gcc
AS := $(ARCH)-elf-as
AR := $(ARCH)-elf-ar
LD := $(ARCH)-elf-gcc


#
# -- Define some build flags
#    -----------------------
#
CC-FLAGS := -c -O2 -g -ffreestanding -Wall -Wextra -I$(SYSROOT-INCL) -isystem=$(SYS-INCL)
AS-FLAGS := -g
AR-FLAGS := rcs
LD-FLAGS := -nostdlib



#
# -- recipe to make all; this one needs to come first
#    ------------------------------------------------
.PHONY: all
all: $(TARGET-ISO)


#
# -- include the make fragments from the other subprojects
#    -----------------------------------------------------
include $(MAKE-FRAG)


#
# -- this is the recipe for the iso image
#    ------------------------------------
$(TARGET-ISO): $(TARGET-GRUB-CFG) $(TARGET-KERNEL) $(LIBS) $(TOOLS) $(MODULES) $(INCLUDES) $(GLOBAL)
	@echo "ISO     " $(TARGET-ISO)
	@mkdir -p $(dir $(TARGET-ISO))
	@grub2-mkrescue -o $(TARGET-ISO) $(SYSROOT) 2> /dev/null


#
# -- this is the recipe for the grub.cfg file
#    ----------------------------------------
$(TARGET-GRUB-CFG): $(GLOBAL)
	@echo "GRUBCFG " $(TARGET-GRUB-CFG)
	@mkdir -p $(SYSROOT-ALL)

	@echo set timeout=3                    		>  $(TARGET-GRUB-CFG)
	@echo set default=0	                  		>> $(TARGET-GRUB-CFG)
	@echo menuentry \"Century\" 	{ 			>> $(TARGET-GRUB-CFG)
	@echo 		multiboot /boot/kernel			>> $(TARGET-GRUB-CFG)
	@echo		boot							>> $(TARGET-GRUB-CFG)
	@echo	}									>> $(TARGET-GRUB-CFG)


#
# -- This recipe will clean the build
#    --------------------------------
.PHONY: clean
clean: $(MAKE-CLEAN)
	rm -Rf $(SYSROOT)
	rm -Rf bin

