#ifndef PARQUET_CONTEXT_H
#define PARQUET_CONTEXT_H

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/writer.h>
#include <parquet/arrow/schema.h>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <cmath>
#include <map>
#include <memory>
#include <algorithm>
#include <string>
#include <typeinfo>
#include <filesystem>
#include "column_data.h"
#include "spdlog/spdlog.h"

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
    size_t ROW_GROUP_COUNT_;
    int append_row_count_;
    bool have_created_table_;
    std::map<std::string, ColumnData> column_data_map_;
    std::shared_ptr<arrow::Schema> schema_;
    std::shared_ptr<arrow::io::FileOutputStream> ostream_;
    std::shared_ptr<parquet::WriterProperties> props_;
    std::shared_ptr<arrow::io::ReadableFile> input_file_;
    std::vector<std::shared_ptr<arrow::Field>> fields_;
    std::vector<uint8_t> cast_vec_;

    // Initialize as false and set true when rows are written
    // to the file. Used to determine if the parquet file is empty
    // and should be deleted.
    bool did_write_columns_;

    // Set to true if empty file deletion is enabled.
    bool empty_file_deletion_enabled_;

    std::unique_ptr<arrow::ArrayBuilder>
    GetBuilderFromDataType(
        const std::shared_ptr<arrow::DataType> dtype,
        const bool& is_list_builder);

    bool AppendColumn(ColumnData& dataInfo,
                      const int& rows,
                      const int offset = 0);

    template <typename T, typename A>
    void Append(const bool& isList,
                const bool& castRequired,
                const int& listCount,
                const int& offset,
                const ColumnData& columnData);

    void CreateBuilders();

    std::vector<int32_t> GetOffsetsVector(const int& n_rows,
                                          const int& elements_per_row,
                                          const int offset = 0);

    bool WriteColsIfReady();

    void FillStringVec(std::vector<std::string>* str_data_vec_ptr,
                       const int& count,
                       const int offset = 0);

    bool IsUnsigned(const std::shared_ptr<arrow::DataType> type);

    std::string GetTypeIDFromArrowType(const std::shared_ptr<arrow::DataType> type,
                                       int& byteSize);

    template <typename castToType>
    void CastTo(const void* const data,
                const CastFromType castFrom,
                const int& size,
                const int& offset);

    // Track the count of appended rows.
    size_t appended_row_count_;

    // Determine if row groups are ready to be written
    // by comparing appended row count to buffer size.
    size_t max_temp_element_count_;
    size_t row_group_count_multiplier_;
    bool ready_for_automatic_tracking_;
    bool print_activity_;
    std::string print_msg_;

   public:
    // User-available variable to access the current
    // count of rows appended to buffers.
    const size_t& append_count_ = appended_row_count_;
    const size_t& row_group_count;

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
    virtual ~ParquetContext();

    /*
		Closes the parquet file
		Automatically called by the destructor
		This function is public for testing purposes

		Inputs: thread_id		--> Optional index of current thread
	*/
    void Close(const uint16_t& thread_id = 0);

    /*
    Get the count of columns by returning the count of the
    fields in the schema. 

    Return:
        Count of columns/fields in the schema. 
    */
    size_t GetColumnCount()
    {
        return schema_->num_fields();
    }

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
								data type used in SetMemoryLocation.
								Casting is generally used to cast
								from unsigned data types to signed.
								Casting is done automatically and is
								only available under the following conditions
								1. only between integers
								2. only if the data type being cast TO
								   is of greater size or equal to the size
								   of the data type being cast from (ie. cannot
								   cast from int32 to int16).
								3. casting TO unsigned integers is not
								   supported

								arrow column type options supported:
								arrow::int64()
								arrow::int32()
								arrow::int16()
								arrow::int8()
								arrow::utf8() -> strings
								arrow::boolean()
								arrow::float32() -> float
								arrow::float64() -> double

				fieldName	-> name of the column
								Must be consistent with fieldName when
								calling the associated SetMemoryLocation
								function call

				listSize	-> if the column is to be a list (meaning
								each row of that column will be a list
								of values) specify the size of the list
								Example: if there are to be 32 integers
								per row listSize=32. If the column is
								not a list, leave as NULL.

		Returns:				True if field created successfully
								False if output type is unsigned
								(unsigned output currently not supported)

	*/
    bool AddField(const std::shared_ptr<arrow::DataType> type,
                  const std::string& fieldName,
                  const int listSize = NULL);

    /*
		Sets a pointer to the memory location of vectors
		where data will be written from. These vectors
		are to be initialized and managed outsize of parquet
		context. Parquet context only stores a pointer to 
		each vector for each column. SetMemoryLocation 
		should be called once for each column added with
		AddField. It should be called after AddField. 
		The vectors should be at least as large as the maximum 
		intended row group size. They should also exceed the 
		amount of rows needed for the maximum intented row
		size called when using
		WriteColumns(const int& rows, const int offset = 0).
		Automatic casting is done when the Nativetype is different
		from the arrow type specified in AddField. Nativetype
		is the type casted FROM and the arrow type is the
		type casted TO. See AddField comments above for details
		on casting restrictions.

		Inputs:	 data		-> vectors to be written to the parquet
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
								be null for a given column. This specification
								is not currently supported for list columns.

								Example: data			= {1,2,3,4}
										 boolField		= {0,1,1,0}
										 parquetColumn	= {NULL,2,3,NULL}

		Returns:				True if successful
								False if fieldName was not added previously with 
								AddField or casting isn't possible
	*/
    template <typename NativeType>
    bool SetMemoryLocation(std::vector<NativeType>& data,
                           const std::string& fieldName,
                           std::vector<uint8_t>* boolField = nullptr);

    /*
		Creates the parquet file with an initial schema
		specified by AddField calls. 
		Note!!!!! OpenForWrite should be called after 
		all SetMemoryLocation and AddField function calls.

		Inputs:	path	 -> full path of paruqet file
								"<path>/file.parquet"
				truncate -> Currently not working in Apache Arrow.
							Since arrow implementation for truncation
							is not currently working, trunucate is
							hardcoded to true no matter what is passed
							for consistent implementation.

		Returns:			True if the parquet file was created successfully
							False if any AddField calls aren't initialized with a 
							subsequent SetMemoryLocation call, or if no AddField
							calls are made.
	*/
    bool OpenForWrite(const std::string path,
                      const bool truncate = true);

    /*
		Writes all columns using vectors passed to SetMemoryLocation. 
		Can be called multiple times to append to the existing parquet file.

		Inputs:	rows	-> the number of rows to be written 
							from the data vectors provided to 
							parquet context from SetMemoryLocation.
							Rows will specify the row group size for
							each write.

				offset	-> An offset can be provided to specify
							the starting location from which to begin
							writing from the SetMemoryLocation vectors.
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

    /*
		Set the parameters used for automatic accounting of rows and 
		writing row groups when necessary. Also handles the 
		writing of remaining rows if the appended row count has not reached
		the critical count required to trigger a write.

		Used in conjunction with IncrementAndWrite and Finalize. The workflow is
		1) setup the vectors/buffers for the parquet table
		2) allocate the necessary memory in each
		3) call AddField for each buffer/column
		4) call SetMemoryLocation for each buffer/column
		5) call this function SetupRowCountTracking, if valid (= true) then
		6) call OpenForWrite() to open the output file
		7) Use the append_count_ index to fill each row
		8) call IncrementAndWrite after each row is filled at index append_count_
		9) continue until the table is complete
		10) call Finalize before exiting

		Inputs: row_group_count            -> The count of rows in a Parquet row group.
											  If the row count has been set via the constructor,
											  this input will override that value. 

				row_group_count_multiplier -> The count of row groups buffered prior to writing.
											  If the row_group_count is 100 and this value is 10
											  then 1000 elements shall have been allocated in the 
											  buffer (a std::vector). Use value 1 if a row group
											  shall be written each time it's buffer if filled.

				print_activity             -> Boolean to turn on (true) and off (false) print statements
											  when writing to parquet file is carried out. 

				print_msg                  -> Message to included in the print statement when data
											  are written to Parquet file. Default is an empty string.

		Returns:			True if row_group_count and row_group_count_multiplier are valid and
							false otherwise.
	
	*/
    bool SetupRowCountTracking(size_t row_group_count,
                               size_t row_group_count_multiplier, bool print_activity,
                               std::string print_msg = "");

    /*
        Enable automatic deletion of an output parquet file if the file has been built
        using SetupRowCountTracking, IncrementAndWrite, and Finalize if zero row groups
        were added by the time Finalize is called. If this
        function is not called sometime after SetupRowCountTracking during file 
        setup the default behavior is to allow a zero-row-group file to remain.

        Inputs: output_path     -> std::string output file path. Same as input 
                                   to OpenForWrite
    */
    void EnableEmptyFileDeletion(const std::string& path);

    /*
	
		Function to be called after the buffers are filled for the current row.
		Use the protected member variable append_count_ as the index in the buffer. It is
		incremented by this function and data are recorded if the parameters configured 
		via SetupRowCountTracking are met. 

		Ex: Simple Parquet file with time and data columns, with data buffered in time
		and data vectors. After the vector size is allocated and AddField and SetMemoryLocation
		are called for each, rows are appended via the following algorithm:
		
		AddData()
		{
			time[append_count_] = new time data
			data[append_count_] = new data value
			IncrementAndWrite();
		}

		If one of the columns is a list and you wish to zero all values, do
			if(IncrementAndWrite())
			{
				fill(list_vec.begin(), list_vec.end(), 0);
			}

		Inputs: thread_id		--> Optional index of current thread

		Returns:
			
						True if the data row group(s) were written and false otherwise. Note
						that for the example of 100 count row groups and a multiplier of 10,
						this function will only return true every 1000th call.

	*/
    bool IncrementAndWrite(const uint16_t& thread_id = 0);

    /*
	
		Write the data remaining in the buffers to disk. Generally used prior to closing
		the file. This function is called automatically by the destructor if automatic 
		accounting of row group appending has been initiated via the SetupRowCountTracking
		function. Nevertheless, it is a best practice to call Finalize() intentionally prior
		to exiting the program. In that case, when it's called by the destructor, it will not
		perform any action because data have already been written the counter will have been
		zeroed.

		Inputs: thread_id		--> Optional index of current thread

	*/
    void Finalize(const uint16_t& thread_id = 0);
};

