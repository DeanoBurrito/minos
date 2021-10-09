#pragma once

#include <stddef.h>
#include <Memory.h>
#include <Platform.h>
#include <Limits.h>
#include <collections/List.h>

namespace Kernel
{
    struct StackTrace
    {
        sl::List<NativePtr> frames;
    };
    
    StackTrace GetCurrentTrace(size_t maxDepth = 0);
}
