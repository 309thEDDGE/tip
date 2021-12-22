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

const int ARINC429_ROW_GROUP_COUNT = 10000;
const int ARINC429_BUFFER_SIZE_MULTIPLIER = 10;

class ParquetARINC429F0 : public ParquetContext
{
   private:
    int max_temp_element_count_;
    uint16_t thread_id_;

    // Set of msg names.
    std::set<std::string> name_set_;

    // Arrays of data to be written to the Parquet table.
    std::vector<uint64_t> time_stamp_;  // save as int64
    std::vector<uint8_t> doy_;          // save as int16
    std::vector<int32_t> gap_time_;     // save as int32
    std::vector<uint8_t> BS_;           // save as single bit
    std::vector<uint8_t> PE_;           // save as single bit
    std::vector<uint8_t> FE_;           // save as single bit
    std::vector<uint8_t> bus_;          // save as int16
    std::vector<uint16_t> label_;        // save as int16
    std::vector<uint8_t> SDI_;          // save as int8
    std::vector<uint32_t> data_;        // save as int32
    std::vector<uint8_t> SSM_;          // save as int8
    std::vector<uint8_t> parity_;       // save as single bit
    std::vector<uint16_t> channel_id_;

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
   uint16_t EncodeARINC429Label(uint8_t& raw_label);

   public:
    ParquetARINC429F0();
    //ParquetMilStd1553F1(ManagedPath outfile, uint16_t ID, bool truncate);
    bool Initialize(const ManagedPath& outfile, uint16_t thread_id);
    void Append(const uint64_t& time_stamp, uint8_t doy,
                const ARINC429F0CSDWFmt* const chan_spec,
                const ARINC429F0MsgFmt* msg, const uint16_t& chanid);
};

#endif