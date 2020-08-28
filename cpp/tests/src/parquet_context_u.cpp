#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/schema.h>
#include <parquet/arrow/writer.h>
#include <vector>
#include <string>
#include <stdio.h>
#include <filesystem>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parquet_context.h"


class ParquetContextTest : public ::testing::Test
{
protected:
	std::string pq_file;
	ParquetContextTest() 
	{
		row_group_count_ = 0;
		current_row_group_ = 0;
		null_bitmap_ = nullptr;
	};

	// Arrow variables.
	arrow::Status st_;
	arrow::MemoryPool* pool_ = arrow::default_memory_pool();
	std::shared_ptr<arrow::io::ReadableFile> arrow_file_;
	std::unique_ptr<parquet::arrow::FileReader> arrow_reader_;
	const uint8_t* null_bitmap_;


	int row_group_count_;
	int current_row_group_;

	void SetUp() override
	{

	}
	~ParquetContextTest()
	{
		if (arrow_file_ != nullptr)
		{
			if (!arrow_file_->closed())
				arrow_file_->Close();
		}

		remove(pq_file.c_str());		
	}

	// Generate Parquet file with single value
	// columns
	template <typename T>
	bool CreateParquetFile(std::shared_ptr<arrow::DataType> type,
		std::string file_name,
		std::vector<std::vector<T>> output,
		int row_group_count, bool truncate = true,
		std::vector<uint8_t>* bool_fields = nullptr)
	{

		file_name = "./" + file_name;

		ParquetContext* pc = new ParquetContext(row_group_count);

		bool ret_value = true;

		// Add each vector as a column
		for (int i = 0; i < output.size(); i++)
		{
			pc->AddField(type, "data" + std::to_string(i));
			ret_value = pc->SetMemoryLocation<T>(output[i], "data" + std::to_string(i), bool_fields);
			if (!ret_value)
				return false;
		}

		// Assume each vector is of the same size
		int row_size = output[0].size();

		pc->OpenForWrite(file_name, truncate);
		for (int i = 0; i < row_size / row_group_count; i++)
		{
			ret_value = pc->WriteColumns(row_group_count, i * row_group_count);
			if (!ret_value)
				return false;
		}

		// The the remaider rows if row_group_count is 
		// not a multiple of the array size
		int remainder = row_size % row_group_count;
		if (remainder > 0)
		{
			ret_value = pc->WriteColumns(remainder,
				(row_size / row_group_count) * row_group_count);
			if (!ret_value)
				return false;
		}


		pc->Close();
		pq_file = file_name;
		delete pc;
		return true;
	}

	// Create parquet file with one list column
	template <typename T>
	bool CreateParquetFileList(std::shared_ptr<arrow::DataType> type, 
		std::string file_name, 
		std::vector<T> output,
		int row_group_count, 
		int list_size, 
		bool truncate = true, 
		std::vector<uint8_t>* bool_fields = nullptr)
	{
		ParquetContext* pc = new ParquetContext(row_group_count);

		file_name = "./" + file_name;

		bool ret_value = true;

		// Add the vector as a list column
		pc->AddField(type, "data", list_size);
		ret_value = pc->SetMemoryLocation<T>(output, "data", bool_fields);
		if (!ret_value)
			return false;


		// Assume each vector is of the same size
		int row_size = output.size() / list_size;
		

		pc->OpenForWrite(file_name, truncate);
		for (int i = 0; i < row_size / row_group_count; i++)
		{
			ret_value = pc->WriteColumns(row_group_count, i * row_group_count);
			if (!ret_value)
				return false;
		}

		// The the remaider rows if row_group_count is 
		// not a multiple of the array size
		
		int remainder = row_size % row_group_count;
		if (remainder > 0)
		{
			ret_value = pc->WriteColumns(remainder,
				(row_size / row_group_count) * row_group_count);
			if (!ret_value)
				return false;
		}

		pc->Close();
		pq_file = file_name;
		delete pc;
		return true;
	}

	template<typename T, typename A>
	bool GetNextRG(int col,
		std::vector<T>& data,		
		bool list = false,
		std::vector<size_t>* null_indicies = nullptr)
	{
		int size = 0;

		if (current_row_group_ >= row_group_count_)
		{
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

			if (null_indicies != nullptr)
			{
				null_indicies->clear();
				for (int i = 0; i < size; i++)
				{
					if (data_array.IsNull(i))
						null_indicies->push_back(i);
				}
			}

			if (data.size() < size)
				data.resize(size);

			std::copy(data_array.raw_values(),
				data_array.raw_values() + data_array.length(),
				data.data());
		}


		current_row_group_++;
		return true;

	}

