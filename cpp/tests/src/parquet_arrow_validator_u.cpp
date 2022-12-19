#include <vector>
#include <string>
#include <filesystem>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parquet_context.h"
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
        int row_size = static_cast<int>(output[0].size());

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
        int row_size = static_cast<int>(output1.size());

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
        int row_count = static_cast<int>(output.size() / list_size);
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

TEST_F(ParquetArrowValidatorTest, ComparatorInitializeNoFilesExist)
{
    Comparator comp;
    std::string file_name1 = "file1.parquet";
    std::string file_name2 = "file2.parquet";
    EXPECT_EQ(EX_NOINPUT, comp.Initialize(ManagedPath(file_name1), ManagedPath(file_name2)));
}

TEST_F(ParquetArrowValidatorTest, ComparatorInitializeNoFilesExist2)
{
    Comparator comp;
    std::string file1 = "file1.parquet";
    std::string file2 = "file2.parquet";

    std::filesystem::create_directory(file1);

    EXPECT_EQ(EX_NOINPUT, comp.Initialize(ManagedPath(file1), ManagedPath(file2)));
    std::filesystem::remove_all(file1);
}

TEST_F(ParquetArrowValidatorTest, ComparatorInitializeInvalidFiles)
{
    int size;
    std::vector<uint16_t> data1 = {1, 1, 1, 1, 1, 2, 2, 2, 2, 2};

    std::string dirname = "file1.parquet";
    ASSERT_TRUE(CreateParquetFile(dirname, data1, 1, 5));

    Comparator comp;

    std::string invalid = "invalid.parquet";

    std::filesystem::create_directory(invalid);

    EXPECT_EQ(EX_IOERR, comp.Initialize(invalid, dirname));
    EXPECT_EQ(EX_IOERR, comp.Initialize(dirname, invalid));

    std::filesystem::remove_all(invalid);
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareVecsMatchingSameSize)
{
    std::vector<int32_t> data_a =
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int32_t> data_b =
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

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
        {1, 2, 3, 4, 5, 0, 7, 8, 9, 10};
    std::vector<int32_t> data_b =
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

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
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    std::vector<int32_t> data_b =
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    int size_a = 13;
    int size_b = 10;

    Comparator comp;

    EXPECT_TRUE(comp.CompareVecs<int32_t>(data_a, size_a, data_b, size_b));
    EXPECT_EQ(size_a, 3);
    EXPECT_EQ(size_b, 0);

    // final matching vector
    data_b = {11, 12, 13};

    size_b = 3;

    EXPECT_TRUE(comp.CompareVecs<int32_t>(data_a, size_a, data_b, size_b));
    EXPECT_EQ(size_a, 0);
    EXPECT_EQ(size_b, 0);
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareVecsMatchingLargerDataB)
{
    std::vector<int32_t> data_a =
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int32_t> data_b =
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

    int size_a = 10;
    int size_b = 13;

    Comparator comp;

    EXPECT_TRUE(comp.CompareVecs<int32_t>(data_a, size_a, data_b, size_b));
    EXPECT_EQ(size_a, 0);
    EXPECT_EQ(size_b, 3);

    // Final matching vector
    data_a = {11, 12, 13};

    size_a = 3;

    EXPECT_TRUE(comp.CompareVecs<int32_t>(data_a, size_a, data_b, size_b));
    EXPECT_EQ(size_a, 0);
    EXPECT_EQ(size_b, 0);
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnOutOfBoundsLow)
{
    std::vector<std::vector<int32_t>> data_a =
        {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    std::vector<std::vector<int32_t>> data_b =
        {{3, 3, 3, 3, 3}, {4, 4, 4, 4, 4}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, data_a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, data_a, 2));

    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, data_b, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, data_b, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    ASSERT_FALSE(comp.CompareColumn(0));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnOutOfBoundsHighFileOne)
{
    std::vector<std::vector<int32_t>> data_a =
        {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    std::vector<std::vector<int32_t>> data_b =
        {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}, {5, 5, 5, 5, 5}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, data_a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, data_a, 2));

    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, data_b, 2));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, data_b, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    ASSERT_FALSE(comp.CompareColumn(3));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnOutOfBoundsHighFileTwo)
{
    std::vector<std::vector<int32_t>> data_a =
        {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}, {5, 5, 5, 5, 5}};
    std::vector<std::vector<int32_t>> data_b =
        {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, data_a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, data_a, 2));

    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, data_b, 2));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, data_b, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    ASSERT_FALSE(comp.CompareColumn(3));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnTwoEquivalentColumns)
{
    //												column1		column2
    std::vector<std::vector<int32_t>> data_a =
        {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    std::vector<std::vector<int32_t>> data_b =
        {{3, 3, 3, 3, 3}, {4, 4, 4, 4, 4}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, data_a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, data_b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, data_a, 2));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, data_b, 1));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_TRUE(comp.CompareColumn(1));
    EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnTwoEquivalentColumnsDifferentFileSizes)
{
    //												column1		column2
    std::vector<std::vector<int32_t>> file1a = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    std::vector<std::vector<int32_t>> file1b = {{5, 5}, {6, 6}};
    std::vector<std::vector<int32_t>> file2 = {{1, 1, 1, 1, 1, 5, 5}, {2, 2, 2, 2, 2, 6, 6}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_TRUE(comp.CompareColumn(1));
    EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnTwoNonEquivalentColumnsDifferentFileSizes)
{
    //												column1		column2
    std::vector<std::vector<int32_t>> file1a = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    std::vector<std::vector<int32_t>> file1b = {{4, 5}, {6, 6}};
    std::vector<std::vector<int32_t>> file2 = {{1, 1, 1, 1, 1, 5, 5}, {2, 2, 2, 2, 2, 6, 7}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
    EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnTwoNonEquivalentColumns1)
{
    //		file1									column1		column2
    std::vector<std::vector<int32_t>> file1a = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    std::vector<std::vector<int32_t>> file1b = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    //		file2
    std::vector<std::vector<int32_t>> file2a = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    std::vector<std::vector<int32_t>> file2b = {{1, 1, 2, 1, 1}, {2, 2, 1, 2, 2}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";

    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2a, 2));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2b, 1));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
    EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnTwoNonEquivalentColumns2)
{
    //		file1									column1		column2
    std::vector<std::vector<int32_t>> file1a = {{1, 1, 2, 1, 1}, {2, 2, 1, 2, 2}};
    std::vector<std::vector<int32_t>> file1b = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    //		file2
    std::vector<std::vector<int32_t>> file2a = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    std::vector<std::vector<int32_t>> file2b = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";

    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2a, 2));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2b, 1));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
    EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareColumnOneMatchOneNonMatch)
{
    //		file1									column1		column2
    std::vector<std::vector<int32_t>> file1a = {{1, 1, 1, 1, 1}, {2, 2, 1, 2, 2}};
    std::vector<std::vector<int32_t>> file1b = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    //		file2
    std::vector<std::vector<int32_t>> file2a = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    std::vector<std::vector<int32_t>> file2b = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";

    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2a, 2));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2b, 1));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_TRUE(comp.CompareColumn(1));
    EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareFileOneBigger1)
{
    //		file1									column1		column2
    std::vector<std::vector<int32_t>> file1a = {{1, 1, 1, 1, 1, 1, 1}, {2, 2, 2, 2, 2, 2, 2}};
    std::vector<std::vector<int32_t>> file1b = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    //		file2
    std::vector<std::vector<int32_t>> file2a = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    std::vector<std::vector<int32_t>> file2b = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";

    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2a, 2));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2b, 1));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
    EXPECT_FALSE(comp.CompareColumn(2));

}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareFileOneBigger2)
{
    //		file1									column1		column2
    std::vector<std::vector<int32_t>> file1a = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    std::vector<std::vector<int32_t>> file1b = {{1, 1, 1, 1, 1, 1, 1}, {2, 2, 2, 2, 2, 2, 2}};
    //		file2
    std::vector<std::vector<int32_t>> file2a = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    std::vector<std::vector<int32_t>> file2b = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";

    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2a, 2));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2b, 1));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
    EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareFileTwoBigger)
{
    //		file1									column1		column2
    std::vector<std::vector<int32_t>> file1a = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    std::vector<std::vector<int32_t>> file1b = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    //		file2
    std::vector<std::vector<int32_t>> file2a = {{1, 1, 1, 1, 1}, {2, 2, 2, 2, 2}};
    std::vector<std::vector<int32_t>> file2b = {{1, 1, 1, 1, 1, 1, 1}, {2, 2, 2, 2, 2, 2, 2}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";

    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2a, 2));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname2, file2b, 1));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
    EXPECT_FALSE(comp.CompareColumn(2));

}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareTwoDifferentDataTypes)
{
    //		file1									column1
    std::vector<std::vector<int32_t>> file1a = {{1, 1, 1, 1, 1}};
    std::vector<std::vector<int32_t>> file1b = {{2, 2, 2, 2, 2}};
    //		file2
    std::vector<std::vector<int64_t>> file2a = {{1, 1, 1, 1, 1}};
    std::vector<std::vector<int64_t>> file2b = {{2, 2, 2, 2, 2}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";

    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int32(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file2a, 2));
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file2b, 1));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
}

