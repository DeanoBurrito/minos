#include <efi.h>
#include <efilib.h>
#include <stddef.h>
#include <stdbool.h>
#include "../BootInfo.h"

//NOTE: this is the host's elf.h
#include <elf.h>

//thanks gcc... :eyeroll:
#define EFISTRING(str) (CHAR16*)(str)

int minos_memcmp(void* a, void* b, size_t count)
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

void minos_memset(void* addr, uint8_t value, size_t count)
{
    uint8_t* ptr = (uint8_t*)addr;
    for (size_t i = 0; i < count; i++)
        ptr[i] = value;
}

void minos_memcpy(void* destination, const void* source, size_t count)
{
    const uint8_t* src = (const uint8_t*)source;
    uint8_t* dest = (uint8_t*)destination;

    for (size_t i = 0; i < count; i++)
        dest[i] = src[i];
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

bool ElfHasRelocations(void* file)
{
    //check for presence of .rela.* sections
    Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)file;
    for (size_t i = 0; i < elfHeader->e_shnum; i++)
    {
        uint64_t shdrAddr = (uint64_t)file + elfHeader->e_shoff + (i * elfHeader->e_shentsize);
        Elf64_Shdr* section = (Elf64_Shdr*)shdrAddr;

        if (section->sh_type != SHT_RELA)
            continue; //not a useful section to us, ignore it

        //found at least 1 rela section. This heuristic could probably be improved, but it works for now.
        return true;
    }
    
    //found no .rela.* sections.
    return false;
}

void RelocateElfPhdr(void* elfFile, void* buffer, size_t bufferSize, uint64_t vaddr, uint64_t slide)
{
    Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)elfFile;
    for (size_t i = 0; i < elfHeader->e_shnum; i++)
    {
        uint64_t shdrAddr = (uint64_t)elfFile + elfHeader->e_shoff + (i * elfHeader->e_shentsize);
        Elf64_Shdr* section = (Elf64_Shdr*)shdrAddr;

        if (section->sh_type != SHT_RELA)
            continue;

        //process .rela.* header
        for (size_t offset = 0; offset < section->sh_size; offset += section->sh_entsize)
        {
            uint64_t relaAddr = (uint64_t)elfFile + section->sh_offset + (i * section->sh_entsize);
            Elf64_Rela* rela = (Elf64_Rela*)relaAddr;

            switch (rela->r_info)
            {
            case R_X86_64_RELATIVE:
                {
                    //check relocation applies to this phdr
                    if (rela->r_offset < vaddr)
                        continue;
                    if (rela->r_offset + 8 > vaddr + bufferSize)
                        continue;

                    //here we go, construct a pointer and apply the relocation
                    //big thanks to the authors of limine bootloader, for helping debug this
                    uint64_t* ptr = (uint64_t*)((uint8_t*)buffer - vaddr + rela->r_offset);
                    *ptr = slide + rela->r_addend;

                    break;
                }

            default:
                {
                    Print(EFISTRING(L"type is: %u"), rela->r_info);
                    FatalError(EFISTRING(L"Kernel elf relocation failed, encountered unknown .rela entry type."));
                    break;
                }
            }
        }
    }
}