	bool GetNextRGBool(int col, std::vector<bool>& data,
		bool list = false,
		std::vector<size_t>* null_indicies = nullptr)
	{
		int size = 0;

		if (current_row_group_ >= row_group_count_)
		{
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
			arrow::ListArray data_list_arr =
				arrow::ListArray(arrow_table->column(0)->data()->chunk(0)->data());

			arrow::BooleanArray data_array=
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
			arrow::BooleanArray data_array =
				arrow::BooleanArray(arrow_table->column(0)->data()->chunk(0)->data());

			size = data_array.length();

			if (null_indicies != nullptr)
			{
				null_indicies->clear();
				for (int i = 0; i < size; i++)
				{
					if (data_array.IsNull(i))
						null_indicies->push_back(i);
				}
			}

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

	bool GetNextRGString(int col, std::vector<std::string>& data,
		bool list = false,
		std::vector<size_t>* null_indicies = nullptr)
	{
		int size = 0;

		if (current_row_group_ >= row_group_count_)
		{
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
			arrow::ListArray data_list_arr =
				arrow::ListArray(arrow_table->column(0)->data()->chunk(0)->data());

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
			arrow::StringArray data_array =
				arrow::StringArray(arrow_table->column(0)->data()->chunk(0)->data());

			size = data_array.length();

			if (null_indicies != nullptr)
			{
				null_indicies->clear();
				for (int i = 0; i < size; i++)
				{
					if (data_array.IsNull(i))
						null_indicies->push_back(i);
				}
			}

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

	bool SetPQPath(std::string file_path)
	{
		current_row_group_ = 0;
		row_group_count_ = 0;

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
		if (file_path.size() > 259)
		{
			std::filesystem::path modified_path("\\\\?\\");
			modified_path += file_path;
			file_path = modified_path.string();
		}		

		if (arrow_file_ != nullptr)
		{
			if (!arrow_file_->closed())
				arrow_file_->Close();
		}

		//printf("Reading parquet file: %s\n", file_path.c_str());
		current_row_group_ = 0;

		// Open file reader.
		st_ = arrow::io::ReadableFile::Open(file_path, pool_, &arrow_file_);
		if (!st_.ok())
		{
			printf("arrow::io::ReadableFile::Open error (ID %s): %s\n",
				st_.CodeAsString().c_str(), st_.message().c_str());
			return false;
		}
		st_ = parquet::arrow::OpenFile(arrow_file_, pool_, &arrow_reader_);
		if (!st_.ok())
		{
			printf("parquet::arrow::OpenFile error (ID %s): %s\n",
				st_.CodeAsString().c_str(), st_.message().c_str());
			return false;
		}


		arrow_reader_->set_use_threads(true);
		arrow_reader_->set_num_threads(2);

		if (!st_.ok())
		{
			printf("GetSchema() error (ID %s): %s\n",
				st_.CodeAsString().c_str(), st_.message().c_str());
			return false;
		}

		// Total count of row groups.
		row_group_count_ = arrow_reader_->num_row_groups();
		//printf("%02d row groups\n", row_group_count_);

		return true;
	}
};

TEST_F(ParquetContextTest, MultipleColumns)
{
	//												column1		column2
	std::vector<std::vector<int64_t>> file = { {16,5,4,9,8}, {7,8,20,50,60} };

	std::string file_name = "file.parquet";

	ASSERT_TRUE(CreateParquetFile(arrow::int64(), file_name, file, 10));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int64_t> input;

	// First Column
	bool ret_val = GetNextRG<int64_t,arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_TRUE(ret_val);
	EXPECT_EQ(input.size(), 5);
	ASSERT_THAT(input, ::testing::ElementsAre(16,5,4,9,8));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_FALSE(ret_val);

	// Second Column
	input.clear();
	ASSERT_TRUE(SetPQPath(file_name));
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(1, input);
	ASSERT_TRUE(ret_val);
	EXPECT_EQ(input.size(), 5);
	ASSERT_THAT(input, ::testing::ElementsAre(7,8,20,50,60));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, SmallRowGroupSize)
{
	//												column1		
	std::vector<std::vector<int64_t>> file = { {16,5,4,9,8} };

	std::string file_name = "file.parquet";

	ASSERT_TRUE(CreateParquetFile(arrow::int64(), file_name, file, 3));


	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int64_t> input;

	// First Row Group
	bool ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_TRUE(ret_val);
	EXPECT_EQ(input.size(), 3);
	ASSERT_THAT(input, ::testing::ElementsAre(16, 5, 4));

	// Second Row Group
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_TRUE(ret_val);
	EXPECT_EQ(input.size(), 2);
	ASSERT_THAT(input, ::testing::ElementsAre(9,8));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, NoCastingDownTest)
{
	std::vector<int64_t> data = { 16,5,4,9,8 };
	std::vector<std::vector<int64_t>> file;
	file.push_back(data);

	std::string file_name = "file.parquet";

	ASSERT_FALSE(CreateParquetFile(arrow::int32(), file_name, file, 50));

	ASSERT_FALSE(SetPQPath(file_name));
}

TEST_F(ParquetContextTest, InvalidListSizeMultiple)
{
	std::vector<int32_t> data = { 16,5,4,9,8 };

	std::string file_name = "file.parquet";

	// list size of 3 is not a multiple of 5 as seen in data
	ASSERT_FALSE(CreateParquetFileList(arrow::int32(), file_name, data, 2, 3));

	ASSERT_FALSE(SetPQPath(file_name));
}

TEST_F(ParquetContextTest, InvalidListSizeMultipleString)
{
	std::vector<std::string> data = { "a","b","c","d","e" };

	std::string file_name = "file.parquet";

	// list size of 3 is not a multiple of 5 as seen in data
	ASSERT_FALSE(CreateParquetFileList(arrow::utf8(), file_name, data, 2, 3));

	ASSERT_FALSE(SetPQPath(file_name));
}

TEST_F(ParquetContextTest, CastingSameSizeAcceptable)
{
	std::vector<uint64_t> data = { 16,5,4,9,8 };
	std::vector<std::vector<uint64_t>> file;
	file.push_back(data);

	std::string file_name = "file.parquet";

	ASSERT_TRUE(CreateParquetFile(arrow::int64(), file_name, file, 50));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int64_t> input;
	bool ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(data, ::testing::ElementsAreArray(input));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, int64Test)
{
	std::vector<uint64_t> data = { 16,5,4,9,8 };
	std::vector<std::vector<uint64_t>> file;
	file.push_back(data);

	std::string file_name = "file.parquet";

	ASSERT_FALSE(CreateParquetFile(arrow::uint64(), file_name, file, 50));

	// Cannot output unsigned 
	ASSERT_FALSE(SetPQPath(file_name));

	// cast
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), file_name, file, 50));
	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int64_t> input;
	bool ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(data, ::testing::ElementsAreArray(input));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_FALSE(ret_val);

	// no cast
	std::vector<int64_t> data_ = { 19,20,5,3,9 };
	std::vector<std::vector<int64_t>> file_;
	file_.push_back(data_);

	ASSERT_TRUE(CreateParquetFile(arrow::int64(), file_name, file_, 50));

	ASSERT_TRUE(SetPQPath(file_name));

	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(data_ , ::testing::ElementsAreArray(input));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, int32Test)
{
	std::vector<uint32_t> data = { 16,5,4,9,8 };
	std::vector<std::vector<uint32_t>> file;
	file.push_back(data);

	std::string file_name = "file.parquet";

	ASSERT_FALSE(CreateParquetFile(arrow::uint32(), file_name, file, 50));

	// Cannot output unsigned 
	ASSERT_FALSE(SetPQPath(file_name));

	// cast
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), file_name, file, 50));
	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int64_t> input;
	bool ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(data, ::testing::ElementsAreArray(input));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_FALSE(ret_val);

	// no cast
	std::vector<int32_t> data_ = { 19,20,5,3,9 };
	std::vector<std::vector<int32_t>> file_;
	file_.push_back(data_);

	ASSERT_TRUE(CreateParquetFile(arrow::int32(), file_name, file_, 50));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int32_t> input_;
	ret_val = GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, input_);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(data_, ::testing::ElementsAreArray(input_));

	// Assert end of data
	input_.clear();
	ret_val = GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, input_, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, int16Test)
{
	std::vector<uint16_t> data = { 16,5,4,9,8 };
	std::vector<std::vector<uint16_t>> file;
	file.push_back(data);

	std::string file_name = "file.parquet";

	ASSERT_FALSE(CreateParquetFile(arrow::uint16(), file_name, file, 50));

	// Cannot output unsigned 
	ASSERT_FALSE(SetPQPath(file_name));

	// cast
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), file_name, file, 50));
	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int32_t> input;
	bool ret_val = GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(data, ::testing::ElementsAreArray(input));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, input, true);
	ASSERT_FALSE(ret_val);

	// no cast
	std::vector<int16_t> data_ = { 19,20,5,3,9 };
	std::vector<std::vector<int16_t>> file_;
	file_.push_back(data_);

	ASSERT_TRUE(CreateParquetFile(arrow::int16(), file_name, file_, 50));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int16_t> input_;
	ret_val = GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, input_);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(data_, ::testing::ElementsAreArray(input_));

	// Assert end of data
	input_.clear();
	ret_val = GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, input_, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, int8Test)
{
	std::vector<uint8_t> data = { 16,5,4,9,8 };
	std::vector<std::vector<uint8_t>> file;
	file.push_back(data);

	std::string file_name = "file.parquet";

	ASSERT_FALSE(CreateParquetFile(arrow::uint8(), file_name, file, 50));

	// Cannot output unsigned 
	ASSERT_FALSE(SetPQPath(file_name));

	// cast
	ASSERT_TRUE(CreateParquetFile(arrow::int16(), file_name, file, 50));
	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int16_t> input;
	bool ret_val = GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(data, ::testing::ElementsAreArray(input));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, input, true);
	ASSERT_FALSE(ret_val);

	// no cast
	std::vector<int8_t> data_ = { 19,20,5,3,9 };
	std::vector<std::vector<int8_t>> file_;
	file_.push_back(data_);

	ASSERT_TRUE(CreateParquetFile(arrow::int8(), file_name, file_, 50));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int8_t> input_;
	ret_val = GetNextRG<int8_t, arrow::NumericArray<arrow::Int8Type>>(0, input_);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(data_, ::testing::ElementsAreArray(input_));

	// Assert end of data
	input_.clear();
	ret_val = GetNextRG<int8_t, arrow::NumericArray<arrow::Int8Type>>(0, input_, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, Float32Test)
{
	std::vector<float> data = { 16.546,5.9856,4.153,9.531,8.897 };
	std::vector<std::vector<float>> file;
	file.push_back(data);

	std::string file_name = "file.parquet";

	// no casting of floating point
	ASSERT_FALSE(CreateParquetFile(arrow::float64(), file_name, file, 50));
	ASSERT_FALSE(SetPQPath(file_name));

	// no cast
	std::vector<float> data_ = { 16.546,5.9856,4.153,9.531,8.894547 };
	std::vector<std::vector<float>> file_;
	file_.push_back(data_);

	ASSERT_TRUE(CreateParquetFile(arrow::float32(), file_name, file_, 50));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<float> input_;
	bool ret_val = GetNextRG<float, arrow::NumericArray<arrow::FloatType>>(0, input_);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(data_, ::testing::ElementsAreArray(input_));

	// Assert end of data
	ret_val = GetNextRG<float, arrow::NumericArray<arrow::FloatType>>(0, input_, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, DoubleTest)
{
	std::vector<double> data = { 16.546,5.9856,4.153,9.531,8.897 };
	std::vector<std::vector<double>> file;
	file.push_back(data);

	std::string file_name = "file.parquet";

	// no casting of floating point
	ASSERT_FALSE(CreateParquetFile(arrow::float32(), file_name, file, 50));
	ASSERT_FALSE(SetPQPath(file_name));

	// no cast
	std::vector<double> data_ = { 16.546,5.9856,4.153,9.531,8.894547 };
	std::vector<std::vector<double>> file_;
	file_.push_back(data_);

	ASSERT_TRUE(CreateParquetFile(arrow::float64(), file_name, file_, 50));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<double> input_;
	bool ret_val = GetNextRG<double, arrow::NumericArray<arrow::DoubleType>>(0, input_);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(data_, ::testing::ElementsAreArray(input_));

	// Assert end of data
	ret_val = GetNextRG<double, arrow::NumericArray<arrow::DoubleType>>(0, input_, true);
	ASSERT_FALSE(ret_val);
}

// String/Boolean/and Numeric cover non list multiple row group tests
TEST_F(ParquetContextTest, BooleanTest)
{
	std::vector<uint8_t> data = { 1,0,1,1,0 };
	std::vector<std::vector<uint8_t>> file;
	file.push_back(data);

	std::string file_name = "file.parquet";

	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), file_name, file, 2));
	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<bool> input;
	bool ret_val = GetNextRGBool(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(1,0));

	input.clear();
	ret_val = GetNextRGBool(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(1,1));

	input.clear();
	ret_val = GetNextRGBool(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(0));

	// Assert end of data
	input.clear();
	ret_val = GetNextRGBool(0, input, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, StringTest)
{
	std::vector<std::string> data = { "a","bkjk;","jke;a","j","a" };
	std::vector<std::vector<std::string>> file;
	file.push_back(data);

	std::string file_name = "file.parquet";

	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), file_name, file, 2));
	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<std::string> input;
	bool ret_val = GetNextRGString(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre("a","bkjk;"));

	input.clear();
	ret_val = GetNextRGString(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre("jke;a", "j"));

	input.clear();
	ret_val = GetNextRGString(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre("a"));

	// Assert end of data
	input.clear();
	ret_val = GetNextRGString(0, input, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, NumericMultipleRGNonList)
{
	std::vector<double> data = { 1.232, 1.567, -567.4, 5.43, -100 };
	std::vector<std::vector<double>> file;
	file.push_back(data);

	std::string file_name = "file.parquet";

	ASSERT_TRUE(CreateParquetFile(arrow::float64(), file_name, file, 2));
	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<double> input;
	bool ret_val = GetNextRG<double, arrow::NumericArray<arrow::DoubleType>>(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(1.232, 1.567));

	input.clear();
	ret_val = GetNextRG<double, arrow::NumericArray<arrow::DoubleType>>(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(-567.4, 5.43));

	input.clear();
	ret_val = GetNextRG<double, arrow::NumericArray<arrow::DoubleType>> (0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(-100));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<double, arrow::NumericArray<arrow::DoubleType>>(0, input, true);
	ASSERT_FALSE(ret_val);
}


