#ifndef TRANSLATABLE_COLUMN_BASE_H
#define TRANSLATABLE_COLUMN_BASE_H

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include "parquet_context.h"
#include "icd_element.h"
#include "icd_translate.h"
#include "spdlog/spdlog.h"

class TranslatableColumnBase
{
   protected:
    // True if the column represents a ridealong column, which is a col
    // that does not require translation and the values are directly copied
    // one-to-one.
    bool is_ridealong_;

    // Account for raw and translated value appended to vectors
    size_t raw_data_append_count_;
    size_t translated_data_append_count_;

    // Name of the column/field.
    std::string col_name_;

    // True if setMemoryLocation() of ParquetContext has been called
    // for this column.
    bool have_set_memory_location_;

    // Arrow type to be used to write to parquet. To be determined
    // automatically based on translated data type.
    std::shared_ptr<arrow::DataType> arrow_type_;

    // Contains information about source data word size and bit
    // configuration, and position of relevant words within the
    // the source payload. Also to be used during the translation
    // stage, converting from raw words to engineering units (EU)
    ICDElement icd_elem_;

    // Raw data word size (ex: 2 for uint8_t, 4 for float)
    size_t raw_data_word_size_;

    // Count of raw data words to be copied into the raw
    // data vector for each value.
    size_t words_per_translated_value_;

    // Offset within the data payload from which words_per_translated_value_
    // count of raw words are copied. Offset 0 copies the first word and
    // any subsequent words. Offset 1 copies the second word, etc.
    size_t payload_offset_;

    // Index identifies the thread in which the class is
    // acted upon.
    size_t thread_index_;

   public:
    const std::string& col_name;
    const bool& is_ridealong;
    const size_t& words_per_translated_value;
    const size_t& payload_offset;
    const size_t& raw_data_append_count;
    const size_t& translated_data_append_count;
    const size_t& thread_index;

    TranslatableColumnBase();
    virtual ~TranslatableColumnBase() {}
    std::string ColName() { return col_name_; }

    /*
    Configure column metadata

    Args:
        col_name        --> name of column
        ridealong       --> True if column is ridealong column type
        arrow_type      --> Shared pointer to arrow output col type
        icd_elem        --> Contains information about payload source words and 
                            bit configuration schema
        thread_index    --> Index of the thread in which the context is
                            processed, counting from zero for the first
                            thread and incrementing for each additional
                            thread.
       
    Return:
        True if col_name is not an empty string.
    */
    virtual bool Configure(const std::string& name, bool ridealong,
                           std::shared_ptr<arrow::DataType> arrow_type,
                           const ICDElement& icd_elem, const size_t& thread_index);

    // virtual size_t GetRawDataAppendCount() { return raw_data_append_count_; }


    /*
    Get const ref to ICDElem.

    Return:
        ICDElem object
    */
   virtual const ICDElement& GetICDElement() { return icd_elem_; }

    /*
    Get raw data vector size.

    Return:
        value of size() method
    */
    virtual size_t GetRawDataVectorSize() { return size_t(0); }

    /*
    Get translated data vector size.

    Return:
        value of size() method
    */
    virtual size_t GetTranslatedDataVectorSize() { return size_t(0); }

    /*
    Given a pointer to a ParquetContext object, add the field corresponding
    to the column instance and call the SetMemoryLocation method.

    Args:
        pq_context  --> Pointer to configured ParquetContext object
    */
    virtual void ConfigureParquetContext(std::shared_ptr<ParquetContext>& pq_context)
    {
    }

    /*
    Log the column schema as defined at the time of the call.
    */
    virtual void LogSchema();

    /*
    Append raw/untranslated data to the raw data vector.

    Args:
        data    --> Pointer to data "payload" from which
                    data will be copied.
        count   --> Total count of elements in the vector
                    pointed by data

    Return:
        True if no errors; false otherwise.
    */
    virtual bool AppendRawData(const uint8_t* data, const size_t& count)
    {
        return true;
    }



    /*
    Overload of AppendRawData with explicit sign.

    Args:
        data    --> Pointer to data "payload" from which
                    data will be copied.
        count   --> Total count of elements in the vector
                    pointed by data

    Return:
        True if no errors; false otherwise.
    */
    virtual bool AppendRawData(const uint8_t* data, const size_t& count, int8_t sign)
    {
        return true;
    }



    /*
    Append ridealong data to column.

    Args:
        data    --> Pointer to data "payload" from which
                    data will be copied.
        count   --> Total count of elements in the vector
                    pointed by data

    Return:
        True if no errors; false otherwise.
    */
    virtual bool AppendRidealongData(const uint8_t* data, const size_t& count)
    {
        return true;
    }

    /*
    Translate column data based on configuration in ICDElement member.

    Args:
        icd_translate   --> ICDTranslate object which carries out translation

    Return:
        True if no errors occur and data are translated as expected; false
        otherwise.
    */
    virtual bool Translate(ICDTranslate& icd_translate)
    {
        return true;
    }
};

#endif  // #ifndef TRANSLATABLE_COLUMN_BASE_H