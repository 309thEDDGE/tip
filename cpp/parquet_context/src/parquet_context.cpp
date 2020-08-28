#include "parquet_context.h"

ParquetContext::ParquetContext() : ROW_GROUP_COUNT_(10000),
have_created_table_(false), path_(""), have_created_writer_(false),
pool_(nullptr), schema_(nullptr), ret_(0),
append_row_count_(0), host_(""), user_(""), port_(-1),
have_created_schema_(false), writer_(nullptr), parquet_stop_(false),
truncate_(true)
{}

ParquetContext::ParquetContext(int rgSize) : ROW_GROUP_COUNT_(rgSize),
have_created_table_(false), path_(""), have_created_writer_(false),
pool_(nullptr), schema_(nullptr), ret_(0),
append_row_count_(0), host_(""), user_(""), port_(-1),
have_created_schema_(false), writer_(nullptr), parquet_stop_(false),
truncate_(true)
{}

ParquetContext::~ParquetContext()
{
	if (have_created_writer_)
	{
		writer_->Close();
		ostream_->Close();
	}
}

void ParquetContext::Close()
{
	if (have_created_writer_)
	{
		writer_->Close();
		ostream_->Close();
	}
}

std::vector<int32_t> 
ParquetContext::GetOffsetsVector(const int& n_rows, 
	const int& elements_per_row, 
	const int offset)
{
	std::vector<int32_t> offsets_vec(n_rows, 0);
	int _offset_ = offset * elements_per_row;
	for (int32_t i = 0; i < n_rows; i++)
	{
		offsets_vec[i] = i * elements_per_row + _offset_;
	}
	return offsets_vec;
}

uint8_t ParquetContext::OpenForWrite(const std::string path, const bool truncate)
{
	path_ = path;
	truncate_ = true;
	schema_ = arrow::schema(fields_);
	have_created_schema_ = true;
	CreateBuilders();
	return ret_;
}

std::unique_ptr<arrow::ArrayBuilder> 
ParquetContext::GetBuilderFromDataType(const std::shared_ptr<arrow::DataType> dtype,
	const bool& is_list_builder)
{
	switch (dtype->id())
	{
	case arrow::UInt64Type::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>
			(pool_, std::make_shared<arrow::UInt64Builder>(pool_));
		else
			return std::make_unique<arrow::UInt64Builder>(pool_);
	}
	case arrow::Int64Type::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>
			(pool_, std::make_shared<arrow::Int64Builder>(pool_));
		else
			return std::make_unique<arrow::Int64Builder>(pool_);
	}
	case arrow::Int32Type::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>
			(pool_, std::make_shared<arrow::Int32Builder>(pool_));
		else
			return std::make_unique<arrow::Int32Builder>(pool_);
	}
	case arrow::UInt32Type::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>
			(pool_, std::make_shared<arrow::UInt32Builder>(pool_));
		else
			return std::make_unique<arrow::UInt32Builder>(pool_);
	}
	case arrow::UInt16Type::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>
			(pool_, std::make_shared<arrow::UInt16Builder>(pool_));
		else
			return std::make_unique<arrow::UInt16Builder>(pool_);
	}
	case arrow::Int16Type::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>
			(pool_, std::make_shared<arrow::Int16Builder>(pool_));
		else
			return std::make_unique<arrow::Int16Builder>(pool_);
	}
	case arrow::UInt8Type::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>
			(pool_, std::make_shared<arrow::UInt8Builder>(pool_));
		else
			return std::make_unique<arrow::UInt8Builder>(pool_);
	}
	case arrow::Int8Type::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>
			(pool_, std::make_shared<arrow::Int8Builder>(pool_));
		else
			return std::make_unique<arrow::Int8Builder>(pool_);
	}
	case arrow::BooleanType::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>
			(pool_, std::make_shared<arrow::BooleanBuilder>(pool_));
		else
			return std::make_unique<arrow::BooleanBuilder>(pool_);
	}
	case arrow::DoubleType::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>
			(pool_, std::make_shared<arrow::DoubleBuilder>(pool_));
		else
			return std::make_unique<arrow::DoubleBuilder>(pool_);
	}
	case arrow::FloatType::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>
			(pool_, std::make_shared<arrow::FloatBuilder>(pool_));
		else
			return std::make_unique<arrow::FloatBuilder>(pool_);
	}
	case arrow::StringType::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>
			(pool_, std::make_shared<arrow::StringBuilder>(pool_));
		else
			return std::make_unique<arrow::StringBuilder>(pool_);
	}
	
	default:
		return std::make_unique<arrow::NullBuilder>(pool_);
	}
}