TEST_F(ParquetArrowValidatorTest, ParquetManagerCompareListMatchUint16)
{
    int size;

    // 10 values
    std::vector<uint16_t> data1 =
        {1, 5, 1, 9, 1, 2, 5, 2, 8, 2};

    // 35 values
    std::vector<uint16_t> data2 =
        {3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7};

    std::string dirname1 = "file1.parquet";
    ASSERT_TRUE(CreateParquetFile(dirname1, data1, 3, 5));
    ASSERT_TRUE(CreateParquetFile(dirname1, data2, 2, 5));

    std::string dirname2 = "file2.parquet";
    ASSERT_TRUE(CreateParquetFile(dirname2, data1, 3, 5));
    ASSERT_TRUE(CreateParquetFile(dirname2, data2, 2, 5));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_TRUE(comp.CompareColumn(1));
    EXPECT_EQ(comp.GetComparedCount(1), 35);
}

TEST_F(ParquetArrowValidatorTest, ParquetManagerCompareListMisMatchUint16)
{
    int size;
    std::vector<uint16_t> data1 =
        {1, 5, 1, 9, 1, 2, 5, 2, 8, 2};
    std::vector<uint16_t> data2 =
        {3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7};
    std::vector<uint16_t> data2b =
        {3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 8};

    std::string dirname1 = "file1.parquet";
    ASSERT_TRUE(CreateParquetFile(dirname1, data1, 3, 5));
    ASSERT_TRUE(CreateParquetFile(dirname1, data2, 2, 5));

    std::string dirname2 = "file2.parquet";
    ASSERT_TRUE(CreateParquetFile(dirname2, data1, 3, 5));
    ASSERT_TRUE(CreateParquetFile(dirname2, data2b, 2, 5));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
}