TEST_F(ParquetContextTest, int64List)
{
	std::vector<uint64_t> file = { 16,5,4,9,8,23,5,4,33,4 };


	std::string file_name = "file.parquet";

	ASSERT_FALSE(CreateParquetFileList(arrow::uint64(), file_name, file, 50, 2));

	// Cannot output unsigned 
	ASSERT_FALSE(SetPQPath(file_name));

	// cast
	ASSERT_TRUE(CreateParquetFileList(arrow::int64(), file_name, file, 50, 2));
	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int64_t> input;
	bool ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(file, ::testing::ElementsAreArray(input));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_FALSE(ret_val);

	// no cast
	std::vector<int64_t> file_ = { 19,20,5,3,9,7 };

	ASSERT_TRUE(CreateParquetFileList(arrow::int64(), file_name, file_, 50, 3));

	ASSERT_TRUE(SetPQPath(file_name));

	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(file_, ::testing::ElementsAreArray(input));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, int32List)
{
	std::vector<uint32_t> file = { 16,5,4,9,8,23,5,4,33,4 };


	std::string file_name = "file.parquet";

	ASSERT_FALSE(CreateParquetFileList(arrow::uint32(), file_name, file, 50, 2));

	// Cannot output unsigned 
	ASSERT_FALSE(SetPQPath(file_name));

	// cast
	ASSERT_TRUE(CreateParquetFileList(arrow::int64(), file_name, file, 50, 2));
	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int64_t> input;
	bool ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(file, ::testing::ElementsAreArray(input));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_FALSE(ret_val);

	// no cast
	std::vector<int32_t> file_ = { 19,20,5,3,9,7 };

	ASSERT_TRUE(CreateParquetFileList(arrow::int32(), file_name, file_, 50, 3));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int32_t> input_;
	ret_val = GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, input_, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(file_, ::testing::ElementsAreArray(input_));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, input_, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, int16List)
{
	std::vector<uint16_t> file = { 16,5,4,9,8,23,5,4,33,4 };


	std::string file_name = "file.parquet";

	ASSERT_FALSE(CreateParquetFileList(arrow::uint16(), file_name, file, 50, 2));

	// Cannot output unsigned 
	ASSERT_FALSE(SetPQPath(file_name));

	// cast
	ASSERT_TRUE(CreateParquetFileList(arrow::int32(), file_name, file, 50, 2));
	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int32_t> input;
	bool ret_val = GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(file, ::testing::ElementsAreArray(input));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, input, true);
	ASSERT_FALSE(ret_val);

	// no cast
	std::vector<int16_t> file_ = { 19,20,5,3,9,7 };

	ASSERT_TRUE(CreateParquetFileList(arrow::int16(), file_name, file_, 50, 3));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int16_t> input_;
	ret_val = GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, input_, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(file_, ::testing::ElementsAreArray(input_));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, input_, true);
	ASSERT_FALSE(ret_val);
}


