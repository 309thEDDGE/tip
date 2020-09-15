#include <vector>
#include <string>
#include <filesystem>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parquet_context.h"
#include "parquet_manager.h"
#include "comparator.h"


class ParquetArrowValidatorTest : public ::testing::Test
{
protected:
	std::vector<std::string> pq_files;
	std::vector<std::string> pq_directories;
	int pq_file_count;
	ParquetArrowValidatorTest()
	{
		
	}
	void SetUp() override
	{
		pq_file_count = 0;
	}
	~ParquetArrowValidatorTest()
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
		pqt_path = pqt_path / std::filesystem::path(std::to_string(pq_file_count) + std::string(".parquet"));
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

		if(!pc.OpenForWrite(path, true))
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
		int row_count = output.size()/list_size;
		if(!pc.OpenForWrite(path, true))
		{
			printf("failed to open parquet path %s\n", path.c_str());
			return false;
		}
		
		int tot_row_groups = row_count / row_group_count;
		for (int i = 0; i < tot_row_groups; i++)
		{
			printf("in here?\n");
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


TEST_F(ParquetArrowValidatorTest, ParquetManagerGetNextRGMultipleFilesSingleRowGroup)
{
	int size;
	std::vector<std::vector<int32_t>> data1 = { { 1, 2, 3 } };
	std::vector<std::vector<int32_t>> data2 = { { 4, 5, 6 } };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data1, 1));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data2, 1));

	ParquetManager pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	std::vector<int32_t> out(100);
	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_EQ(size,1);
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


TEST_F(ParquetArrowValidatorTest, ParquetManagerGetNextRGMultipleFilesNonMultipleRowGroup)
{
	int size;
	std::vector<std::vector<int32_t>> data1 = { { 1, 2, 3, 4, 5 } };
	std::vector<std::vector<int32_t>> data2 = { { 6, 7, 8 } };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data1, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data2, 2));

	ParquetManager pm;
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

TEST_F(ParquetArrowValidatorTest, GetNextRGNoParquetFiles)
{
	int size;
	ParquetManager pm;
	ASSERT_FALSE(pm.SetPQPath("."));

	std::vector<int32_t> out(100);
	bool return_val = 
		pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size);
	ASSERT_FALSE(return_val);
}

TEST_F(ParquetArrowValidatorTest, ParquetManagerGetNextRGMultipleFilesReachesEnd)
{
	int size;
	std::vector<std::vector<int32_t>> data1 = { { 1, 2, 3, 4, 5 } };
	std::vector<std::vector<int32_t>> data2 = { { 6, 7, 8 } };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data1, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data2, 2));

	ParquetManager pm;
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

TEST_F(ParquetArrowValidatorTest, ParquetManagerGetNextRGMultipleFilesSecondColumn)
{
	int size;
	std::vector<std::vector<int32_t>> data1 = 
	{ {9, 9, 9, 9, 9}, { 1, 2, 3, 4, 5 } };
	std::vector<std::vector<int32_t>> data2 = 
	{ {9, 9, 9},  { 6, 7, 8 } };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data1, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname, data2, 2));

	ParquetManager pm;
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

TEST_F(ParquetArrowValidatorTest, ParquetManagerGetNextRGMultipleFilesListColumn)
{
	printf("test beginning");
	int size;
	std::vector<uint16_t> data1 = 
	{ 1,1,1,1,1, 2,2,2,2,2 };
	std::vector<uint16_t> data2 = 
	{ 3,3,3,3,3, 4,4,4,4,4, 5,5,5,5,5, 6,6,6,6,6, 7,7,7,7,7 };

	std::string dirname = "file1.parquet";
	printf("no parquet files created");
	ASSERT_TRUE(CreateParquetFile(dirname, data1, 1, 5));

	printf("parquet file created");
	ASSERT_TRUE(CreateParquetFile(dirname, data2, 2, 5));

	printf("parquet file created");

	ParquetManager pm;
	ASSERT_TRUE(pm.SetPQPath(dirname));

	printf("set pq path success");

	std::vector<int32_t> out(100);
	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size, true);

	printf("rg1");

	ASSERT_EQ(size, 5);
	EXPECT_EQ(out[0], 1);
	EXPECT_EQ(out[1], 1);
	EXPECT_EQ(out[2], 1);
	EXPECT_EQ(out[3], 1);
	EXPECT_EQ(out[4], 1);

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size, true);

	printf("rg2");
	ASSERT_EQ(size, 5);
	EXPECT_EQ(out[0], 2);
	EXPECT_EQ(out[1], 2);
	EXPECT_EQ(out[2], 2);
	EXPECT_EQ(out[3], 2);
	EXPECT_EQ(out[4], 2);

	pm.GetNextRG<int32_t, arrow::NumericArray<arrow::Int32Type>>(0, out, size, true);
	printf("rg3");
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
	printf("rg4");
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
	printf("rg5");
	ASSERT_EQ(size, 5);
	EXPECT_EQ(out[0], 7);
	EXPECT_EQ(out[1], 7);
	EXPECT_EQ(out[2], 7);
	EXPECT_EQ(out[3], 7);
	EXPECT_EQ(out[4], 7);

}


