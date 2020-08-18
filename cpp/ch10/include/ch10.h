#ifndef CH10DATATYPE_H
#define CH10DATATYPE_H

#include <stdint.h>

enum class Ch10DataType: uint8_t
{
	TIME_DATA_F1		= 0x11,
	MILSTD1553_DATA_F1	= 0x19,
	VIDEO_DATA_F0	 	= 0x40,
	DISCRETE_DATA_F1	= 0x29,
	ANALOG_DATA_F1		= 0x21,
};

#endif 