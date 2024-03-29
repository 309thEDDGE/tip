#ifndef COMPARATOR
#define COMPARATOR

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/schema.h>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "sysexits.h"
#include "parquet_reader.h"
#include "managed_path.h"

class Comparator
{
   private:
    bool compare_vec_result_;
    int begin_pos_1_;
    int begin_pos_2_;
    ParquetReader pm1_;
    ParquetReader pm2_;
    std::map<int, int> compared_count_;
    std::map<int, bool> columns_passed_;
    bool failure_;
    void InitializeStats();

    template <typename A, typename T>
    bool Compare(int column, bool is_list);
    bool CompareBool(int column, bool is_list);
    bool CompareString(int column, bool is_list);

    void ZeroRG()
    {
        pm1_.Zeroize();
        pm2_.Zeroize();
    }

   public:
    Comparator()
    {
        begin_pos_1_ = 0;
        begin_pos_2_ = 0;
        compare_vec_result_ = false;
        failure_ = false;
    };

    ~Comparator() {}

    /*
		Pass the two parquet folder paths to be compared

		Returns: nonzero -> If either path is invalid
				 0  -> Other wise	
	*/
    int Initialize(ManagedPath path1, ManagedPath path2);
    int Initialize(std::string path1, std::string path2);

    /*
		Compares two vectors from start position to (size + start position)
		Updates size and start positions after the comparison is made

		Returns: False -> If vectors from start position to (size + start position) 
							don't match
				 True  -> If vectors from start position to (size + start position) 
							match
	*/
    template <typename T>
    bool CompareVecs(std::vector<T>& vec1,
                     int& size1,
                     std::vector<T>& vec2,
                     int& size2,
                     int column = 0);

    /*
		Compares columns between two parquet files
		If the data types of the two columns are not
		equivalent they will not match.
		Note!!! if the column is a list, it will assume
		that the list type is INT32. 

		Returns: False -> If columns don't match
				 True  -> If columns match
	*/
    bool CompareColumn(int column);

    /*
		Retrieve the amount of rows compared in a given column
		If the column is of type list, (rows compared * list size)
		will be returned
		Note! this is only accurate if the two columns
		passed the comparison

		Returns: the number of columns compared (only accurate
		if the two columns passed the comparison)
	*/
    int GetComparedCount(int column);

    /*
		Check if a given column passed the last comparison

		Returns: False -> If column didn't match
				 True  -> If column matched
	*/
    bool CheckPassed(int column);

    /*
		Compare all the rows in the two parquet files

		Returns: False -> If any column doesn't match, or if
							one column doesn't exist in the other file
				 True  -> If all columns match
	*/
    bool CompareAll();

    /*
		Loop over vectors vec1 and vec2, comparing row_count 
		rows. Comparison indices of vec1 and vec2 start at 
		begin_pos_1_ and begin_pos_2_, respectively. Used
		primarily for comparing float values.

		Return: False -> If any rows do not equate and both rows
		                 are not NaNs.
			    True  -> Otherwise
	*/
    template <typename T>
    bool ComparisonLoop(const std::vector<T>& vec1,
                        const std::vector<T>& vec2,
                        int row_count,
                        int column);
};