TEST_F(ParquetArrowValidatorTest, ComparatorInitializeNoFilesExist)
{
	Comparator comp;
	EXPECT_FALSE(comp.Initialize("file1.parquet", "file2.parquet"));
}

TEST_F(ParquetArrowValidatorTest, ComparatorInitializeNoFilesExist2)
{
	Comparator comp;
	std::string file1 = "file1.parquet";

	std::filesystem::create_directory(file1);

	EXPECT_FALSE(comp.Initialize(file1, "file2.parquet"));
	std::filesystem::remove_all(file1);
}

TEST_F(ParquetArrowValidatorTest, ComparatorInitializeInvalidFiles)
{
	int size;
	std::vector<uint16_t> data1 = { 1,1,1,1,1, 2,2,2,2,2 };

	std::string dirname = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(dirname, data1, 1, 5));

	Comparator comp;

	std::string invalid = "invalid.parquet";

	std::filesystem::create_directory(invalid);	

	EXPECT_FALSE(comp.Initialize(invalid, dirname));
	EXPECT_FALSE(comp.Initialize(dirname, invalid));

	std::filesystem::remove_all(invalid);
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareVecsMatchingSameSize)
{
	std::vector<int32_t> data_a = 
	{ 1,2,3,4,5,6,7,8,9,10 };
	std::vector<int32_t> data_b = 
	{ 1,2,3,4,5,6,7,8,9,10 };

	int size_a = 10;
	int size_b = 10;

	Comparator comp;

	EXPECT_TRUE(comp.CompareVecs<int32_t>(data_a, size_a, data_b, size_b));
	EXPECT_EQ(size_a, 0);
	EXPECT_EQ(size_b, 0);

	size_a = 10;
	size_b = 10;

	EXPECT_TRUE(comp.CompareVecs<int32_t>(data_a, size_a, data_b, size_b));
	EXPECT_EQ(size_a, 0);
	EXPECT_EQ(size_b, 0);
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareVecsNonMatchingSameSize)
{
	std::vector<int32_t> data_a = 
	{ 1,2,3,4,5,0,7,8,9,10 };
	std::vector<int32_t> data_b = 
	{ 1,2,3,4,5,6,7,8,9,10 };

	int size_a = 10;
	int size_b = 10;

	Comparator comp;

	EXPECT_FALSE(comp.CompareVecs<int32_t>(data_a, size_a, data_b, size_b));
	EXPECT_EQ(size_a, 0);
	EXPECT_EQ(size_b, 0);

	size_a = 10;
	size_b = 10;

	EXPECT_FALSE(comp.CompareVecs<int32_t>(data_a, size_a, data_b, size_b));
	EXPECT_EQ(size_a, 0);
	EXPECT_EQ(size_b, 0);
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareVecsMatchingLargerDataA)
{
	std::vector<int32_t> data_a = 
	{ 1,2,3,4,5,6,7,8,9,10,11,12,13 };
	std::vector<int32_t> data_b = 
	{ 1,2,3,4,5,6,7,8,9,10 };

	int size_a = 13;
	int size_b = 10;

	Comparator comp;

	EXPECT_TRUE(comp.CompareVecs<int32_t>(data_a, size_a, data_b, size_b));
	EXPECT_EQ(size_a, 3);
	EXPECT_EQ(size_b, 0);

	// final matching vector
	data_b = { 11,12,13 };

	size_b = 3;

	EXPECT_TRUE(comp.CompareVecs<int32_t>(data_a, size_a, data_b, size_b));
	EXPECT_EQ(size_a, 0);
	EXPECT_EQ(size_b, 0);
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareVecsMatchingLargerDataB)
{
	std::vector<int32_t> data_a = 
	{ 1,2,3,4,5,6,7,8,9,10 };
	std::vector<int32_t> data_b = 
	{ 1,2,3,4,5,6,7,8,9,10,11,12,13 };

	int size_a = 10;
	int size_b = 13;

	Comparator comp;

	EXPECT_TRUE(comp.CompareVecs<int32_t>(data_a, size_a, data_b, size_b));
	EXPECT_EQ(size_a, 0);
	EXPECT_EQ(size_b, 3);

	// Final matching vector
	data_a = { 11,12,13 };

	size_a = 3;

	EXPECT_TRUE(comp.CompareVecs<int32_t>(data_a, size_a, data_b, size_b));
	EXPECT_EQ(size_a, 0);
	EXPECT_EQ(size_b, 0);
}


TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnOutOfBoundsLow)
{
	std::vector<std::vector<int32_t>> data_a = 
	{ {1,1,1,1,1}, {2,2,2,2,2} };
	std::vector<std::vector<int32_t>> data_b = 
	{ {3,3,3,3,3}, {4,4,4,4,4} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, data_a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, data_a, 2));

	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, data_b, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, data_b, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	ASSERT_FALSE(comp.CompareColumn(0));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnOutOfBoundsHighFileOne)
{
	std::vector<std::vector<int32_t>> data_a = 
	{ {1,1,1,1,1}, {2,2,2,2,2} };
	std::vector<std::vector<int32_t>> data_b = 
	{ {1,1,1,1,1}, {2,2,2,2,2}, {5,5,5,5,5} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, data_a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, data_a, 2));

	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, data_b, 2));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, data_b, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	ASSERT_FALSE(comp.CompareColumn(3));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnOutOfBoundsHighFileTwo)
{
	std::vector<std::vector<int32_t>> data_a = 
	{ {1,1,1,1,1}, {2,2,2,2,2}, {5,5,5,5,5} };
	std::vector<std::vector<int32_t>> data_b = 
	{ {1,1,1,1,1}, {2,2,2,2,2} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, data_a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, data_a, 2));

	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, data_b, 2));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, data_b, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	ASSERT_FALSE(comp.CompareColumn(3));
}


TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnTwoEquivalentColumns)
{
	//												column1		column2
	std::vector<std::vector<int32_t>> data_a = 
	{ {1,1,1,1,1}, {2,2,2,2,2} };
	std::vector<std::vector<int32_t>> data_b =
	{ {3,3,3,3,3}, {4,4,4,4,4} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, data_a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, data_b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, data_a, 2));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, data_b, 1));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnTwoEquivalentColumnsDifferentFileSizes)
{
	//												column1		column2
	std::vector<std::vector<int32_t>> file1a = { {1,1,1,1,1}, {2,2,2,2,2} };
	std::vector<std::vector<int32_t>> file1b = { {5,5}, {6,6} };
	std::vector<std::vector<int32_t>> file2 = { {1,1,1,1,1,5,5}, {2,2,2,2,2,6,6} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnTwoNonEquivalentColumnsDifferentFileSizes)
{
	//												column1		column2
	std::vector<std::vector<int32_t>> file1a = { {1,1,1,1,1}, {2,2,2,2,2} };
	std::vector<std::vector<int32_t>> file1b = { {4,5}, {6,6} };
	std::vector<std::vector<int32_t>> file2 = { {1,1,1,1,1,5,5}, {2,2,2,2,2,6,7} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareColumn(1));
	EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnTwoNonEquivalentColumns1)
{
	//		file1									column1		column2
	std::vector<std::vector<int32_t>> file1a = { {1,1,1,1,1}, {2,2,2,2,2} };
	std::vector<std::vector<int32_t>> file1b = { {1,1,1,1,1}, {2,2,2,2,2} };
	//		file2
	std::vector<std::vector<int32_t>> file2a = { {1,1,1,1,1}, {2,2,2,2,2} };
	std::vector<std::vector<int32_t>> file2b = { {1,1,2,1,1}, {2,2,1,2,2} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2a, 2));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2b, 1));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareColumn(1));
	EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnTwoNonEquivalentColumns2)
{
	//		file1									column1		column2
	std::vector<std::vector<int32_t>> file1a = { {1,1,2,1,1}, {2,2,1,2,2} };
	std::vector<std::vector<int32_t>> file1b = { {1,1,1,1,1}, {2,2,2,2,2} };
	//		file2
	std::vector<std::vector<int32_t>> file2a = { {1,1,1,1,1}, {2,2,2,2,2} };
	std::vector<std::vector<int32_t>> file2b = { {1,1,1,1,1}, {2,2,2,2,2} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";

	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2a, 2));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2b, 1));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareColumn(1));
	EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnOneMatchOneNonMatch)
{
	//		file1									column1		column2
	std::vector<std::vector<int32_t>> file1a = { {1,1,1,1,1}, {2,2,1,2,2} };
	std::vector<std::vector<int32_t>> file1b = { {1,1,1,1,1}, {2,2,2,2,2} };
	//		file2
	std::vector<std::vector<int32_t>> file2a = { {1,1,1,1,1}, {2,2,2,2,2} };
	std::vector<std::vector<int32_t>> file2b = { {1,1,1,1,1}, {2,2,2,2,2} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";

	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2a, 2));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2b, 1));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareFileOneBigger1)
{
	//		file1									column1		column2
	std::vector<std::vector<int32_t>> file1a = { {1,1,1,1,1,1,1}, {2,2,2,2,2,2,2} };
	std::vector<std::vector<int32_t>> file1b = { {1,1,1,1,1}, {2,2,2,2,2} };
	//		file2
	std::vector<std::vector<int32_t>> file2a = { {1,1,1,1,1}, {2,2,2,2,2} };
	std::vector<std::vector<int32_t>> file2b = { {1,1,1,1,1}, {2,2,2,2,2} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";

	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2a, 2));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2b, 1));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareColumn(1));
	EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareFileOneBigger2)
{
	//		file1									column1		column2
	std::vector<std::vector<int32_t>> file1a = { {1,1,1,1,1}, {2,2,2,2,2} };
	std::vector<std::vector<int32_t>> file1b = { {1,1,1,1,1,1,1}, {2,2,2,2,2,2,2} };
	//		file2
	std::vector<std::vector<int32_t>> file2a = { {1,1,1,1,1}, {2,2,2,2,2} };
	std::vector<std::vector<int32_t>> file2b = { {1,1,1,1,1}, {2,2,2,2,2} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";

	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2a, 2));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2b, 1));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareColumn(1));
	EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareFileTwoBigger)
{
	//		file1									column1		column2
	std::vector<std::vector<int32_t>> file1a = { {1,1,1,1,1}, {2,2,2,2,2} };
	std::vector<std::vector<int32_t>> file1b = { {1,1,1,1,1}, {2,2,2,2,2} };
	//		file2
	std::vector<std::vector<int32_t>> file2a = { {1,1,1,1,1}, {2,2,2,2,2} };
	std::vector<std::vector<int32_t>> file2b = { {1,1,1,1,1,1,1}, {2,2,2,2,2,2,2} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";

	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2a, 2));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2b, 1));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareColumn(1));
	EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareTwoDifferentDataTypes)
{
	//		file1									column1		
	std::vector<std::vector<int32_t>> file1a = { {1,1,1,1,1} };
	std::vector<std::vector<int32_t>> file1b = { {2,2,2,2,2} };
	//		file2
	std::vector<std::vector<int64_t>> file2a = { {1,1,1,1,1} };
	std::vector<std::vector<int64_t>> file2b = { {2,2,2,2,2} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";

	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file2a, 2));
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file2b, 1));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareColumn(1));
}

