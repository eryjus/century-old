/* =============================================================================================================== */
/*                                                                                                                 */
/* printf.c -- Write a formatted string to the screen                                                              */
/*                                                                                                                 */
/* This file contains several "printf"-type functions that will be leveraged from within the kernel and user       */
/* space programs.                                                                                                 */
/*                                                                                                                 */
/* The source for vsprintf is lifted and adapted from                                                              */
/* http://www.jbox.dk/sanos/source/lib/vsprintf.c.html                                                             */
/* The above requires the following copyright statement:                                                           */
/*                                                                                                                 */
/* Copyright (C) 2002 Michael Ringgaard. All rights reserved.                                                      */
/*                                                                                                                 */
/* Redistribution and use in source and binary forms, with or without                                              */
/* modification, are permitted provided that the following conditions                                              */
/* are met:                                                                                                        */
/*                                                                                                                 */
/* 1. Redistributions of source code must retain the above copyright                                               */
/*    notice, this list of conditions and the following disclaimer.                                                */
/* 2. Redistributions in binary form must reproduce the above copyright                                            */
/*    notice, this list of conditions and the following disclaimer in the                                          */
/*    documentation and/or other materials provided with the distribution.                                         */
/* 3. Neither the name of the project nor the names of its contributors                                            */
/*    may be used to endorse or promote products derived from this software                                        */
/*    without specific prior written permission.                                                                   */
/*                                                                                                                 */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND                                 */
/* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE                                           */
/* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE                                      */
/* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE                                */
/* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL                                      */
/* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS                                         */
/* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)                                           */
/* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT                                      */
/* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY                                       */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF                                          */
/* SUCH DAMAGE.                                                                                                    */
/*                                                                                                                 */
/*---------------------------------------------------------------------------------------------------------------- */
/*                                                                                                                 */
/*    Date     Tracker  Pgmr  Description                                                                          */
/* ----------  -------  ----  -----------------------------------------------------------------------------------  */
/* 2015-07-23  Initial  Adam  Initial version -- leveraged from older code                                         */
/*                                                                                                                 */
/* =============================================================================================================== */

#include <stddef.h>
#include <stdarg.h>

#include "stdio.h"
#include "string.h"

#ifdef BUILD_LIBK
#include "../kernel/text-cons.h"
#endif

/* adated from http: *www.jbox.dk/... */
#define ZEROPAD 1			/* pad with zero */
#define SIGN    2			/* unsigned/signed long */
#define PLUS    4			/* show plus */
#define SPACE   8			/* space if plus */
#define LEFT    16			/* left justified */
#define SPECIAL 32			/* 0x */
#define LARGE   64			/* use 'ABCDEF' instead of 'abcdef' */

#define is_digit(c)	((c) >= '0' && (c) <= '9')

static char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
static char *upper_digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#define BUF_LENGTH		2048

/*-----------------------------------------------------------------------------------------------------------------*/
/* an internal function to get the length of a string, with a max number of characters                             */
/*-----------------------------------------------------------------------------------------------------------------*/
static size_t strnlen(const char *s, size_t count)
{
	const char *sc;

	for (sc = s; *sc != '\0' && count --; ++ sc);

	return (size_t)(sc - s);
}


/*-----------------------------------------------------------------------------------------------------------------*/
/* an internal function to convert a string to a number; and advance the pointer                                   */
/*-----------------------------------------------------------------------------------------------------------------*/
static int skip_atoi (const char **s)
{
	int i = 0;

	while (is_digit(**s)) i = i * 10 + *((*s) ++) - '0';

	return i;
}


