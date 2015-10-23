//===================================================================================================================
//
// syscall.c -- This file contains the functions necessary to process a system call.
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
// User code makes a system call with INT T_SYSCALL.  System call number in %eax.  Arguments on the stack, from the
// user call to the C library system call function. The saved user %esp points to a saved program counter, and then
// the first argument.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Version  Tracker  Pgmr  Description
//  ----------  -------  -------  ----  -----------------------------------------------------------------------------
//  2015-10-01   v0.3      #64    ADCL  Add partition block related system calls
//  2015-10-21   v0.3      #83    ADCL  Document this file and format it in accordance with my personal standards.
//  2015-10-22   v0.3      #75    ADCL  Publish a feature to read a raw disk sector
//
//===================================================================================================================


#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"


//-------------------------------------------------------------------------------------------------------------------
// fetchint() -- Fetch the int at addr from process p.
//-------------------------------------------------------------------------------------------------------------------
int fetchint(struct proc *p, uint addr, int *ip)
{
    if (addr >= p->sz || addr + 4 > p->sz) return -1;
    *ip = *(int*)(addr);

    return 0;
}


//-------------------------------------------------------------------------------------------------------------------
// fetchstr() -- Fetch the nul-terminated string at addr from process p.  Doesn't actually copy the string - just
//               sets *pp to point at it.  Returns length of string, not including null.
//-------------------------------------------------------------------------------------------------------------------
int fetchstr(struct proc *p, uint addr, char **pp)
{
    char *s, *ep;

    if (addr >= p->sz) return -1;
    *pp = (char*)addr;
    ep = (char*)p->sz;

    for (s = *pp; s < ep; s ++) if (*s == 0) return s - *pp;

    return -1;
}


//-------------------------------------------------------------------------------------------------------------------
// argint() -- Fetch the nth 32-bit system call argument.
//-------------------------------------------------------------------------------------------------------------------
int argint(int n, int *ip)
{
    return fetchint(proc, proc->tf->esp + 4 + 4 * n, ip);
}


//-------------------------------------------------------------------------------------------------------------------
// argptr() -- Fetch the nth word-sized system call argument as a pointer to a block of memory of size n bytes.
//             Check that the pointer lies within the process address space.
//-------------------------------------------------------------------------------------------------------------------
int argptr(int n, char **pp, int size)
{
    int i;

    if (argint(n, &i) < 0) return -1;
    if ((uint)i >= proc->sz || (uint)i+size > proc->sz) return -1;

    *pp = (char*)i;

    return 0;
}


//-------------------------------------------------------------------------------------------------------------------
// argstr() -- Fetch the nth word-sized system call argument as a string pointer.  Check that the pointer is valid
//             and the string is null-terminated.  (There is no shared writable memory, so the string can't change
//             between this check and being used by the kernel.)
//-------------------------------------------------------------------------------------------------------------------
int argstr(int n, char **pp)
{
    int addr;

    if (argint(n, &addr) < 0) return -1;

    return fetchstr(proc, addr, pp);
}


//
// -- Some function prototypes for local use
//    --------------------------------------
extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);
extern int sys_shared(void);
extern int sys_pread(void);
extern int sys_pwrite(void);
extern int sys_ext2readsb(void);
extern int sys_readsect(void);


//
// -- This array contains a list of functions to be called based on the system call function number requested.
//    --------------------------------------------------------------------------------------------------------
static int (*syscalls[])(void) = {
    [SYS_fork]    sys_fork,
    [SYS_exit]    sys_exit,
    [SYS_wait]    sys_wait,
    [SYS_pipe]    sys_pipe,
    [SYS_read]    sys_read,
    [SYS_kill]    sys_kill,
    [SYS_exec]    sys_exec,
    [SYS_fstat]   sys_fstat,
    [SYS_chdir]   sys_chdir,
    [SYS_dup]     sys_dup,
    [SYS_getpid]  sys_getpid,
    [SYS_sbrk]    sys_sbrk,
    [SYS_sleep]   sys_sleep,
    [SYS_uptime]  sys_uptime,
    [SYS_open]    sys_open,
    [SYS_write]   sys_write,
    [SYS_mknod]   sys_mknod,
    [SYS_unlink]  sys_unlink,
    [SYS_link]    sys_link,
    [SYS_mkdir]   sys_mkdir,
    [SYS_close]   sys_close,
    [SYS_shared]  sys_shared,
    [SYS_partblockread]   sys_pread,
    [SYS_partblockwrite]   sys_pwrite,
    [SYS_ext2readsb]    sys_ext2readsb,
    [SYS_readsect]  sys_readsect,
};


//-------------------------------------------------------------------------------------------------------------------
// syscall() -- do the actual business of a system call
//-------------------------------------------------------------------------------------------------------------------
void syscall(void)
{
    int num;

    num = proc->tf->eax;

    if (num >= 0 && num < SYS_open && syscalls[num]) {
        proc->tf->eax = syscalls[num]();
    } else if (num >= SYS_open && num < NELEM(syscalls) && syscalls[num]) {
        proc->tf->eax = syscalls[num]();
    } else {
        cprintf("%d %s: unknown sys call %d\n", proc->pid, proc->name, num);
        proc->tf->eax = -1;
    }
}