TEST_F(ParquetArrowValidatorTest, ParquetManagerCompareListTypeWithNonListType)
{
    int size;
    std::vector<uint16_t> data1 = {1, 5, 1, 9, 1};

    std::vector<std::vector<int64_t>> data2 = {{1, 5, 1, 9, 1}};

    std::string dirname1 = "file1.parquet";
    ASSERT_TRUE(CreateParquetFile(dirname1, data1, 3, 5));

    std::string dirname2 = "file2.parquet";
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, data2, 3));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareInt64Match)
{
    //												column1					column2
    std::vector<std::vector<int64_t>> file1a = {{16, 5, 4, 9, 8}, {7, 8, 20, 50, 60}};
    std::vector<std::vector<int64_t>> file1b = {{20, 1000}, {51, 5000}};
    std::vector<std::vector<int64_t>> file2 = {{16, 5, 4, 9, 8, 20, 1000}, {7, 8, 20, 50, 60, 51, 5000}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_TRUE(comp.CompareColumn(1));
    EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareInt64MisMatch)
{
    //												column1					column2
    std::vector<std::vector<int64_t>> file1a = {{16, 5, 4, 9, 8}, {7, 8, 20, 50, 60}};
    std::vector<std::vector<int64_t>> file1b = {{21, 1000}, {51, 5000}};
    std::vector<std::vector<int64_t>> file2 = {{16, 5, 4, 9, 8, 20, 1000}, {7, 8, 20, 50, 60, 52, 5000}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
    EXPECT_FALSE(comp.CompareColumn(2));
}

// int 32 tested extensively above

TEST_F(ParquetArrowValidatorTest, ComparatorCompareInt16Match)
{
    //												column1					column2
    std::vector<std::vector<int16_t>> file1a = {{16, 5, 4, -9, 8}, {7, 8, 20, 50, 60}};
    std::vector<std::vector<int16_t>> file1b = {{20, 1000}, {51, -5000}};
    std::vector<std::vector<int16_t>> file2 = {{16, 5, 4, -9, 8, 20, 1000}, {7, 8, 20, 50, 60, 51, -5000}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_TRUE(comp.CompareColumn(1));
    EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareInt16MisMatch)
{
    //												column1					column2
    std::vector<std::vector<int16_t>> file1a = {{16, -5, 4, 9, 8}, {7, 8, 20, 50, -60}};
    std::vector<std::vector<int16_t>> file1b = {{21, 1000}, {51, 5000}};
    std::vector<std::vector<int16_t>> file2 = {{16, -5, 4, 9, 8, 20, 1000}, {7, 8, 20, 50, -60, 52, 5000}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int16(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
    EXPECT_FALSE(comp.CompareColumn(2));

}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareInt8Match)
{
    //												column1				column2
    std::vector<std::vector<int8_t>> file1a = {{16, 5, 4, 9, 8}, {7, 8, 20, 50, 60}};
    std::vector<std::vector<int8_t>> file1b = {{20, 100}, {51, 50}};
    std::vector<std::vector<int8_t>> file2 = {{16, 5, 4, 9, 8, 20, 100}, {7, 8, 20, 50, 60, 51, 50}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int8(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int8(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int8(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_TRUE(comp.CompareColumn(1));
    EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareInt8MisMatch)
{
    //												column1				column2
    std::vector<std::vector<int8_t>> file1a = {{16, 5, 4, 9, 8}, {7, 8, 20, 50, 60}};
    std::vector<std::vector<int8_t>> file1b = {{21, 100}, {51, 50}};
    std::vector<std::vector<int8_t>> file2 = {{16, 5, 4, 9, 8, 20, 100}, {7, 8, 20, 50, 60, 52, 50}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int8(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int8(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int8(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
    EXPECT_FALSE(comp.CompareColumn(2));

}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareBooleanMatch)
{
    //												column1			column2
    std::vector<std::vector<uint8_t>> file1a = {{0, 1, 0, 0, 1}, {0, 0, 0, 1, 0}};
    std::vector<std::vector<uint8_t>> file1b = {{1, 1}, {1, 0}};
    std::vector<std::vector<uint8_t>> file2 = {{0, 1, 0, 0, 1, 1, 1}, {0, 0, 0, 1, 0, 1, 0}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_TRUE(comp.CompareColumn(1));
    EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareBooleanMismatch)
{
    //												column1			column2
    std::vector<std::vector<uint8_t>> file1a = {{0, 1, 0, 0, 1}, {0, 0, 0, 1, 0}};
    std::vector<std::vector<uint8_t>> file1b = {{1, 1}, {0, 0}};
    std::vector<std::vector<uint8_t>> file2 = {{0, 1, 0, 1, 1, 1, 1}, {0, 0, 0, 1, 0, 1, 0}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
    EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareStringMatch)
{
    //												column1								column2
    std::vector<std::vector<std::string>> file1a = {{"a", "d", "e", "f", "b"}, {"c", "g", "i", "p", "b"}};
    std::vector<std::vector<std::string>> file1b = {{"f", "g"}, {"t", "l"}};
    std::vector<std::vector<std::string>> file2 = {{"a", "d", "e", "f", "b", "f", "g"}, {"c", "g", "i", "p", "b", "t", "l"}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_TRUE(comp.CompareColumn(1));
    EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareStringMisMatch)
{
    //												column1							column2
    std::vector<std::vector<std::string>> file1a = {{"a", "d", "e", "f", "b"}, {"c", "g", "i", "p", "b"}};
    std::vector<std::vector<std::string>> file1b = {{"f", "g"}, {"t", "k"}};
    std::vector<std::vector<std::string>> file2 = {{"a", "p", "e", "f", "b", "f", "g"}, {"c", "g", "i", "p", "b", "t", "l"}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
    EXPECT_FALSE(comp.CompareColumn(2));

}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareDoubleMatch)
{
    //												column1				column2
    std::vector<std::vector<double>> file1a = {{16.54, 5.34, 4.24,
                                                9.14, 8.11},
                                               {7.1, 8.2, 20.3, 50.4, 60.5}};
    std::vector<std::vector<double>> file1b = {{-20.2, 100.1}, {51.6, 50.7}};

    std::vector<std::vector<double>> file2 = {{16.54, 5.34, 4.24,
                                               9.14, 8.11,
                                               -20.2, 100.1},
                                              {7.1, 8.2, 20.3, 50.4, 60.5, 51.6, 50.7}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_TRUE(comp.CompareColumn(1));
    EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareDoubleMisMatch)
{
    //												column1				column2
    std::vector<std::vector<double>> file1a = {{16.54, 5.34, 4.24,
                                                9.14, 8.11},
                                               {7.1, 8.2, 20.3, 50.4, 60.5}};
    std::vector<std::vector<double>> file1b = {{-20.2, 11100.1}, {51.6, 50.7}};

    std::vector<std::vector<double>> file2 = {{16.54, 5.34, 4.24,
                                               9.14, 8.11,
                                               -20.2, 100.1},
                                              {7.1, 8.2, 20.3, 50.4, 60.5, 51.6, 50.8}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
    EXPECT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareDoubleNaN)
{
    //												column1
    std::vector<std::vector<double>> file1a = {{16.54, 5.34, 4.24, 9.14, std::numeric_limits<double>::quiet_NaN()}};
    std::vector<std::vector<double>> file1b = {{-20.2, std::numeric_limits<double>::quiet_NaN(), 100.1}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname2, file1a, 2));
    ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname2, file1b, 1));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_TRUE(comp.CompareColumn(1));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareDoubleDifferentNaNImpls)
{
    //												column1
    std::vector<std::vector<double>> file1a = {{16.54, 5.34, 4.24, -std::numeric_limits<double>::quiet_NaN()}};
    std::vector<std::vector<double>> file1b = {{-20.2, std::numeric_limits<double>::quiet_NaN(), 100.1, -1345677.4}};

    std::vector<std::vector<double>> file2a = {{16.54, 5.34, 4.24, std::numeric_limits<double>::quiet_NaN()}};
    std::vector<std::vector<double>> file2b = {{-20.2, std::numeric_limits<double>::quiet_NaN(), 100.1, -1345677.4}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname1, file1a, 2));
    ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname2, file2a, 2));
    ASSERT_TRUE(CreateParquetFile(arrow::float64(), dirname2, file2b, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_TRUE(comp.CompareColumn(1));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareFloatMatch)
{
    //												column1				column2
    std::vector<std::vector<float>> file1a = {{16.54F, 5.34F, 4.24F,
                                               9.14F, 8.11F},
                                              {7.1F, 8.2F, 20.3F, 50.4F, 60.5F}};
    std::vector<std::vector<float>> file1b = {{-20.2F, 100.1F}, {51.6F, 50.7F}};

    std::vector<std::vector<float>> file2 = {{16.54F, 5.34F, 4.24F,
                                              9.14F, 8.11F,
                                              -20.2F, 100.1F},
                                             {7.1F, 8.2F, 20.3F, 50.4F, 60.5F, 51.6F, 50.7F}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_TRUE(comp.CompareColumn(1));
    EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareFloatMisMatch)
{
    //												column1				column2
     std::vector<std::vector<float>> file1a = {{16.54F, 5.34F, 4.24F,
                                               9.14F, 8.11F},
                                              {7.1F, 8.2F, 20.3F, 50.4F, 60.5F}};
    std::vector<std::vector<float>> file1b = {{-20.2F, 11100.1F}, {51.6F, 50.7F}};

    std::vector<std::vector<float>> file2 = {{16.54F, 5.34F, 4.24F,
                                              9.14F, 8.11F,
                                              -20.2F, 100.1F},
                                             {7.1F, 8.2F, 20.3F, 50.4F, 60.5F, 51.6F, 50.8F}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
    EXPECT_FALSE(comp.CompareColumn(2));
}

/*
	The next two tests are to protect against the beginning postions
	of each vector not being initialized to 0 at each call of CompareColumn.
	One possible route that a vector position would be left at some
	other location is if the row group sizes for each file were 
	different and non matching for the first column. This would
	leave the beginning postion of one of the vectors at a location
	other than the beginning of the vector when CompareColumn is called
	on the next column.
*/
TEST_F(ParquetArrowValidatorTest, ComparatorEnsureVectorBeginningPosition1Reset)
{
    //												column1				column2
    std::vector<std::vector<float>> file1 = {{-16.54F, 5.34F, 4.24F,
                                              9.14F, 8.11F,
                                              -20.2F, 101.1F},
                                             {7.1F, 8.2F, 20.3F, 50.4F, 60.5F, 51.6F, 50.7F}};
    std::vector<std::vector<float>> file2 = {{16.54F, 5.34F, 4.24F,
                                              9.14F, 8.11F,
                                              -20.2F, 101.1F},
                                             {7.1F, 8.2F, 20.3F, 50.4F, 60.5F, 51.6F, 50.7F}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file1, 5));

    // file 2 - Note that the only change from ComparatorEnsureVectorBeginningPosition2Reset is
    // that the row group size flipped, this should expose begin_pos_1_ if it were not initialize
    // back to 0 at CompareColumn(2)
    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
    EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorEnsureVectorBeginningPosition2Reset)
{
    //												column1				column2
    std::vector<std::vector<float>> file1 = {{-16.54F, 5.34F, 4.24F,
                                              9.14F, 8.11F,
                                              -20.2F, 101.1F},
                                             {7.1F, 8.2F, 20.3F, 50.4F, 60.5F, 51.6F, 50.7F}};
    std::vector<std::vector<float>> file2 = {{16.54F, 5.34F, 4.24F,
                                              9.14F, 8.11F,
                                              -20.2F, 101.1F},
                                             {7.1F, 8.2F, 20.3F, 50.4F, 60.5F, 51.6F, 50.7F}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file1, 2));

    // file 2  - Note that the only change from ComparatorEnsureVectorBeginningPosition1Reset is
    // that the row group size flipped, this should expose begin_pos_2_ if it were not initialize
    // back to 0 at CompareColumn(2)
    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname2, file2, 5));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareColumn(1));
    EXPECT_TRUE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareFloatNaN)
{
    //												column1
    std::vector<std::vector<float>> file1a = {{16.54F, 5.34F, 4.24F, 9.14F, std::numeric_limits<float>::quiet_NaN()}};
    std::vector<std::vector<float>> file1b = {{-20.2F, std::numeric_limits<float>::quiet_NaN(), 100.1F}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname2, file1a, 2));
    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname2, file1b, 1));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_TRUE(comp.CompareColumn(1));
}

TEST_F(ParquetArrowValidatorTest, ComparatorCompareTotalCountInts)
{
    //												column1					column2
    std::vector<std::vector<int64_t>> file1a = {{16, 5, 4, 9, 8}, {7, 8, 20, 50, 60}};
    std::vector<std::vector<int64_t>> file1b = {{20, 1000}, {50, 5000}};
    std::vector<std::vector<int64_t>> file2 = {{16, 5, 4, 9, 8, 20, 1000}, {7, 8, 20, 50, 60, 50, 5000}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_EQ(comp.GetComparedCount(1), 0);
    EXPECT_EQ(comp.GetComparedCount(2), 0);
    EXPECT_TRUE(comp.CompareColumn(1));
    EXPECT_EQ(comp.GetComparedCount(1), 7);
    EXPECT_EQ(comp.GetComparedCount(2), 0);
    EXPECT_TRUE(comp.CompareColumn(2));
    EXPECT_EQ(comp.GetComparedCount(1), 7);
    EXPECT_EQ(comp.GetComparedCount(2), 7);

    //		reset counter							column1					column2
    std::vector<std::vector<int64_t>> file1aa = {{16, 5, 4, 9, 8}, {7, 8, 20, 50, 60}};
    std::vector<std::vector<int64_t>> file1bb = {{20}, {50}};
    std::vector<std::vector<int64_t>> file2a = {{16, 5, 4, 9, 8, 20}, {7, 8, 20, 50, 60, 50}};

    dirname1 = "file3.parquet";
    dirname2 = "file4.parquet";

    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1aa, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1bb, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file2a, 2));

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
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
    std::vector<std::vector<uint8_t>> file1a = {{0, 1, 0, 0, 1}, {0, 0, 0, 1, 0}};
    std::vector<std::vector<uint8_t>> file1b = {{1, 1}, {1, 0}};
    std::vector<std::vector<uint8_t>> file2 = {{0, 1, 0, 0, 1, 1, 1}, {0, 0, 0, 1, 0, 1, 0}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
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
    std::vector<std::vector<uint8_t>> file1aa = {{0, 1, 0, 0, 1}, {0, 0, 0, 1, 0}};
    std::vector<std::vector<uint8_t>> file1bb = {{1}, {1}};
    std::vector<std::vector<uint8_t>> file2a = {{0, 1, 0, 0, 1, 1}, {0, 0, 0, 1, 0, 1}};

    dirname1 = "file3.parquet";
    dirname2 = "file4.parquet";

    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file1aa, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file1bb, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname2, file2a, 2));

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
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
    std::vector<std::vector<std::string>> file1a = {{"a", "d", "e", "f", "b"}, {"c", "g", "i", "p", "b"}};
    std::vector<std::vector<std::string>> file1b = {{"f", "g"}, {"t", "k"}};
    std::vector<std::vector<std::string>> file2 = {{"a", "d", "e", "f", "b", "f", "g"}, {"c", "g", "i", "p", "b", "t", "k"}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_EQ(comp.GetComparedCount(1), 0);
    EXPECT_EQ(comp.GetComparedCount(2), 0);
    EXPECT_TRUE(comp.CompareColumn(1));
    EXPECT_EQ(comp.GetComparedCount(1), 7);
    EXPECT_EQ(comp.GetComparedCount(2), 0);
    EXPECT_TRUE(comp.CompareColumn(2));
    EXPECT_EQ(comp.GetComparedCount(1), 7);
    EXPECT_EQ(comp.GetComparedCount(2), 7);

    //resets counter									column1							column2
    std::vector<std::vector<std::string>> file1aa = {{"a", "d", "e", "f", "b"}, {"c", "g", "i", "p", "b"}};
    std::vector<std::vector<std::string>> file1bb = {{"f"}, {"t"}};
    std::vector<std::vector<std::string>> file2a = {{"a", "d", "e", "f", "b", "f"}, {"c", "g", "i", "p", "b", "t"}};

    dirname1 = "file3.parquet";
    dirname2 = "file4.parquet";

    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file1aa, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file1bb, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname2, file2a, 2));

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
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
    std::vector<std::vector<int64_t>> file1a = {{16, 5, 4, 9, 8}, {7, 8, 20, 50, 60}, {8, 3, 7, 1, 5}};
    std::vector<std::vector<int64_t>> file1b = {{20, 1000}, {50, 51000}, {5, 3}};

    // file 2 missing column 3
    std::vector<std::vector<int64_t>> file2 = {{16, 5, 4, 9, 8, 20, 1000}, {7, 8, 20, 50, 60, 50, 5000}};

    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1a, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1b, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file2, 2));

    Comparator comp;

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
    EXPECT_FALSE(comp.CompareAll());
    EXPECT_EQ(comp.GetComparedCount(1), 7);
    EXPECT_EQ(comp.GetComparedCount(2), 7);
    EXPECT_EQ(comp.GetComparedCount(3), 0);
    EXPECT_TRUE(comp.CheckPassed(1));
    EXPECT_FALSE(comp.CheckPassed(2));
    EXPECT_FALSE(comp.CheckPassed(3));

    //resets stats and should pass					column1					column2
    std::vector<std::vector<int64_t>> file1aa = {{16, 5, 4, 9, 8}, {7, 8, 20, 50, 60}};
    std::vector<std::vector<int64_t>> file1bb = {{20}, {50}};
    std::vector<std::vector<int64_t>> file2a = {{16, 5, 4, 9, 8, 20}, {7, 8, 20, 50, 60, 50}};

    dirname1 = "file3.parquet";
    dirname2 = "file4.parquet";

    // file 1
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1aa, 3));
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file1bb, 2));

    // file 2
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file2a, 2));

    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));
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
    std::vector<std::vector<uint8_t>> file = {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
    std::vector<uint8_t> bool_fields = {0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1};

    ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file, 6, &bool_fields));
    ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname2, file, 6, &bool_fields));

    Comparator comp;
    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));

    ASSERT_TRUE(comp.CompareAll());
}

