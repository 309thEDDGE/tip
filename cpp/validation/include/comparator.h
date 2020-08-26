#ifndef COMPARATOR
#define COMPARATOR

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/schema.h>
#include <filesystem>
#include <map>
#include <set>
#include "parquet_manager.h"


class Comparator {

private:
	bool compare_vec_result_;
	int begin_pos_1_;
	int begin_pos_2_;
	ParquetManager pm1_;
	ParquetManager pm2_;
	std::map<int, int> compared_count_;
	std::map<int, bool> columns_passed_;
	void InitializeStats();
	
	template<typename A, typename T>
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
	};

	~Comparator() {};

	/*
		Pass the two parquet folder paths to be compared

		Returns: False -> If either path is invalid
				 True  -> Other wise	
	*/
	bool Initialize(std::string file1, std::string file2);

	/*
		Compares two vectors from start position to (size + start position)
		Updates size and start positions after the comparison is made

		Returns: False -> If vectors from start position to (size + start position) 
							don't match
				 True  -> If vectors from start position to (size + start position) 
							match
	*/
	template<typename T>
	bool CompareVecs(std::vector<T>& vec1, 
		int& size1,
		std::vector<T>& vec2, 
		int& size2,
		int column = 0);

	/*
		Compares columns between two parquet files
		If the data types of the two columns are not
		equivalent they will not match.
		Note: if the column is a list, it will assume
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
};


template<typename T>
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
				std::equal((uint8_t*)vec2.data() + (begin_pos_2_ * sizeof(T)),
					(uint8_t*)vec2.data() + ((size2 + begin_pos_2_) * sizeof(T)),
					(uint8_t*)vec1.data() + (begin_pos_1_ * sizeof(T)));
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
				std::equal((uint8_t*)vec2.data() + (begin_pos_2_ * sizeof(T)),
					(uint8_t*)vec2.data() + ((size2 + begin_pos_2_) * sizeof(T)),
					(uint8_t*)vec1.data() + (begin_pos_1_ * sizeof(T)));
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
				std::equal((uint8_t*)vec1.data() + (begin_pos_1_ * sizeof(T)),
					(uint8_t*)vec1.data() + ((size1 + begin_pos_1_) * sizeof(T)),
					(uint8_t*)vec2.data() + (begin_pos_2_ * sizeof(T)));
			compared_count_[column] = compared_count_[column] + size1;
			size2 = size2 - size1;
			begin_pos_2_ += size1;
			begin_pos_1_ = 0;
			size1 = 0;
		}
	}	

	return compare_vec_result_;
}

template<typename A, typename T>
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

#endif