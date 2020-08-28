#include "translatable_1553_table.h"

Translatable1553Table::Translatable1553Table(std::string msg_name, ICDData* icd, size_t icd_table_index)
	: row_group_size_(0), name_(msg_name), column_count_(0), col_ind_(0), source_raw_data_ptr_(nullptr),
	translation_err_count_(0), temp_wordno_(0), temp_bitmsb_(0), icd_schema_(ICDElementSchema::BAD),
	temp_is_bitlevel_(false), temp_byte_count_(0), allocated_row_count_(0), pq_(nullptr), temp_name_(""),
	bad_value_(0), icd_(icd), icd_translate_(), icd_table_index_(icd_table_index)
{

}

uint8_t Translatable1553Table::create_ridealong_column(const int& row_alloc_count, std::shared_ptr<arrow::Field> arrow_field)
{
	std::string name = arrow_field->name();
	std::shared_ptr<arrow::DataType> arrow_type = arrow_field->type();
	switch (arrow_type->id())
	{
	case arrow::Int64Type::type_id:
	{
		ridealong_columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<int64_t>(name, true, 
			arrow_type, row_alloc_count)));
		return 0;
	}
	default:
	{
		printf("Translatable1553Table::create_ridealong_column(): No case for arrow type %s",
			arrow_field->type()->name().c_str());
	}
	}
	
	return 1;
}

std::string& Translatable1553Table::name()
{
	return name_;
}

uint8_t Translatable1553Table::create_translatable_columns(const int& row_alloc_count)
{
	allocated_row_count_ = row_alloc_count;

	// Get the ICD element indices associated with the
	// table index.
	const std::vector<size_t>& elem_inds = icd_->GetTableIndices(icd_table_index_);

	// Check to see if there are zero valid entries.
	if (elem_inds.size() == 0)
	{
		printf("Translatable1553Table::create_translatable_columns(): No valid columns for table %s\n",
			name_.c_str());
		bad_value_ = 1;
		return 0;
	}

	// For each column name, create a new TranslatableColumn and append it to the
	// vector of columns.
	const ICDElement* icd_elem_ptr = nullptr;
	for (std::vector<size_t>::const_iterator it = elem_inds.begin(); it != elem_inds.end(); ++it)
	{
		icd_elem_ptr = icd_->GetElementPointerByIndex(*it);

		/*********** DEBUG ***********/
		/*if (icd_elem_ptr->is_bitlevel_)
		{
			printf("!!!!WARNING!!!!! Translatable1553Table::create_translatable_columns(): skipping bit-level cols!\n");
			continue;
		}*/
		/*********** END DEBUG ***********/
#ifdef DEBUG
#if DEBUG > 2
		printf("col name: %s\n", icd_elem_ptr->elem_name_.c_str());
#endif
#endif

		if (create_column_by_attributes(icd_elem_ptr, row_alloc_count) == 1)
			return 1;
	}

	column_count_ = columns_.size();

	// If there are zero columns present then the table is null. Indicate
	// by setting table state as bad.
	if (column_count_ == 0)
		bad_value_ = 1;

	return 0;
}

