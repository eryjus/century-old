//===================================================================================================================
//
// sysfile.c -- This file contains the functions necessary to process a system call into the file system.
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
// File-system system calls.  Mostly argument checking, since we don't trust user code, and makes calls into file.c
// and fs.c.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Version  Tracker  Pgmr  Description
//  ----------  -------  -------  ----  -----------------------------------------------------------------------------
//  2015-10-01   v0.3      #64    ADCL  Add partition block related system calls
//  2015-10-21   v0.3      #84    ADCL  Document this file and format it in accordance with my personal standards.
//  2015-10-22   v0.3      #75    ADCL  Publish a feature to read a raw disk sector
//
//===================================================================================================================


#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "fs.h"
#include "file.h"
#include "fcntl.h"
#include "buf.h"


//-------------------------------------------------------------------------------------------------------------------
// argfd() -- Fetch the nth word-sized system call argument as a file descriptor and return both the descriptor and
//            the corresponding struct file.
//-------------------------------------------------------------------------------------------------------------------
static int argfd(int n, int *pfd, struct file **pf)
{
    int fd;
    struct file *f;

    if (argint(n, &fd) < 0) return -1;
    if (fd < 0 || fd >= NOFILE || (f = proc->ofile[fd]) == 0) return -1;
    if (pfd) *pfd = fd;
    if (pf) *pf = f;

    return 0;
}


//-------------------------------------------------------------------------------------------------------------------
// fdalloc() -- Allocate a file descriptor for the given file.  Takes over file reference from caller on success.
//-------------------------------------------------------------------------------------------------------------------
static int fdalloc(struct file *f)
{
    int fd;

    for (fd = 0; fd < NOFILE; fd ++) {
        if (proc->ofile[fd] == 0) {
            proc->ofile[fd] = f;

            return fd;
        }
    }

    return -1;
}


//-------------------------------------------------------------------------------------------------------------------
// sys_dup() -- duplicate a file
//-------------------------------------------------------------------------------------------------------------------
int sys_dup(void)
{
    struct file *f;
    int fd;

    if (argfd(0, 0, &f) < 0) return -1;
    if ((fd=fdalloc(f)) < 0) return -1;

    filedup(f);

    return fd;
}


//-------------------------------------------------------------------------------------------------------------------
// sys_read() -- read a file
//-------------------------------------------------------------------------------------------------------------------
int sys_read(void)
{
    struct file *f;
    int n;
    char *p;

    if (argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0) return -1;

    return fileread(f, p, n);
}


//-------------------------------------------------------------------------------------------------------------------
// sys_write() -- write to a file
//-------------------------------------------------------------------------------------------------------------------
int sys_write(void)
{
    struct file *f;
    int n;
    char *p;

    if (argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0) return -1;

    return filewrite(f, p, n);
}


//-------------------------------------------------------------------------------------------------------------------
// sys_close() -- close a file
//-------------------------------------------------------------------------------------------------------------------
int sys_close(void)
{
    int fd;
    struct file *f;

    if (argfd(0, &fd, &f) < 0) return -1;

    proc->ofile[fd] = 0;
    fileclose(f);

    return 0;
}


//-------------------------------------------------------------------------------------------------------------------
// sys_fstat() -- get file attributes
//-------------------------------------------------------------------------------------------------------------------
int sys_fstat(void)
{
    struct file *f;
    struct stat *st;

    if (argfd(0, 0, &f) < 0 || argptr(1, (void*)&st, sizeof(*st)) < 0) return -1;

    return filestat(f, st);
}


//-------------------------------------------------------------------------------------------------------------------
// sys_link() -- Create the path new as a link to the same inode as old.
//-------------------------------------------------------------------------------------------------------------------
int sys_link(void)
{
    char name[DIRSIZ], *new, *old;
    struct inode *dp, *ip;

    if (argstr(0, &old) < 0 || argstr(1, &new) < 0) return -1;
    if ((ip = namei(old)) == 0) return -1;

    begin_trans();
    ilock(ip);

    if (ip->type == T_DIR) {
        iunlockput(ip);
        commit_trans();
        return -1;
    }

    ip->nlink ++;
    iupdate(ip);
    iunlock(ip);

    if ((dp = nameiparent(new, name)) == 0) goto bad;

    ilock(dp);

    if (dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0) {
        iunlockput(dp);
        goto bad;
    }

    iunlockput(dp);
    iput(ip);

    commit_trans();

    return 0;

bad:
    ilock(ip);
    ip->nlink --;
    iupdate(ip);
    iunlockput(ip);
    commit_trans();

    return -1;
}