TEST_F(ParquetArrowValidatorTest, ParquetManagerCompareListMatchUint16)
{
	int size;
	
	// 10 values
	std::vector<uint16_t> data1 = 
	{ 1,5,1,9,1, 2,5,2,8,2 };
	
	// 35 values
	std::vector<uint16_t> data2 = 
	{ 3,3,3,3,3, 4,4,4,4,4, 5,5,5,5,5, 6,6,6,6,6, 7,7,7,7,7 };

	std::string dirname1 = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(dirname1, data1, 3, 5));
	/* ASSERT_TRUE(CreateParquetFile(dirname1, data2, 2, 5));

	std::string dirname2 = "file2.parquet";
	ASSERT_TRUE(CreateParquetFile(dirname2, data1, 3, 5));
	ASSERT_TRUE(CreateParquetFile(dirname2, data2, 2, 5));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_EQ(comp.GetComparedCount(1), 35); */
}

TEST_F(ParquetArrowValidatorTest, ParquetManagerCompareListMisMatchUint16)
{
	int size;
	std::vector<uint16_t> data1 = 
	{ 1,5,1,9,1, 2,5,2,8,2 };
	std::vector<uint16_t> data2 = 
	{ 3,3,3,3,3, 4,4,4,4,4, 5,5,5,5,5, 6,6,6,6,6, 7,7,7,7,7 };
	std::vector<uint16_t> data2b = 
	{ 3,3,3,3,3, 4,4,4,4,4, 5,5,5,5,5, 6,6,6,6,6, 7,7,7,7,8 };

	std::string dirname1 = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(dirname1, data1, 3, 5));
	ASSERT_TRUE(CreateParquetFile(dirname1, data2, 2, 5));

	std::string dirname2 = "file2.parquet";
	ASSERT_TRUE(CreateParquetFile(dirname2, data1, 3, 5));
	ASSERT_TRUE(CreateParquetFile(dirname2, data2b, 2, 5));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareColumn(1));
}