UINTN LoadKernel(EFI_FILE* kernelFile, BootInfo* bootInfo, uint64_t slide)
{
    /* Process outline:
        -load entire file into memory
        -validate elf header is what we want
        -work out where we want to actually load the kernel for when we start running it's code
        -copy phdrs into this area
        -apply relocations from shdrs as we go (if available)
        -return entry point
    */

    //get kernel file info
    EFI_FILE_INFO* fileInfo;
    UINTN efiInfoSize;
    uefi_call_wrapper(kernelFile->GetInfo, 4, kernelFile, &gEfiFileInfoGuid, &efiInfoSize, NULL);
    uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, efiInfoSize, (void**)&fileInfo);
    uefi_call_wrapper(kernelFile->GetInfo, 4, kernelFile, &gEfiFileInfoGuid, &efiInfoSize, (void**)fileInfo);

    //allocate buffer to hold working copy of kernel file
    void* loadedFile = NULL;
    UINTN fileSize = fileInfo->FileSize;
    uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, fileSize, (void**)&loadedFile);
    uefi_call_wrapper(kernelFile->SetPosition, 2, kernelFile, 0); //not really necessary, but it helps me sleep at night
    uefi_call_wrapper(kernelFile->Read, 3, kernelFile, &fileSize, loadedFile);

    if (loadedFile == NULL)
        FatalError(EFISTRING(L"Could not allocate buffer to load kernel into."));
    else
        Print(EFISTRING(L"Loaded kernel elf, %u bytes.\n\r"), fileSize);

    //verify elf header details
    Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)loadedFile;
    if (minos_memcmp(&elfHeader->e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0)
        FatalError(EFISTRING(L"Kernel elf header magic invalid.\n\r"));
    if (elfHeader->e_ident[EI_CLASS] != ELFCLASS64)
        FatalError(EFISTRING(L"Kernel elf not 64-bit.\n\r"));
    if (elfHeader->e_ident[EI_DATA] != ELFDATA2LSB)
        FatalError(EFISTRING(L"Kernel elf has wrong endianness.\n\r"));
    if (elfHeader->e_type != ET_EXEC)
        FatalError(EFISTRING(L"Kernel elf is not marked as executable (hahaha what, how?).\n\r"));
    if (elfHeader->e_machine != EM_X86_64)
        FatalError(EFISTRING(L"Kernel elf is not compiled for x86_64.\n\r"));
    if (elfHeader->e_version != EV_CURRENT)
        FatalError(EFISTRING(L"Kernel elf has incorrect elf version.\n\r"));

    Print(EFISTRING(L"Kernel elf header parsed, looks good.\n\r"));

    bool doRelocations = false;
    if (ElfHasRelocations(loadedFile))
    {
        if (slide != 0)
        {
            doRelocations = true;
            Print(EFISTRING(L"Kernel relocations enabled, offset=0x%x.\n\r"), slide);
        }
        else
            Print(EFISTRING(L"Kernel has relocation info, but offset is 0.\n\r"));
    }
    else
        Print(EFISTRING(L"Kernel has no relocation info.\n\r"));

    uint64_t kernelLowest = (uint64_t)-1;
    uint64_t kernelHighest = 0;

    //parse program headers, loading them where we desire, and applying relocations as well
    for (size_t i = 0; i < elfHeader->e_phnum; i++)
    {
        uint64_t phdrAddr = (uint64_t)loadedFile + elfHeader->e_phoff + (i * elfHeader->e_phentsize);
        Elf64_Phdr* phdr = (Elf64_Phdr*)phdrAddr;

        if (phdr->p_type != PT_LOAD)
            continue;

        size_t numPagesRequired = (phdr->p_memsz + 0xFFF) / 0x1000;
        uint64_t phdrLoadDest = phdr->p_vaddr + (doRelocations ? slide : 0);

        //allocate memory and load program data
        uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderData, numPagesRequired, &phdrLoadDest);
        minos_memcpy((void*)phdrLoadDest, (void*)((uint64_t)loadedFile + phdr->p_offset), phdr->p_filesz);

        //TODO: zero any any data in phdr that isnt loaded from file (.bss needs it)
        
        //keep track of extremities of where kernel occupies
        if (phdrLoadDest < kernelLowest)
            kernelLowest = phdrLoadDest;
        if (phdrLoadDest + (numPagesRequired * 0x1000) > kernelHighest)
            kernelHighest = phdrLoadDest + (numPagesRequired * 0x1000);

        if (doRelocations)
            RelocateElfPhdr(loadedFile, (void*)phdrLoadDest, numPagesRequired * 0x1000, phdr->p_vaddr, slide);
    }

    //get the entry address (while we can access it), and then free our working file buffer
    UINTN entryAddr = elfHeader->e_entry + (doRelocations ? slide : 0);
    uefi_call_wrapper(BS->FreePool, 1, loadedFile);

    //stash necessary info in bootinfo, and return entry address
    bootInfo->kernelStartAddr = kernelLowest;
    bootInfo->kernelSize = kernelHighest - kernelLowest;

    Print(EFISTRING(L"Successfully loaded kernel, entry 0x%x\n\r"), entryAddr);
    return entryAddr;
}

