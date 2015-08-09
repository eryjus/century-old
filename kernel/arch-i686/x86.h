/* =============================================================================================================== */
/*                                                                                                                 */
/* x86.h -- This file contains the inline assembly functions for the x86 32-bit processors.                        */
/*                                                                                                                 */
/* This file was leveraged from the xv6 operating system (since I am not that strong in inline assembly).          */
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
/* 2015-08-02  Initial  Adam  This is the initial version.  Portions inspired by xv6.                              */
/*                                                                                                                 */
/* =============================================================================================================== */

#include <stdint.h>

// Routines to let C code use special x86 instructions.
static inline uint8_t inb(uint16_t port)
{
    uint8_t data;
    asm volatile("in %1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline void insl(int port, void *addr, int cnt)
{
    asm volatile("cld; rep insl" :
        "=D" (addr), "=c" (cnt) :
        "d" (port), "0" (addr), "1" (cnt) :
        "memory", "cc");
}

static inline void outb(uint16_t port, uint8_t data)
{
    asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline void outw(uint16_t port, uint16_t data)
{
    asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline void outsl(int port, const void *addr, int cnt)
{
    asm volatile("cld; rep outsl" :
        "=S" (addr), "=c" (cnt) :
        "d" (port), "0" (addr), "1" (cnt) :
        "cc");
}

static inline void stosb(void *addr, int data, int cnt)
{
    asm volatile("cld; rep stosb" :
        "=D" (addr), "=c" (cnt) :
        "0" (addr), "1" (cnt), "a" (data) :
        "memory", "cc");
}

static inline void stosl(void *addr, int data, int cnt)
{
    asm volatile("cld; rep stosl" :
        "=D" (addr), "=c" (cnt) :
        "0" (addr), "1" (cnt), "a" (data) :
        "memory", "cc");
}

struct segdesc;

static inline void lgdt(struct segdesc *p, int size)
{
    volatile uint16_t pd[3];

    pd[0] = size-1;
    pd[1] = (unsigned int)p;
    pd[2] = (unsigned int)p >> 16;

    asm volatile("lgdt (%0)" : : "r" (pd));
}

struct gatedesc;

static inline void lidt(struct gatedesc *p, int size)
{
    volatile uint16_t pd[3];

    pd[0] = size-1;
    pd[1] = (unsigned int)p;
    pd[2] = (unsigned int)p >> 16;

    asm volatile("lidt (%0)" : : "r" (pd));
}

static inline void ltr(uint16_t sel)
{
    asm volatile("ltr %0" : : "r" (sel));
}

static inline unsigned int readeflags(void)
{
    unsigned int eflags;

    asm volatile("pushfl; popl %0" : "=r" (eflags));

    return eflags;
}

static inline void loadgs(uint16_t v)
{
    asm volatile("movw %0, %%gs" : : "r" (v));
}

static inline void cli(void)
{
    asm volatile("cli");
}

static inline void sti(void)
{
    asm volatile("sti");
}

static inline unsigned int xchg(volatile unsigned int *addr, unsigned int newval)
{
    unsigned int result;

    // The + in "+m" denotes a read-modify-write operand.
    asm volatile("lock; xchgl %0, %1" :
        "+m" (*addr), "=a" (result) :
        "1" (newval) :
        "cc");

    return result;
}

static inline unsigned int rcr2(void)
{
    unsigned int val;

    asm volatile("movl %%cr2,%0" : "=r" (val));

    return val;
}

static inline void lcr3(unsigned int val)
{
    asm volatile("movl %0,%%cr3" : : "r" (val));
}

// Layout of the trap frame built on the stack by the
// hardware and by trapasm.S, and passed to trap().

struct trapframe {
    // registers as pushed by pusha
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int oesp;      // useless & ignored
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;

    // rest of trap frame
    uint16_t gs;
    uint16_t padding1;
    uint16_t fs;
    uint16_t padding2;
    uint16_t es;
    uint16_t padding3;
    uint16_t ds;
    uint16_t padding4;
    unsigned int trapno;

    // below here defined by x86 hardware
    unsigned int err;
    unsigned int eip;
    uint16_t cs;
    uint16_t padding5;
    unsigned int eflags;

    // below here only when crossing rings, such as from user to kernel
    unsigned int esp;
    uint16_t ss;
    uint16_t padding6;
};
