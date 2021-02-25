
#include "ch10_packet_header_component.h"

Ch10Status Ch10PacketHeaderComponent::Parse(const uint8_t*& data)
{
    // Parse the standard header portion.
    ParseElements(std_elems_vec_, data);

    // If the sync pattern is not correct, then this is an invalid header.
    if ((*std_hdr_elem_.element)->sync != sync_)
        return Ch10Status::BAD_SYNC;

    // Calculate and compare checksum.
    status_ = VerifyHeaderChecksum((const uint8_t*)(*std_hdr_elem_.element),
        (*std_hdr_elem_.element)->checksum);
    if (status_ != Ch10Status::CHECKSUM_TRUE)
    {
        return status_;
    }

    return Ch10Status::OK;
}

Ch10Status Ch10PacketHeaderComponent::ParseSecondaryHeader(const uint8_t*& data)
{
    // Only parse if the secondary header option is enabled.
    if ((*std_hdr_elem_.element)->secondary_hdr == 1)
    {
        printf("Ch10PacketHeaderComponent::ParseSecondaryHeader(): Secondary header not handled!\n");
        switch ((*std_hdr_elem_.element)->time_format)
        {
        case 0:
        {
            ParseElements(secondary_binwt_elems_vec_, data);
        }
        case 1: 
        {
            ParseElements(secondary_ieee_elems_vec_, data);
        }
        case 2:
        {
            ParseElements(secondary_ertc_elems_vec_, data);
        }
        default:
        {
            return Ch10Status::INVALID_SECONDARY_HDR_FMT;
            printf("!!! Ch10PacketHeaderComponent::ParseSecondaryHeader(): Invalid secondary header format!\n");
        }
        }
    }
    return Ch10Status::OK;
}

Ch10Status Ch10PacketHeaderComponent::VerifyHeaderChecksum(const uint8_t* pkt_data,
    const uint32_t& checksum_value)
{
    checksum_unit_count_ = header_checksum_byte_count_ / 2;
    checksum_data_ptr16_ = (const uint16_t*)pkt_data;
    checksum_value16_ = 0;
    for (uint8_t i = 0; i < checksum_unit_count_; i++)
        checksum_value16_ += checksum_data_ptr16_[i];

    if (checksum_value16_ == checksum_value)
        return Ch10Status::CHECKSUM_TRUE;

    printf("Ch10PacketHeaderComponent::VerifyHeaderChecksum(): False!\n");
    return Ch10Status::CHECKSUM_FALSE;
}

Ch10Status Ch10PacketHeaderComponent::VerifyDataChecksum(const uint8_t* body_data,
    const uint32_t& checksum_existence, const uint32_t& pkt_size, 
    const uint32_t& secondary_hdr)
{
    // Calculate the total count of bytes in the data checksum.
    uint32_t checksum_byte_count = 0;
    if (secondary_hdr)
        checksum_byte_count = pkt_size - std_hdr_size_ - secondary_hdr_size_;
    else
        checksum_byte_count = pkt_size - std_hdr_size_;
    
    // 16-bit data checksum
    if (checksum_existence == 2)
    {
        // Adjust total byte count by the last few bytes in the footer
        // which are occupied by the value of the data checksum.
        checksum_byte_count -= 2;

        // Get the last 2 bytes from the packet and interpret as
        // the recorded checksum value.
        checksum_data_ptr16_ = (const uint16_t*)(body_data + checksum_byte_count);
        uint16_t checksum_value = *checksum_data_ptr16_;

        checksum_unit_count_ = checksum_byte_count / 2;
        checksum_data_ptr16_ = (const uint16_t*)body_data;
        checksum_value16_ = 0;
        for (uint32_t i = 0; i < checksum_unit_count_; i++)
        {
            checksum_value16_ += checksum_data_ptr16_[i];
            //printf("summing %hu\n", checksum_data_ptr16_[i]);
        }
        /*printf("Ch10PacketHeaderComponent::VerifyDataChecksum(): checksum_value16_ = %hu\n",
            checksum_value16_);
        printf("expected value = %hu\n", checksum_value);*/
        if (checksum_value16_ == checksum_value)
            return Ch10Status::CHECKSUM_TRUE;
    }
    // No data checksum present.
    else if (checksum_existence == 0)
        return Ch10Status::CHECKSUM_NOT_PRESENT;
    // 8-bit data checksum
    else if (checksum_existence == 1)
    {
        checksum_byte_count -= 1;

        checksum_data_ptr8_ = (const uint8_t*)(body_data + checksum_byte_count);
        uint8_t checksum_value = *checksum_data_ptr8_;

        checksum_unit_count_ = checksum_byte_count;
        checksum_data_ptr8_ = body_data;
        checksum_value8_ = 0;
        for (uint32_t i = 0; i < checksum_unit_count_; i++)
        {
            checksum_value8_ += checksum_data_ptr8_[i];
            //printf("summing %hhu\n", checksum_data_ptr8_[i]);
        }
        /*printf("Ch10PacketHeaderComponent::VerifyDataChecksum(): checksum_value8_ = %hhu\n",
            checksum_value8_);
        printf("expected value = %hhu\n", checksum_value);*/
        if (checksum_value8_ == checksum_value)
            return Ch10Status::CHECKSUM_TRUE;
    }
    // 32-bit data checksum
    else if (checksum_existence == 3)
    {
        checksum_byte_count -= 4;

        checksum_data_ptr32_ = (const uint32_t*)(body_data + checksum_byte_count);
        uint32_t checksum_value = *checksum_data_ptr32_;

        checksum_unit_count_ = checksum_byte_count / 4;
        checksum_data_ptr32_ = (const uint32_t*)body_data;
        checksum_value32_ = 0;
        for (uint32_t i = 0; i < checksum_unit_count_; i++)
            checksum_value32_ += checksum_data_ptr32_[i];

        if (checksum_value32_ == checksum_value)
            return Ch10Status::CHECKSUM_TRUE;
    }
    
    printf("Ch10PacketHeaderComponent::VerifyDataChecksum(): False!\n");
    return Ch10Status::CHECKSUM_FALSE;
}