void CollectAcpiInfo(BootInfo* bootInfo)
{
    EFI_GUID acpiTableId = ACPI_20_TABLE_GUID; 
    char* rsdptrStr = (char*)"RSD PTR ";

    EFI_CONFIGURATION_TABLE* efiConfigTable = ST->ConfigurationTable;
    for (size_t i = 0; i < ST->NumberOfTableEntries; i++)
    {
        if (CompareGuid(&efiConfigTable->VendorGuid, &acpiTableId) == 0)
        {
            if (minos_memcmp(rsdptrStr, (char*)efiConfigTable->VendorTable, 8) == 0)
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
        case EfiBootServicesData:
        case EfiBootServicesCode:
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
    Print(EFISTRING(L"---- Boot Info ----\n\r"));
    
    Print(EFISTRING(L"Bootloader ID: %x\n\r"), bootInfo->bootloaderId);
    Print(EFISTRING(L"Kernel: start=0x%x, len=0x%x\n\r"), bootInfo->kernelStartAddr, bootInfo->kernelSize);
    Print(EFISTRING(L"Rsdp: 0x%x\n\r"), bootInfo->rsdp);

    Print(EFISTRING(L"Memory descriptors: \n\r"));
    MemoryRegionDescriptor* desc = bootInfo->memoryDescriptors;
    for (size_t i = 0; i < bootInfo->memoryDescriptorsCount; i++)
    {
        Print(EFISTRING(L"[%u] phys=0x%x, virt=0x%x, pages=0x%x, free=%x\n\r"), 
            i, desc->physicalStart, desc->virtualStart, desc->numberOfPages, desc->flags.free);
        desc++;
    }

    Print(EFISTRING(L"Framebuffer: w=%u, h=%u, base=0x%x, stride=%u, format=0x%x\n\r"), 
        bootInfo->framebuffer.width, 
        bootInfo->framebuffer.height, 
        bootInfo->framebuffer.base, 
        bootInfo->framebuffer.pixelFormat);

    Print(EFISTRING(L"--- END ----\n\r"));
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
    InitializeLib(imageHandle, systemTable);
    Print(EFISTRING(L"Minos UEFI pre-kernel initialized.\n\r"));
    Print(EFISTRING(L"Mapping required info for kernel boot.\n\r"));

    BootInfo bootInfo;
    bootInfo.bootloaderId = BOOTLOADER_ID_UEFI;

    EFI_FILE* kernelFile = LoadFile(NULL, EFISTRING(L"kernel.elf"), imageHandle);
    if (kernelFile == NULL)
        FatalError(EFISTRING(L"Could find kernel.elf on efi image. Aborting init."));
    
    UINTN kernelEntryAddress = LoadKernel(kernelFile, &bootInfo, 0);
    CollectAcpiInfo(&bootInfo);
    CollectGopInfo(&bootInfo);
    UINTN memoryMapKey = CollectMemmapInfo(&bootInfo);

    //Commenting this out for most builds, as it can be super slow to print all the boot info
    //PrintBootInfo(&bootInfo);

    //super important, otherwise we'll get killed by uefi watchdog after 5 minutes
    Print(EFISTRING(L"Exiting boot services, then jumping to kernel code.\n\r"));
    BS->ExitBootServices(imageHandle, memoryMapKey);

    //now we're really in the wilds.
    void (*KernelMain)(BootInfo*) = ((__attribute__((sysv_abi)) void(*)(BootInfo*)) kernelEntryAddress);
    KernelMain(&bootInfo);

    __builtin_unreachable();
}
