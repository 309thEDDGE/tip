#include "comparator.h"

bool Comparator::Initialize(std::string file1, std::string file2)
{
	if (!std::filesystem::exists(file1))
	{
		printf("\nERROR!! parquet directory %s doesn't exist: \n", file1.c_str());
		return false;
	}

	if (!std::filesystem::exists(file2))
	{
		printf("\nERROR!! parquet directory %s doesn't exist: \n", file2.c_str());
		return false;
	}

	bool return_status = false;
	return_status = pm1_.SetPQPath(file1);
	if (!return_status)
		return false;
	return_status = pm2_.SetPQPath(file2);
	if (!return_status)
		return false;
	
	InitializeStats();

	return return_status;
}

void Comparator::InitializeStats()
{
	compared_count_.clear();
	columns_passed_.clear();

	int max_fields = 0;
	if (pm2_.schema_->num_fields() > pm1_.schema_->num_fields())
		max_fields = pm2_.schema_->num_fields();
	else
		max_fields = pm1_.schema_->num_fields();

	for (int i = 0; i < max_fields; i++)
	{
		compared_count_[(i + 1)] = 0;
		columns_passed_[(i + 1)] = false;
	}
}
bool Comparator::CompareColumn(int column)
{
	printf("\n ---Comparing Column %d--- \n", column);
	bool is_list = false;

	// Make sure the column does not exceed the total number
	// of columns for either parquet file
	if (column > pm1_.schema_->num_fields() || column < 1)
	{
		printf("\nERROR!! Column %d not within range 0 -> %d \n", 
			column, pm1_.schema_->num_fields());
		return false;
	}

	// print column name
	printf("File 1 Col Name: %s \n", 
		pm1_.schema_->fields()[column - 1]->name().c_str());
	if (column > pm2_.schema_->num_fields() || column < 1)
	{
		printf("\nERROR!! Column %d not within range 0 -> %d \n", 
			column, pm2_.schema_->num_fields());
		return false;
	}

	// print column name
	printf("File 2 Col Name: %s \n",
		pm2_.schema_->fields()[column - 1]->name().c_str());

	// Start the comparison at the first parquet file in the folder
	ZeroRG();

	// reset stats for the column
	compared_count_[column] = 0;
	columns_passed_[column] = false;

	int dtype1 = 
		pm1_.schema_->fields()[column - 1]->type()->id();
	int dtype2 = 
		pm2_.schema_->fields()[column - 1]->type()->id();

	if (dtype1 != dtype2)
	{
		printf("\nERROR!! Column %d datatypes do not match\n", 
			(column));
		return false;
	}

	// If it is a list, assume the data type is Int32Type
	if (dtype1 == arrow::ListType::type_id)
	{
		is_list = true;
		dtype1 = arrow::Int32Type::type_id;
	}
	
	bool ret_status = false;
	switch (dtype1)
	{
		
		case arrow::Int64Type::type_id:
		{
			printf("Type: Int64Type\n");
			ret_status = Compare< arrow::NumericArray<arrow::Int64Type>, int64_t>(column, is_list);
			break;
		}
		case arrow::Int32Type::type_id:
		{
			printf("Type: Int32Type\n");
			ret_status = Compare<arrow::NumericArray<arrow::Int32Type>, int32_t>(column, is_list);
			break;
		}
		case arrow::Int16Type::type_id:
		{
			printf("Type: Int16Type\n");
			ret_status = Compare< arrow::NumericArray<arrow::Int16Type>, int16_t>(column, is_list);
			break;
		}
		case arrow::Int8Type::type_id:
		{
			printf("Type: Int8Type\n");
			ret_status = Compare< arrow::NumericArray<arrow::Int8Type>, int8_t>(column, is_list);
			break;
		}
		case arrow::BooleanType::type_id:
		{
			printf("Type: BooleanType\n");
			ret_status = CompareBool(column, is_list);
			break;
		}
		case arrow::DoubleType::type_id:
		{
			printf("Type: DoubleType\n");
			ret_status = Compare< arrow::NumericArray<arrow::DoubleType>, double>(column, is_list);
			break;
		}
		case arrow::FloatType::type_id:
		{
			printf("Type: FloatType\n");
			ret_status = Compare< arrow::NumericArray<arrow::FloatType>, float>(column, is_list);
			break;
		}
		case arrow::StringType::type_id:
		{
			printf("Type: StringType\n");
			ret_status = CompareString(column, is_list);
			break;
		}

		default:
			ret_status = false;
			break;

	}

	if (ret_status)
	{
		printf("Rows Passed: %d\n", compared_count_[column]);
		printf("PASSED\n");
	}		
	else
	{
		printf("approx rows Passed: %d\n", compared_count_[column]);
		printf("FAILED\n");
	}
	return ret_status;
}