TEST_F(ParquetArrowValidatorTest, ParquetManagerCompareListTypeWithNonListType)
{
	int size;
	std::vector<uint16_t> data1 = { 1,5,1,9,1};

	std::vector<std::vector<int64_t>> data2 = { {1,5,1,9,1} };

	std::string dirname1 = "file1.parquet";
	ASSERT_TRUE(CreateParquetFile(dirname1, data1, 3, 5));

	std::string dirname2 = "file2.parquet";
	ASSERT_TRUE(CreateParquetFile(arrow::int64(),dirname2, data2, 3));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareColumn(1));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareInt64Match)
{
	//												column1					column2
	std::vector<std::vector<int64_t>> file1a = {	{16,5,4,9,8},			{7,8,20,50,60} };
	std::vector<std::vector<int64_t>> file1b = {	{20,1000},				{51,5000} };
	std::vector<std::vector<int64_t>> file2 =  {	{16,5,4,9,8,20,1000},	{7,8,20,50,60,51,5000} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareInt64MisMatch)
{
	//												column1					column2
	std::vector<std::vector<int64_t>> file1a = { {16,5,4,9,8},			{7,8,20,50,60} };
	std::vector<std::vector<int64_t>> file1b = { {21,1000},				{51,5000} };
	std::vector<std::vector<int64_t>> file2 = { {16,5,4,9,8,20,1000},	{7,8,20,50,60,52,5000} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareColumn(1));
	EXPECT_FALSE(comp.CompareColumn(2));
}

// int 32 tested extensively above

TEST_F(ParquetArrowValidatorTest, ComparatorCompareInt16Match)
{
	//												column1					column2
	std::vector<std::vector<int16_t>> file1a = { {16,5,4,-9,8},			{7,8,20,50,60} };
	std::vector<std::vector<int16_t>> file1b = { {20,1000},				{51,-5000} };
	std::vector<std::vector<int16_t>> file2 = { {16,5,4,-9,8,20,1000},	{7,8,20,50,60,51,-5000} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareInt16MisMatch)
{
	//												column1					column2
	std::vector<std::vector<int16_t>> file1a = { {16,-5,4,9,8},			{7,8,20,50,-60} };
	std::vector<std::vector<int16_t>> file1b = { {21,1000},				{51,5000} };
	std::vector<std::vector<int16_t>> file2 = { {16,-5,4,9,8,20,1000},	{7,8,20,50,-60,52,5000} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareColumn(1));
	EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareInt8Match)
{
	//												column1				column2
	std::vector<std::vector<int8_t>> file1a = { {16,5,4,9,8},		{7,8,20,50,60} };
	std::vector<std::vector<int8_t>> file1b = { {20,100},			{51,50} };
	std::vector<std::vector<int8_t>> file2 = { {16,5,4,9,8,20,100},	{7,8,20,50,60,51,50} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int8(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int8(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int8(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareInt8MisMatch)
{
	//												column1				column2
	std::vector<std::vector<int8_t>> file1a = { {16,5,4,9,8},		{7,8,20,50,60} };
	std::vector<std::vector<int8_t>> file1b = { {21,100},			{51,50} };
	std::vector<std::vector<int8_t>> file2 = { {16,5,4,9,8,20,100},	{7,8,20,50,60,52,50} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int8(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int8(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int8(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareColumn(1));
	EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareBooleanMatch)
{
	//												column1			column2
	std::vector<std::vector<uint8_t>> file1a = { {0,1,0,0,1},	{0,0,0,1,0} };
	std::vector<std::vector<uint8_t>> file1b = { {1,1},			{1,0} };
	std::vector<std::vector<uint8_t>> file2 = { {0,1,0,0,1,1,1},{0,0,0,1,0,1,0} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareBooleanMismatch)
{
	//												column1			column2
	std::vector<std::vector<uint8_t>> file1a = { {0,1,0,0,1},	{0,0,0,1,0} };
	std::vector<std::vector<uint8_t>> file1b = { {1,1},			{0,0} };
	std::vector<std::vector<uint8_t>> file2 = { {0,1,0,1,1,1,1},{0,0,0,1,0,1,0} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareColumn(1));
	EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareStringMatch)
{
	//												column1								column2
	std::vector<std::vector<std::string>> file1a = { {"a","d","e","f","b"},			{"c","g","i","p","b"} };
	std::vector<std::vector<std::string>> file1b = { {"f","g"},						{"t","l"} };
	std::vector<std::vector<std::string>> file2 = { {"a","d","e","f","b","f","g"},	{"c","g","i","p","b","t","l"} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareStringMisMatch)
{
	//												column1							column2
	std::vector<std::vector<std::string>> file1a = { {"a","d","e","f","b"},			{"c","g","i","p","b"} };
	std::vector<std::vector<std::string>> file1b = { {"f","g"},						{"t","k"} };
	std::vector<std::vector<std::string>> file2 = { {"a","p","e","f","b","f","g"},	{"c","g","i","p","b","t","l"} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareColumn(1));
	EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareDoubleMatch)
{
	//												column1				column2
	std::vector<std::vector<double>> file1a = { {16.54,5.34,4.24,
													9.14,8.11},		{7.1,8.2,20.3,50.4,60.5} };
	std::vector<std::vector<double>> file1b = { {-20.2,100.1},			{51.6,50.7} };

	std::vector<std::vector<double>> file2 = { {16.54,5.34,4.24,
												9.14,8.11,
												-20.2,100.1},		{7.1,8.2,20.3,50.4,60.5,51.6,50.7} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareDoubleMisMatch)
{
	//												column1				column2
	std::vector<std::vector<double>> file1a = { {16.54,5.34,4.24,
													9.14,8.11},		{7.1,8.2,20.3,50.4,60.5} };
	std::vector<std::vector<double>> file1b = { {-20.2,11100.1},			{51.6,50.7} };

	std::vector<std::vector<double>> file2 = { {16.54,5.34,4.24,
												9.14,8.11,
												-20.2,100.1},		{7.1,8.2,20.3,50.4,60.5,51.6,50.8} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareColumn(1));
	EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareDoubleNaN)
{
	//												column1				
	std::vector<std::vector<double>> file1a = { {16.54, 5.34, 4.24, 9.14, std::numeric_limits<double>::quiet_NaN() } };
	std::vector<std::vector<double>> file1b = { {-20.2, std::numeric_limits<double>::quiet_NaN(), 100.1} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname2, file1a, 2));
	ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname2, file1b, 1));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_TRUE(comp.CompareColumn(1));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareFloatMatch)
{
	//												column1				column2
	std::vector<std::vector<float>> file1a = { {16.54,5.34,4.24,
													9.14,8.11},		{7.1,8.2,20.3,50.4,60.5} };
	std::vector<std::vector<float>> file1b = { {-20.2,100.1},			{51.6,50.7} };

	std::vector<std::vector<float>> file2 = { {16.54,5.34,4.24,
												9.14,8.11,
												-20.2,100.1},		{7.1,8.2,20.3,50.4,60.5,51.6,50.7} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareFloatMisMatch)
{
	//												column1				column2
	std::vector<std::vector<float>> file1a = { {16.54,5.34,4.24,
													9.14,8.11},		{7.1,8.2,20.3,50.4,60.5} };
	std::vector<std::vector<float>> file1b = { {-20.2,11100.1},			{51.6,50.7} };

	std::vector<std::vector<float>> file2 = { {16.54,5.34,4.24,
												9.14,8.11,
												-20.2,100.1},		{7.1,8.2,20.3,50.4,60.5,51.6,50.8} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareColumn(1));
	EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareFloatNaN)
{
	//												column1				
	std::vector<std::vector<float>> file1a = { {16.54, 5.34, 4.24, 9.14, std::numeric_limits<float>::quiet_NaN() } };
	std::vector<std::vector<float>> file1b = { {-20.2, std::numeric_limits<float>::quiet_NaN(), 100.1} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname2, file1a, 2));
	ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname2, file1b, 1));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_TRUE(comp.CompareColumn(1));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareTotalCountInts)
{
	//												column1					column2
	std::vector<std::vector<int64_t>> file1a = { {16,5,4,9,8},			{7,8,20,50,60} };
	std::vector<std::vector<int64_t>> file1b = { {20,1000},				{50,5000} };
	std::vector<std::vector<int64_t>> file2 = { {16,5,4,9,8,20,1000},	{7,8,20,50,60,50,5000} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_EQ(comp.GetComparedCount(1), 0);
	EXPECT_EQ(comp.GetComparedCount(2), 0);
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_EQ(comp.GetComparedCount(1), 7);
	EXPECT_EQ(comp.GetComparedCount(2), 0);
	EXPECT_TRUE(comp.CompareColumn(2));
	EXPECT_EQ(comp.GetComparedCount(1), 7);
	EXPECT_EQ(comp.GetComparedCount(2), 7);

	//		reset counter							column1					column2
	std::vector<std::vector<int64_t>> file1aa = { {16,5,4,9,8},			{7,8,20,50,60} };
	std::vector<std::vector<int64_t>> file1bb = { {20},				{50} };
	std::vector<std::vector<int64_t>> file2a = { {16,5,4,9,8,20},	{7,8,20,50,60,50} };

	dirname1 = "file3.parquet";
	dirname2 = "file4.parquet";

	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1aa, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1bb, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file2a, 2));

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_EQ(comp.GetComparedCount(1), 0);
	EXPECT_EQ(comp.GetComparedCount(2), 0);
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_EQ(comp.GetComparedCount(1), 6);
	EXPECT_EQ(comp.GetComparedCount(2), 0);
	EXPECT_TRUE(comp.CompareColumn(2));
	EXPECT_EQ(comp.GetComparedCount(1), 6);
	EXPECT_EQ(comp.GetComparedCount(2), 6);

	// compare same column twice
	EXPECT_TRUE(comp.CompareColumn(2));
	EXPECT_EQ(comp.GetComparedCount(1), 6);
	EXPECT_EQ(comp.GetComparedCount(2), 6);
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareTotalCountBool)
{
	//												column1			column2
	std::vector<std::vector<uint8_t>> file1a = { {0,1,0,0,1},	{0,0,0,1,0} };
	std::vector<std::vector<uint8_t>> file1b = { {1,1},			{1,0} };
	std::vector<std::vector<uint8_t>> file2 = { {0,1,0,0,1,1,1},{0,0,0,1,0,1,0} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_EQ(comp.GetComparedCount(1), 0);
	EXPECT_EQ(comp.GetComparedCount(2), 0);
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_EQ(comp.GetComparedCount(1), 7);
	EXPECT_EQ(comp.GetComparedCount(2), 0);
	EXPECT_TRUE(comp.CompareColumn(2));
	EXPECT_EQ(comp.GetComparedCount(1), 7);
	EXPECT_EQ(comp.GetComparedCount(2), 7);

	//resets counter
	//												column1			column2
	std::vector<std::vector<uint8_t>> file1aa = { {0,1,0,0,1},	{0,0,0,1,0} };
	std::vector<std::vector<uint8_t>> file1bb = { {1},			{1} };
	std::vector<std::vector<uint8_t>> file2a = { {0,1,0,0,1,1},	{0,0,0,1,0,1} };


	dirname1 = "file3.parquet";
	dirname2 = "file4.parquet";

	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file1aa, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file1bb, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname2, file2a, 2));

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_EQ(comp.GetComparedCount(1), 0);
	EXPECT_EQ(comp.GetComparedCount(2), 0);
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_EQ(comp.GetComparedCount(1), 6);
	EXPECT_EQ(comp.GetComparedCount(2), 0);
	EXPECT_TRUE(comp.CompareColumn(2));
	EXPECT_EQ(comp.GetComparedCount(1), 6);
	EXPECT_EQ(comp.GetComparedCount(2), 6);

	// compare same column twice
	EXPECT_TRUE(comp.CompareColumn(2));
	EXPECT_EQ(comp.GetComparedCount(1), 6);
	EXPECT_EQ(comp.GetComparedCount(2), 6);
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareTotalCountStrings)
{
	//												column1							column2
	std::vector<std::vector<std::string>> file1a = { {"a","d","e","f","b"},			{"c","g","i","p","b"} };
	std::vector<std::vector<std::string>> file1b = { {"f","g"},						{"t","k"} };
	std::vector<std::vector<std::string>> file2 = { {"a","d","e","f","b","f","g"},	{"c","g","i","p","b","t","k"} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_EQ(comp.GetComparedCount(1), 0);
	EXPECT_EQ(comp.GetComparedCount(2), 0);
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_EQ(comp.GetComparedCount(1), 7);
	EXPECT_EQ(comp.GetComparedCount(2), 0);
	EXPECT_TRUE(comp.CompareColumn(2));
	EXPECT_EQ(comp.GetComparedCount(1), 7);
	EXPECT_EQ(comp.GetComparedCount(2), 7);

	//resets counter									column1							column2
	std::vector<std::vector<std::string>> file1aa = { {"a","d","e","f","b"},			{"c","g","i","p","b"} };
	std::vector<std::vector<std::string>> file1bb = { {"f"},							{"t"} };
	std::vector<std::vector<std::string>> file2a = { {"a","d","e","f","b","f"},		{"c","g","i","p","b","t"} };

	dirname1 = "file3.parquet";
	dirname2 = "file4.parquet";

	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file1aa, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file1bb, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname2, file2a, 2));

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_EQ(comp.GetComparedCount(1), 0);
	EXPECT_EQ(comp.GetComparedCount(2), 0);
	EXPECT_TRUE(comp.CompareColumn(1));
	EXPECT_EQ(comp.GetComparedCount(1), 6);
	EXPECT_EQ(comp.GetComparedCount(2), 0);
	EXPECT_TRUE(comp.CompareColumn(2));
	EXPECT_EQ(comp.GetComparedCount(1), 6);
	EXPECT_EQ(comp.GetComparedCount(2), 6);

	// compare same column twice
	EXPECT_TRUE(comp.CompareColumn(2));
	EXPECT_EQ(comp.GetComparedCount(1), 6);
	EXPECT_EQ(comp.GetComparedCount(2), 6);
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareAllTotalCountInts)
{
	//												column1					column2			column3
	std::vector<std::vector<int64_t>> file1a = { {16,5,4,9,8},			{7,8,20,50,60},		{8,3,7,1,5} };
	std::vector<std::vector<int64_t>> file1b = { {20,1000},				{50,51000},			{5,3 } };

	// file 2 missing column 3
	std::vector<std::vector<int64_t>> file2 = { {16,5,4,9,8,20,1000},	{7,8,20,50,60,50,5000} };

	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1a, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1b, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file2, 2));

	Comparator comp;

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_FALSE(comp.CompareAll());
	EXPECT_EQ(comp.GetComparedCount(1), 7);
	EXPECT_EQ(comp.GetComparedCount(2), 7);
	EXPECT_EQ(comp.GetComparedCount(3), 0);
	EXPECT_TRUE(comp.CheckPassed(1));
	EXPECT_FALSE(comp.CheckPassed(2));
	EXPECT_FALSE(comp.CheckPassed(3));

	//resets stats and should pass					column1					column2
	std::vector<std::vector<int64_t>> file1aa = { {16,5,4,9,8},			{7,8,20,50,60} };
	std::vector<std::vector<int64_t>> file1bb = { {20},				{50} };
	std::vector<std::vector<int64_t>> file2a = { {16,5,4,9,8,20},	{7,8,20,50,60,50} };

	dirname1 = "file3.parquet";
	dirname2 = "file4.parquet";

	// file 1
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1aa, 3));
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1bb, 2));

	// file 2
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file2a, 2));

	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));
	EXPECT_EQ(comp.GetComparedCount(1), 0);
	EXPECT_EQ(comp.GetComparedCount(2), 0);
	EXPECT_EQ(comp.GetComparedCount(3), 0);
	EXPECT_FALSE(comp.CheckPassed(1));
	EXPECT_FALSE(comp.CheckPassed(2));
	EXPECT_FALSE(comp.CheckPassed(3));

	EXPECT_TRUE(comp.CompareAll());
	EXPECT_TRUE(comp.CheckPassed(1));
	EXPECT_TRUE(comp.CheckPassed(2));
	EXPECT_FALSE(comp.CheckPassed(3));
	EXPECT_EQ(comp.GetComparedCount(1), 6);
	EXPECT_EQ(comp.GetComparedCount(2), 6);
	EXPECT_EQ(comp.GetComparedCount(3), 0);
}

