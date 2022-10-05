#ifndef TRANSLATE_TABULAR_CONTEXT_BASE_H_
#define TRANSLATE_TABULAR_CONTEXT_BASE_H_

// Include "translatable_table_base.h" first!
#include "translatable_table_base.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "translate_status.h"
#include "managed_path.h"
#include "spdlog/spdlog.h"

class TranslateTabularContextBase
{
   protected:
    // Hold the pointers to the tables to which raw and translated data will be written.
    std::unordered_map<size_t, std::shared_ptr<TranslatableTableBase>> table_index_to_table_map_;

    // Ridealong column are those which ought to
    // be copied without translation from parsed,
    // aka "raw", tables into output translated data tables.
    std::vector<std::string> ridealong_col_names_;

    // Raw table col names are the names of the columns
    // from the raw data tables which will be used during
    // translation. This includes message/type identification
    // and column(s) which contain the raw data to be
    // translated.
    std::vector<std::string> data_col_names_;

   public:
    // const std::string& input_file_extension;
    // const size_t& raw_data_word_size;
    const std::vector<std::string>& ridealong_col_names;
    const std::vector<std::string>& data_col_names;

    TranslateTabularContextBase() : ridealong_col_names(ridealong_col_names_),
                                    data_col_names(data_col_names_) {}
    virtual ~TranslateTabularContextBase();

    /*
    Copy object and return pointer to new object. Override
    in derived class.
    */
    virtual std::shared_ptr<TranslateTabularContextBase> Clone();

    /*
    Check for required configuration:
        - Configure has been called with non-empty string and non-zero
          word size
        - SetColumnNames has been called and data_col_names has size > 0.
          ridealong_col_names is not required and is therefore not checked.

    Return:
        True if configured as expected, false otherwise.
    */
    virtual bool IsConfigured();

    /*
    Set relevant column names. Identifies names of columns from
    the input files tables which shall be used during translation.

    Args:
        ridealong_col_names --> names of columns which shall be created
                                as ridealong columns in the final
                                translated data table
        data_col_names      --> names of all columns which shall be used
                                during identification of message/type,
                                including raw data columns which shall be
                                translated
    */
    virtual void SetColumnNames(const std::vector<std::string>& ridealong_col_names,
                                const std::vector<std::string>& data_col_names);

    /*
    Open one input file in preparation for reading contents.

    Args:
        input_path  --> Path to inpute file
        thread_index--> Index of the thread in which the context is
                        processed, counting from zero for the first
                        thread and incrementing for each additional
                        thread.

    Return:
        Status indicating explicitly if translation for loop should
        'continue', proceed, or fail.
   */
    virtual TranslateStatus OpenInputFile(const ManagedPath& input_path,
                                          const size_t& thread_index)
    {
        return TranslateStatus::OK;
    }

    /*
    Open output file in preparation for writing data.

    Args:
        output_dir      --> Base output dir
        output_base_name--> base name of output file
        thread_index--> Index of the thread in which the context is
                        processed, counting from zero for the first
                        thread and incrementing for each additional
                        thread.


    Return:
        Status indicating explicitly if translation for loop should
        'continue', proceed, or fail.
    */
    virtual TranslateStatus OpenOutputFile(const ManagedPath& output_dir,
                                           const ManagedPath& output_base_name, const size_t& thread_index)
    {
        return TranslateStatus::OK;
    }

    /*
    Ingest a single input file.

    Args:
        thread_index--> Index of the thread in which the context is
                        processed, counting from zero for the first
                        thread and incrementing for each additional
                        thread.

    Return:
        Status indicating explicitly if translation for loop should
        'continue', proceed, or fail.
    */
    virtual TranslateStatus ConsumeFile(const size_t& thread_index)
    {
        return TranslateStatus::OK;
    }

    /*
    Close input file

    Args:
        thread_index--> Index of the thread in which the context is
                        processed, counting from zero for the first
                        thread and incrementing for each additional
                        thread.

    Return:
        Status indicating explicitly if translation for loop should
        'continue', proceed, or fail.
    */
    virtual TranslateStatus CloseInputFile(const size_t& thread_index)
    {
        return TranslateStatus::OK;
    }

