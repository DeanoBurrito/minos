#pragma once

#define PSF1_Magic_0 0x36
#define PSF1_Magic_1 0x04

typedef struct
{
    unsigned char magic[2];
    unsigned char mode;
    unsigned char charSize;
} PSF1_Header;

typedef struct 
{
    PSF1_Header* psf1_haeder;
    void* glyphBuffer;
} PSF1_Font;