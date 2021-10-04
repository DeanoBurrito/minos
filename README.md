![](https://tokei.rs/b1/github/deanoburrito/minos)

# Minos

# Building
Minos is light on dependencies, and the build environment reflects that.
If compiling for a platform other than the one you're running on, you will need a cross-compiler,
the osdev wiki has a great guide on getting one setup [here](https://wiki.osdev.org/GCC_Cross-Compiler).
Otherwise to build you will need the following:
- A linux-like environment. Minos can be built on a full linux install, but WSL and cygwin work as well.
- A compiler, the build system is setup to use a GCC cross compiler by default. This should be in your `$PATH`.
- GNU make (or any compatable tool).
- Xorriso and mtools for building images.
- Gnu-efi is required for building the uefi bootloader, source available [here](https://sourceforge.net/p/gnu-efi/code/ci/master/tree/)

Some nice to have's for running and debugging:
- Qemu is the default VM used for running, not required for building. OVMF is required for booting uefi-bootloader
- GDB is always useful for debugging. Note: yours may need to be configured for cross-platform debugging

Once your environment is set up, the root makefile is current under the `kernel/` directory. 
Each target does what you'd expect, but for the purpose of documentation:
- `make all` builds the kernel in it's current config, and any required projects (syslib, initdisk).
- `make iso` runs all, builds the current bootloader and creates a bootable iso.
- `make run` same as iso, but launches qemu with the iso as a cdrom.
- `make clean` removes all build related files (for all projects), forcing a complete rebuild.

# Supported platforms
Currently Minos only runs on x86_64 CPUs, and requires uefi support for the bootloader.
There are plans to support Aarch32/64 (raspberry pi 1 -> 4 and pi zero), 
and to add more x86_64 bootloaders.

# Project Layout
Currently there are a number of top-level directories, each of these is a mostly isolated sub-project.
- `boot/`: This is where the bootloaders live. They're mutually exclusive when building, and can be selected in the kernel makefile. 
- `kernel/`: Here is the kernel itself, this is where the exciting buiness happens. There's some shared headers in boot dir, in order to receive data from the bootloader.
- `kernel-disk/`: Kernel init disk. Loaded with the kernel binary, contains code and data that is not essential enough to be built into the kernel itself.
- `syslib/`: System library. Contains a collection of utility code, nothing specific to kernel or user development.
- `userlib/`: Userspace library. Contains code for performing syscalls and other useful functions. 

The `build/` and `include/` directories in each project same the same purpose, storing compiled and linked files, and for holding header files.

### Kernel Source Layout
For the kernel, there are some more notable directories.
- `arch/` contains cpu isa specific code (x86/x86_64/arm6/etc...), check the local readme for more details.
- `arch/platform/` contains platform specific code (think raspbi 2 vs raspbi 4), where functionality may require more than a driver.
- `kshell/` contains code relating to the kernel-mode shell. 

# Build System
Its super simple, its all makefiles. The root makefile is currently in the `kernel/` directory, 
however there are plans to restructure that at some point.
This kernel makefile has references to all the other projects, and will manage them as required.
This includes the different bootloaders.

If your environment is set up differently to mine, all the relevant variables are in the top-most sections (interal and external references)

# Other Notes
compile_flags.txt is specific to my install of all these tools, and may not work for you.
TODO.txt is notes for my future self on implementation notes and all that.

# Development Goals
Ultimately this is a hobby project, and so there's no real target feature-set.
Various feature's I'd like to include are listed below, and organised into milestones.

<details>
    <summary>Project milestones</summary>

### Pre-Milestone 1 features
- [x] Flexible UEFI bootloader
- [x] IDT and GDT implemented
- [x] PS/2 Keyboard driver
- [x] Basic memory manager and heap allocator
- [x] Basic kernel-mode renderer
- [x] String and string builders
- [x] CMOS RTC

### Milestone 1 - Stable kernel
- [x] APIC/IOAPIC drivers
- [x] Basic ACPI support (parsing tables)
- [x] HPET driver - partial
- [x] Initdisk support
- [x] Complete virtual memory manager
- [x] Working kernel scheduler
- [x] FPU and SSE support
- [x] Completed string formatting
- [x] Slab allocator and composite allocators

#### Milestone 1.1 - Kernel improvements
- [ ] Interrupts abstraction + API (template/inheritance based) 
- [ ] Timers abstraction + API
- [ ] Squash current bugs list
- [ ] Sync primatives (semaphore, mutex, spinlock)
- [ ] Fix HPET and APIC bugs specificially.

### Milestone 2 - Userland
- [ ] Placeholder accounts - int based (0 = kernel, 1 = user)
- [ ] Processes/Threads with permissions
- [ ] Basic elf parser/loader
- [ ] IPC and system calls
- [ ] Loadable drivers (kernel/user via process permissions)

#### Milestone 2.1 - Revisting boot protocols
- [ ] Multiboot 1
- [ ] Stivale 2
- [ ] Migrate UEFI bootloader to be fully c++
    
### Milestone 3 - Userland++
- [ ] Process 0 (init) - I'm coming for you, systemd.
- [ ] Multiple accounts
- [ ] Userspace drivers and FUSE fs
- [ ] Virtual (proc) filesystem
    
### Milestone 4 - AML interp
- [ ] ACPI/AML driver (lai is worth looking into)
- This is going to be a big detour, hence its own milestone. 

### Milestone 5 - Driver-mania
- [ ] PCI(e) subsystem
- [ ] AHCI and NVME drivers
- [ ] Ext2 filesystem driver
- [ ] Qemu networking driver
- [ ] Qemu graphics driver
- [ ] FAT filesystem driver

### Beyond That ...
- [ ] Networking stack
- [ ] Expanded template library
- [ ] Multicore booting
- [ ] Multicore scheduling
- [ ] Libc implementation (port mlibc?)

</details>

<details>
    <summary>Credits and special mentions</summary>
    Big thanks to the authors behind the osdev wiki, and the people on the unaffiliated osdev discord.
    Likewise the Intel SDM authors, and especially my local coffee shop. 💌 (haha hope your browser supports unicode)
</details>