TEST_F(ParquetContextTest, int8List)
{
	std::vector<uint8_t> file = { 16,5,4,9,8,23,5,4,33,4 };

	std::string file_name = "file.parquet";

	ASSERT_FALSE(CreateParquetFileList(arrow::uint8(), file_name, file, 50, 2));

	// Cannot output unsigned 
	ASSERT_FALSE(SetPQPath(file_name));

	// cast
	ASSERT_TRUE(CreateParquetFileList(arrow::int16(), file_name, file, 50, 2));
	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int16_t> input;
	bool ret_val = GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(file, ::testing::ElementsAreArray(input));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, input, true);
	ASSERT_FALSE(ret_val);

	// no cast
	std::vector<int8_t> file_ = { 19,20,5,3,9,7 };

	ASSERT_TRUE(CreateParquetFileList(arrow::int8(), file_name, file_, 50, 3));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int8_t> input_;
	ret_val = GetNextRG<int8_t, arrow::NumericArray<arrow::Int8Type>>(0, input_, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(file_, ::testing::ElementsAreArray(input_));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int8_t, arrow::NumericArray<arrow::Int8Type>>(0, input_, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, StringList)
{
	std::string file_name = "file.parquet";
	std::vector<std::string> file = { "bob","george","dale","Pale","F","Jackie Chan" };

	ASSERT_TRUE(CreateParquetFileList(arrow::utf8(), file_name, file, 50, 3));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<std::string> input;
	bool ret_val = GetNextRGString(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(file, ::testing::ElementsAreArray(input));

	// Assert end of data
	input.clear();
	ret_val = GetNextRGString(0, input, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, BoolList)
{
	std::string file_name = "file.parquet";
	std::vector<uint8_t> file = { 0,1,1,0,1,0,1,0,0,1 };

	ASSERT_TRUE(CreateParquetFileList(arrow::boolean(), file_name, file, 50, 2));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<bool> input;
	bool ret_val = GetNextRGBool(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(file, ::testing::ElementsAreArray(input));

	// Assert end of data
	input.clear();
	ret_val = GetNextRGBool(0, input, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, WriteColumnsNoArguments)
{
	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());

	std::string file_name = "file.parquet";
	std::vector<int64_t> file =
	{ 1,2,3,4,5,6,7,8 };

	file_name = "./" + file_name;

	ParquetContext* pc = new ParquetContext(50);

	pc->AddField(arrow::int64(), "data");
	pc->SetMemoryLocation<int64_t>(file, "data");


	pc->OpenForWrite(file_name, true);

	// Should return false because default 
	// row group size of 50 exceeds value count
	ASSERT_FALSE(pc->WriteColumns());

	pc->Close();
	pq_file = file_name;
	delete pc;

	ASSERT_FALSE(SetPQPath(file_name));

	ParquetContext* pc2 = new ParquetContext(8);

	pc2->AddField(arrow::int64(), "data");
	pc2->SetMemoryLocation<int64_t>(file, "data");


	pc2->OpenForWrite(file_name, true);


	ASSERT_TRUE(pc2->WriteColumns());

	pc2->Close();
	pq_file = file_name;
	delete pc2;

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<std::int64_t> input;
	bool ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_FALSE(ret_val);

	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());
}

TEST_F(ParquetContextTest, BoolListMultipleRG)
{
	std::string file_name = "file.parquet";
	std::vector<uint8_t> file = { 0,1,1,0,1,0,1,0,0,1,0,1,1,1 };

	ASSERT_TRUE(CreateParquetFileList(arrow::boolean(), file_name, file, 3, 2));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<bool> input;
	bool ret_val = GetNextRGBool(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(0, 1, 1, 0, 1, 0));

	input.clear();
	ret_val = GetNextRGBool(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(1, 0, 0, 1, 0, 1));

	input.clear();
	ret_val = GetNextRGBool(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(1, 1));

	// Assert end of data
	ret_val = GetNextRGBool(0, input, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, Int64ListMultipleRG)
{
	std::string file_name = "file.parquet";
	std::vector<int64_t> file = { 5,10,12,13,15,8,9,1,3,4,5,7,9,6 };

	ASSERT_TRUE(CreateParquetFileList(arrow::int64(), file_name, file, 3, 2));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int64_t> input;
	bool ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(5, 10, 12, 13, 15, 8));

	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(9, 1, 3, 4, 5, 7));

	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(9, 6));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, Int32ListWithCastMultipleRG)
{
	std::string file_name = "file.parquet";
	std::vector<uint16_t> file = { 5,10,12,13,15,8,9,1,3,4,5,6,9,8 };

	ASSERT_TRUE(CreateParquetFileList(arrow::int32(), file_name, file, 3, 2));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int32_t> input;
	bool ret_val = GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(5, 10, 12, 13, 15, 8));

	input.clear();
	ret_val = GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(9, 1, 3, 4, 5, 6));

	input.clear();
	ret_val = GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(9, 8));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, input, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, StringListMultipleRG)
{
	std::string file_name = "file.parquet";
	std::vector<std::string> file = 
	{ "a","b","d","jack","bell","bell",
		"dale","fell","cake","say what??","ee","ff",
		"gg","d2"};

	ASSERT_TRUE(CreateParquetFileList(arrow::utf8(), file_name, file, 3, 2));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<std::string> input;
	bool ret_val = GetNextRGString(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre("a", "b", "d", "jack", "bell", "bell"));

	input.clear();
	ret_val = GetNextRGString(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre("dale", "fell", "cake", "say what??","ee","ff"));

	input.clear();
	ret_val = GetNextRGString(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre("gg","d2"));

	// Assert end of data
	input.clear();
	ret_val = GetNextRGString(0, input, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, NumericWriteOutPortionOfVector)
{
	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());

	std::string file_name = "file.parquet";
	std::vector<int64_t> file =
	{ 1,2,3,4,5,6,7,8,9 };

	file_name = "./" + file_name;

	ParquetContext* pc = new ParquetContext(50);

	pc->AddField(arrow::int64(), "data");
	pc->SetMemoryLocation<int64_t>(file, "data");


	pc->OpenForWrite(file_name, true);

	// Offset
	pc->WriteColumns(2, 3);

	pc->Close();
	pq_file = file_name;
	delete pc;

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<std::int64_t> input;
	bool ret_val = GetNextRG<int64_t,arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(4,5));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_FALSE(ret_val);

	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());

	ParquetContext* pc2 = new ParquetContext(50);


	pc2->AddField(arrow::int64(), "data");
	pc2->SetMemoryLocation<int64_t>(file, "data");


	pc2->OpenForWrite(file_name, true);

	// No offset
	pc2->WriteColumns(2, 0);

	pc2->Close();
	pq_file = file_name;
	delete pc2;

	ASSERT_TRUE(SetPQPath(file_name));

	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(1,2));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_FALSE(ret_val);

	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());
}

