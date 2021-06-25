#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parquet_context.h"
#include "parquet_reader.h"


class ParquetReaderTest : public ::testing::Test
{
protected:
	std::vector<std::string> pq_files;
	std::vector<std::string> pq_directories;
	int pq_file_count;
	ParquetReaderTest()
	{

	}
	void SetUp() override
	{
		pq_file_count = 0;
	}
	~ParquetReaderTest()
	{
		for (int i = 0; i < pq_files.size(); i++)
		{
			remove(pq_files[i].c_str());
		}
		pq_files.clear();

		for (int i = 0; i < pq_directories.size(); i++)
		{
			std::filesystem::remove_all(pq_directories[i]);
		}
	}

	void TearDown()
	{

	}

	// Generate Parquet file with single value
	// columns
	template <typename T>
	bool CreateParquetFile(std::shared_ptr<arrow::DataType> type,
		std::string directory,
		std::vector<std::vector<T>> output,
		int row_group_count,
		std::vector<uint8_t>* bool_fields = nullptr)
	{
		if (!std::filesystem::exists(directory))
		{
			if (!std::filesystem::create_directory(directory))
			{
				printf("failed to create directory %s: \n", directory.c_str());
				return false;
			}
		}

		std::filesystem::path pqt_path(directory);
		pqt_path = pqt_path / std::filesystem::path(
			std::to_string(pq_file_count) + std::string(".parquet"));
		std::string path = pqt_path.string();

		ParquetContext pc(row_group_count);

		// Add each vector as a column
		for (int i = 0; i < output.size(); i++)
		{
			pc.AddField(type, "data" + std::to_string(i));
			pc.SetMemoryLocation<T>(output[i], "data" + std::to_string(i), bool_fields);
		}

		// Assume each vector is of the same size
		int row_size = output[0].size();

		if (!pc.OpenForWrite(path, true))
		{
			printf("failed to open parquet path %s\n", path.c_str());
			return false;
		}
		for (int i = 0; i < row_size / row_group_count; i++)
		{
			pc.WriteColumns(row_group_count, i * row_group_count);
		}

		// The the remaider rows if row_group_count is
		// not a multiple of the array size
		int remainder = row_size % row_group_count;
		if (remainder > 0)
		{
			pc.WriteColumns(remainder,
				(row_size / row_group_count) * row_group_count);
		}


		pc.Close();
		pq_files.push_back(path);
		pq_directories.push_back(directory);
		pq_file_count++;
		return true;
	}

	// Create parquet file with one list column
	template <typename T>
	bool CreateParquetFile(std::shared_ptr<arrow::DataType> type,
		std::string directory,
		std::vector<T>& output,
		int row_group_count, int list_size)
	{

		if (!std::filesystem::exists(directory))
		{
			if (!std::filesystem::create_directory(directory))
			{
				printf("failed to create directory %s: \n", directory.c_str());
				return false;
			}
		}

		std::filesystem::path pqt_path(directory);
		pqt_path = pqt_path / std::filesystem::path(
			std::to_string(pq_file_count) + std::string(".parquet"));

		std::string path = pqt_path.string();

		ParquetContext pc(row_group_count);

		// Add the vector as a list column
		pc.AddField(type, "data", list_size);
		pc.SetMemoryLocation<T>(output, "data");

		// Assume each vector is of the same size
		int row_count = output.size() / list_size;
		if (!pc.OpenForWrite(path, true))
		{
			printf("failed to open parquet path %s\n", path.c_str());
			return false;
		}

		int tot_row_groups = row_count / row_group_count;
		for (int i = 0; i < tot_row_groups; i++)
		{
			pc.WriteColumns(row_group_count, i * row_group_count);
		}

		// The the remaider rows if row_group_count is
		// not a multiple of the array size
		int remainder = row_count % row_group_count;
		int offset = tot_row_groups * row_group_count;
		if (remainder > 0)
		{
			pc.WriteColumns(remainder, offset);
		}

		pc.Close();
		pq_files.push_back(path);
		pq_directories.push_back(directory);
		pq_file_count++;
		return true;
	}

