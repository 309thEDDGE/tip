
#ifndef CH10_1553F1_MSG_HDR_FORMAT_H_
#define CH10_1553F1_MSG_HDR_FORMAT_H_

#include <cstdint>

class MilStd1553F1CSDWFmt
{
   public:
    uint32_t count : 24;
    uint32_t : 6;
    uint32_t ttb : 2;
};

class MilStd1553F1DataHeaderFmt
{
   public:
    uint16_t : 3;
    uint16_t WE : 1;   // invalid word error
    uint16_t SE : 1;   // sync type error
    uint16_t WCE : 1;  // word count error
    uint16_t : 3;
    uint16_t TO : 1;       // response time out
    uint16_t FE : 1;       // format error
    uint16_t RR : 1;       // RT to RT transfer
    uint16_t ME : 1;       // message error
    uint16_t bus_dir : 1;  // 0 = msg from chan A, 1 = msg from chan B
    uint16_t : 0;
    uint16_t gap1 : 8;     // time from command/data word to first and only status
    uint16_t gap2 : 8;     // time from last data word and second status word
    uint16_t length : 16;  // total bytes in the message (command, data, status)
};

class MilStd1553F1DataHeaderCommWordFmt
{
   public:
    uint16_t : 3;
    uint16_t WE : 1;   // invalid word error
    uint16_t SE : 1;   // sync type error
    uint16_t WCE : 1;  // word count error
    uint16_t : 3;
    uint16_t TO : 1;       // response time out
    uint16_t FE : 1;       // format error
    uint16_t RR : 1;       // RT to RT transfer
    uint16_t ME : 1;       // message error
    uint16_t bus_dir : 1;  // 0 = msg from chan A, 1 = msg from chan B
    uint16_t : 0;
    uint16_t gap1 : 8;          // time from command/data word to first and only status
    uint16_t gap2 : 8;          // time from last data word and second status word
    uint16_t length : 16;       // total bytes in the message (command, data, status)
    uint16_t word_count1 : 5;   // command word, N transmitted/requested messages
    uint16_t sub_addr1 : 5;     // command word, sub address location
    uint16_t tx1 : 1;           // command word, message is for remote to transmit
    uint16_t remote_addr1 : 5;  // command word, remote LRU addr.
    uint16_t word_count2 : 5;   // command word, N transmitted/requested messages
    uint16_t sub_addr2 : 5;     // command word, sub address location
    uint16_t tx2 : 1;           // command word, message is for remote to transmit
    uint16_t remote_addr2 : 5;  // command word, remote LRU addr.
};

class MilStd1553F1DataHeaderCommWordOnlyFmt
{
   public:
    uint16_t : 16;
    uint16_t : 16;
    uint16_t : 16;
    uint16_t comm_word1 : 16;
    uint16_t comm_word2 : 16;
};

class MilStd1553F1StatusWordFmt
{
    public:
        uint16_t terminal       : 1;
        uint16_t dynbusctrl     : 1;
        uint16_t subsys         : 1;
        uint16_t busy           : 1;
        uint16_t bcastrcv       : 1;
        uint16_t                : 3;
        uint16_t svcreq         : 1;
        uint16_t instr          : 1;
        uint16_t msgerr         : 1;
        uint16_t rtaddr         : 5; 
};

#endif