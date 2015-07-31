# century -- A hobby operating system

Century is a project to merge the works of Royalty, CenturyOS, Century-64, and
Project Rosewood into a single multi-host operating system.

The motivation for this project was envisioned after a fatal crash of my
development PC as a result of a multi-disk raid failure.  I had backups of most
of my code, but I did lose some work.

Additionally, the projects had their own issues, which I plan to address in this
project:
* Royalty was my first attempt at writing a kernel.  It was a monolithic kernel
  whose code was not structured well.  This was a 32-bit kernel.
* CenturyOS was the next genesis of the Royalty.  Much of the code was lifted
  and moved to CenturyOS at the same time organizing the code better into
  separate "modules" as it were.  However, all the code was still compiled into
  a single monolithic kernel.  Additionally, each function had its own source
  file, which made it simple to find a function.  However, since the build system
  was lacking, I had to do a full build with each iteration.
* Project Rosewood was a further incarnation of the Royalty-CenturyOS development
  line that never really got anywhere.
* Century-64 is a fresh start at a 64-bit OS written completely in assembly.

The objective here is to lift relevant portions of each of these systems and lay
them into this multi-host directory structure.  Additionally, I plan to support
ARM processors as well (namely, RaspberryPi initially).

In order to accomplish this, I have taken on the task to properly segregate the
architecture-specific code from the common code and to refine the build system
to work properly in this highly complex environment.  With the bare bones source
in the initial commits, I think I have been able to accomplish this.

As new source members are added to modules, I should not have to make significant
changes to the build system's Makefile/Makefrag files.  The only changes I might
have to make is to add architecture-specific code to the libc/libk libraries.
When I add another module to the build, I should only need to be concerned with
getting its Makefrag file correct and it should intergrate into the build system
nicely without making sweeping changes to the files.

# Roadmap

For this initial version (say, v0.1), I plan on getting my 32-bit kernel back
to a stable state.  These features will include:
* GDT/IDT
* Physical Memory Management
* Virtual Memory Management
* Kernel Heap Management
* Process Management with Scheduler
* Spinlocks

Then, with the next version (say, v0.2), I plan on "duplicating" all this with
a 64-bit target.  I say duplicating in quotes because there will be some code
reuse, of course.  And, if I am able to get this all accomplished before the
end of 2016, I will be happy!