TEST_F(ParquetArrowValidatorTest, Int64NullMatch)
{
    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    std::vector<std::vector<int64_t>> file = {{1, 2, 1, 3, 1, 4, 5, 6, 8, 9, 1, 2, 9, 8}};
    std::vector<uint8_t> bool_fields = {0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1};

    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file, 6, &bool_fields));
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file, 6, &bool_fields));

    Comparator comp;
    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));

    ASSERT_TRUE(comp.CompareAll());
}

TEST_F(ParquetArrowValidatorTest, FloatNullMatch)
{
    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    std::vector<std::vector<float>> file = {{1.5F, 2.F, 1.F, 3.F, 1.F, 4.F, 5.5F, 6.F, -8.66F, 9.F, 1.F, 2.F, 9.F, 8.F}};
    std::vector<uint8_t> bool_fields = {0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1};

    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file, 6, &bool_fields));
    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname2, file, 6, &bool_fields));

    Comparator comp;
    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));

    ASSERT_TRUE(comp.CompareAll());
}

TEST_F(ParquetArrowValidatorTest, StringNullMatch)
{
    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    std::vector<std::vector<std::string>> file = {{"a", "b", "c", "d", "ee", "ff", "gg", "h", "i", "jj", "k"}};
    std::vector<uint8_t> bool_fields = {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1};

    ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file, 6, &bool_fields));
    ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname2, file, 6, &bool_fields));

    Comparator comp;
    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));

    ASSERT_TRUE(comp.CompareAll());
}

