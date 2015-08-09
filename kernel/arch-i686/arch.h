/* =============================================================================================================== */
/*                                                                                                                 */
/* arch-i686.h -- This file contains the information of several key bits of information for the 32-bit processors  */
/*                                                                                                                 */
/* This is an architecture-specific implementation of several processor-related flags, registers, stack states,    */
/* etc.  This file should be the one-stop for all the 32-bit intel processor code.                                 */
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
/* The memory layout for Century on a 32-bit processor (non-PAE) is organized as follows:                          */
/*                                                                                                                 */
/*  +------------+------------+-------------------------+----------------------------------------------------+     */
/*  | from virt  |  to virt   |      mapped physical    |                                                    |     */
/*  |   memory   |   memory   |      memory address     |                                                    |     */
/*  |  address   |  address   |          range          |  Usage                                             |     */
/*  +------------+------------+-------------------------+----------------------------------------------------+     */
/*  | 0x00000000 | 0x80000000 |          varies         | User space                                         |     */
/*  +------------+------------+-------------------------+----------------------------------------------------+     */
/*  |   P2V(0)   |   stext    |          0 - 1MB        | device access                                      |     */
/*  +------------+------------+-------------------------+----------------------------------------------------+     */
/*  |   stext    |    data    | 0x00100000 - v2p(data)  | kernel code (etext is provided by the linker)      |     */
/*  +------------+------------+-------------------------+----------------------------------------------------+     */
/*  |   data     |  PHYS_END  | v2p(data) -             | kernel data (including and stack; end provided     |     */
/*  |            |            |           v2p(PHYS_END) | by the linker)                                     |     */
/*  +------------+------------+-------------------------+----------------------------------------------------+     */
/*  | 0xfe000000 | 0x00000000 | 0xfe000000 - 0x00000000 | additional direct access devices (like ioapic)     |     */
/*  +------------+------------+-------------------------+----------------------------------------------------+     */
/*                                                                                                                 */
/* --------------------------------------------------------------------------------------------------------------- */
/*                                                                                                                 */
/*    Date     Tracker  Pgmr  Description                                                                          */
/* ----------  -------  ----  -----------------------------------------------------------------------------------  */
/* 2015-07-31  Initial  Adam  This is the initial version.  Portions inspired by xv6.                              */
/*                                                                                                                 */
/* =============================================================================================================== */

#ifndef __ARCH_H__
#define __ARCH_H__ 1


#define CONSOLE_COLS 80
#define CONSOLE_ROWS 25

#ifdef __ASSEMBLER__
#define CONSOLE_MEMORY (0xb8000)
#else
#define CONSOLE_MEMORY (uint16_t *)0xb8000
#endif

//
// -- Some basic configuration constants needed to 32-bit processors
//    --------------------------------------------------------------
#define     STACK_SIZE      (0x1000)        // use 4K stacks


//
// -- the following are the flags bits that are in the eflags register
//    ----------------------------------------------------------------
#define     FLG_CF          (1<<0)          // carry flag
#define     FLG_PF          (1<<2)          // parity flag
#define     FLG_AF          (1<<4)          // auxiliary carry flag
#define     FLG_ZF          (1<<6)          // zero flag
#define     FLG_SF          (1<<7)          // sign flag
#define     FLG_TF          (1<<8)          // trap flag
#define     FLG_IF          (1<<9)          // interrupt flag
#define     FLG_DF          (1<<10)         // direction flag
#define     FLG_OF          (1<<11)         // overflow flag
#define     FLG_IOPL_MASK   (3<<12)         // IO Privilege level mask
#define     FLG_IOPL_0      (0)             // IOPL 0
#define     FLG_IOPL_1      (1<<12)         // IOPL 1
#define     FLG_IOPL_2      (2<<12)         // IOPL 2
#define     FLG_IOPL_3      (3<<12)         // IOPL 3
#define     FLG_NT          (1<<14)         // nested task flag
#define     FLG_RF          (1<<16)         // resume flag
#define     FLG_VM          (1<<17)         // virtual 8086 mode
#define     FLG_AC          (1<<18)         // alignment check
#define     FLG_VIF         (1<<19)         // virtual interrupt flag
#define     FLG_VIP         (1<<20)         // virtual interrupt pending
#define     FLG_ID          (1<<21)         // ID Flag


