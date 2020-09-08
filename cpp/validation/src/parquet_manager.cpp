#include "parquet_manager.h"

bool ParquetManager::OpenNextParquetFile()
{
	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	if (current_file_ >= input_parquet_paths_.size())
		return false;


	std::string file_path = input_parquet_paths_[current_file_];
	//printf("Reading parquet file: %s\n", file_path.c_str());
	current_row_group_ = 0;

	// Open file reader.
#ifdef NEWARROW
	try
	{
	PARQUET_ASSIGN_OR_THROW(arrow_file_, 
		arrow::io::ReadableFile::Open(file_path, pool_));     
	}
	catch(...)
	{
		printf("arrow::io::ReadableFile::Open error\n");
		return false;
	}
#else
	st_ = arrow::io::ReadableFile::Open(file_path, pool_, &arrow_file_);
	if (!st_.ok())
	{
		printf("arrow::io::ReadableFile::Open error (ID %s): %s\n",
			st_.CodeAsString().c_str(), st_.message().c_str());
		return false;
	}
#endif
	st_ = parquet::arrow::OpenFile(arrow_file_, pool_, &arrow_reader_);
	if (!st_.ok())
	{
		printf("parquet::arrow::OpenFile error (ID %s): %s\n",
			st_.CodeAsString().c_str(), st_.message().c_str());
		return false;
	}

	
	arrow_reader_->set_use_threads(true);
#ifndef NEWARROW
	arrow_reader_->set_num_threads(2);
#endif

	// Get schema1.
	st_ = arrow_reader_->GetSchema(&schema_);
	if (!st_.ok())
	{
		printf("GetSchema() error (ID %s): %s\n",
			st_.CodeAsString().c_str(), st_.message().c_str());
		return false;
	}

	// Total count of row groups.
	row_group_count_ = arrow_reader_->num_row_groups();
	//printf("%02d row groups\n", row_group_count_);

	current_file_++;
	return true;
}

bool ParquetManager::SetPQPath(std::string base_path)
{
	current_row_group_ = 0;
	current_file_ = 0;
	row_group_count_ = 0;
	input_parquet_paths_.clear();

	for (auto& p : std::filesystem::directory_iterator(base_path))
	{
		std::string temp_path = p.path().string();
		if (!p.is_directory())
		{
			if (p.path().extension().string() == ".parquet")
			{
				/*
				Test -- prepend boost::filesystem extended-length path prefix.
				Note: This seems to work with Apache Arrow 0.14.0 for reasons explained
				below. However, it will probably fail with Apache Arrow 0.15.0 and higher,
				or maybe an earlier version.

				Explanation: Prior to 0.15.0 Arrow relies on boost::filesystem to handle
				path manipulation and open files, etc. In JIRA issue ARROW-6613 ("[C++] Remove
				dependency on boost::filesystem") boost::filesystem was removed prior to
				the release of Arrow 0.15.0 which occurred on 20191005. See
				arrow.apache.org/release/0.15.0.html. When std::filesystem is used, paths are
				limited to 260 characters due to Windows MAX_PATH setting. Isaac verified this by
				running translate_1553_from_parquet.cpp on files which resulted in output paths
				below and above MAX_PATH. Output files with length less than MAX_PATH complete
				without issue and longer paths fail due to an Arrow IO problem in which filesystem
				says it can't find the path.

				Per boost.org/doc/libs/1_58_0/libs/filesystem/doc/reference.html#long-path-warning
				one way to get around the MAX_PATH limitation is to use the path prefix that is
				filled in temp_path below. Inclusion of this prefix allow Arrow to process long
				path lengths without issue.
				*/
				if (temp_path.size() > 259)
				{
					std::filesystem::path modified_path("\\\\?\\");
					modified_path += temp_path;
					input_parquet_paths_.push_back(modified_path.string());
				}
				else
				{
					input_parquet_paths_.push_back(temp_path);
				}
			}
				
		}
	}

	// sort the paths
	std::sort(input_parquet_paths_.begin(), input_parquet_paths_.end());

	return OpenNextParquetFile();
}


bool ParquetManager::GetNextRGBool(int col, std::vector<uint8_t>& data, 
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

	if (list)
	{
#ifdef NEWARROW
		arrow::ListArray data_list_arr =
			arrow::ListArray(arrow_table->column(0)->chunk(0)->data());
#else
		arrow::ListArray data_list_arr =
			arrow::ListArray(arrow_table->column(0)->data()->chunk(0)->data());
#endif

		arrow::BooleanArray data_array =
			arrow::BooleanArray(data_list_arr.values()->data());

		size = data_array.length();

		if (data.size() < size)
			data.resize(size);

		for (int i = 0; i < size; i++)
		{
			data[i] = data_array.Value(i);
		}
	}
	else
	{
#ifdef NEWARROW
		arrow::BooleanArray data_array =
			arrow::BooleanArray(arrow_table->column(0)->chunk(0)->data());
#else
		arrow::BooleanArray data_array =
			arrow::BooleanArray(arrow_table->column(0)->data()->chunk(0)->data());
#endif

		size = data_array.length();

		if (data.size() < size)
			data.resize(size);

		for (int i = 0; i < size; i++)
		{
			data[i] = data_array.Value(i);
		}
	}

	current_row_group_++;
	return true;
}

bool ParquetManager::GetNextRGString(int col, std::vector<std::string>& data, 
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

	if (list)
	{
#ifdef NEWARROW
		arrow::ListArray data_list_arr =
			arrow::ListArray(arrow_table->column(0)->chunk(0)->data());
#else
		arrow::ListArray data_list_arr =
			arrow::ListArray(arrow_table->column(0)->data()->chunk(0)->data());
#endif

		arrow::StringArray data_array =
			arrow::StringArray(data_list_arr.values()->data());

		size = data_array.length();

		if (data.size() < size)
			data.resize(size);

		for (int i = 0; i < size; i++)
		{
			data[i] = data_array.GetString(i);
		}
	}
	else
	{
#ifdef NEWARROW
		arrow::StringArray data_array =
			arrow::StringArray(arrow_table->column(0)->chunk(0)->data());
#else
		arrow::StringArray data_array =
			arrow::StringArray(arrow_table->column(0)->data()->chunk(0)->data());
#endif

		size = data_array.length();

		if (data.size() < size)
			data.resize(size);

		for (int i = 0; i < size; i++)
		{
			data[i] = data_array.GetString(i);
		}
	}

	current_row_group_++;
	return true;
}