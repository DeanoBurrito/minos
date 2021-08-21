#include <cpuid.h>
#include <drivers/CPU.h>

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
            cpuIdSupported = false;
        else
            cpuIdSupported = true;

        if (cpuIdSupported)
        {
            uint32_t eax, ebx, ecx, edx;

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

            //cache leaf extended leaf 1
            __get_cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
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
            return (cpuId_leaf1_edx & (1 << 10)) != 0;
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
        extern void LoadGDT_impl(GDTDescriptor* address);
    }
    
    void CPU::LoadGDT(GDTDescriptor* address)
    {
        LoadGDT_impl(address);
    }

    void CPU::LoadIDT(IDTR *idtr)
    {
        asm volatile("lidt 0(%0)"
                     :
                     : "r"(idtr));
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
        uint32_t hi = (value & 0xffff0000) >> 16;
        uint32_t lo = (value & 0x0000ffff);
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
        return lo | (hi << 16);
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
    
    char* CPU::GetArchitectureName()
    {
        return cpuArch;
    }

    char* CPU::GetVendorName()
    {
        if (cpuIdSupported)
            return cpuIdVendorStr;
        return (char*)"CPUID unsupported";
    }
}