TEST_F(ParquetContextTest, StringWriteOutPortionOfVector)
{
	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());

	std::string file_name = "file.parquet";
	std::vector<std::string> file =
	{ "a","b","d","jack","bell","bell",
		"dale","fell","cake","say what??","ee","ff",
		"gg","d2" };

	file_name = "./" + file_name;


	ParquetContext* pc = new ParquetContext(50);


	pc->AddField(arrow::utf8(), "data");
	pc->SetMemoryLocation<std::string>(file, "data");	


	pc->OpenForWrite(file_name, true);
	
	// Offset
	pc->WriteColumns(1, 3);

	pc->Close();
	pq_file = file_name;
	delete pc;

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<std::string> input;
	bool ret_val = GetNextRGString(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre("jack"));

	// Assert end of data
	input.clear();
	ret_val = GetNextRGString(0, input);
	ASSERT_FALSE(ret_val);

	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());
	
	ParquetContext* pc2 = new ParquetContext(50);


	pc2->AddField(arrow::utf8(), "data");
	pc2->SetMemoryLocation<std::string>(file, "data");


	pc2->OpenForWrite(file_name, true);

	// No Offset
	pc2->WriteColumns(2, 0);

	pc2->Close();
	pq_file = file_name;
	delete pc2;

	ASSERT_TRUE(SetPQPath(file_name));

	input.clear();
	ret_val = GetNextRGString(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre("a", "b"));

	// Assert end of data
	input.clear();
	ret_val = GetNextRGString(0, input);
	ASSERT_FALSE(ret_val);

	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());
}

