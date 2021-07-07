#ifndef COLUMNDATA_H
#define COLUMNDATA_H

#include <arrow/api.h>
#include <string>
#include <vector>
#include <typeinfo>
#include <memory>

enum CastFromType
{
    TypeUINT64,
    TypeINT64,
    TypeUINT32,
    TypeINT32,
    TypeUINT16,
    TypeINT16,
    TypeUINT8,
    TypeINT8,
    TypeNONE,
};

class ColumnData
{
   private:
    void SetTypeEnum(std::string name)
    {
        if (name == typeid(int8_t).name())
        {
            cast_from_ = CastFromType::TypeINT8;
        }
        else if (name == typeid(uint8_t).name())
        {
            cast_from_ = CastFromType::TypeUINT8;
        }
        else if (name == typeid(int16_t).name())
        {
            cast_from_ = CastFromType::TypeINT16;
        }
        else if (name == typeid(uint16_t).name())
        {
            cast_from_ = CastFromType::TypeUINT16;
        }
        else if (name == typeid(int32_t).name())
        {
            cast_from_ = CastFromType::TypeINT32;
        }
        else if (name == typeid(uint32_t).name())
        {
            cast_from_ = CastFromType::TypeUINT32;
        }
        else if (name == typeid(int64_t).name())
        {
            cast_from_ = CastFromType::TypeINT64;
        }
        else if (name == typeid(uint64_t).name())
        {
            cast_from_ = CastFromType::TypeUINT64;
        }
        else
        {
            cast_from_ = CastFromType::TypeNONE;
        }
    }

   public:
    std::string field_name_;
    std::shared_ptr<arrow::DataType> type_;
    void* data_;
    std::vector<uint8_t>* null_values_;
    std::vector<std::string>* str_ptr_;
    bool is_list_;
    int list_size_;
    int initial_max_row_size_;
    bool pointer_set_;
    std::shared_ptr<arrow::ArrayBuilder> builder_;
    std::shared_ptr<arrow::ListBuilder> list_builder_;
    bool ready_for_write_;
    std::string type_ID_;  //intended output type (arrow type)
    int byte_size_;
    CastFromType cast_from_;

    ColumnData() : type_(nullptr), pointer_set_(false), ready_for_write_(false), cast_from_(CastFromType::TypeNONE), builder_(), list_builder_(), initial_max_row_size_(0) {}

    ColumnData(std::shared_ptr<arrow::DataType> type, std::string fieldName, std::string typeID,
               int byteSize, int listSize = NULL) : field_name_(fieldName), type_ID_(typeID), byte_size_(byteSize), type_(type), pointer_set_(false), ready_for_write_(false), cast_from_(CastFromType::TypeNONE), builder_(), list_builder_(), initial_max_row_size_(0)
    {
        if (listSize == NULL)
        {
            is_list_ = false;
            list_size_ = NULL;
        }
        else
        {
            is_list_ = true;
            list_size_ = listSize;
        }
    }

    void SetColumnData(void* data, std::string name,
                       std::string castFrom,
                       std::vector<uint8_t>* boolField,
                       int initialRowSize)
    {
        data_ = data;
        field_name_ = name;
        str_ptr_ = nullptr;
        SetTypeEnum(castFrom);
        if (boolField == nullptr)
        {
            null_values_ = nullptr;
        }
        else
        {
            null_values_ = boolField;
        }
        pointer_set_ = true;
        if (is_list_)
            initial_max_row_size_ = initialRowSize / list_size_;
        else
            initial_max_row_size_ = initialRowSize;
    }

    void SetColumnData(std::vector<std::string>& data,
                       std::string name,
                       std::vector<uint8_t>* boolField)
    {
        data_ = nullptr;
        str_ptr_ = &data;
        cast_from_ = CastFromType::TypeNONE;
        field_name_ = name;
        if (boolField == nullptr)
        {
            null_values_ = nullptr;
        }
        else
        {
            null_values_ = boolField;
        }
        pointer_set_ = true;
        if (is_list_)
            initial_max_row_size_ = data.size() / list_size_;
        else
            initial_max_row_size_ = data.size();
    }

    ~ColumnData(){}
};

#endif
