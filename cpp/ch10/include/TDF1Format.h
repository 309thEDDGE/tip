#ifndef TDF1FMT_H
#define TDF1FMT_H

#include <stdint.h>

class TDF1ChanSpecFormat
{
	public:
	uint32_t 	src					: 4;
	uint32_t 	time_fmt			: 4;
	uint32_t 	leap_year	 		: 1;
	uint32_t 	date_fmt			: 1;
	//uint32_t						: 0; // skip to end if uint32_t
};

class TDF1Data3Format
{
	public:
	uint16_t	Tmn	: 4;
	uint16_t	Hmn : 4;
	uint16_t	Sn	: 4;
	uint16_t	TSn	: 3;
	uint16_t		: 0; // skip to end of uint16_t
	uint16_t 	Mn	: 4;
	uint16_t 	TMn	: 4;
	uint16_t 	Hn	: 4;
	uint16_t 	THn	: 2;
	uint16_t 		: 0; // skip to end of uint16_t
	uint16_t 	Dn	: 4;
	uint16_t 	TDn : 4;
	uint16_t	HDn	: 2;
};

class TDF1Data4Format
{
	public:
	uint16_t	Tmn	: 4;
	uint16_t	Hmn : 4;
	uint16_t	Sn	: 4;
	uint16_t	TSn	: 3;
	uint16_t		: 0; // skip to end of uint16_t
	uint16_t 	Mn	: 4;
	uint16_t 	TMn	: 4;
	uint16_t 	Hn	: 4;
	uint16_t 	THn	: 2;
	uint16_t 		: 0; // skip to end of uint16_t
	uint16_t 	Dn	: 4;
	uint16_t 	TDn : 4;
	uint16_t	On	: 4;
	uint16_t 	TOn	: 1;
	uint16_t 		: 0; // skip to end of uint16_t
	uint16_t 	Yn	: 4;
	uint16_t 	TYn	: 4;
	uint16_t 	HYn	: 4;
	uint16_t	OYn	: 2;
};

enum class TDF1Status: uint8_t
{
	PARSE_OK			= 1,
	PARSE_FAIL			= 0,
	INVALID				= 2, // Time format (fmt) is 0xF = None (invalid) or time source is 0xF = None
	INCOMPATIBLE		= 3, // Time source is internal (0) and time format is not real-time clock (3)
};

#endif