TEST_F(ParquetContextTest, NumericListWriteOutPortionOfVector)
{
	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());

	std::string file_name = "file.parquet";
	std::vector<int64_t> file =
	{ 1,2,3,4,5,6,7,8,9 };

	file_name = "./" + file_name;

	ParquetContext* pc = new ParquetContext(50);

	pc->AddField(arrow::int64(), "data", 3);
	pc->SetMemoryLocation<int64_t>(file, "data");


	pc->OpenForWrite(file_name, true);

	// Offset
	pc->WriteColumns(2, 1);

	pc->Close();
	pq_file = file_name;
	delete pc;

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<std::int64_t> input;
	bool ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(4, 5, 6, 7, 8, 9));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_FALSE(ret_val);

	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());

	ParquetContext* pc2 = new ParquetContext(50);


	pc2->AddField(arrow::int64(), "data", 3);
	pc2->SetMemoryLocation<int64_t>(file, "data");


	pc2->OpenForWrite(file_name, true);

	// No offset
	pc2->WriteColumns(2, 0);

	pc2->Close();
	pq_file = file_name;
	delete pc2;

	ASSERT_TRUE(SetPQPath(file_name));

	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(1, 2, 3, 4, 5, 6));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_FALSE(ret_val);

	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());
}

TEST_F(ParquetContextTest, StringListWriteOutPortionOfVector)
{
	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());

	std::string file_name = "file.parquet";
	std::vector<std::string> file =
	{ "a","b","d","jack","bell","bell",
		"dale","fell","cake","say what??","ee","ff",
		"gg","d2","bk" };

	file_name = "./" + file_name;


	ParquetContext* pc = new ParquetContext(50);


	pc->AddField(arrow::utf8(), "data", 3);
	pc->SetMemoryLocation<std::string>(file, "data");


	pc->OpenForWrite(file_name, true);

	// Offset
	pc->WriteColumns(2, 1);

	pc->Close();
	pq_file = file_name;
	delete pc;

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<std::string> input;
	bool ret_val = GetNextRGString(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre("jack", "bell", "bell", "dale", "fell", "cake"));

	// Assert end of data
	input.clear();
	ret_val = GetNextRGString(0, input, true);
	ASSERT_FALSE(ret_val);

	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());

	ParquetContext* pc2 = new ParquetContext(50);


	pc2->AddField(arrow::utf8(), "data", 3);
	pc2->SetMemoryLocation<std::string>(file, "data");


	pc2->OpenForWrite(file_name, true);

	// No Offset
	pc2->WriteColumns(2, 0);

	pc2->Close();
	pq_file = file_name;
	delete pc2;

	ASSERT_TRUE(SetPQPath(file_name));

	input.clear();
	ret_val = GetNextRGString(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre("a", "b", "d", "jack", "bell", "bell"));

	// Assert end of data
	input.clear();
	ret_val = GetNextRGString(0, input, true);
	ASSERT_FALSE(ret_val);

	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());
}

TEST_F(ParquetContextTest, DefaultRGWriteColumnsDecoupled)
{
	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());

	std::string file_name = "file.parquet";
	std::vector<int64_t> file =
	{ 1,2,3,4,5,6,7,8,9 };

	file_name = "./" + file_name;

	// Default Row Group size of 2 is smaller than
	// the rows passed to Write Columns of 6
	ParquetContext* pc = new ParquetContext(2);

	pc->AddField(arrow::int64(), "data");
	pc->SetMemoryLocation<int64_t>(file, "data");


	pc->OpenForWrite(file_name, true);

	pc->WriteColumns(6, 0);

	pc->Close();
	pq_file = file_name;
	delete pc;

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<std::int64_t> input;
	bool ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(1, 2, 3, 4, 5, 6));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_FALSE(ret_val);

	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());
}

TEST_F(ParquetContextTest, NumericWriteOutMoreThanAvailable)
{
	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());

	std::string file_name = "file.parquet";
	std::vector<int64_t> file =
	{ 1,2,3,4,5,6,7,8 };

	file_name = "./" + file_name;

	ParquetContext* pc = new ParquetContext(50);

	pc->AddField(arrow::int64(), "data");
	pc->SetMemoryLocation<int64_t>(file, "data");


	pc->OpenForWrite(file_name, true);

	ASSERT_FALSE(pc->WriteColumns(8, 1));

	pc->Close();
	pq_file = file_name;
	delete pc;

	ASSERT_FALSE(SetPQPath(file_name));


	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());

	// List
	ParquetContext* pc2 = new ParquetContext(10);

	pc2->AddField(arrow::int64(), "data", 2);
	pc2->SetMemoryLocation<int64_t>(file, "data");


	pc2->OpenForWrite(file_name, true);

	ASSERT_FALSE(pc2->WriteColumns(4, 1));

	pc2->Close();
	pq_file = file_name;
	delete pc2;

	ASSERT_FALSE(SetPQPath(file_name));

	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());
}