	// Generate Parquet file with two
	// columns with different schemas
	template <typename T1, typename T2>
	bool CreateTwoColParquetFile(std::string directory,
		std::shared_ptr<arrow::DataType> type1,
		std::vector<T1> output1,
		std::string colname1,
		std::shared_ptr<arrow::DataType> type2,
		std::vector<T2> output2,
		std::string colname2,
		int row_group_count)
	{
		if (!std::filesystem::exists(directory))
		{
			if (!std::filesystem::create_directory(directory))
			{
				printf("failed to create directory %s: \n", directory.c_str());
				return false;
			}
		}

		std::filesystem::path pqt_path(directory);
		pqt_path = pqt_path / std::filesystem::path(
			std::to_string(pq_file_count) + std::string(".parquet"));
		std::string path = pqt_path.string();

		ParquetContext pc(row_group_count);

		// Add each column
		pc.AddField(type1, colname1);
		pc.SetMemoryLocation(output1, colname1, nullptr);
		pc.AddField(type2, colname2);
		pc.SetMemoryLocation(output2, colname2, nullptr);

		// Assume each vector is of the same size
		int row_size = output1.size();

		if (!pc.OpenForWrite(path, true))
		{
			printf("failed to open parquet path %s\n", path.c_str());
			return false;
		}
		for (int i = 0; i < row_size / row_group_count; i++)
		{
			pc.WriteColumns(row_group_count, i * row_group_count);
		}

		// The the remaider rows if row_group_count is
		// not a multiple of the array size
		int remainder = row_size % row_group_count;
		if (remainder > 0)
		{
			pc.WriteColumns(remainder,
				(row_size / row_group_count) * row_group_count);
		}


		pc.Close();
		pq_files.push_back(path);
		pq_directories.push_back(directory);
		pq_file_count++;
		return true;
	}
};


TEST_F(ParquetReaderTest, GetNextRGMultipleFilesSingleRowGroup)
{
	int size;
	std::vector<std::vector<int32_t>> data1 = { { 1, 2, 3 } };
	std::vector<std::vector<int32_t>> data2 = { { 4, 5, 6 } };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data1, 1));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data2, 1));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	std::vector<int32_t> out(100);
	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 1);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 2);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 3);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 4);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 5);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 6);

	ASSERT_FALSE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 0);
}

TEST_F(ParquetReaderTest, GetNextRGManualIncrementInt32)
{
	int size;
	std::vector<std::vector<int32_t>> data1 = { { 1, 2, 3 } };
	std::vector<std::vector<int32_t>> data2 = { { 4, 5, 6 } };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data1, 1));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data2, 1));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	pm.SetManualRowgroupIncrementMode();

	std::vector<int32_t> out(100);
	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 1);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 1);

	pm.IncrementRG();

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 2);

	pm.IncrementRG();

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 3);

	pm.IncrementRG();

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 4);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 4);

	pm.IncrementRG();

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 5);

	pm.IncrementRG();

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 6);

	pm.IncrementRG();

	ASSERT_FALSE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 0);
}

TEST_F(ParquetReaderTest, IncrementRGDoesNotIncrementWhenSetManualRowgroupIncrementModeIsNotCalled)
{
	int size;
	std::vector<std::vector<int32_t>> data1 = { { 1, 2, 3 } };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data1, 1));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	std::vector<int32_t> out(100);
	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 1);

	pm.IncrementRG();
	pm.IncrementRG();
	pm.IncrementRG();
	pm.IncrementRG();

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 2);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 3);

	ASSERT_FALSE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 0);
}

TEST_F(ParquetReaderTest, GetNextRGMultipleFilesNonMultipleRowGroup)
{
	int size;
	std::vector<std::vector<int32_t>> data1 = { { 1, 2, 3, 4, 5 } };
	std::vector<std::vector<int32_t>> data2 = { { 6, 7, 8 } };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data1, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data2, 2));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	std::vector<int32_t> out(100);
	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 3);
	EXPECT_EQ(out[0], 1);
	EXPECT_EQ(out[1], 2);
	EXPECT_EQ(out[2], 3);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], 4);
	EXPECT_EQ(out[1], 5);


	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], 6);
	EXPECT_EQ(out[1], 7);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 8);

	ASSERT_FALSE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 0);
}

TEST_F(ParquetReaderTest, EmptyParquetFile)
{
	int size;
	ParquetReader pm;

	std::string dir = "empty_parquet.parquet";
	ASSERT_TRUE(std::filesystem::create_directory(dir));

	ASSERT_FALSE(pm.SetPQPath(dir));

	std::vector<int32_t> out(100);
	ASSERT_FALSE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
	ASSERT_EQ(size, 0);
	ASSERT_EQ(pm.GetSchema()->num_fields(), 0);

	ASSERT_TRUE(std::filesystem::remove_all(dir));
}

