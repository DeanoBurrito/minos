#include "GDT.h"

namespace Kernel
{
    __attribute__((aligned(4096)))
    GDT defaultGdt{
        {0, 0, 0, 0x00, 0x00, 0}, //null
        {0, 0, 0, 0x9A, 0xA0, 0}, //kernel code
        {0, 0, 0, 0x92, 0xA0, 0}, //kernel data
        {0, 0, 0, 0x00, 0x00, 0}, //user null
        {0, 0, 0, 0x9A, 0xA0, 0}, //kernel code
        {0, 0, 0, 0x92, 0xA0, 0}, //kernel data
    };
}
