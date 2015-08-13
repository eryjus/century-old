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
/* 2015-08-13  -------  Adam  Added the architecture-specific structures used to store info about processors       */
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
struct SegmentDescriptor {
    unsigned int limit_15_0 : 16;           // the low 16 bits os the segment limit
    unsigned int base_15_0 : 16;            // the low 16 bits of the base address
    unsigned int base_23_16 : 8;            // middle 8 bits of hte seg base address
};

#endif


#ifndef __ASSEMBLER__

#include <stdint.h>

typedef uint32_t pte_t;
typedef uint32_t pde_t;

extern volatile unsigned int *lapic;

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


#ifndef __ASSEMBLER__
struct TaskState {
    unsigned int link;               // Old ts selector
    unsigned int esp0;               // Stack pointers and segment selectors
    uint16_t ss0;              // after an increase in privilege level
    uint16_t padding1;
    unsigned int *esp1;
    uint16_t ss1;
    uint16_t padding2;
    unsigned int *esp2;
    uint16_t ss2;
    uint16_t padding3;
    void *cr3;               // Page directory base
    unsigned int *eip;               // Saved state from last task switch
    unsigned int eflags;
    unsigned int eax;                // More saved state (registers)
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebx;
    unsigned int *esp;
    unsigned int *ebp;
    unsigned int esi;
    unsigned int edi;
    uint16_t es;               // Even more saved state (segment selectors)
    uint16_t padding4;
    uint16_t cs;
    uint16_t padding5;
    uint16_t ss;
    uint16_t padding6;
    uint16_t ds;
    uint16_t padding7;
    uint16_t fs;
    uint16_t padding8;
    uint16_t gs;
    uint16_t padding9;
    uint16_t ldt;
    uint16_t padding10;
    uint16_t t;                // Trap on task switch
    uint16_t iomb;             // I/O map base address
};

#endif


//
// -- The following section is from the Multiprocessor Specification (v1.4).  This specification
//    can be found at http://developer.intel.com/design/pentium/datashts/24201606.pdf.
//    ------------------------------------------------------------------------------------------

#ifndef __ASSEMBLER__

//
// -- This structure contains a physical address pointer to the MP configuration table and other MP feature
//    information bytes.  When present, this structure indicates that the system conforms to the MP specification.
//    This structure must be stored in at least one of the following memory locations, because the operating system
//    searches for the MP floating pointer structure in the order described below:
//    a.   In the first kilobyte of Extended BIOS Data Area (EBDA), or
//    b.   Within the last kilobyte of system base memory (e.g., 639K-640K for systems with 640KB of base memory or
//         511K-512K for systems with 512 KB of base memory) if the EBDA segment is undefined, or
//    c.   In the BIOS ROM address space between 0F0000h and 0FFFFFh.
//    -------------------------------------------------------------------------------------------------------------
struct MPFloatingPointer {                  // FLoating Pointer Structure
    uint8_t signature[4];                   // contains "_MP_"
    void *physAddr;                         // phys addr of MP config table
    uint8_t length;                         // length in 16-byte blocks, which is exactly 1
    uint8_t specRev;                        // revision level, which should be [14]
    uint8_t checkSum;                       // all bytes must add up to 0
    uint8_t type;                           // MP system config type
    uint8_t imcrp;                          // Bit 7 indicates that IMCRP is present (PIC mode implemented)
    uint8_t reserved[3];                    // Reserved, but should be taken into account when checking the checksum
};


struct MPConfig {                           // configuration table header
    uint8_t signature[4];                   // "PCMP"
    uint16_t length;                        // total table length
    uint8_t version;                        // [14]
    uint8_t checkSum;                       // all bytes must add up to 0
    uint8_t product[20];                    // product id
    unsigned int *oemTable;                 // OEM table pointer
    uint16_t oemLength;                     // OEM table length
    uint16_t entry;                         // entry count
    unsigned int *lapicAddr;                // address of local APIC
    uint16_t xLength;                       // extended table length
    uint8_t xCheckSum;                      // extended table checksum
    uint8_t reserved;                       // Reserved, but should be taken into account when checking the checksum
};


struct MPProcessor {                        // processor table entry
    uint8_t type;                           // entry type (0)
    uint8_t apicId;                         // local APIC id
    uint8_t version;                        // local APIC verison
    uint8_t flags;                          // CPU flags
    uint8_t signature[4];                   // CPU signature
    unsigned int feature;                       // feature flags from CPUID instruction
    uint8_t reserved[8];                    // Reserved, but should be taken into account when checking the checksum
};

#define MPP_BOOT 0x02                       // This cpu is the bootstrap processor.


struct MPIoapic {                           // I/O APIC table entry
    uint8_t type;                           // entry type (2)
    uint8_t apicNo;                         // I/O APIC id
    uint8_t version;                        // I/O APIC version
    uint8_t flags;                          // I/O APIC flags
    unsigned int *addr;                     // I/O APIC address
};


#define MP_PROC         0x00                // One per processor
#define MP_BUS          0x01                // One per bus
#define MP_IOAPIC       0x02                // One per I/O APIC
#define MP_IOINTR       0x03                // One per bus interrupt source
#define MP_LINTR        0x04                // One per system interrupt source

#endif

#endif