//-------------------------------------------------------------------------------------------------------------------
// isdirempty() -- Is the directory dp empty except for "." and ".." ?
//-------------------------------------------------------------------------------------------------------------------
static int isdirempty(struct inode *dp)
{
    int off;
    struct dirent de;

    for (off = 2 * sizeof(de); off < dp->size; off += sizeof(de)) {
        if (readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) panic("isdirempty: readi");
        if(de.inum != 0) return 0;
    }

    return 1;
}


//-------------------------------------------------------------------------------------------------------------------
// sys_unlink() -- Remove a file from the file system
//-------------------------------------------------------------------------------------------------------------------
int sys_unlink(void)
{
    struct inode *ip, *dp;
    struct dirent de;
    char name[DIRSIZ], *path;
    uint off;

    if (argstr(0, &path) < 0) return -1;
    if ((dp = nameiparent(path, name)) == 0) return -1;

    begin_trans();
    ilock(dp);

    //
    // -- Cannot unlink "." or "..".
    //    --------------------------
    if (namecmp(name, ".") == 0 || namecmp(name, "..") == 0) goto bad;
    if ((ip = dirlookup(dp, name, &off)) == 0) goto bad;

    ilock(ip);

    if (ip->nlink < 1) panic("unlink: nlink < 1");
    if (ip->type == T_DIR && !isdirempty(ip)) {
        iunlockput(ip);
        goto bad;
    }

    memset(&de, 0, sizeof(de));

    if (writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) panic("unlink: writei");
    if (ip->type == T_DIR) {
        dp->nlink --;
        iupdate(dp);
    }

    iunlockput(dp);

    ip->nlink --;
    iupdate(ip);
    iunlockput(ip);
    commit_trans();

    return 0;

bad:
    iunlockput(dp);
    commit_trans();

    return -1;
}


//-------------------------------------------------------------------------------------------------------------------
// create() -- Create a new file/dir
//-------------------------------------------------------------------------------------------------------------------
static struct inode *create(char *path, short type, short major, short minor)
{
    uint off;
    struct inode *ip, *dp;
    char name[DIRSIZ];

    if ((dp = nameiparent(path, name)) == 0) return 0;

    ilock(dp);

    if ((ip = dirlookup(dp, name, &off)) != 0) {
        iunlockput(dp);
        ilock(ip);

        if (type == T_FILE && ip->type == T_FILE) return ip;

        iunlockput(ip);

        return 0;
    }

    if ((ip = ialloc(dp->dev, type)) == 0) panic("create: ialloc");

    ilock(ip);
    ip->major = major;
    ip->minor = minor;
    ip->nlink = 1;
    iupdate(ip);

    if (type == T_DIR) {    // Create . and .. entries.
        dp->nlink ++;       // for ".."
        iupdate(dp);

        //
        // -- No ip->nlink++ for ".": avoid cyclic ref count.
        //    -----------------------------------------------
        if (dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0) panic("create dots");
    }

    if (dirlink(dp, name, ip->inum) < 0) panic("create: dirlink");

    iunlockput(dp);

    return ip;
}


