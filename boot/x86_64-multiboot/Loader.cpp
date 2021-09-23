#include "Loader.h"
#include <stddef.h>

extern "C" char _binary_build_kernel_tar_start;
extern "C" char _binary_build_kernel_tar_end;
extern "C" char _binary_build_kernel_tar_size;

//real values, after tar header, and not padded for sector size
size_t kernelBegin;
size_t kernelLength;

bool KernelValid()
{
    return false;
}

bool LoadKernel()
{
    return true;
}

void* GetKernelEntry()
{
    return nullptr;
}
