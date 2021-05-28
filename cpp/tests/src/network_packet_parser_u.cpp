#include <cstdint>
#include <vector>
#include <string>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "network_packet_parser.h"
#include "ethernet_data.h"
#include "spdlog_setup_helper_funcs.h"

using ::testing::_;
using ::testing::SetArgReferee;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Ref;
using ::testing::DoAll;
using ::testing::SetArgPointee;
using ::testing::NiceMock;

class MockNPPTopLevel : public NetworkPacketParser
{
public:
	MockNPPTopLevel() : NetworkPacketParser() {}
	MOCK_METHOD2(ParseEthernet, bool(Tins::Dot3& dot3_pdu, EthernetData* ed));
	MOCK_METHOD2(ParseEthernetII, bool(Tins::EthernetII& ethii_pdu, EthernetData* ed));
};


class NetworkPacketParserTest : public ::testing::Test
{
protected:
	NetworkPacketParser npp_;
	EthernetData eth_data_;
	uint32_t data_length_;
	uint16_t ethertype_;
	std::string dst_mac_string_;
	std::string src_mac_string_;
	bool result_;
	NiceMock<MockNPPTopLevel> mock_npp_top_level_;
	NetworkPacketParser* npp_ptr_;

    NetworkPacketParserTest() : npp_(), eth_data_(), data_length_(0),
		result_(false), ethertype_(0), mock_npp_top_level_(), npp_ptr_(nullptr)
    {
		
    }

	static void SetUpTestCase() 
	{
		CreateStdoutLoggerWithName("npp_logger");
	}
};

TEST_F(NetworkPacketParserTest, ParseHandleMalformedPacket)
{
	std::vector<uint8_t> buff(10); // too small
	data_length_ = buff.size();
	result_ = npp_.Parse(buff.data(), data_length_, &eth_data_);
	EXPECT_FALSE(result_);
}

TEST_F(NetworkPacketParserTest, ParseIdentifyDot3)
{
	std::vector<uint8_t> buff(1000);
	data_length_ = buff.size();
	Tins::Dot3 dot3(buff.data(), data_length_);
	dot3.length(data_length_);
	std::vector<uint8_t> serial = dot3.serialize();
	data_length_ = serial.size();

	EXPECT_CALL(mock_npp_top_level_, ParseEthernet(_, &eth_data_))
		.Times(1).WillOnce(Return(true));

	result_ = mock_npp_top_level_.Parse(serial.data(), data_length_, &eth_data_);
	EXPECT_TRUE(result_);
}

TEST_F(NetworkPacketParserTest, ParseIdentifyEthernetII)
{
	Tins::EthernetII eth2 = Tins::EthernetII() / Tins::IP() / Tins::TCP();
	std::vector<uint8_t> serial = eth2.serialize();
	data_length_ = serial.size();

	EXPECT_CALL(mock_npp_top_level_, ParseEthernetII(_, &eth_data_))
		.Times(1).WillOnce(Return(true));

	result_ = mock_npp_top_level_.Parse(serial.data(), data_length_, &eth_data_);
	EXPECT_TRUE(result_);
}

TEST_F(NetworkPacketParserTest, ParseEthernetHeaderData)
{
	dst_mac_string_ = "00:01:fa:9e:1a:cd";
	src_mac_string_ = "2b:01:f7:ae:5c:3f";
	Tins::HWAddress<6> dst_mac(dst_mac_string_);
	Tins::HWAddress<6> src_mac(src_mac_string_);
	Tins::Dot3 dot3;
	dot3.dst_addr(dst_mac);
	dot3.src_addr(src_mac);
	ethertype_ = 1200;
	dot3.length(ethertype_);
	result_ = npp_.ParseEthernet(dot3, &eth_data_);
	EXPECT_TRUE(result_);
	EXPECT_EQ(dst_mac_string_, eth_data_.dst_mac_addr_);
	EXPECT_EQ(src_mac_string_, eth_data_.src_mac_addr_);
	EXPECT_EQ(ethertype_, eth_data_.ethertype_);
}

//TEST_F(NetworkPacketParserTest, ParseEthernetIIHeaderData)
//{
//	dst_mac_string_ = "00:01:fa:9e:1a:cd";
//	src_mac_string_ = "2b:01:f7:ae:5c:3f";
//	Tins::HWAddress<6> dst_mac(dst_mac_string_);
//	Tins::HWAddress<6> src_mac(src_mac_string_);
//	Tins::Dot3 dot3;
//	dot3.dst_addr(dst_mac);
//	dot3.src_addr(src_mac);
//	ethertype_ = 1200;
//	dot3.payload(ethertype_);
//	result_ = npp_.ParseEthernet(dot3, &eth_data_);
//	EXPECT_TRUE(result_);
//	EXPECT_EQ(dst_mac_string_, eth_data_.dst_mac_addr_);
//	EXPECT_EQ(src_mac_string_, eth_data_.src_mac_addr_);
//	EXPECT_EQ(ethertype_, eth_data_.ethertype_);
//}

