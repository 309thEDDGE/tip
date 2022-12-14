#ifndef PARQUET_TDPF1_MOCK_H_
#define PARQUET_TDPF1_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parquet_context.h"
#include "parquet_tdpf1.h"

class MockParquetTDPF1 : public ParquetTDPF1
{
   public:
    MockParquetTDPF1(ParquetContext* pq_ctx) : ParquetTDPF1(pq_ctx) {}
    MOCK_METHOD2(Initialize, int(const ManagedPath& outfile, uint16_t thread_id));
    MOCK_METHOD2(Append, void(const uint64_t& time_stamp, const TDF1CSDWFmt& tdp));
    MOCK_METHOD1(Close, void(const uint16_t& thread_id));
};


#endif  // PARQUET_TDPF1_MOCK_H_