TEST_F(ParquetReaderTest, ParquetFolderWithParquetAndNonParquetFiles)
{
	std::string directory = "directory.parquet";

	if (std::filesystem::exists(directory))
	{
		ASSERT_TRUE(std::filesystem::remove_all(directory));
	}

	ASSERT_TRUE(std::filesystem::create_directory(directory));


	std::filesystem::path pqt_path(directory);
	pqt_path = pqt_path / std::filesystem::path(
		std::to_string(pq_file_count) + std::string(".parquet"));
	std::string path = pqt_path.string();

	// create text file that should be ignored
	std::filesystem::path text_path_(directory);
	text_path_ = pqt_path / std::filesystem::path(
		std::to_string(pq_file_count) + std::string(".txt"));
	std::string text_path = text_path_.string();

	std::ofstream file;
	file.open(text_path);
	file << "yo";
	file.close();

	int row_group_count = 3;

	ParquetContext pc(row_group_count);

	// Add each vector as a column
	pc.AddField(arrow::int16(), "data");

	std::vector<int16_t> output{ 1,2,3 };
	pc.SetMemoryLocation<int16_t>(output, "data");

	int row_size = output.size();

	ASSERT_TRUE(pc.OpenForWrite(path, true));

	for (int i = 0; i < row_size / row_group_count; i++)
	{
		pc.WriteColumns(row_group_count, i * row_group_count);
	}

	// The the remaider rows if row_group_count is
	// not a multiple of the array size
	int remainder = row_size % row_group_count;
	if (remainder > 0)
	{
		pc.WriteColumns(remainder,
			(row_size / row_group_count) * row_group_count);
	}

	pc.Close();
	pq_files.push_back(path);
	pq_files.push_back(text_path);
	pq_directories.push_back(directory);
	pq_file_count++;

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(directory));

	std::vector<int16_t> out(100);
	int size = 0;
	ASSERT_TRUE((pm.GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, out, size)));

	ASSERT_EQ(size, 3);

	ASSERT_FALSE((pm.GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, out, size)));
}


TEST_F(ParquetReaderTest, InvalidParquetFile)
{
	std::string directory = "directory.parquet";

	if (std::filesystem::exists(directory))
	{
		ASSERT_TRUE(std::filesystem::remove_all(directory));
	}

	ASSERT_TRUE(std::filesystem::create_directory(directory));


	std::filesystem::path pqt_path(directory);
	pqt_path = pqt_path / std::filesystem::path(
		std::to_string(pq_file_count) + std::string(".parquet"));
	std::string path = pqt_path.string();

	std::ofstream file;
	file.open(path);
	file << "yo";
	file.close();

	pq_files.push_back(path);
	pq_directories.push_back(directory);
	pq_file_count++;

	ParquetReader pm;
	ASSERT_FALSE(pm.SetPQPath(directory));

	std::vector<int16_t> out(100);
	int size = 0;
	ASSERT_FALSE((pm.GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, out, size)));

	ASSERT_EQ(size, 0);
	ASSERT_EQ(pm.GetSchema()->num_fields(), 0);
}

TEST_F(ParquetReaderTest, NonExistantParquetFile)
{
	int size;
	ParquetReader pm;

	std::string dir = "non_existant_parquet.parquet";

	ASSERT_FALSE(pm.SetPQPath(dir));

	std::vector<int32_t> out(100);
	ASSERT_FALSE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));

	ASSERT_EQ(size, 0);
	ASSERT_EQ(pm.GetSchema()->num_fields(), 0);
}

TEST_F(ParquetReaderTest, GetNextRGMultipleFilesReachesEnd)
{
	int size;
	std::vector<std::vector<int32_t>> data1 = { { 1, 2, 3, 4, 5 } };
	std::vector<std::vector<int32_t>> data2 = { { 6, 7, 8 } };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data1, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data2, 2));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	std::vector<int32_t> out(100);
	// out of bounds column
	ASSERT_FALSE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(1, out, size)));
	ASSERT_EQ(size, 0);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));

	ASSERT_FALSE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));

	ASSERT_FALSE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size)));
}

