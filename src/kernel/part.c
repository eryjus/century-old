//===================================================================================================================
//
// part.c -- This file contains the partition table management functions and structures
//
// The partition management functions will take care of calculating the offsets from the start of any particular
// parition on the disk.  Therefore reading a partition block will read its associated block(s) from disk and
// return the value to the requester.
//
//      CenturyOS is a Hobby Operating System written mostly in C.
//          Copyright (c) 2014-2015  Adam Scott Clark
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
//  2015-10-06   v0.3      #64    ADCL  Develop the ability to read a partition table and identify the partitions on
//                                      the disk.  Partition table structures will be added to the kernel.
//  2015-10-21   v0.3      #89    ADCL  Create several new partition interface functions: validpart(), partdev()
//
//===================================================================================================================


#include "types.h"
#include "defs.h"
#include "buf.h"
#include "param.h"
#include "ext2.h"


//-------------------------------------------------------------------------------------------------------------------
// This structure is a partition entry in a master boot record.
//-------------------------------------------------------------------------------------------------------------------
struct partEntry {
    unsigned char bootFlag;
    unsigned char startHead;
    uint startSector:6;
    uint startCylinder_8_9:2;
    unsigned char startCylinder_0_7;
    unsigned char systemID;
    unsigned char endHead;
    uint endSector:6;
    uint endCylinder_8_9:2;
    unsigned char endCylinder_0_7;
    unsigned long relativeStart;
    unsigned long totalSectors;
};


//-------------------------------------------------------------------------------------------------------------------
// This unnamed type and array are the internal representation of a partition we can process.
//-------------------------------------------------------------------------------------------------------------------
struct partition {
    uint dev;
    uint startSector;
    uint endSector;
    uint valid;
    uint blocksize;
} parts[MAXPARTS];


//-------------------------------------------------------------------------------------------------------------------
// partinit() -- initialize the partition structures
//-------------------------------------------------------------------------------------------------------------------
void partinit(void)
{
    extern struct buf *breadpolling(uint dev, uint sector);
    struct buf *b = breadpolling(BOOTDEV, 0);
    struct partEntry *pe = (struct partEntry *)(&b->data[446]);
    struct ext2sb *sb;
    int i, x;

    for (i = x = 0; i < 4; i ++) {
        if (pe[i].systemID != 0x83) continue;

        parts[x].valid = 1;
        parts[x].dev = BOOTDEV;
        parts[x].startSector = pe[i].relativeStart;
        parts[x].endSector = pe[i].totalSectors + pe[i].relativeStart;
    }

    brelse(b);

    for (i = 0; i < MAXPARTS; i ++) {
        if (!parts[i].valid) continue;

        b = breadpolling(BOOTDEV, 2 + parts[i].startSector);
        sb = (struct ext2sb *)b->data;
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!    Be extra careful here, we now have a 1024-byte structure    !!!
        // !!!    mapped to a 512-byte buffer!  The data is located in the    !!!
        // !!!    first 512 bytes so there really shouldn't be a problem.     !!!
        // !!!                JUST DON'T GET AMBITIOUS!!!                     !!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        if (sb->s_magic != EXT2_SUPER_MAGIC) {
            cprintf("Disk %d, Partition %d: Invalid EXT2 Superblock signature\n", parts[i].dev, i);
            parts[i].valid = 0;
            brelse(b);
            continue;
        }

        parts[i].blocksize = 1024 << sb->s_log_block_size;

        cprintf("Disk %d, Partition %d: ext2 (%s)\n", parts[i].dev, i,
                (char *[]){"clean", "dirty"}[sb->s_state - 1]);
        cprintf("  Free blocks: %d/%d\n", sb->s_free_blocks_count, sb->s_blocks_count);
        cprintf("  Free inodes: %d/%d\n", sb->s_free_inodes_count, sb->s_inodes_count);

        //
        // -- Check the first backup superblock for consistency
        //    -------------------------------------------------
        cprintf("Getting backup superblock at block %d\n", sb->s_blocks_per_group + 1);
        struct buf *bb = breadpolling(parts[i].dev, parts[i].startSector + (sb->s_blocks_per_group + 1) * parts[i].blocksize / 512);
        struct ext2sb *bsb = (struct ext2sb *)bb->data;

        if (bsb->s_magic != EXT2_SUPER_MAGIC) cprintf("Error: ext2 backup superblock bad magic number\n");
        if (memcmp(sb, bsb, 512) != 0) cprintf("Error: ext2 backup superblock not in sync\n");

        //
        // -- release only after all references to the data are complete
        //    ----------------------------------------------------------
        brelse(b);
        brelse(bb);
    }

    cprintf("\n");
}


//-------------------------------------------------------------------------------------------------------------------
// partoffset() -- get the partition starting offset
//-------------------------------------------------------------------------------------------------------------------
int partoffset(uint partnum, uint sectnum)
{
    return sectnum + (validpart(partnum)?parts[partnum].startSector:0);
}


//-------------------------------------------------------------------------------------------------------------------
// partdev() -- get the partition device number
//-------------------------------------------------------------------------------------------------------------------
int partdev(uint partnum)
{
    return (validpart(partnum)?parts[partnum].dev:-1);
}


//-------------------------------------------------------------------------------------------------------------------
// validpart() -- is the partition number a valid partition?
//-------------------------------------------------------------------------------------------------------------------
bool validpart(uint partnum)
{
    if (partnum < 0 || partnum >= MAXPARTS || parts[partnum].valid == false) return false;
    else return true;
}


//-------------------------------------------------------------------------------------------------------------------
// pread() -- Return the contents of the indicated partition block
//-------------------------------------------------------------------------------------------------------------------
int pread(uint partnum, uint block, void *dest, uint sz)
{
    if (partnum >= MAXPARTS || !parts[partnum].valid) panic("pread");
    if (parts[partnum].blocksize > sz) return -1;

    int num = parts[partnum].blocksize / 512;
    uint sector;

    for (sector = block * num + parts[partnum].startSector; num > 0; num --, sector ++) {
        if (sector >= parts[partnum].endSector) panic("pread: bad sector number");
        struct buf *b = bread(parts[partnum].dev, sector);
        memmove(dest, b->data, 512);
        dest += 512;
        brelse(b);
    }

    return 0;
}


//-------------------------------------------------------------------------------------------------------------------
// pwrite() -- Write the contents of the indicated partition block
//-------------------------------------------------------------------------------------------------------------------
int pwrite(uint partnum, uint block, void *dest, uint sz)
{
    if (partnum >= MAXPARTS || !parts[partnum].valid) panic("pwrite");
    if (parts[partnum].blocksize > sz) return -1;

    int num = parts[partnum].blocksize / 512;
    uint sector;

    for (sector = block * num + parts[partnum].startSector; num > 0; num --, sector ++) {
        if (sector >= parts[partnum].endSector) panic("pwrite: bad sector number");
        struct buf *b = bread(parts[partnum].dev, sector);
        memmove(b->data, dest, 512);
        begin_trans();
        log_write(b);
        commit_trans();
        dest += 512;
        brelse(b);
    }

    return 0;
}
