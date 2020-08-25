#ifndef COLUMNDATA_H
#define COLUMNDATA_H

#include<string>
#include<vector>
#include <arrow/api.h>
#include <typeinfo>

class ColumnData
{
	public:
		ColumnData() : type_(nullptr), pointer_set_(false), ready_for_write_(false), cast_from_(""),
			builder_(), list_builder_(), initial_max_row_size_(0) {}

		ColumnData(std::shared_ptr<arrow::DataType> type, std::string fieldName, std::string typeID ,
			int byteSize, int listSize = NULL) : field_name_(fieldName), type_ID_(typeID), byte_size_(byteSize),
			type_(type), pointer_set_(false), ready_for_write_(false), cast_from_(""), builder_(), 
			list_builder_(), initial_max_row_size_(0)
		{
			if (listSize == NULL){
				is_list_ = false;
				list_size_ = NULL;
			}
			else {
				is_list_ = true;
				list_size_ = listSize;
			}
		}
		
		void SetColumnData(void* data, std::string name, std::string castFrom, uint8_t* valid, int initialRowSize){
			data_ = data;
			field_name_ = name;
			str_ptr_ = nullptr;
			cast_from_ = castFrom;
			if (valid == nullptr) {
				null_values_ = nullptr;
			} else {
				null_values_ = valid;
			}
			pointer_set_ = true;
			if (is_list_)
				initial_max_row_size_ = initialRowSize / list_size_;
			else
				initial_max_row_size_ = initialRowSize;
		}

		void SetColumnData(std::vector<std::string>& data, std::string name, uint8_t* valid, int initialRowSize) {
			data_ = nullptr;
			str_ptr_ = &data;
			cast_from_ = "";
			field_name_ = name;
			if (valid == nullptr) {
				null_values_ = nullptr;
			}
			else {
				null_values_ = valid;
			}
			pointer_set_ = true;
			if (is_list_)
				initial_max_row_size_ = initialRowSize / list_size_;
			else
				initial_max_row_size_ = initialRowSize;

		}
		~ColumnData() {};
		
		std::string field_name_;
		std::shared_ptr<arrow::DataType> type_;
		void* data_;
		uint8_t* null_values_;
		std::vector<std::string>* str_ptr_;
		bool is_list_;
		int list_size_;
		int initial_max_row_size_;
		bool pointer_set_;
		std::shared_ptr<arrow::ArrayBuilder> builder_;
		std::shared_ptr<arrow::ListBuilder> list_builder_;
		bool ready_for_write_;
		std::string type_ID_;
		int byte_size_;
		std::string cast_from_;
};

#endif

