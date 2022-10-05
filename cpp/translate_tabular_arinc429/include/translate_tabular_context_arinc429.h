#ifndef TRANSLATE_TABULAR_CONTEXT_ARINC429_H_
#define TRANSLATE_TABULAR_CONTEXT_ARINC429_H_

#include <string>
#include <memory>
#include <cstdint>
#include <vector>
#include <set>
#include <unordered_map>
#include <map>


/*
Include parquet_reader.h first
*/
#include "parquet_reader.h"
#include "translate_tabular_context_base.h"
#include "translate_tabular_parquet.h"
#include "translatable_table_base.h"
#include "translatable_arinc429_table.h"
#include "managed_path.h"
#include "spdlog/spdlog.h"
#include "arinc429_data.h"
#include "icd_element.h"

class MS16Word
{
public:
    uint16_t word: 15;
    uint16_t sign: 1;
};

class ARINC429Word
{
public:
    uint32_t low_word: 16;
    uint32_t MS16Word;
};

class TranslateTabularContextARINC429 : public TranslateTabularParquet
{
private:

    // manages access to vectors of vectors of ICD elements
    ARINC429Data arinc_data_;

    // map providing the arinc word name at a given table_index and vector index matching the
    // arinc word data vector, vector<vector<ICDElement>>, found in element_table created by
    // Organize429ICD
    std::unordered_map<size_t,std::vector<std::string>> arinc_word_names_;

    // Vectors into which row group column data will be copied
    std::vector<int64_t> time_;       // should all be adjusted to be int64T? compare with 1553 time
    std::vector<uint16_t> channelid_;
    std::vector<uint8_t> bus_;
    std::vector<uint16_t> label_;
    std::vector<uint8_t> sdi_;
    std::vector<uint32_t> data_;
    std::vector<uint8_t> ssm_;
    std::vector<uint8_t> parity_;

    std::unordered_map<std::string, size_t> arinc_word_name_to_unique_index_map_;
    std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>> chanid_busnum_labels_;


public:
    const std::set<std::string>& translated_msg_names;

    TranslateTabularContextARINC429(ARINC429Data icd);
    virtual ~TranslateTabularContextARINC429() {}

    virtual std::map<uint32_t, std::map<uint32_t, std::set<uint16_t>>> GetChanidBusnameLabels()
    {
        return chanid_busnum_labels_;
    }

    //////////////////////////////////////////////////////////////////
    //                     Internal Functions
    //////////////////////////////////////////////////////////////////

    // See description in TranslateTabularContextBase
    virtual std::shared_ptr<TranslateTabularContextBase> Clone();

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
        icd_data        --> Vector of ICDElement objects
        table_index     --> Index relevant to a table in ICDData object
        table_map       --> Map of index to TranslatableTableBase object
                            to which is added the current table_index and
                            table object that is created.
        output_path     --> Complete output path for table data
        thread_index    --> Index of the thread in which the context is
                            processed, counting from zero for the first
                            thread and incrementing for each additional
                            thread.
        table_name      --> Name of the table which will be created. Originates
                            from the name of the ARINC 429 word, provided in DTS.
        vector_index    --> index used to access icd_data in vector<vector<ICDElement>>

    Return:
        True if no problems occur; false otherwise.
    */
    bool CreateTable(const std::vector<ICDElement>& icd_data, size_t table_index,
                    std::unordered_map<size_t, std::shared_ptr<TranslatableTableBase>>& table_map,
                    const ManagedPath& output_path, const size_t& thread_index,
                    const std::string& table_name, const size_t& vector_index);



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



    // Override of TranslateTabularContextBase. Creates a TranslatableARINC429Table
    // and adds it as the TranslatableTableBase class to the table_map.
    virtual bool CreateTranslatableTable(const std::string& name, size_t row_group_size,
                            size_t index,
                            std::unordered_map<size_t, std::shared_ptr<TranslatableTableBase>>& table_map,
                            const ManagedPath& output_path, const size_t& thread_index);


    // Similar to TranslateTabularContextBase::AppendColumn, except that it appends
    // a TranslatableExplicitSignColumn
    template <typename RawType, typename TranslatedType>
    bool AppendExplicitSignColumn(std::shared_ptr<TranslatableTableBase>& table_ptr,
                      const std::string& col_name, bool ridealong, const ICDElement& icd_elem);



    // Similar to TranslateTabularParquet::AppendTimeAndRawDataToTable with the
    // addition of the SSM argument.
    bool AppendTimeAndRawDataToTable(const size_t& thread_index,
        std::shared_ptr<TranslatableTableBase> table, const uint8_t* time_data,
        const uint8_t* raw_data, const size_t& raw_data_count,
        const std::string& table_name, int8_t sign);

};

template <typename RawType, typename TranslatedType>
bool TranslateTabularContextARINC429::AppendExplicitSignColumn(
    std::shared_ptr<TranslatableTableBase>& table_ptr,
    const std::string& col_name, bool ridealong, const ICDElement& icd_elem)
{
        if (!AppendColumnPreCheck(table_ptr))
        {
            SPDLOG_WARN("Failed AppendColumnPreCheck");
            return false;
        }

        std::shared_ptr<arrow::DataType> arrow_type = GetArrowType<TranslatedType>();
        if (arrow_type == nullptr)
        {
            SPDLOG_WARN(
                "Arrow type is nullptr, define new GetArrowType specialization "
                "or use different TranslatedType");
            return false;
        }

        std::shared_ptr<TranslatableARINC429Table> arinc_table_ptr =
            std::dynamic_pointer_cast<TranslatableARINC429Table>(table_ptr);
        if (!arinc_table_ptr->AppendExplicitSignTranslatableColumn<RawType, TranslatedType>(
                col_name, ridealong, arrow_type, icd_elem))
        {
            SPDLOG_WARN("AppendTranslatableColumn failure");
            return false;
        }
        return true;
}


#endif  // TRANSLATE_TABULAR_CONTEXT_ARINC429_H_
