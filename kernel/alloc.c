/* =============================================================================================================== */
/*                                                                                                                 */
/* alloc.c -- This file contains the functions to allocate memory for the VMM at a page level                      */
/*                                                                                                                 */
/*      Century is a Hobby Operating System.                                                                       */
/*      Copyright (C) 2014, 2015  Adam Scott Clark                                                                 */
/*                                                                                                                 */
/*      This program is free software: you can redistribute it and/or modify                                       */
/*      it under the terms of the GNU General Public License as published by                                       */
/*      the Free Software Foundation, either version 3 of the License, or                                          */
/*      any later version.                                                                                         */
/*                                                                                                                 */
/*      This program is distributed in the hope that it will be useful,                                            */
/*      but WITHOUT ANY WARRANTY; without even the implied warranty of                                             */
/*      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                                              */
/*      GNU General Public License for more details.                                                               */
/*                                                                                                                 */
/*      You should have received a copy of the GNU General Public License along                                    */
/*      with this program.  If not, see http://www.gnu.org/licenses/gpl-3.0-standalone.html.                       */
/*                                                                                                                 */
/* --------------------------------------------------------------------------------------------------------------- */
/*                                                                                                                 */
/*    Date     Tracker  Pgmr  Description                                                                          */
/* ----------  -------  ----  -----------------------------------------------------------------------------------  */
/* 2015-08-01  Initial  Adam  This is the initial version; which will be in flux for some time.                    */
/*                                                                                                                 */
/* =============================================================================================================== */

#include "arch.h"
#include "kern-defs.h"

#include <string.h>

char *newEnd = 0;           // This will track the memory used during startup

//
// -- This function is only used during startup to allocate memory before we have a proper memory manager
//    up and running.  It assumes that all memory will be allocated from the end of the .bss section up
//    and will be less than 4MB (which is the size of the single page that is mapped).
//    ---------------------------------------------------------------------------------------------------
char *AllocAtEntry(void)
{
    if (newEnd == 0) newEnd = end;
    if ((unsigned int)newEnd >= KERN_BASE + 0x400000) panic("Out of memory: only 4MB is mapped at startup");

    void *p = (void *)PAGE_ROUNDUP((unsigned int)newEnd);
    memset(p, 0, PAGE_SIZE);
    newEnd = (char *)p + PAGE_SIZE;

    return p;
}
