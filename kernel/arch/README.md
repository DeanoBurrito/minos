# CPU Architectures
Each folder here represents a single CPU architecture. The code in each of these is mutually exclusive.
For each architecture added, Platform.h should be modified to include a new definition, and have it's ArchitectureDefs.h
be included when the matching architecture is available (see X86_64 for reference implementation).

# API requirements
Each architecture is required to implement (at a bare minimum):
- linker.lds: linker script to link the executable.
- local.mk: sub-makefile for the local directory, see the build system documentation for more details
- Implementation of the functions defined in CPU.h
- Implementation of the functions defined in Panic.h

There's a soft-standard of providing an ArchitectureDefs.h in the corresponding `include/arch` directory, 
where any magic numbers can be defined.

Outside of these requirements, architectures can do what they need to here. Exposing functionality can be done in the
matching directory in the `include/arch/` folder.

# Platform specific features
Some platforms that share ISAs may have unique features that need more than a simple device driver.
In this case the `platform` folder can contain a uniquely named folder for that platform. 
This isnt currently supported by the build system, but will be soon. The reference implementation is likely to be
the raspberry pi 3/4/zero-w.
