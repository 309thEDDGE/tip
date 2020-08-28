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
	int append_row_count_;
	bool have_created_table_;
	uint8_t ret_;
	std::map<std::string, ColumnData> column_data_map_;
	std::shared_ptr<arrow::Schema> schema_;
	std::shared_ptr<arrow::io::FileOutputStream> ostream_;
	std::shared_ptr<parquet::WriterProperties> props_;
	std::shared_ptr<arrow::io::ReadableFile> input_file_;
	std::vector<std::shared_ptr<arrow::Field>> fields_;
	std::vector<uint8_t> cast_vec_;

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

	template<typename castToType> 
	void CastTo(const void const* data,
		const CastFromType castFrom,
		const int& size,
		const int& offset);
	

public:
	/*
		Initializes parquet context with a default row
		group size of 10000
	*/
	ParquetContext();

	/*
		Initializes parquet context with a row group size
		specified by rgSize

		Note: row group sizes are either set by the default
		row group size when calling WriteColumns() or by
		rows when calling 
		WriteColumns(const int& rows, const int offset=0)
	*/
	ParquetContext(int rgSize);
	~ParquetContext();

	/*
		Closes the parquet file
		Automatically called by the destructor
		This function is public for testing purposes
	*/
	void Close();

	/*
		Adds a column to a parquet file. Used in conjunction with 
		SetMemoryLocation. Also includes automatic casting.
		Must be called before the correlated call to 
		SetMemoryLocation.

		Inputs:	type		-> desired datatype for the column.
								If casting is done, this is the
								data type that the data source 
								will be cast TO. The cast FROM 
								data type is specified by the 
								data type passed to SetMemoryLocation.
								Casting is generally used to cast
								from unsigned data types to signed.
								Casting is done automatically and is
								only available under the following conditions
								1. only between integers
								2. only if the data type being cast TO
								is of greater size or equal to the size
								of the data type being cast from (ie. cannot
								cast from int32 to int16).

								type options include:
								arrow::int64()
								arrow::int32()
								arrow::int16()
								arrow::int8()
								arrow::utf8() -> strings
								arrow::boolean()
								arrow::float32() -> float
								arrow::float64() -> double
				fieldName	-> name of the column
								Must be the same name as fieldName when
								calling the associated SetMemoryLocation
								function call
				listSize	-> if the column is to be a list (meaning
								each row of that column will be a list
								of values) specify the size of the list
								Example: if there are to be 32 integers
								per row listSize=32. If the column is
								not a list, leave as NULL.

	*/
	void AddField(const std::shared_ptr<arrow::DataType> type, 
		const std::string& fieldName, 
		const int listSize=NULL);

	/*
		Sets a pointer to the memory location of vectors
		where data will be writen from. These vectors
		are to be initialized and managed outsize of parquet
		context. Parquet context only stores a pointer to 
		each vector for each column. SetMemoryLocation 
		should be called once for each column along side
		AddField. It should be called after AddField. 
		The vectors should be at least as large as the maximum 
		intended row group size. They should also exceed the 
		amount of rows needed for 
		WriteColumns(const int& rows, const int offset = 0).
		Automatic casting is done when the Nativetype is different
		from the arrow type specified in AddField. Nativetype
		is the type casted from and the arrow type is the
		type casted to. See AddField comments above for details
		on casting restrictions.

		Inputs:	 data		-> vectors to be writen to the parquet
								file. Memory is managed outside 
								parquet context. Only a pointer is
								stored in parquet context. Vectors 
								for lists should be of size (max 
								intented row group size * list size). 
								Vectors for normal columns should be 
								of size (max intended row group size).

				fieldName	-> Name of the column. Must match the 
								fieldName for the associated AddField
								call.

				boolField	-> boolField is a vector of uint8_t that
								will specify to arrow which rows should
								be null for the given column. This specification
								is not currently supported for list columns.
								Example: data			= {1,2,3,4}
										 boolField		= {0,1,1,0}
										 parquetColumn	= {NULL,2,3,NULL}
	*/
	template<typename NativeType> 
	bool SetMemoryLocation(std::vector<NativeType>& data, 
		const std::string& fieldName, 
		std::vector<uint8_t>* boolField=nullptr);

	// Overloaded function for strings. 
	template<typename NativeType>
	bool SetMemoryLocation(std::vector<std::string>& strVec, 
		const std::string& fieldName, 
		std::vector<uint8_t>* boolField=nullptr);

	/*
		Sets the parquet file output path. Note that OpenForWrite
		should be called after all SetMemoryLocation and
		AddField function calls.

		Inputs:	path	 -> full path of paruqet file
								"<path>/file.parquet"
				truncate -> Currently not working in Apache Arrow.
							Since arrow implementation for truncation
							is not currently working, trunucate is
							hardcoded to true no matter what is passed
							for consistent implementation.
	*/
	uint8_t OpenForWrite(const std::string path,
		const bool truncate = true);

	/*
		Writes all data vectors to all columns in the parquet file. 
		Can be called multiple times to append to the existing parquet file.

		Inputs:	rows	-> the number of rows to be written 
							from the data vectors provided to 
							parquet context from SetMemoryLocation.
							Rows will specify the row group size for
							each write.

				offset	-> An offset can be provided to specify
							the starting location of the SetMemoryLocation
							vectors	provided to start writing from. 
							If the column is a list, the actual starting
							location in the vector will be (offset * listSize).

		Returns:		   True if all columns were written successfully, 
							False otherwise
						   Note: If rows + offset exceeds the amount of 
						   data in the original vectors passed to 
						   SetMemoryLocation, False will be returned.
	*/
	bool WriteColumns(const int& rows, const int offset = 0);

	/*
		Writes all data vectors to all columns in the parquet file. 
		Can be called multiple times to append to the existing parquet file.
		Writes the number of rows specified by rgSize passed to the 
		constructor. 10,000 rows are written if the default constructor was used.

		Returns:		   True if all columns were written successfully,
							False otherwise
						   Note: If the default row group size
						   exceeds the amount of data in the original 
						   vectors passed to SetMemoryLocation,
						   False will be returned.
	*/
	bool WriteColumns();
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

		// Resize the global cast vector to the minimum size needed
		// in bytes
		if (cast_vec_.size() < (append_row_count_ * listCount * sizeof(T)))
			cast_vec_.resize(append_row_count_ * listCount * sizeof(T));

		if (castRequired)
		{
			std::vector<int32_t> offsets_vec = 
				GetOffsetsVector(append_row_count_, listCount, 0);

			bldr->AppendValues(offsets_vec.data(), append_row_count_);

			A* sub_bldr =
				static_cast<A*>(bldr->value_builder());			

			CastTo<T>(columnData.data_,
				columnData.cast_from_,
				append_row_count_ * listCount,
				offset * listCount);

			sub_bldr->AppendValues((T*)cast_vec_.data(), 
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

		// Resize the global cast vector to the minimum size needed
		// in bytes
		if (cast_vec_.size() < (append_row_count_ * sizeof(T)))
			cast_vec_.resize(append_row_count_ * sizeof(T));

		if (columnData.null_values_ == nullptr) 
		{
			if (castRequired) 
			{
				CastTo<T>(columnData.data_,
					columnData.cast_from_,
					append_row_count_,
					offset);

				bldr->AppendValues((T*)cast_vec_.data(),
					append_row_count_);
			}
			else
				bldr->AppendValues(((T*)columnData.data_) + offset, 
					append_row_count_);
		}
		else
		{
			if (castRequired) 
			{
				CastTo<T>(columnData.data_,
					columnData.cast_from_,
					append_row_count_,
					offset);

				bldr->AppendValues((T*)cast_vec_.data(),
					append_row_count_,
					columnData.null_values_->data() + offset);

			}
			else
				bldr->AppendValues(((T*)columnData.data_) + offset,
					append_row_count_, 
					columnData.null_values_->data() + offset);
		}
	}
}

