![](https://tokei.rs/b1/github/deanoburrito/minos)

# Minos

# Building
Minos tries to keep the build system quite vanilla, and not require too many external dependencies.
The build environment does has a few requirements though (namely a cross-compiler, gnu-efi or limine, and xorriso).
Check a litle further down for the full list, however if you want the easy solution, run
`scripts/setup-env.sh`. The script has a step that install libs required to build gcc via apt, this can be commented out
and performed manually if not using apt.
After install the toolchain, you'll need to point the root makefile to where you installed it.
Now you're ready to go, `make all` will generate an iso and `make run` will launch qemu with the iso loaded (if installed).

### Required tools
-Linux-like environment. I've used WSL2 successfully in the past, cygwin will sometimes work.
-A compiler with support for > c++17. This is to build the cross compiler(s).
-GNU make.
-xorriso and mtools.
-Some bootloaders require extra tools. The uefi one requires gnu-efi, and the stivale ones limine to be installed.
-Qemu is not required, but is a nice to have for development. Unless you're hardcore.
-Same goes for GDB.

### Make? Make what?
Once the toolchain is set up, you're ready to go. The various make targets are described below. 
Most will output a bootable iso to `iso/`.
- `make all` builds the complete minos system. Kernel, userspace lib, and all the bundled apps.
- `make no-apps` similar to above, except apps are neither built nor packed in the iso.
- `make core` again similar to above, except only the kernel is built and packaged.
- `make clean` cleans all project build files and generated isos, except for app build files.
- `make clean-apps` cleans **all** build files. Better to run clean on any problematic apps themselves.
- `make run` builds 'run-target' (an easily changable alias for an above target) and launches qemu.
- `make debug` same as run, but stops qemu and waits for a gdb connection on port 1234.
- `make validate-toolchain` checks that the makefile can access all the tools it needs. Will report basic errors.

# Supported platforms
Currently Minos only runs on x86_64 CPUs, and requires uefi support for the bootloader.
There are plans to support Aarch32/64 (raspberry pi 1 -> 4 and pi zero), 
and to add more x86_64 bootloaders.

# Project Layout
Currently there are a number of top-level directories, each of these is a mostly isolated sub-project.
- `apps/`: Coming soon! (depending on your definition of soon)
- `boot/`: Bootloaders like here. They're mutually exclusive when building, the current one is selected in the root makefile.
- `kernel/`: Here is the kernel itself, this is where the good stuff happens. There's a shared header from the boot dir (BootInfo.h).
- `kernel-disk/`: Kernel init disk. Loaded with the kernel binary, contains code and data that is not essential enough to be built into the kernel itself.
- `syslib/`: System library. Contains utility code, replacements for parts of the C++ stdlib. Statically linked.
- `userlib/`: Userspace library. Contains code for performing syscalls and other useful functions. 

The `build/` and `include/` directories in each project same the same purpose, storing compiled and linked files, and for holding header files.

### Kernel Source Layout
For the kernel, there are some more notable directories.
- `arch/` contains cpu isa specific code (x86/x86_64/arm6/etc...), check the local readme for more details.
- `arch/platform/` contains platform specific code (think raspbi 2 vs raspbi 4), where functionality may require more than a driver.
- `kshell/` contains code relating to the kernel-mode shell. 

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

#### Milestone 1.1 - Better build system
- [x] Implemented! :D

#### Milestone 1.2 - Kernel improvements
- [x] Interrupts abstraction + API 
- [ ] Syslib improvements (hashtable/hashmap, circularqueue, tuple, optional).
- [ ] Timers abstraction + API
- [ ] Squash current bugs list + tech debt
- [ ] Sync primatives (semaphore, mutex, spinlock)
- [ ] Fix HPET and APIC bugs specificially.

#### Milestone 1.3 - KShell functionality
- [ ] Proper command parsing/exec
- [ ] Implement a few useful debugging commands (mem dump, process tree)
- [x] Added a nice blinking cursor, and status text decays away.

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
- [ ] Port the original doom!
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
