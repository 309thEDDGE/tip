#ifndef TRANSLATABLE_1553_TABLE_H
#define TRANSLATABLE_1553_TABLE_H

#include <thread>
#include <chrono>
#include <vector>
#include <memory>
#include <string>
#include "translatable_column.h"
#include "parquet_context.h"
#include "icd_data.h"
#include "icd_translate.h"
#include "managed_path.h"

class Translatable1553Table
{
   private:
    const int DATA_PAYLOAD_LIST_COUNT = 32;
    // Vector of vectors of 1553 message payloads, i.e., uint16_t.
    // Each column/field of the table will occupy one of the vectors
    // and fill it with raw data, whether single or multiple words per
    // datum. The first vector will be occupied by the first column/field
    // in the table and the second vector will be occupied by second
    // column, etc.
    //std::vector<std::vector<uint16_t>> raw_col_payloads_;

    // Vector of unique_ptr to TranslatableColumns. Each
    // object represents a single table column and includes
    // raw data and vector for translated data output.
    std::vector<std::unique_ptr<TranslatableBase>> ridealong_columns_;
    std::vector<std::unique_ptr<TranslatableBase>> columns_;

    // Table name is equivalent to the comet message name.
    // Note: this may not be necessary because CometTranslationUnit
    // also encapsulates this information.
    std::string name_;

    // Row group size, same for all columns.
    int64_t row_group_size_;

    // Number of bytes to be consumed by the data type for the translated EUs.
    uint8_t datum_byte_count_;

    // Total number of elements in columns_ vector.
    uint16_t column_count_;

    // Index in columns_
    uint16_t col_ind_;

    // Translation error count.
    uint16_t translation_err_count_;

    // Pointer to current location of raw data for the given column.
    const int32_t* source_raw_data_ptr_;

    // The number of raw data values represented to be stored in each columns'
    // raw data vector and the number of translated values. Used primarily
    // as an argument to ParquetContext.
    int allocated_row_count_;

    // ParquetContext pointer.
    std::unique_ptr<ParquetContext> pq_;

    // Value that indicates a table is "bad" in at least one way.
    // A bad table is one that does not allow creation of ParquetContext,
    // does not append data to the raw data vector, and resizes any
    // ridealone column vectors as zero. The member variable below
    // indicates that the table is bad if the value is greater than zero.
    // The specific value encodes the reason for categorizing the table as
    // bad.
    //
    // 1 = No valid comet translations for any column. Ex: C05.
    uint8_t bad_value_;

    // Temporary translation attributes to filled by TranslatableColumn.
    size_t icd_table_index_;
    ICDData* icd_;
    ICDTranslate icd_translate_;
    ICDElementSchema icd_schema_;
    uint8_t create_column_by_attributes(const ICDElement* icd_elem_ptr, const int& row_alloc_count);
    uint8_t temp_wordno_;
    uint8_t temp_bitmsb_;
    bool temp_is_bitlevel_;
    bool temp_have_set_mem_loc_;
    uint8_t temp_byte_count_;
    std::string temp_name_;

    uint8_t set_parquet_context_memory_location(uint16_t& col_ind, bool& is_ridealong_column);

   public:
    Translatable1553Table(std::string msg_name, ICDData* icd, size_t icd_table_index);
    uint8_t create_translatable_columns(const int& row_alloc_count);
    uint8_t create_ridealong_column(const int& row_alloc_count, std::shared_ptr<arrow::Field> arrow_field);

    void append_data(int64_t& time, const int32_t* raw_data, int64_t& row_ind, bool debug);
    void append_data(int64_t& time, const int32_t* raw_data, int64_t& row_ind);
    std::string& name();
    uint16_t translate();
    uint8_t configure_parquet_context(ManagedPath& output_dir, ManagedPath& base_name,
                                      bool is_multithreaded, uint8_t id);
    void write_to_parquet_file(uint16_t& write_rows_count);
    bool is_bad();
};

#endif
