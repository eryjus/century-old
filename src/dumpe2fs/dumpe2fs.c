//===================================================================================================================
//
// dumpe2fs.c -- This source file contains the user program to dump the critical structures of an EXT2 file system.
//
// This program runs in user-space.  It will read the superblock and other file system structures and report on
// their contents.  What is significant about this program is that it uses a newly-minted system function call
// to read the superblock.
//
// However, this program reads a file system block and not a disk sector for the superblock.  This is technically
// incorrect as the superblock is located at a fixed sector on the disk, and not on a fixed file system block number.
// Redmine #89 has been entered to address this issue.
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
//  2015-10-19   v0.3    #71,87   ADCL  Initial version, and properly document this source.
//
//===================================================================================================================


#include "types.h"
#include "user.h"

#include "ext2.h"


struct ext2sb buf;

//-------------------------------------------------------------------------------------------------------------------
// main() -- this is the main entry point.  It dumps the contents of the ext2 superblock.
//-------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    int rev = 0;

    if (ext2readsb(0, &buf) == -1) {
        printf(1, "Error\n");
        exit();
    }

    if (buf.s_rev_level > 0) rev = 1;
    else {
        buf.s_first_ino = EXT2_GOOD_OLD_FIRST_INO;
        buf.s_inode_size = EXT2_GOOD_OLD_INODE_SIZE;
    }

    printf(1, "Filesystem volume name:   %s\n", (rev||!buf.s_volume_name[0]?"<none>":buf.s_volume_name));
    printf(1, "Last mounted on:          <not implemented>\n");
    printf(1, "Filesystem UUID:          <not implemented>\n");
    printf(1, "Filesystem magic number:  0x%x\n", buf.s_magic);
    printf(1, "Filesystem revision #:    %d (%s)\n", buf.s_rev_level,
            (char *[]){"good old rev 0", "dynamic"}[buf.s_rev_level]);
    printf(1, "Filesystem features:      <not implemented>\n");
    printf(1, "Default mount options:    <not implemented>\n");
    printf(1, "Filesystem state:         %s\n", (char *[]){"clean", "dirty"}[buf.s_state - 1]);
    printf(1, "Errors behavior:          %s\n", (char *[]){"continue",
                                                            "remount ro",
                                                            "panic"}[buf.s_errors - 1]);
    printf(1, "Filesystem OS type:       %s\n",
            buf.s_creator_os==100?"CenturyOS":(char *[]){"Linux",
                                                            "Hurd",
                                                            "Masix",
                                                            "FreeBSD",
                                                            "Lites"}[buf.s_creator_os]);
    printf(1, "Inode count:              %d\n", buf.s_inodes_count);
    printf(1, "Block count:              %d\n", buf.s_blocks_count);
    printf(1, "Reserved block count:     %d\n", buf.s_r_blocks_count);
    printf(1, "Free blocks:              %d\n", buf.s_free_blocks_count);
    printf(1, "Free inodes:              %d\n", buf.s_free_inodes_count);
    printf(1, "First block:              %d\n", buf.s_first_data_block);
    printf(1, "Block size:               %d\n", 1024 << buf.s_log_block_size);
    printf(1, "Fragment size:            %d\n", 1024 << buf.s_log_frag_size);
    printf(1, "Blocks per group:         %d\n", buf.s_blocks_per_group);
    printf(1, "Fragments per group:      %d\n", buf.s_frags_per_group);
    printf(1, "Inodes per group:         %d\n", buf.s_inodes_per_group);
    printf(1, "Filesystem created:       <not implemented>\n");
    printf(1, "Last mount time:          <not implemented>\n");
    printf(1, "Last write time:          <not implemented>\n");
    printf(1, "Mount count:              %d\n", buf.s_mnt_count);
    printf(1, "Maximum mount count:      %d\n", buf.s_max_mnt_count);
    printf(1, "Last checked:             <not implemented>\n");
    printf(1, "Check interval:           %d\n", buf.s_checkinterval);
    printf(1, "Next check after:         <not implemented>\n");
    printf(1, "Reserved blocks uid:      %d\n", buf.s_def_resuid);
    printf(1, "Reserved blocks gid:      %d\n", buf.s_def_resgid);
    printf(1, "First inode:              %d\n", buf.s_first_ino);
    printf(1, "Inode size:               %d\n", buf.s_inode_size);
    printf(1, "Journal inode:            %d\n", buf.s_journal_inum);
    printf(1, "Default directory hash:   <not implemented>\n");
    printf(1, "Directory hash seed:      <not implemented>\n");
    printf(1, "Journal backup:           <not implemented>\n");

    partblockwrite(0, 1, &buf, sizeof(struct ext2sb));

    exit();
}