void ParquetContext::CreateBuilders()
{
	pool_ = arrow::default_memory_pool();
	ret_ = 0;
	arrow::NullType null_type;
	arrow::Type::type dtype = arrow::Type::NA;
	for (std::map<std::string, ColumnData>::iterator 
		it = column_data_map_.begin(); 
		it != column_data_map_.end(); 
		++it) 
	{
		std::string fieldName = it->second.field_name_;
		dtype = it->second.type_->id();

		it->second.builder_ = GetBuilderFromDataType(
			it->second.type_, it->second.is_list_);
	}
}

void ParquetContext::WriteColsIfReady()
{
	// Check if the table has been created. If not, then create it.
	if (!have_created_table_)
	{
		if (!have_created_writer_)
		{
#ifdef NEWARROW
			PARQUET_ASSIGN_OR_THROW(ostream_,
						arrow::io::FileOutputStream::Open(path_));
#else
			st_ = arrow::io::FileOutputStream::Open(path_, 
				!truncate_,
				&ostream_);

			if (!st_.ok())
			{
				printf("FileOutputStream::Open error (ID %s): %s\n", 
					st_.CodeAsString().c_str(), 
					st_.message().c_str());

				ret_ = 3;
				return;
			}
#endif

			// Create properties for the writer.
			parquet::WriterProperties::Builder props_builder;
			props_builder.compression(parquet::Compression::GZIP);
			props_builder.memory_pool(pool_);
			props_builder.enable_dictionary();

			// The only encoding that works
			props_builder.encoding(parquet::Encoding::PLAIN); 
			props_builder.enable_statistics();
			props_ = props_builder.build();

			st_ = parquet::arrow::FileWriter::Open(*schema_, pool_, 
				ostream_, 
				props_, 
				&writer_);

			if (!st_.ok())
			{
				printf("parquet::arrow::FileWriter::Open error (ID %s): %s\n", 
					st_.CodeAsString().c_str(), 
					st_.message().c_str());

				ret_ = 4;
				return;
			}
			have_created_writer_ = true;			
		}
		
#ifdef NEWARROW
		std::vector<std::shared_ptr<arrow::Array>> arr_vec;
#else
		arrow::ArrayVector arr_vec;
#endif
		// Loop over the builders and "Finish" them in order.
		for (int field_ind = 0; field_ind < schema_->num_fields(); field_ind++)
		{
			std::shared_ptr<arrow::Array> temp_array_ptr;

			st_ = column_data_map_[schema_->field(field_ind)->name()]
				.builder_->Finish(&temp_array_ptr);

			if (!st_.ok())
			{
				printf("\"Finish\" error (ID %s): %s\n", 
					st_.CodeAsString().c_str(), 
					st_.message().c_str());
				ret_ = 5;
				return;
			}
			arr_vec.push_back(temp_array_ptr);
		}

		// Make the Table and write it.
		std::shared_ptr<arrow::Table> table = arrow::Table::Make(schema_, arr_vec);
		st_ = writer_->WriteTable(*table, append_row_count_);
		if (!st_.ok())
		{
			printf("WriteTable error (ID %s): %s\n", 
				st_.CodeAsString().c_str(), 
				st_.message().c_str());

			ret_ = 6;
			return;
		}
		have_created_table_ = true;
		
		// Debug, check if table has metadata.
		if(table->schema()->HasMetadata())
		  printf("after being written, table has metadata\n");
#ifdef NEWARROW
		if(writer_->schema()->HasMetadata())
		  printf("after writetable, writer has metadata\n");
#endif
	}
	else
	{
		// Write columns in order.
		writer_->NewRowGroup(append_row_count_);
		for (int field_ind = 0; field_ind < schema_->num_fields(); field_ind++)
		{
			std::shared_ptr<arrow::Array> temp_array_ptr;
			st_ = column_data_map_[schema_->field(field_ind)->name()].
				builder_->Finish(&temp_array_ptr);

			if (!st_.ok())
			{
				printf("\"Finish\" error (ID %s): %s", 
					st_.CodeAsString().c_str(), 
					st_.message().c_str());

				ret_ = 7;
				return;
			}
			st_ = writer_->WriteColumnChunk(*temp_array_ptr);
			if (!st_.ok())
			{
				printf("WriteColumnChunk error (ID %s): %s", 
					st_.CodeAsString().c_str(), 
					st_.message().c_str());

				ret_ = 8;
				return;
			}
		}
	}
}

