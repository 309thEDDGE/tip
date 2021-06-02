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
	MOCK_METHOD2(ParseEthernet, bool(Tins::Dot3& dot3_pdu, EthernetData* const ed));
	MOCK_METHOD2(ParseEthernetII, bool(Tins::EthernetII& ethii_pdu, EthernetData* const ed));
};

class MockNPPSecondLevel : public NetworkPacketParser
{
public:
	MockNPPSecondLevel() : NetworkPacketParser() {}
	MOCK_METHOD2(ParseIPv4, bool(Tins::IP* ip_pdu, EthernetData* const ed));
	MOCK_METHOD2(ParseEthernetLLC, bool(Tins::LLC* llc_pdu, EthernetData* const ed));
	MOCK_METHOD3(ParseRaw, bool(Tins::RawPDU* raw_pdu, EthernetData* const ed, 
		const uint32_t& max_pload_size));
	MOCK_METHOD2(ParseUDP, bool(Tins::UDP* udp_pdu, EthernetData* const ed));
	MOCK_METHOD2(ParseTCP, bool(Tins::TCP* tcp_pdu, EthernetData* const ed));
};

class MockNPPSelector : public NetworkPacketParser
{
public:
	MockNPPSelector() : NetworkPacketParser() {}
	MOCK_METHOD2(ParserSelector, bool(Tins::PDU* pdu_ptr, EthernetData* const ed));
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
	NiceMock<MockNPPSecondLevel> mock_npp_sec_level_;
	NiceMock<MockNPPSelector> mock_npp_selector_;
	NetworkPacketParser* npp_ptr_;
	Tins::PDU* pdu_ptr_;

    NetworkPacketParserTest() : npp_(), eth_data_(), data_length_(0),
		result_(false), ethertype_(0), mock_npp_top_level_(), npp_ptr_(nullptr),
		mock_npp_sec_level_(), pdu_ptr_(nullptr), mock_npp_selector_()
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
	Tins::Dot3 dot3 = Tins::Dot3() / Tins::LLC();
	dot3.dst_addr(dst_mac);
	dot3.src_addr(src_mac);
	ethertype_ = 1200;
	dot3.length(ethertype_);

	ON_CALL(mock_npp_selector_, ParserSelector(_, _)).
		WillByDefault(Return(true));

	result_ = mock_npp_selector_.ParseEthernet(dot3, &eth_data_);
	EXPECT_TRUE(result_);
	EXPECT_EQ(dst_mac_string_, eth_data_.dst_mac_addr_);
	EXPECT_EQ(src_mac_string_, eth_data_.src_mac_addr_);
	EXPECT_EQ(ethertype_, eth_data_.ethertype_);
}

TEST_F(NetworkPacketParserTest, ParseEthernetIIHeaderData)
{
	dst_mac_string_ = "00:01:fa:9e:1a:cd";
	src_mac_string_ = "2b:01:f7:ae:5c:3f";
	Tins::HWAddress<6> dst_mac(dst_mac_string_);
	Tins::HWAddress<6> src_mac(src_mac_string_);
	Tins::EthernetII eth2 = Tins::EthernetII() / Tins::IP() / Tins::TCP();
	eth2.dst_addr(dst_mac);
	eth2.src_addr(src_mac);
	ethertype_ = 1200;
	eth2.payload_type(ethertype_);

	ON_CALL(mock_npp_sec_level_, ParseIPv4(_, _)).WillByDefault(Return(true));

	result_ = mock_npp_sec_level_.ParseEthernetII(eth2, &eth_data_);
	EXPECT_TRUE(result_);
	EXPECT_EQ(dst_mac_string_, eth_data_.dst_mac_addr_);
	EXPECT_EQ(src_mac_string_, eth_data_.src_mac_addr_);
	EXPECT_EQ(ethertype_, eth_data_.ethertype_);
}

TEST_F(NetworkPacketParserTest, ParseEthernetIIParseIPv4)
{
	Tins::EthernetII eth2 = Tins::EthernetII() / Tins::IP() / Tins::TCP();

	EXPECT_CALL(mock_npp_sec_level_, ParseIPv4(_, &eth_data_)).
		Times(1).WillOnce(Return(true));

	result_ = mock_npp_sec_level_.ParseEthernetII(eth2, &eth_data_);
	EXPECT_TRUE(result_);
}

TEST_F(NetworkPacketParserTest, ParseEthernetParseEthernetLLC)
{
	Tins::Dot3 dot3 = Tins::Dot3() / Tins::LLC();

	EXPECT_CALL(mock_npp_sec_level_, ParseEthernetLLC(_, &eth_data_)).
		Times(1).WillOnce(Return(true));

	result_ = mock_npp_sec_level_.ParseEthernet(dot3, &eth_data_);
	EXPECT_TRUE(result_);
}

