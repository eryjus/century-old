/* =============================================================================================================== */
/*                                                                                                                 */
/* text-cons.h -- This file contains the prototypes for a standard text console interface                          */
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

#ifndef __TEXT_CONS_H__
#define __TEXT_CONS_H__

#include <stdint.h>
#include <stddef.h>

enum EgaColor {
    EGA_BLACK = 0,
    EGA_BLUE = 1,
    EGA_GREEN = 2,
    EGA_CYAN = 3,
	EGA_RED = 4,
	EGA_MAGENTA = 5,
	EGA_BROWN = 6,
	EGA_LIGHT_GREY = 7,
	EGA_DARK_GREY = 8,
	EGA_LIGHT_BLUE = 9,
	EGA_LIGHT_GREEN = 10,
	EGA_LIGHT_CYAN = 11,
	EGA_LIGHT_RED = 12,
	EGA_LIGHT_MAGENTA = 13,
	EGA_LIGHT_BROWN = 14,
	EGA_WHITE = 15,
};

#define MAKE_COLOR(f,b) ((uint8_t)((f) | (b << 4)))
#define MAKE_EGA_POS(c,a) ((uint16_t)((c) | (a << 8)))

extern const size_t CONSOLE_COLS;
extern const size_t CONSOLE_ROWS;
extern uint16_t *const CONSOLE_MEMORY;

void console_Init(void);
void console_Cls(void);
void console_PutChar(char c);
void console_WriteString(const char *s);

#endif