//
// -- Control register flags
//    ----------------------
#define     CR0_PE          (1<<0)          // Protetion Enabled
#define     CR0_MP          (1<<1)          // Monitor Co-Processor
#define     CR0_EM          (1<<2)          // Emulation
#define     CR0_TS          (1<<3)          // Task switched
#define     CR0_ET          (1<<4)          // Extension Type
#define     CR0_NE          (1<<5)          // Numeric Error
#define     CR0_WP          (1<<16)         // Write Protect
#define     CR0_AM          (1<<18)         // Alignment Mask
#define     CR0_NW          (1<<29)         // Not write-through
#define     CR0_CD          (1<<30)         // Cache Disable
#define     CR0_PG          (1<<31)         // Paging Enabled

#define     CR4_PSE         (1<<4)          // Page Size extension


//
// -- Segment index numbers
//    ---------------------
#define     SEG_IDX_KCODE   1               // kernel code
#define     SEG_IDX_KDATA   2               // kernel data & stack
#define     SEG_IDX_KCPU    3               // kernel per-cpu data
#define     SEG_IDX_UCODE   4               // user code
#define     SEG_IDX_UDATA   5               // user daata & stack
#define     SEG_IDX_TSS     6               // this process's task state


//
// -- The following structure and intiailzer macro cannot be used in assembly.
//    ------------------------------------------------------------------------

#ifndef __ASSEMBLER__

//
// -- segment descriptor
//    ------------------
struct seg_desc {
    unsigned int limit_15_0 : 16;           // the low 16 bits os the segment limit
    unsigned int base_15_0 : 16;            // the low 16 bits of the base address
    unsigned int base_23_16 : 8;            // middle 8 bits of hte seg base address
};

#endif


#ifndef __ASSEMBLER__

#include <stdint.h>

typedef uint32_t pte_t;
typedef uint32_t pde_t;

#endif

//
// -- The following constants and macros are used for virtual memory management
//    -------------------------------------------------------------------------
#define KERN_BASE           0x80000000
#define KERN_LINK           (KERN_BASE + (1024 * 1024))     // or, 0x80100000
#define PHYS_STOP           0x0e000000
#define DEV_SPACE           0xfe000000

#define PAGE_SIZE           4096
#define NUM_PDENTRIES       1024
#define NUM_PTENTRIES       1024

#define PD_SHIFT            22
#define PT_SHIFT            12

#define PD_IDX(va)          (((unsigned int)(va) >> PD_SHIFT) & 0x3ff)
#define PT_IDX(va)          (((unsigned int)(va) >> PT_SHIFT) & 0x3ff)

#define PAGE_ROUNDUP(sz)    (((sz)+PAGE_SIZE-1) & ~(PAGE_SIZE-1))
#define PAGE_ROUNDDOWN(a)   (((a)) & ~(PAGE_SIZE-1))

#define PTE_P               1<<0            // Present
#define PTE_W               1<<1            // writable
#define PTE_U               1<<2            // User
#define PTE_PWT             1<<3            // Write-Through
#define PTE_PCD             1<<4            // cache disable
#define PTE_A               1<<5            // Accessed
#define PTE_D               1<<6            // Dirty
#define PTE_PS              1<<7            // Page Size

#define PTE_ADDR(pte) ((unsigned int)(pte) & ~0xFFF)

#ifdef __ASSEMBLER__

#define V2P(x) x - (KERN_BASE)              // same as C V2P, but without casts
#define P2V(x) x + (KERN_BASE)              // same as C P2V, but without casts

#else

#define V2P(a) ((unsigned int) a - KERN_BASE)
#define P2V(a) ((void *) a + KERN_BASE)

static inline unsigned int v2p(void *a) { return (unsigned int) a - KERN_BASE; }
static inline void *p2v(unsigned int a) { return (void *) a + KERN_BASE; }

extern char end[];          // This will be provided by the linker and will be page-aligned
extern char data_start[];

void lcr3(unsigned int);
unsigned int rcr3(void);

#endif

#endif
