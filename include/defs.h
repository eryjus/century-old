//===================================================================================================================
//
// defs.h -- This file contains the forward definitions and function prototypes for xv6, and ultimately CenturyOS.
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
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Version  Tracker  Pgmr  Description
//  ----------  -------  -------  ----  -----------------------------------------------------------------------------
//  2015-10-14   v0.3    #64,67   ADCL  Add the function prototypes into this file for reading and writing a
//                                      partition block
//  2015-10-14   v0.3      #88    ADCL  Properly document this file to conform to my standards
//  2015-10-21   v0.3      #89    ADCL  Add the ability to read an EXT2 superblock
//
//===================================================================================================================


#include <stdbool.h>

//
// -- Some forward definitions for pointers in the following prototypes
//    -----------------------------------------------------------------
struct buf;             // buf.h -- This is a buffered IO buffer structure
struct context;         // proc.h -- This is a process context for several registers
struct file;            // file.h -- This is a file structure
struct inode;           // file.h -- This is an inode structure on disk
struct pipe;            // pipe.c -- This is a structure to control piping data from one program to another
struct proc;            // proc.h -- This is a process structure
struct spinlock;        // spinlock.h -- This is a spinlock structure
struct stat;            // stat.h -- File/directory types (statuses)
struct superblock;      // fs.h -- This ia a superblock on disk


//
// -- functions found in bio.c
//    ------------------------
void            binit(void);
struct buf     *bread(uint dev, uint sector);
void            brelse(struct buf *buffer);
void            bwrite(struct buf *buffer);


//
// -- functions found in console.c
//    ----------------------------
void            consoleinit(void);
void            cprintf(char *fmt, ...);
void            consoleintr(int(*func)(void));
void            panic(char *msg) __attribute__((noreturn));


//
// -- functions found in exec.c
//    -------------------------
int             exec(char *path, char **arguments);


//
// -- functions found in file.c
//    -------------------------
struct file    *filealloc(void);
void            fileclose(struct file *fp);
struct file    *filedup(struct file *fp);
void            fileinit(void);
int             fileread(struct file *fp, char *buffer, int n);
int             filestat(struct file *fp, struct stat *status);
int             filewrite(struct file *fp, char *buffer, int n);


//
// -- functions found in fs.c
//    -----------------------
void            readsb(int dev, struct superblock *sb);
int             dirlink(struct inode *inode, char *name, uint inum);
struct inode   *dirlookup(struct inode *inode, char *name, uint *poffset);
struct inode   *ialloc(uint dev, short type);
struct inode   *idup(struct inode *inode);
void            iinit(void);
void            ilock(struct inode *inode);
void            iput(struct inode *inode);
void            iunlock(struct inode *inode);
void            iunlockput(struct inode *inode);
void            iupdate(struct inode *inode);
int             namecmp(const char *source, const char *target);
struct inode   *namei(char *path);
struct inode   *nameiparent(char *path, char *parent);
int             readi(struct inode *inode, char *destination, uint offset, uint num);
void            stati(struct inode *inode, struct stat *status);
int             writei(struct inode *inode, char *source, uint offset, uint num);


//
// -- functions found in ide.c
//    ------------------------
void            ideinit(void);
void            ideintr(void);
void            iderw(struct buf *buffer);


//
// -- functions found in ioapic.c
//    ---------------------------
void            ioapicenable(int irq, int cpu);
extern uchar    ioapicid;
void            ioapicinit(void);


//
// -- functions found in kalloc.c
//    ---------------------------
char           *kalloc(void);
void            kfree(char *memblock);
void            kinit1(void *vfrom, void *vto);
void            kinit2(void *vfrom, void *vto);


//
// -- functions found in kbd.c
//    ------------------------
void            kbdintr(void);


//
// -- functions found in lapic.c
//    --------------------------
int             cpunum(void);
extern volatile uint *lapic;
void            lapiceoi(void);
void            lapicinit(int apicnum);
void            lapicstartap(uchar apicid, uint addr);
void            microdelay(int usec);


