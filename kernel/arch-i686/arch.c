/* =============================================================================================================== */
/*                                                                                                                 */
/* page.c -- This file contains initial paging table for bootstrapping Century.                                    */
/*                                                                                                                 */
/* This is an architecture-specific the initial paging tables.                                                     */
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
/* 2015-08-13  -------  Adam  Added the architecture-specific code to detect the number of processors and collect  */
/*                            that data in internal structures.                                                    */
/*                                                                                                                 */
/* =============================================================================================================== */

#include "x86.h"


#include "arch.h"
#include "../kern-defs.h"
#include "../process.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


//===================================================================================================================
//
// == This section contains the Architecture-Specific functions for the virtual memory implementation.
//
//===================================================================================================================


//
// -- This table will be used to setup the bootstrap page tables.  This will be very temporaty and will be replaced
//    as soon as possible during the initialization.  These pages are 4MB in size.
//    -------------------------------------------------------------------------------------------------------------
__attribute__((__aligned__(PAGE_SIZE)))

pte_t bootPD[NUM_PDENTRIES] = {
    [0]                         = ((0) | PTE_P | PTE_W | PTE_PS),    // Identity map the first 4MB memory
    [KERN_BASE >> PD_SHIFT]     = ((0) | PTE_P | PTE_W | PTE_PS),    // Map the upper mem loc to the proper phys
};


//
// -- This structure is used to define the memory configuration used by the kernel.  The kernel will use the upper
//    half of the virtual memory space.  Therefore, the drivers in the first MB of address space are mapped up to
//    the first MB after address 0x80000000 (such as the video memory buffer).  For things to work out properly,
//    note that the virtual address from 0x80000000 to P2V(PHY_STOP) is mapped contiguous to physical addresses
//    0 to PHYS_TOP.  This is the secret to the memory mapping success.
//    ------------------------------------------------------------------------------------------------------------
static struct kmap_t {
    void *virtAddr;
    unsigned int physStart;
    unsigned int physEnd;
    int permissions;
} kernelMemoryMap[] = {
//  { virt addr             from phys addr      to phys addr        permissions  }
//    ------------          ------------------  ------------------  -----------
    { P2V(0),               0,                  1024 * 1024,        PTE_W        },
    { (void *)KERN_LINK,    V2P(KERN_LINK),     V2P(data_start),    0            },
    { data_start,           V2P(data_start),    PHYS_STOP,          PTE_W        },
    { (void *)DEV_SPACE,    DEV_SPACE,          0,                  PTE_W        },
};


//
// -- This variable is the pointer to the kernel page directory in virtual address space.
//    -----------------------------------------------------------------------------------
pte_t *kernPageDir;


//
// -- This function will return the tage table entry (pointer) for the specified virtual address.  If
//    needed, it will allocate more memory from the kernel heap for a new page table, using the supplied
//    function.  This supplied function is important because it will be different during startup and
//    normal operation.  If a new table is allocated, it clears the bits to 0 to ensure that there
//    is no garbage to impact operation.
//    --------------------------------------------------------------------------------------------------
static pte_t *WalkPageDir(pde_t *pgDir, const void *virtAddr, char *(*alloc)())
{
    pde_t *pde;
    pte_t *pgTab;

    pde = &pgDir[PD_IDX(virtAddr)];

    if (*pde & PTE_P) pgTab = (pte_t *)p2v(PTE_ADDR(*pde));
    else {
        if (!alloc || (pgTab = (pte_t *)alloc()) == 0) return 0;

        memset(pgTab, 0, PAGE_SIZE);
        *pde = v2p(pgTab) | PTE_P | PTE_W | PTE_U;
    }

    return &pgTab[PT_IDX(virtAddr)];
}


//
// -- The purpose of this function is to map a block of virtual address pages to physical address frames.
//    ---------------------------------------------------------------------------------------------------
static int MapPages(pde_t *pgDir, void *virtAddr, unsigned int size, unsigned int physAddr, int permissions,
        char *(*alloc)())
{
    char *addr;
    char *last;
    pte_t *pte;

    if (physAddr & (PAGE_SIZE-1)) panic("Physical Address Alignment");
    if ((unsigned int)virtAddr & (PAGE_SIZE-1)) panic("Virtual Address Alignment");

    addr = (char *)PAGE_ROUNDDOWN((unsigned int)virtAddr);
    last = (char *)PAGE_ROUNDDOWN((unsigned int)virtAddr + size - 1);

    for (;;) {
        pte = WalkPageDir(pgDir, addr, alloc);
        if (pte == 0) return -1;
        if (*pte & PTE_P) panic("remapping present page");

        *pte = physAddr | permissions | PTE_P;

        if (addr == last) break;

        addr += PAGE_SIZE;
        physAddr += PAGE_SIZE;
    }

    return 0;
}


//
// -- This function is used to setup the virtual memory manager, creating the page tables for normal
//    operation.  Recall that in order to bootstrap the kernel, a small page table was created with 1
//    4MB page that was identity mappped.  With this setup, we will be mapping the kernel into higher
//    memory.
//    -----------------------------------------------------------------------------------------------
pde_t *SetupKernVM(char *(*alloc)())
{
    pde_t *pgDir;
    struct kmap_t *k;

    if (p2v(PHYS_STOP) >= (void *)DEV_SPACE) panic("Physical Memory Limit (PHYS_STOP) too big");

    pgDir = (pde_t *)alloc();
    if (pgDir == 0) return 0;
    memset(pgDir, 0, PAGE_SIZE);

    for (k = kernelMemoryMap; k < &kernelMemoryMap[NELEM(kernelMemoryMap)]; k ++) {
        if (MapPages(pgDir, k->virtAddr, k->physEnd - k->physStart, k->physStart, k->permissions, alloc) < 0) {
            return 0;
        }
    }

    return pgDir;
}


