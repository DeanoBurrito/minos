#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include <stddef.h>
#include "../BootInfo.h"

void init_gop(BootInfo* bInfo)
{
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
    EFI_STATUS status;

    status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
    if (EFI_ERROR(status))
    {
        Print(L"Unable to locate GOP protocol!\n\r");
        return;
    }
    else
        Print(L"GOP located, stashing info...\n\r");

    bInfo->framebuffer.base = gop->Mode->FrameBufferBase;
    bInfo->framebuffer.bufferSize = gop->Mode->FrameBufferSize;
    bInfo->framebuffer.width = gop->Mode->Info->HorizontalResolution;
    bInfo->framebuffer.height = gop->Mode->Info->VerticalResolution;
    bInfo->framebuffer.stride = gop->Mode->Info->PixelsPerScanLine;

    switch (gop->Mode->Info->PixelFormat)
    {
    case PixelRedGreenBlueReserved8BitPerColor:
        bInfo->framebuffer.pixelFormat = PIXEL_FORMAT_RedGreenBlueReserved_8BPP;
        break;
    case PixelBlueGreenRedReserved8BitPerColor:
        bInfo->framebuffer.pixelFormat = PIXEL_FORMAT_BlueGreenRedReserved_8BPP;
        break;
    default:
        bInfo->framebuffer.pixelFormat = PIXEL_FORMAT_Unknown;
        break;
    }
}

void loop()
{
    EFI_INPUT_KEY key;
    EFI_STATUS status;
    while ((status = ST->ConIn->ReadKeyStroke(ST->ConIn, &key)) == EFI_NOT_READY);
}

