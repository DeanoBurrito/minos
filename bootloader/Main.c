#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include <stddef.h>
#include "BootInfo.h"
#include "PSF1.h"

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

    bInfo->gop.baseAddress = (void*)gop->Mode->FrameBufferBase;
    bInfo->gop.bufferSize = gop->Mode->FrameBufferSize;
    bInfo->gop.width = gop->Mode->Info->HorizontalResolution;
    bInfo->gop.height = gop->Mode->Info->VerticalResolution;
    bInfo->gop.pixelsPerScanline = gop->Mode->Info->PixelsPerScanLine;

    if (gop->Mode->Info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor)
        bInfo->gop.pixelFormat = PIXEL_FORMAT_RedGreenBlueReserved_8BPP;
    else if (gop->Mode->Info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor)
        bInfo->gop.pixelFormat = PIXEL_FORMAT_BlueGreenRedReserved_8BPP;
    else
        bInfo->gop.pixelFormat = PIXEL_FORMAT_Unknown;
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

PSF1_Font* load_font(EFI_FILE* directory, CHAR16* path, EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
    EFI_FILE* font = load_file(directory, path, imageHandle, systemTable);

    if (font == NULL)
        return NULL;

    PSF1_Header* header;
    uefi_call_wrapper(systemTable->BootServices->AllocatePool, 3, EfiLoaderData, sizeof(PSF1_Header), (void**)&header);
    UINTN size = sizeof(PSF1_Header);
    uefi_call_wrapper(font->Read, 3, font, &size, header);

    if (header->magic[0] != PSF1_Magic_0 || header->magic[1] != PSF1_Magic_1)
        return NULL;

    UINTN glyphBufferSize = header->charSize * 256;
    if (header->mode == 1)
    { //512 glyphs
        glyphBufferSize = header->charSize * 512;
    }

    void* glyphBuffer;
    {
        uefi_call_wrapper(font->SetPosition, 2, font, sizeof(PSF1_Header));
        uefi_call_wrapper(systemTable->BootServices->AllocatePool, 3, EfiLoaderData, glyphBufferSize, (void**)&glyphBuffer);
        uefi_call_wrapper(font->Read, 3, font, &glyphBufferSize, glyphBuffer);
    }

    PSF1_Font* fontObj;
    uefi_call_wrapper(systemTable->BootServices->AllocatePool, 3, EfiLoaderData, sizeof(PSF1_Font), (void**)&fontObj);
    fontObj->psf1_haeder = header;
    fontObj->glyphBuffer = glyphBuffer;

    return fontObj;
}

void init_memmap(BootInfo *bInfo)
{
    EFI_MEMORY_DESCRIPTOR *map = NULL;
    UINTN mapSize;
    UINTN mapKey;
    UINTN descriptorSize;
    UINT32 descriptorVersion;

    uefi_call_wrapper(ST->BootServices->GetMemoryMap, 5, &mapSize, map, &mapKey, &descriptorSize, &descriptorVersion);
    uefi_call_wrapper(ST->BootServices->AllocatePool, 3, EfiLoaderData, mapSize, (void**)&map);
    uefi_call_wrapper(ST->BootServices->GetMemoryMap, 5, &mapSize, map, &mapKey, &descriptorSize, &descriptorVersion);

    bInfo->memoryMap.descriptor = map;
    bInfo->memoryMap.descriptorSize = descriptorSize;
    bInfo->memoryMap.size = mapSize;
    bInfo->memoryMap.key = mapKey;
    bInfo->memoryMap.descriptorVersion = descriptorVersion;
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

EFI_STATUS EFIAPI efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
    InitializeLib(imageHandle, systemTable);
    Print(L"EFI bootloader initialized, mapping system table and loading kernel...\n\r");

    EFI_FILE* kernelFile = load_file(NULL, L"kernel-x86_64.elf", imageHandle, systemTable);
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

    EFI_FILE* assetsDirectory = load_file(NULL, L"assets", imageHandle, systemTable);

    PSF1_Font* font = load_font(assetsDirectory, L"zap-light16.psf", imageHandle, systemTable);
    if (font == NULL)
    {
        Print(L"Unable to load default kernel font file.");
        loop();
    }

    BootInfo bootInfo;
    bootInfo.font = font;

    init_gop(&bootInfo);
    Print(L"GOP: base=0x%x size=0x%x width=%d height=%d pps=%d format=%d\n\r", bootInfo.gop.baseAddress, bootInfo.gop.bufferSize, bootInfo.gop.width, bootInfo.gop.height, bootInfo.gop.pixelsPerScanline, bootInfo.gop.pixelFormat);

    init_memmap(&bootInfo);
    Print(L"\n\r");

    //VERY IMPORTANT: if we neglect this, UEFI will assume continued ownership of system, and we'll be in all sorts of chaos ;-;
    systemTable->BootServices->ExitBootServices(imageHandle, bootInfo.memoryMap.key);
    
    void (*KernelMain)(BootInfo*) = ((__attribute__((sysv_abi)) void (*)(BootInfo*) ) header.e_entry);
    KernelMain(&bootInfo);
    
    return EFI_SUCCESS;
}