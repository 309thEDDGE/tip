#ifndef TRANSLATABLE_TABLE_BASE_H
#define TRANSLATABLE_TABLE_BASE_H

#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "icd_element.h"
#include "icd_translate.h"
#include "translatable_column_base.h"
#include "translatable_column_template.h"
#include "parquet_context.h"
#include "managed_path.h"
#include "spdlog/spdlog.h"

class TranslatableTableBase
{
    /*

    TODO: Move Parquet-specific code to derived class. Modify
    existing functions which take a ParquetContext object as an
    argument by removing that argument and calling the base
    class function in the derived function.
    */

   protected:
    // Set to false if a problem occurs and the table should not
    // be used or configured further, including appending to and
    // translating columns.
    bool is_valid_;

    // Columns in the table are represented by pointers to
    // TranslatableColumnBase.
    std::vector<std::shared_ptr<TranslatableColumnBase>> columns_;

    // Track the indices of ridealong columns by their names.
    std::unordered_map<std::string, size_t> ridealong_col_name_to_index_map_;

    // Table name
    std::string name_;

    // Ouput parquet table row group size. Also the maximum count
    // of AppendRawData/AppendRidealongData calls before a row group
    // needs to be written.
    size_t row_group_size_;

    // Count of calls to AppendRawData without any row groups being written.
    // Reset when a row group is written.
    size_t append_count_;

    // Complete output file path
    ManagedPath output_path_;

    // Pointer to ParquetContext object which is created
    // by ConfigurePqContext method
    std::shared_ptr<ParquetContext> pq_ctx_;

    // Set to true if ParquetContext has been configured.
    // See ConfigurePQContext().
    bool is_pqctx_configured_;

    // Translate vectors of raw data based on configuration
    // defined in ICDElement object. Single instance of
    // of ICDTranslate saves memory and is passed to
    // TranslatableColumnBase::Translate()
    ICDTranslate icd_translate_;

    // Index identifies the thread in which the class is
    // acted upon.
    size_t thread_index_;

   public:
    const bool& is_valid;
    const bool& is_pqctx_configured;
    const std::string& name;
    const size_t& row_group_size;
    const ManagedPath& output_path;
    const size_t& thread_index;
    const std::unordered_map<std::string, size_t>& ridealong_col_name_to_index_map;

    TranslatableTableBase();
    virtual ~TranslatableTableBase();

    /*
    Get count of columns in table.

    Return:
        Size of columns_
    */
    size_t GetColumnCount() { return columns_.size(); }

    /*
    Get a pointer to a column at the given index.

    Args:
        index   --> Index in vector of desired column

    Return:
        nullptr if index is invalid or pointer to Column
    */
    virtual const std::shared_ptr<TranslatableColumnBase> GetColumnByIndex(size_t index);

    /*
    Configure table metadata, including name and row group size

    Args:
        name            --> Table name
        row_group_size  --> count of rows in a row group
        output_path     --> Complete output path for table data
        thread_index    --> Index of the thread in which the context is
                            processed, counting from zero for the first
                            thread and incrementing for each additional
                            thread.

    Return:
        True if table name is not an empty string and row_group_size is greater
        than zero; false otherwise.
    */
    virtual bool Configure(const std::string& name, size_t row_group_size,
                           const ManagedPath& output_path, const size_t& thread_index);

    /*
    Create a TranslatableColumnTemplate-derived object which is pointed
    by TranslatableColumnBase. Add the column pointer to a vector
    of pointers. This function was created to simplify testing and
    is wrapped by the other function of the same name to gain access
    to the private columns_ member.

    Args:
        col_name        --> name of column
        ridealong       --> True if column is ridealong column type
        arrow_type      --> Shared pointer to arrow output col type
        columns_vec     --> Vector of pointer to TranslatableColumnBase
                            to which newly created object will be added
        icd_elem        --> Contains information about payload source words and
                            bit configuration schema

    Return:
        True if no errors occur; false otherwise.
    */
    template <typename RawType, typename TranslatedType>
    bool AppendTranslatableColumn(const std::string& col_name, bool ridealong,
                                  std::shared_ptr<arrow::DataType> arrow_type, const ICDElement& icd_elem,
                                  std::vector<std::shared_ptr<TranslatableColumnBase>>& columns_vec);

    template <typename RawType, typename TranslatedType>
    bool AppendTranslatableColumn(const std::string& col_name, bool ridealong,
                                  std::shared_ptr<arrow::DataType> arrow_type, const ICDElement& icd_elem);

    /*
    Log the table schema as defined at the time of the call.
    */
    virtual void LogSchema();

    /*
    Append data to the raw data vectors in each of the columns.

    Args:
        data    --> Pointer to the first element of the raw data
                    from which data shall be copied into the various
                    columns
        count   --> Total count of elements in the vector
                    pointed by data

    Return:
        True if no errors occur; false otherwise.
    */
    virtual bool AppendRawData(const uint8_t* data, const size_t& count);



