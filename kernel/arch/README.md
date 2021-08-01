# Notes on implementing platform layers

### What must be implemented in this folder
- The entire CPU.h class requires definitions
- Any platform specific ABI stubs
- Linked script for this platform
- A `make.cfg` that adds the required *.cpp and *.asm files to the build process
- Sources to build crti.o and crtn.o
- A function `void SchedulerTimerInterruptHandler()` that will be called on each timer interrupt. It should implement thread switching for that platform. Seee x86_64 for base implementation.