//
// -- This function is used to switch paging to the kernel page directory.  Notice the traslation to the physical
//    address.
//    -----------------------------------------------------------------------------------------------------------
void SwitchKernPageDir(void)
{
    lcr3(v2p(kernPageDir)); // switch to the kernel page table
}


//
// -- This function is called from the kernel initialization function to setup the Virtual Memory Manager.  When
//    this function exist, we will be running in upper memory.
//    ----------------------------------------------------------------------------------------------------------
void VMMInit(void)
{
    kernPageDir = SetupKernVM(AllocAtEntry);
    SwitchKernPageDir();
}


//===================================================================================================================
//
// == This section contains the Architecture-Specific functions for MultiProcessor Specification.
//
//===================================================================================================================



struct Cpu cpus[NCPUS];
static struct Cpu *bcpu;
bool isMP = false;
int nCPU;
uint8_t ioapicID;
volatile unsigned int *lapic;
static struct MPFloatingPointer *fltTable;

//
// -- This function will execute the sanity checks to ensure that we have a good Floating Pointer table and
//    subordinate Config table.
//    -----------------------------------------------------------------------------------------------------
static bool SanityCheckMPPtr(struct MPFloatingPointer *mpPtr)
{
    uint8_t sum = 0, *wrk = (uint8_t *)mpPtr;
    int len = sizeof(struct MPFloatingPointer);
    struct MPConfig *ptr;

    while (len > 0) sum += wrk[-- len];
    if (sum) return false;
    if (*((uint32_t *)p2v((unsigned int)mpPtr->physAddr)) != *((uint32_t *)"PCMP")) return false;

    ptr = p2v((unsigned int)mpPtr->physAddr);
    if (ptr->version != 1 && ptr->version != 4) return false;

    return true;
}


//
// -- This function will search a block of memory for the _MP_ MultiProcessor signature and if found, do some
//    extended checking for the proper signature and checksum.
//    -------------------------------------------------------------------------------------------------------
static struct MPConfig *SearchForConfigTable(uint8_t *start, unsigned int length)
{
    for (; length > 0; start += 16, length -= 16) {
        if (*((uint32_t *)start) == *((uint32_t *)"_MP_") && SanityCheckMPPtr((struct MPFloatingPointer *)start)) {
            struct MPFloatingPointer *wrk = (struct MPFloatingPointer *)start;
            fltTable = wrk;
            return (struct MPConfig *)p2v((unsigned int)wrk->physAddr);
        }
    }

    return 0;
}


//
// -- This function will search all the known locations for the MultiProcessor Configuration table.  It may
//    be located in one of 3 places, which are searched in order:
//    1.  The first 1K of the Extended BIOS Data Area
//    2.  The last 1K of the low memory (memory before 640K)
//    3.  The address range from 0xf0000 to 0xfffff
//    -----------------------------------------------------------------------------------------------------
static struct MPConfig *FindConfigTable(void)
{
    struct MPConfig *cfgTable;
    uint8_t *bda = p2v(0x400);

    cfgTable = SearchForConfigTable(p2v((bda[0x0f] << 8 | bda[0x0e]) << 4), 1024);
    if (!cfgTable) cfgTable = SearchForConfigTable(p2v((bda[0x14] << 8 | bda[0x13]) * 1024) - 1024, 1024);
    if (!cfgTable) cfgTable = SearchForConfigTable(p2v(0xf0000), 0x10000);
    if (!cfgTable) return 0;

    return cfgTable;
}


//
// -- Initialize the multiprocessor configuration
//    -------------------------------------------
void MultiProcessorInit(void)
{
    struct MPConfig *cfgTable = FindConfigTable();
    uint8_t *ptr, *end;
    struct MPProcessor *proc;
    struct MPIoapic *ioapic;

    bcpu = &cpus[0];                    // set a default boot cpu

    if (!cfgTable) goto NoMP;

    isMP = true;
    lapic = cfgTable->lapicAddr;
    ptr = ((uint8_t *)cfgTable) + sizeof(struct MPConfig);
    end = ((uint8_t *)cfgTable) + cfgTable->length;

    while (ptr < end) {
        switch (*ptr) {
        case MP_PROC:
            proc = (struct MPProcessor *)ptr;

            if (nCPU != proc->apicId) {
                printf("MultiProcessor Specification: ncpu=%d; apicid=%d\n", nCPU, proc->apicId);
                isMP = false;
                goto NoMP;
            }

            if (proc->flags & MPP_BOOT) bcpu = &cpus[nCPU];

            cpus[nCPU].id = nCPU;
            nCPU ++;
            ptr += sizeof(struct MPProcessor);
            break;

        case MP_IOAPIC:
            ioapic = (struct MPIoapic *)ptr;
            ioapicID = ioapic->apicNo;
            ptr += sizeof(struct MPIoapic);
            break;

        case MP_BUS:
        case MP_IOINTR:
        case MP_LINTR:
            ptr += 8;
            break;

        default:
            printf("Unknown MP specification %d @ %p\n", *ptr, ptr);
            isMP = false;
            goto NoMP;
        }
    }

    if (!isMP) {
NoMP:                   // Something went wrong, so assume we are just a single processor
        nCPU = 1;
        lapic = 0;
        ioapicID = 0;

        return;
    }

    if (fltTable->imcrp) {
        outb(0x22, 0x70);                   // Select IMCR
        outb(0x23, inb(0x23) | 1);          // Mask external interrupts.
    }
}