    /*
    Similar to AppendRawData, with the addition of explicit sign.

    Args:
        data    --> Pointer to the first element of the raw data
                    from which data shall be copied into the various
                    columns
        count   --> Total count of elements in the vector
                    pointed by data
        sign    --> Explicit sign

    Return:
        True if no errors occur; false otherwise.
    */
    virtual bool AppendRawData(const uint8_t* data, const size_t& count, int8_t sign)
    { return true; }



    /*
    Helper function to iterates over columns and calls 
    TranslatableColumnBase::AppendRawData. Facilitates overloads
    for special column types. Called in AppendRawData.

    Args:
        data    --> Pointer to the first element of the raw data
                    from which data shall be copied into the various
                    columns
        count   --> Total count of elements in the vector
                    pointed by data
    */
    virtual void _AppendToColumns(const uint8_t* data, const size_t& count); 



    /*
    Similar to above, with explicit sign included.

    Args:
        data    --> Pointer to the first element of the raw data
                    from which data shall be copied into the various
                    columns
        count   --> Total count of elements in the vector
                    pointed by data
        sign    --> Explicit sign
    */
    virtual void _AppendToColumns(const uint8_t* data, const size_t& count, int8_t sign) {}

    /*
    Helper function to simplify AppendRawData. 
    */
    virtual bool _IncrementAndTranslate();


    /*
    Append data to the raw data vectors in each of the columns.

    Args:
        data    --> Pointer to the first element of the raw data
                    from which data shall be copied into the various
                    columns
        count   --> Total count of elements in the vector
                    pointed by data
        col_name--> name of column

    Return:
        True if no errors occur; false otherwise.
    */
    virtual bool AppendRidealongColumnData(const uint8_t* data, const size_t& count,
                                           const std::string& col_name);

    /*
    Finalize ParquetContext configuration after calls to AppendTranslatableColumn. Set
    up TranslatableColumn-to-parquet schema mapping and create the output
    file. Set is_pqctx_configured_ to true, which will prevent the addition
    of columns.

    Args:
        pq_ctx      --> Pointer to ParquetContext which shall be assigned
                        to an object and configured
        columns     --> Vector of pointer to TranslatableColumnBase. The collection
                        of columns which will be used to configure ParquetContext
        is_valid    --> Boolean flag indicates obj configuration state. See
                        description of is_valid_ above.
        rg_count    --> Count of rows in a Parquet row group
        output_path --> Complete output path for table data

    Return:
        True if no errors; false otherwise. False if columns.size() equal
        to zero.
    */
    virtual bool ConfigurePqContext(std::shared_ptr<ParquetContext>& pq_ctx,
                                    const std::vector<std::shared_ptr<TranslatableColumnBase>>& columns,
                                    bool is_valid, size_t rg_count, const ManagedPath& output_path);

    virtual bool ConfigurePqContext();

    /*
    Close the pq_ctx_ output file. Note: if/when the Parquet-
    specific functionality is separated from the base class and
    moved to a derived class. This function will become a generic
    base class virtual function.
    */
    virtual void CloseOutputFile();
};

template <typename RawType, typename TranslatedType>
bool TranslatableTableBase::AppendTranslatableColumn(
    const std::string& col_name, bool ridealong,
    std::shared_ptr<arrow::DataType> arrow_type, const ICDElement& icd_elem,
    std::vector<std::shared_ptr<TranslatableColumnBase>>& columns_vec)
{
    if (!is_valid_)
    {
        SPDLOG_WARN(
            "{:02d} Table has not been configured or is not "
            "valid/has experience some other error",
            thread_index_);
        return false;
    }

    std::shared_ptr<TranslatableColumnTemplate<RawType, TranslatedType>> col =
        std::make_shared<TranslatableColumnTemplate<RawType, TranslatedType>>();
    if (!col->Configure(col_name, ridealong, arrow_type, row_group_size_,
                        icd_elem, thread_index_))
    {
        SPDLOG_WARN("{:02d} Failed to configure new column", thread_index_);
        is_valid_ = false;
        return false;
    }

    std::shared_ptr<TranslatableColumnBase> base_col = col;
    columns_vec.push_back(base_col);

    // If it's a ridealong column. Add it's index in columns_ to the map
    if (ridealong)
    {
        ridealong_col_name_to_index_map_[col_name] = columns_vec.size() - 1;
    }

    return true;
}

template <typename RawType, typename TranslatedType>
bool TranslatableTableBase::AppendTranslatableColumn(
    const std::string& col_name, bool ridealong,
    std::shared_ptr<arrow::DataType> arrow_type, const ICDElement& icd_elem)
{
    is_valid_ = AppendTranslatableColumn<RawType, TranslatedType>(
        col_name, ridealong, arrow_type, icd_elem, columns_);
    return is_valid_;
}
#endif  // #ifndef TRANSLATABLE_TABLE_BASE_H