template <typename T>
bool Comparator::CompareVecs(std::vector<T>& vec1,
                             int& size1,
                             std::vector<T>& vec2,
                             int& size2,
                             int column)
{
    if (typeid(T) == typeid(std::string))
    {
        // If nothing exists in one of the vectors
        // comparison should be false
        if (size1 == 0 || size2 == 0)
        {
            begin_pos_1_ = 0;
            begin_pos_2_ = 0;
            size1 = 0;
            size2 = 0;
            return false;
        }
        // If both vectors are the same size
        // compare both and reset all positions
        if (size1 == size2)
        {
            compare_vec_result_ =
                std::equal(vec2.begin() + begin_pos_2_,
                           vec2.begin() + (size2 + begin_pos_2_),
                           vec1.begin() + begin_pos_1_);
            compared_count_[column] = compared_count_[column] + size1;
            begin_pos_1_ = 0;
            begin_pos_2_ = 0;
            size1 = 0;
            size2 = 0;
        }
        // If vector 1 is bigger than vector 2
        // compare against vector 2 and prepare
        // vector 1 position for the next comparison
        else if (size1 > size2)
        {
            compare_vec_result_ =
                std::equal(vec2.begin() + begin_pos_2_,
                           vec2.begin() + (size2 + begin_pos_2_),
                           vec1.begin() + begin_pos_1_);
            compared_count_[column] = compared_count_[column] + size2;
            size1 = size1 - size2;
            begin_pos_1_ += size2;
            begin_pos_2_ = 0;
            size2 = 0;
        }
        // If vector 2 is bigger than vector 1
        // compare against vector 1 and prepare
        // vector 2 position for the next comparison
        else
        {
            compare_vec_result_ =
                std::equal(vec1.begin() + begin_pos_1_,
                           vec1.begin() + (size1 + begin_pos_1_),
                           vec2.begin() + begin_pos_2_);
            compared_count_[column] = compared_count_[column] + size1;
            size2 = size2 - size1;
            begin_pos_2_ += size1;
            begin_pos_1_ = 0;
            size1 = 0;
        }
    }
    else
    {
        // Note the conversion to uint8_t allows for comparisons of NaN
        // in the case of float and double
        // If nothing exists in one of the vectors
        // comparison should be false
        if (size1 == 0 || size2 == 0)
        {
            begin_pos_1_ = 0;
            begin_pos_2_ = 0;
            size1 = 0;
            size2 = 0;
            return false;
        }
        // If both vectors are the same size
        // compare both and reset all positions
        if (size1 == size2)
        {
            compare_vec_result_ =
                std::equal(reinterpret_cast<uint8_t*>(vec2.data()) + (begin_pos_2_ * sizeof(T)),
                           reinterpret_cast<uint8_t*>(vec2.data()) + ((size2 + begin_pos_2_) * sizeof(T)),
                           reinterpret_cast<uint8_t*>(vec1.data()) + (begin_pos_1_ * sizeof(T)));
            compared_count_[column] = compared_count_[column] + size1;
            begin_pos_1_ = 0;
            begin_pos_2_ = 0;
            size1 = 0;
            size2 = 0;
        }
        // If vector 1 is bigger than vector 2
        // compare against vector 2 and prepare
        // vector 1 position for the next comparison
        else if (size1 > size2)
        {
            compare_vec_result_ =
                std::equal(reinterpret_cast<uint8_t*>(vec2.data()) + (begin_pos_2_ * sizeof(T)),
                           reinterpret_cast<uint8_t*>(vec2.data()) + ((size2 + begin_pos_2_) * sizeof(T)),
                           reinterpret_cast<uint8_t*>(vec1.data()) + (begin_pos_1_ * sizeof(T)));
            compared_count_[column] = compared_count_[column] + size2;
            size1 = size1 - size2;
            begin_pos_1_ += size2;
            begin_pos_2_ = 0;
            size2 = 0;
        }
        // If vector 2 is bigger than vector 1
        // compare against vector 1 and prepare
        // vector 2 position for the next comparison
        else
        {
            compare_vec_result_ =
                std::equal(reinterpret_cast<uint8_t*>(vec1.data()) + (begin_pos_1_ * sizeof(T)),
                           reinterpret_cast<uint8_t*>(vec1.data()) + ((size1 + begin_pos_1_) * sizeof(T)),
                           reinterpret_cast<uint8_t*>(vec2.data()) + (begin_pos_2_ * sizeof(T)));
            compared_count_[column] = compared_count_[column] + size1;
            size2 = size2 - size1;
            begin_pos_2_ += size1;
            begin_pos_1_ = 0;
            size1 = 0;
        }
    }

    return compare_vec_result_;
}

template <typename A, typename T>
bool Comparator::Compare(int column, bool is_list)
{
    // adjust to zero based column
    column--;

    int size1 = 0;
    int size2 = 0;

    std::vector<T> buffer1;
    std::vector<T> buffer2;

    if (!pm1_.GetNextRG<T, A>(column,
                              buffer1,
                              size1, is_list))
    {
        return false;
    }

    if (!pm2_.GetNextRG<T, A>(column,
                              buffer2,
                              size2, is_list))
    {
        return false;
    }
    /*
	for (int i = 0; i < size1; i++)
	{
		if (buffer1[i] != buffer2[i])
			printf("buffers at position %d aren't equivalent: buffer1 = %d, buffer2 = %d\n", buffer1[i], buffer2[i]);
	}*/

    while (size1 > 0 || size2 > 0)
    {
        if (!CompareVecs(buffer1, size1, buffer2, size2, (column + 1)))
        {
            return false;
        }

        if (size1 == 0)
        {
            pm1_.GetNextRG<T, A>(column,
                                 buffer1,
                                 size1, is_list);
        }
        if (size2 == 0)
        {
            pm2_.GetNextRG<T, A>(column,
                                 buffer2,
                                 size2, is_list);
        }
    }
    return true;
}

template <typename T>
bool Comparator::ComparisonLoop(const std::vector<T>& vec1,
                                const std::vector<T>& vec2,
                                int row_count,
                                int column)
{
    bool comparison_result = true;
    bool both_are_nan = false;
    for (int i = 0; i < row_count; i++)
    {
        if (vec2[begin_pos_2_ + i] != vec1[begin_pos_1_ + i])
        {
            // NaN checks are ignored because NaNs are, by definition, not comparable,
            // or at least will always be compared as unequal. In the context of this code,
            // the purpose of which is to compare two parquet files and indicate equality,
            // the comparison of NaNs will indicate that the two files are unequal always.
            // We do not wish to indicate inequality in this case, so if the values in
            // comparison are not equal and at least one is not a NaN, then indicate
            // inequality.
            both_are_nan = std::isnan(vec2[begin_pos_2_ + i]) && std::isnan(vec1[begin_pos_1_ + i]);
            if (!both_are_nan)
                comparison_result = false;
        }
    }
    compared_count_[column] = compared_count_[column] + row_count;

    return comparison_result;
}
#endif