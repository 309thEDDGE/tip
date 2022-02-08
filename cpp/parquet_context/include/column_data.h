#ifndef COLUMNDATA_H
#define COLUMNDATA_H

#include <arrow/api.h>
#include <string>
#include <vector>
#include <typeinfo>
#include <memory>
#include "spdlog/spdlog.h"

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

class InputDataBase
{
    private:
        void PrintNotImplWarning(const char* type_name) const 
        { SPDLOG_ERROR("CopyToCompatible NOT IMPLEMENTED for type: {:s}!", type_name); }

    protected:

        // Pointer to return which can be casted as needed
        const void* data_ptr_;

    public:
        // const void* const data_ptr; 
        InputDataBase() : data_ptr_(nullptr) {}
        virtual ~InputDataBase() {}

        virtual void CopyToCompatible(int8_t* to_data_ptr, const int& size,
            const int& offset) const {PrintNotImplWarning(typeid(int8_t).name());}
        virtual void CopyToCompatible(double* to_data_ptr, const int& size,
            const int& offset) const { PrintNotImplWarning(typeid(double).name()); }
        virtual void CopyToCompatible(float* to_data_ptr, const int& size,
            const int& offset) const {PrintNotImplWarning(typeid(float).name());}

        virtual void CopyToCompatible(int16_t* to_data_ptr, const int& size,
            const int& offset) const {}
        virtual void CopyToCompatible(int32_t* to_data_ptr, const int& size,
            const int& offset) const {}
        virtual void CopyToCompatible(int64_t* to_data_ptr, const int& size,
            const int& offset) const {}
        virtual void CopyToCompatible(uint16_t* to_data_ptr, const int& size,
            const int& offset) const {PrintNotImplWarning(typeid(uint16_t).name());}
        virtual void CopyToCompatible(uint32_t* to_data_ptr, const int& size,
            const int& offset) const {PrintNotImplWarning(typeid(uint32_t).name());}
        virtual void CopyToCompatible(uint64_t* to_data_ptr, const int& size,
            const int& offset) const {PrintNotImplWarning(typeid(uint64_t).name());}
        virtual void CopyToCompatible(uint8_t* to_data_ptr, const int& size,
            const int& offset) const {PrintNotImplWarning(typeid(uint8_t).name());}


};

template<typename T>
class InputData : public InputDataBase
{
    private:
        const T* const orig_data_ptr_;
    
    public:
        InputData(const T* const input_ptr) : orig_data_ptr_(input_ptr)
        {
            data_ptr_ = reinterpret_cast<const void*>(input_ptr);
        }
        virtual ~InputData() {}

        const void* const Data() const 
        { return data_ptr_; }

        virtual void CopyToCompatible(int8_t* to_data_ptr, const int& size, 
            const int& offset) const 
        {
            for(size_t i = 0; i < size; i++)
            {
                to_data_ptr[i] = static_cast<int8_t>(orig_data_ptr_[offset+i]);
            }
        }
        virtual void CopyToCompatible(int16_t* to_data_ptr, const int& size, 
            const int& offset) const 
        {
            for(size_t i = 0; i < size; i++)
            {
                to_data_ptr[i] = static_cast<int16_t>(orig_data_ptr_[offset+i]);
            }
        }
        virtual void CopyToCompatible(int32_t* to_data_ptr, const int& size, 
            const int& offset) const 
        {
            for(size_t i = 0; i < size; i++)
            {
                to_data_ptr[i] = static_cast<int32_t>(orig_data_ptr_[offset+i]);
            }
        }
        virtual void CopyToCompatible(int64_t* to_data_ptr, const int& size, 
            const int& offset) const
        {
            for(size_t i = 0; i < size; i++)
            {
                to_data_ptr[i] = static_cast<int64_t>(orig_data_ptr_[offset+i]);
            }
        }
};

class ColumnData
{
   private:
    std::shared_ptr<InputDataBase> input_data_;
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

    ColumnData() : type_(nullptr), pointer_set_(false), ready_for_write_(false), cast_from_(CastFromType::TypeNONE), builder_(), list_builder_(), initial_max_row_size_(0), input_data_(nullptr), field_name_("") {}

    ColumnData(std::shared_ptr<arrow::DataType> type, std::string fieldName, std::string typeID,
               int byteSize, int listSize = 0) : field_name_(fieldName), type_ID_(typeID), byte_size_(byteSize), type_(type), pointer_set_(false), ready_for_write_(false), cast_from_(CastFromType::TypeNONE), builder_(), list_builder_(), initial_max_row_size_(0), input_data_(nullptr)
    {
        if (listSize == 0)
        {
            is_list_ = false;
            list_size_ = 0;
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

    void SetInputData(std::shared_ptr<InputDataBase> input_data)
    {
        input_data_ = input_data;
    }

    const std::shared_ptr<InputDataBase> GetInputData() const
    { return input_data_; }

    ~ColumnData() {}
};

#endif
