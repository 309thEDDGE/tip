#ifndef CH10_TDPF1_HDR_FORMAT_H_
#define CH10_TDPF1_HDR_FORMAT_H_ 

#include <cstdint>

class TDF1CSDWFmt
{
   public:
    uint32_t src : 4;
    uint32_t time_fmt : 4;
    uint32_t leap_year : 1;
    uint32_t date_fmt : 1;

    bool operator==(const TDF1CSDWFmt& rhs) const
    {
        return src == rhs.src
            && time_fmt == rhs.time_fmt
            && leap_year == rhs.leap_year
            && date_fmt == rhs.date_fmt;
    }
};

class TDF1DataIRIGFmt
{
   public:
    uint16_t Tmn : 4;
    uint16_t Hmn : 4;
    uint16_t Sn : 4;
    uint16_t TSn : 3;
    uint16_t : 0;  // skip to end of uint16_t
    uint16_t Mn : 4;
    uint16_t TMn : 4;
    uint16_t Hn : 4;
    uint16_t THn : 2;
    uint16_t : 0;  // skip to end of uint16_t
    uint16_t Dn : 4;
    uint16_t TDn : 4;
    uint16_t HDn : 2;
};

class TDF1DataNonIRIGFmt
{
   public:
    uint16_t Tmn : 4;
    uint16_t Hmn : 4;
    uint16_t Sn : 4;
    uint16_t TSn : 3;
    uint16_t : 0;  // skip to end of uint16_t
    uint16_t Mn : 4;
    uint16_t TMn : 4;
    uint16_t Hn : 4;
    uint16_t THn : 2;
    uint16_t : 0;  // skip to end of uint16_t
    uint16_t Dn : 4;
    uint16_t TDn : 4;
    uint16_t On : 4;
    uint16_t TOn : 1;
    uint16_t : 0;  // skip to end of uint16_t
    uint16_t Yn : 4;
    uint16_t TYn : 4;
    uint16_t HYn : 4;
    uint16_t OYn : 2;
};

#endif  // CH10_TDPF1_HDR_FORMAT_H_