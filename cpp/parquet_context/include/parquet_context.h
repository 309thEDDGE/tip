#ifndef PARQUET_CONTEXT_H
#define PARQUET_CONTEXT_H

#include <vector>
#include <cstdint>
#include <stdio.h>
#include <map>
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/writer.h>
#include <parquet/arrow/schema.h>
#include <typeinfo>

#include "column_data.h"

/*

!!! NOTICE !!!

The preprocessor macros ARROW_STATIC and PARQUET_STATIC must be defined
if linking to the library statically.

!!! NOTICE !!!

*/

class ParquetContext
{
private:
	std::string host_;
	std::string user_;
	std::string path_;
	std::vector<std::string> temp_string_vec_;
	int port_;
	bool truncate_;
	arrow::Status st_;
	bool have_created_writer_;
	bool have_created_schema_;
	arrow::MemoryPool* pool_;
	bool parquet_stop_;
	std::unique_ptr<parquet::arrow::FileWriter> writer_;
	int ROW_GROUP_COUNT_;
	uint8_t ret_;
	int append_row_count_;
	bool have_created_table_;
	std::map<std::string, ColumnData> column_data_map_;
	std::shared_ptr<arrow::Schema> schema_;
	std::shared_ptr<arrow::io::FileOutputStream> ostream_;
	std::shared_ptr<parquet::WriterProperties> props_;
	std::shared_ptr<arrow::io::ReadableFile> input_file_;
	std::vector<std::shared_ptr<arrow::Field>> fields_;

	std::unique_ptr<arrow::ArrayBuilder> 
		GetBuilderFromDataType(
			const std::shared_ptr<arrow::DataType> dtype,
			const bool& is_list_builder);

	bool AppendColumn(ColumnData& dataInfo, 
		const int& rows, 
		const int offset = 0);

	template<typename T, typename A>
	void Append(const bool& isList,
		const bool& castRequired,
		const int& listCount,
		const int& offset, 
		const ColumnData& columnData);

	void CreateBuilders();

	std::vector<int32_t> GetOffsetsVector(const int& n_rows, 
		const int& elements_per_row, 
		const int offset = 0);

	void WriteColsIfReady();

	void FillStringVec(std::vector<std::string>* str_data_vec_ptr, 
		const int& count,
		const int offset = 0);	

	bool IsUnsigned(const std::shared_ptr<arrow::DataType> type);

	std::string GetTypeIDFromArrowType(const std::shared_ptr<arrow::DataType> type,
		int& byteSize);

	template<typename castType> void CastTo(const void const * data,
		std::vector<castType>& buildVec,
		const std::string& castFrom,
		const int& size,
		const int& offset);
	

public:
	ParquetContext();
	ParquetContext(int rgSize);
	~ParquetContext();

	void Close();

	uint8_t OpenForWrite(const std::string& path,
		const bool& truncate);

	void AddField(const std::shared_ptr<arrow::DataType> type, 
		const std::string& fieldName, 
		const int listSize=NULL);

	bool WriteColumns(const int& rows, const int offset=0);
	
	bool WriteColumns();

	template<typename NativeType> 
	bool SetMemoryLocation(std::vector<NativeType>& data, 
		const std::string&, 
		uint8_t* boolField=nullptr);

	template<typename NativeType>
	bool SetMemoryLocation(std::vector<std::string>& strVec, 
		const std::string& fieldName, 
		uint8_t* boolField=nullptr);
};