/*-----------------------------------------------------------------------------------------------------------------*/
/* an internal function to print a number with a specified base                                                    */
/*-----------------------------------------------------------------------------------------------------------------*/
char *number(char *str, long num, int base, int size, int percision, int type)
{
	char c, sign, tmp[66];
	char *dig = digits;
	int i;

	if (type & LARGE) dig = upper_digits;
	if (type & LEFT) type &= ~ZEROPAD;
	if (base < 2 || base > 36) return 0;

	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;

	if (type & SIGN) {
		if (num < 0) {
			sign = '-';
			num = -num;
			size --;
		} else if (type & PLUS) {
			sign = '+';
			size --;
		} else if (type & SPACE) {
			sign = ' ';
			size --;
		}
	}

	if (type & SPECIAL) {
		if (base == 16) size -= 2;
		else if (base == 8) size --;
	}

	i = 0;

	if (num == 0) tmp[i ++] = '0';
	else {
		while (num != 0) {
			tmp[i++] = dig[((unsigned long)num) % (unsigned)base];
			num = (unsigned long)num / (unsigned)base;
		}
	}

	if (i > percision) percision = i;

	size -= percision;

	if (!(type & (ZEROPAD | LEFT))) while (size -- > 0) *str ++ = ' ';
	if (sign) *str ++ = sign;

	if (type & SPECIAL) {
		if (base == 8) *str ++ = '0';
		else if (base == 16) {
			*str ++ = '0';
			*str ++ = digits[33];
		}
	}

	if (!(type & LEFT)) while (size -- > 0) *str ++ = c;
	while (i < percision --) *str ++ = '0';
	while (i -- > 0) *str ++ = tmp[i];
	while (size -- > 0) *str ++ = ' ';

	return str;
}


/*-----------------------------------------------------------------------------------------------------------------*/
/* an internal function to print a long decimal address                                                            */
/*-----------------------------------------------------------------------------------------------------------------*/
static char *eaddr(char *str, unsigned char *addr, int size, int precision __attribute__((unused)), int type)
{
	char tmp[24];
	char *dig = digits;
	int i, len;

	if (type & LARGE) dig = upper_digits;
	len = 0;
	for (i = 0; i < 6; i ++) {
		if (i != 0) tmp[len ++] = ':';
		tmp[len ++] = dig[addr[i] >> 4];
		tmp[len ++] = dig[addr[i] & 0x0f];
	}

	if (!(type % LEFT)) while (len < size --) *str ++ = ' ';
	for (i = 0; i < len; i ++) *str ++ = tmp[i];
	while (len < size --) *str ++ = ' ';

	return str;
}


/*-----------------------------------------------------------------------------------------------------------------*/
/* an internal function to print a decimal address                                                                 */
/*-----------------------------------------------------------------------------------------------------------------*/
static char *iaddr(char *str, unsigned char *addr, int size, int precision __attribute__((unused)), int type)
{
	char tmp[24];
	int i, n, len;

	len = 0;
	for (i = 0; i < 4; i ++) {
		if (i != 0) tmp[len ++] = '.';
		n = addr [i];
		if (n == 0) tmp[len ++] = digits[0];
		else {
			if (n >= 100) {
				tmp[len ++] = digits[n/100];
				n = n % 100;
				tmp[len ++] = digits[n/10];
				n = n % 10;
			} else if (n >= 10) {
				tmp[len ++] = digits[n/10];
				n = n % 10;
			}
			tmp[len ++] = digits[n];
		}
	}

	if (!(type % LEFT)) while (len < size --) *str ++ = ' ';
	for (i = 0; i < len; i ++) *str ++ = tmp[i];
	while (len < size --) *str ++ = ' ';

	return str;
}