TEST_F(ParquetReaderTest, GetNextRGMultipleFilesSecondColumn)
{
	int size;
	std::vector<std::vector<int32_t>> data1 =
	{ {9, 9, 9, 9, 9}, { 1, 2, 3, 4, 5 } };
	std::vector<std::vector<int32_t>> data2 =
	{ {9, 9, 9},  { 6, 7, 8 } };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data1, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data2, 2));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	std::vector<int32_t> out(100);
	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(1, out, size)));
	ASSERT_EQ(size, 3);
	EXPECT_EQ(out[0], 1);
	EXPECT_EQ(out[1], 2);
	EXPECT_EQ(out[2], 3);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(1, out, size)));
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], 4);
	EXPECT_EQ(out[1], 5);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(1, out, size)));
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], 6);
	EXPECT_EQ(out[1], 7);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(1, out, size)));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 8);

	ASSERT_FALSE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(1, out, size)));
	ASSERT_EQ(size, 0);
}

TEST_F(ParquetReaderTest, GetNextRGMultipleFilesListColumn)
{
	int size;
	std::vector<uint16_t> data1 =
	{ 1,1,1,1,1, 2,2,2,2,2 };
	std::vector<uint16_t> data2 =
	{ 3,3,3,3,3, 4,4,4,4,4, 5,5,5,5,5, 6,6,6,6,6, 7,7,7,7,7 };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile<uint16_t>(arrow::int32(), dirname, data1, 1, 5));

	ASSERT_TRUE(CreateParquetFile<uint16_t>(arrow::int32(), dirname, data2, 2, 5));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	std::vector<int32_t> out(100);
	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size, true)));

	ASSERT_EQ(size, 5);
	EXPECT_EQ(out[0], 1);
	EXPECT_EQ(out[1], 1);
	EXPECT_EQ(out[2], 1);
	EXPECT_EQ(out[3], 1);
	EXPECT_EQ(out[4], 1);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size, true)));

	ASSERT_EQ(size, 5);
	EXPECT_EQ(out[0], 2);
	EXPECT_EQ(out[1], 2);
	EXPECT_EQ(out[2], 2);
	EXPECT_EQ(out[3], 2);
	EXPECT_EQ(out[4], 2);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size, true)));
	ASSERT_EQ(size, 10);
	EXPECT_EQ(out[0], 3);
	EXPECT_EQ(out[1], 3);
	EXPECT_EQ(out[2], 3);
	EXPECT_EQ(out[3], 3);
	EXPECT_EQ(out[4], 3);
	EXPECT_EQ(out[5], 4);
	EXPECT_EQ(out[6], 4);
	EXPECT_EQ(out[7], 4);
	EXPECT_EQ(out[8], 4);
	EXPECT_EQ(out[9], 4);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size, true)));
	ASSERT_EQ(size, 10);
	EXPECT_EQ(out[0], 5);
	EXPECT_EQ(out[1], 5);
	EXPECT_EQ(out[2], 5);
	EXPECT_EQ(out[3], 5);
	EXPECT_EQ(out[4], 5);
	EXPECT_EQ(out[5], 6);
	EXPECT_EQ(out[6], 6);
	EXPECT_EQ(out[7], 6);
	EXPECT_EQ(out[8], 6);
	EXPECT_EQ(out[9], 6);

	ASSERT_TRUE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size, true)));
	ASSERT_EQ(size, 5);
	EXPECT_EQ(out[0], 7);
	EXPECT_EQ(out[1], 7);
	EXPECT_EQ(out[2], 7);
	EXPECT_EQ(out[3], 7);
	EXPECT_EQ(out[4], 7);

	ASSERT_FALSE((pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size, true)));
	EXPECT_EQ(size, 0);
}

TEST_F(ParquetReaderTest, GetColumnNumberFromName)
{
	std::vector<uint16_t> data =
	{ 3,3,3,3,3, 4,4,4,4,4, 5,5,5,5,5, 6,6,6,6,6, 7,7,7,7,7 };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile<uint16_t>(arrow::int32(),dirname, data, 1, 5));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	ASSERT_EQ(pm.GetColumnNumberFromName("data"),0);
	ASSERT_EQ(pm.GetColumnNumberFromName("junk"),-1);
}

