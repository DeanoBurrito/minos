# Minos - Minal Operating System
(It's more of a hobby project, than anything serious really.)

# Building
Minos requires a few things to build properly:
- A linux-like environment. I use WSL personally, I imagine cygwin would work as well.
- A cross compiler, preferably one in your `$PATH`. Minos is built with GCC 11, but this can be edited in kernel/makefile under CXX and LD vars. If you're unsure of how to get a hold of one, the os dev wiki have a great guide [here](https://wiki.osdev.org/GCC_Cross-Compiler)
- GNU make (any recent version), mtools, xorriso.
- GDB is always useful.
- If you want to build and run using the default makefile, you'll need QEMU and ovmf.

Once you've got those, cd into `kernel/` and run `make all|iso|run|clean`. 
Each target does what you'd expect, but for the purpose of documentation:
- `make all` builds the current kernel and leaves in the `build/` directory.
- `make iso` builds the kernel and a bootloader, then creates a bootable iso.
- `make run` same as iso, but launches qemu with the iso as a cdrom.
- `make clean` removes all build related files (for all projects), forcing a complete rebuild.

# Supported platforms
Currently Minos only runs on x86_64 CPUs, and requires uefi support for the bootloader.
I have do plans to eventually port it to Aarch64 (raspberry pi 3/4), and add a simple bios bootloader as well.
Best compatability is with QEMU, as that's what I develop it on.

# Project Layout
Currently there are 3 top-level directories, each of these is a mostly isolated sub-project.
- `boot/`: This is where the bootloaders live. They're mutually exclusive when building, and can be selected in the kernel makefile. 
- `kernel/`: Here is the kernel itself, this is where the exciting buiness happens. There's some shared headers in boot dir, in order to receive data from the bootloader.
- `kernel/arch/xyz/`: Architecture specific code, where xyz is the target arch (x86_64 for example). Only one arch directory can be linked at a time, and essentially serves as the HAL.
- `kernel-disk/`: Kernel ramdisk. Currently implemented as DOS style FAT12/FAT16 disk (created using mtools). This is embedded IN the kernel elf, so so small files only, but it has the benefit that post-bootloader we have access to a filesystem without actually needing drivers.
- `syslib/`: System library. Contains a collection of utility code, nothing specific to kernel or user development.
- `userlib/`: Userspace library. Contains code for performing syscalls and other useful functions. 

The `build/` and `include/` directories in each project same the same purpose, storing compiled and linked files, and for holding header files.

# Build System
Its super simple, its all makefiles. The root makefile is currently in the `kernel/` directory, 
however there are plans to restructure that at some point.
This kernel makefile has references to all the other projects, and will manage them as required.
This includes the different bootloaders.

# Other Notes
compile_flags.txt is specific to my install of all these tools, and may not work for you.
TODO.txt is notes for my future self on implementation notes and all that.

# Development Goals
Just here to feel like there's progress being made.

### Previous features
- [x] Flexible UEFI bootloader
- [x] IDT and GDT implemented and *understood*
- [x] PS/2 Keyboard driver
- [x] Basic memory manager and heap allocator
- [x] String and string builders

### Next Milestone
- [x] APIC/IOAPIC drivers
- [x] HPET driver
- [ ] Initdisk support
- [ ] Complete virtual memory manager
- [ ] Working kernel scheduler
- [ ] FPU and SSE support
- [ ] Completed string formatting

### Beyond that
- [ ] Virtual file system
- [ ] Userland
- [ ] Expanded template library
- [ ] IPC and system calls