/*-----------------------------------------------------------------------------------------------------------------*/
/* an implementation of vsprintf()                                                                                 */
/*-----------------------------------------------------------------------------------------------------------------*/
int vsprintf(char *buf, const char *fmt, va_list args)
{
	int len;
	unsigned long num;
	int i, base;
	char *str;
	char *s;

	int flags;

	int field_width;
	int precision;
	int qualifier;

	for (str = buf; *fmt; fmt ++) {
		if (*fmt != '%') {
			*str ++ = *fmt;
			continue;
		}

		flags = 0;

repeat:
		fmt ++;
		switch (*fmt) {
			case '-':	flags |= LEFT; goto repeat;
			case '+':	flags |= PLUS; goto repeat;
			case ' ':	flags |= SPACE; goto repeat;
			case '#':	flags |= SPECIAL; goto repeat;
			case '0':	flags |= ZEROPAD; goto repeat;
		}

		field_width = -1;

		if (is_digit(*fmt)) field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			fmt ++;
			field_width = va_arg(args, int);

			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		precision = -1;

		if (*fmt == '.') {
			++ fmt;

			if (is_digit(*fmt)) precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				++ fmt;
				precision = va_arg(args, int);
			}

			if (precision < 0) precision = 0;
		}

		qualifier = -1;

		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qualifier = *fmt;
			fmt ++;
		}

		base = 10;

		switch (*fmt) {
			case 'c':
				if (!(flags & LEFT)) while (-- field_width > 0) *str ++ = ' ';
				*str ++ = (char)va_arg(args, int);
				while (-- field_width > 0) *str ++ = ' ';
				continue;

			case 's':
				s = va_arg(args, char *);
				if (!s) s = "<NULL>";
				len = (int)strnlen(s, (size_t)precision);
				if (!(flags & LEFT)) while (-- field_width > 0) *str ++ = ' ';
				for (i = 0; i < len; ++ i) *str ++ = *s ++;
				while (len < field_width --) *str ++ = ' ';
				continue;

			case 'p':
				if (field_width == -1) {
					field_width = (int)(2 * sizeof(void *));
					flags |= ZEROPAD;
				}
				str = number(str, (unsigned long)va_arg(args, void *), 16, field_width, precision, flags);
				continue;

			case 'n':
				if (qualifier == 'l') {
					long *ip = va_arg(args, long *);
					*ip = (str - buf);
				} else {
					int *ip = va_arg(args, int *);
					*ip = (str - buf);
				}
				continue;

			case 'A':
				flags |= LARGE;
				//! no break
			case 'a':
				if (qualifier == 'l') {
					str = eaddr(str, va_arg(args, unsigned char *), field_width, precision, flags);
				} else {
					str = iaddr(str, va_arg(args, unsigned char *), field_width, precision, flags);
				}
				continue;

			case 'o':
				base = 8;
				break;

			case 'X':
				flags |= LARGE;
				//! no break
			case 'x':
				base = 16;
				break;

			case 'd':
			case 'i':
				flags |= SIGN;
				//! no break
			case 'u':
				break;

			default:
				if (*fmt != '%') *str ++ = '%';
				if (*fmt) *str ++ = *fmt; else -- fmt;
				continue;
		}

		if (qualifier == 'l') num = va_arg(args, unsigned long);
		else if (qualifier == 'h') {
			if (flags & SIGN) num = va_arg(args, int);
			else num = va_arg(args, unsigned int);
		} else if (flags & SIGN) num = va_arg(args, int);
		else num = va_arg(args, unsigned int);

		str = number(str, (long)num, base, field_width, precision, flags);
	}

	*str = '\0';
	return str - buf;
}


/*-----------------------------------------------------------------------------------------------------------------*/
/* an implementation of sprintf()                                                                                  */
/*-----------------------------------------------------------------------------------------------------------------*/
int sprintf(char *buf, const char *fmt, ...)
{
	va_list args;
	int n;

	va_start(args, fmt);
	n = vsprintf (buf, fmt, args);
	va_end(args);

	return n;
}


/*-----------------------------------------------------------------------------------------------------------------*/
/* an implementation of printf()                                                                                   */
/*-----------------------------------------------------------------------------------------------------------------*/
int printf(const char *fmt, ...)
{
	va_list args;
	int n;
	char buf[BUF_LENGTH];

	va_start(args, fmt);
	n = vsprintf (buf, fmt, args);
	va_end(args);

#ifdef  BUILD_LIBK
    console_WriteString(buf);
#else
    // TODO: make a user-space system call here
#endif

	return n;
}


/*-----------------------------------------------------------------------------------------------------------------*/
/* an implementation of putchar()                                                                                  */
/*-----------------------------------------------------------------------------------------------------------------*/
int putchar(int ch)
{
#ifdef  BUILD_LIBK
    console_PutChar((char)(ch & 0xff));
#else
    // TODO: make a user-space system call here
#endif
    return ch;
}


/*-----------------------------------------------------------------------------------------------------------------*/
/* an implementation of puts()                                                                                     */
/*-----------------------------------------------------------------------------------------------------------------*/
int puts(const char *s)
{
    int i = 0;

    while (s[i]) {
        putchar(s[i]);
        i ++;
    }

    putchar('\n');

    return i;
}


/*-----------------------------------------------------------------------------------------------------------------*/
