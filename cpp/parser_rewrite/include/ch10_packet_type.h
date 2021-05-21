
#ifndef CH10_PACKET_TYPE_H_
#define CH10_PACKET_TYPE_H_

#include <cstdint>
#include <string>
#include <map>

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
	ETHERNET_DATA_F0 = 0x68,
};

const std::map<Ch10PacketType, std::string> ch10packettype_to_string_map =
{
	{Ch10PacketType::NONE, "NONE"},
	{Ch10PacketType::COMPUTER_GENERATED_DATA_F1, "COMPUTER_GENERATED_DATA_F1"},
	{Ch10PacketType::TIME_DATA_F1, "TIME_DATA_F1"},
	{Ch10PacketType::MILSTD1553_F1, "MILSTD1553_F1"},
	{Ch10PacketType::VIDEO_DATA_F0, "VIDEO_DATA_F0"},
	{Ch10PacketType::ETHERNET_DATA_F0, "ETHERNET_DATA_F0"}
};

#endif