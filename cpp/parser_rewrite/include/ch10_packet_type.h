
#ifndef CH10_PACKET_TYPE_H_
#define CH10_PACKET_TYPE_H_

enum class Ch10PacketType : uint8_t
{
	/*
	WARNING! Update CreatePacketTypeConfigReference with
	any packet types that are added to this enum! Also update
	relevant unit tests in ch10_context_u.cpp.
	*/
	NONE = 0xFF,
	COMPUTER_GENERATED_DATA_F1 = 0x01,
	TIME_DATA_F1 = 0x11,
	MILSTD1553_F1 = 0x19,
	VIDEO_DATA_F0 = 0x40,
};

#endif