TEST_F(ParquetContextTest, StringWriteOutMoreThanAvailable)
{
	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());

	std::string file_name = "file.parquet";
	std::vector<std::string> file =
	{ "a","b","c","d" };

	file_name = "./" + file_name;

	// row group greater than row write length
	ParquetContext* pc = new ParquetContext(10);


	pc->AddField(arrow::utf8(), "data");
	pc->SetMemoryLocation<std::string>(file, "data");


	pc->OpenForWrite(file_name, true);

	ASSERT_FALSE(pc->WriteColumns(4, 1));

	pc->Close();
	pq_file = file_name;
	delete pc;

	ASSERT_FALSE(SetPQPath(file_name));

	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());

	// Lists
	ParquetContext* pc2 = new ParquetContext(10);

	pc2->AddField(arrow::utf8(), "data", 2);
	pc2->SetMemoryLocation<std::string>(file, "data");


	pc2->OpenForWrite(file_name, true);

	ASSERT_FALSE(pc2->WriteColumns(2, 1));

	pc2->Close();
	pq_file = file_name;
	delete pc2;

	ASSERT_FALSE(SetPQPath(file_name));

	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());
}

TEST_F(ParquetContextTest, NoCastingFromStrings)
{
	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());

	std::string file_name = "file.parquet";
	std::vector<std::string> file =
	{ "a","b","d","jack","bell","tell" };

	ASSERT_FALSE(CreateParquetFileList(arrow::int16(), file_name, file, 1, 2));

	ASSERT_FALSE(SetPQPath(file_name));
}

TEST_F(ParquetContextTest, NoCastingToStrings)
{
	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());

	std::string file_name = "file.parquet";
	std::vector<int16_t> file =
	{ 0,1,2,3,4,5 };

	ASSERT_FALSE(CreateParquetFileList(arrow::utf8(), file_name, file, 1, 2));

	ASSERT_FALSE(SetPQPath(file_name));
}

TEST_F(ParquetContextTest, NoCastingToBool)
{
	if (arrow_file_ != nullptr)
	{
		if (!arrow_file_->closed())
			arrow_file_->Close();
	}

	remove(pq_file.c_str());

	std::string file_name = "file.parquet";
	std::vector<int16_t> file =
	{ 0,1,2,3,4,5 };

	ASSERT_FALSE(CreateParquetFileList(arrow::boolean(), file_name, file, 3, 2));

	ASSERT_FALSE(SetPQPath(file_name));
}

// null lists are not possible and should yield
// all original values
TEST_F(ParquetContextTest, Int64ListNull)
{
	std::string file_name = "file.parquet";
	std::vector<int64_t> file =		   { 5,10,12,13,15,8,9,1,3,4,5,7,9,6 };
	std::vector<uint8_t> bool_fields = { 0,1,1,1,0,0,0,0,1,1,0,0,1,1, };

	ASSERT_TRUE(CreateParquetFileList(arrow::int64(), file_name, file, 3, 2, true, &bool_fields));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int64_t> input;
	bool ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(5, 10, 12, 13, 15, 8));

	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(9, 1, 3, 4, 5, 7));

	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(9, 6));

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, true);
	ASSERT_FALSE(ret_val);
}

// Null fields not applicable for lists 
// Should yield the same result
TEST_F(ParquetContextTest, StringListNull)
{
	std::string file_name = "file.parquet";
	std::vector<std::string> file =
	{ "a","b","d","jack","bell","bell",
		"dale","fell","cake","say what??","ee","ff",
		"gg","d2" };
	std::vector<uint8_t> null_fields = { 1,0,1,0,0,1,1,0,1,0,1,0,1,1 };

	ASSERT_TRUE(CreateParquetFileList(arrow::utf8(), file_name, file, 3, 2, true, &null_fields));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<std::string> input;
	bool ret_val = GetNextRGString(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre("a", "b", "d", "jack", "bell", "bell"));

	input.clear();
	ret_val = GetNextRGString(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre("dale", "fell", "cake", "say what??", "ee", "ff"));

	input.clear();
	ret_val = GetNextRGString(0, input, true);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre("gg", "d2"));

	// Assert end of data
	input.clear();
	ret_val = GetNextRGString(0, input, true);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, Int64Null)
{
	std::string file_name = "file.parquet";
	std::vector<std::vector<int64_t>> file = { { 5,10,12,13,15,8, 9,1,3,4,5,7, 9,6 } };
	std::vector<uint8_t> bool_fields =         { 0, 1, 1, 1, 0,0, 0,0,1,1,0,0, 1,1 };

	ASSERT_TRUE(CreateParquetFile(arrow::int64(), file_name, file, 6, true, &bool_fields));
	std::vector<size_t> null_indicies;
	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int64_t> input;
	bool ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, false, &null_indicies);
	ASSERT_TRUE(ret_val);
	ASSERT_EQ(input[1], 10);
	ASSERT_EQ(input[2], 12);
	ASSERT_EQ(input[3], 13);
	ASSERT_THAT(null_indicies, ::testing::ElementsAre(0, 4, 5));


	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, false, &null_indicies);
	ASSERT_TRUE(ret_val);
	ASSERT_EQ(input[2], 3);
	ASSERT_EQ(input[3], 4);
	ASSERT_THAT(null_indicies, ::testing::ElementsAre(0, 1, 4, 5));

	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, false, &null_indicies);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(9, 6));
	ASSERT_EQ(null_indicies.size(), 0);

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input, false, &null_indicies);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, Int64NullWithCast)
{
	std::string file_name = "file.parquet";
	std::vector<std::vector<uint16_t>> file = { { 5,10,12,13,15,8, 9,1,3,4,5,7, 9,6 } };
	std::vector<uint8_t> bool_fields = { 0, 1, 1, 1, 0,0, 0,0,1,1,0,0, 1,1 };

	ASSERT_TRUE(CreateParquetFile(arrow::int32(), file_name, file, 6, true, &bool_fields));
	std::vector<size_t> null_indicies;
	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<int32_t> input;
	bool ret_val = GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, input, false, &null_indicies);
	ASSERT_TRUE(ret_val);
	ASSERT_EQ(input[1], 10);
	ASSERT_EQ(input[2], 12);
	ASSERT_EQ(input[3], 13);
	ASSERT_THAT(null_indicies, ::testing::ElementsAre(0, 4, 5));


	input.clear();
	ret_val = GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, input, false, &null_indicies);
	ASSERT_TRUE(ret_val);
	ASSERT_EQ(input[2], 3);
	ASSERT_EQ(input[3], 4);
	ASSERT_THAT(null_indicies, ::testing::ElementsAre(0, 1, 4, 5));

	input.clear();
	ret_val = GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, input, false, &null_indicies);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(9, 6));
	ASSERT_EQ(null_indicies.size(), 0);

	// Assert end of data
	input.clear();
	ret_val = GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, input, false, &null_indicies);
	ASSERT_FALSE(ret_val);
}


