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
#include "translate_tabular_parquet.h"
#include "translatable_table_base.h"
#include "icd_data.h"
#include "managed_path.h"
#include "spdlog/spdlog.h"

class TranslateTabularContext1553 : public TranslateTabularParquet
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


    //////////////////////////////////////////////////////////////////
    //                     Internal Functions
    //////////////////////////////////////////////////////////////////

    virtual std::shared_ptr<TranslateTabularContextBase> Clone();

    // See description in TranslateTabularContextBase

    // Note: no need to override CloseInputFile because ParquetReader will
    // close the first file by default if SetPQPath is called again.


    /*
    Get vectors of data from the current row group and iterate
    over data, identify messages, create tables, and translate as 
    necessary.
    */
    virtual TranslateStatus ConsumeRowGroup(const size_t& thread_index);

    virtual bool FillRowGroupVectors();


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



    /*
    Check if table index is one of a pre-selected set of table which ought
    to be translated. 

    Args:
        thread_index        --> current thread index
        should_select_msg   --> True if messages ought to be
                                selected for translation based on 
                                a set of indices
        selected_tables     --> Set of indices which are considered
                                to be table/message selections
        table_index         --> Index of table which is to be checked
                                against the selection
    
    Return:
        This check is only relevant if should_select_msg bool
        is true. If false, the function shall always return true. Otherwise
        return true if the table index is in the selected_tables.
    */
    bool IsSelectedMessage(const size_t& thread_index, bool should_select_msg,
        const std::set<size_t>& selected_tables, const size_t& table_index);
};


#endif  // #define TRANSLATE_TABULAR_CONTEXT_1553_H_
