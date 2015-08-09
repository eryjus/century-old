/* =============================================================================================================== */
/*                                                                                                                 */
/* text-cons.c -- This file contains functions and internal data to manage the text console.                       */
/*                                                                                                                 */
/* This is an architecture-specific implementation of a text video driver.                                         */
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
/* 2015-07-25  Initial  Adam  This is the initial version.  Portions are taken from some old recovered code.       */
/* 2015-08-09  -------  Adam  Some cleanup where the mapping of the video memory is moved to higher-half and       */
/*                            needs to be adjusted proeprly.                                                       */
/*                                                                                                                 */
/* =============================================================================================================== */

#include "../text-cons.h"
#include "arch.h"
#include <stdarg.h>

/*
 * -- some global variables that will be used by the driver
 *    -----------------------------------------------------
 */
uint16_t _colPos = 0;
uint16_t _rowPos = 0;
uint8_t _consAttr = 0x07;

/*
 * -- some local prototypes that will be written in the asm source
 *    ------------------------------------------------------------
 */
extern void console_SetCursor(uint16_t _rowPos, uint16_t _colPos);
extern void console_ScrollUp(uint16_t blank);


/*-----------------------------------------------------------------------------------------------------------------*/
/* Put a character on the console screen -- managing the cursor at the same time.                                  */
/*-----------------------------------------------------------------------------------------------------------------*/
void console_PutChar(char c)
{
    uint16_t *buf = P2V(CONSOLE_MEMORY);

    if (c == '\n' || c == '\r') {
newline:
		_colPos = 0;
		_rowPos ++;
		if (_rowPos >= CONSOLE_ROWS) {
			console_ScrollUp(MAKE_EGA_POS(' ', _consAttr));
			_rowPos = CONSOLE_ROWS - 1;
			_colPos = 0;
		}

		goto out;
	}

    buf[_colPos + _rowPos * CONSOLE_COLS] = MAKE_EGA_POS(c & 0xff, _consAttr);

	_colPos ++;

	if (_colPos >= CONSOLE_COLS) goto newline;

out:
	console_SetCursor(_rowPos, _colPos);
}


/*-----------------------------------------------------------------------------------------------------------------*/
/* Write a string to the console, one character at a time                                                          */
/*-----------------------------------------------------------------------------------------------------------------*/
void console_WriteString(const char *s)
{
    while (*s) {
        console_PutChar(*s ++);
    }
}


/*-----------------------------------------------------------------------------------------------------------------*/
/* Clear the console screen                                                                                        */
/*-----------------------------------------------------------------------------------------------------------------*/
void console_Cls(void)
{
    uint16_t *buf = P2V(CONSOLE_MEMORY);
    uint32_t i;

    _colPos = 0;
    _rowPos = 0;
    console_SetCursor(_rowPos, _colPos);

    for (i = 0; i < CONSOLE_COLS * CONSOLE_ROWS; i ++) {
        buf[i] = MAKE_EGA_POS(' ' & 0xff, _consAttr);
    }
}


/*-----------------------------------------------------------------------------------------------------------------*/
/* Initialize the text console and prepare it to display data                                                      */
/*-----------------------------------------------------------------------------------------------------------------*/
void console_Init(void)
{
    _colPos = 0;
    _rowPos = 0;
    _consAttr = 0x07;
    console_SetCursor(_rowPos, _colPos);

//    console_Cls();
}


/*-----------------------------------------------------------------------------------------------------------------*/
