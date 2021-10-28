#include <efi.h>
#include <efilib.h>
#include <stddef.h>
#include <stdbool.h>
#include "../BootInfo.h"

//NOTE: this is the host's elf.h
#include <elf.h>

//thanks gcc... :eyeroll:
#define EFISTRING(str) (CHAR16*)(str)

int memcmp2(void* a, void* b, size_t count)
{
    uint8_t* ax = (uint8_t*)a;
    uint8_t* bx = (uint8_t*)b;
    for (size_t i = 0; i < count; i++)
    {
        if (ax[i] < bx[i])
            return -1;
        if (ax[i] > bx[i])
            return 1;
    }

    return 0;
}

void FatalError(const CHAR16* reason)
{
    Print(EFISTRING(L"---- FATAL ERROR: ----\n\r"));
    Print(reason);
    Print(EFISTRING(L"\n\rAborting init.\n\r"));

    while (1)
        asm volatile("hlt");
    
    __builtin_unreachable();
}

EFI_FILE* LoadFile(EFI_FILE* parentDir, const CHAR16* path, EFI_HANDLE image)
{
    EFI_FILE* file = NULL;
    EFI_LOADED_IMAGE_PROTOCOL* loadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fsProtocol;

    uefi_call_wrapper(BS->HandleProtocol, 3, image, &gEfiLoadedImageProtocolGuid, (void**)&loadedImage);
    uefi_call_wrapper(BS->HandleProtocol, 3, loadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&fsProtocol);

    if (!parentDir)
        uefi_call_wrapper(fsProtocol->OpenVolume, 2, fsProtocol, &parentDir);

    EFI_STATUS status = uefi_call_wrapper(parentDir->Open, 5, parentDir, &file, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
    if (status != EFI_SUCCESS)
        Print(EFISTRING(L"Could not open file: %s\n\r"), path);
    else
        Print(EFISTRING(L"File loaded: %s\n\r"), path);
    
    return file;
}

UINTN LoadKernel(EFI_FILE* kernelFile, BootInfo* bootInfo)
{
    Elf64_Ehdr elfHeader;
    UINTN efiInfoSize;
    EFI_FILE_INFO* fileInfo;
    UINTN elfHeaderSize = sizeof(Elf64_Ehdr);

    //read elf header info memory
    uefi_call_wrapper(kernelFile->GetInfo, 4, kernelFile, &gEfiFileInfoGuid, &efiInfoSize, NULL);
    uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, efiInfoSize, (void**)&fileInfo);
    uefi_call_wrapper(kernelFile->GetInfo, 4, kernelFile, &gEfiFileInfoGuid, &efiInfoSize, (void**)fileInfo);
    uefi_call_wrapper(kernelFile->Read, 3, kernelFile, &elfHeaderSize, &elfHeader);

    //check elf magic values match
    if (memcmp2(&elfHeader.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0)
        FatalError(EFISTRING(L"Kernel elf header magic invalid."));
    if (elfHeader.e_ident[EI_CLASS] != ELFCLASS64)
        FatalError(EFISTRING(L"Kernel elf not 64-bit."));
    if (elfHeader.e_ident[EI_DATA] != ELFDATA2LSB)
        FatalError(EFISTRING(L"Kernel elf has wrong endianness."));
    if (elfHeader.e_type != ET_EXEC)
        FatalError(EFISTRING(L"Kernel elf is not marked as executable (hahaha what, how?)."));
    if (elfHeader.e_machine != EM_X86_64)
        FatalError(EFISTRING(L"Kernel elf is not compiled for x86_64."));
    if (elfHeader.e_version != EV_CURRENT)
        FatalError(EFISTRING(L"Kernel elf has incorrect elf version."));

    Print(EFISTRING(L"Kernel elf header parsed, looks good.\n\r"));
    
    Elf64_Phdr* phdrs;
    UINTN phdrsSize = elfHeader.e_phnum * elfHeader.e_phentsize;
    uefi_call_wrapper(kernelFile->SetPosition, 2, kernelFile, elfHeader.e_phoff);

    uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, phdrsSize, (void**)&phdrs);
    uefi_call_wrapper(kernelFile->Read, 3, kernelFile, &phdrsSize, phdrs);

#define NEXT_PHDR (Elf64_Phdr*)((uint64_t)currentPhdr + elfHeader.e_phentsize)

    //lots of pointer and back conversions: basically we just want to touch each phdr
    Elf64_Phdr* phdrsEnd = (Elf64_Phdr*)((uint64_t)phdrs + phdrsSize);
    uint64_t kernelLowest = 0xFFFFFFFFFFFFFFFF;
    uint64_t kernelHighest = 0;
    for (Elf64_Phdr* currentPhdr = phdrs; (uint64_t)currentPhdr < (uint64_t)phdrsEnd; currentPhdr = NEXT_PHDR)
    {
        if (currentPhdr->p_type != PT_LOAD)
            continue;

        //TODO: add an address slide, so we can load the kernel at any address easily (higher half?)
        size_t numPagesRequired = (currentPhdr->p_memsz + 0x1000 - 1) / 0x1000;
        Elf64_Addr phdrMemoryDest = currentPhdr->p_paddr;
        UINTN phdrSize = currentPhdr->p_filesz;

        uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderData, numPagesRequired, &phdrMemoryDest);
        uefi_call_wrapper(kernelFile->SetPosition, 2, kernelFile, currentPhdr->p_offset);
        uefi_call_wrapper(kernelFile->Read, 3, kernelFile, &phdrSize, (void*)phdrMemoryDest);

        //update lowest and highest address used by the kernel (this assumes we load without gaps)
        if (phdrMemoryDest < kernelLowest)
            kernelLowest = phdrMemoryDest;
        if (phdrMemoryDest + (numPagesRequired * 0x1000) > kernelHighest)
            kernelHighest = phdrMemoryDest + (numPagesRequired * 0x1000);
    }

    //set the actual start and length values
    bootInfo->kernelStartAddr = kernelLowest;
    bootInfo->kernelSize = kernelHighest - kernelLowest;

    Print(EFISTRING(L"Successfully parsed and loaded kernel elf.\n\r"));
    return elfHeader.e_entry + bootInfo->kernelStartAddr; //offset the entry based on where the kernel binary ends up in memory
}

