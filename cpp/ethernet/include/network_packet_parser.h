#ifndef NETWORK_PACKET_PARSER_H
#define NETWORK_PACKET_PARSER_H

#include <cstdint>
#include <cstdio>
#include "tins/tins.h"

class NetworkPacketParser
{
private:

public:
	NetworkPacketParser();

	// 802.3
	uint8_t ParseEthernetDot3(const uint8_t* buffer, const uint32_t& tot_size);
};

#endif