template<typename T, typename A>
void ParquetContext::Append(const bool& isList, 
	const bool& castRequired, 
	const int& listCount, 
	const int& offset,  
	const ColumnData& columnData)
{
	if (isList)
	{
		// Get the relevant builder for the data type.
		std::shared_ptr<arrow::ListBuilder> bldr =
			std::dynamic_pointer_cast<arrow::ListBuilder>(columnData.builder_);

		// Resize array to allocate space and append data.
		bldr->Resize(append_row_count_);

		if (castRequired)
		{
			std::vector<int32_t> offsets_vec = 
				GetOffsetsVector(append_row_count_, listCount, 0);

			bldr->AppendValues(offsets_vec.data(), append_row_count_);

			A* sub_bldr =
				static_cast<A*>(bldr->value_builder());

			std::vector<T> castVec(append_row_count_ * listCount);

			CastTo<T>(columnData.data_, 
				castVec, 
				columnData.cast_from_, 
				append_row_count_ * listCount, 
				offset * listCount);

			sub_bldr->AppendValues(castVec.data(), 
				append_row_count_ * listCount);
		}
		else
		{
			std::vector<int32_t> offsets_vec = 
				GetOffsetsVector(append_row_count_, listCount, 0);

			bldr->AppendValues(offsets_vec.data(), append_row_count_);
			A* sub_bldr =
				static_cast<A*>(bldr->value_builder());
			sub_bldr->AppendValues((T*)columnData.data_ + (offset * listCount), 
				append_row_count_ * listCount);
		}
	}
	else 
	{
		std::shared_ptr<A> bldr =
			std::dynamic_pointer_cast<A>(columnData.builder_);

		// Resize array to allocate space and append data.
		bldr->Resize(append_row_count_);
		if (columnData.null_values_ == nullptr) 
		{
			if (castRequired) 
			{
				std::vector<T> castVec(append_row_count_);
				CastTo<T>(columnData.data_, 
					castVec, 
					columnData.cast_from_, 
					append_row_count_, 
					offset);

				bldr->AppendValues(castVec.data(), append_row_count_);
			}
			else
				bldr->AppendValues(((T*)columnData.data_) + offset, 
					append_row_count_);
		}
		else
		{
			if (castRequired) 
			{
				std::vector<T> castVec(append_row_count_);

				CastTo<T>(columnData.data_, 
					castVec, 
					columnData.cast_from_, 
					append_row_count_, 
					offset);

				bldr->AppendValues(castVec.data(), 
					append_row_count_,
					columnData.null_values_);

			}
			else
				bldr->AppendValues(((T*)columnData.data_) + offset,
					append_row_count_, 
					columnData.null_values_);
		}
	}
}

template<typename castType> void ParquetContext::CastTo(const void const * data, 
	std::vector<castType>& buildVec,
	const std::string& castFrom, 
	const int& size, 
	const int& offset)
{
	if (castFrom == typeid(int8_t).name()) 
	{
		std::copy((int8_t*)data + offset, 
			(int8_t*)data + offset + size, 
			buildVec.data());
	}
	else if (castFrom == typeid(uint8_t).name()) 
	{
		std::copy((uint8_t*)data + offset, 
			(uint8_t*)data + offset + size, 
			buildVec.data());
	}
	else if (castFrom == typeid(int16_t).name()) 
	{
		std::copy((int16_t*)data + offset, 
			(int16_t*)data + offset + size, 
			buildVec.data());
	}
	else if (castFrom == typeid(uint16_t).name()) 
	{
		std::copy((uint16_t*)data + offset, 
			(uint16_t*)data + offset + size, 
			buildVec.data());
	}
	else if (castFrom == typeid(int32_t).name()) 
	{
		std::copy((int32_t*)data + offset, 
			(int32_t*)data + offset + size, 
			buildVec.data());
	}
	else if (castFrom == typeid(uint32_t).name()) 
	{
		std::copy((uint32_t*)data + offset, 
			(uint32_t*)data + offset + size, 
			buildVec.data());
	}
	else if (castFrom == typeid(int64_t).name()) 
	{
		std::copy((int64_t*)data + offset, 
			(int64_t*)data + offset + size, 
			buildVec.data());
	}
	else if (castFrom == typeid(uint64_t).name()) 
	{
		std::copy((uint64_t*)data + offset, 
			(uint64_t*)data + offset + size, 
			buildVec.data());
	}
}

