#ifndef TRANSLATE_TABULAR_CONTEXT_1553_H_
#define TRANSLATE_TABULAR_CONTEXT_1553_H_

#include <string>
#include <set>
#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>
#include <sstream>

/*
Include parquet_reader.h first
*/
#include "parquet_reader.h"
#include "translate_tabular_context_base.h"
#include "translatable_table_base.h"
#include "icd_data.h"
#include "managed_path.h"
#include "spdlog/spdlog.h"

class TranslateTabularContext1553 : public TranslateTabularContextBase
{
   private:
    // Organized data specific to identifying 1553 messages and defining
    // translation metadata for the elements (columns) of each message.
    ICDData icd_data_;

    // Count of raw payload words per row.
    static const size_t raw_data_count_per_row_;

    // User-defined (via config file) list of selected message names
    // (selected_msg_names_), corresponding set of the indices
    // of the tables as determined from the ICDData object (selected_table_indices_),
    // and status of existence of user-input values. It's possible
    // that the user did not select messages and therefore did not
    // provide a list, in which case the vector input to the constructor
    // will be empty.
    std::set<std::string> selected_msg_names_;
    std::set<size_t> selected_table_indices_;
    bool should_select_msgs_;

    // Indices of messages for which tables have been created
    std::set<size_t> table_indices_;

    // Names of messages which are translated, for metadata records
    // after the translate stage is complete.
    std::set<std::string> translated_msg_names_;

    // The output dir and path.
    ManagedPath output_dir_;
    ManagedPath output_base_path_;

    // Read parquet files
    ParquetReader pq_reader_;
    size_t input_row_group_count_;
    size_t current_row_group_row_count_;
    size_t row_group_index_;
    std::shared_ptr<arrow::Schema> schema_;

    // Vectors into which row group data will be read.
    std::vector<uint64_t> time_;
    std::vector<uint16_t> raw_data_;
    std::vector<uint16_t> channelid_;
    std::vector<uint8_t> txrtaddr_;
    std::vector<uint8_t> rxrtaddr_;
    std::vector<uint8_t> txsubaddr_;
    std::vector<uint8_t> rxsubaddr_;

   public:
    const std::set<std::string>& translated_msg_names;

    TranslateTabularContext1553(ICDData icd,
                                const std::set<std::string>& selected_msg_names);
    virtual ~TranslateTabularContext1553() {}

    /*
    Check the following:
        - TranslateTabularContextBase::IsConfigured is true
        - 

    Return:
        True if configuration is as required; false otherwise.
    */
    virtual bool IsConfigured();

    //////////////////////////////////////////////////////////////////
    //                     Internal Functions
    //////////////////////////////////////////////////////////////////

    virtual std::shared_ptr<TranslateTabularContextBase> Clone();

    // See description in TranslateTabularContextBase
    virtual TranslateStatus OpenInputFile(const ManagedPath& input_path,
                                          const size_t& thread_index);
    virtual TranslateStatus OpenOutputFile(const ManagedPath& output_dir,
                                           const ManagedPath& output_base_name, const size_t& thread_index);
    virtual TranslateStatus CloseOutputFile(const size_t& thread_index,
                                            bool is_final_file);

    // Note: no need to override CloseInputFile because ParquetReader will
    // close the first file by default if SetPQPath is called again.

    virtual TranslateStatus ConsumeFile(const size_t& thread_index);

    /*
    Get vectors of data from the current row group and iterate
    over data, identify messages, create tables, and translate as 
    necessary.
    */
    TranslateStatus ConsumeRowGroup(const size_t& thread_index);

    /*
    Fill vectors of data from the current row group.

    TODO: Need to determine how to generalize and test this function.

    Return:
        True if no errors; false otherwise.
    */
    bool FillRowGroupVectors();

    /*
    Helper function to fill a vector from a column in the current
    row group using ParquetReader::GetNextRG().


    */
    template <typename vectype, typename arrowtype>
    bool FillRGVector(std::vector<vectype>& output_vec, std::string col_name,
                      int& row_count, bool is_list);

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
    Wrapper function TranslatableTabularContextBase::CreateTranslatableTable() 
    specific to 1553 and table information determined from ICDData.

    Args:
        icd_data        --> Configured ICDData object
        table_index     --> Index relevant to a table in ICDData object
        table_map       --> Map of index to TranslatableTableBase object
                            to which is added the current table_index and
                            table object that is created.
        output_path     --> Complete output path for table data
        thread_index--> Index of the thread in which the context is
                        processed, counting from zero for the first
                        thread and incrementing for each additional
                        thread.

    Return:
        True if no problems occur; false otherwise. 
    */
    bool CreateTable(const ICDData& icd_data, size_t table_index,
                     std::unordered_map<size_t, std::shared_ptr<TranslatableTableBase>>& table_map,
                     const ManagedPath& output_path, const size_t& thread_index);

    /*
    Adapt ICDElement data to TranslateTabularContextBase::AppendColumn.

    Args:
        table_ptr       --> Pointer to table to which a column should be appended
        icd_elem        --> Configured ICDElement object

    Return:
        True if no errors; false otherwise.
    */
    bool AppendColumnFromICDElement(std::shared_ptr<TranslatableTableBase>& table_ptr,
                                    const ICDElement& icd_elem);

    /*
    Print all current row payload values to logs.
    */
    void PrintPayloadVals(std::string table_name, const uint16_t* data_ptr);
};

template <typename vectype, typename arrowtype>
bool TranslateTabularContext1553::FillRGVector(std::vector<vectype>& output_vec,
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

#endif  // #define TRANSLATE_TABULAR_CONTEXT_1553_H_
