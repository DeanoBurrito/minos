#pragma once

#include <stdint.h>

#define PORT_CMOS_SELECT 0x70
#define PORT_CMOS_DATA 0x71

#define CMOS_REGISTER_SECONDS 0x00
#define CMOS_REGISTER_MINUTES 0x02
#define CMOS_REGISTER_HOURS 0x04
#define CMOS_REGISTER_MONTHDAY 0x07
#define CMOS_REGISTER_MONTH 0x08
#define CMOS_REGISTER_YEAR 0x09

#define CMOS_REGISTER_STATUS_A 0x0A
#define CMOS_REGISTER_STATUS_B 0x0B

namespace Kernel::Drivers
{   
    class CmosRTC
    {
    private:
        static uint8_t centuryRegister;
        static bool using24HourTime;
        static bool usingBcdFormatting;
        static bool UpdateInProgress();

    public:
        static void Init();
        static void SetCenturyRegister(uint8_t reg);

        //seconds and minutes are 0-59, hours 0-23
        static void GetTime(uint8_t* const seconds, uint8_t* const minutes, uint8_t* const hours);
        //day 1-31, month 1-12, year
        static void GetDate(uint8_t* const day, uint8_t* const month, uint16_t* const year);
    };
}