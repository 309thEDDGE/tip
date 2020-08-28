#ifndef MILSTD1553F1FMT_H
#define MILSTD1553F1FMT_H

#include <cstdint>

class MilStd1553F1ChanSpecFormat
{
	public:
	uint32_t count				: 24;
	uint32_t 					: 6;
	uint32_t ttb			 	: 2;
};

enum class MilStd1553F1Status: uint8_t
{
	PARSE_OK			= 1,
	PARSE_FAIL			= 0,
	TS_NOT_IMPL			= 2,
	TS_RESERVED			= 3,
	MSG_IDENTITY_FAIL	= 4,
	WRD_COUNT_MISMATCH	= 5,
	BUS_ID				= 6,
};

class MilStd1553F1Msg
{
	public:
	//uint32_t ts1;			 // time stamp
	//uint32_t ts2;
	uint16_t 			: 3; 
	uint16_t WE			: 1; // invalid word error
	uint16_t SE			: 1; // sync type error
	uint16_t WCE		: 1; // word count error 
	uint16_t 			: 3;
	uint16_t TO			: 1; // response time out
	uint16_t FE			: 1; // format error 
	uint16_t RR			: 1; // RT to RT transfer
	uint16_t ME			: 1; // message error
	uint16_t bus_dir	: 1; // 0 = msg from chan A, 1 = msg from chan B
	uint16_t 			: 0; 
	uint16_t gap1       : 8; // time from command/data word to first and only status
	uint16_t gap2       : 8; // time from last data word and second status word
	uint16_t length; 		 // total bytes in the message (command, data, status)
};

class MilStd1553F1MsgCommWord
{
public:
	//uint32_t ts1;			 // time stamp
	//uint32_t ts2;
	uint16_t : 3;
	uint16_t WE : 1; // invalid word error
	uint16_t SE : 1; // sync type error
	uint16_t WCE : 1; // word count error 
	uint16_t : 3;
	uint16_t TO : 1; // response time out
	uint16_t FE : 1; // format error 
	uint16_t RR : 1; // RT to RT transfer
	uint16_t ME : 1; // message error
	uint16_t bus_dir : 1; // 0 = msg from chan A, 1 = msg from chan B
	uint16_t : 0;
	uint16_t gap1 : 8; // time from command/data word to first and only status
	uint16_t gap2 : 8; // time from last data word and second status word
	uint16_t length : 16; // total bytes in the message (command, data, status)
	uint16_t word_count1 : 5; // command word, N transmitted/requested messages
	uint16_t sub_addr1 : 5; // command word, sub address location
	uint16_t tx1 : 1; // command word, message is for remote to transmit
	uint16_t remote_addr1 : 5; // command word, remote LRU addr.
	uint16_t word_count2 : 5; // command word, N transmitted/requested messages
	uint16_t sub_addr2 : 5; // command word, sub address location
	uint16_t tx2 : 1; // command word, message is for remote to transmit
	uint16_t remote_addr2 : 5; // command word, remote LRU addr.
};

//class MilStd1553F1Msg
//{
//	public:
//	//uint32_t ts1;			 // time stamp
//	//uint32_t ts2;
//	uint32_t 			: 3; 
//	uint32_t WE			: 1; // invalid word error
//	uint32_t SE			: 1; // sync type error
//	uint32_t WCE		: 1; // word count error 
//	uint32_t 			: 3;
//	uint32_t TO			: 1; // response time out
//	uint32_t FE			: 1; // format error 
//	uint32_t RR			: 1; // RT to RT transfer
//	uint32_t ME			: 1; // message error
//	uint32_t bus_dir	: 1; // 0 = msg from chan A, 1 = msg from chan B
//	uint32_t 			: 2; 
//	uint32_t gap1       : 8; // time from command/data word to first and only status
//	uint32_t gap2       : 8; // time from last data word and second status word
//	uint32_t length		: 16; // total bytes in the message (command, data, status)
//};

//class CommandWord
//{
//public:
//	uint16_t word_count : 5;
//	uint16_t sub_addr : 5;
//	uint16_t tx : 1;
//	uint16_t remote_addr : 5;
//};

#endif