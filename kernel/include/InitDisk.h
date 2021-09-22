#pragma once

#include <stddef.h>
#include <String.h>

namespace Kernel
{
    //'loads' the init disk, parses the filesystem and maps all known files.
    void LoadInitDisk();
    //returns true if file exists, false if not. If file was found, returns pts to start/end, and the size of said file
    bool GetFileData(string filename, void** start, void** end, size_t* size);
}
