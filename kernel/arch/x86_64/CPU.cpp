#include <cpuid.h>
#include <drivers/CPU.h>
#include <Platform.h>
#include <KLog.h>

PLATFORM_REQUIRED(MINOS_PLATFORM_X86_64)

namespace Kernel::Drivers
{   
    //this just stops us clogging the kernel namespace
    namespace CPU_x86_64
    {
        char cpuIdVendorStr[13];
        bool cpuIdSupported;
        uint32_t cpuId_leaf1_ecx = 0;
        uint32_t cpuId_leaf1_edx = 0;

        char cpuArch[] = "x86_64\0";
    }
    using namespace CPU_x86_64;
    
    void CPU::Init()
    {
        //perform cpu id, and cache results
        uint64_t higheshIdSupported = __get_cpuid_max(0x80000000, (unsigned int*)cpuIdVendorStr);
        if (higheshIdSupported == 0)
        {
            cpuIdSupported = false;
            Log("CPUID not supported!");
        }
        else
            cpuIdSupported = true;


        if (cpuIdSupported)
        {
            uint32_t eax = 0;
            uint32_t ebx = 0;
            uint32_t ecx = 0;
            uint32_t edx = 0;

            //leaf 0, cpu vendor name
            __get_cpuid(0, &eax, &ebx, &ecx, &edx);
            cpuIdVendorStr[0] = (ebx & 0x00'00'00'ff);
            cpuIdVendorStr[1] = (ebx & 0x00'00'ff'00) >> 8;
            cpuIdVendorStr[2] = (ebx & 0x00'ff'00'00) >> 16;
            cpuIdVendorStr[3] = (ebx & 0xff'00'00'00) >> 24;
            cpuIdVendorStr[4] = (edx & 0x00'00'00'ff);
            cpuIdVendorStr[5] = (edx & 0x00'00'ff'00) >> 8;
            cpuIdVendorStr[6] = (edx & 0x00'ff'00'00) >> 16;
            cpuIdVendorStr[7] = (edx & 0xff'00'00'00) >> 24;
            cpuIdVendorStr[8] = (ecx & 0x00'00'00'ff);
            cpuIdVendorStr[9] = (ecx & 0x00'00'ff'00) >> 8;
            cpuIdVendorStr[10] = (ecx & 0x00'ff'00'00) >> 16;
            cpuIdVendorStr[11] = (ecx & 0xff'00'00'00) >> 24;
            cpuIdVendorStr[12] = 0; //null terminated because i value my sanity.

            //cache leaf 1
            //TODO: compare extended and regular cpuid leaves, check if they match (use extended if they do)
            __get_cpuid(1, &eax, &ebx, &ecx, &edx);
            cpuId_leaf1_ecx = ecx;
            cpuId_leaf1_edx = edx;
        }
    }

    void CPU::EnableInterrupts()
    {
        asm volatile("sti");
    }

    void CPU::DisableInterrupts()
    {
        asm volatile("cli");
    }

    bool CPU::InterruptsEnabled()
    {
        uint64_t flags;
        asm volatile("pushf; pop %0" 
                    : "=rm" (flags)
                    :
                    : "memory");

        return (flags & 0b1'000'000'000) != 0;
    }

    bool CPU::FeatureSupported(CpuFeatureFlag flag)
    {
        if (!cpuIdSupported)
            return false;
        
        switch (flag)
        {
        case CpuFeatureFlag::SSE3:
            return (cpuId_leaf1_ecx & (1 << 0)) != 0;
        case CpuFeatureFlag::PCLMUL:
            return (cpuId_leaf1_ecx & (1 << 1)) != 0;
        case CpuFeatureFlag::DTES64:
            return (cpuId_leaf1_ecx & (1 << 2)) != 0;
        case CpuFeatureFlag::MONITOR:
            return (cpuId_leaf1_ecx & (1 << 3)) != 0;
        case CpuFeatureFlag::DS_CPL:
            return (cpuId_leaf1_ecx & (1 << 4)) != 0;
        case CpuFeatureFlag::VMX:
            return (cpuId_leaf1_ecx & (1 << 5)) != 0;
        case CpuFeatureFlag::SMX:
            return (cpuId_leaf1_ecx & (1 << 6)) != 0;
        case CpuFeatureFlag::EST:
            return (cpuId_leaf1_ecx & (1 << 7)) != 0;
        case CpuFeatureFlag::TM2:
            return (cpuId_leaf1_ecx & (1 << 8)) != 0;
        case CpuFeatureFlag::SSSE3:
            return (cpuId_leaf1_ecx & (1 << 9)) != 0;
        case CpuFeatureFlag::CID:
            return (cpuId_leaf1_ecx & (1 << 10)) != 0;
        case CpuFeatureFlag::FMA:
            return (cpuId_leaf1_ecx & (1 << 12)) != 0;
        case CpuFeatureFlag::CX16:
            return (cpuId_leaf1_ecx & (1 << 13)) != 0;
        case CpuFeatureFlag::ETPRD:
            return (cpuId_leaf1_ecx & (1 << 14)) != 0;
        case CpuFeatureFlag::PDCM:
            return (cpuId_leaf1_ecx & (1 << 15)) != 0;
        case CpuFeatureFlag::PCIDE:
            return (cpuId_leaf1_ecx & (1 << 17)) != 0;
        case CpuFeatureFlag::DCA:
            return (cpuId_leaf1_ecx & (1 << 18)) != 0;
        case CpuFeatureFlag::SSE4_1:
            return (cpuId_leaf1_ecx & (1 << 19)) != 0;
        case CpuFeatureFlag::SSE4_2:
            return (cpuId_leaf1_ecx & (1 << 20)) != 0;
        case CpuFeatureFlag::x2APIC:
            return (cpuId_leaf1_ecx & (1 << 21)) != 0;
        case CpuFeatureFlag::MOVBE:
            return (cpuId_leaf1_ecx & (1 << 22)) != 0;
        case CpuFeatureFlag::POPCNT:
            return (cpuId_leaf1_ecx & (1 << 23)) != 0;
        case CpuFeatureFlag::AES:
            return (cpuId_leaf1_ecx & (1 << 25)) != 0;
        case CpuFeatureFlag::XSAVE:
            return (cpuId_leaf1_ecx & (1 << 26)) != 0;
        case CpuFeatureFlag::OSXSAVE:
            return (cpuId_leaf1_ecx & (1 << 27)) != 0;
        case CpuFeatureFlag::AVX:
            return (cpuId_leaf1_ecx & (1 << 28)) != 0;
        case CpuFeatureFlag::FPU:
            return (cpuId_leaf1_edx & (1 << 0)) != 0;
        case CpuFeatureFlag::VME:
            return (cpuId_leaf1_edx & (1 << 1)) != 0;
        case CpuFeatureFlag::DE:
            return (cpuId_leaf1_edx & (1 << 2)) != 0;
        case CpuFeatureFlag::PSE:
            return (cpuId_leaf1_edx & (1 << 3)) != 0;
        case CpuFeatureFlag::TSC:
            return (cpuId_leaf1_edx & (1 << 4)) != 0;
        case CpuFeatureFlag::MSR:
            return (cpuId_leaf1_edx & (1 << 5)) != 0;
        case CpuFeatureFlag::PAE:
            return (cpuId_leaf1_edx & (1 << 6)) != 0;
        case CpuFeatureFlag::MCE:
            return (cpuId_leaf1_edx & (1 << 7)) != 0;
        case CpuFeatureFlag::CX8:
            return (cpuId_leaf1_edx & (1 << 8)) != 0;
        case CpuFeatureFlag::APIC:
            return (cpuId_leaf1_edx & (1 << 9)) != 0;
        case CpuFeatureFlag::SEP:
            return (cpuId_leaf1_edx & (1 << 11)) != 0;
        case CpuFeatureFlag::MTRR:
            return (cpuId_leaf1_edx & (1 << 12)) != 0;
        case CpuFeatureFlag::PGE:
            return (cpuId_leaf1_edx & (1 << 13)) != 0;
        case CpuFeatureFlag::MCA:
            return (cpuId_leaf1_edx & (1 << 14)) != 0;
        case CpuFeatureFlag::CMOV:
            return (cpuId_leaf1_edx & (1 << 15)) != 0;
        case CpuFeatureFlag::PAT:
            return (cpuId_leaf1_edx & (1 << 16)) != 0;
        case CpuFeatureFlag::PSE36:
            return (cpuId_leaf1_edx & (1 << 17)) != 0;
        case CpuFeatureFlag::PSN:
            return (cpuId_leaf1_edx & (1 << 18)) != 0;
        case CpuFeatureFlag::CLF:
            return (cpuId_leaf1_edx & (1 << 19)) != 0;
        case CpuFeatureFlag::NX:
            return (cpuId_leaf1_edx & (1 << 20)) != 0;
        case CpuFeatureFlag::DTES:
            return (cpuId_leaf1_edx & (1 << 21)) != 0;
        case CpuFeatureFlag::ACPI:
            return (cpuId_leaf1_edx & (1 << 22)) != 0;
        case CpuFeatureFlag::MMX:
            return (cpuId_leaf1_edx & (1 << 23)) != 0;
        case CpuFeatureFlag::FXSR:
            return (cpuId_leaf1_edx & (1 << 24)) != 0;
        case CpuFeatureFlag::SSE:
            return (cpuId_leaf1_edx & (1 << 25)) != 0;
        case CpuFeatureFlag::SSE2:
            return (cpuId_leaf1_edx & (1 << 26)) != 0;
        case CpuFeatureFlag::SS:
            return (cpuId_leaf1_edx & (1 << 27)) != 0;
        case CpuFeatureFlag::HTT:
            return (cpuId_leaf1_edx & (1 << 28)) != 0;
        case CpuFeatureFlag::TM1:
            return (cpuId_leaf1_edx & (1 << 29)) != 0;
        case CpuFeatureFlag::IA64:
            return (cpuId_leaf1_edx & (1 << 30)) != 0;
        case CpuFeatureFlag::PBE:
            return (cpuId_leaf1_edx & (1 << 31)) != 0;
        default:
            return false;
        }
    }

    void CPU::LoadPageTableMap(void *topLevelAddress)
    {
        asm volatile("mov %0, %%cr3"
                     :
                     : "r"(topLevelAddress));
    }

    void CPU::InvalidatePageTable(void* tableAddress)
    {
        asm volatile("invlpg (%0)"
                    :
                    : "r"((uint64_t)tableAddress)
                    : "memory");
    }

    extern "C" 
    {
        extern void LoadGDT_impl(void* address);
    }

    void CPU::LoadTable(CpuTable table, void* addr)
    {
        switch (table)
        {
            case CpuTable::x86_64_GDT:
                LoadGDT_impl(addr);
                return;
            case CpuTable::X86_64_IDT:
                asm volatile("lidt 0(%0)" : : "r"(addr));
                return;
        }
    }

    void CPU::Halt()
    {
        asm volatile("     \
    loop: \n            \
        hlt\n           \
        jmp loop\n      \
    ");
    }

    void CPU::WriteMSR(uint32_t id, uint64_t value)
    {
        uint32_t hi = value >> 32;
        uint32_t lo = value & 0xFFFF'FFFF;
        asm volatile("wrmsr"
                    :
                    : "a"(lo), "d"(hi), "c"(id));
    }

    uint64_t CPU::ReadMSR(uint32_t id)
    {
        uint32_t hi, lo;
        asm volatile("rdmsr"
                    : "=a"(lo), "=d"(hi)
                    : "c"(id));
        return lo | ((uint64_t)hi << 32); 
    }

    void CPU::PortWrite8(uint16_t port, uint8_t data)
    {
        asm volatile("outb %0, %1"
                     :
                     : "a"(data), "Nd"(port));
    }

    void CPU::PortWrite16(uint16_t port, uint16_t data)
    {
        asm volatile("outw %0, %1"
                     :
                     : "a"(data), "Nd"(port));
    }

    void CPU::PortWrite32(uint16_t port, uint32_t data)
    {
        asm volatile("outl %0, %1"
                     :
                     : "a"(data), "Nd"(port));
    }

    uint8_t CPU::PortRead8(uint16_t port)
    {
        uint8_t value;
        asm volatile("inb %1, %0"
                     : "=a"(value)
                     : "Nd"(port));

        return value;
    }

    uint16_t CPU::PortRead16(uint16_t port)
    {
        uint16_t value;
        asm volatile("inw %1, %0"
                     : "=a"(value)
                     : "Nd"(port));

        return value;
    }

    uint32_t CPU::PortRead32(uint16_t port)
    {
        uint32_t value;
        asm volatile("inl %1, %0"
                     : "=a"(value)
                     : "Nd"(port));

        return value;
    }

    void CPU::PortIOWait()
    {
        //read from port 0x80 (will force io lines to complete previous operations)
        PortRead8(PORT_USUALLY_EMPTY);
    }
    
    const char* CPU::GetArchitectureName()
    {
        return cpuArch;
    }

    const char* CPU::GetVendorName()
    {
        if (cpuIdSupported)
            return cpuIdVendorStr;
        return (char*)"CPUID unsupported";
    }

    const char* CPU::GetFeatureShortName(CpuFeatureFlag flag)
    {
        switch (flag) 
        {
        case CpuFeatureFlag::SSE3:
            return "sse3";
        case CpuFeatureFlag::PCLMUL:
            return "pclmul";
        case CpuFeatureFlag::DTES64:
            return "dtes64";
        case CpuFeatureFlag::MONITOR:
            return "monitor";
        case CpuFeatureFlag::DS_CPL:
            return "ds_cpl";
        case CpuFeatureFlag::VMX:
            return "vmx";
        case CpuFeatureFlag::SMX:
            return "smx";
        case CpuFeatureFlag::EST:
            return "est";
        case CpuFeatureFlag::TM2:
            return "tm2";
        case CpuFeatureFlag::SSSE3:
            return "ssse3";
        case CpuFeatureFlag::CID:
            return "cid";
        case CpuFeatureFlag::FMA:
            return "fma";
        case CpuFeatureFlag::CX16:
            return "cx16";
        case CpuFeatureFlag::ETPRD:
            return "etprd";
        case CpuFeatureFlag::PDCM:
            return "pdcm";
        case CpuFeatureFlag::PCIDE:
            return "pcide";
        case CpuFeatureFlag::DCA:
            return "dca";
        case CpuFeatureFlag::SSE4_1:
            return "sse4_1";
        case CpuFeatureFlag::SSE4_2:
            return "sse4_2";
        case CpuFeatureFlag::x2APIC:
            return "x2apic";
        case CpuFeatureFlag::MOVBE:
            return "movbe";
        case CpuFeatureFlag::POPCNT:
            return "popcnt";
        case CpuFeatureFlag::AES:
            return "aes";
        case CpuFeatureFlag::XSAVE:
            return "xsave";
        case CpuFeatureFlag::OSXSAVE:
            return "osxsave";
        case CpuFeatureFlag::AVX:
            return "avx";
        case CpuFeatureFlag::FPU:
            return "fpu (lol)";
        case CpuFeatureFlag::VME:
            return "vme";
        case CpuFeatureFlag::DE:
            return "de";
        case CpuFeatureFlag::PSE:
            return "pse";
        case CpuFeatureFlag::TSC:
            return "tsc";
        case CpuFeatureFlag::MSR:
            return "msr";
        case CpuFeatureFlag::PAE:
            return "pae";
        case CpuFeatureFlag::MCE:
            return "mce";
        case CpuFeatureFlag::CX8:
            return "cx8";
        case CpuFeatureFlag::APIC:
            return "apic";
        case CpuFeatureFlag::SEP:
            return "sep";
        case CpuFeatureFlag::MTRR:
            return "mtrr";
        case CpuFeatureFlag::PGE:
            return "pge";
        case CpuFeatureFlag::MCA:
            return "mca";
        case CpuFeatureFlag::CMOV:
            return "cmov";
        case CpuFeatureFlag::PAT:
            return "pat";
        case CpuFeatureFlag::PSE36:
            return "pse36";
        case CpuFeatureFlag::PSN:
            return "psn";
        case CpuFeatureFlag::CLF:
            return "clf";
        case CpuFeatureFlag::NX:
            return "nx";
        case CpuFeatureFlag::DTES:
            return "dtes";
        case CpuFeatureFlag::ACPI:
            return "acpi";
        case CpuFeatureFlag::MMX:
            return "mmx";
        case CpuFeatureFlag::FXSR:
            return "fxsr";
        case CpuFeatureFlag::SSE:
            return "sse";
        case CpuFeatureFlag::SSE2:
            return "sse2";
        case CpuFeatureFlag::SS:
            return "ss";
        case CpuFeatureFlag::HTT:
            return "htt";
        case CpuFeatureFlag::TM1:
            return "tm1";
        case CpuFeatureFlag::IA64:
            return "ia64";
        case CpuFeatureFlag::PBE:
            return "pbe";
        default:
            return "(no short name)";
        }
    }

    const char* CPU::GetFeatureLongName(CpuFeatureFlag flag)
    {
        switch (flag) 
        {
        //TODO: add feature long names
        default:
            return "(no long name)";
        }
    }
}
