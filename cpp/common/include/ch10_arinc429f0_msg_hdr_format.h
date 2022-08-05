#ifndef CH10_429F0_MSG_HDR_FORMAT_H_
#define CH10_429F0_MSG_HDR_FORMAT_H_

#include <cstdint>

class ARINC429F0CSDWFmt
{
   public:
    uint32_t count : 16;   // count of ARINC words in message
    uint32_t : 16;
};

class ARINC429F0DataHeaderFmt
{
   public:
    uint32_t gap: 20;      // time gap from preceeding bus word in 0.1 microseconds
    uint32_t : 1;
    uint32_t BS : 1;       // bus speed - high or low
    uint32_t PE : 1;       // parity error
    uint32_t FE : 1;       // format error
    uint32_t bus : 8;      // 429 bus number associated with following data word
};

class ARINC429F0MsgFmt
{
   public:
    uint32_t gap: 20;      // time gap from preceeding bus word in 0.1 microseconds
    uint32_t : 1;
    uint32_t BS : 1;       // bus speed - high or low
    uint32_t PE : 1;       // parity error
    uint32_t FE : 1;       // format error
    uint32_t bus : 8;      // 429 bus number associated with following data word
    uint32_t label : 8;    // ARINC 429 octal label
    uint32_t SDI : 2;      // Source Dest Identifier
    uint32_t data : 19;    // data payload
    uint32_t SSM : 2;      // Sign/Status Matrix
    uint32_t parity: 1;    // arinc word parity bit - odd parity
};

#endif