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
##                #18    Adam  Removed the _hello target from this Makefile.                                       ##
##                #12    Adam  Removed the _cat target from this Makefile.                                         ##
##                #13    Adam  Removed the _crash target from this Makefile.                                       ##
##                #14    Adam  Removed the _echo target from this Makefile.                                        ##
##  2015-09-28    #15    Adam  Removed the _factor target from this Makefile.                                      ##
##                #16    Adam  Removed the _forktest target from this Makefile.                                    ##
##                #17    Adam  Removed the _grep target from this Makefile.                                        ##
##                #19    Adam  Removed the _init target from this Makefile.                                        ##
##                #20    Adam  Removed the _kill target from this Makefile.                                        ##
##                #21    Adam  Removed the _ln target from this Makefile.                                          ##
##                #22    Adam  Removed the _ls target from this Makefile.                                          ##
##                #23    Adam  Removed the _mkdir target from this Makefile.                                       ##
##                #24    Adam  Removed the _null target from this Makefile.                                        ##
##                #25    Adam  Removed the _rm target from this Makefile.                                          ##
##                #26    Adam  Removed the _sh target from this Makefile.                                          ##
##                #27    Adam  Removed the _share target from this Makefile.                                       ##
##                #29    Adam  Removed the _stressfs target from this Makefile.                                    ##
##                #30    Adam  Removed the _usertests target from this Makefile.                                   ##
##                #31    Adam  Removed the _wc target from this Makefile.                                          ##
##                #32    Adam  Removed the _zombie target from this Makefile.                                      ##
##  2015-09-30    #33    Adam  Removed the bootblock target from this Makefile.                                    ##
##                #35    Adam  Removed the entryother target from this Makefile.                                   ##
##                #36    Adam  Removed the initcode target from this Makefile.                                     ##
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


#
# -- The following empty names are used to track user program targets -- filled in by the Makefrag files
#    ---------------------------------------------------------------------------------------------------
DEPEND-FILES	:=
UPROGS			:=


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
all: xv6.img


#
# -- Include sub-project files
#    -------------------------
include $(MAKE-FRAG)


xv6.img: bootblock kernel fs.img
	@dd if=/dev/zero of=xv6.img count=10000
	@dd if=bootblock of=xv6.img conv=notrunc
	@dd if=kernel of=xv6.img seek=1 conv=notrunc

xv6memfs.img: bootblock kernelmemfs
	@dd if=/dev/zero of=xv6memfs.img count=10000
	@dd if=bootblock of=xv6memfs.img conv=notrunc
	@dd if=kernelmemfs of=xv6memfs.img seek=1 conv=notrunc


# make a printout
FILES 			:= $(shell grep -v '^\#' runoff.list)
PRINT 			:= runoff.list runoff.spec README toc.hdr toc.ftr $(FILES)

tags: $(OBJS) entryother.S _init
	etags *.S *.c



fs.img: mkfs doc/README $(UPROGS)
	@mkdir -p sysroot
	@echo "COPYTO:" $@
	@cp doc/README .
	@./mkfs fs.img README $(UPROGS)
	@rm -f README


ifneq ($(MAKECMDGOALS),clean)
-include $(DEPEND-FILES)
endif


clean: $(MAKE-CLEAN)
	@rm -f *.tex *.dvi *.idx *.aux *.log *.ind *.ilg \
		*.o *.d *.asm *.sym vectors.S bootblock entryother \
		initcode initcode.out kernel xv6.img fs.img kernelmemfs mkfs \
		.gdbinit \
		$(UPROGS)
	@rm -fR sysroot


xv6.pdf: $(PRINT)
	./runoff
	ls -l xv6.pdf

print: xv6.pdf

# run in emulators

bochs : fs.img xv6.img
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
QEMUOPTS = -hdb fs.img xv6.img -smp $(CPUS) -m 512 $(QEMUEXTRA)

qemu: fs.img xv6.img
	$(QEMU) -serial mon:stdio $(QEMUOPTS)

qemu-memfs: xv6memfs.img
	$(QEMU) xv6memfs.img -smp $(CPUS)

qemu-nox: fs.img xv6.img
	$(QEMU) -nographic $(QEMUOPTS)

.gdbinit: .gdbinit.tmpl
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

qemu-gdb: fs.img xv6.img .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -serial mon:stdio $(QEMUOPTS) -S $(QEMUGDB)

qemu-nox-gdb: fs.img xv6.img .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -nographic $(QEMUOPTS) -S $(QEMUGDB)

# the dd below stretches the fs.img out so it is large enough not to
# cause VBox to give an error
fs.vdi: fs.img
	rm -f fs.vdi
	dd if=/dev/zero count=10000 >> fs.img
	VBoxManage convertfromraw fs.img fs.vdi

xv6.vdi: xv6.img
	rm -f xv6.vdi
	VBoxManage convertfromraw xv6.img xv6.vdi

vbox: xv6.vdi fs.vdi

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
	@echo "MAKE-CLEAN     =" $(MAKE-CLEAN)
	@echo "MAKE-FRAG      =" $(MAKE-FRAG)
	@echo "MAKE-FILES     =" $(MAKE-FILES)
	@echo "UPROGS         =" $(UPROGS)
	@echo "ULIB           =" $(ULIB)
	@echo "DEPEND-FILES   =" $(DEPEND-FILES)
