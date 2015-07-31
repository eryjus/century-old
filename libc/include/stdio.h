/* =============================================================================================================== */
/*                                                                                                                 */
/* stdio.h -- This file contains the prototypes for  Standard IO related C functions.                              */
/*                                                                                                                 */
/* This file is a Century-specific version of the C standard library for standard IO functions.  It                */
/* is a watered down version for this stage of development.                                                        */
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
/*---------------------------------------------------------------------------------------------------------------- */
/*                                                                                                                 */
/*    Date     Tracker  Pgmr  Description                                                                          */
/* ----------  -------  ----  -----------------------------------------------------------------------------------  */
/* 2015-07-23  Initial  Adam  This is the initial version.  Portions of this file has been lifted from             */
/*                            OSDev.org's Meaty Skeleton wiki and are released under CC0 licensing.  See           */
/*                            http://wiki.osdev.org/Meaty_Skeleton & http://wiki.osdev.org/OSDev_Wiki:Copyrights   */
/*                            for details.                                                                         */
/*                                                                                                                 */
/* =============================================================================================================== */

#ifndef __STDIO_H__
#define __STDIO_H__ 1

#include <stddef.h>
#include <stdarg.h>

int printf(const char *fmt, ...);
int sprintf(char *buffer, const char *fmt, ...);
int vsprintf (char *buf, const char *fmt, va_list args);
int putchar(int);
int puts(const char*);

#endif