TEST_F(ParquetContextTest, StringNull)
{
	std::string file_name = "file.parquet";
	std::vector<std::vector<std::string>> file =
	{ { "a","b","d","jack","bell","bell",
		"dale","fell","cake","say what??","ee","ff",
		"gg","d2" } };
	std::vector<uint8_t> null_fields = { 1,0,1,0,0,1,
										 1,0,1,0,1,0,
										 1,1 };

	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), file_name, file, 6, true, &null_fields));

	ASSERT_TRUE(SetPQPath(file_name));

	std::vector<std::string> input;
	std::vector<size_t> null_indicies;
	bool ret_val = GetNextRGString(0, input, false, &null_indicies);
	ASSERT_TRUE(ret_val);
	ASSERT_EQ(input[0], "a");
	ASSERT_EQ(input[2], "d");
	ASSERT_EQ(input[5], "bell");
	ASSERT_THAT(null_indicies, ::testing::ElementsAre(1,3,4));

	input.clear();
	ret_val = GetNextRGString(0, input, false, &null_indicies);
	ASSERT_TRUE(ret_val);
	ASSERT_EQ(input[0], "dale");
	ASSERT_EQ(input[2], "cake");
	ASSERT_EQ(input[4], "ee");
	ASSERT_THAT(null_indicies, ::testing::ElementsAre(1, 3, 5));

	input.clear();
	ret_val = GetNextRGString(0, input, false, &null_indicies);
	ASSERT_TRUE(ret_val);
	ASSERT_EQ(null_indicies.size(), 0);
	ASSERT_THAT(input, ::testing::ElementsAre("gg", "d2"));

	// Assert end of data
	input.clear();
	ret_val = GetNextRGString(0, input);
	ASSERT_FALSE(ret_val);
}

TEST_F(ParquetContextTest, StringNullVectorNotSameSizeAsDataVector)
{
	std::string file_name = "file.parquet";
	std::vector<std::vector<std::string>> file =
	{ { "a","b","d","jack","bell","bell",
		"dale","fell","cake","say what??","ee","ff",
		"gg","d2" } };
	std::vector<uint8_t> null_fields = { 1,0,1,0,0,1,
										 1,0,1,0,1,0,
										 1 };

	ASSERT_FALSE(CreateParquetFile(arrow::utf8(), file_name, file, 6, true, &null_fields));

	ASSERT_FALSE(SetPQPath(file_name)); 

}

TEST_F(ParquetContextTest, Int64NullNotSameSizeAsDataVector)
{
	std::string file_name = "file.parquet";
	std::vector<std::vector<int64_t>> file = { { 5,10,12,13,15,8, 9,1,3,4,5,7, 9,6 } };
	std::vector<uint8_t> bool_fields = { 0, 1, 1, 1, 0,0, 0,0,1,1,0,0, 1 };

	ASSERT_FALSE(CreateParquetFile(arrow::int64(), file_name, file, 6, true, &bool_fields));
	ASSERT_FALSE(SetPQPath(file_name));
}

// Arrow Truncation doesn't work
// See st_ = arrow::io::FileOutputStream::Open(path_, !truncate_, &ostream_);
// in void ParquetContext::write_cols_if_ready() and notice how it gets passed 
// correctly
//
// To make sure parquet context doesn't vary
// implementation if arrow fixes the truncate problem,
// trancate is hard coded to always remain true
TEST_F(ParquetContextTest, Truncate)
{
	//												column1		
	std::vector<std::vector<int64_t>> file = { {16,5,4,9,8} };

	std::string dirname1 = "file.parquet";

	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file, 50));


	ASSERT_TRUE(SetPQPath(dirname1));

	std::vector<int64_t> input;

	bool ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(16, 5, 4, 9, 8));

	// Write with truncation true
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file, 50, true));

	input.clear();
	ASSERT_TRUE(SetPQPath(dirname1));
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_TRUE(ret_val);
	ASSERT_THAT(input, ::testing::ElementsAre(16, 5, 4, 9, 8));
	if (!arrow_file_->closed())
		arrow_file_->Close();

	// Write with truncation false
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file, 50, false));

	input.clear();
	ASSERT_TRUE(SetPQPath(dirname1));
	ret_val = GetNextRG<int64_t, arrow::NumericArray<arrow::Int64Type>>(0, input);
	ASSERT_TRUE(ret_val);
	// If truncate were fixed and worked properly this test would be replaced with
	// ASSERT_THAT(input, ::testing::ElementsAre(16, 5, 4, 9, 8, 16, 5, 4, 9, 8));
	ASSERT_THAT(input, ::testing::ElementsAre(16, 5, 4, 9, 8));
}
