# ===================================================================================================================
#
#  text-cons-low.S -- This source contains the low level interfaces to manage the intel console.
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
#  2015-07-24  Initial  Adam  This is the initial version.  It has been lifted from older code.
#
# ===================================================================================================================


# *******************************************************************************************************************
# Expose some labels for global functions
# *******************************************************************************************************************
	    .global		console_ScrollUp
	    .global     console_SetCursor


# *******************************************************************************************************************
# Define some external labels that we need
# *******************************************************************************************************************
        .extern     CONSOLE_COLS
        .extern     CONSOLE_ROWS
        .extern     CONSOLE_MEMORY


# *******************************************************************************************************************
# This is the beginning of the code segment
# *******************************************************************************************************************
        .section    .text
        .align      4

console_ScrollUp:
# -------------------------------------------------------------------------------------------------------------------
# scroll the screen up one line; this function does not mainpulate the cursor
# -------------------------------------------------------------------------------------------------------------------
                    push            %ebp                    # save caller's stack frame
                    mov             %esp,%ebp               # create a new stack frame
	                push	        %edi        		    # save edi
	                push	        %esi				    # save esi
	                push	        %ecx				    # save ecx
	                push	        %eax				    # save eax
	                pushf	    						    # save flags
	                cli 				                    # disable interrupts for a moment

                    add             $(CONSOLE_MEMORY),%edi  # get the console memory buffer
                    mov             %edi,%esi               # set the destination as well
                    add             $(CONSOLE_ROWS),%esi    # adjust down one row
                    mov             $(CONSOLE_ROWS),%eax    # get the number of rows
                    mov             $(CONSOLE_COLS),%edx    # get the number of cols
                    dec             %eax                    # we move 1 less row
                    rep             movsw                   # move the data

                    mov             $(CONSOLE_COLS),%ecx    # get the number of cols
                    mov             16(%ebp),%ax            # get the byte/attr to use as fill
                    rep             stosw                   # blank the line

	                popf	    						    # restore the flags
	                pop		        %eax				    # restore eax
	                pop		        %ecx				    # restore ecx
	                pop		        %esi				    # restore esi
	                pop		        %edi				    # restore edi
	                pop		        %ebp				    # restore edi

	                ret

# -------------------------------------------------------------------------------------------------------------------


console_SetCursor:
# -------------------------------------------------------------------------------------------------------------------
# move the cursor to the specified row/col
# -------------------------------------------------------------------------------------------------------------------
	                push	        %ebp				    # create a stack frame
	                mov		        %esp,%ebp			    # ... stack frame
	                push	        %eax    				# save eax
	                push	        %ebx    				# save ebx
	                push	        %ecx    				# save ecx
	                push	        %edx    				# save edx

	                mov		        12(%ebp),%eax	    	# get the column
	                mov		        8(%ebp),%ebx   		    # get the row

# the location will be rows * 80 + cols
	                mov		        %ebx,%ecx   			# we will manipulate both
	                shl		        $6,%ebx			        # multiply by 64
	                shl		        $4,%ecx      			# multiply by 16
	                add		        %ecx,%ebx   			# add them together to get *80
	                add		        %ebx,%eax   			# add the col# eax is the address

	                mov		        %ax,%bx			        # get the result
	                and		        $0x00ff,%bx		        # get the LSB
	                shr		        $8,%ax			        # move the MSB
	                and		        $0x00ff,%ax		        # ensure just the byte

                	push	        %ax				        # save our work
                	mov		        $0x3d4,%dx		        # write to port 0x3d4
	                mov		        $0x0e,%al			    # we want register 0x0e
                	out		        %al,%dx			        # want to set Cursor MSB
	                pop		        %ax				        # get our work back
	                inc		        %dx				        # set the byte
	                out		        %al,%dx			        # write the data

	                dec		        %dx				        # back to 3d4
	                mov		        $0x0f,%al			    # want register 0x0f
	                out		        %al,%dx			        # want to set cursor LSB
	                inc		        %dx				        # set the byte
	                mov		        %bl,%al			        # move to al
	                out		        %al,%dx     			# write the data

	                pop		        %edx    				# save edx
	                pop		        %ecx    				# save ecx
	                pop		        %ebx    				# save ebx
	                pop		        %eax    				# save eax
	                pop		        %ebp    				# restore previous frame

                	ret

# -------------------------------------------------------------------------------------------------------------------
