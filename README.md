# Minos - Minal Operating System
(It's more of a hobby project, than anything serious really.)

# Building
Minos requires a few things to build properly:
- A linux-like environment. I use WSL personally, I imagine cygwin would work as well.
- A cross compiler, preferably one in your `$PATH`. Minos is built with GCC 11, but this can be edited in kernel/makefile under CXX and LD vars. If you're unsure of how to get a hold of one, the os dev wiki have a great guide [here](https://wiki.osdev.org/GCC_Cross-Compiler)
- GNU make (any recent version), mtools, xorriso
- If you want to build and run using the default makefile, you'll need QEMU and ovmf.

Once you've got those, cd into `kernel/` and run `make all|iso|run|clean`. 
Each target does what you'd expect, but for the purpose of documentation:
- `make all` builds the current kernel and leaves in the `build/` directory.
- `make iso` builds the kernel, the bootloader, creates a bootable iso.
- `make run` builds kernel and bootloader, creates an iso and then launches qemu with that iso.
- `make clean` removes the build directories (kernel and bootloader) completely, forcing a complete rebuild of the project.

# Supported platforms
Currently Minos only runs on x86_64 CPUs, and requires uefi support for the bootloader.
I have do plans to eventually port it to Aarch64 (raspberry pi 3/4), and add a simple bios bootloader as well.
Best compatability is with QEMU, as that's what I develop it on.

# OS Features
Right now? Not much. I'm spending lots of exploring documentation, and writing drivers for various parts system components rather than actual features.

# Project Layout
Currently there are 3 top-level directories, each of these is a mostly isolated sub-project.
- `bootloader/`: This is where the UEFI bootloader lives. 
- `kernel/`: Here is the kernel itself, this is where the exciting buiness happens. There is a dependancy on the bootloader project for the BootInfo structure, and it statically links against syslib for some formatting features.
- `kernel/arch/xyz/`: Architecture specific code, where xyz is the target arch (x86_64 for example). Only one arch directory can be linked at a time, and essentially serves as the HAL.
- `kernel-disk/`: Kernel ramdisk. Currently empty, once the kernel starts requiring some resources other than code at boot-time, this is where they'll be stored. This will be squashed into a blob, and required as part of BootInfo in the future.
- `syslib/`: System library. Contains a collection of utility code, nothing specific to kernel or user development.
- `userlib/`: Userspace library. Contains code for performing syscalls and other useful functions. 

The `build/` and `include/` directories in each project same the same purpose, storing compiled and linked files, and for holding header files.

# Other Notes
compile_flags.txt is specific to my install of all these tools, and may not work for you.

TODO.txt is notes for my future self on implementation notes and all that.