int Comparator::GetComparedCount(int column)
{
	if (compared_count_.find(column) == compared_count_.end())
	{
		return 0;
	}
	else
		return compared_count_[column];
}

bool Comparator::CheckPassed(int column)
{
	if (columns_passed_.find(column) == columns_passed_.end())
	{
		return false;
	}
	else
		return columns_passed_[column];
}

bool Comparator::CompareAll()
{
	InitializeStats();

	int max_fields = 0;
	if (pm2_.schema_->num_fields() > pm1_.schema_->num_fields())
		max_fields = pm2_.schema_->num_fields();
	else
		max_fields = pm1_.schema_->num_fields();

	for (int i = 0; i < max_fields; i++)
	{
		columns_passed_[i + 1] = CompareColumn(i + 1);
	}


	printf("\n\n------------Final Results------------\n");
	bool pass = true;

	// if the parquet files don't have the same column count return false
	if (pm2_.schema_->num_fields() != pm1_.schema_->num_fields())
		pass = false;
	
	std::vector<int> passed_cols;
	std::vector<int> failed_cols;
	// if any of the column mismatched return false
	for (std::map<int, bool>::iterator it = columns_passed_.begin(); it != columns_passed_.end(); ++it)
	{
		if (!it->second)
		{
			pass = false;
			failed_cols.push_back(it->first);
		}
		else
		{
			passed_cols.push_back(it->first);
		}
	}

	// Print Passes
	printf("\nPassed Cols--\n");
	for (int i = 0; i < passed_cols.size(); i++)
	{
		printf("%d", passed_cols[i]);
		if (i != passed_cols.size() - 1)
			printf(", ");

	}

	printf("\n\n");
	// Print Fails
	printf("\n\nFailed Cols--\n");
	for (int i = 0; i < failed_cols.size(); i++)
	{
		printf("%d", failed_cols[i]);
		if (i != failed_cols.size() - 1)
			printf(", ");
	}

	if (!pass)
		printf("\n\nOverall -> Fail\n");
	else
		printf("\n\nOverall -> Pass\n");
	printf("------------\n\n");
	return pass;
}


bool Comparator::CompareString(int column, bool is_list)
{
	// adjust to zero based column
	column--;

	int size1 = 0;
	int size2 = 0;

	std::vector<std::string> buffer1;
	std::vector<std::string> buffer2;

	if (!pm1_.GetNextRGString(column,
		buffer1,
		size1, is_list))
	{
		return false;
	}

	if (!pm2_.GetNextRGString(column,
		buffer2,
		size2, is_list))
	{
		return false;
	}

	while (size1 > 0 || size2 > 0)
	{
		if (!CompareVecs(buffer1, size1, buffer2, size2, (column + 1)))
		{
			return false;
		}

		if (size1 == 0)
		{
			pm1_.GetNextRGString(column,
				buffer1,
				size1, is_list);
		}
		if (size2 == 0)
		{
			pm2_.GetNextRGString(column,
				buffer2,
				size2, is_list);
		}
	}
	return true;
}



bool Comparator::CompareBool(int column, bool is_list)
{
	// adjust to zero based column
	column--;

	int size1 = 0;
	int size2 = 0;

	std::vector<bool> buffer1;
	std::vector<bool> buffer2;

	if (!pm1_.GetNextRGBool(column,
		buffer1,
		size1, is_list))
	{
		return false;
	}

	if (!pm2_.GetNextRGBool(column,
		buffer2,
		size2, is_list))
	{
		return false;
	}

	while (size1 > 0 || size2 > 0)
	{
		if (!CompareVecs(buffer1, size1, buffer2, size2, (column + 1)))
		{
			return false;
		}

		if (size1 == 0)
		{
			pm1_.GetNextRGBool(column,
				buffer1,
				size1, is_list);
		}
		if (size2 == 0)
		{
			pm2_.GetNextRGBool(column,
				buffer2,
				size2, is_list);
		}
	}
	return true;
}