#####################################################################################################################
##                                                                                                                 ##
##  targets.ml -- This file contains the final target definitions for all the builds                               ##
##                                                                                                                 ##
##  This file is necessary because the variables are not defined in total before all the rules are processed       ##
##  (at least from what I observed).  Therefore, I want to be able to define all the targets prior to starting     ##
##  any of the processing and includes.                                                                            ##
##                                                                                                                 ##
## --------------------------------------------------------------------------------------------------------------- ##
##                                                                                                                 ##
##     Date     Tracker  Pgmr  Description                                                                         ##
##  ----------  -------  ----  ----------------------------------------------------------------------------------  ##
##  2015-07-29  Initial  Adam  Initial version                                                                     ##
##                                                                                                                 ##
#####################################################################################################################


#
# -- These are the big-name targets and shouls be able to be found in $(SYSROOT) somewhere
#    -------------------------------------------------------------------------------------
TARGET-ISO 		:= bin/Century-$(ARCH).iso
TARGET-KERNEL 	:= $(SYSROOT-BOOT)/kernel
TARGET-LIBK 	:= $(SYSROOT-LIB)/libk.a
TARGET-LIBC 	:= $(SYSROOT-LIB)/libc.a
TARGET-GRUB-CFG := $(SYSROOT-GRUB)/grub.cfg


#
# -- These are some internal and intermediate targets
#    ------------------------------------------------
MAKE-FRAG 		:= $(wildcard **/Makefrag)
MAKE-CLEAN 		:= $(subst /,,$(addprefix clean,$(dir $(MAKE-FRAG))))
MAKE-FILES 		:= Makefile $(MAKE-FRAG) $(wildcard *.mk)


#
# -- GLOBAL -- if one of these files change, we need to build everything
#    -------------------------------------------------------------------
GLOBAL			:= $(MAKE-FILES)
