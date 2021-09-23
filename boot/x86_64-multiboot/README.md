# Multiboot-1 bootloader

Currently this works, but requires some hacks. Graphics support is currently broken.

Some notes about the implementation are worth documenting how this works:
- The kernel itself is build normally, and runs as if it was launched by the uefi bootloader.
- Once the kernel is built, it is copied to the local build directory and packed into a tarball (objcopy will attempt to translate it to i386 otherwise).
- I think objcopy is doing what most people would want it to do, but the tarball just stops that, whilst only adding 512 bytes in size (the file header).
- Once the real kernel file's location has been found, its business as usual. Memory maps are generated, framebuffer is found (soon!) and kernel is loaded
    into memory and it's entry is called.