uint8_t Translatable1553Table::create_column_by_attributes(const ICDElement* icd_elem_ptr,
	const int& row_alloc_count)
{
	// Treat bit-level and word-level translation columns differently. Bit-level translation
	// for signed vs. unsigned bits should be treated differently.
#ifdef DEBUG
#if DEBUG > 3
	printf("schema is %hhu\n", static_cast<uint8_t>(icd_elem_ptr->schema_));
#endif
#endif
	if (icd_elem_ptr->schema_ == ICDElementSchema::SIGNEDBITS)
	{
		if (icd_elem_ptr->bit_count_ < 25)
		{
			columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<float>(
				icd_elem_ptr, false, arrow::float32(), row_alloc_count)));
		}
		else
		{
			columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<double>(
				icd_elem_ptr, false, arrow::float64(), row_alloc_count)));
		}
		return 0;
	}
	else if (icd_elem_ptr->schema_ == ICDElementSchema::UNSIGNEDBITS)
	{
		if (icd_elem_ptr->bit_count_ == 1)
		{
			columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<uint8_t>(
				icd_elem_ptr, false, arrow::boolean(), row_alloc_count)));
			return 0;
		}
		else if (icd_elem_ptr->bit_count_ < 25)
		{
			columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<float>(
				icd_elem_ptr, false, arrow::float32(), row_alloc_count)));
			return 0;
		}
		else
		{
			columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<double>(
				icd_elem_ptr, false, arrow::float64(), row_alloc_count)));
			return 0;
		}
		return 1;

	}
	else if (icd_elem_ptr->schema_ == ICDElementSchema::ASCII)
	{
		columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<int8_t>(
			icd_elem_ptr, false, arrow::int8(), row_alloc_count)));
		return 0;
	}
	else
	{
		// The translated data type depends on the comet-specified schema.
		switch (icd_elem_ptr->schema_)
		{
		case ICDElementSchema::SIGNED16:
		{
			columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<float>(
				icd_elem_ptr, false, arrow::float32(), row_alloc_count)));
			return 0;
		}
		case ICDElementSchema::SIGNED32:
		{
			columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<double>(
				icd_elem_ptr, false, arrow::float64(), row_alloc_count)));
			return 0;
		}
		case ICDElementSchema::UNSIGNED16:
		{
			columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<float>(
				icd_elem_ptr, false, arrow::float32(), row_alloc_count)));
			return 0;
		}
		case ICDElementSchema::UNSIGNED32:
		{
			columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<double>(
				icd_elem_ptr, false, arrow::float64(), row_alloc_count)));
			return 0;
		}
		case ICDElementSchema::FLOAT32_1750:
		{
			columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<float>(
				icd_elem_ptr, false, arrow::float32(), row_alloc_count)));
			return 0;
		}
		case ICDElementSchema::FLOAT32_GPS:
		{
			columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<float>(
				icd_elem_ptr, false, arrow::float32(), row_alloc_count)));
			return 0;
		}
		case ICDElementSchema::FLOAT32_IEEE:
		{
			columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<float>(
				icd_elem_ptr, false, arrow::float32(), row_alloc_count)));
			return 0;
		}
		case ICDElementSchema::FLOAT64_IEEE:
		{
			columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<double>(
				icd_elem_ptr, false, arrow::float64(), row_alloc_count)));
			return 0;
		}
		case ICDElementSchema::FLOAT64_GPS:
		{
			columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<double>(
				icd_elem_ptr, false, arrow::float64(), row_alloc_count)));
			return 0;
		}
		case ICDElementSchema::FLOAT16:
		{
			columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<float>(
				icd_elem_ptr, false, arrow::float32(), row_alloc_count)));
			return 0;
		}
		case ICDElementSchema::CAPS:
		{
			columns_.push_back(std::unique_ptr<TranslatableBase>(new TranslatableColumn<double>(
				icd_elem_ptr, false, arrow::float64(), row_alloc_count)));
			return 0;
		}
		default:
		{
			printf("Translatable1553Table::create_column_by_attributes(): No column created!\n");
			return 1;
		}
		}
	}

	return 1;
}

void Translatable1553Table::append_data(int64_t& time, const int32_t* raw_data, int64_t& row_ind, bool debug)
{
	// Cast the ridealong columns in order to append the specific data type.
	// TODO: Will need to have switch logic at some point to handle multiple ridealong columns
	// or pure virtual functions that must be overloaded to handle custom ridealong cols.
	/*if (ridealong_columns_.size() > 0)*/
#ifdef DEBUG
#if DEBUG > 2
		printf("ridealong_columns_ size = %zu\n", ridealong_columns_.size());
#endif
#endif

	dynamic_cast<TranslatableColumn<int64_t>*>(ridealong_columns_[0].get())->append_translated_value(time);
	/*else
	{
		printf("ridealong_columns == 0\n");
		return;
	}*/

	// For each of the other TranslatableColumn(s), append the raw values.
#ifdef DEBUG
#if DEBUG > 2
	printf("columns_ size = %zu\n", columns_.size());
#endif
#endif
		
	/*if (column_count_ != columns_.size())
	{
		printf("column_count_ not equal to col size\n");
		return;
	}*/
	for (col_ind_ = 0; col_ind_ < column_count_; col_ind_++)
	{
#ifdef DEBUG
#if DEBUG > 3
		printf("appending raw values to col with index %02hu, row %lld, offset %hhu\n", col_ind_,
			row_ind, columns_[col_ind_]->offset());
#endif
#endif
		source_raw_data_ptr_ = raw_data + row_ind * uint64_t(DATA_PAYLOAD_LIST_COUNT) + columns_[col_ind_]->offset();
		columns_[col_ind_]->append_raw_values(source_raw_data_ptr_);
	}
}

