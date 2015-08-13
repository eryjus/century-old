/* =============================================================================================================== */
/*                                                                                                                 */
/* kernel.c -- This file contains the functions to get the kernel initialized and running properly                 */
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
/* 2015-08-13  -------  Adam  Completed initialization step #5 -- Determine basic machine information              */
/*                                                                                                                 */
/* =============================================================================================================== */

#include <stdio.h>

#include "text-cons.h"
#include "kern-defs.h"

void kMain(void)
{
    VMMInit();
    MultiProcessorInit();

    printf("kMain()\n");
    hang();
}

void panic(const char *s)
{
    puts(s);
    hang();
}
