/* =============================================================================================================== */
/*                                                                                                                 */
/* string.c -- This file contains the implementation for string-related C functions.                               */
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

#include "string.h"


/*-----------------------------------------------------------------------------------------------------------------*/
/* an implementation of memmove()                                                                                  */
/*-----------------------------------------------------------------------------------------------------------------*/
void *memmove(void *dstptr, const void *srcptr, size_t size)
{
    unsigned char *dst = (unsigned char *)dstptr;
	const unsigned char *src = (const unsigned char *)srcptr;

	if (dst == src) return dstptr;

	if (dst < src) {
		for (size_t i = 0; i < size; i ++) dst[i] = src[i];
	} else {
		for (size_t i = size; i != 0; i --) dst[i - 1] = src[i - 1];
    }

	return dstptr;
}


/*-----------------------------------------------------------------------------------------------------------------*/
/* an implementation of strlen()                                                                                   */
/*-----------------------------------------------------------------------------------------------------------------*/
size_t strlen(const char *string)
{
    size_t result = 0;

    while (string[result]) result ++;

    return result;
}


/*-----------------------------------------------------------------------------------------------------------------*/
/* an implementation of memcmp()                                                                                   */
/*-----------------------------------------------------------------------------------------------------------------*/
int memcmp(const void *aptr, const void *bptr, size_t size)
{
    const unsigned char *a = (const unsigned char *)aptr;
    const unsigned char *b = (const unsigned char *)bptr;

    for (size_t i = 0; i < size; i ++) {
        if (a[i] < b[i]) return -1;
        else if (a[i] > b[i]) return 1;
    }

    return 0;
}


/*-----------------------------------------------------------------------------------------------------------------*/
/* an implementation of memset()                                                                                   */
/*-----------------------------------------------------------------------------------------------------------------*/
void *memset(void *bufptr, int value, size_t size)
{
    unsigned char *buf = (unsigned char *)bufptr;

    for (size_t i = 0; i < size; i ++) buf[i] = (unsigned char)value;

    return bufptr;
}


/*-----------------------------------------------------------------------------------------------------------------*/
/* an implementation of memcpy()                                                                                   */
/*-----------------------------------------------------------------------------------------------------------------*/
void *memcpy(void *__restrict dstptr, const void *__restrict srcptr, size_t size)
{
    unsigned char *dst = (unsigned char *)dstptr;
    const unsigned char *src = (const unsigned char *)srcptr;

    if (dst == src) return dstptr;
    for (size_t i = 0; i < size; i ++) dst[i] = src[i];

    return dstptr;
}


/*-----------------------------------------------------------------------------------------------------------------*/