template <typename T, typename A>
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

            sub_bldr->AppendValues(reinterpret_cast<T*>(cast_vec_.data()),
                                   append_row_count_ * listCount);
        }
        else
        {
            std::vector<int32_t> offsets_vec =
                GetOffsetsVector(append_row_count_, listCount, 0);

            bldr->AppendValues(offsets_vec.data(), append_row_count_);
            A* sub_bldr =
                static_cast<A*>(bldr->value_builder());
            sub_bldr->AppendValues(reinterpret_cast<T*>(columnData.data_) + (offset * listCount),
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

                bldr->AppendValues(reinterpret_cast<T*>(cast_vec_.data()),
                                   append_row_count_);
            }
            else
            {
                bldr->AppendValues(reinterpret_cast<T*>(columnData.data_) + offset,
                                   append_row_count_);
            }
        }
        else
        {
            if (castRequired)
            {
                CastTo<T>(columnData.data_,
                          columnData.cast_from_,
                          append_row_count_,
                          offset);

                bldr->AppendValues(reinterpret_cast<T*>(cast_vec_.data()),
                                   append_row_count_,
                                   columnData.null_values_->data() + offset);
            }
            else
                bldr->AppendValues(reinterpret_cast<T*>(columnData.data_) + offset,
                                   append_row_count_,
                                   columnData.null_values_->data() + offset);
        }
    }
}

