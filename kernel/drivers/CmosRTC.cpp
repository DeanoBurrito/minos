#include <drivers/CmosRTC.h>
#include <drivers/CPU.h>
#include <InterruptScopeGuard.h>

#define READ_RTC_REG(x, y) CPU::PortWrite8(PORT_CMOS_SELECT, x); y = CPU::PortRead8(PORT_CMOS_DATA);

namespace Kernel::Drivers
{
    uint8_t CmosRTC::centuryRegister = 0;
    bool CmosRTC::using24HourTime;
    bool CmosRTC::usingBcdFormatting;

    bool CmosRTC::UpdateInProgress()
    {
        CPU::PortWrite8(PORT_CMOS_SELECT, CMOS_REGISTER_STATUS_A);
        return CPU::PortRead8(PORT_CMOS_DATA) & 0x80;
    }

    void CmosRTC::Init()
    {
        InterruptScopeGuard interruptGuard;

        //getting current encoding settings from status register B
        CPU::PortWrite8(PORT_CMOS_SELECT, CMOS_REGISTER_STATUS_B);
        uint8_t statusB = CPU::PortRead8(PORT_CMOS_DATA);
        using24HourTime = (statusB & 0x02) != 0;
        usingBcdFormatting = (statusB & 0x04) != 0;
    }

    void CmosRTC::SetCenturyRegister(uint8_t reg)
    {
        centuryRegister = reg;
    }

    void CmosRTC::GetTime(uint8_t* const seconds, uint8_t* const minutes, uint8_t* const hours)
    {   
        //>>>while CMOS is updating, read all registers, then read them again after flag is dropped. If we get identical values then we're good
        //Instead of that, we're just reading once because this is running in a VM, and most likely will be overriden by network time.
        READ_RTC_REG(CMOS_REGISTER_SECONDS, *seconds);
        READ_RTC_REG(CMOS_REGISTER_MINUTES, *minutes);
        READ_RTC_REG(CMOS_REGISTER_HOURS, *hours);

        //decode BCD if needed
        if (usingBcdFormatting)
        {
            *seconds = (*seconds & 0x0F) + ((*seconds / 16) * 10);
            *minutes = (*minutes & 0x0F) + ((*minutes / 16) * 10);
            *hours = ((*hours & 0x0F) + (((*hours & 0x70) / 16) * 10)) | (*hours & 0x80);
        }

        //convert to 24 if needed, either by status B or the flag being set
        if (using24HourTime && (*hours & 0x80))
            *hours = ((*hours & 0x7F) + 12) % 24;
    }

    void CmosRTC::GetDate(uint8_t* const dayOfMonth, uint8_t* const month, uint16_t* const year)
    {
        READ_RTC_REG(CMOS_REGISTER_MONTHDAY, *dayOfMonth);
        READ_RTC_REG(CMOS_REGISTER_MONTH, *month);

        *year = 0; //zero this to ensure when we start doing maths later, its not wonky (just trust me here)
        READ_RTC_REG(CMOS_REGISTER_YEAR, *year);

        uint8_t century = 20; //default to the year 20xx, incase this isnt overriden.
        if (centuryRegister > 0)
        {
            READ_RTC_REG(centuryRegister, century);
        }

        if (usingBcdFormatting)
        {
            *dayOfMonth = (*dayOfMonth & 0x0F) + ((*dayOfMonth / 16) * 10);
            *month = (*month & 0x0F) + ((*month / 16) * 10);
            *year = (*year & 0x0F) + ((*year / 16) * 10);

            if (centuryRegister > 0)
                century = (century & 0x0F) + ((century / 16) * 10);
        }

        *year = *year + (century * 100);
    }
}