bool ParquetContext::AppendColumn(ColumnData& columnData, 
	const int& rows, 
	const int offset) 
{
	int datatypeID = columnData.type_->id(); 
	bool isList = columnData.is_list_;
	
	bool castRequired;

	if (columnData.cast_from_ == NONE)
		castRequired = false;
	else
		castRequired = true;
	
	int listCount = 0;

	if (isList) 
		listCount = columnData.list_size_;

	append_row_count_ = rows;

	if (rows + offset > columnData.initial_max_row_size_)
	{
		printf("Error!! initial vector does not"
			" contain enough information to write"
			" %d rows with a %d offset for column %s\n", 
			rows, offset, columnData.type_->name().c_str());
		return false;
	}

	switch (datatypeID)
	{

	case arrow::Int64Type::type_id:
	{
		Append<int64_t, arrow::NumericBuilder<arrow::Int64Type>>(isList,
			castRequired,
			listCount,
			offset,
			columnData);
		break;
	}

	case arrow::UInt64Type::type_id:
	{
		Append<uint64_t, arrow::NumericBuilder<arrow::UInt64Type>>(isList, 
			castRequired, 
			listCount, 
			offset,
			columnData);
		break;
	}
	
	case arrow::Int32Type::type_id:
	{
		Append<int32_t, arrow::NumericBuilder<arrow::Int32Type>>(isList,
			castRequired,
			listCount,
			offset,
			columnData);
		break;
	}

	case arrow::UInt32Type::type_id:
	{
		Append<uint32_t, arrow::NumericBuilder<arrow::UInt32Type>>(isList,
			castRequired,
			listCount,
			offset,
			columnData);
		break;
	}
	
	case arrow::Int16Type::type_id:
	{
		Append<int16_t, arrow::NumericBuilder<arrow::Int16Type>>(isList,
			castRequired,
			listCount,
			offset,
			columnData);
		break;
	}

	case arrow::UInt16Type::type_id:
	{
		Append<uint16_t, arrow::NumericBuilder<arrow::UInt16Type>>(isList,
			castRequired,
			listCount,
			offset,
			columnData);
		break;
	}

	case arrow::Int8Type::type_id:
	{
		Append<int8_t, arrow::NumericBuilder<arrow::Int8Type>>(isList,
			castRequired,
			listCount,
			offset,
			columnData);
		break;
	}

	case arrow::UInt8Type::type_id:
	{
		Append<uint8_t, arrow::NumericBuilder<arrow::UInt8Type>>(isList,
			castRequired,
			listCount,
			offset,
			columnData);
		break;
	}

	case arrow::DoubleType::type_id:
	{
		castRequired = false;
		Append<double, arrow::NumericBuilder<arrow::DoubleType>>(isList,
			castRequired,
			listCount,
			offset,
			columnData);
		break;
	}

	case arrow::FloatType::type_id:
	{
		castRequired = false;
		Append<float, arrow::NumericBuilder<arrow::FloatType>>(isList,
			castRequired,
			listCount,
			offset,
			columnData);
		break;
	}

	case arrow::BooleanType::type_id:
	{
		castRequired = false;
		Append<uint8_t, arrow::BooleanBuilder>(isList,
			castRequired,
			listCount,
			offset,
			columnData);
		break;
	}

	case arrow::StringType::type_id:
	{
		if (isList) 
		{
			// Get the relevant builder for the data type.
			std::shared_ptr<arrow::ListBuilder> bldr =
				std::dynamic_pointer_cast<arrow::ListBuilder>(columnData.builder_);

			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);

			std::vector<int32_t> offsets_vec = 
				GetOffsetsVector(append_row_count_, listCount, 0);

			bldr->AppendValues(offsets_vec.data(), append_row_count_);
			arrow::StringBuilder* sub_bldr =
				static_cast<arrow::StringBuilder*>(bldr->value_builder());

			// The only way to convey to Arrow the row count for the
			// StringBuilder class is via the implicit vector size.
			//
			// If the desired append row count is not equivalent
			// to initial_max_row_size_, a new string vector is built
			// with a size consistent with the desired row count.
			// For lists initial_max_row_size_ = initial vector size / list size
			// For non list columns initial_max_row_size_ = initial vector size
			if (append_row_count_ == columnData.initial_max_row_size_)
			{
				sub_bldr->AppendValues(*columnData.str_ptr_);
			}
			else
			{
				FillStringVec(columnData.str_ptr_,
					append_row_count_* listCount,
					offset* listCount);

				sub_bldr->AppendValues(temp_string_vec_);
			}
		}
		else 
		{
			std::shared_ptr<arrow::StringBuilder> bldr = 
				std::dynamic_pointer_cast<arrow::StringBuilder>(columnData.builder_);

			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);

			if (append_row_count_ == columnData.initial_max_row_size_)
			{
				if (columnData.null_values_ == nullptr)
				{
					bldr->AppendValues(*columnData.str_ptr_);
				}
				else
				{
					bldr->AppendValues(*columnData.str_ptr_, columnData.null_values_->data() + offset);
				}
			}
			else
			{
				FillStringVec(columnData.str_ptr_, append_row_count_, offset);
				if (columnData.null_values_ == nullptr)
				{
					bldr->AppendValues(temp_string_vec_);
				}
				else
				{
					bldr->AppendValues(temp_string_vec_, columnData.null_values_->data() + offset);
				}
			}			
		}
		break;
	}
	default:
		printf("Error!! Data type not included: %s\n", columnData.type_->name().c_str());
		return false;
		break;
	}

	// Make note that this column has been updated by setting the relevant bool.
	columnData.ready_for_write_ = true;

	return true;

}