void CollectAcpiInfo(BootInfo* bootInfo)
{
    EFI_GUID acpiTableId = ACPI_20_TABLE_GUID; 
    char* rsdptrStr = (char*)"RSD PTR ";

    EFI_CONFIGURATION_TABLE* efiConfigTable = ST->ConfigurationTable;
    for (size_t i = 0; i < ST->NumberOfTableEntries; i++)
    {
        if (CompareGuid(&efiConfigTable->VendorGuid, &acpiTableId))
        {
            if (memcmp2(rsdptrStr, (char*)efiConfigTable->VendorTable, 8))
            {
                Print(EFISTRING(L"Found ACPI 2.0+ table with matching RSD_PTR_. Stashing value.\n\r"));
                bootInfo->rsdp = (NativePtr)efiConfigTable->VendorTable;
                return;
            }
            else
                Print(EFISTRING(L"Found ACPI 2.0+ table, but RSD_PTR_ is missing. No beuno.\n\r"));
        }

        efiConfigTable++;
    }

    FatalError(EFISTRING(L"Could not find ACPI 2.0+ tables.\n\r"));
}

void CollectGopInfo(BootInfo* bootInfo)
{
    EFI_GUID gopId = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;

    EFI_STATUS status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopId, NULL, (void**)&gop);
    if (status != EFI_SUCCESS)
        FatalError(EFISTRING(L"Could not get GOP framebuffer."));

    bootInfo->framebuffer.base = gop->Mode->FrameBufferBase;
    bootInfo->framebuffer.bufferSize = gop->Mode->FrameBufferSize;
    bootInfo->framebuffer.width = gop->Mode->Info->HorizontalResolution;
    bootInfo->framebuffer.height = gop->Mode->Info->VerticalResolution;
    bootInfo->framebuffer.stride = gop->Mode->Info->PixelsPerScanLine;

    switch (gop->Mode->Info->PixelFormat)
    {
    case PixelRedGreenBlueReserved8BitPerColor:
        bootInfo->framebuffer.pixelFormat = PIXEL_FORMAT_RedGreenBlueReserved_8BPP;
        break;
    case PixelBlueGreenRedReserved8BitPerColor:
        bootInfo->framebuffer.pixelFormat = PIXEL_FORMAT_BlueGreenRedReserved_8BPP;
        break;

    default:
        FatalError(EFISTRING(L"Unable to determine GOP framebuffer pixel format."));
        break;
    }

    Print(EFISTRING(L"Stashed framebuffer info.\n\r"));
}