template<typename NativeType> 
bool ParquetContext::SetMemoryLocation(std::vector<NativeType>& data, 
	const std::string& fieldName, 
	uint8_t* boolField)
{

	for (std::map<std::string, ColumnData>::iterator 
		it = column_data_map_.begin(); 
		it != column_data_map_.end(); 
		++it) {

		if (it->first == fieldName) 
		{
			NativeType a;

			// If it is a list and boolField is defined
			// make sure to reset boolField to null since
			// null lists aren't available
			if (it->second.is_list_)
			{
				if (boolField != nullptr)
				{
#ifdef DEBUG
#if DEBUG > 1
					printf("Warning!!!!!!!!!!!!  Null fields for lists"
						"are currently unavailable: %s\n",
						fieldName.c_str());
#endif
#endif
					boolField = nullptr;
				}

				if (data.size() % it->second.list_size_ != 0)
				{
					printf("Error!!!!!!!!!!!!  list size specified (%d)"
						" is not a multiple of total data length (%d) "
						"for column: %s\n",
						it->second.list_size_,
						data.size(),
						fieldName);
					parquet_stop_ = true;
					return false;
				}
				else
				{
					it->second.SetColumnData(data.data(),
						fieldName,
						typeid(NativeType).name(),
						boolField, data.size());
				}
				
			}

			// Check if ptr is already set
			if (it->second.pointer_set_)
			{
#ifdef DEBUG
#if DEBUG > 1
				printf("Warning!!!!!!!!!!!!  in SetMemoryLocation, ptr"
						"is already set for: %s\n", 
					fieldName.c_str());
#endif
#endif
			}

			// If the casting is required			
			else if (typeid(NativeType).name() != it->second.type_ID_) 
			{
				// Check if other types are being written
				// to a string (NativeType being cast to second.typeID)
				if (it->second.type_ID_ == typeid(std::string).name() && 
					typeid(std::string).name() != typeid(NativeType).name()) 
				{
#ifdef DEBUG
#if DEBUG > 1
					printf("Warning!!!!!!!!!!!!  can't cast from other data"
							"type to string for: %s\n", 
						fieldName.c_str());
#endif
#endif
				}
				// Cast to larger datatype check
				else if (it->second.byte_size_ < sizeof(a)) 
				{
#ifdef DEBUG
#if DEBUG > 1
					printf("Warning!!!!!!!!!!!!  Intended datatype to be cast"
							"is smaller than the given datatype for: %s\n", 
						fieldName.c_str());
#endif
#endif
				}// Check if floating point casting is happening
				else if (it->second.type_ID_ == typeid(float).name() ||
					it->second.type_ID_ == typeid(double).name() || 
					typeid(NativeType).name() == typeid(float).name() || 
					typeid(NativeType).name() == typeid(double).name()) 
				{
#ifdef DEBUG
#if DEBUG > 1
					printf("Warning!!!!!!!!!!!!  Can't cast floating"
							"point data types: %s, \n", 
						fieldName.c_str());
#endif
#endif
				}
				// Equal size datatypes check
				else if (it->second.byte_size_ == sizeof(a)) 
				{
					it->second.SetColumnData(data.data(), 
						fieldName, 
						typeid(NativeType).name(),
						boolField,
						data.size());
#ifdef DEBUG
#if DEBUG > 1
					printf("Warning!!!!!!!!!!!!  Intended datatype to "
							"be cast is equal to the casting type for: %s\n", 
						fieldName.c_str());
#endif
#endif
				}
				else {
					it->second.SetColumnData(data.data(), 
						fieldName, 
						typeid(NativeType).name(), 
						boolField, 
						data.size());
#ifdef DEBUG
#if DEBUG > 1
					printf("Cast from %s planned for: %s, \n", 
						typeid(NativeType).name(), 
						fieldName.c_str());
#endif
#endif
				}
			}
			// data types are the same and no casting required
			else
			{
				it->second.SetColumnData(data.data(), 
					fieldName, 
					"", 
					boolField,
					data.size());
			}

			return true;
		}
	}
	printf("ERROR!!! -> Field name doesn't exist: %s\n", 
		fieldName.c_str());

	parquet_stop_ = true;
	return false;
}

template<typename NativeType>
bool ParquetContext::SetMemoryLocation(std::vector<std::string>& data,
	const std::string& fieldName,
	uint8_t* boolField)
{
	for (std::map<std::string, ColumnData>::iterator
		it = column_data_map_.begin();
		it != column_data_map_.end();
		++it)
	{
		if (it->first == fieldName)
		{
			if (typeid(std::string).name() != it->second.type_ID_)
			{
#ifdef DEBUG
#if DEBUG > 1
				printf("Warning!!!!!!!!!!!!  in SetMemoryLocation, "
					"can't cast from string to other types: %s\n",
					fieldName);
#endif
#endif
			}
			else if (it->second.pointer_set_)
			{
#ifdef DEBUG
#if DEBUG > 1
				printf("Warning!!!!!!!!!!!!  in SetMemoryLocation, "
					"ptr is already set for: %s\n",
					fieldName);
#endif
#endif
			}
			else if (it->second.is_list_)
			{
				if (boolField != nullptr)
				{
#ifdef DEBUG
#if DEBUG > 1
					printf("Warning!!!!!!!!!!!!  Null fields for lists"
						"are currently unavailable: %s\n",
						fieldName);
#endif
#endif
				}
				if (data.size() % it->second.list_size_ != 0)
				{
					printf("Error!!!!!!!!!!!!  list size specified (%d)"
							" is not a multiple of total data length (%d) "
							"for column: %s\n", 
						it->second.list_size_,
						data.size(),
						fieldName);
					parquet_stop_ = true;
					return false;
				}
				else
				{
					it->second.SetColumnData(data, fieldName, nullptr, data.size());
				}
			}
			else
			{
#ifdef DEBUG
#if DEBUG > 1
				printf("setting field info for %s\n",
					fieldName.c_str());
#endif
#endif
				it->second.SetColumnData(data, fieldName, boolField, data.size());
			}
			return true;
		}
	}
	printf("ERROR!!! -> Field name doesn't exist:%s\n",
		fieldName.c_str());
	parquet_stop_ = true;
	return false;
}


#endif