    /*
    Close output file

    Args:
        thread_index--> Index of the thread in which the context is
                        processed, counting from zero for the first
                        thread and incrementing for each additional
                        thread.
        is_final_file-> Set to true if the last file has completed
                        processing

    Return:
        Status indicating explicitly if translation for loop should
        'continue', proceed, or fail.
    */
    virtual TranslateStatus CloseOutputFile(const size_t& thread_index,
                                            bool is_final_file)
    {
        return TranslateStatus::OK;
    }

    /*
    Create an object which derives from TranslatableTableBase and
    store it's shared_ptr in a vector. Create the table columns
    from input arguments

    Args:
        name            --> Table name
        row_group_size  --> count of rows in a row group
        index           --> Index which identifies the table
        table_map       --> Map of index to TranslatableTableBase object
                            to which is added the current table_index and
                            table object that is created.
        output_path     --> Complete output path for table data
        thread_index    --> Index of the thread in which the context is
                            processed, counting from zero for the first
                            thread and incrementing for each additional
                            thread.

    Return:
        True if no problems occur; false otherwise.
    */
    virtual bool CreateTranslatableTable(const std::string& name, size_t row_group_size,
                                         size_t index,
                                         std::unordered_map<size_t, std::shared_ptr<TranslatableTableBase>>& table_map,
                                         const ManagedPath& output_path, const size_t& thread_index);

    /*
    Append a column to a table.

    Args:
        table_ptr       --> Pointer to table to which a column should be appended
        col_name        --> name of column
        ridealong       --> True if column is ridealong column type
        icd_elem        --> Contains information about payload source words and
                            bit configuration schema

    Return:
        True if no errors occur; false otherwise.
    */
    template <typename RawType, typename TranslatedType>
    bool AppendColumn(std::shared_ptr<TranslatableTableBase>& table_ptr,
                      const std::string& col_name, bool ridealong, const ICDElement& icd_elem);

    /*
    Append a column whose translated data shall be stored in a uint8_t vector
    and written to a an arrow::boolean() column.
    AppendColumn<RawType, bool> can't be used because both the data
    are read as bit values, not boolean, and std::vector<bool> does not
    implement the data() member function which is used by ParquetContext.

    The TranslatedType found in AppendColumn is implicitly defined to
    be uint8_t. This function is otherwise a mirror of AppendColumn.
    */
    template <typename RawType>
    bool AppendBooleanColumn(std::shared_ptr<TranslatableTableBase>& table_ptr,
                             const std::string& col_name, bool ridealong, const ICDElement& icd_elem);

    template <typename RawType>
    bool AppendASCIIColumn(std::shared_ptr<TranslatableTableBase>& table_ptr,
                           const std::string& col_name, bool ridealong, const ICDElement& icd_elem);

    /*
    Conduct the following checks in preparation for adding columns:
        - table_ptr is not nullptr
        - Context (this) IsConfigured()
        - pointed table is configured (is_valid == true)
    This function is a helper function for AppendColumn.

    Args:
        table_ptr       --> Pointer to table to which a column should be appended

    Return:
        True if no errors occur; false otherwise.
    */
    bool AppendColumnPreCheck(std::shared_ptr<TranslatableTableBase>& table_ptr);

    /*
    Get a shared ptr to arrow datatype

    Return:
        shared_ptr to Arrow datatype. nullptr if template specialization
        for the given template argument has not been defined.
    */
    template <typename TranslatedType>
    std::shared_ptr<arrow::DataType> GetArrowType();
};

template <typename RawType, typename TranslatedType>
bool TranslateTabularContextBase::AppendColumn(
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

    if (!table_ptr->AppendTranslatableColumn<RawType, TranslatedType>(
            col_name, ridealong, arrow_type, icd_elem))
    {
        SPDLOG_WARN("AppendTranslatableColumn failure");
        return false;
    }
    return true;
}

