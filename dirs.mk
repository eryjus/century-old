#####################################################################################################################
##                                                                                                                 ##
##  dirs.ml -- This file contains the directory information for the build                                          ##
##                                                                                                                 ##
## --------------------------------------------------------------------------------------------------------------- ##
##                                                                                                                 ##
##     Date     Tracker  Pgmr  Description                                                                         ##
##  ----------  -------  ----  ----------------------------------------------------------------------------------  ##
##  2015-07-29  Initial  Adam  Initial version                                                                     ##
##                                                                                                                 ##
#####################################################################################################################

ifndef SYSROOT
SYSROOT := isodir
endif

#
# -- The following directories are built from the sysroot directory
#    --------------------------------------------------------------
SYSROOT-BIN 	:= $(SYSROOT)/bin
SYSROOT-BOOT 	:= $(SYSROOT)/boot
SYSROOT-GRUB 	:= $(SYSROOT-BOOT)/grub
SYSROOT-HOME	:= $(SYSROOT)/home
SYSROOT-USR		:= $(SYSROOT)/usr
SYSROOT-LIB		:= $(SYSROOT)/lib
SYSROOT-INCL	:= $(SYSROOT-USR)/local/include
SYSROOT-ALL		:= $(SYSROOT-BIN) $(SYSROOT-GRUB) $(SYSROOT-HOME) $(SYSROOT-LIB) $(SYSROOT-INCL)


#
# -- The following directories are built from the subproject directory
#    -----------------------------------------------------------------
SUBP-BIN		:= bin
SUBP-LIB 		:= lib
SUBP-OBJ		:= obj


#
# -- The following is the architecture-specific directory and may appear an many locations
#    -------------------------------------------------------------------------------------
ARCH-DIR		:= arch-$(ARCH)


#
# -- The following is the compiler system include directory
#    ------------------------------------------------------
SYS-INCL := $(shell $(CC) --print-search-dirs | grep install)
SYS-INCL := $(subst install: ,,$(SYS-INCL))include