TEST_F(ParquetArrowValidatorTest, BoolNullMisMatch)
{
    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    std::vector<std::vector<uint8_t>> file = {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
    std::vector<uint8_t> bool_fields1 = {0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1};
    std::vector<uint8_t> bool_fields2 = {0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0};

    ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname1, file, 6, &bool_fields1));
    ASSERT_TRUE(CreateParquetFile(arrow::boolean(), dirname2, file, 6, &bool_fields2));

    Comparator comp;
    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));

    ASSERT_FALSE(comp.CompareAll());
}

TEST_F(ParquetArrowValidatorTest, Int64NullMisMatch)
{
    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    std::vector<std::vector<int64_t>> file = {{1, 2, 1, 3, 1, 4, 5, 6, 8, 9, 1, 2, 9, 8}};
    std::vector<uint8_t> bool_fields1 = {0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1};
    std::vector<uint8_t> bool_fields2 = {0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0};

    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname1, file, 6, &bool_fields1));
    ASSERT_TRUE(CreateParquetFile(arrow::int64(), dirname2, file, 6, &bool_fields2));

    Comparator comp;
    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));

    ASSERT_FALSE(comp.CompareAll());
}

TEST_F(ParquetArrowValidatorTest, FloatNullMisMatch)
{
    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    std::vector<std::vector<float>> file = {{1.5F, 2.F, 1.F, 3.F, 1.F, 4.F, 5.5F, 6.F, -8.66F, 9.F, 1.F, 2.F, 9.F, 8.F}};
    std::vector<uint8_t> bool_fields = {0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1};
    std::vector<uint8_t> bool_fields1 = {0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1};
    std::vector<uint8_t> bool_fields2 = {0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0};

    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname1, file, 6, &bool_fields1));
    ASSERT_TRUE(CreateParquetFile(arrow::float32(), dirname2, file, 6, &bool_fields2));

    Comparator comp;
    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));

    ASSERT_FALSE(comp.CompareAll());
}

