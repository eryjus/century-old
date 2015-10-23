//===================================================================================================================
//
// fdisk.c -- This source file contains the user program to display a disk's partition table.
//
// This program runs in user-space.  It will read the master boot record (MBR) from disk 0 and dump the partition
// table information on screen.
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
//  2015-10-22   v0.3      #68    ADCL  Initial version of fdisk
//
//===================================================================================================================


#include "types.h"
#include "user.h"


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
// main() -- this is the main entry point.  It dumps the contents of the partition table.
//-------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    char mbr[512];
    struct partEntry *pe = (struct partEntry *)&mbr[446];
    int i;

    if (readsect(0, 0, mbr) == -1) {
        printf(1, "Unable to read the disk\n");
        exit();
    }

    for (i = 0; i < 4; i ++) {
        printf(1, "Partition %d: %s\n", i, pe[i].bootFlag==0x80?"(Bootable)":"");

        switch (pe[i].systemID) {
        case 0x00:
            printf(1, "  Unused\n");
            continue;

        case 0x01:
            printf(1, "  DOS 12-bit FAT\n");
            break;

        case 0x04:
            printf(1, "  DOS 16-bit FAT (<=32MB)\n");
            break;

        case 0x05:
            printf(1, "  DOS Extended Partition\n");
            break;

        case 0x06:
            printf(1, "  DOS 16-bit FAT (>32MB)\n");
            break;

        case 0x07:
            printf(1, "  OS/2 HPFS\n");
            break;

        case 0x08:
            printf(1, "  Windows NTFS\n");
            break;

        case 0x82:
            printf(1, "  Linux Swap\n");
            break;

        case 0x83:
            printf(1, "  Linux EXT2\n");
            break;

        default:
            printf(1, "  Unknown type %x\n", pe[i].systemID);
            break;
        }

        printf(1, "  LBA Start %d, for %d sectors\n", pe[i].relativeStart, pe[i].totalSectors);
        printf(1, "  Starting Cyl/Head/Sect: %d/%d/%d\n", (pe[i].startCylinder_8_9 << 8) + pe[i].startCylinder_0_7,
                pe[i].startHead, pe[i].startSector);
        printf(1, "  Ending Cyl/Head/Sect: %d/%d/%d\n", (pe[i].endCylinder_8_9 << 8) + pe[i].endCylinder_0_7,
                pe[i].endHead, pe[i].endSector);
    }

    exit();
}
