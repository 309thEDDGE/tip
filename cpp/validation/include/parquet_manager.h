#ifndef PARQUET_MANAGER
#define PARQUET_MANAGER

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/schema.h>
#include <parquet/arrow/writer.h>
#include <string>
#include <vector>
#include <filesystem>

class ParquetManager {
	
private:
	std::vector<std::string> input_parquet_paths_;

	// Arrow variables.
	arrow::Status st_;
	arrow::MemoryPool* pool_ = arrow::default_memory_pool();
	std::shared_ptr<arrow::io::ReadableFile> arrow_file_;
	std::unique_ptr<parquet::arrow::FileReader> arrow_reader_;
	

	int row_group_count_;
	int current_row_group_;
	int current_file_;

	bool OpenNextParquetFile();
	

public:
	ParquetManager() 
	{
		row_group_count_ = 0;
		current_row_group_ = 0;
		current_file_ = 0;
	};
	~ParquetManager() 
	{
		if (arrow_file_ != nullptr)
		{
			if (!arrow_file_->closed())
				arrow_file_->Close();
		}
	};

	std::shared_ptr<arrow::Schema> schema_;

	/*
		Initializes the parquet folder

		Returns: False -> If invalid parquet folder
				 True  -> If valid parquet folder
	*/
	bool SetPQPath(std::string base_path);

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
};

template<typename T, typename A>
bool ParquetManager::GetNextRG(int col, 
	std::vector<T>& data, 
	int& size, 
	bool list)
{
	size = 0;

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

	if(list)
	{
		arrow::ListArray data_list_arr =
			arrow::ListArray(arrow_table->column(0)->data()->chunk(0)->data());


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
		A data_array =
			A(arrow_table->column(0)->data()->chunk(0)->data());

		size = data_array.length();

		if (data.size() < size)
			data.resize(size);

		std::copy(data_array.raw_values(), 
			data_array.raw_values() + data_array.length(), 
			data.data());
	}
	

	current_row_group_++;
	return true;

}
#endif