void Translatable1553Table::append_data(int64_t& time, const int32_t* raw_data, int64_t& row_ind)
{
	// Cast the ridealong columns in order to append the specific data type.
	// TODO: Will need to have switch logic at some point to handle multiple ridealong columns
	// or pure virtual functions that must be overloaded to handle custom ridealong cols.
	dynamic_cast<TranslatableColumn<int64_t>*>(ridealong_columns_[0].get())->append_translated_value(time);

	// For each of the other TranslatableColumn(s), append the raw values.
	for (col_ind_ = 0; col_ind_ < column_count_; col_ind_++)
	{
		source_raw_data_ptr_ = raw_data + row_ind * uint64_t(DATA_PAYLOAD_LIST_COUNT) + columns_[col_ind_]->offset();
		columns_[col_ind_]->append_raw_values(source_raw_data_ptr_);

		/********* Debug *********/
		/*if (columns_[col_ind_]->name() == "CI03_I-2509")
		{
			printf("word 24 val: %hu\n", *(raw_data + row_ind * uint64_t(DATA_PAYLOAD_LIST_COUNT) + 24));
		}*/
		/********* End Debug *********/
	}
}

uint16_t Translatable1553Table::translate()
{
	printf("\nTranslatable1553Table::translate(): table %s", name_.c_str());

	// For each TranslatableColumn in columns_, translate
	// the raw data.
	translation_err_count_ = 0;
	bool translation_failure = false;
	const ICDElement* icd_ptr = nullptr;
	for (col_ind_ = 0; col_ind_ < column_count_; col_ind_++)
	{
		translation_failure = false;
		icd_ptr = columns_[col_ind_]->icd_elem_ptr_;
#ifdef DEBUG
#if DEBUG > 1
		printf("Column to be translated: %s\n", columns_[col_ind_]->name().c_str());
#endif
#endif

		/*
		Translation routines use the .size() of input raw data vector to determine
		how many values are present that require translation. If the vector is filled
		with a count not equal to the allocated size, then the arbitrary data that
		exist in the memory after the currently filled count, then all memory slots
		up to the allocated amount will translated and garbage data will result in
		the output. For this reason, check to see if the fill count is less than the
		allocated size and resize the vector to the fill count. This should only occur
		during the finalize_translation() routine in which all input Parquet files
		have been processed and there are remaining raw data in some of the tables.
		In this case, this program will exit after this final translation step so we
		don't need to worry about resizing the input vectors for further processing.
		*/
		if (columns_[col_ind_]->append_count() != columns_[col_ind_]->raw_data_ref_.size())
		{
#ifdef DEBUG
#if DEBUG > 1
			printf("Resizing raw data to %u elements!\n", columns_[col_ind_]->append_count());
#endif
#endif
			columns_[col_ind_]->raw_data_ref_.resize(columns_[col_ind_]->append_count());
		}

		if (icd_ptr->is_bitlevel_)
		{
			// bit-level translation type
			switch (icd_ptr->schema_)
			{
			case ICDElementSchema::ASCII:
				if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
					dynamic_cast<TranslatableColumn<int8_t>*>(columns_[col_ind_].get())->translated_data_ref_,
					*icd_ptr))
				{
					translation_failure = true; translation_err_count_++; 
				}
				break;
			case ICDElementSchema::SIGNEDBITS:
				if (icd_ptr->bit_count_ < 25)
				{
					if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
						dynamic_cast<TranslatableColumn<float>*>(columns_[col_ind_].get())->translated_data_ref_,
						*icd_ptr))
					{
						translation_failure = true; translation_err_count_++;
					}
				}
				else
				{
					if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
						dynamic_cast<TranslatableColumn<double>*>(columns_[col_ind_].get())->translated_data_ref_,
						*icd_ptr))
					{
						translation_failure = true; translation_err_count_++;
					}
				}
				break;
			case ICDElementSchema::UNSIGNEDBITS:
				if (icd_ptr->bit_count_ == 1)
				{
					if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
						dynamic_cast<TranslatableColumn<uint8_t>*>(columns_[col_ind_].get())->translated_data_ref_,
						*icd_ptr))
					{
						translation_failure = true; translation_err_count_++;
					}
				}
				else if (icd_ptr->bit_count_ < 25)
				{
					if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
						dynamic_cast<TranslatableColumn<float>*>(columns_[col_ind_].get())->translated_data_ref_,
						*icd_ptr))
					{
						translation_failure = true; translation_err_count_++;
					}
				}
				else
				{
					if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
						dynamic_cast<TranslatableColumn<double>*>(columns_[col_ind_].get())->translated_data_ref_,
						*icd_ptr))
					{
						translation_failure = true; translation_err_count_++;
					}
				}
				break;
			}
		}
		else
		{
			// word translation type
			switch (icd_ptr->schema_)
			{
			case ICDElementSchema::MULTIPLE_BIT:
				printf("Translatable1553Table::translate(): Error: bitmsb is zero, should not have multiple bit type\n");
				translation_err_count_++;
				break;
			case ICDElementSchema::MODE_CODE:
				printf("Translatable1553Table::translate(): Error: MODE CODE -- no data to translate\n");
				translation_err_count_++;
				break;
			case ICDElementSchema::SIGNED16:
				if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
					dynamic_cast<TranslatableColumn<float>*>(columns_[col_ind_].get())->translated_data_ref_,
					*icd_ptr))
				{
					translation_failure = true; translation_err_count_++;
				}				
				break;
			case ICDElementSchema::SIGNED32:
				/*if (print_msg_type)
					printf("message type: int32 (%u elements)\n\n", uint32_t(raw_data_[i].size() / words_per_datum_count));*/
					// Note the special translation function ("...int32"): the output array passed below (tdata_int32_[]) is 
					// actually an vector of floats. The scale value applied during translation may result in a value
					// than can't be expressed as an integer. In example, latitude expressed in Pirad is -0.5 to 0.5. 
					// Therefore return a float array.

					/*
					Note: Original data type is int32 which has 31 bits precision. Originally these values were
					translated to float32 which has ~24 bits precision (6 -- 9 sig figs) counting the implicit or
					hidden bit. Although this level of precision is likely greater than the majority of the relevant
					associated errors, there is still some possibility of precision loss. To avoid this issue the
					decision was made by the team (Isaac Myers and Justin Roberts) to use double as translated
					(and scaled) output type. Several other translation routines use large translated type for
					the same reason.
					*/
				if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
					dynamic_cast<TranslatableColumn<double>*>(columns_[col_ind_].get())->translated_data_ref_,
					*icd_ptr))
				{
					translation_failure = true; translation_err_count_++;
				}				
				break;
			case ICDElementSchema::UNSIGNED16:
				if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
					dynamic_cast<TranslatableColumn<float>*>(columns_[col_ind_].get())->translated_data_ref_,
					*icd_ptr))
				{
					translation_failure = true; translation_err_count_++;
				}				
				break;
			case ICDElementSchema::UNSIGNED32:
				if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
					dynamic_cast<TranslatableColumn<double>*>(columns_[col_ind_].get())->translated_data_ref_,
					*icd_ptr))
				{
					translation_failure = true; translation_err_count_++;
				}				
				break;
			case ICDElementSchema::FLOAT32_1750:
				if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
					dynamic_cast<TranslatableColumn<float>*>(columns_[col_ind_].get())->translated_data_ref_,
					*icd_ptr))
				{
					translation_failure = true; translation_err_count_++;
				}				
				break;
			case ICDElementSchema::FLOAT32_IEEE:
				if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
					dynamic_cast<TranslatableColumn<float>*>(columns_[col_ind_].get())->translated_data_ref_,
					*icd_ptr))
				{
					translation_failure = true; translation_err_count_++;
				}				
				break;
			case ICDElementSchema::FLOAT64_IEEE:
				if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
					dynamic_cast<TranslatableColumn<double>*>(columns_[col_ind_].get())->translated_data_ref_,
					*icd_ptr))
				{
					translation_failure = true; translation_err_count_++;
				}				
				break;
			case ICDElementSchema::FLOAT16:
				if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
					dynamic_cast<TranslatableColumn<float>*>(columns_[col_ind_].get())->translated_data_ref_,
					*icd_ptr))
				{
					translation_failure = true; translation_err_count_++;
				}				
				break;
			case ICDElementSchema::CAPS:
				if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
					dynamic_cast<TranslatableColumn<double>*>(columns_[col_ind_].get())->translated_data_ref_,
					*icd_ptr))
				{
					translation_failure = true; translation_err_count_++;
				}				
				break;
			case ICDElementSchema::FLOAT32_GPS:
				if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
					dynamic_cast<TranslatableColumn<float>*>(columns_[col_ind_].get())->translated_data_ref_,
					*icd_ptr))
				{
					translation_failure = true; translation_err_count_++;
				}				
				break;
			case ICDElementSchema::FLOAT64_GPS:
				if (!icd_translate_.TranslateArrayOfElement(columns_[col_ind_]->raw_data_ref_,
					dynamic_cast<TranslatableColumn<double>*>(columns_[col_ind_].get())->translated_data_ref_,
					*icd_ptr))
				{
					translation_failure = true; translation_err_count_++;
				}				
				break;
			}
		}