TEST_F(NetworkPacketParserTest, ParseIPv4HeaderData)
{
	Tins::IP ip = Tins::IP() / Tins::TCP();
	std::string src_ipaddr_str("192.168.0.1");
	std::string dst_ipaddr_str("255.255.255.0");
	Tins::IPv4Address src_ipaddr(src_ipaddr_str);
	Tins::IPv4Address dst_ipaddr(dst_ipaddr_str);
	ip.src_addr(src_ipaddr);
	ip.dst_addr(dst_ipaddr);
	uint16_t id = 23;
	ip.id(id);
	uint8_t protocol = 3;
	ip.protocol(protocol);
	uint16_t frag_offset = 10;
	Tins::small_uint<13> fragment_offset = frag_offset;
	ip.fragment_offset(fragment_offset);

	ON_CALL(mock_npp_selector_, ParserSelector(_, _)).
		WillByDefault(Return(true));

	result_ = mock_npp_selector_.ParseIPv4(&ip, &eth_data_);
	EXPECT_TRUE(result_);
	EXPECT_EQ(src_ipaddr_str, eth_data_.src_ip_addr_);
	EXPECT_EQ(dst_ipaddr_str, eth_data_.dst_ip_addr_);
	EXPECT_EQ(id, eth_data_.id_);
	EXPECT_EQ(protocol, eth_data_.protocol_);
	EXPECT_EQ(frag_offset, eth_data_.offset_);

}

TEST_F(NetworkPacketParserTest, ParseEthernetLLCHeaderData)
{
	Tins::LLC llc;
	uint8_t dsap = 45;
	uint8_t ssap = 128;
	llc.dsap(dsap);
	llc.ssap(ssap);
	uint8_t format = 0; // either 0, 1, or 3, 0 = information
	llc.type(static_cast<Tins::LLC::Format>(format));

	// Both snd and rcv seq are valid for format = 0
	uint8_t snd_seq_num = 23; // must be modulo 128
	uint8_t rcv_seq_num = 79; // must be modulo 128
	llc.send_seq_number(snd_seq_num);
	llc.receive_seq_number(rcv_seq_num);

	ON_CALL(mock_npp_selector_, ParserSelector(_, _)).
		WillByDefault(Return(true));

	result_ = mock_npp_selector_.ParseEthernetLLC(&llc, &eth_data_);
	EXPECT_TRUE(result_);
	EXPECT_EQ(dsap, eth_data_.dsap_);
	EXPECT_EQ(ssap, eth_data_.ssap_);
	EXPECT_EQ(format, eth_data_.frame_format_);
	EXPECT_EQ(snd_seq_num, eth_data_.snd_seq_number_);
	EXPECT_EQ(rcv_seq_num, eth_data_.rcv_seq_number_);
}

TEST_F(NetworkPacketParserTest, ParseUDPHeaderData)
{
	Tins::UDP udp;
	uint16_t dst_port = 3090;
	uint16_t src_port = 8081;
	udp.dport(dst_port);
	udp.sport(src_port);

	ON_CALL(mock_npp_selector_, ParserSelector(_, _)).
		WillByDefault(Return(true));

	result_ = mock_npp_selector_.ParseUDP(&udp, &eth_data_);
	EXPECT_TRUE(result_);
	EXPECT_EQ(mock_npp_selector_.max_payload_size, EthernetData::max_udp_payload_size_);
	EXPECT_EQ(dst_port, eth_data_.dst_port_);
	EXPECT_EQ(src_port, eth_data_.src_port_);
}

TEST_F(NetworkPacketParserTest, ParseTCPHeaderData)
{
	Tins::TCP tcp;
	uint16_t dst_port = 3090;
	uint16_t src_port = 8081;
	tcp.dport(dst_port);
	tcp.sport(src_port);

	ON_CALL(mock_npp_selector_, ParserSelector(_, _)).
		WillByDefault(Return(true));

	result_ = mock_npp_selector_.ParseTCP(&tcp, &eth_data_);
	EXPECT_TRUE(result_);
	EXPECT_EQ(mock_npp_selector_.max_payload_size, EthernetData::max_tcp_payload_size_);
	EXPECT_EQ(dst_port, eth_data_.dst_port_);
	EXPECT_EQ(src_port, eth_data_.src_port_);
}

TEST_F(NetworkPacketParserTest, ParseRawHeaderAndPayload)
{
	uint32_t pload_size = 627;
	Tins::RawPDU::payload_type pload(pload_size, 24);
	Tins::RawPDU raw(pload);
	uint32_t max_size = 800; // > pload_size

	result_ = npp_.ParseRaw(&raw, &eth_data_, max_size);
	EXPECT_TRUE(result_);
	EXPECT_EQ(pload_size, eth_data_.payload_size_);

	// Test the first pload_size elements.
	EXPECT_THAT(pload, ::testing::ElementsAreArray(eth_data_.payload_ptr_,
		pload_size));

	// Confirm that the remaining mtu_ - pload_size elements
	// are zeros.
	std::vector<uint8_t> expect_zeros(EthernetData::max_payload_size_ - pload_size, 0);
	EXPECT_THAT(expect_zeros, ::testing::ElementsAreArray(
		eth_data_.payload_ptr_ + pload_size, EthernetData::max_payload_size_ - pload_size));
}

