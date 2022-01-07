#ifndef TRANSLATE_TABULAR_CONTEXT_ARINC429_H_
#define TRANSLATE_TABULAR_CONTEXT_ARINC429_H_

#include <string>
#include <memory>
#include <cstdint>
#include <vector>


/*
Include parquet_reader.h first
*/
#include "parquet_reader.h"
#include "translate_tabular_context_base.h"
#include "translate_tabular_parquet.h"
#include "translatable_table_base.h"
#include "managed_path.h"
#include "spdlog/spdlog.h"

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

    // Vectors into which row group column data will be copied
    std::vector<uint64_t> time_;
    std::vector<uint16_t> channelid_;
    std::vector<uint8_t> bus_;
    std::vector<uint16_t> label_;
    std::vector<uint8_t> sdi_;
    std::vector<uint32_t> data_;
    std::vector<uint8_t> ssm_;
    std::vector<uint8_t> parity_;

public:
    TranslateTabularContextARINC429();
    virtual ~TranslateTabularContextARINC429() {}


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
    // bool CreateTable(const ICDData& icd_data, size_t table_index,
    //                  std::unordered_map<size_t, std::shared_ptr<TranslatableTableBase>>& table_map,
    //                  const ManagedPath& output_path, const size_t& thread_index);



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

};


#endif  // TRANSLATE_TABULAR_CONTEXT_ARINC429_H_