#ifdef DEBUG
#if DEBUG > 1
		if(translation_failure)
		{
			printf("\Column %s translation error\n", columns_[col_ind_]->name().c_str());
		}
#endif
#endif

		// Reset Column append count.
		columns_[col_ind_]->reset_append_count();
	}

	// Reset ridealong columns append count.
	for (int i = 0; i < ridealong_columns_.size(); i++)
		ridealong_columns_[i]->reset_translated_data_append_count();

	return translation_err_count_;
}

uint8_t Translatable1553Table::configure_parquet_context(std::filesystem::path& output_dir, 
	std::filesystem::path& base_name, bool is_multithreaded, uint8_t id)
{
	// Create the Parquet file output path.
	std::string parquet_file_name = "_" + name_ + ".parquet";
	std::filesystem::path parquet_path = output_dir / base_name;
	parquet_path += std::filesystem::path(parquet_file_name);
	parquet_file_name = parquet_path.string();
	//printf("Parquet path: %s\n", parquet_file_name.c_str());

	if (is_multithreaded)
	{
		/*
		If this code is being executed in multithreaded manner, then instead of
		creating a single output Parquet file to contain the translated values for a
		 given table, create a directory with the parquet_path created above and
		 write to files within that directory, indexed via the id argument.
		*/

		// If the path doesn't exist as a directory, create it.
		if (!std::filesystem::exists(parquet_path))
		{
			if (!std::filesystem::create_directory(parquet_path))
			{
				printf("Translatable1553Table::configure_parquet_context(): Failed to create directory %s\n",
					parquet_path.string().c_str());
				return 1;
			}
		}

		// Create the output parquet file name based on the parquet_path file name.
		parquet_path /= parquet_path.stem();
		char buff[20];
		sprintf(buff, "__%02hhu.parquet", id);
		std::string ext(buff);
		parquet_path += std::filesystem::path(ext);

#ifdef __WIN64
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
		limited to 260 characters due to Windows MAX_PATH setting. I verified this by
		running translate_1553_from_parquet.cpp on files which resulted in output paths
		below and above MAX_PATH. Output files with length less than MAX_PATH complete
		without issue and longer paths fail due to an Arrow IO problem in which filesystem
		says it can't find the path. 

		Per boost.org/doc/libs/1_58_0/libs/filesystem/doc/reference.html#long-path-warning
		one way to get around the MAX_PATH limitation is to use the path prefix that I
		fill in temp_path below. Inclusion of this prefix allow Arrow to process long
		path lengths without issue. 
		*/
		std::filesystem::path temp_path("\\\\?\\");
		temp_path += parquet_path;
		parquet_path = temp_path;
		// end test -- delete this block to remove test feature
#endif
		parquet_file_name = parquet_path.string();
	}

	// Create ParquetContext object.
	pq_ = std::make_unique<ParquetContext>(allocated_row_count_);

	// Add fields and corresponding memory locations to ParquetContext table. 
	// Loop over ridealong columns, then the other columns.
	bool is_ridealong = true;
	for (uint16_t i = 0; i < ridealong_columns_.size(); i++)
	{
#ifdef DEBUG
#if DEBUG > 2
		printf("Adding ridealong column field: %s\n", ridealong_columns_[i]->name().c_str());
#endif
#endif
		pq_->AddField(ridealong_columns_[i]->arrow_type(), ridealong_columns_[i]->name());
		if (set_parquet_context_memory_location(i, is_ridealong) == 1)
		{
			printf("Translatable1553Table::configure_parquet_context(): Failed to set memory location on col %s\n",
				ridealong_columns_[i]->name().c_str());
			return 1;
		}
	}

	is_ridealong = false;
	for (col_ind_ = 0; col_ind_ < column_count_; col_ind_++)
	{
#ifdef DEBUG
#if DEBUG > 2
		printf("Adding translatable column field: %s\n", columns_[col_ind_]->name().c_str());
#endif
#endif
		pq_->AddField(columns_[col_ind_]->arrow_type(), columns_[col_ind_]->name());
#ifdef DEBUG
#if DEBUG > 2
		printf("Setting translatable column ParquetContext mem location: %s\n", 
			columns_[col_ind_]->name().c_str());
#endif
#endif
		if (set_parquet_context_memory_location(col_ind_, is_ridealong) == 1)
		{
			printf("Translatable1553Table::configure_parquet_context(): Failed to set memory location on col %s\n",
				columns_[col_ind_]->name().c_str());
			return 1;
		}
	}
	
	// Open the Parquet file for writing. Set truncate (2nd arg) to true to
	// create new file and overwrite existing file, if present.
	if (pq_->OpenForWrite(parquet_file_name, true) == false)
	{
		printf("Translatable1553Table::configure_parquet_context(): ParquetContext::open_for_write() error for path %s\n",
			parquet_file_name.c_str());
		return 1;
	}

	return 0;
}


