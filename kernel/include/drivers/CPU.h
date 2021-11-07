#pragma once

#include <stdint.h>

#define PORT_USUALLY_EMPTY 0x80

//TODO: this relies on our interrupt defines never changing. A more dynamic way to generate this would be nice.
#define ISSUE_INTERRUPT_SCHEDULER_YIELD asm volatile("int $0x30");

namespace Kernel::Drivers
{
    //These flags are platform independant, they get translated to platform-specific offsets in CPU.cpp for that platform.
    enum class CpuFeatureFlag : uint32_t
    {
        SSE3,
        PCLMUL,
        DTES64,
        MONITOR,
        DS_CPL,
        VMX,
        SMX,
        EST,
        TM2,
        SSSE3,
        CID,
        FMA,
        CX16,
        ETPRD,
        PDCM,
        PCIDE,
        DCA,
        SSE4_1,
        SSE4_2,
        x2APIC,
        MOVBE,
        POPCNT,
        AES,
        XSAVE,
        OSXSAVE,
        AVX,
        FPU,
        VME,
        DE,
        PSE,
        TSC,
        MSR,
        PAE,
        MCE,
        CX8,
        APIC,
        SEP,
        MTRR,
        PGE,
        MCA,
        CMOV,
        PAT,
        PSE36,
        PSN,
        CLF,
        NX,
        DTES,
        ACPI,
        MMX,
        FXSR,
        SSE,
        SSE2,
        SS,
        HTT,
        TM1,
        IA64,
        PBE,

        Count,
    };

    enum class CpuTable : uint8_t
    {
        x86_64_GDT,
        X86_64_IDT,
    };
    
    class CPU
    {
    public:
        static void Init();
 
        static void EnableInterrupts();
        static void DisableInterrupts();
        static bool InterruptsEnabled();

        static bool FeatureSupported(CpuFeatureFlag flag);

        static void LoadPageTableMap(void* toplevelAddress);
        static void InvalidatePageTable(void* tableAddress);
        static void LoadTable(CpuTable table, void* addr);
        static void Halt();

        static void WriteMSR(uint32_t id, uint64_t value);
        static uint64_t ReadMSR(uint32_t);

        static void PortWrite8(uint16_t port, uint8_t data);
        static void PortWrite16(uint16_t port, uint16_t data);
        static void PortWrite32(uint16_t port, uint32_t data);

        static uint8_t PortRead8(uint16_t port);
        static uint16_t PortRead16(uint16_t port);
        static uint32_t PortRead32(uint16_t port);

        static void PortIOWait(); //waits until IO bus has completed any outstanding operations.

        static const char* GetArchitectureName();
        static const char* GetVendorName();

        static const char* GetFeatureShortName(CpuFeatureFlag flag);
        static const char* GetFeatureLongName(CpuFeatureFlag flag);
    };
}