TEST_F(ParquetArrowValidatorTest, BoolNullMatch)
{
	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	std::vector<std::vector<uint8_t>> file = { { 1,1,1,1,1,1, 1,1,1,1,1,1, 1,1 } };
	std::vector<uint8_t> bool_fields = {         0,1,1,1,0,0, 0,0,1,1,0,0, 1,1 };

	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file, 6, &bool_fields));
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname2, file, 6, &bool_fields));

	Comparator comp;
	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));

	ASSERT_TRUE(comp.CompareAll());
}

TEST_F(ParquetArrowValidatorTest, Int64NullMatch)
{
	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	std::vector<std::vector<int64_t>> file = { { 1,2,1,3,1,4, 5,6,8,9,1,2, 9,8 } };
	std::vector<uint8_t> bool_fields = {         0,1,1,1,0,0, 0,0,1,1,0,0, 1,1 };

	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file, 6, &bool_fields));
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file, 6, &bool_fields));

	Comparator comp;
	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));

	ASSERT_TRUE(comp.CompareAll());
}

TEST_F(ParquetArrowValidatorTest, FloatNullMatch)
{
	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	std::vector<std::vector<float>> file = { { 1.5,2,1,3,1,4, 5.5,6,-8.66,9,1,2, 9,8 } };
	std::vector<uint8_t> bool_fields = {       0,  1,1,1,0,0, 0,  0, 1,   1,0,0, 1,1 };

	ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file, 6, &bool_fields));
	ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname2, file, 6, &bool_fields));

	Comparator comp;
	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));

	ASSERT_TRUE(comp.CompareAll());
}

