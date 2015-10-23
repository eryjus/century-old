//===================================================================================================================
//
// ext2.h -- This file contains the structures for maintaining an ext2 file system
//
// The ext2 file system is the first supported file system for CenturyOS.  The structures in this file are
// taken directly from the published document: _The_Second_Extended_File_System_, _Internal_Layout_ by David
// Poirier. (c) 2001-2011.
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
//  2015-10-06   v0.3      #70    ADCL  Initial version -- need to be able to read an ext2 superblock
//
//===================================================================================================================


//
// -- This structure defines the EXT2 superblock.  This block is located at exaxtly 1024 bytes from the start of
//    the disk.  In some cases (where block size == 1024), this will be block #1 (block #0 has the boot record).  In
//    other cases, this will be block #0 and will be located not at the start of the block, but 1024 bytes from the
//    start.
//    ---------------------------------------------------------------------------------------------------------------
struct ext2sb {
    uint s_inodes_count;                // total number of inodes, used and unused
    uint s_blocks_count;                // total number of blocks, used, unused, and reserved
    uint s_r_blocks_count;              // reserved block count
    uint s_free_blocks_count;           // free block count
    uint s_free_inodes_count;           // free inodes count
    uint s_first_data_block;            // block location of the superblock (0 for block size > 1K
    uint s_log_block_size;              // block size: 1024 << s_log_block_size
    uint s_log_frag_size;               // fragment size: 1024 << s_log_frag_size
    uint s_blocks_per_group;            // total number of blocks per group
    uint s_frags_per_group;             // total number of fragments per group
    uint s_inodes_per_group;            // total number of inodes per group
    uint s_mtime;                       // posix time when the system was last mounted
    uint s_wtime;                       // posix time when the system was last written
    ushort s_mnt_count;                 // number of mounts since last full verification
    ushort s_max_mnt_count;             // the max numebr of mounts before a full verification is required
    ushort s_magic;                     // MUST be 0xef53
    ushort s_state;                     // system state
    ushort s_errors;                    // what should the OS do when an error is detected
    ushort s_minor_rev_level;           // the minor revision level
    uint s_lastcheck;                   // posix time of the last file system check
    uint s_checkinterval;               // posix time interval allowed between file system checks
    uint s_creator_os;                  // creator OS
    uint s_rev_level;                   // revision level
    ushort s_def_resuid;                // default user id for reserved blocks (linux uses 0)
    ushort s_def_resgid;                // default group id for reserved blocks (linux uses 0)

//extended fields
    uint s_first_ino;
    ushort s_inode_size;
    ushort s_block_group_nr;
    uint s_feature_compat;
    uint s_feature_incompat;
    uint s_feature_ro_compat;
    char s_uuid[16];
    char s_volume_name[16];
    char s_last_mounted[64];
    uint s_algo_bitmap;
    uchar s_prealloc_blocks;
    uchar s_prealloc_dir_blocks;
    ushort __alignment;
    char s_journal_uuid[16];
    uint s_journal_inum;
    uint s_journal_dev;
    uint s_last_orphan;
    uint s_hash_seed[4];
    uchar s_def_hash_version;
    uchar __padding[3];
    uint s_default_mount_options;
    uint s_first_meta_bg;
    char __unused[760];
};


//
// -- The following are constants that are used in the EXT2 superblock
//    ----------------------------------------------------------------
enum {
    EXT2_SUPER_MAGIC            = 0xef53,
};


enum {
    EXT2_VALID_FS               = 1,                    // unmounted cleanly
    EXT2_ERROR_FS               = 2,                    // errors detected
};


enum {
    EXT2_ERRORS_CONTINUE        = 1,                    // continue as if nothing happened
    EXT2_ERRORS_RO              = 2,                    // remount as read-only mode
    EXT2_ERRORS_PANIC           = 3,                    // panic the kernel
};


enum {
    EXT2_OS_LINUX               = 0,                    // linux
    EXT2_OS_HURD                = 1,                    // GNU HURD
    EXT2_OS_MASIX               = 2,                    // MASIX
    EXT2_OS_FREEBSD             = 3,                    // FreeBSD
    EXT2_OS_LITES               = 4,                    // Lites
    EXT2_OS_CENTURY             = 100,                  // We swooped this for CenturyOS
};


enum {
    EXT2_GOOD_OLD_REV           = 0,                    // Revision 0
    EXT2_DYNAMIC_REV            = 1,                    // Revision 1 with variable idode sizes, etc.
};


//
// -- Rev 0 constants
//    ---------------
enum {
    EXT2_GOOD_OLD_FIRST_INO     = 11,                   // rev 0 fixed first inode number
    EXT2_GOOD_OLD_INODE_SIZE    = 128,                  // rev 0 fixed inode size
};
