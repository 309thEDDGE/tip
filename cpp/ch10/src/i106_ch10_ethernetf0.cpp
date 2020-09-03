#include "i106_ch10_ethernetf0.h"

I106Ch10EthernetF0::I106Ch10EthernetF0() : I106ParseContext(), frame_index_(0),
framelen_(0), framelen_ptr_((uint8_t*)&framelen_), typelen_ptr_(nullptr), 
i106_status_(I106Status::I106_OK), npp()
{

}

uint8_t I106Ch10EthernetF0::Ingest(I106C10Header* header, void* buffer)
{
	retcode_ = 0;
	frame_index_ = 0;

	i106_status_ = I106_Decode_FirstEthernetF0(header, buffer, &i106_ethmsg_);
	if (i106_status_ != I106Status::I106_OK)
	{
		printf("\n(%03u) I106Ch10EthernetF0::Ingest(): I106_Decode_FirstEthernetF0: %s\n",
			id_, I106ErrorString(i106_status_));

		// There ought to be at least one message in a 1553 packet so I106_NO_MORE_DATA
		// error doesn't apply and the ReadData function that reads the packet body
		// ought to have checked for buffer overruns so the I106_BUFFER_OVERRUN error 
		// does also not apply. If there is a single error then something else is wrong 
		// and we should exit.
		retcode_ = 1;
		return retcode_;
	}

	// There is currently only one format possible, IEEE-802.3 given
	// when Format = 0. Exit if otherwise.
	if (i106_ethmsg_.CSDW->Format != 0)
	{
		printf("\n(%03u) I106Ch10EthernetF0::Ingest(): Ethernet Format NOT EQUAL 0!\n",
			id_);
		retcode_ = 1;
		return retcode_;
	}

	// Record the first frame.
	if (RecordFrame() == 1)
	{
		return retcode_;
	}

	while ((i106_status_ = I106_Decode_NextEthernetF0(&i106_ethmsg_)) == I106Status::I106_OK)
	{
		// Record the frame.
		if (RecordFrame() == 1)
		{
			return retcode_;
		}
	}

	if (i106_status_ != I106Status::I106_NO_MORE_DATA)
	{
		printf("\n(%03u) I106Ch10EthernetF0::Ingest(): I106_Decode_NextEthernetF0 not I106_NO_MORE_DATA (%s)!\n",
			id_, I106ErrorString(i106_status_));
	}

	if (frame_index_ != i106_ethmsg_.CSDW->Frames)
	{
		printf("\n(%03u) I106Ch10EthernetF0::Ingest(): Read frame count (%u) NOT EQUAL to header frame count (%u)!\n",
			id_, frame_index_, i106_ethmsg_.CSDW->Frames);
	}

	return retcode_;
}

uint8_t I106Ch10EthernetF0::RecordFrame()
{
	// Set the IPH pointer and time stamp pointer.
	i106_ethiph_ = i106_ethmsg_.IPH;
	ts_ptr_ = (const Ch10TimeStamp*)i106_ethiph_;

	// Calculate the absolute time stamp for the current Ethernet frame.
	if (CalcAbsTimeFromTs() == 1)
	{
		retcode_ = 1;
		return retcode_;
	}

	if (npp.ParseEthernetDot3(i106_ethmsg_.Data, i106_ethiph_->Length) == 1)
	{
		retcode_ = 1;
		return retcode_;
	}

	// Set the Ethernet frame struct.
	//i106_ethframe_ = (const EthernetF0_Physical_FullMAC*)i106_ethmsg_.Data;

	// Test: calculate length, compare with length from IPH.
	// The payload length given within the frame plus the MAC addresses (2 * 6),
	// Ethertype/length (2) and frame check sequence (4) (= total of 18) is equal to the total 
	// frame size given in the IPH.
	//CalculateFrameLength();
	/*if (i106_ethiph_->Length - 18 != framelen_)
	{
		printf("\n(%03u) I106Ch10EthernetF0::Ingest(): payload length not in agreement:\n",
			id_, frame_index_, i106_ethmsg_.CSDW->Frames);
		printf("data length = %u - 18 != frame length = %hu\n", i106_ethiph_->Length, framelen_);

		retcode_ = 1;
		return retcode_;
	}

	CreateStringMACAddrs();*/

	// Add data to Parquet file.

	frame_index_++;
	return retcode_;
}

void I106Ch10EthernetF0::CalculateFrameLength()
{
	typelen_ptr_ = (const uint8_t*)&i106_ethframe_->TypeLen;
	framelen_ptr_[0] = typelen_ptr_[1];
	framelen_ptr_[1] = typelen_ptr_[0];
}

void I106Ch10EthernetF0::CreateStringMACAddrs()
{
	//dest_mac_addr_.clear();
	//src_mac_addr_.clear();
	for (uint8_t ind = 0; ind < 6; ind++)
	{
		if (ind < 5)
		{
			dest_mac_stream_ << std::hex << i106_ethframe_->Destination[ind] << ":";
			src_mac_stream_ << std::hex << i106_ethframe_->Source[ind] << ":";
		}
		else
		{
			dest_mac_stream_ << std::hex << i106_ethframe_->Destination[ind];
			src_mac_stream_ << std::hex << i106_ethframe_->Source[ind] << ":";
		}
	}

	dest_mac_addr_ = dest_mac_stream_.str();
	src_mac_addr_ = src_mac_stream_.str();
	//printf("dest MAC: %s, src MAC: %s\n", dest_mac_addr_.c_str(), src_mac_addr_.c_str());
}