TEST_F(ParquetArrowValidatorTest, StringNullMatch)
{
	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	std::vector<std::vector<std::string>> file = { { "a","b","c","d","ee","ff","gg","h","i","jj","k" } };
	std::vector<uint8_t> bool_fields = {              1,  0,  1,  0,  1,   1,   0,   1,  0,  1,   1 };

	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file, 6, &bool_fields));
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname2, file, 6, &bool_fields));

	Comparator comp;
	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));

	ASSERT_TRUE(comp.CompareAll());
}

TEST_F(ParquetArrowValidatorTest, BoolNullMisMatch)
{
	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	std::vector<std::vector<uint8_t>> file = { { 1,1,1,1,1,1, 1,1,1,1,1,1, 1,1 } };
	std::vector<uint8_t> bool_fields1 = {         0,1,1,1,0,0, 0,0,1,1,0,0, 1,1 };
	std::vector<uint8_t> bool_fields2 = {         0,1,1,1,0,0, 0,0,1,1,0,0, 1,0 };

	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file, 6, &bool_fields1));
	ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname2, file, 6, &bool_fields2));

	Comparator comp;
	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));

	ASSERT_FALSE(comp.CompareAll());
}

TEST_F(ParquetArrowValidatorTest, Int64NullMisMatch)
{
	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	std::vector<std::vector<int64_t>> file = { { 1,2,1,3,1,4, 5,6,8,9,1,2, 9,8 } };
	std::vector<uint8_t> bool_fields1 = {         0,1,1,1,0,0, 0,0,1,1,0,0, 1,1 };
	std::vector<uint8_t> bool_fields2 = {         0,1,1,1,0,0, 0,0,1,1,0,0, 1,0 };

	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file, 6, &bool_fields1));
	ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file, 6, &bool_fields2));

	Comparator comp;
	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));

	ASSERT_FALSE(comp.CompareAll());
}

