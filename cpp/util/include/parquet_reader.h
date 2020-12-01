
#ifndef PARQUET_READER
#define PARQUET_READER

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/schema.h>
#include <parquet/arrow/writer.h>
#include <string>
#include <vector>
#include <filesystem>
#include "managed_path.h"

class ParquetReader {

private:
	//std::vector<std::string> input_parquet_paths_;
	std::vector<ManagedPath> input_parquet_paths_;

	// Arrow variables.
	arrow::Status st_;
	arrow::MemoryPool* pool_ = arrow::default_memory_pool();
	std::shared_ptr<arrow::io::ReadableFile> arrow_file_;
	std::unique_ptr<parquet::arrow::FileReader> arrow_reader_;
	bool manual_rowgroup_increment_mode_;

	std::vector<std::shared_ptr<arrow::Field>> initial_fields_ =
		std::vector<std::shared_ptr<arrow::Field>>();


	int row_group_count_;
	int current_row_group_;
	int current_file_;
	bool data_found_;
	std::shared_ptr<arrow::Schema> schema_;

	bool OpenNextParquetFile();

public:
	ParquetReader()
	{
		manual_rowgroup_increment_mode_ = false;
		row_group_count_ = 0;
		current_row_group_ = 0;
		current_file_ = 0;
		data_found_ = false;
		schema_ = arrow::schema(initial_fields_);
	};

	~ParquetReader()
	{
		if (arrow_file_ != nullptr)
		{
			if (!arrow_file_->closed())
				arrow_file_->Close();
		}
	};

	

	/*
		Initializes the parquet folder

		Returns: False -> If invalid parquet folder
							1. Non existent base_path
							2. Empty parquet folder
							3. Invalid parquet files
							4. Column count not consistent
							5. Schema types not consistent
							6. Column names not consistent
				 True  -> If valid parquet folder
	*/
	/*bool SetPQPath(std::string base_path);*/
	bool SetPQPath(ManagedPath base_path);

	/*
		Gets the next row group for a given column
		in the parquet folder and inserts it into
		the vector passed by reference.
		Assigns the row group row count to the size
		parameter passed by reference

		Returns: False -> If it is the end of file
				 True  -> If the row group was read successfully
	*/
	template<typename T, typename A>
	bool GetNextRG(int col, std::vector<T>& data,
		int& size,
		bool list = false);
	bool GetNextRGBool(int col, std::vector<uint8_t>& data,
		int& size,
		bool list = false);
	bool GetNextRGString(int col, std::vector<std::string>& data,
		int& size,
		bool list = false);

	/*
		GetColumnNumberFromName get the column number
		from a column name

		returns -> column number if found (0 for the first column) 
				   and -1 if the column name does not exist
	*/
	int GetColumnNumberFromName(std::string col_name);

	/*
		Resets parquet file reads to the first
		file in the parquet directory
	*/
	void Zeroize()
	{
		row_group_count_ = 0;
		current_row_group_ = 0;
		current_file_ = 0;
		OpenNextParquetFile();
	};

	/*
		If set, row groups will need to be incremented
		using IncrementRG(). If not set, row
		groups will be incremented automatically 
		every time GetNextRG() is called.
	*/
	void SetManualRowgroupIncrementMode()
	{
		manual_rowgroup_increment_mode_ = true;
	}

	/*
		Called to manually increment row groups.
		Only works when manual_rowgroup_incement_mode_ 
		is set to true (call SetManualRowgroupIncrementMode() to
		set to true)
	*/
	void IncrementRG()
	{
		if(manual_rowgroup_increment_mode_)
			++current_row_group_;
	};

	/*
		Return the current file row_group_count_
		by value.
	*/
	int GetRowGroupCount() { return row_group_count_; }

	/*
		Return the count of input_parquet_paths_
		by value. Cast to int since other counts,
		such as row_group_count_ is stored as int.
	*/
	int GetInputParquetPathsCount() { return int(input_parquet_paths_.size()); }

	/*
		Return the schema for the parquet file
	*/
	std::shared_ptr<arrow::Schema> GetSchema() { return schema_; }
	
};

template<typename T, typename A>
bool ParquetReader::GetNextRG(int col,
	std::vector<T>& data,
	int& size,
	bool list)
{
	size = 0;

	if (col >= schema_->num_fields())
		return false;

	if (current_row_group_ >= row_group_count_)
	{
		if (!OpenNextParquetFile())
			return false;
	}

	//printf("\rExtracting row group %03d\n", (current_row_group_ + 1));

	// Read row group from first file
	std::shared_ptr<arrow::Table> arrow_table;
	st_ = arrow_reader_->ReadRowGroup(current_row_group_,
		std::vector<int>({ col }),
		&arrow_table);

	if (!st_.ok())
	{
		printf("arrow::io::ReadableFile::ReadRowGroup error (ID %s): %s\n",
			st_.CodeAsString().c_str(), st_.message().c_str());
		return false;
	}

	if (list)
	{
#ifdef NEWARROW
		arrow::ListArray data_list_arr =
			arrow::ListArray(arrow_table->column(0)->chunk(0)->data());
#else
		arrow::ListArray data_list_arr =
			arrow::ListArray(arrow_table->column(0)->data()->chunk(0)->data());
#endif


		A data_array =
			A(data_list_arr.values()->data());

		size = data_array.length();


		if (data.size() < size)
			data.resize(size);

		std::copy(data_array.raw_values(),
			data_array.raw_values() + data_array.length(),
			data.data());
	}
	else
	{
#ifdef NEWARROW
		A data_array =
			A(arrow_table->column(0)->chunk(0)->data());
#else
		A data_array =
			A(arrow_table->column(0)->data()->chunk(0)->data());
#endif

		size = data_array.length();

		if (data.size() < size)
			data.resize(size);

		int null_count = data_array.null_count();

		if (null_count > 0)
		{
			for (int i = 0; i < size; i++)
			{
				data[i] = data_array.Value(i);
				// Arrow doesn't insert consistent
				// data when a row is NULL
				// The value in that row is overridden
				// for this reason to ensure comparisons match
				// later on down the road
				if (data_array.IsNull(i))
					data[i] = NULL;
			}
		}
		else
		{

			std::copy(data_array.raw_values(),
				data_array.raw_values() + data_array.length(),
				data.data());
		}
	}

	if(!manual_rowgroup_increment_mode_)
		current_row_group_++;

	return true;

}
#endif