void ParquetContext::AddField(const std::shared_ptr<arrow::DataType> type, 
	const std::string& fieldName, 
	const int listSize)
{
	//printf("Add field %s\n", fieldName.c_str());
	bool isList;
	if (listSize == NULL)
		isList = false;
	else
		isList = true;

	// Spark can't read unsigned data types from a parquet file
	if (IsUnsigned(type)) 
	{
#ifdef DEBUG
#if DEBUG > 1
		printf("Warning!!!!!!!!!!!!  Unsigned types are currently"
				"not available for writing parquet");
#endif
#endif
		return;
		
	}
	int byteSize;
	std::string typeID = GetTypeIDFromArrowType(type, byteSize);
	column_data_map_[fieldName] = ColumnData(type, 
		fieldName, 
		typeID, 
		byteSize, 
		listSize);

	std::shared_ptr<arrow::Field> tempField;

	if (isList) 
	{
		tempField = arrow::field(fieldName, arrow::list(type));
		fields_.push_back(tempField);
	}
	else 
	{
		tempField = arrow::field(fieldName, type);
		fields_.push_back(tempField);
	}
}

bool ParquetContext::IsUnsigned(const std::shared_ptr<arrow::DataType> type) 
{
	if (type->id() == arrow::UInt64Type::type_id || 
		type->id() == arrow::UInt32Type::type_id || 
		type->id() == arrow::UInt16Type::type_id || 
		type->id() == arrow::UInt8Type::type_id)
		return true;
	else
		return false;
}