EFI_FILE* load_file(EFI_FILE* directory, CHAR16* path, EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
    EFI_FILE* loadedFile;

    EFI_LOADED_IMAGE_PROTOCOL* loadedImage;
    uefi_call_wrapper(systemTable->BootServices->HandleProtocol, 3, imageHandle, &gEfiLoadedImageProtocolGuid, (void**)&loadedImage);

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fileSystem;
    uefi_call_wrapper(systemTable->BootServices->HandleProtocol, 3, loadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&fileSystem);

    if (directory == NULL)
    {
        uefi_call_wrapper(fileSystem->OpenVolume, 2, fileSystem, &directory);
    }
    
    EFI_STATUS status = uefi_call_wrapper(directory->Open, 5, directory, &loadedFile, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
    if (status != EFI_SUCCESS)
        return NULL;

    return loadedFile;
}

// PSF1_Font* load_font(EFI_FILE* directory, CHAR16* path, EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
// {
//     //TODO: obsolete, extract functionality into new psf1 driver and remove this code.
//     EFI_FILE* font = load_file(directory, path, imageHandle, systemTable);

//     if (font == NULL)
//         return NULL;

//     PSF1_Header* header;
//     uefi_call_wrapper(systemTable->BootServices->AllocatePool, 3, EfiLoaderData, sizeof(PSF1_Header), (void**)&header);
//     UINTN size = sizeof(PSF1_Header);
//     uefi_call_wrapper(font->Read, 3, font, &size, header);

//     if (header->magic[0] != PSF1_Magic_0 || header->magic[1] != PSF1_Magic_1)
//         return NULL;

//     UINTN glyphBufferSize = header->charSize * 256;
//     if (header->mode == 1)
//     { //512 glyphs
//         glyphBufferSize = header->charSize * 512;
//     }

//     void* glyphBuffer;
//     {
//         uefi_call_wrapper(font->SetPosition, 2, font, sizeof(PSF1_Header));
//         uefi_call_wrapper(systemTable->BootServices->AllocatePool, 3, EfiLoaderData, glyphBufferSize, (void**)&glyphBuffer);
//         uefi_call_wrapper(font->Read, 3, font, &glyphBufferSize, glyphBuffer);
//     }

//     PSF1_Font* fontObj;
//     uefi_call_wrapper(systemTable->BootServices->AllocatePool, 3, EfiLoaderData, sizeof(PSF1_Font), (void**)&fontObj);
//     fontObj->psf1_haeder = header;
//     fontObj->glyphBuffer = glyphBuffer;

//     return fontObj;
// }

UINTN init_memmap(BootInfo *bInfo)
{
    EFI_MEMORY_DESCRIPTOR *map = NULL;
    UINTN mapSize;
    UINTN mapKey;
    UINTN descriptorSize;
    UINT32 descriptorVersion;

    uefi_call_wrapper(ST->BootServices->GetMemoryMap, 5, &mapSize, map, &mapKey, &descriptorSize, &descriptorVersion);
    uefi_call_wrapper(ST->BootServices->AllocatePool, 3, EfiLoaderData, mapSize, (void**)&map);
    uefi_call_wrapper(ST->BootServices->GetMemoryMap, 5, &mapSize, map, &mapKey, &descriptorSize, &descriptorVersion);

    //marshal descriptors into boot info
    bInfo->memoryDescriptorsCount = mapSize / descriptorSize;
    uefi_call_wrapper(ST->BootServices->AllocatePool, 3, EfiLoaderData, bInfo->memoryDescriptorsCount * sizeof(MemoryRegionDescriptor), (void**)&bInfo->memoryDescriptors);
    MemoryRegionDescriptor* descriptor = bInfo->memoryDescriptors;

    for (size_t i  = 0; i < bInfo->memoryDescriptorsCount; i++)
    {
        descriptor->physicalStart = map->PhysicalStart;
        descriptor->virtualStart = map->VirtualStart;
        descriptor->numberOfPages = map->NumberOfPages;
        
        switch (map->Type)
        {
        case EfiConventionalMemory: //usable RAM
            descriptor->flags.free = 1;
            descriptor->flags.mustMap = 0;
            break;
            
        case EfiLoaderData: //already in use, but we should map it anyway
        case EfiLoaderCode: //(current program lives in these two)
            descriptor->flags.free = 0;
            descriptor->flags.mustMap = 1;
            break;

        case EfiACPIReclaimMemory:
        case EfiACPIMemoryNVS: //we'll need ACPI anyway, so may as well have them mapped up front
            descriptor->flags.free = 0;
            descriptor->flags.mustMap = 1;
            break;

        default: //anything else has default flags (not free and no need to map)
            descriptor->flags.raw = 0;
            break;
        }
        
        descriptor++;
        map = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)map + descriptorSize);
    }

    return mapKey;
}

int memcmp(const void* aPtr, const void* bPtr, size_t n)
{
    const unsigned char *a = aPtr, *b = bPtr;
    for (size_t i = 0; i < n; i++)
    {
        if (a[i] < b[i])
            return -1;
        if (a[i] > b[i])
            return 1;
    }
    
    return 0;
}