template<typename castToType>
void ParquetContext::CastTo(const void const* data,
	const CastFromType castFrom,
	const int& size,
	const int& offset)
{
	switch (castFrom)
	{
	case INT8:
		std::copy((int8_t*)data + offset,
			(int8_t*)data + offset + size,
			(castToType*)cast_vec_.data());
		break;

	case UINT8:
		std::copy((uint8_t*)data + offset,
			(uint8_t*)data + offset + size,
			(castToType*)cast_vec_.data());
		break;

	case INT16:
		std::copy((int16_t*)data + offset,
			(int16_t*)data + offset + size,
			(castToType*)cast_vec_.data());
		break;

	case UINT16:
		std::copy((uint16_t*)data + offset,
			(uint16_t*)data + offset + size,
			(castToType*)cast_vec_.data());
		break;

	case INT32:
		std::copy((int32_t*)data + offset,
			(int32_t*)data + offset + size,
			(castToType*)cast_vec_.data());
		break;
	
	case UINT32:
		std::copy((uint32_t*)data + offset,
			(uint32_t*)data + offset + size,
			(castToType*)cast_vec_.data());
		break;

	case INT64:
		std::copy((int64_t*)data + offset,
			(int64_t*)data + offset + size,
			(castToType*)cast_vec_.data());
		break;

	case UINT64:
		std::copy((uint64_t*)data + offset,
			(uint64_t*)data + offset + size,
			(castToType*)cast_vec_.data());
		break;
	}
}

