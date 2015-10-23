//===================================================================================================================
//
// ide.c -- This file contains a simple PIO-based (non-DMA) IDE driver code.
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
//  2015-10-07   v0.3      #72    ADCL  Create a new function iderwpoll() that is the same as iderw() but instead
//                                      of getting an interrupt on completion, poll the status register for
//                                      completion.  This function would only be used during initialization.
//  2015-10-20   v0.3      #81    ADCL  Document this file and format it in accordance with my personal standards.
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
#include "traps.h"
#include "spinlock.h"
#include "buf.h"


//
// -- IDE status flags
//    ----------------
#define IDE_BSY       0x80
#define IDE_DRDY      0x40
#define IDE_DF        0x20
#define IDE_ERR       0x01


//
// -- IDE command values
//    ------------------
#define IDE_CMD_READ  0x20
#define IDE_CMD_WRITE 0x30


//
// -- idequeue points to the buf now being read/written to the disk.  idequeue->qnext points to the next buf to be
//    processed.  You must hold idelock while manipulating queue.
//    ------------------------------------------------------------------------------------------------------------
static struct spinlock idelock;
static struct buf *idequeue;
static int havedisk1;
static void idestart(struct buf *);


//-------------------------------------------------------------------------------------------------------------------
// idewait() -- Wait for IDE disk to become ready.
//-------------------------------------------------------------------------------------------------------------------
static int idewait(int checkerr)
{
    int r;

    while (((r = inb(0x1f7)) & (IDE_BSY | IDE_DRDY)) != IDE_DRDY) {}
    if (checkerr && (r & (IDE_DF | IDE_ERR)) != 0) return -1;

    return 0;
}


//-------------------------------------------------------------------------------------------------------------------
// ideinit() -- initialize the ide driver
//-------------------------------------------------------------------------------------------------------------------
void ideinit(void)
{
    int i;

    initlock(&idelock, "ide");
    picenable(IRQ_IDE);
    ioapicenable(IRQ_IDE, ncpu - 1);
    idewait(0);

    //
    // -- Check if disk 1 is present
    //    --------------------------
    outb(0x1f6, 0xe0 | (1<<4));

    for (i = 0; i < 1000; i ++) {
        if (inb(0x1f7) != 0) {
            havedisk1 = 1;
            break;
        }
    }

    //
    // -- Switch back to disk 0.
    //    ----------------------
    outb(0x1f6, 0xe0 | (0<<4));
}


//-------------------------------------------------------------------------------------------------------------------
// idestart() -- Start the request for b.  Caller must hold idelock.
//-------------------------------------------------------------------------------------------------------------------
static void idestart(struct buf *b)
{
    if (b == 0) panic("idestart");

    idewait(0);
    outb(0x3f6, 0);  // generate interrupt
    outb(0x1f2, 1);  // number of sectors
    outb(0x1f3, b->sector & 0xff);
    outb(0x1f4, (b->sector >> 8) & 0xff);
    outb(0x1f5, (b->sector >> 16) & 0xff);
    outb(0x1f6, 0xe0 | ((b->dev & 1) << 4) | ((b->sector >> 24) & 0x0f));

    if (b->flags & B_DIRTY) {
        outb(0x1f7, IDE_CMD_WRITE);
        outsl(0x1f0, b->data, 512 / 4);
    } else {
        outb(0x1f7, IDE_CMD_READ);
    }
}


//-------------------------------------------------------------------------------------------------------------------
// ideintr() -- Interrupt handler.
//-------------------------------------------------------------------------------------------------------------------
void ideintr(void)
{
    struct buf *b;

    //
    // -- First queued buffer is the active request.
    //    ------------------------------------------
    acquire(&idelock);

    if ((b = idequeue) == 0) {
        release(&idelock);

        // cprintf("spurious IDE interrupt\n");
        return;
    }

    idequeue = b->qnext;

    //
    // -- Read data if needed.
    //    --------------------
    if (!(b->flags & B_DIRTY) && idewait(1) >= 0) insl(0x1f0, b->data, 512/4);


    //
    // -- Wake process waiting for this buf.
    //    ----------------------------------
    b->flags |= B_VALID;
    b->flags &= ~B_DIRTY;
    wakeup(b);

    //
    // -- Start disk on next buf in queue.
    //    --------------------------------
    if(idequeue != 0) idestart(idequeue);

    release(&idelock);
}


//-------------------------------------------------------------------------------------------------------------------
// iderw() -- Sync buf with disk.  If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.  Else if
//            B_VALID is not set, read buf from disk, set B_VALID.
//-------------------------------------------------------------------------------------------------------------------
void iderw(struct buf *b)
{
    struct buf **pp;

    if (!(b->flags & B_BUSY)) panic("iderw: buf not busy");
    if ((b->flags & (B_VALID|B_DIRTY)) == B_VALID) panic("iderw: nothing to do");
    if (b->dev != 0 && !havedisk1) panic("iderw: ide disk 1 not present");

    acquire(&idelock);

    //
    // -- Append b to idequeue.
    //    ---------------------
    b->qnext = 0;

    for (pp = &idequeue; *pp; pp = &(*pp)->qnext) {}
    *pp = b;

    //
    // -- Start disk if necessary.
    //    ------------------------
    if (idequeue == b) idestart(b);

    //
    // -- Wait for request to finish.
    //    ---------------------------
    while ((b->flags & (B_VALID|B_DIRTY)) != B_VALID) sleep(b, &idelock);

    release(&idelock);
}


//-------------------------------------------------------------------------------------------------------------------
// iderwpolling() -- Read a new buffer from disk.  We will not support updates from this function.  As a result,
//                   locks are not needed.
//-------------------------------------------------------------------------------------------------------------------
void iderwpoll(struct buf *b)
{
    if(!(b->flags & B_BUSY)) panic("iderw: buf not busy");

    outb(0x1f6, 0xe0 | (1 << 4));
    idewait(0);
    outb(0x3f6, 1 << 1);  // do not generate interrupt
    outb(0x1f2, 1);  // number of sectors
    outb(0x1f3, b->sector & 0xff);
    outb(0x1f4, (b->sector >> 8) & 0xff);
    outb(0x1f5, (b->sector >> 16) & 0xff);
    outb(0x1f6, 0xe0 | ((b->dev & 1) << 4) | ((b->sector >> 24) & 0x0f));

    outb(0x1f7, IDE_CMD_READ);

    //
    // -- Wait for request to finish.
    //    ---------------------------
    if (idewait(1) == -1) return;

    insl(0x1f0, b->data, 512 / 4);
    b->flags |= B_VALID;

    outb(0x1f6, 0xe0 | (0 << 4));
}


//-------------------------------------------------------------------------------------------------------------------
// idereadsect() -- read a raw sector number from a disk device
//-------------------------------------------------------------------------------------------------------------------
int idereadsect(uint dev, uint sect, char *buffer)
{
    struct buf *b = bread(dev, sect);
    memmove(buffer, b->data, 512);
    brelse(b);

    return 0;
}
