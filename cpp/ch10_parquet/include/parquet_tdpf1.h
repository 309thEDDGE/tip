#ifndef PARQUET_TDPF1_H
#define PARQUET_TDPF1_H

#include <set>
#include <cmath>
#include <string>
#include <vector>
#include "parquet_context.h"
#include "managed_path.h"
#include "ch10_tdpf1_hdr_format.h"
#include "spdlog/spdlog.h"


class ParquetTDPF1
{
   private:
    int max_temp_element_count_;
    ParquetContext* pq_ctx_;

   public:
    static const int TDP_ROW_GROUP_COUNT;
    static const int TDP_BUFFER_SIZE_MULTIPLIER;
    uint16_t thread_id_;

    // Arrays of data to be written to the Parquet table.
    std::vector<int64_t> time_stamp_;  // save as int64
    std::vector<int8_t> src_;  // TDF1CSDWFmt::src
    std::vector<int8_t> time_fmt_;  // TDF1CSDWFmt::time_fmt
    std::vector<uint8_t> leap_year_;  // TDF1CSDWFmt::leap_year, store as bit
    std::vector<uint8_t> date_fmt_;  // TDF1CSDWFmt::date_fmt, store as bit

    std::string outfile_;

      // Static functions which return static const data 
      // remove the need to declare exports when building
      // dynamic libraries in Windows. 
      static int GetRowGroupRowCount();
      static int GetRowGroupBufferCount();

    ParquetTDPF1(ParquetContext* pq_ctx);
    virtual bool Initialize(const ManagedPath& outfile, uint16_t thread_id);
    virtual void Append(const uint64_t& time_stamp, const TDF1CSDWFmt& tdp);
    virtual void Close(const uint16_t& thread_id)
    { pq_ctx_->Close(thread_id); }
};

#endif  // PARQUET_TDPF1_H