//-------------------------------------------------------------------------------------------------------------------
// sys_open() -- open a file
//-------------------------------------------------------------------------------------------------------------------
int sys_open(void)
{
    char *path;
    int fd, omode;
    struct file *f;
    struct inode *ip;

    if (argstr(0, &path) < 0 || argint(1, &omode) < 0) return -1;
    if (omode & O_CREATE) {
        begin_trans();
        ip = create(path, T_FILE, 0, 0);
        commit_trans();

        if (ip == 0) return -1;
    } else {
        if ((ip = namei(path)) == 0) return -1;

        ilock(ip);

        if (ip->type == T_DIR && omode != O_RDONLY) {
            iunlockput(ip);
            return -1;
        }
    }

    if ((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0) {
        if(f) fileclose(f);

        iunlockput(ip);

        return -1;
    }

    iunlock(ip);

    f->type = FD_INODE;
    f->ip = ip;
    f->off = 0;
    f->readable = !(omode & O_WRONLY);
    f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

    return fd;
}


//-------------------------------------------------------------------------------------------------------------------
// sys_mkdir() -- make a directory
//-------------------------------------------------------------------------------------------------------------------
int sys_mkdir(void)
{
    char *path;
    struct inode *ip;

    begin_trans();

    if (argstr(0, &path) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0) {
        commit_trans();

        return -1;
    }

    iunlockput(ip);
    commit_trans();

    return 0;
}


//-------------------------------------------------------------------------------------------------------------------
// sys_mknod() -- make a node (dev, pipe, etc.)
//-------------------------------------------------------------------------------------------------------------------
int sys_mknod(void)
{
    struct inode *ip;
    char *path;
    int len;
    int major, minor;

    begin_trans();

    if ((len=argstr(0, &path)) < 0 || argint(1, &major) < 0 || argint(2, &minor) < 0 ||
            (ip = create(path, T_DEV, major, minor)) == 0) {
        commit_trans();

        return -1;
    }

    iunlockput(ip);
    commit_trans();

    return 0;
}


//-------------------------------------------------------------------------------------------------------------------
// sys_chdir() -- change directory
//-------------------------------------------------------------------------------------------------------------------
int sys_chdir(void)
{
    char *path;
    struct inode *ip;

    if (argstr(0, &path) < 0 || (ip = namei(path)) == 0)     return -1;

    ilock(ip);

    if (ip->type != T_DIR) {
        iunlockput(ip);
        return -1;
    }

    iunlock(ip);
    iput(proc->cwd);
    proc->cwd = ip;

    return 0;
}


//-------------------------------------------------------------------------------------------------------------------
// sys_exec() -- execute a binary
//-------------------------------------------------------------------------------------------------------------------
int sys_exec(void)
{
    char *path, *argv[MAXARG];
    int i;
    uint uargv, uarg;

    if (argstr(0, &path) < 0 || argint(1, (int*)&uargv) < 0) return -1;

    memset(argv, 0, sizeof(argv));

    for (i = 0;; i ++) {
        if (i >= NELEM(argv)) return -1;
        if (fetchint(proc, uargv + 4 * i, (int*)&uarg) < 0) return -1;
        if (uarg == 0) {
            argv[i] = 0;
            break;
        }
        if (fetchstr(proc, uarg, &argv[i]) < 0) return -1;
    }

    return exec(path, argv);
}


//-------------------------------------------------------------------------------------------------------------------
// sys_pipe() -- pipe output from one process into input for another
//-------------------------------------------------------------------------------------------------------------------
int sys_pipe(void)
{
    int *fd;
    struct file *rf, *wf;
    int fd0, fd1;

    if (argptr(0, (void*)&fd, 2*sizeof(fd[0])) < 0) return -1;
    if (pipealloc(&rf, &wf) < 0) return -1;

    fd0 = -1;

    if ((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0) {
        if (fd0 >= 0) proc->ofile[fd0] = 0;

        fileclose(rf);
        fileclose(wf);

        return -1;
    }

    fd[0] = fd0;
    fd[1] = fd1;

    return 0;
}


//-------------------------------------------------------------------------------------------------------------------
// sys_pread() -- read a partition block
//-------------------------------------------------------------------------------------------------------------------
int sys_pread(void)
{
    uint partnum;
    uint block;
    uint sz;
    char *dest;

    if (argint(0, (int *)&partnum) < 0 || argint(1, (int *)&block) < 0
            || argint(3, (int *)&sz) || argptr(2, &dest, sz)) return -1;

    return pread(partnum, block, dest, sz);
}


//-------------------------------------------------------------------------------------------------------------------
// sys_pwrite() -- write a partition block
//-------------------------------------------------------------------------------------------------------------------
int sys_pwrite(void)
{
    uint partnum;
    uint block;
    uint sz;
    char *dest;

    if (argint(0, (int *)&partnum) < 0 || argint(1, (int *)&block) < 0
            || argint(3, (int *)&sz) || argptr(2, &dest, sz)) return -1;

    return pwrite(partnum, block, dest, sz);
}



//-------------------------------------------------------------------------------------------------------------------
// sys_readsect() -- read a raw sector from disk
//-------------------------------------------------------------------------------------------------------------------
int sys_readsect(void)
{
    extern int idereadsect(uint, uint, char *);

    uint dev;
    uint sect;
    char *dest;

    if (argint(0, (int *)&dev) < 0 || argint(1, (int *)&sect) || argptr(2, &dest, 512)) return -1;

    return idereadsect(dev, sect, dest);
}
