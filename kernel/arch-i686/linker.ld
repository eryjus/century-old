/*===========================================================================================*/
/*                                                                                           */
/* linker.ld                                                                                 */
/*                                                                                           */
/* This file contains the command script to feed to the linker that will determine how and   */
/* where to put the sections of the executable.  Most notably, the .mbheader section needs   */
/* to come first as it is required to be in the first 8K of the file.                        */
/*                                                                                           */
/*********************************************************************************************/
/*                                                                                           */
/*      Century is a Hobby Operating System.                                                 */
/*      Copyright (C) 2014, 2015  Adam Scott Clark                                           */
/*                                                                                           */
/*      This program is free software: you can redistribute it and/or modify                 */
/*      it under the terms of the GNU General Public License as published by                 */
/*      the Free Software Foundation, either version 3 of the License, or                    */
/*      any later version.                                                                   */
/*                                                                                           */
/*      This program is distributed in the hope that it will be useful,                      */
/*      but WITHOUT ANY WARRANTY; without even the implied warranty of                       */
/*      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                        */
/*      GNU General Public License for more details.                                         */
/*                                                                                           */
/*      You should have received a copy of the GNU General Public License along              */
/*      with this program.  If not, see http://www.gnu.org/licenses/gpl-3.0-standalone.html. */
/*                                                                                           */
/*********************************************************************************************/
/*                                                                                           */
/*    Date     Tracker  Pgmr  Description                                                    */
/* ----------  ------   ----  -------------------------------------------------------------- */
/* 2015/07/22  Initial  ADCL  Leveraged from osdev.org -- "Meaty Skeleton" wiki              */
/*                                                                                           */
/*===========================================================================================*/

ENTRY (_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
	. = 1M;

	.text BLOCK(4K) : ALIGN (0x1000)
	{
	    *(.mbheader)
		*(.code)
		*(.text)
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

    end = .; _end = .; __end = .;
}
