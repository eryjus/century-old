#####################################################################################################################
##                                                                                                                 ##
##  Makefile -- This file contains the core build information for building xv6 (with all architectures)            ##
##                                                                                                                 ##
##  OK, so after reading the outstanding paper by Peter Miller (http://aegis.sourceforge.net/auug97.pdf), I have   ##
##  removed the recursion from this make script and have changed the endire build method.  Initially, I wanted to  ##
##  be able to add a new module and have it automatically picked up in the build.  However, after reading the      ##
##  above white paper, I know why I am having trouble with my builds.  So, it will be changed.                     ##
##                                                                                                                 ##
## --------------------------------------------------------------------------------------------------------------- ##
##                                                                                                                 ##
##     Date     Tracker  Pgmr  Description                                                                         ##
##  ----------  -------  ----  ----------------------------------------------------------------------------------  ##
##  2015-09-27  Initial  Adam  Initial version.  Initially borrowed from the xv6 build system.                     ##
##  2015-09-29  #12-#36  Adam  Reorganized this Makefile into subprojects                                          ##
##  2015-09-30    #42    Adam  It is time to boot from GRUB.  The entry.S source has already been prepared to be   ##
##                             Multiboot ready.  So the task is to take the 2-disk boot configuration and          ##
##                             consolidate it down to a single disk.  To do this, we will install GRUB on a disk   ##
##                             image file formatted in the ext2 file system.  This will involve a boot directory   ##
##                             while all the rest of the executables will reside in the root folder until we have  ##
##                             a more robust system.  In the end if all goes well, we will be able to depricate    ##
##                             the bootblock module and the mkfs build utility.                                    ##
##  2015-10-05    #58    Adam  Removed deprecated bootblock from the build                                         ##
##                #46    Adam  Eliminated xv6.img disk image                                                       ##
##                #44    Adam  Eliminated UPROGS2 as no longer needed                                              ##
##                                                                                                                 ##
#####################################################################################################################


.SILENT:


#
# -- These are some internal and intermediate targets
#    ------------------------------------------------
MAKE-FRAG 		:= $(sort $(wildcard **/**/Makefrag))
MAKE-PARTS		:= $(MAKE-FRAG)
MAKE-CLEAN 		:= $(subst /,,$(subst src/,,$(addprefix clean-,$(dir $(MAKE-PARTS)))))
MAKE-FILES 		:= Makefile $(MAKE-PARTS) $(wildcard *.mk)

DISP-VARS 		:= $(sort $(subst /,,$(subst src/,,$(addprefix disp-vars-,$(dir $(MAKE-PARTS))))))

INCLUDE			:= include


#
# -- Set some strings for helping with the targets
#    ---------------------------------------------
TOOLPREFIX 		:= i686-elf-
QEMU 			:= /bin/qemu-system-i386
SYSROOT			:= sysroot
UFS 			:= usrfs


#
# -- The following empty names are used to track user program targets -- filled in by the Makefrag files
#    ---------------------------------------------------------------------------------------------------
DEPEND-FILES	:=
UPROGS			:=
TARGETS			:=


#
# -- The following are the tools for the cross compiler
#    --------------------------------------------------
CC 				:= $(TOOLPREFIX)gcc
AS 				:= $(TOOLPREFIX)gas
AR 				:= $(TOOLPREFIX)ar
LD 				:= $(TOOLPREFIX)ld
OBJCOPY 		:= $(TOOLPREFIX)objcopy
OBJDUMP 		:= $(TOOLPREFIX)objdump

CFLAGS 			:= -fno-pic -static -fno-builtin -fno-strict-aliasing -Wall -ggdb -m32 -Werror -fno-omit-frame-pointer
CFLAGS 			+= $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
CFLAGS 			+= -Iinclude

ARFLAGS 		:= rcs

ASFLAGS 		:= -m32 -gdwarf-2 -Wa,-divide

