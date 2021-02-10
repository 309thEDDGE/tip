#include "ch10_context.h"

void Ch10Context::SetSearchingForTDP(bool should_search)
{
	searching_for_tdp_ = should_search;
	found_tdp_ = false;
}

Ch10Status Ch10Context::ContinueWithPacketType(uint8_t data_type)
{
	// If the boolean searching_for_tdp_ is true then return false unless
	// the current packet is a "TMATS" (computer generated data, format 1) 
	// or TDP.
	if (searching_for_tdp_)
	{
		if (!found_tdp_)
		{
			if (data_type == static_cast<uint8_t>(Ch10PacketType::COMPUTER_GENERATED_DATA_F1))
			{
				printf("TMATS found!\n");
				return Ch10Status::PKT_TYPE_YES;
			}
			else if (data_type == static_cast<uint8_t>(Ch10PacketType::TIME_DATA_F1))
			{
				printf("TDP found!\n");
				found_tdp_ = true;
				return Ch10Status::PKT_TYPE_YES;
			}
			return Ch10Status::PKT_TYPE_NO;
		}
	}
	else
	{
		if (data_type == static_cast<uint8_t>(Ch10PacketType::TIME_DATA_F1))
		{
			return Ch10Status::PKT_TYPE_EXIT;
		}
	}

	// Check if the packet type is one that is configured to be parsed.
	//
	// NOt sure about the best way to do this. 
	
	return Ch10Status::PKT_TYPE_YES;
}

void Ch10Context::UpdateAbsolutePosition(uint64_t new_absolute_pos)
{
	absolute_position_ = new_absolute_pos;
}