int strcmp(CHAR8* a, CHAR8* b, UINTN len)
{
    for (UINTN i = 0; i < len; i++)
    {
        if (a[0] != b[0])
            return 0;
    }
    return 1;
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
    InitializeLib(imageHandle, systemTable);
    Print(L"EFI bootloader initialized, mapping system table and loading kernel...\n\r");

    EFI_FILE* kernelFile = load_file(NULL, L"kernel.elf", imageHandle, systemTable);
    if (kernelFile == NULL)
    {
        Print(L"Unable to load kernel file!\n\r");
        loop();
    }
    else
        Print(L"Kernel file handle located, loading...\n\r");
    
    //read elf64 header
    Elf64_Ehdr header;
    {
        UINTN fileInfoSize;
        EFI_FILE_INFO* fileInfo;

        uefi_call_wrapper(kernelFile->GetInfo, 4, kernelFile, &gEfiFileInfoGuid, &fileInfoSize, NULL);
        uefi_call_wrapper(systemTable->BootServices->AllocatePool, 3, EfiLoaderData, fileInfoSize, (void**)&fileInfo);
        uefi_call_wrapper(kernelFile->GetInfo, 4, kernelFile, &gEfiFileInfoGuid, &fileInfoSize, (void**)fileInfo);

        UINTN size = sizeof(header);
        uefi_call_wrapper(kernelFile->Read, 3, kernelFile, &size, &header);
    }

    if (memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 
        || header.e_ident[EI_CLASS] != ELFCLASS64
        || header.e_ident[EI_DATA] != ELFDATA2LSB
        || header.e_type != ET_EXEC
        || header.e_machine != EM_X86_64
        || header.e_version != EV_CURRENT)
        {
            Print(L"Bad kernel format. Elf header is malformed.\n\r");
            loop();
        }
    else
        Print(L"Kernel header processed, verified.\n\r");
    
    Elf64_Phdr* phdrs;
    {
        uefi_call_wrapper(kernelFile->SetPosition, 2, kernelFile, header.e_phoff);

        UINTN size = header.e_phnum * header.e_phentsize;
        uefi_call_wrapper(systemTable->BootServices->AllocatePool, 3, EfiLoaderData, size, (void**)&phdrs);
        uefi_call_wrapper(kernelFile->Read, 3, kernelFile, &size, phdrs);
    }

    for (Elf64_Phdr* phdr = phdrs; (char*)phdr < (char*)phdrs + header.e_phnum * header.e_phentsize; phdr = (Elf64_Phdr*)((char*)phdr + header.e_phentsize))
    {
        switch (phdr->p_type)
        {
            case PT_LOAD:
            {
                int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
                Elf64_Addr segment = phdr->p_paddr;
                uefi_call_wrapper(systemTable->BootServices->AllocatePages, 4, AllocateAddress, EfiLoaderData, pages, &segment);

                uefi_call_wrapper(kernelFile->SetPosition, 2, kernelFile, phdr->p_offset);
                UINTN size = phdr->p_filesz;
                uefi_call_wrapper(kernelFile->Read, 3, kernelFile, &size, (void*)segment);
                break;
            }
        }
    }

    //Get RSDP so we can use ACPI and other goodies in the kernel
    EFI_CONFIGURATION_TABLE* configTable = systemTable->ConfigurationTable;
    void* rsdp = NULL;
    EFI_GUID acpiTableGuid = ACPI_20_TABLE_GUID;
    for (UINTN index = 0; index < systemTable->NumberOfTableEntries; index++)
    {
        if (CompareGuid(&configTable[index].VendorGuid, &acpiTableGuid))
        {
            if (strcmp((CHAR8*)"RSD PTR ", (CHAR8*)configTable->VendorTable, 8))
            {
                rsdp = (uint64_t)configTable->VendorTable;
                Print(L"RSDP: 0x%x\r\n", rsdp);
                break;
            }
        }

        configTable++;
    }

    BootInfo bootInfo;
    bootInfo.rsdp = rsdp;

    init_gop(&bootInfo);
    Print(L"GOP: base=0x%x size=0x%x width=%d height=%d pps=%d format=%d\n\r", bootInfo.framebuffer.base, bootInfo.framebuffer.bufferSize, bootInfo.framebuffer.width, bootInfo.framebuffer.height, bootInfo.framebuffer.stride, bootInfo.framebuffer.pixelFormat);

    UINTN memoryMapKey = init_memmap(&bootInfo);
    Print(L"\n\r");

    //VERY IMPORTANT: if we neglect this, UEFI will assume continued ownership of system, and we'll be in all sorts of chaos ;-;
    systemTable->BootServices->ExitBootServices(imageHandle, memoryMapKey);
    
    void (*KernelMain)(BootInfo*) = ((__attribute__((sysv_abi)) void (*)(BootInfo*) ) header.e_entry);
    KernelMain(&bootInfo);
    
    return EFI_SUCCESS;
}