//
// -- functions found in log.c
//    ------------------------
void            initlog(void);
void            log_write(struct buf *buffer);
void            begin_trans(void);
void            commit_trans(void);


//
// -- functions found in mp.c
//    -----------------------
extern int      ismp;
int             mpbcpu(void);
void            mpinit(void);
void            mpstartthem(void);


//
// -- functions found in picirq.c
//    ---------------------------
void            picenable(int);
void            picinit(void);


//
// -- functions found in pipe.c
//    -------------------------
int             pipealloc(struct file **file0, struct file **file1);
void            pipeclose(struct pipe *pipe, int writable);
int             piperead(struct pipe *pipe, char *addr, int n);
int             pipewrite(struct pipe *pipe, char *addr, int n);


//
// -- functions found in proc.c
//    -------------------------
struct proc    *copyproc(struct proc *process);
void            exit(void);
int             fork(void);
int             growproc(int n);
int             kill(int pid);
void            pinit(void);
void            procdump(void);
void            scheduler(void) __attribute__((noreturn));
void            sched(void);
void            sleep(void *chan, struct spinlock *lock);
void            userinit(void);
int             wait(void);
void            wakeup(void *chan);
void            yield(void);
void            sharedinit(void);
struct shared  *sharedalloc(void);


//
// -- functions found in swtch.S
//    --------------------------
void            swtch(struct context **oldctx, struct context *newctx);


//
// -- functions found in spinlock.c
//    -----------------------------
void            acquire(struct spinlock *lock);
void            getcallerpcs(void *stackptr, uint *callstack);
int             holding(struct spinlock *lock);
void            initlock(struct spinlock *lock, char *name);
void            release(struct spinlock *lock);
void            pushcli(void);
void            popcli(void);


//
// -- functions found in string.c
//    ---------------------------
int             memcmp(const void *lhs, const void *rhs, uint n);
void           *memmove(void *dest, const void *src, uint n);
void           *memset(void *dest, int val, uint n);
char           *safestrcpy(char *dest, const char *src, int n);
int             strlen(const char *str);
int             strncmp(const char *lhs, const char *rhs, uint n);
char           *strncpy(char *dest, const char *src, int n);


//
// -- functions found in syscall.c
//    ----------------------------
int             argint(int n, int *intptr);
int             argptr(int n, char **ptrptr, int sz);
int             argstr(int, char **strptr);
int             fetchint(struct proc *process, uint addr, int *intptr);
int             fetchstr(struct proc *process, uint addr, char **ptrptr);
void            syscall(void);


//
// -- functions found in timer.c
//    --------------------------
void            timerinit(void);


//
// -- functions found in trap.c
//    -------------------------
void            idtinit(void);
extern uint     ticks;
void            tvinit(void);
extern struct spinlock tickslock;


//
// -- functions found in uart.c
//    -------------------------
void            uartinit(void);
void            uartintr(void);
void            uartputc(int ch);


//
// -- functions found in vm.c
//    -----------------------
void            seginit(void);
void            kvmalloc(void);
void            vmenable(void);
pde_t          *setupkvm(void);
char           *uva2ka(pde_t *pgdir, char *uva);
int             allocuvm(pde_t *pdgir, uint oldsz, uint newsz);
int             deallocuvm(pde_t *pgdir, uint oldsz, uint newsz);
void            freevm(pde_t *pgdir);
void            inituvm(pde_t *pgdir, char *init, uint sz);
int             loaduvm(pde_t *pgdir, char *addr, struct inode *inode, uint offset, uint sz);
pde_t          *copyuvm(pde_t *pgdir, uint sz);
void            switchuvm(struct proc *process);
void            switchkvm(void);
int             copyout(pde_t *pgdir, uint va, void *p, uint len);
void            clearpteu(pde_t *pgdir, char *uva);


// number of elements in fixed-size array
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))


//
// -- functions found in part.c
//    -------------------------
bool validpart(uint partnum);
int partdev(uint partnum);
int partoffset(uint partnum, uint sectnum);
int pread(uint partnum, uint block, void *dest, uint sz);
int pwrite(uint partnum, uint block, void *dest, uint sz);