TEST_F(ParquetArrowValidatorTest, StringNullMisMatch)
{
    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";
    std::vector<std::vector<std::string>> file = {{"a", "b", "c", "d", "ee", "ff", "gg", "h", "i", "jj", "k"}};
    std::vector<uint8_t> bool_fields1 = {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1};
    std::vector<uint8_t> bool_fields2 = {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0};

    ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname1, file, 6, &bool_fields1));
    ASSERT_TRUE(CreateParquetFile(arrow::utf8(), dirname2, file, 6, &bool_fields2));

    Comparator comp;
    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));

    ASSERT_FALSE(comp.CompareAll());
}

TEST_F(ParquetArrowValidatorTest, CompareEmptyFilesSameSchema)
{
    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";

    // Zero data rows configured.
    std::vector<float> col0data;
    std::vector<int16_t> col1data;

    ASSERT_TRUE(CreateTwoColParquetFile(dirname1, arrow::float32(), col0data, "col1",
                                        arrow::int16(), col1data, "col2", 5));
    ASSERT_TRUE(CreateTwoColParquetFile(dirname2, arrow::float32(), col0data, "col1",
                                        arrow::int16(), col1data, "col2", 5));

    Comparator comp;
    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));

    ASSERT_TRUE(comp.CompareAll());
    ASSERT_TRUE(comp.CompareColumn(1));
}

