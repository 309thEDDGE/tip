#ifndef FIELD_INFO_H
#define FIELD_INFO_H

#include<string>
#include<vector>
#include <arrow/api.h>
#include <typeinfo>

class Field_Info
{
	public:
		Field_Info() : pointerSet(false), readyForWrite(false), castFrom("") {}
		Field_Info(std::shared_ptr<arrow::DataType> type_, std::string fieldName_, std::string typeID_ , 
			int byteSize_, int listSize_ = NULL) : fieldName(fieldName_), typeID(typeID_), byteSize(byteSize_),
			type(type_), pointerSet(false), readyForWrite(false), builder(), list_builder()
		{
			if (listSize_ == NULL){
				isList = false;
				listSize = NULL;
			}
			else {
				isList = true;
				listSize = listSize_;
			}
		}
		
		void set_Field_Info(void* data_, std::string name, std::string castFrom_, uint8_t* valid_){
			data = data_;
			fieldName = name;
			strPtr = nullptr;
			castFrom = castFrom_;
			if (valid_ == nullptr) {
				nullValues = nullptr;
			} else {
				nullValues = valid_;
			}
			pointerSet = true;
		}

		void set_Field_Info(std::vector<std::string>& data_, std::string name, uint8_t* valid_) {
			data = nullptr;
			strPtr = &data_;
			castFrom = "";
			fieldName = name;
			if (valid_ == nullptr) {
				nullValues = nullptr;
			}
			else {
				nullValues = valid_;
			}
			pointerSet = true;
		}
		~Field_Info() {};
		
		std::string fieldName;
		std::shared_ptr<arrow::DataType> type;
		void* data;
		uint8_t* nullValues;
		std::vector<std::string>* strPtr;
		bool isList;
		int listSize;
		bool pointerSet;
		std::shared_ptr<arrow::ArrayBuilder> builder;
		std::shared_ptr<arrow::ListBuilder> list_builder;
		bool readyForWrite;
		std::string typeID;
		int byteSize;
		std::string castFrom;
};

#endif