template<typename NativeType> 
bool ParquetContext::SetMemoryLocation(std::vector<NativeType>& data, 
	const std::string& fieldName, 
	std::vector<uint8_t>* boolField)
{

	for (std::map<std::string, ColumnData>::iterator 
		it = column_data_map_.begin(); 
		it != column_data_map_.end(); 
		++it) 
	{
		if (it->first == fieldName) 
		{
			NativeType a;
			// If it is a list and boolField is defined
			// make sure to reset boolField to null since
			// null lists aren't available
			if (it->second.is_list_)
			{
				// Check that the list size is a multiple of the 
				// vector size provided
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
			}

			// The null field vector must be the same size as the 
			// data vector
			if (boolField != nullptr)
			{
				if (boolField->size() != data.size())
				{
					printf("Error!!!!!!!!!!!!  null field vector must be the "
						"same size as data vector: %s\n",
						fieldName);
					parquet_stop_ = true;
					return false;
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

			// If casting is required			
			if (typeid(NativeType).name() != it->second.type_ID_) 
			{
				// Check if other types are being written
				// to or from a string or to boolean from 
				// anything but uint8_t 
				// NativeType is they type being cast FROM and second.type_->id() 
				// is the type being cast TO which is the original arrow type passed 
				// to AddField 
				// Note: It is impossible to stop boolean from being cast
				// up to a larger type, since NativeType for boolean is uint8_t.
				// The only way to stop boolean from being cast
				// up would be to stop all uint8_t from being cast up.
				// Also note that it->second.type_ID_ is originally retrieved from
				// ParquetContext::GetTypeIDFromArrowType and every type is as 
				// expected except boolean. Boolean arrow types will result in 
				// uint8_t being assigned to it->second.type_ID_
				if (it->second.type_->id() == arrow::StringType::type_id ||
					typeid(std::string).name() == typeid(NativeType).name() ||
					it->second.type_->id() == arrow::BooleanType::type_id)
				{
					printf("Error!!!!!!!!!!!!  can't cast from other data"
							"type to string or bool for: %s\n", 
						fieldName.c_str());
					parquet_stop_ = true;
					return false;
				}

				// Cast to larger datatype check
				if (it->second.byte_size_ < sizeof(a)) 
				{
					printf("Error!!!!!!!!!!!!  Intended datatype to be cast"
							"is smaller than the given datatype for: %s\n", 
						fieldName.c_str());
					parquet_stop_ = true;
					return false;
				}

				// Check if floating point casting is happening
				if (it->second.type_ID_ == typeid(float).name() ||
					it->second.type_ID_ == typeid(double).name() || 
					typeid(NativeType).name() == typeid(float).name() || 
					typeid(NativeType).name() == typeid(double).name()) 
				{
					printf("Error!!!!!!!!!!!!  Can't cast floating"
							"point data types: %s, \n", 
						fieldName.c_str());
					parquet_stop_ = true;
					return false;
				}

				// Equal size datatypes check
				if (it->second.byte_size_ == sizeof(a)) 
				{
#ifdef DEBUG
#if DEBUG > 1
					printf("Warning!!!!!!!!!!!!  Intended datatype to "
							"be cast is equal to the casting type for: %s\n", 
						fieldName.c_str());
#endif
#endif
				}
				
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
			// Data types are the same and no casting required
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
	std::vector<uint8_t>* boolField)
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
				printf("Error!!!!!!!!!!!!  in SetMemoryLocation, "
					"can't cast from string to other types: %s\n",
					fieldName);
				parquet_stop_ = true;
				return false;
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
			if (it->second.is_list_)
			{

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

				if (boolField != nullptr)
				{
#ifdef DEBUG
#if DEBUG > 1
					printf("Warning!!!!!!!!!!!!  Null fields for lists"
						"are currently unavailable: %s\n",
						fieldName);
#endif
#endif
					boolField = nullptr;
				}
				
			}

			// The null field vector must be the same size as the 
			// data vector
			if (boolField != nullptr)
			{
				if (boolField->size() != data.size())
				{
					printf("Error!!!!!!!!!!!!  null field vector must be the "
						"same size as data vector: %s\n",
						fieldName);
					parquet_stop_ = true;
					return false;
				}
			}
#ifdef DEBUG
#if DEBUG > 1
			printf("setting field info for %s\n",
				fieldName.c_str());
#endif
#endif
			it->second.SetColumnData(data, fieldName, boolField, data.size());
			
			return true;
		}
	}
	printf("ERROR!!! -> Field name doesn't exist:%s\n",
		fieldName.c_str());
	parquet_stop_ = true;
	return false;
}


#endif