UINTN CollectMemmapInfo(BootInfo* bootInfo)
{
    EFI_MEMORY_DESCRIPTOR* memMap = NULL;
    UINTN mapSize, mapKey, descriptorSize, descriptorVersion;

    //populate above fields
    uefi_call_wrapper(BS->GetMemoryMap, 5, &mapSize, memMap, &mapKey, &descriptorSize, &descriptorVersion);
    uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, mapSize, (void**)&memMap);
    uefi_call_wrapper(BS->GetMemoryMap, 5, &mapSize, memMap, &mapKey, &descriptorSize, &descriptorVersion);

    //allocate a buffer for us to store our own version of the memory map
    bootInfo->memoryDescriptorsCount = mapSize / descriptorSize;
    uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, bootInfo->memoryDescriptorsCount * sizeof(MemoryRegionDescriptor), (void**)&bootInfo->memoryDescriptors);
    
    MemoryRegionDescriptor* desc = bootInfo->memoryDescriptors;
    for (size_t i = 0; i < bootInfo->memoryDescriptorsCount; i++)
    {
        desc->physicalStart = memMap->PhysicalStart;
        desc->virtualStart = memMap->VirtualStart;
        desc->numberOfPages = memMap->NumberOfPages;

        switch (memMap->Type)
        {
        case EfiConventionalMemory: //the good stuff, usable memory
            desc->flags.free = true;
            desc->flags.mustMap = false;
            break;

        case EfiLoaderData: //not free, but we should map these
        case EfiLoaderCode:
            desc->flags.free = false;
            desc->flags.mustMap = true;
            break;

        default:
            desc->flags.raw = 0; //dont do anything with it. We dont care what it is.
        }

        //update for next cycle
        desc++;
        memMap = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)memMap + descriptorSize);
    }

    Print(EFISTRING(L"Successfully parsed memory map.\n\r"));
    return mapKey;
}

void PrintBootInfo(BootInfo* bootInfo)
{
    
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
    InitializeLib(imageHandle, systemTable);
    Print(EFISTRING(L"Minos UEFI pre-kernel initialized.\n\r"));
    Print(EFISTRING(L"Mapping required info for kernel boot.\n\r"));

    BootInfo bootInfo;
    EFI_FILE* kernelFile = LoadFile(NULL, EFISTRING(L"kernel.elf"), imageHandle);
    if (kernelFile == NULL)
        FatalError(EFISTRING(L"Could find kernel.elf on efi image. Aborting init."));
    
    UINTN kernelEntryAddress = LoadKernel(kernelFile, &bootInfo);
    CollectAcpiInfo(&bootInfo);
    CollectGopInfo(&bootInfo);
    UINTN memoryMapKey = CollectMemmapInfo(&bootInfo);

    PrintBootInfo(&bootInfo);

    //super important, otherwise we'll get killed by uefi watchdog after 5 minutes
    Print(EFISTRING(L"Exiting boot services, then jumping to kernel code.\n\r"));
    BS->ExitBootServices(imageHandle, memoryMapKey);

    //now we're really in the wilds.
    void (*KernelMain)(BootInfo*) = ((__attribute__((sysv_abi)) void(*)(BootInfo*)) kernelEntryAddress);
    KernelMain(&bootInfo);

    __builtin_unreachable();
}
