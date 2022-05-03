#ifndef TRANSLATE_TABULAR_PARQUET_H_
#define TRANSLATE_TABULAR_PARQUET_H_

#include <string>
#include <memory>
#include <cstdint>
#include <set>
#include <vector>

/*
Include parquet_reader.h first
*/
#include "parquet_reader.h"
#include "translate_tabular_context_base.h"
#include "translatable_table_base.h"
#include "managed_path.h"
#include "spdlog/spdlog.h"


class TranslateTabularParquet : public TranslateTabularContextBase
{
protected:

    // The output dir and path.
    ManagedPath output_dir_;
    ManagedPath output_base_path_;

    // Read parquet files
    ParquetReader pq_reader_;
    size_t input_row_group_count_;
    size_t current_row_group_row_count_;
    size_t row_group_index_;
    std::shared_ptr<arrow::Schema> schema_;

    // Indices of messages for which tables have been created
    std::set<size_t> table_indices_;

    // Names of messages which are translated, for metadata records
    // after the translate stage is complete.
    std::set<std::string> translated_msg_names_;


public:
    TranslateTabularParquet();
    virtual ~TranslateTabularParquet() {}


    //////////////////////////////////////////////////////////////////
    //                     Internal Functions
    //////////////////////////////////////////////////////////////////

    // See description in TranslateTabularContextBase
    virtual TranslateStatus OpenInputFile(const ManagedPath& input_path,
                                          const size_t& thread_index);
    virtual TranslateStatus OpenOutputFile(const ManagedPath& output_dir,
                                           const ManagedPath& output_base_name, 
                                           const size_t& thread_index);
    virtual TranslateStatus CloseOutputFile(const size_t& thread_index,
                                            bool is_final_file);
    virtual TranslateStatus ConsumeFile(const size_t& thread_index);



    /*
    Get vectors of data from the current row group and iterate
    over data, identify messages, create tables, and translate as 
    necessary.
    */
    virtual TranslateStatus ConsumeRowGroup(const size_t& thread_index);



    /*
    Create the complete output path given the current table name
    and thread index.

    Args:
        output_dir      --> Base output dir
        output_base_name--> base name of output file
        table_name      --> Name of 1553 message/table
        thread_index--> Index of the thread in which the context is
                        processed, counting from zero for the first
                        thread and incrementing for each additional
                        thread.
    
    Return:
        Output path
    */
    ManagedPath CreateTableOutputPath(const ManagedPath& output_dir,
                                      const ManagedPath& output_base_name, std::string table_name,
                                      size_t thread_index);



    /*
    Create table output path parent directory if it doesn't exist.

    Args:
        thread_index    --> current thread index
        table_path      --> Complete table output path
        table_name      --> Name of table for logging purposes

    Return:
        False if output directory does not exist and fails to 
        be created; true otherwise.
    */
    bool CreateTableOutputDir(const size_t& thread_index, const ManagedPath& table_path,
        const std::string& table_name);



    /*
    Check if the row_group_index_ is equal to input_row_group_count_. If so,
    there are no more row groups to read. Fill the vectors with the current
    row group using the specialized FillRowGroupVectors function, then 
    increment the reader and the row group index for the next read.

    Args:
        thread_index        --> Current thread index. Used in log entries
                                to improve relevance.
        row_group_count     --> Count of row groups
        row_group_index     --> Current row group index

    Return:
        TranslateStatus: CONTINUE if there are no remaining row groups
        in the current file, FAIL if FillRowGroupVectors fails. Otherwise
        OK.
    */
    virtual TranslateStatus ReadRowGroup(const size_t& thread_index, 
        const size_t& row_group_count, size_t& row_group_index);



    /*
    Helper function to fill a vector from a column in the current
    row group using ParquetReader::GetNextRG().

    Args:
        output_vec  --> Vector into which all data from a column
                        in the current row group will be placed
        col_name    --> Name of column to read from the current
                        row group
        row_count   --> output value, count of rows in the row group
                        read to fill the column. Equivalent to the
                        count of elements in the vector which are
                        valid values from the row group
        is_list     --> set to true if the column is a list type
                        column

    Return:
        True if no errors occur with the call to ParquetReader::GetNextRG;
        false otherwise.
    */
    template <typename vectype, typename arrowtype>
    bool FillRGVector(std::vector<vectype>& output_vec, std::string col_name,
                      int& row_count, bool is_list);


    
    /*
    Fill vectors of data from the current row group. Must update
    current_row_group_row_count_. 

    TODO: Need to determine how to generalize and test this function.

    Return:
        True if no errors; false otherwise.
    */
    virtual bool FillRowGroupVectors() { return true; }



    /*
    Append time column data and all raw data to columns of a table
    from pointers. Only append data if the table is marked as valid.

    Args:
        thread_index        --> current thread index
        table               --> Pointer to table with columns to
                                which ridealong (time) and raw data
                                will be appended
        time_data           --> Pointer to time data
        raw_data            --> Pointer to beginning of raw data
        raw_data_count      --> Count of raw data words in raw data payload
        table_name          --> Name of table for logs

    Return:
        True always. The table functions that are called will mark the
        table as invalid if an error occurs and the next time this
        function is called the table will be skipped. Currently the logic
        is to avoid termination if a single table is marked as invalid so
        there is no use for returning false. 
    */
    virtual bool AppendTimeAndRawDataToTable(const size_t& thread_index, 
        std::shared_ptr<TranslatableTableBase> table, const uint8_t* time_data,
        const uint8_t* raw_data, const size_t& raw_data_count, 
        const std::string& table_name); 
};

template <typename vectype, typename arrowtype>
bool TranslateTabularParquet::FillRGVector(std::vector<vectype>& output_vec,
                                               std::string col_name,
                                               int& row_count, bool is_list)
{
    if (!pq_reader_.GetNextRG<vectype, arrowtype>(
            pq_reader_.GetColumnNumberFromName(col_name), output_vec, row_count, is_list))
    {
        SPDLOG_ERROR("Failed to read column \"{:s}\" from current row group", col_name);
        return false;
    }
    SPDLOG_DEBUG("Filled column \"{:s}\" with {:d} rows from current row group", col_name,
                 row_count);
    return true;
}

template <>
inline bool TranslateTabularParquet::FillRGVector<uint8_t, arrow::NumericArray<arrow::BooleanType>>(
    std::vector<uint8_t>& output_vec,
    std::string col_name,
    int& row_count, bool is_list)
{
    if (!pq_reader_.GetNextRGBool(
            pq_reader_.GetColumnNumberFromName(col_name), output_vec, row_count, is_list))
    {
        SPDLOG_ERROR("Failed to read column \"{:s}\" from current row group", col_name);
        return false;
    }
    SPDLOG_DEBUG("Filled column \"{:s}\" with {:d} rows from current row group", col_name,
                 row_count);
    return true;
}

#endif  // TRANSLATE_TABULAR_PARQUET_H_
