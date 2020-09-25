#include <vector>
#include <string>
#include <filesystem>
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
	bool CreateParquetFile(std::string directory, std::vector<uint16_t>& output,
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
		pc.AddField(arrow::int32(), "data", list_size);
		pc.SetMemoryLocation<uint16_t>(output, "data");

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
	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 1);

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 2);

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 3);

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 4);

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 5);

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 6);
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
	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 1);

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 1);

	pm.IncrementRG();

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 2);

	pm.IncrementRG();

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 3);

	pm.IncrementRG();

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 4);

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 4);

	pm.IncrementRG();

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 5);

	pm.IncrementRG();

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 6);
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
	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 3);
	EXPECT_EQ(out[0], 1);
	EXPECT_EQ(out[1], 2);
	EXPECT_EQ(out[2], 3);

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], 4);
	EXPECT_EQ(out[1], 5);


	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], 6);
	EXPECT_EQ(out[1], 7);

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 8);
}

TEST_F(ParquetReaderTest, GetNextRGNoParquetFiles)
{
	int size;
	ParquetReader pm;
	ASSERT_FALSE(pm.SetPQPath("."));

	std::vector<int32_t> out(100);
	bool return_val =
		pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_FALSE(return_val);
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
	bool return_val =
		pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_TRUE(return_val);

	return_val =
		pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_TRUE(return_val);

	return_val =
		pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_TRUE(return_val);

	return_val =
		pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_TRUE(return_val);

	return_val =
		pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_FALSE(return_val);

	return_val =
		pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_FALSE(return_val);
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
	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(1, out, size);
	ASSERT_EQ(size, 3);
	EXPECT_EQ(out[0], 1);
	EXPECT_EQ(out[1], 2);
	EXPECT_EQ(out[2], 3);

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(1, out, size);
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], 4);
	EXPECT_EQ(out[1], 5);

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(1, out, size);
	ASSERT_EQ(size, 2);
	EXPECT_EQ(out[0], 6);
	EXPECT_EQ(out[1], 7);

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(1, out, size);
	ASSERT_EQ(size, 1);
	EXPECT_EQ(out[0], 8);
}

TEST_F(ParquetReaderTest, GetNextRGMultipleFilesListColumn)
{
	int size;
	std::vector<uint16_t> data1 =
	{ 1,1,1,1,1, 2,2,2,2,2 };
	std::vector<uint16_t> data2 =
	{ 3,3,3,3,3, 4,4,4,4,4, 5,5,5,5,5, 6,6,6,6,6, 7,7,7,7,7 };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(dirname, data1, 1, 5));

	ASSERT_TRUE(CreateParquetFile(dirname, data2, 2, 5));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	std::vector<int32_t> out(100);
	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size, true);

	ASSERT_EQ(size, 5);
	EXPECT_EQ(out[0], 1);
	EXPECT_EQ(out[1], 1);
	EXPECT_EQ(out[2], 1);
	EXPECT_EQ(out[3], 1);
	EXPECT_EQ(out[4], 1);

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size, true);

	ASSERT_EQ(size, 5);
	EXPECT_EQ(out[0], 2);
	EXPECT_EQ(out[1], 2);
	EXPECT_EQ(out[2], 2);
	EXPECT_EQ(out[3], 2);
	EXPECT_EQ(out[4], 2);

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size, true);
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

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size, true);
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

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size, true);
	ASSERT_EQ(size, 5);
	EXPECT_EQ(out[0], 7);
	EXPECT_EQ(out[1], 7);
	EXPECT_EQ(out[2], 7);
	EXPECT_EQ(out[3], 7);
	EXPECT_EQ(out[4], 7);

}

TEST_F(ParquetReaderTest, GetColumnNumberFromName)
{
	std::vector<uint16_t> data =
	{ 3,3,3,3,3, 4,4,4,4,4, 5,5,5,5,5, 6,6,6,6,6, 7,7,7,7,7 };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(dirname, data, 1, 5));

	ParquetReader pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	ASSERT_EQ(pm.GetColumnNumberFromName("data"),0);
	ASSERT_EQ(pm.GetColumnNumberFromName("junk"),-1);
}