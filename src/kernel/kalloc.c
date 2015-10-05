//===================================================================================================================
//
// kalloc.c -- This file contains the physical memort allocator functions and structures
//
// The phycal memory allocator is used to allocate big blocks of memory (4K) for user processes, kernel stacks, page
// tables, pipe buffers, and the like.  Its implementation is of a linked list (stack) of pages where the linked list
// is mapped into the first few bytes of the empty page.
//
//      CenturyOS is a Hobby Operating System written mostly in C.
//          Copyright (c) 2014-2015  Adam Scott Clark
//      Portions (c) 2006-2009 Frans Kaashoek, Robert Morris, Russ Cox, Massachusetts Institute of Technology
//
//      This program is free software: you can redistribute it and/or modify it under the terms of the GNU General
//      Public License as published by the Free Software Foundation, either version 3 of the License, or any later
//      version.
//
//      This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
//      implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//      for more details.
//
//      You should have received a copy of the GNU General Public License along with this program.  If not, see
//      http://www.gnu.org/licenses/gpl-3.0-standalone.html.
//
// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages after installing a full page table that maps them
//    on all cores.
//
// There are several sanity checks that have been incorporated into these functions.  The basic sanity checks are
// on at all times, but the extended sanity checks can be enabled by defining the macro KALLOC_SANITY when
// compiling this file.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Pgmr  Description
//  ----------  -------  ----  ------------------------------------------------------------------------------------
//  2015-10-02    #50    ADCL  There is something corrupting the kmem structures, so the first order of business is
//                             to add some sanity checks into these management functions.  There are already some
//                             such checks, but they are not nearly robust enough for troubleshooting, at least not
//                             in the initial stages.  In addition, I will start to modify and format this source
//                             to match my personal development standards.
//
//===================================================================================================================

#define KALLOC_SANITY

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "x86.h"

static void freerange(void *vstart, void *vend);
extern char end[];              // first address after kernel loaded from ELF file

//
// -- This structure is mapped into the first few bytes of an empty page on the free pages list.  Quite simply,
//    it is a linked list that implements a stack of free pages.
//    ---------------------------------------------------------------------------------------------------------
struct run {
    struct run *next;           // points to the next free page, or 0 if this is the last free page.
};


//
// -- This structure and the kmem variable is used to keep track of the free pages.  It is used and managed in
//    several points in time during initialization.  During early initialization, we will place a number of free
//    free pages in the stack to get us started.  During this early initialization, we will not be able to use
//    the lock.  Later, we will add more pages into the stack as we know where the end of memory is, and turn on
//    the use of the lock.  Any later uses of this structure will then require the lock since additional processes
//    might want to use the structure at the same time.
//    ------------------------------------------------------------------------------------------------------------
struct {
  struct spinlock lock;         // this is the lock
  int use_lock;                 // is locking on the structure enabled?
  struct run *freelist;         // this is the stack of free pages
} kmem;


//-------------------------------------------------------------------------------------------------------------------
// kinit1() -- the early initialization of memory.  It will free the pages that are initially mapped in the startup
//             page table (with 1M pages).
//-------------------------------------------------------------------------------------------------------------------
void kinit1(void *vstart, void *vend)
{
    initlock(&kmem.lock, "kmem");
    kmem.use_lock = 0;              // no need to use the lock yet
    freerange(vstart, vend);
}


//-------------------------------------------------------------------------------------------------------------------
// kinit2() -- the late initialization for the memory structures.  It will free all the remaining paged that are
//             covered by the full paging tables available to all processors.
//-------------------------------------------------------------------------------------------------------------------
void kinit2(void *vstart, void *vend)
{
    freerange(vstart, vend);
    kmem.use_lock = 1;              // now, we need to enforce locking
}


//-------------------------------------------------------------------------------------------------------------------
// freerange() -- free a range of memory from vstart to vend.  Note that the addresses passed in do not need to be
//                page aligned.  Any partial page at vstart is skilled to the next full page.  Though a little more
//                subtle, vend is checked at the end to not "free" any partial page at the end of the range.
//-------------------------------------------------------------------------------------------------------------------
static void freerange(void *vstart, void *vend)
{
    char *p = (char *)PGROUNDUP((uint)vstart);

    for ( ; p + PGSIZE <= (char *)vend; p += PGSIZE) kfree(p);
}


//-------------------------------------------------------------------------------------------------------------------
// kfree() -- Free the page of physical memory pointed at by v, which normally should have been returned by a call to
//            kalloc().  (The exception is when initializing the allocator; see kinit1 & kinit2 above.)
//
// There are some assitional sanity checks to be added into the proess:
// 1.  kmem.freelist % PGSIZE == 0
// 2.  kmem.freelist >= end
// 3.  v2p(kmem.freelist) < PHYSTOP
//
// In addition, when KALLOC_SANITY is defined, the individual checks for the page to free (v) will be individualized.
//-------------------------------------------------------------------------------------------------------------------
void kfree(char *v)
{
    struct run *r;

#ifdef KALLOC_SANITY
    if ((uint)v % PGSIZE) panic("kfree: unaligned page");
    if ((uint)v < (uint)end) panic("kfree: freeing memory in kernel proper");
    if (v2p(v) >= PHYSTOP) panic("kfree: physical page out of range");

    if ((uint)kmem.freelist % PGSIZE) { BREAK; panic("kfree: unaligned top of stack"); }
    if (kmem.freelist && (uint)kmem.freelist < (uint)end) { BREAK; panic("kfree: top of stack too in kernel"); }
    if (kmem.freelist && v2p(kmem.freelist) >= PHYSTOP) { BREAK; panic("kfree: top of stack past physical memory"); }
#else
    if((uint)v % PGSIZE || v < end || v2p(v) >= PHYSTOP) panic("kfree");
#endif

    // Fill with junk to catch dangling refs.
    memset(v, (uint)-1, PGSIZE);

    if (kmem.use_lock) acquire(&kmem.lock);

    // push the frame on the stack
    r = (struct run *)v;
    r->next = kmem.freelist;
    kmem.freelist = r;

    if (kmem.use_lock) release(&kmem.lock);
}


//-------------------------------------------------------------------------------------------------------------------
// kalloc -- Allocate one 4096-byte page of physical memory.  Returns a pointer that the kernel can use.  Returns 0
//           if the memory cannot be allocated.
//
// There are some assitional sanity checks to be added into the proess:
// 1.  kmem.freelist % PGSIZE == 0
// 2.  kmem.freelist >= end
// 3.  v2p(kmem.freelist) < PHYSTOP
//-------------------------------------------------------------------------------------------------------------------
char *kalloc(void)
{
    struct run *r;

#ifdef KALLOC_SANITY
    if ((uint)kmem.freelist % PGSIZE) { BREAK; panic("kalloc: unaligned top of stack"); }
    if (kmem.freelist && (uint)kmem.freelist < (uint)end) { BREAK; panic("kalloc: top of stack too in kernel"); }
    if (kmem.freelist && v2p(kmem.freelist) >= PHYSTOP) { BREAK; panic("kalloc: top of stack past phys memory"); }
#endif

    if(kmem.use_lock) acquire(&kmem.lock);

    r = kmem.freelist;
    if(r) kmem.freelist = r->next;

    if(kmem.use_lock) release(&kmem.lock);

    return (char*)r;
}