TEST_F(ParquetReaderTest, GetNextRGBool)
{
	int size;
	std::vector<std::vector<uint8_t>> data1 = { { 0, 1, 0, 1, 1 } };
	std::vector<std::vector<uint8_t>> data2 = { { 0, 0, 1} };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname, data1, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname, data2, 2));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	std::vector<uint8_t> out(100);

	// out of bounds column
	ASSERT_FALSE(pm.GetNextRGBool(1, out, size));
	ASSERT_EQ(size, 0);

	ASSERT_TRUE(pm.GetNextRGBool(0, out, size));
	ASSERT_EQ(size, 3);
	EXPECT_EQ(out[0], 0);
	EXPECT_EQ(out[1], 1);
	EXPECT_EQ(out[2], 0);

	ASSERT_TRUE(pm.GetNextRGBool(0, out, size));
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], 1);
	EXPECT_EQ(out[1], 1);

	ASSERT_TRUE(pm.GetNextRGBool(0, out, size));
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], 0);
	EXPECT_EQ(out[1], 0);

	ASSERT_TRUE(pm.GetNextRGBool(0, out, size));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 1);

	ASSERT_FALSE(pm.GetNextRGBool(0, out, size));
	EXPECT_EQ(size, 0);
}

TEST_F(ParquetReaderTest, GetNextRGBoolManualIncrement)
{
	int size;
	std::vector<std::vector<uint8_t>> data1 = { { 0, 1, 0, 1, 1 } };
	std::vector<std::vector<uint8_t>> data2 = { { 0, 0, 1} };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname, data1, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname, data2, 2));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	pm.SetManualRowgroupIncrementMode();

	std::vector<uint8_t> out(100);
	ASSERT_TRUE(pm.GetNextRGBool(0, out, size));
	ASSERT_EQ(size, 3);
	EXPECT_EQ(out[0], 0);
	EXPECT_EQ(out[1], 1);
	EXPECT_EQ(out[2], 0);

	ASSERT_TRUE(pm.GetNextRGBool(0, out, size));
	ASSERT_EQ(size, 3);
	EXPECT_EQ(out[0], 0);
	EXPECT_EQ(out[1], 1);
	EXPECT_EQ(out[2], 0);

	pm.IncrementRG();

	ASSERT_TRUE(pm.GetNextRGBool(0, out, size));
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], 1);
	EXPECT_EQ(out[1], 1);

	pm.IncrementRG();

	ASSERT_TRUE(pm.GetNextRGBool(0, out, size));
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], 0);
	EXPECT_EQ(out[1], 0);

	ASSERT_TRUE(pm.GetNextRGBool(0, out, size));
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], 0);
	EXPECT_EQ(out[1], 0);

	pm.IncrementRG();

	ASSERT_TRUE(pm.GetNextRGBool(0, out, size));
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 1);

	pm.IncrementRG();

	ASSERT_FALSE(pm.GetNextRGBool(0, out, size));
	EXPECT_EQ(size, 0);
}


TEST_F(ParquetReaderTest, GetNextRGString)
{
	int size;
	std::vector<std::vector<std::string>> data1 = { { "a","b","c","d","e" } };
	std::vector<std::vector<std::string>> data2 = { { "Han Solo", "chewbacca"} };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname, data1, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname, data2, 2));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	std::vector<std::string> out(100);

	// out of bounds
	ASSERT_FALSE(pm.GetNextRGString(1, out, size));
	ASSERT_EQ(size, 0);

	ASSERT_TRUE(pm.GetNextRGString(0, out, size));
	ASSERT_EQ(size, 3);
	EXPECT_EQ(out[0], "a");
	EXPECT_EQ(out[1], "b");
	EXPECT_EQ(out[2], "c");

	ASSERT_TRUE(pm.GetNextRGString(0, out, size));
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], "d");
	EXPECT_EQ(out[1], "e");

	ASSERT_TRUE(pm.GetNextRGString(0, out, size));
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], "Han Solo");
	EXPECT_EQ(out[1], "chewbacca");

	ASSERT_FALSE(pm.GetNextRGString(0, out, size));
	EXPECT_EQ(size, 0);
}