TEST_F(ParquetArrowValidatorTest, FloatNullMisMatch)
{
	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	std::vector<std::vector<float>> file = { { 1.5,2,1,3,1,4, 5.5,6,-8.66,9,1,2, 9,8 } };
	std::vector<uint8_t> bool_fields1 = {       0, 1,1,1,0,0, 0,  0, 1,   1,0,0, 1,1 };
	std::vector<uint8_t> bool_fields2 = {       0, 1,1,1,0,0, 0,  0, 1,   1,0,0, 1,0 };

	ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file, 6, &bool_fields1));
	ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname2, file, 6, &bool_fields2));

	Comparator comp;
	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));

	ASSERT_FALSE(comp.CompareAll());
}

TEST_F(ParquetArrowValidatorTest, StringNullMisMatch)
{
	std::string dirname1 = "file1.parquet";
	std::string dirname2 = "file2.parquet";
	std::vector<std::vector<std::string>> file = { { "a","b","c","d","ee","ff","gg","h","i","jj","k" } };
	std::vector<uint8_t> bool_fields1 = {              1,  0,  1,  0,  1,   1,   0,   1,  0,  1,   1 };
	std::vector<uint8_t> bool_fields2 = {              1,  0,  1,  0,  1,   1,   0,   1,  0,  1,   0 };

	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file, 6, &bool_fields1));
	ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname2, file, 6, &bool_fields2));

	Comparator comp;
	ASSERT_TRUE(comp.Initialize(dirname1, dirname2));

	ASSERT_FALSE(comp.CompareAll());
}