TEST_F(NetworkPacketParserTest, ParseRawHeaderAndPayloadGreaterThanMax)
{
	uint32_t pload_size = 1533;
	uint32_t max_size = 800; // < pload_size

	Tins::RawPDU::payload_type pload(pload_size, 19);
	Tins::RawPDU raw(pload);

	result_ = npp_.ParseRaw(&raw, &eth_data_, max_size);
	EXPECT_FALSE(result_);
}

TEST_F(NetworkPacketParserTest, ParseRawHeaderAndPayloadInChain)
{
	uint32_t pload_size = 123;
	Tins::RawPDU::payload_type pload(pload_size, 9);

	// Construct a chain of PDUs terminating in a raw
	// payload.
	Tins::EthernetII eth_packet = Tins::EthernetII() / Tins::IP() / 
		Tins::TCP() / Tins::RawPDU(pload);
	
	// Get the buffer associated with this packet.
	// It is important to only call serialize() once. It appears
	// that calling it multiple times results in undefined behavior.
	Tins::PDU::serialization_type serial_data = eth_packet.serialize();
	uint8_t* buff = serial_data.data();
	data_length_ = serial_data.size();

	result_ = npp_.Parse(buff, data_length_, &eth_data_);
	EXPECT_TRUE(result_);
	EXPECT_EQ(pload_size, eth_data_.payload_size_);

	// Test the first pload_size elements.
	EXPECT_THAT(pload, ::testing::ElementsAreArray(eth_data_.payload_ptr_, 
		pload_size));

	// Confirm that the remaining mtu_ - pload_size elements
	// are zeros.
	std::vector<uint8_t> expect_zeros(EthernetData::max_payload_size_ - pload_size, 0);
	EXPECT_THAT(expect_zeros, ::testing::ElementsAreArray(
		eth_data_.payload_ptr_ + pload_size, EthernetData::max_payload_size_ - pload_size));
}

TEST_F(NetworkPacketParserTest, ParserSelectorNullPointer)
{
	pdu_ptr_ = nullptr;
	result_ = npp_.ParserSelector(pdu_ptr_, &eth_data_);
	EXPECT_FALSE(result_);
}

TEST_F(NetworkPacketParserTest, ParserSelectorParseEthernetLLC)
{
	Tins::LLC llc;
	pdu_ptr_ = dynamic_cast<Tins::PDU*>(&llc);

	EXPECT_CALL(mock_npp_sec_level_, ParseEthernetLLC(&llc, &eth_data_)).
		Times(1).WillOnce(Return(true));

	result_ = mock_npp_sec_level_.ParserSelector(pdu_ptr_, &eth_data_);
	EXPECT_TRUE(result_);
}

TEST_F(NetworkPacketParserTest, ParserSelectorParseRaw)
{
	uint32_t pload_size = 123;
	Tins::RawPDU::payload_type pload(pload_size);
	Tins::RawPDU raw(pload);
	pdu_ptr_ = dynamic_cast<Tins::PDU*>(&raw);

	EXPECT_CALL(mock_npp_sec_level_, ParseRaw(&raw, &eth_data_, _)).
		Times(1).WillOnce(Return(true));

	result_ = mock_npp_sec_level_.ParserSelector(pdu_ptr_, &eth_data_);
	EXPECT_TRUE(result_);
}

TEST_F(NetworkPacketParserTest, ParserSelectorParseIPv4)
{
	Tins::IP ip;
	pdu_ptr_ = dynamic_cast<Tins::PDU*>(&ip);

	EXPECT_CALL(mock_npp_sec_level_, ParseIPv4(&ip, &eth_data_)).
		Times(1).WillOnce(Return(true));

	result_ = mock_npp_sec_level_.ParserSelector(pdu_ptr_, &eth_data_);
	EXPECT_TRUE(result_);
}

TEST_F(NetworkPacketParserTest, ParserSelectorParseUDP)
{
	Tins::UDP udp;
	pdu_ptr_ = dynamic_cast<Tins::PDU*>(&udp);

	EXPECT_CALL(mock_npp_sec_level_, ParseUDP(&udp, &eth_data_)).
		Times(1).WillOnce(Return(true));

	result_ = mock_npp_sec_level_.ParserSelector(pdu_ptr_, &eth_data_);
	EXPECT_TRUE(result_);
}

TEST_F(NetworkPacketParserTest, ParserSelectorParseTCP)
{
	Tins::TCP tcp;
	pdu_ptr_ = dynamic_cast<Tins::PDU*>(&tcp);

	EXPECT_CALL(mock_npp_sec_level_, ParseTCP(&tcp, &eth_data_)).
		Times(1).WillOnce(Return(true));

	result_ = mock_npp_sec_level_.ParserSelector(pdu_ptr_, &eth_data_);
	EXPECT_TRUE(result_);
}


