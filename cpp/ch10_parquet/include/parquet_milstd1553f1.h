#ifndef PARQUET_MILSTD1553F1_H
#define PARQUET_MILSTD1553F1_H

#include <set>
#include <cmath>
#include <string>
#include <vector>
#include "parquet_context.h"
#include "managed_path.h"
#include "ch10_1553f1_msg_hdr_format.h"
#include "spdlog/spdlog.h"

class ParquetMilStd1553F1
{
   private:
      int max_temp_element_count_;

      // ParquetContext --> dependency injection
      ParquetContext* pq_ctx_;

   public:
      const uint16_t* commword_ptr_;
      uint16_t thread_id_;
      static const int DEFAULT_ROW_GROUP_COUNT;
      static const int DEFAULT_BUFFER_SIZE_MULTIPLIER;
      static const int DATA_PAYLOAD_LIST_COUNT;
      std::string outfile_; 

      // Arrays of data to be written to the Parquet table.
      std::vector<int64_t> time_stamp_;  // save as int64
      std::vector<uint8_t> doy_;          // save as single bit
      std::vector<int8_t> ttb_;           // save as int8
      std::vector<uint8_t> WE_;           // save as single bit
      std::vector<uint8_t> SE_;           // save as single bit
      std::vector<uint8_t> WCE_;          // save as single bit
      std::vector<uint8_t> TO_;           // save as single bit
      std::vector<uint8_t> FE_;           // save as single bit
      std::vector<uint8_t> RR_;           // save as single bit
      std::vector<uint8_t> ME_;           // save as single bit
      std::vector<int16_t> gap1_;         // save as int16
      std::vector<int16_t> gap2_;         // save as int16
      std::vector<uint8_t> mode_code_;    // save as single bit
      std::vector<int32_t> data_;        // for all data payloads, save as int16
      std::vector<int32_t> comm_word1_;
      std::vector<int32_t> comm_word2_;
      std::vector<int8_t> rtaddr1_;
      std::vector<uint8_t> tr1_;  // save as single bit
      std::vector<int8_t> subaddr1_;
      std::vector<int8_t> wrdcnt1_;
      std::vector<int8_t> rtaddr2_;
      std::vector<uint8_t> tr2_;  // save as single bit
      std::vector<int8_t> subaddr2_;
      std::vector<int8_t> wrdcnt2_;
      std::vector<int32_t> channel_id_;
      std::vector<int8_t> totwrdcnt_;
      std::vector<int8_t> calcwrdcnt_;
      std::vector<uint8_t> payload_incomplete_;

      ////////////////////////////////////////////////////////
      //     Arrays for status words and decomposition
      ////////////////////////////////////////////////////////

      std::vector<int32_t> status_word1_;
      std::vector<uint8_t> terminal1_;  // single bit
      std::vector<uint8_t> dynbusctrl1_;  // single bit
      std::vector<uint8_t> subsys1_;  // single bit
      std::vector<uint8_t> busy1_;  // single bit
      std::vector<uint8_t> bcastrcv1_;  // single bit
      std::vector<uint8_t> svcreq1_;  // single bit
      std::vector<uint8_t> instr1_;  // single bit
      std::vector<uint8_t> msgerr1_;  // single bit
      std::vector<int8_t> status_rtaddr1_; 

      std::vector<int32_t> status_word2_;
      std::vector<uint8_t> terminal2_;  // single bit
      std::vector<uint8_t> dynbusctrl2_;  // single bit
      std::vector<uint8_t> subsys2_;  // single bit
      std::vector<uint8_t> busy2_;  // single bit
      std::vector<uint8_t> bcastrcv2_;  // single bit
      std::vector<uint8_t> svcreq2_;  // single bit
      std::vector<uint8_t> instr2_;  // single bit
      std::vector<uint8_t> msgerr2_;  // single bit
      std::vector<int8_t> status_rtaddr2_; 

      // Static functions which return static const data 
      // remove the need to declare exports when building
      // dynamic libraries in Windows. 
      static int GetRowGroupRowCount();
      static int GetRowGroupBufferCount();
      static int GetDataPayloadListElementCount();

      ParquetMilStd1553F1(ParquetContext* parquet_context);
      int Initialize(const ManagedPath& outfile, uint16_t thread_id);
      void Append(const uint64_t& time_stamp, uint8_t doy,
                  const MilStd1553F1CSDWFmt* const chan_spec,
                  const MilStd1553F1DataHeaderCommWordFmt* msg, const uint16_t* const data,
                  const uint16_t& chanid, int8_t calcwrdcnt,
                  uint8_t payload_incomplete, const MilStd1553F1StatusWordFmt* statwrd1,
                  const MilStd1553F1StatusWordFmt* statwrd2);
};

#endif