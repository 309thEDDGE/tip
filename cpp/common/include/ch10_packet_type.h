
#ifndef CH10_PACKET_TYPE_H_
#define CH10_PACKET_TYPE_H_

#include <cstdint>
#include <string>
#include <map>

enum class Ch10PacketType : uint8_t
{
    /*
	WARNING! Update relevant unit tests in ch10_context_u.cpp
	when this enum is updated.
	*/
    NONE = 0xFF,
    COMPUTER_GENERATED_DATA_F0 = 0x00,
    COMPUTER_GENERATED_DATA_F1 = 0x01,
    COMPUTER_GENERATED_DATA_F2 = 0x02,
    COMPUTER_GENERATED_DATA_F3 = 0x03,
    PCM_F0 = 0x08,
    PCM_F1 = 0X09,
    TIME_DATA_F1 = 0x11,
    MILSTD1553_F1 = 0x19,
    MILSTD1553_F2 = 0x1A,
    ANALOG_F1 = 0x21,
    DISCRETE_F1 = 0x29,
    MESSAGE_F0 = 0x30,
    ARINC429_F0 = 0x38,
    VIDEO_DATA_F0 = 0x40,
    VIDEO_DATA_F1 = 0x41,
    VIDEO_DATA_F2 = 0x42,
    VIDEO_DATA_F3 = 0x43,
    VIDEO_DATA_F4 = 0x44,
    IMAGE_F0 = 0x48,
    IMAGE_F1 = 0x49,
    IMAGE_F2 = 0x4A,
    UART_F0 = 0x50,
    IEEE1394_F0 = 0x58,
    IEEE1394_F1 = 0x59,
    PARALLEL_F0 = 0x60,
    ETHERNET_DATA_F0 = 0x68,
    ETHERNET_DATA_F1 = 0x69,
    TSPI_CTS_F0 = 0x70,
    TSPI_CTS_F1 = 0x71,
    TSPI_CTS_F2 = 0x72,
    CAN_BUS = 0x78,
    FIBRE_CHANNEL_F0 = 0x79,
};

const std::map<Ch10PacketType, std::string> ch10packettype_to_string_map =
    {
        {Ch10PacketType::NONE, "NONE"},
        {Ch10PacketType::COMPUTER_GENERATED_DATA_F0, "COMPUTER_GENERATED_DATA_F0"},
        {Ch10PacketType::COMPUTER_GENERATED_DATA_F1, "COMPUTER_GENERATED_DATA_F1"},
        {Ch10PacketType::COMPUTER_GENERATED_DATA_F2, "COMPUTER_GENERATED_DATA_F2"},
        {Ch10PacketType::COMPUTER_GENERATED_DATA_F3, "COMPUTER_GENERATED_DATA_F3"},
        {Ch10PacketType::PCM_F0, "PCM_F0"},
        {Ch10PacketType::PCM_F1, "PCM_F1"},
        {Ch10PacketType::TIME_DATA_F1, "TIME_DATA_F1"},
        {Ch10PacketType::MILSTD1553_F1, "MILSTD1553_F1"},
        {Ch10PacketType::MILSTD1553_F2, "MILSTD1553_F2"},
        {Ch10PacketType::ANALOG_F1, "ANALOG_F1"},
        {Ch10PacketType::DISCRETE_F1, "DISCRETE_F1"},
        {Ch10PacketType::MESSAGE_F0, "MESSAGE_F0"},
        {Ch10PacketType::ARINC429_F0, "ARINC429_F0"},
        {Ch10PacketType::VIDEO_DATA_F0, "VIDEO_DATA_F0"},
        {Ch10PacketType::VIDEO_DATA_F1, "VIDEO_DATA_F1"},
        {Ch10PacketType::VIDEO_DATA_F2, "VIDEO_DATA_F2"},
        {Ch10PacketType::VIDEO_DATA_F3, "VIDEO_DATA_F3"},
        {Ch10PacketType::VIDEO_DATA_F4, "VIDEO_DATA_F4"},
        {Ch10PacketType::IMAGE_F0, "IMAGE_F0"},
        {Ch10PacketType::IMAGE_F1, "IMAGE_F1"},
        {Ch10PacketType::IMAGE_F2, "IMAGE_F2"},
        {Ch10PacketType::UART_F0, "UART_F0"},
        {Ch10PacketType::IEEE1394_F0, "IEEE1394_F0"},
        {Ch10PacketType::IEEE1394_F1, "IEEE1394_F1"},
        {Ch10PacketType::PARALLEL_F0, "PARALLEL_F0"},
        {Ch10PacketType::ETHERNET_DATA_F0, "ETHERNET_DATA_F0"},
        {Ch10PacketType::ETHERNET_DATA_F1, "ETHERNET_DATA_F1"},
        {Ch10PacketType::TSPI_CTS_F0, "TSPI_CTS_F0"},
        {Ch10PacketType::TSPI_CTS_F1, "TSPI_CTS_F1"},
        {Ch10PacketType::TSPI_CTS_F2, "TSPI_CTS_F2"},
        {Ch10PacketType::CAN_BUS, "CAN_BUS"},
        {Ch10PacketType::FIBRE_CHANNEL_F0, "FIBRE_CHANNEL_F0"}
    };
#endif