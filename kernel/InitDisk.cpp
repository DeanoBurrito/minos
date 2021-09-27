#include <InitDisk.h>
#include <KLog.h>
#include <StringExtras.h>
#include <Formatting.h>
#include <stdint.h>
#include <Memory.h>

namespace Kernel
{
    extern "C"
    {
        //NOTE: these MUST match the output filename of the kernel disk, otherwise this will fail.
        extern char _binary____kernel_disk_build_kdisk_bin_start;
        extern char _binary____kernel_disk_build_kdisk_bin_size;
        extern char _binary____kernel_disk_build_kdisk_bin_end;
    }

    enum TarFiletype : uint8_t
    {
        Normal = 0,
        Normal2 = '0',

        HardLink = '1',
        SymLink = '2',
        CharacterDevice = '3',
        BlockDevice = '4',
        Directory = '5',
        Pipe = '6',
    };

    #define USTAR_FILENAME_LENGTH 100
    #define USTAR_FILESIZE_LENGTH 12
    #define USTAR_SECTOR_SIZE 512

    struct TarHeader
    {
        uint8_t filename[USTAR_FILENAME_LENGTH];
        uint64_t mode;
        uint64_t ownerId;
        uint64_t groupId;
        uint8_t fileSize[USTAR_FILESIZE_LENGTH];
        uint8_t modifiedTime[12];
        uint64_t checksum;
        uint8_t type;
        uint8_t linkedFilename[USTAR_FILENAME_LENGTH];
        uint8_t signature[6]; //should be "ustar\0"
        uint8_t version[2]; //should be "00"
        uint8_t ownerName[32];
        uint8_t groupName[32];
        uint64_t devMajor;
        uint64_t devMinor;
        uint8_t filenamePrefix[155]; //Not sure what this is used for?
    } __attribute__((packed));

    size_t diskStart;
    size_t diskEnd;
    size_t diskLength;

    size_t OctalToUseful(uint8_t* oct, size_t maxSize)
    {
        //Original code from OSDEV, licensed under public domain: https://wiki.osdev.org/USTAR
        size_t ret = 0;
        uint8_t* ptr = oct;

        while (maxSize-- > 0)
        {
            ret *= 8;
            ret += *ptr - '0';
            ptr++;
        }

        return ret;
    }
    
    void LoadInitDisk()
    {
        //alias these to some more user-friendly values
        diskStart = (size_t)&_binary____kernel_disk_build_kdisk_bin_start;
        diskEnd = (size_t)&_binary____kernel_disk_build_kdisk_bin_end;
        diskLength = (size_t)&_binary____kernel_disk_build_kdisk_bin_size;
        
        string fstr = "Init disk located 0x%llx - 0x%llx (%llu bytes long).";
        Log(sl::FormatToString(0, &fstr, diskStart, diskEnd, diskLength).Data());
    }

    bool GetFileData(string filename, void** start, void** end, size_t* size)
    {
        sl::UIntPtr scanPtr(diskStart);
        TarHeader* scan = reinterpret_cast<TarHeader*>(scanPtr.ptr);

        while ((size_t)scan < diskEnd)
        {
            if (sl::memcmp("ustar", scan->signature, 5) != 0)
            {
                //somehow we ended up reading a sector as a header, without a valid signature
                scanPtr.raw += USTAR_SECTOR_SIZE;
                scan = reinterpret_cast<TarHeader*>(scanPtr.ptr);
                continue;
            }
            
            size_t strlen = sl::memfirst(scan->filename, 0, USTAR_FILENAME_LENGTH);
            if (filename.Size() < strlen)
                strlen = filename.Size();
            
            if (sl::memcmp(scan->filename, filename.Data(), strlen) == 0)
            {
                //bingo, matching filename
                *size = OctalToUseful(scan->fileSize, USTAR_FILESIZE_LENGTH - 1);

                sl::UIntPtr ip(scan);
                ip.raw += USTAR_SECTOR_SIZE;
                *start = ip.ptr;
                ip.raw += *size;
                *end = ip.ptr;

                return true;
            }

            //it's a miss, calculate where the next header should be and look there
            size_t fileSize = OctalToUseful(scan->fileSize, USTAR_FILESIZE_LENGTH - 1);
            size_t remainder = USTAR_SECTOR_SIZE - (fileSize % USTAR_SECTOR_SIZE);
            if (remainder == USTAR_SECTOR_SIZE)
                remainder = 0; //if filesize is sector aligned, then dont consume next sector, itll be a header.

            scanPtr.raw += USTAR_SECTOR_SIZE + fileSize + remainder; //consume header + all data sectors (rounding up to consunme the entire last one)
            scan = reinterpret_cast<TarHeader*>(scanPtr.ptr);
        }
        
        *start = *end = size = nullptr;
        return false; 
    }
}