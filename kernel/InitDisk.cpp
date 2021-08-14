#include <InitDisk.h>
#include <KLog.h>
#include <StringUtil.h>

namespace Kernel
{
    void LoadInitDisk()
    {
        Log("Init disk start at: 0x", false);
        Log(ToStrHex((uint64_t)&_binary____kernel_disk_build_kdisk_img_start));

        Log("Init disk end at: 0x", false);
        Log(ToStrHex((uint64_t)&_binary____kernel_disk_build_kdisk_img_end));

        Log("Init disk length: 0x", false);
        Log(ToStrHex((uint64_t)&_binary____kernel_disk_build_kdisk_img_size));
    }

    bool GetFileData(void** start, void** end, size_t* size)
    {
        *start = *end = size = nullptr;
        return false; //TODO:
    }
}