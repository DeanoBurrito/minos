#pragma once

#include <stddef.h>

extern "C"
{
    extern char _binary____kernel_disk_build_kdisk_img_start;
    extern char _binary____kernel_disk_build_kdisk_img_size;
    extern char _binary____kernel_disk_build_kdisk_img_end;
}

namespace Kernel
{
    //'loads' the init disk, parses the filesystem and maps all known files.
    void LoadInitDisk();
    //returns true if file exists, false if not. If file was found, returns pts to start/end, and the size of said file
    bool GetFileData(void** start, void** end, size_t* size);
}