bool ParquetContext::WriteColumns(const int& rows,const int offset)
{
	
	for (std::map<std::string, ColumnData>::iterator 
		it = column_data_map_.begin(); 
		it != column_data_map_.end();
		++it) 
	{
		if (!it->second.pointer_set_) 
		{
#ifdef DEBUG
#if DEBUG > 1
			printf("Warning!!!!!!, memory location for field not set: %s \n", 
				it->second.field_name_);
#endif
#endif
			return false;
		}
	}
	if (parquet_stop_) 
	{
#ifdef DEBUG
#if DEBUG > 1
		printf("Warning!!!!!!, parquetStop = true\n");
#endif
#endif
		return false;
	}

	append_row_count_ = rows;
	bool ret_val = true;

	try
	{
		if (rows > 0)
		{
			for (std::map<std::string, ColumnData>::iterator
				it = column_data_map_.begin();
				it != column_data_map_.end();
				++it)
			{
				ret_val = AppendColumn(it->second, rows, offset);
				if (!ret_val)
					return false;
			}
		}

		WriteColsIfReady();

		// Reset the status of each column.
		for (std::map<std::string, ColumnData>::iterator
			it = column_data_map_.begin();
			it != column_data_map_.end();
			++it)
		{
			it->second.ready_for_write_ = false;
		}
		return true;
	}
	
	catch (...)
	{
		return false;
	}
	
}

bool ParquetContext::WriteColumns()
{
	return WriteColumns(ROW_GROUP_COUNT_, 0);
}


std::string ParquetContext::GetTypeIDFromArrowType(const std::shared_ptr<arrow::DataType> type, 
	int& byteSize)
{
	switch (type->id())
	{
	case arrow::UInt64Type::type_id:
	{
		uint64_t a;
		byteSize = sizeof(a);
		return typeid(a).name();
		break;

	}
	case arrow::Int64Type::type_id:
	{
		int64_t a;
		byteSize = sizeof(a);
		return typeid(a).name();
		break;
	}
	case arrow::Int32Type::type_id:
	{
		int32_t a;
		byteSize = sizeof(a);
		return typeid(a).name();
		break;
	}
	case arrow::UInt32Type::type_id:
	{
		uint32_t a;
		byteSize = sizeof(a);
		return typeid(a).name();
		break;
	}
	case arrow::UInt16Type::type_id:
	{
		uint16_t a;
		byteSize = sizeof(a);
		return typeid(a).name();
		break;
	}
	case arrow::Int16Type::type_id:
	{
		int16_t a;
		byteSize = sizeof(a);
		return typeid(a).name();
		break;
	}
	case arrow::UInt8Type::type_id:
	{
		uint8_t a;
		byteSize = sizeof(a);
		return typeid(a).name();
		break;
	}
	case arrow::Int8Type::type_id:
	{
		int8_t a;
		byteSize = sizeof(a);
		return typeid(a).name();
		break;
	}
	// Note that boolean types need to come in
	// as uint8_t
	case arrow::BooleanType::type_id:
	{
		uint8_t a;
		byteSize = sizeof(a);
		return typeid(a).name();
		break;
	}
	case arrow::DoubleType::type_id:
	{
		double a;
		byteSize = sizeof(a);
		return typeid(a).name();
		break;
	}
	case arrow::FloatType::type_id:
	{
		float a;
		byteSize = sizeof(a);
		return typeid(a).name();
		break;
	}
	case arrow::StringType::type_id:
	{
		std::string a;
		byteSize = sizeof(a);
		return typeid(a).name();
		break;
	}

	default:
		printf("Data type not included in add field: \n");
		return "NA";
		break;
	}
}

void ParquetContext::FillStringVec(std::vector<std::string>* str_data_vec_ptr, 
	const int& count, 
	const int offset)
{
	if (temp_string_vec_.size() != count)
		temp_string_vec_.resize(count);

	std::copy((*str_data_vec_ptr).data() + offset, 
		(*str_data_vec_ptr).data() + offset + count, 
		temp_string_vec_.data());
}

