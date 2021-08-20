#include <InitDisk.h>
#include <KLog.h>
#include <StringExtras.h>
#include <stdint-gcc.h>

namespace Kernel
{
    struct FatBootRecord
    {
        uint8_t bootBytes[3]; //bytes to jump to actual boot code
        uint64_t oemId;
        uint16_t bytesPerSector;
        uint8_t sectorsPerCluster;
        uint16_t reservedSectorsCount; //includes boot record sectors
        uint8_t fatCount; //usually 2
        uint16_t directoriesCount;
        uint16_t logicalSectorCount; //if 0, check extended version of this field
        uint8_t mediaDescriptorType;
        uint16_t sectorsPerFat;
        uint16_t sectorsPerTrack;
        uint16_t mediaHeadCount;
        uint32_t hiddenSectorsCount;
        uint32_t largeSectorCount;
    } __attribute__((packed));

    struct Fat12ExtendedBootRecord : public FatBootRecord
    {
        uint8_t driveIndex;
        uint8_t ntFlags;
        uint8_t signature; //must be 0x28/0x29
        uint32_t volumeSerial;
        uint8_t volumeLabel[11]; //padded with spaces
        uint8_t junkIdentifier[8]; //meant to be a textual representation of filesystem type.
    } __attribute__((packed));

    struct FatDirectory
    {
        uint8_t dosFilename[11]; //lame 8.3 filename
        uint8_t attribs;
        uint8_t ntReserved;
        uint8_t creationSubsecond;
        uint16_t creationTime;
        uint16_t creationDate;
        uint16_t accessedDate;
        uint16_t firstEntryHigh; //0 for fat12/fat16
        uint16_t modifiedTime;
        uint16_t modifiedDate;
        uint16_t firstEntryLow;
        uint32_t sizeFileBytes;
    } __attribute__((packed));

    struct FatLFN
    {
        uint8_t nameIndex; //where we should place this in the full filename
        uint8_t wideChars[10]; //actually 5 w i d e chars.
        uint8_t attribs; //always 0x0F
        uint8_t entryType; //0 for names
        uint8_t checksum;
        uint8_t wideChars2[12];
        uint16_t reserved;
        uint8_t wideChars3[4];
    };
    
    Fat12ExtendedBootRecord* bootRecord;

    bool ChainLoadFile(void** start, void** end, size_t* size)
    {

    }
    
    void LoadInitDisk()
    {
        Log("Init disk start at: 0x", false);
        Log(sl::UIntToString((uint64_t)&_binary____kernel_disk_build_kdisk_img_start, BASE_HEX).Data());

        Log("Init disk end at: 0x", false);
        Log(sl::UIntToString((uint64_t)&_binary____kernel_disk_build_kdisk_img_end, BASE_HEX).Data());

        Log("Init disk length: 0x", false);
        Log(sl::UIntToString((uint64_t)&_binary____kernel_disk_build_kdisk_img_size, BASE_HEX).Data());

        bootRecord = reinterpret_cast<Fat12ExtendedBootRecord*>(&_binary____kernel_disk_build_kdisk_img_start);
        //total sectors in volume (including boot record)
        uint64_t totalSectors = bootRecord->logicalSectorCount == 0 ? bootRecord->largeSectorCount : bootRecord->logicalSectorCount;
        //size of root directory (unless fat32)
        uint64_t rootDirectorySize = ((bootRecord->directoriesCount * 32) + (bootRecord->bytesPerSector - 1)) / bootRecord->bytesPerSector;
        //fat size in bytes
        uint64_t fatSize = bootRecord->sectorsPerFat;
        //first data sector (i.e. first location stuff can be stored)
        uint64_t firstDataSector = bootRecord->reservedSectorsCount + (bootRecord->fatCount * bootRecord->sectorsPerFat) + rootDirectorySize;
        //first sector!
        uint64_t fatAddr = bootRecord->reservedSectorsCount;
        //total number of data sectors
        uint64_t dataSectorsCount = totalSectors - (bootRecord->reservedSectorsCount + (bootRecord->fatCount * fatSize) + rootDirectorySize);
        //total number of clusters
        uint64_t clustersCount = dataSectorsCount / bootRecord->sectorsPerCluster;

        //root directory is located directly after FAT.
        uint64_t rootDirAddr = firstDataSector - rootDirectorySize;
        
        /*  READING DIRS: (entries are 32bytes)
            1. If first byte is 0, this is the end of the chain. Read no more.
            2. If first byte is 0xE5, this entry is unused.
            3. check attribs, if LFN is present, parse that and load it, otherwise display short filename.
            4. check next cluster, to see if more directory entries are present.
        */

        /*  FAT TABLE:
            - Takes up exactly 1 sector?
            - offset can be obtained by multiplying active cluster by 1.5
            - sectorWithinFat = first fat sector + (offset / sectorSize)
            - entry offset = sectorWithinFat % section size
            - read data from sector "sectorWithinFat" into sector sized array (fatTableArray)
            - table_value = *(uint16_t*)&fatTableArray[entry offset]
            - if active cluster AND 0x1, right-shift tablevalue by 4 bts, otherwise bitwise AND it with 0x0FFF
            - tablevalue now points to the next cluster in the chain
            
            if tablevalue >= 0xFF8, no more clusters, if its ==0xFF7 then cluster is marked as bad. Otherwise it is next cluster.
        */
    }

    bool GetFileData(void** start, void** end, size_t* size)
    {
        *start = *end = size = nullptr;
        return false; //TODO:
    }
}