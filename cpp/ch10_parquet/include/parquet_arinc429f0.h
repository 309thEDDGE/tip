#ifndef PARQUET_ARINC429F0_H
#define PARQUET_ARINC429F0_H

#include <set>
#include <cmath>
#include <string>
#include <vector>
#include "parquet_context.h"
#include "managed_path.h"
#include "ch10_arinc429f0_msg_hdr_format.h"
#include "spdlog/spdlog.h"


class ParquetARINC429F0
{
   private:
    int max_temp_element_count_;
    ParquetContext* pq_ctx_;
    uint32_t temp_octal_label_;

   public:
    static const int ARINC429_ROW_GROUP_COUNT;
    static const int ARINC429_BUFFER_SIZE_MULTIPLIER;
    uint16_t thread_id_;

    // Arrays of data to be written to the Parquet table.
    std::vector<int64_t> time_stamp_;  // save as int64
    std::vector<uint8_t> doy_;          // save as int16
    std::vector<int32_t> gap_time_;     // save as int32
    std::vector<uint8_t> BS_;           // save as single bit
    std::vector<uint8_t> PE_;           // save as single bit
    std::vector<uint8_t> FE_;           // save as single bit
    std::vector<int16_t> bus_;          // save as int16
    std::vector<int16_t> label_;        // save as int16
    std::vector<int8_t> SDI_;          // save as int8
    std::vector<int32_t> data_;        // save as int32
    std::vector<int8_t> SSM_;          // save as int8
    std::vector<uint8_t> parity_;       // save as single bit
    std::vector<int32_t> channel_id_;

    std::string outfile_;

    ParquetARINC429F0(ParquetContext* pq_ctx);
    bool Initialize(const ManagedPath& outfile, uint16_t thread_id);
    void Append(const uint64_t& time_stamp, uint8_t doy,
                const ARINC429F0MsgFmt* msg, const uint16_t& chanid);

   /*
	Take the raw uint8_t ARINC 429 label value parsed directly from the
    429 word in the chapter 10 source. Convert the value to octal label format.

    Args:
        raw_label   --> uint8_t taken directly from the ARINC 429 label field

    Return:
        uint8_t storing value equal to the ARINC 429 label if it were in it's
        octal form.

    Example:
    Parsed label == 83(dec) == 01010011(bin)  ===> 11001010(bin)  ==  312(oct)
    Return uint8_t with value = 312. This will allow the parsed parquet label
    output to equal 312.
	*/
   uint32_t EncodeARINC429Label(uint32_t raw_label);
};

#endif