template <typename castToType>
void ParquetContext::CastTo(const void* const data,
                            const CastFromType castFrom,
                            const int& size,
                            const int& offset)
{
    switch (castFrom)
    {
        case CastFromType::TypeINT8:
            std::copy(reinterpret_cast<const int8_t* const>(data) + offset,
                      reinterpret_cast<const int8_t* const>(data) + offset + size,
                      reinterpret_cast<castToType*>(cast_vec_.data()));
            break;

        case CastFromType::TypeUINT8:
            std::copy(reinterpret_cast<const uint8_t* const>(data) + offset,
                      reinterpret_cast<const uint8_t* const>(data) + offset + size,
                      reinterpret_cast<castToType*>(cast_vec_.data()));
            break;

        case CastFromType::TypeINT16:
            std::copy(reinterpret_cast<const int16_t* const>(data) + offset,
                      reinterpret_cast<const int16_t* const>(data) + offset + size,
                      reinterpret_cast<castToType*>(cast_vec_.data()));
            break;

        case CastFromType::TypeUINT16:
            std::copy(reinterpret_cast<const uint16_t* const>(data) + offset,
                      reinterpret_cast<const uint16_t* const>(data) + offset + size,
                      reinterpret_cast<castToType*>(cast_vec_.data()));
            break;

        case CastFromType::TypeINT32:
            std::copy(reinterpret_cast<const int32_t* const>(data) + offset,
                      reinterpret_cast<const int32_t* const>(data) + offset + size,
                      reinterpret_cast<castToType*>(cast_vec_.data()));
            break;

        case CastFromType::TypeUINT32:
            std::copy(reinterpret_cast<const uint32_t* const>(data) + offset,
                      reinterpret_cast<const uint32_t* const>(data) + offset + size,
                      reinterpret_cast<castToType*>(cast_vec_.data()));
            break;

        case CastFromType::TypeINT64:
            std::copy(reinterpret_cast<const int64_t* const>(data) + offset,
                      reinterpret_cast<const int64_t* const>(data) + offset + size,
                      reinterpret_cast<castToType*>(cast_vec_.data()));
            break;

        case CastFromType::TypeUINT64:
            std::copy(reinterpret_cast<const uint64_t* const>(data) + offset,
                      reinterpret_cast<const uint64_t* const>(data) + offset + size,
                      reinterpret_cast<castToType*>(cast_vec_.data()));
            break;
    }
}