uint8_t Translatable1553Table::set_parquet_context_memory_location(uint16_t& col_ind, bool& is_ridealong_column)
{
	temp_name_ = columns_[col_ind]->icd_elem_ptr_->elem_name_;
	if (is_ridealong_column)
	{
		//std::string name = arrow_field->name();
		//std::shared_ptr<arrow::DataType> arrow_type = arrow_field->type();
		switch (ridealong_columns_[col_ind]->arrow_type()->id())
		{
		case arrow::Int64Type::type_id:
		{
			pq_->SetMemoryLocation<int64_t>(
				dynamic_cast<TranslatableColumn<int64_t>*>(ridealong_columns_[col_ind].get())->translated_data_ref_,
				ridealong_columns_[col_ind]->name());
			return 0;
		}
		default:
		{
			printf("Translatable1553Table::set_parquet_context_memory_location() [ridealong column]: No case for arrow type %s",
				ridealong_columns_[col_ind]->arrow_type()->name().c_str());
			return 1;
		}
		}
	}
	else
	{
		if (columns_[col_ind]->icd_elem_ptr_->is_bitlevel_)
		{
			// bit-level translation type
			switch (columns_[col_ind]->icd_elem_ptr_->schema_)
			{
			case ICDElementSchema::ASCII:
				pq_->SetMemoryLocation<int8_t>(
					dynamic_cast<TranslatableColumn<int8_t>*>(columns_[col_ind].get())->translated_data_ref_,
					temp_name_);
				break;
			case ICDElementSchema::SIGNEDBITS:
				if (columns_[col_ind]->icd_elem_ptr_->bit_count_ < 25)
				{
					pq_->SetMemoryLocation<float>(
						dynamic_cast<TranslatableColumn<float>*>(columns_[col_ind].get())->translated_data_ref_,
						temp_name_);
				}
				else
				{
					pq_->SetMemoryLocation<double>(
						dynamic_cast<TranslatableColumn<double>*>(columns_[col_ind].get())->translated_data_ref_,
						temp_name_);
				}
				break;
			case ICDElementSchema::UNSIGNEDBITS:
				if (columns_[col_ind]->icd_elem_ptr_->bit_count_ == 1)
				{
					pq_->SetMemoryLocation<uint8_t>(
						dynamic_cast<TranslatableColumn<uint8_t>*>(columns_[col_ind].get())->translated_data_ref_,
						temp_name_);
				}
				else if (columns_[col_ind]->icd_elem_ptr_->bit_count_ < 25)
				{
					pq_->SetMemoryLocation<float>(
						dynamic_cast<TranslatableColumn<float>*>(columns_[col_ind].get())->translated_data_ref_,
						temp_name_);
				}
				else
				{
					pq_->SetMemoryLocation<double>(
						dynamic_cast<TranslatableColumn<double>*>(columns_[col_ind].get())->translated_data_ref_,
						temp_name_);
				}
				break;
			}
		}
		else
		{
			// word translation type
			switch (columns_[col_ind]->icd_elem_ptr_->schema_)
			{
			case ICDElementSchema::MULTIPLE_BIT:
				printf("Error: bitmsb is zero, should not have multiple bit type\n");
				return 1;
			case ICDElementSchema::MODE_CODE:
				printf("Error: MODE CODE -- no data to translate\n");
				return 1;
			case ICDElementSchema::SIGNED16:
				pq_->SetMemoryLocation<float>(
					dynamic_cast<TranslatableColumn<float>*>(columns_[col_ind].get())->translated_data_ref_,
					temp_name_);
				break;
			case ICDElementSchema::SIGNED32:
				pq_->SetMemoryLocation<double>(
					dynamic_cast<TranslatableColumn<double>*>(columns_[col_ind].get())->translated_data_ref_,
					temp_name_);
				break;
			case ICDElementSchema::UNSIGNED16:
				pq_->SetMemoryLocation<float>(
					dynamic_cast<TranslatableColumn<float>*>(columns_[col_ind].get())->translated_data_ref_,
					temp_name_);
				break;
			case ICDElementSchema::UNSIGNED32:
				pq_->SetMemoryLocation<double>(
					dynamic_cast<TranslatableColumn<double>*>(columns_[col_ind].get())->translated_data_ref_,
					temp_name_);
				break;
			case ICDElementSchema::FLOAT32_1750:
				pq_->SetMemoryLocation<float>(
					dynamic_cast<TranslatableColumn<float>*>(columns_[col_ind].get())->translated_data_ref_,
					temp_name_);
				break;
			case ICDElementSchema::FLOAT32_IEEE:
				pq_->SetMemoryLocation<float>(
					dynamic_cast<TranslatableColumn<float>*>(columns_[col_ind].get())->translated_data_ref_,
					temp_name_);
				break;
			case ICDElementSchema::FLOAT64_IEEE:
				pq_->SetMemoryLocation<double>(
					dynamic_cast<TranslatableColumn<double>*>(columns_[col_ind].get())->translated_data_ref_,
					temp_name_);
				break;
			case ICDElementSchema::FLOAT16:
				pq_->SetMemoryLocation<float>(
					dynamic_cast<TranslatableColumn<float>*>(columns_[col_ind].get())->translated_data_ref_,
					temp_name_);
				break;
			case ICDElementSchema::CAPS:
				pq_->SetMemoryLocation<double>(
					dynamic_cast<TranslatableColumn<double>*>(columns_[col_ind].get())->translated_data_ref_,
					temp_name_);
				break;
			case ICDElementSchema::FLOAT32_GPS:
				pq_->SetMemoryLocation<float>(
					dynamic_cast<TranslatableColumn<float>*>(columns_[col_ind].get())->translated_data_ref_,
					temp_name_);
				break;
			case ICDElementSchema::FLOAT64_GPS:
				pq_->SetMemoryLocation<double>(
					dynamic_cast<TranslatableColumn<double>*>(columns_[col_ind].get())->translated_data_ref_,
					temp_name_);
				break;
			default:
				printf("Translatable1553Table::set_parquet_context_memory_location(): Invalid schema = %hhu\n",
					static_cast<uint8_t>(columns_[col_ind]->icd_elem_ptr_->schema_));
				return 1;
			}
		}
	}
	return 0;
}

void Translatable1553Table::write_to_parquet_file(uint16_t& write_rows_count)
{
	pq_->WriteColumns(write_rows_count);
}

bool Translatable1553Table::is_bad()
{
	if (bad_value_ > 0)
		return true;
	else
		return false;
}