template <typename RawType>
bool TranslateTabularContextBase::AppendBooleanColumn(
    std::shared_ptr<TranslatableTableBase>& table_ptr, const std::string& col_name,
    bool ridealong, const ICDElement& icd_elem)
{
    if (!AppendColumnPreCheck(table_ptr))
    {
        SPDLOG_WARN("Failed AppendColumnPreCheck");
        return false;
    }

    std::shared_ptr<arrow::DataType> arrow_type = GetArrowType<bool>();
    if (arrow_type == nullptr)
    {
        SPDLOG_WARN(
            "Arrow type is nullptr, define new GetArrowType specialization "
            "or use different TranslatedType");
        return false;
    }

    if (!table_ptr->AppendTranslatableColumn<RawType, uint8_t>(
            col_name, ridealong, arrow_type, icd_elem))
    {
        SPDLOG_WARN("AppendTranslatableColumn failure");
        return false;
    }
    return true;
}

template <typename RawType>
bool TranslateTabularContextBase::AppendASCIIColumn(
    std::shared_ptr<TranslatableTableBase>& table_ptr, const std::string& col_name,
    bool ridealong, const ICDElement& icd_elem)
{
    if (!AppendColumnPreCheck(table_ptr))
    {
        SPDLOG_WARN("Failed AppendColumnPreCheck");
        return false;
    }
    // int8_t ==> int8_t or char ==> utf8?
    std::shared_ptr<arrow::DataType> arrow_type = GetArrowType<int8_t>();
    if (arrow_type == nullptr)
    {
        SPDLOG_WARN(
            "Arrow type is nullptr, define new GetArrowType specialization "
            "or use different TranslatedType");
        return false;
    }

    if (!table_ptr->AppendTranslatableColumn<RawType, int8_t>(
            col_name, ridealong, arrow_type, icd_elem))
    {
        SPDLOG_WARN("AppendTranslatableColumn failure");
        return false;
    }
    return true;
}

template <typename TranslatedType>
std::shared_ptr<arrow::DataType> TranslateTabularContextBase::GetArrowType()
{
    return nullptr;
}

template <>
inline std::shared_ptr<arrow::DataType> TranslateTabularContextBase::GetArrowType<bool>()
{
    return arrow::boolean();
}

template <>
inline std::shared_ptr<arrow::DataType> TranslateTabularContextBase::GetArrowType<int8_t>()
{
    return arrow::int8();
}

template <>
inline std::shared_ptr<arrow::DataType> TranslateTabularContextBase::GetArrowType<int16_t>()
{
    return arrow::int16();
}

template <>
inline std::shared_ptr<arrow::DataType> TranslateTabularContextBase::GetArrowType<uint8_t>()
{
    return arrow::int16();
}

template <>
inline std::shared_ptr<arrow::DataType> TranslateTabularContextBase::GetArrowType<int32_t>()
{
    return arrow::int32();
}

template <>
inline std::shared_ptr<arrow::DataType> TranslateTabularContextBase::GetArrowType<uint16_t>()
{
    return arrow::int32();
}

template <>
inline std::shared_ptr<arrow::DataType> TranslateTabularContextBase::GetArrowType<int64_t>()
{
    return arrow::int64();
}

template <>
inline std::shared_ptr<arrow::DataType> TranslateTabularContextBase::GetArrowType<uint64_t>()
{
    return arrow::int64();
}

template <>
inline std::shared_ptr<arrow::DataType> TranslateTabularContextBase::GetArrowType<float>()
{
    return arrow::float32();
}

template <>
inline std::shared_ptr<arrow::DataType> TranslateTabularContextBase::GetArrowType<double>()
{
    return arrow::float64();
}

template <>
inline std::shared_ptr<arrow::DataType> TranslateTabularContextBase::GetArrowType<char>()
{
    return arrow::utf8();
}
#endif  // #define TRANSLATE_TABULAR_CONTEXT_BASE_H_