template <typename NativeType>
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
                    SPDLOG_CRITICAL(
                        "list size specified {:d}"
                        " is not a multiple of total data length {:d} "
                        "for column: {:s}",
                        it->second.list_size_,
                        data.size(),
                        fieldName);
                    parquet_stop_ = true;
                    return false;
                }

                if (boolField != nullptr)
                {
                    SPDLOG_WARN(
                        "Null fields for lists "
                        "are currently unavailable: {:s}",
                        fieldName);
                    boolField = nullptr;
                }
            }

            // The null field vector must be the same size as the
            // data vector
            if (boolField != nullptr)
            {
                if (boolField->size() != data.size())
                {
                    SPDLOG_CRITICAL(
                        "null field vector must be the "
                        "same size as data vector: {:s}",
                        fieldName);
                    parquet_stop_ = true;
                    return false;
                }
            }

            // Check if ptr is already set
            if (it->second.pointer_set_)
            {
                SPDLOG_WARN("ptr is already set for: {:s}", fieldName);
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
                    SPDLOG_CRITICAL(
                        "can't cast from other data type "
                        "to string or bool for: {:s}",
                        fieldName);
                    parquet_stop_ = true;
                    return false;
                }

                // Cast to larger datatype check
                if (it->second.byte_size_ < sizeof(a))
                {
                    SPDLOG_CRITICAL(
                        "Intended datatype to be cast "
                        "is smaller than the given datatype for: {:s}",
                        fieldName);
                    parquet_stop_ = true;
                    return false;
                }

                // Check if floating point casting is happening
                if (it->second.type_ID_ == typeid(float).name() ||
                    it->second.type_ID_ == typeid(double).name() ||
                    typeid(NativeType).name() == typeid(float).name() ||
                    typeid(NativeType).name() == typeid(double).name())
                {
                    SPDLOG_CRITICAL("Can't cast floating point data types: {:s}",
                                    fieldName);
                    parquet_stop_ = true;
                    return false;
                }

                // Equal size datatypes check
                if (it->second.byte_size_ == sizeof(a))
                {
                    SPDLOG_DEBUG(
                        "Intended datatype to "
                        "be cast is equal to the casting type for: {:s}",
                        fieldName);
                }

                it->second.SetColumnData(data.data(),
                                         fieldName,
                                         typeid(NativeType).name(),
                                         boolField,
                                         data.size());

                SPDLOG_DEBUG("Cast from {:s} planned for: {:s}",
                             typeid(NativeType).name(),
                             fieldName);
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

    SPDLOG_CRITICAL("Field name doesn't exist: {:s}", fieldName);
    parquet_stop_ = true;
    return false;
}

// Specialization for string data.
template <>
inline bool ParquetContext::SetMemoryLocation<std::string>(std::vector<std::string>& data,
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
                SPDLOG_CRITICAL("can't cast from string to other types: {:s}",
                                fieldName);
                parquet_stop_ = true;
                return false;
            }
            else if (it->second.pointer_set_)
            {
                SPDLOG_WARN("ptr is already set for: {:s}", fieldName);
            }
            if (it->second.is_list_)
            {
                if (data.size() % it->second.list_size_ != 0)
                {
                    SPDLOG_CRITICAL(
                        "list size specified ({:d})"
                        " is not a multiple of total data length ({:d}) "
                        "for column: {:s}",
                        it->second.list_size_,
                        data.size(),
                        fieldName);
                    parquet_stop_ = true;
                    return false;
                }

                if (boolField != nullptr)
                {
                    SPDLOG_WARN(
                        "Null fields for lists "
                        "are currently unavailable: {:s}",
                        fieldName);
                    boolField = nullptr;
                }
            }

            // The null field vector must be the same size as the
            // data vector
            if (boolField != nullptr)
            {
                if (boolField->size() != data.size())
                {
                    SPDLOG_CRITICAL(
                        "null field vector must be the "
                        "same size as data vector: {:s}",
                        fieldName);
                    parquet_stop_ = true;
                    return false;
                }
            }
            SPDLOG_DEBUG("setting field info for {:s}", fieldName);
            it->second.SetColumnData(data, fieldName, boolField);

            return true;
        }
    }
    SPDLOG_CRITICAL("Field name doesn't exist: {:s}", fieldName);
    parquet_stop_ = true;
    return false;
}

#endif