TEST_F(ParquetReaderTest, GetNextRGStringManualIncrement)
{
	int size;
	std::vector<std::vector<std::string>> data1 = { { "a","b","c","d","e" } };
	std::vector<std::vector<std::string>> data2 = { { "Han Solo", "chewbacca"} };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname, data1, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname, data2, 2));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	pm.SetManualRowgroupIncrementMode();

	std::vector<std::string> out(100);
	ASSERT_TRUE(pm.GetNextRGString(0, out, size));
	ASSERT_EQ(size, 3);
	EXPECT_EQ(out[0], "a");
	EXPECT_EQ(out[1], "b");
	EXPECT_EQ(out[2], "c");

	ASSERT_TRUE(pm.GetNextRGString(0, out, size));
	ASSERT_EQ(size, 3);
	EXPECT_EQ(out[0], "a");
	EXPECT_EQ(out[1], "b");
	EXPECT_EQ(out[2], "c");

	pm.IncrementRG();

	ASSERT_TRUE(pm.GetNextRGString(0, out, size));
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], "d");
	EXPECT_EQ(out[1], "e");

	ASSERT_TRUE(pm.GetNextRGString(0, out, size));
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], "d");
	EXPECT_EQ(out[1], "e");

	pm.IncrementRG();

	ASSERT_TRUE(pm.GetNextRGString(0, out, size));
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], "Han Solo");
	EXPECT_EQ(out[1], "chewbacca");

	pm.IncrementRG();

	ASSERT_FALSE(pm.GetNextRGString(0, out, size));
	EXPECT_EQ(size, 0);
}

TEST_F(ParquetReaderTest, GetNextRGBoolListColumn)
{
	int size;
	std::vector<uint8_t> data1 =
	{ 1,1,1,1,1, 0,0,0,0,0 };
	std::vector<uint8_t> data2 =
	{ 0,1,0,1,0, 0,0,1,1,0, 1,1,0,0,0 };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile<uint8_t>(arrow::boolean(), dirname, data1, 1, 5));

	ASSERT_TRUE(CreateParquetFile<uint8_t>(arrow::boolean(), dirname, data2, 2, 5));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	std::vector<uint8_t> out(100);
	ASSERT_TRUE((pm.GetNextRGBool(0, out, size, true)));

	ASSERT_EQ(size, 5);
	EXPECT_EQ(out[0], 1);
	EXPECT_EQ(out[1], 1);
	EXPECT_EQ(out[2], 1);
	EXPECT_EQ(out[3], 1);
	EXPECT_EQ(out[4], 1);

	ASSERT_TRUE((pm.GetNextRGBool(0, out, size, true)));

	ASSERT_EQ(size, 5);
	EXPECT_EQ(out[0], 0);
	EXPECT_EQ(out[1], 0);
	EXPECT_EQ(out[2], 0);
	EXPECT_EQ(out[3], 0);
	EXPECT_EQ(out[4], 0);

	ASSERT_TRUE((pm.GetNextRGBool(0, out, size, true)));
	ASSERT_EQ(size, 10);
	EXPECT_EQ(out[0], 0);
	EXPECT_EQ(out[1], 1);
	EXPECT_EQ(out[2], 0);
	EXPECT_EQ(out[3], 1);
	EXPECT_EQ(out[4], 0);
	EXPECT_EQ(out[5], 0);
	EXPECT_EQ(out[6], 0);
	EXPECT_EQ(out[7], 1);
	EXPECT_EQ(out[8], 1);
	EXPECT_EQ(out[9], 0);

	ASSERT_TRUE((pm.GetNextRGBool(0, out, size, true)));
	ASSERT_EQ(size, 5);
	EXPECT_EQ(out[0], 1);
	EXPECT_EQ(out[1], 1);
	EXPECT_EQ(out[2], 0);
	EXPECT_EQ(out[3], 0);
	EXPECT_EQ(out[4], 0);

	ASSERT_FALSE((pm.GetNextRGBool(0, out, size, true)));
	EXPECT_EQ(size, 0);
}

TEST_F(ParquetReaderTest, GetNextRGStringList)
{
	int size;
	std::vector<std::string> data1 =
	{ "hey", "jack"};
	std::vector<std::string> data2 =
	{ "ie", "chan", "yo", "whats up"};

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile<std::string>(arrow::utf8(), dirname, data1, 1, 2));

	ASSERT_TRUE(CreateParquetFile<std::string>(arrow::utf8(), dirname, data2, 1, 2));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	std::vector<std::string> out(100);
	ASSERT_TRUE((pm.GetNextRGString(0, out, size, true)));

	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], "hey");
	EXPECT_EQ(out[1], "jack");

	ASSERT_TRUE((pm.GetNextRGString(0, out, size, true)));

	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], "ie");
	EXPECT_EQ(out[1], "chan");

	ASSERT_TRUE((pm.GetNextRGString(0, out, size, true)));
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], "yo");
	EXPECT_EQ(out[1], "whats up");

	ASSERT_FALSE((pm.GetNextRGString(0, out, size, true)));
	EXPECT_EQ(size, 0);
}

