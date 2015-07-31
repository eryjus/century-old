/* =============================================================================================================== */
/*                                                                                                                 */
/* string.h -- This file contains the prototypes for string-related C functions.                                   */
/*                                                                                                                 */
/* This file is a Century-specific version of the C standard library for the string manipulation functions.  It    */
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
/*      ----------------------------------------------------------------------------------------------             */
/*                                                                                                                 */
/*      Portions of this source are licensed under CC0 licensing.  Please see the comments below for               */
/*      the source of the code.  Please see https://creativecommons.org/publicdomain/zero/1.0/legalcode            */
/*      for the details of the CC0 license.                                                                        */
/*                                                                                                                 */
/* --------------------------------------------------------------------------------------------------------------- */
/*                                                                                                                 */
/*    Date     Tracker  Pgmr  Description                                                                          */
/* ----------  -------  ----  -----------------------------------------------------------------------------------  */
/* 2015-07-22  Initial  Adam  This is the initial version.  Portions of this file has been lifted from             */
/*                            OSDev.org's Meaty Skeleton wiki and are released under CC0 licensing.  See           */
/*                            http://wiki.osdev.org/Meaty_Skeleton & http://wiki.osdev.org/OSDev_Wiki:Copyrights   */
/*                            for details.                                                                         */
/*                                                                                                                 */
/* =============================================================================================================== */

#ifndef __STRING_H__
#define __STRING_H__ 1

#include <stddef.h>

int memcmp(const void *, const void *, size_t);
void *memcpy(void * __restrict, const void * __restrict, size_t);
void *memmove(void *, const void *, size_t);
void *memset(void *, int, size_t);
size_t strlen(const char *);

#endif
