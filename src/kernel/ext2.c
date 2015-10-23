//===================================================================================================================
//
// ext2.c -- This file contains the implementation of the ext2 file system.
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
//  2015-10-21   v0.3      #89    ADCL  Initial implementation.  This file starts with an implementation to read the
//                                      EXT2 superblock.
//
//===================================================================================================================


#include "types.h"
#include "buf.h"
#include "defs.h"
#include "ext2.h"


//-------------------------------------------------------------------------------------------------------------------
// ext2readsb() -- read an ext2 superblock at sector #2
//-------------------------------------------------------------------------------------------------------------------
void ext2readsb(uint part, struct ext2sb *sb)
{
    struct buf *b;
    uint dev;

    if (!validpart(part)) panic("ext2readsb: invalid partition");

    dev = partdev(part);
    b = bread(dev, partoffset(part, 2));
    memmove(sb, b->data, 512);
    brelse(b);

    b = bread(dev, partoffset(part, 3));
    memmove(((char *)sb) + 512, b->data, 512);
    brelse(b);
}


//-------------------------------------------------------------------------------------------------------------------
// sys_ext2readsb() -- the system call entry point to read an EXT2 superblock
//-------------------------------------------------------------------------------------------------------------------
int sys_ext2readsb(void)
{
    uint part;
    char *sb;

    if (argint(0, (int *)&part) < 0 || argptr(1, &sb, sizeof(struct ext2sb))) return -1;

    ext2readsb(part, (void *)sb);

    return 0;
}