TEST_F(ParquetReaderTest, GetNextRGEmptyFiles)
{
	int size;
	std::vector<std::vector<int16_t>> data1 = { {},{} };
	std::vector<std::vector<int16_t>> data2 = { {},{} };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname, data1, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname, data2, 2));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	std::vector<int16_t> out(100);
	ASSERT_FALSE((pm.GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, out, size)));
	ASSERT_EQ(size, 0);
}

TEST_F(ParquetReaderTest, GetNextRGEmptyFilesBetweenData)
{
	int size;
	std::vector<std::vector<int16_t>> data1 = { {} };
	std::vector<std::vector<int16_t>> data2 = { {1,5} };
	std::vector<std::vector<int16_t>> data3 = { {} };
	std::vector<std::vector<int16_t>> data4 = { {8,2,6} };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname, data1, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname, data2, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname, data3, 2));
	ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname, data4, 2));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	std::vector<int16_t> out(100);
	ASSERT_TRUE((pm.GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, out, size)));
	ASSERT_EQ(size, 2);
	ASSERT_EQ(out[0], 1);
	ASSERT_EQ(out[1], 5);

	out.clear();
	ASSERT_TRUE((pm.GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, out, size)));
	ASSERT_EQ(size, 2);
	ASSERT_EQ(out[0], 8);
	ASSERT_EQ(out[1], 2);

	out.clear();
	ASSERT_TRUE((pm.GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, out, size)));
	ASSERT_EQ(size, 1);
	ASSERT_EQ(out[0], 6);

	out.clear();
	ASSERT_FALSE((pm.GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, out, size)));
	ASSERT_EQ(size, 0);
}

TEST_F(ParquetReaderTest, SetPQPathChecksForConsistentSchemaType)
{
	int size;

	std::vector<int16_t> data1 = { 1,5 };
	std::vector<int32_t> data2 = { 8,2 };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateTwoColParquetFile(dirname, arrow::int16(), data1, "col1",
		arrow::int16(), data1, "col2", 2));
	ASSERT_TRUE(CreateTwoColParquetFile(dirname, arrow::int16(), data1, "col1",
		arrow::int32(), data2, "col2", 2));

	ParquetReader pm;
	ASSERT_FALSE(pm.SetPQPath(dirname));

	std::vector<int16_t> out(100);
	ASSERT_FALSE((pm.GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, out, size)));
	ASSERT_EQ(size, 0);
	ASSERT_EQ(pm.GetInputParquetPathsCount(), 0);
}

TEST_F(ParquetReaderTest, SetPQPathMoreColumnsInSecondFile)
{
	int size;

	std::vector<std::vector<int16_t>> data1 = { {1,5} };
	std::vector<std::vector<int16_t>> data2 = { {8,2,6}, {1,2,3} };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname, data1, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname, data2, 3));

	ParquetReader pm;
	ASSERT_FALSE(pm.SetPQPath(dirname));

	std::vector<int16_t> out(100);
	ASSERT_FALSE((pm.GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, out, size)));
	ASSERT_EQ(size, 0);
	ASSERT_EQ(pm.GetInputParquetPathsCount(), 0);
}

TEST_F(ParquetReaderTest, SetPQPathChecksForConsistentSchemaNames)
{
	int size;

	std::vector<int16_t> data1 = { 1,5 };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateTwoColParquetFile(dirname, arrow::int16(), data1, "col1",
		arrow::int16(), data1, "col2", 2));
	ASSERT_TRUE(CreateTwoColParquetFile(dirname, arrow::int16(), data1, "col1",
		arrow::int16(), data1, "col_INCONSISTENT", 2));

	ParquetReader pm;
	ASSERT_FALSE(pm.SetPQPath(dirname));

	std::vector<int16_t> out(100);
	ASSERT_FALSE((pm.GetNextRG<int16_t, arrow::NumericArray<arrow::Int16Type>>(0, out, size)));
	ASSERT_EQ(size, 0);
	ASSERT_EQ(pm.GetInputParquetPathsCount(), 0);
}
