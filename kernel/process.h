/* =============================================================================================================== */
/*                                                                                                                 */
/* process.h -- This file contains the structures for manaing processes across multiple processors                 */
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
/* 2015-08-13  Initial  Adam  This is the initial version.                                                         */
/*                                                                                                                 */
/* =============================================================================================================== */

#ifndef __PROCESS_H__
#define __PROCESS_H__


#include <stdbool.h>
#include <stdint.h>
#include "kern-defs.h"


//
// -- Some forward declarations
//    -------------------------
struct context;
struct proc;
struct taskstate;

// Per−CPU state
struct Cpu {
    uint8_t id;                         // Local APIC ID; index into cpus[] below
    struct context *scheduler;          // swtch() here to enter scheduler
    struct TaskState ts;                // Used by x86 to find stack for interrupt
    struct SegmentDescriptor gdt[NSEGS];          // x86 global descriptor table
    volatile bool started;              // Has the CPU started?
    int nCli;                           // Depth of pushcli nesting.
    int intEnabled;                     // Were interrupts enabled before pushcli?

    // Cpu−local storage variables; see below
    struct Cpu *cpu;                    // Pointer to this element, i.e.: &cpus[ID]
    struct Proc *proc;                  // The currently−running process
};


//
// -- Declare the cpu-local variables.  This means that there are one unique set per CPU.
//    So, this is where it gets tricky.  I will try to explain it carefully so that it is easily understood.
//    This array is contains one entry per CPU.  Each CPU needs to be able to access it's own data in this array
//    through a standard defined variable.  In this way, the same code can be written for each CPU, and the
//    specific setup of the CPU will allow access to the specific data unique for that CPU.  The key to success
//    in this endeavor is the GDT.  We will set up a specific segment for this CPU-specific data.  The
//    SegmentInit() function will take good care of this for us.
//
//    The point here will be that we will define a really short segment for the CPU-specific data that has an
//    offset to the start of the data (not 0 as typically defined) and a length of 8 (for the 2 pointers we
//    will establish).
//
//    Finally, we will need to tell the compiler to tell the assembler how to handle the variable access.  We
//    will do this when we declare the variable.  They will be declared as extern, but will provide the access
//    definition in the declaration through an "asm()" suffix.
//
//    So, we first declare the cpus array.  Within the structure element of this array, will have 2 elements
//    (cpu and proc) that will ultimately become the unique data segmetn for this CPU.
//    ----------------------------------------------------------------------------------------------------------
extern struct Cpu cpus[NCPUS];


// -- Now, we will declare the standard pointer to the cpu element in the cpus array.  The suffix we will add
//    to the end of the external variable declaration will instruct the compiler (and by extension the assembler)
//    how to access this variable.  In the case of cpu, this will be at segment gs and offset 0 (%gs:0).  Now,
//    the key will be to construct the gs segment definition properly (again, see the SegmentInit() function).
//    For now, we are only concerned with informing the compiler where this data is stored, rather than allocating
//    it on its own in the data segment.  Ultimately, it is the segment descriptor that is cpu-specific -- and
//    that segment descriptor is the magic!
//
//    The short version is this: the address of cpu is %gs:0, and the segment pointed to by %gs is
//    &cpus[CpuNum()].  cpu == cpus[CpuNum()].cpu == &cpus[CpuNum()]
//    ------------------------------------------------------------------------------------------------------------
extern struct Cpu *cpu asm("%gs:0");


// -- The currently running proc on this CPU is also handled the same as cpu above, except that it is the next
//    double word after the cpu address.
//
//    proc = cpus[CpuNum()].proc.
//    --------------------------------------------------------------------------------------------------------
extern struct Proc *proc asm("%gs:4");
