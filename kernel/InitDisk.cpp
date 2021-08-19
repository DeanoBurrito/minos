#include <InitDisk.h>
#include <KLog.h>
#include <StringExtras.h>

namespace Kernel
{
    void LoadInitDisk()
    {
        Log("Init disk start at: 0x", false);
        Log(sl::UIntToString((uint64_t)&_binary____kernel_disk_build_kdisk_img_start, BASE_HEX).Data());

        Log("Init disk end at: 0x", false);
        Log(sl::UIntToString((uint64_t)&_binary____kernel_disk_build_kdisk_img_end, BASE_HEX).Data());

        Log("Init disk length: 0x", false);
        Log(sl::UIntToString((uint64_t)&_binary____kernel_disk_build_kdisk_img_size, BASE_HEX).Data());
    }

    bool GetFileData(void** start, void** end, size_t* size)
    {
        *start = *end = size = nullptr;
        return false; //TODO:
    }
}