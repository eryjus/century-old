//===================================================================================================================
//
// bio.c -- This file contains the buffer cache implementation
//
// The buffer cache is a linked list of buf structures holding cached copies of disk block contents.  Caching disk
// blocks in memory reduces the number of disk reads and also provides a synchronization point for disk blocks used
// by multiple processes.
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
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer, so do not keep them longer than necessary.
//
// The implementation uses three state flags internally:
// * B_BUSY: the block has been returned from bread and has not been passed back to brelse.
// * B_VALID: the buffer data has been read from the disk.
// * B_DIRTY: the buffer data has been modified and needs to be written to disk.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Version  Tracker  Pgmr  Description
//  ----------  -------  -------  ----  -----------------------------------------------------------------------------
//  2015-10-07   v0.3      #72    ADCL  Create a new function breadpolling() that is the same as bread() but instead
//                                      of getting an interrupt on completion, poll the status register for
//                                      completion.  This function would only be used during initialization.
//  2015-10-20   v0.3      #80    ADCL  Document this source file and arrange the code to meet my coding standards.
//
//===================================================================================================================


#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "buf.h"


//-------------------------------------------------------------------------------------------------------------------
// This unnamed structure variable is used to keep track of the buffer cache in total.
//-------------------------------------------------------------------------------------------------------------------
struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // head.next is most recently used.
  // ----------------------------------------------
  struct buf head;
} bcache;



//-------------------------------------------------------------------------------------------------------------------
// binit() -- initialize the buffer cache structure
//-------------------------------------------------------------------------------------------------------------------
void binit(void)
{
    struct buf *b;

    initlock(&bcache.lock, "bcache");

//
// -- Create linked list of buffers
//    -----------------------------
    bcache.head.prev = &bcache.head;
    bcache.head.next = &bcache.head;

    for (b = bcache.buf; b < bcache.buf + NBUF; b ++) {
        b->next = bcache.head.next;
        b->prev = &bcache.head;
        b->dev = -1;
        bcache.head.next->prev = b;
        bcache.head.next = b;
    }
}


//-------------------------------------------------------------------------------------------------------------------
// bget() -- Look through buffer cache for sector on device dev.  If not found, allocate fresh block. In either case,
//           return B_BUSY buffer.
//-------------------------------------------------------------------------------------------------------------------
static struct buf *bget(uint dev, uint sector)
{
    struct buf *b;

    acquire(&bcache.lock);

loop:  // Is the sector already cached?
    for (b = bcache.head.next; b != &bcache.head; b = b->next) {
        if (b->dev == dev && b->sector == sector) {
            if (!(b->flags & B_BUSY)) {
                b->flags |= B_BUSY;
                release(&bcache.lock);

                return b;
            }

            sleep(b, &bcache.lock);

            goto loop;
        }
    }

//
// -- Not cached; recycle some non-busy and clean buffer.
//    ---------------------------------------------------
    for (b = bcache.head.prev; b != &bcache.head; b = b->prev) {
        if ((b->flags & B_BUSY) == 0 && (b->flags & B_DIRTY) == 0) {
            b->dev = dev;
            b->sector = sector;
            b->flags = B_BUSY;
            release(&bcache.lock);

            return b;
        }
    }

    panic("bget: no buffers");
}


//-------------------------------------------------------------------------------------------------------------------
// bread() -- Return a B_BUSY buf with the contents of the indicated disk sector.
//-------------------------------------------------------------------------------------------------------------------
struct buf *bread(uint dev, uint sector)
{
    struct buf *b;

    b = bget(dev, sector);

    if (!(b->flags & B_VALID)) iderw(b);

    return b;
}


//-------------------------------------------------------------------------------------------------------------------
// bwrite() -- Write b's contents to disk.  Must be B_BUSY.
//-------------------------------------------------------------------------------------------------------------------
void bwrite(struct buf *b)
{
    if ((b->flags & B_BUSY) == 0) panic("bwrite");

    b->flags |= B_DIRTY;
    iderw(b);
}


//-------------------------------------------------------------------------------------------------------------------
// brelse() -- Release a B_BUSY buffer.  Move to the head of the MRU list.
//-------------------------------------------------------------------------------------------------------------------
void brelse(struct buf *b)
{
    if((b->flags & B_BUSY) == 0) panic("brelse");

    acquire(&bcache.lock);

    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    bcache.head.next->prev = b;
    bcache.head.next = b;

    b->flags &= ~B_BUSY;
    wakeup(b);

    release(&bcache.lock);
}


//-------------------------------------------------------------------------------------------------------------------
// breadpolling() -- Return a B_BUSY buf with the contents of the indicated disk sector.  Use polling instead of
//                   interrupts.
//-------------------------------------------------------------------------------------------------------------------
struct buf *breadpolling(uint dev, uint sector)
{
    extern void iderwpoll(struct buf *b);
    struct buf *b;

    b = bget(dev, sector);

    if (!(b->flags & B_VALID)) iderwpoll(b);

    return b;
}