TEST_F(ParquetArrowValidatorTest, CompareEmptyFilesDifferentNames)
{
    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";

    // Zero data rows configured.
    std::vector<float> col0data;
    std::vector<int16_t> col1data;

    ASSERT_TRUE(CreateTwoColParquetFile(dirname1, arrow::float32(), col0data, "col1",
                                        arrow::int16(), col1data, "col2", 5));
    ASSERT_TRUE(CreateTwoColParquetFile(dirname2, arrow::float32(), col0data, "col1",
                                        arrow::int16(), col1data, "colA", 5));

    Comparator comp;
    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));

    ASSERT_FALSE(comp.CompareAll());
    ASSERT_TRUE(comp.CompareColumn(1));
    ASSERT_FALSE(comp.CompareColumn(2));
}

TEST_F(ParquetArrowValidatorTest, CompareSameDataDifferentColNames)
{
    std::string dirname1 = "file1.parquet";
    std::string dirname2 = "file2.parquet";

    // Zero data rows configured.
    std::vector<float> col0data = {1.0, 2., 3., 4., 5., 6., 7., 8., 9., 0.};
    std::vector<int16_t> col1data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};

    ASSERT_TRUE(CreateTwoColParquetFile(dirname1, arrow::float32(), col0data, "col1",
                                        arrow::int16(), col1data, "col2", 5));
    ASSERT_TRUE(CreateTwoColParquetFile(dirname2, arrow::float32(), col0data, "col1",
                                        arrow::int16(), col1data, "colA", 5));

    Comparator comp;
    ASSERT_EQ(EX_OK, comp.Initialize(dirname1, dirname2));

    ASSERT_FALSE(comp.CompareAll());
    ASSERT_TRUE(comp.CompareColumn(1));
    ASSERT_FALSE(comp.CompareColumn(2));
}