LDFLAGS 		+= -m $(shell $(LD) -V | grep elf_i386 2>/dev/null) -L.


#
# -- Make sure we have a default build before we include all the other modules
#    -------------------------------------------------------------------------
.PHONY: all
all: xv6-grub.img


#
# -- Include sub-project files
#    -------------------------
include $(MAKE-FRAG)


#
# -- This rule and the following recipes are used to build a disk image that can be booted with grub:
#    * create a disk image, size = 20MB
#    * make the partition table, partition it, and set it to bootable
#    * map the partitions from the image file
#    * write an ext2 file system to the first partition
#    * create a temporary mount point
#    * Mount the filesystem via loopback
#    * copy the files to the disk
#    * create a device map for grub
#    * install grub
#    * unmount the device
#    * unmap the image
#
#    In the event of an error along the way, the image is umounted and the partitions unmapped.
#    Finally, if the error cleanup is completely suffessful, then false is called to fail the
#    recipe.
#    ------------------------------------------------------------------------------------------------
xv6-grub.img: $(SYSROOT)/boot/kernel fs.img grub.cfg
	echo "IMG   :" $@
	sudo rm -fR p1
	rm -f $@
	mkdir -p ./p1
	mkdir -p ./tmp
	(dd if=/dev/zero of=$@ count=20 bs=1048576 &> /dev/null;								\
		parted --script $@ mklabel msdos mkpart p ext2 1 20 set 1 boot on; 					\
		sudo kpartx -as $@;																	\
		sudo mkfs.ext2 /dev/mapper/loop0p1 &> /dev/null;									\
		sudo mount /dev/mapper/loop0p1 ./p1;												\
		echo "(hd0) /dev/loop0" > ./tmp/device.map;											\
		sudo mkdir -p ./p1/boot/grub2/locale;												\
		sudo mkdir -p ./p1/boot/grub2/i386-pc;												\
		sudo cp -R $(SYSROOT)/* p1/;														\
		sudo cp $(SYSROOT)/boot/kernel p1;																	\
		sudo cp grub.cfg p1/boot/grub2/grub.cfg;											\
		sudo grub2-install --no-floppy --grub-mkdevicemap=tmp/device.map 					\
				--modules="biosdisk part_msdos ext2 configfile normal multiboot" 			\
				--root-directory=`readlink -f p1` /dev/loop0 &> /dev/null;					\
		sudo umount ./p1;																	\
		sudo kpartx -d $@ &> /dev/null;														\
	) || 																					\
	(	sudo umount ./p1;																	\
		sudo kpartx -d $@ &> /dev/null;														\
	) || false


#
# -- create the grub config file
#    ---------------------------
grub.cfg: Makefile
	echo "GRUB  :" $@
	echo 'set timeout=0'						 > $@
	echo 'menuentry "Century OS"' { 			>> $@
	echo '	multiboot /kernel /kernel'	     	>> $@
	echo '	boot'								>> $@
	echo '}'									>> $@


# make a printout
FILES 			:= $(shell grep -v '^\#' runoff.list)
PRINT 			:= runoff.list runoff.spec README toc.hdr toc.ftr $(FILES)

tags: $(OBJS) entryother.S _init
	etags *.S *.c



fs.img: mkfs doc/README $(UPROGS)
	echo "MKFS  :" $@
	cp doc/README .
	./mkfs fs.img README $(UPROGS) &> /dev/null
	rm -f ./README


ifneq ($(MAKECMDGOALS),clean)
-include $(DEPEND-FILES)
endif


clean: $(MAKE-CLEAN)
	rm -f *.tex *.dvi *.idx *.aux *.log *.ind *.ilg \
		*.o *.d *.asm *.sym vectors.S entryother \
		initcode initcode.out kernel kernelmemfs mkfs \
		.gdbinit *.img README libu.a \
		$(UPROGS) grub.cfg .bochsrc bochsout.txt
	rm -fR $(SYSROOT)
	rm -fR usrfs
	rm -fR tmp
	rm -fR lib
	sudo rm -fR p1


xv6.pdf: $(PRINT)
	./runoff
	ls -l xv6.pdf

print: xv6.pdf

# run in emulators

bochs : xv6-grub.img
	if [ ! -e .bochsrc ]; then ln -s dot-bochsrc .bochsrc; fi
	bochs -q

# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line changed in 0.11
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)
ifndef CPUS
CPUS := 2
endif
#QEMUOPTS = -hdb fs.img xv6.img -smp $(CPUS) -m 512 $(QEMUEXTRA)
QEMUOPTS = -hdb fs.img xv6-grub.img -smp $(CPUS) -m 1024 $(QEMUEXTRA)

qemu: xv6-grub.img
	$(QEMU) -serial mon:stdio $(QEMUOPTS)

qemu-nox: fs.img xv6-grub.img
	$(QEMU) -nographic $(QEMUOPTS)

.gdbinit: .gdbinit.tmpl
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

qemu-gdb: fs.img xv6-grub.img .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -serial mon:stdio $(QEMUOPTS) -S $(QEMUGDB)

qemu-nox-gdb: fs.img xv6-grub.img .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -nographic $(QEMUOPTS) -S $(QEMUGDB)

# the dd below stretches the fs.img out so it is large enough not to
# cause VBox to give an error
fs.vdi: fs.img
	rm -f fs.vdi
	dd if=/dev/zero count=10000 >> fs.img
	VBoxManage convertfromraw fs.img fs.vdi

xv6-grub.vdi: xv6-grub.img
	rm -f xv6-grub.vdi
	VBoxManage convertfromraw xv6-grub.img xv6-grub.vdi

vbox: xv6-grub.vdi fs.vdi

# CUT HERE
# prepare dist for students
# after running make dist, probably want to
# rename it to rev0 or rev1 or so on and then
# check in that version.

EXTRA=\
	mkfs.c ulib.c user.h cat.c echo.c forktest.c grep.c kill.c\
	ln.c ls.c mkdir.c rm.c stressfs.c usertests.c wc.c zombie.c\
	printf.c umalloc.c\
	README dot-bochsrc *.pl toc.* runoff runoff1 runoff.list\
	.gdbinit.tmpl gdbutil\

dist:
	rm -rf dist
	mkdir dist
	for i in $(FILES); \
	do \
		grep -v PAGEBREAK $$i >dist/$$i; \
	done
	sed '/CUT HERE/,$$d' Makefile >dist/Makefile
	echo >dist/runoff.spec
	cp $(EXTRA) dist

dist-test:
	rm -rf dist
	make dist
	rm -rf dist-test
	mkdir dist-test
	cp dist/* dist-test
	cd dist-test; $(MAKE) print
	cd dist-test; $(MAKE) bochs || true
	cd dist-test; $(MAKE) qemu

# update this rule (change rev#) when it is time to
# make a new revision.
tar:
	rm -rf /tmp/xv6
	mkdir -p /tmp/xv6
	cp dist/* dist/.gdbinit.tmpl /tmp/xv6
	(cd /tmp; tar cf - xv6) | gzip >xv6-rev5.tar.gz

.PHONY: dist-test dist


#
# -- Finally, a build target used for debugging this build system
#    ------------------------------------------------------------
.PHONY: disp-vars
disp-vars: $(DISP-VARS)
	echo "MAKE-CLEAN     =" $(MAKE-CLEAN)
	echo "MAKE-FRAG      =" $(MAKE-FRAG)
	echo "MAKE-FILES     =" $(MAKE-FILES)
	echo "UPROGS         =" $(UPROGS)
	echo "ULIB           =" $(ULIB)
	echo "DEPEND-FILES   =" $(DEPEND-FILES)
	echo "TARGETS        =" $(TARGETS)
