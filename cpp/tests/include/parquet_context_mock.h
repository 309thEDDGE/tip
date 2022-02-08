#ifndef PARQUET_CONTEXT_MOCK_H_
#define PARQUET_CONTEXT_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parquet_context.h"

class MockParquetContext : public ParquetContext
{
   public:
    MockParquetContext() : ParquetContext() {}
    MOCK_METHOD3(AddField, bool(const std::shared_ptr<arrow::DataType> type,
                const std::string& fieldName, int listSize));
    MOCK_METHOD2(OpenForWrite, bool(const std::string path,
                const bool truncate));
    MOCK_METHOD4(SetupRowCountTracking, bool(size_t row_group_count,
                               size_t row_group_count_multiplier, bool print_activity,
                               std::string print_msg));
    MOCK_METHOD1(EnableEmptyFileDeletion, void(const std::string& path));
    MOCK_METHOD1(IncrementAndWrite, bool(const uint16_t& thread_id));
    MOCK_METHOD3(SetMemLocI64, bool(std::vector<int64_t>& data,
                const std::string& fieldName, std::vector<uint8_t>* boolField));
    MOCK_METHOD3(SetMemLocI32, bool(std::vector<int32_t>& data,
                const std::string& fieldName, std::vector<uint8_t>* boolField));
    MOCK_METHOD3(SetMemLocI16, bool(std::vector<int16_t>& data,
                const std::string& fieldName, std::vector<uint8_t>* boolField));
    MOCK_METHOD3(SetMemLocI8, bool(std::vector<int8_t>& data,
                const std::string& fieldName, std::vector<uint8_t>* boolField));
    MOCK_METHOD3(SetMemLocString, bool(std::vector<std::string>& data,
                const std::string& fieldName, std::vector<uint8_t>* boolField));
    MOCK_METHOD3(SetMemLocUI8, bool(std::vector<uint8_t>& data,
                const std::string& fieldName, std::vector<uint8_t>* boolField));
};


#endif  // PARQUET_CONTEXT_MOCK_H_
