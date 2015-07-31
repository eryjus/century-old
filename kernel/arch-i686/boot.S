# ===================================================================================================================
#
#  boot.S -- This source file contains the entry point for the operating system, as required for the i386 processor
#
#  This file is the architecture-specific entry point for the i386 processor family.  It will be responsbile for
#  putting the system into a defined state for the kernel main entry point.  Since Multiboot will handle most of
#  this for us in the i386 family, there is not to much to actually do here.
#
#       Century is a Hobby Operating System.
#       Copyright (C) 2014, 2015  Adam Scott Clark
#
#       This program is free software: you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation, either version 3 of the License, or
#       any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License along
#       with this program.  If not, see http://www.gnu.org/licenses/gpl-3.0-standalone.html.
#
#  The kernel (specifically the .mbheader section) will be loaded at 0x10000 (1MB).
#
# -------------------------------------------------------------------------------------------------------------------
#
#     Date     Tracker  Pgmr  Description
#  ----------  -------  ----  --------------------------------------------------------------------------------------
#  2015-07-20  Initial  Adam  This is the initial version.  The body of this file has been lifted from a previous
#                             version and its header comments have been dropped.
#
# ===================================================================================================================


# *******************************************************************************************************************
# Expose some labels for global functions
# *******************************************************************************************************************
        .global	    _start				                    # this is the kernel entry point
        .global	    hang					                # make hang() visible


# *******************************************************************************************************************
# Define some external labels that we need
# *******************************************************************************************************************
        .extern     kInit                                   # the initialization functions
        .extern     kMain                                   # the kernel main routines


# *******************************************************************************************************************
# Define some constants for the multi-boot loader specification
# *******************************************************************************************************************
        .equ        MODULEALIGN,1<<0			            # align loaded on page boundaries
        .equ        MEMINFO,1<<1			                # provide memory map
        .equ        VIDEOINFO,1<<2      			        # provide video information

        .equ        FLAGS,MODULEALIGN|MEMINFO|VIDEOINFO     # this is the multiboot flag
        .equ        MAGIC,0x1badb002		                # magic number for the header to be found
        .equ        CHECKSUM,-(MAGIC+FLAGS)	                # check number


# *******************************************************************************************************************
# Setup the size of the kernel stack
# *******************************************************************************************************************
        .equ        STACKSIZE,0x4000    			        # that's 16K


# *******************************************************************************************************************
# The mbheader section is required to be in the first 16?K of the final executable.  This section is organized by
# the linker.ld linker script to be at the beginning of the executable.
# *******************************************************************************************************************
        .section    .mbheader
        .align      4

        .long       MAGIC			                        # this is the real header block in the seg
		.long	    FLAGS			                        # flags of info to provide
		.long	    CHECKSUM		                        # the proper checksum for the mb spec
		.long       0,0,0,0,0
		.long	    1				                        # EGA text mode
		.long	    80				                        # 80 columns
		.long       25				                        # 25 lines
		.long	    8				                        # color depth


# *******************************************************************************************************************
# This is the beginning of the code segment
# *******************************************************************************************************************
        .section    .text
        .align      4

_start:
# -------------------------------------------------------------------------------------------------------------------
# set up a stack right out of the gates
# -------------------------------------------------------------------------------------------------------------------
	                cli								        # just in case, no interupts please
	                movl	        $(stack+STACKSIZE),%esp	# setup stack
	                push	        %eax		            # push multiboot magic number
	                push	        %ebx				    # push multiboot info struct

	                call	        kInit				    # call the C initialization function
	                call            kMain                   # call the main C kernel function

hang:
# -------------------------------------------------------------------------------------------------------------------
# if you get back here, then you have a problem and need to hang.  hang() is also a function that is called to
# hang the processor.
# -------------------------------------------------------------------------------------------------------------------
	                cli								        # stop all interrupts
	                hlt								        # halt the processor and force a hard reboot
	                jmp		        hang				    # loop, just in case


# *******************************************************************************************************************
# This is the beginning of the bss segment
# *******************************************************************************************************************
        .section    .bss
        .align      4


# -------------------------------------------------------------------------------------------------------------------
# reserve the stack space on the bss segment for now
# -------------------------------------------------------------------------------------------------------------------
        .comm       stack,STACKSIZE			                # reserve 16K for a stack


# *******************************************************************************************************************
