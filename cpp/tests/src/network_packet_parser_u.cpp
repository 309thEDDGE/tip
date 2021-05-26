#include <cstdint>
#include <vector>
#include <string>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "network_packet_parser.h"
#include "spdlog_setup_helper_funcs.h"

class NetworkPacketParserTest : public ::testing::Test
{
protected:
	NetworkPacketParser npp_;
	EthernetData eth_data_;
	uint32_t data_length_;
	bool result_;

    NetworkPacketParserTest() : npp_(), eth_data_(), data_length_(0),
		result_(false)
    {
		CreateStdoutLoggerWithName("npp_logger");
    }
};

TEST_F(NetworkPacketParserTest, ParseBadBuffer)
{
	std::vector<uint8_t> buff(500);
	data_length_ = buff.size();
	result_ = npp_.Parse(buff.data(), data_length_, &eth_data_);
	EXPECT_FALSE(result_);
}