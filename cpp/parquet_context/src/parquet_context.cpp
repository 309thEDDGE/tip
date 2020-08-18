#include "parquet_context.h"

template bool ParquetContext::setMemoryLocation<std::string>(std::vector<std::string>&, std::string, uint8_t* boolField);
template bool ParquetContext::setMemoryLocation<double>(std::vector<double>&, std::string, uint8_t* boolField);
template bool ParquetContext::setMemoryLocation<float>(std::vector<float>&, std::string, uint8_t* boolField);
template bool ParquetContext::setMemoryLocation<int8_t>(std::vector<int8_t>&, std::string, uint8_t* boolField);
template bool ParquetContext::setMemoryLocation<uint8_t>(std::vector<uint8_t>&, std::string, uint8_t* boolField);
template bool ParquetContext::setMemoryLocation<int16_t>(std::vector<int16_t>&, std::string, uint8_t* boolField);
template bool ParquetContext::setMemoryLocation<uint16_t>(std::vector<uint16_t>&, std::string, uint8_t* boolField);
template bool ParquetContext::setMemoryLocation<int32_t>(std::vector<int32_t>&, std::string, uint8_t* boolField);
template bool ParquetContext::setMemoryLocation<uint32_t>(std::vector<uint32_t>&, std::string, uint8_t* boolField);
template bool ParquetContext::setMemoryLocation<int64_t>(std::vector <int64_t>&, std::string, uint8_t* boolField);
template bool ParquetContext::setMemoryLocation<uint64_t>(std::vector<uint64_t>&, std::string, uint8_t* boolField);
//template bool ParquetContext::setMemoryLocation<bool>(std::vector<bool>&, std::string, uint8_t* boolField);


template void ParquetContext::castTo<int8_t>(void*, std::vector<int8_t>&, std::string, int, int);
template void ParquetContext::castTo<uint8_t>(void*, std::vector<uint8_t>&, std::string, int, int);
template void ParquetContext::castTo<int16_t>(void*, std::vector<int16_t>&, std::string, int, int);
template void ParquetContext::castTo<uint16_t>(void*, std::vector<uint16_t>&, std::string, int, int);
template void ParquetContext::castTo<int32_t>(void*, std::vector<int32_t>&, std::string, int, int);
template void ParquetContext::castTo<uint32_t>(void*, std::vector<uint32_t>&, std::string, int, int);
template void ParquetContext::castTo<int64_t>(void*, std::vector<int64_t>&, std::string, int, int);
template void ParquetContext::castTo<uint64_t>(void*, std::vector<uint64_t>&, std::string, int, int);

ParquetContext::ParquetContext() : ROW_GROUP_COUNT_(10000),
have_created_table_(false), path_(""), have_created_writer_(false),
pool_(nullptr), schema_(nullptr), have_created_reader_(false), ret_(UINT8_MAX),
append_row_count_(0), is_hdfs_output_(false), host_(""), user_(""), port_(-1),
writeonly_(false), have_created_schema_(false), reader_(nullptr), writer_(nullptr), parquetStop(false),
truncate_(true), readwrite_(false)
{}

ParquetContext::ParquetContext(int rgSize) : ROW_GROUP_COUNT_(rgSize),
have_created_table_(false), path_(""), have_created_writer_(false),
pool_(nullptr), schema_(nullptr), have_created_reader_(false), ret_(UINT8_MAX),
append_row_count_(0), is_hdfs_output_(false), host_(""), user_(""), port_(-1),
writeonly_(false), have_created_schema_(false), reader_(nullptr), writer_(nullptr), parquetStop(false),
truncate_(true), readwrite_(false)
{}

ParquetContext::~ParquetContext()
{
	//printf("(destructor)\n");
	if (have_created_writer_)
	{
		writer_->Close();
		ostream_->Close();
	}
	if (have_created_reader_)
	{
		input_file_->Close();
	}
}

void ParquetContext::Close()
{
	//printf("(destructor)\n");
	if (have_created_writer_)
	{
		writer_->Close();
		ostream_->Close();
	}
	if (have_created_reader_)
	{
		input_file_->Close();
	}
}

std::vector<int32_t> ParquetContext::get_offsets_vector(int64_t& n_rows, int64_t& elements_per_row, int offset=0)
{
	std::vector<int32_t> offsets_vec(n_rows, 0);
	int _offset_ = offset * elements_per_row;
	for (int32_t i = 0; i < n_rows; i++)
	{
		offsets_vec[i] = i * elements_per_row + _offset_;
	}
	return offsets_vec;
}

uint8_t ParquetContext::open_for_write(std::string& path, bool truncate)
{
	path_ = path;
	truncate_ = truncate;
	create_schema();
	schema_ = arrow::schema(fields);
	have_created_schema_ = true;
	create_builders();
	writeonly_ = true;
	return ret_;
}

uint8_t ParquetContext::open_for_hdfs_write(std::string& hdfs_path, std::string& host,
	int& port, std::string& user)
{
	path_ = hdfs_path;
	host_ = host;
	port_ = port;
	user_ = user;
	is_hdfs_output_ = true;
	create_schema();
	have_created_schema_ = true;
	create_builders();
	writeonly_ = true;

	// HDFS writing does not work. Return 1.
	ret_ = 1;
	return ret_;
}

uint8_t ParquetContext::open_for_read(std::string& path, bool readwrite)
{
	path_ = path;
	readwrite_ = readwrite;
	writeonly_ = false;
	return ret_;
}

void ParquetContext::create_reader()
{
	if (!have_created_reader_)
	{
		// Note: readwrite_ = true not handled. 
		ret_ = 0;
#ifdef NEWARROW
		PARQUET_ASSIGN_OR_THROW(input_file_,
					arrow::io::ReadableFile::Open(path_, pool_));
#else
		st_ = arrow::io::ReadableFile::Open(path_, pool_, &input_file_);
		if (!st_.ok())
		{
			printf("arrow::io::ReadableFile::Open error (ID %s): %s\n", st_.CodeAsString().c_str(), st_.message().c_str());
			ret_ = 3;
			return;
		}
#endif
		st_ = parquet::arrow::OpenFile(input_file_, pool_, &reader_);
		if (!st_.ok())
		{
			printf("parquet::arrrow::OpenFile error (ID %s): %s\n", st_.CodeAsString().c_str(), st_.message().c_str());
			ret_ = 3;
			return;
		}
		have_created_reader_ = true;
	}
}

void ParquetContext::create_schema()
{
}

void ParquetContext::print_schema()
{
	if (writeonly_)
	{
		if (have_created_schema_)
			printf("%s\n", schema_->ToString().c_str());
		else
			printf("Have not yet created schema\n");
	}
	else
	{
		create_reader();
		if (ret_ != 0)
			return;
		st_ = reader_->GetSchema(&schema_);
		if (!st_.ok())
		{
			printf("parquet::arrow::FileReader::GetSchema error (ID %s): %s\n", st_.CodeAsString().c_str(), st_.message().c_str());
			ret_ = 1;
		}
		else
		{
			printf("%s\n", schema_->ToString().c_str());
		}
	}
}

void ParquetContext::print_metadata()
{
	if (writeonly_)
	{
		if (have_created_schema_)
		{
			if(schema_->HasMetadata())
				printf("%s\n", schema_->metadata()->ToString().c_str());

			// Below doesn't work because the field-level metadata was broken at the time
			// of this Arrow build.
			/*std::shared_ptr<arrow::Field> field;
			for (int i = 0; i < schema_->num_fields(); i++)
			{
				field = schema_->field(i);
				if (field->HasMetadata())
					printf("%s:\n%s\n", field->name().c_str(), field->metadata()->ToString().c_str());
				else
					printf("%s: NONE\n", field->name().c_str());
			}*/
		}
		else
			printf("Have not yet created schema\n");
	}
	else
	{
		create_reader();
		if (ret_ != 0)
			return;
		st_ = reader_->GetSchema(&schema_);
		if (!st_.ok())
		{
			printf("parquet::arrow::FileReader::GetSchema error (ID %s): %s\n", st_.CodeAsString().c_str(), st_.message().c_str());
			ret_ = 1;
		}
		else
		{
			if (schema_->HasMetadata())
				printf("%s\n", schema_->metadata()->ToString().c_str());

			// Below doesn't work because the field-level metadata was broken at the time
			// of this Arrow build.
			/*std::shared_ptr<arrow::Field> field;
			for (int i = 0; i < schema_->num_fields(); i++)
			{
				field = schema_->field(i);
				if (field->HasMetadata())
					printf("%s:\n%s\n", field->name().c_str(), field->metadata()->ToString().c_str());
				else
					printf("%s: NONE\n", field->name().c_str());
			}*/
		}
		/*std::shared_ptr<arrow::Table> table;
		st_ = reader_->RowGroup(0)->ReadTable(&table);
		std::shared_ptr<arrow::Schema> schema = table->schema();
		printf("table rows: %lld\n", table->num_rows());
		printf("%s\n", schema->metadata()->ToString().c_str());
		if (schema->GetFieldByName("etime")->HasMetadata())
			printf("schema etime has md\n");
		else
			printf("schema etime DOES NOT HAVE md\n");*/
	}
}

std::unordered_map<std::string, std::string> ParquetContext::get_metadata()
{
	std::unordered_map<std::string, std::string> kvmd;
	if (writeonly_)
	{
		if (have_created_schema_)
		{
			if (schema_->HasMetadata())
			{
				schema_->metadata()->ToUnorderedMap(&kvmd);
				return kvmd;
			}
		}
		else
			printf("Have not yet created schema\n");
	}
	else
	{
		create_reader();
		if (ret_ != 0)
			return kvmd;
#ifdef NEWARROW
		// Both methods should work for obtaining schema with metadata, though neither do
		// at the moment. This method may not work for old versions of arrow (~0.14).
		std::shared_ptr<arrow::Table> table;
		st_ = reader_->ReadTable(&table);
		if (!st_.ok())
		{
			printf("parquet::arrow::FileReader::ReadTable error (ID %s): %s\n", 
			       st_.CodeAsString().c_str(), st_.message().c_str());
			ret_ = 1;
			return kvmd;
		}
		schema_ = table->schema();
#else
		st_ = reader_->GetSchema(&schema_);
		if (!st_.ok())
		{
			printf("parquet::arrow::FileReader::GetSchema error (ID %s): %s\n", 
			       st_.CodeAsString().c_str(), st_.message().c_str());
			ret_ = 1;
			return kvmd;
		}
#endif
		
		printf("reading metadata ...\n");
		//printf("schema to string: %s\n", schema_->ToString(true).c_str());
		//printf("metadata size: %llu\n", schema_->metadata()->size());
		if (schema_->HasMetadata())
		  {
		    printf("schema has metadata, size: %llu\n", schema_->metadata()->size());
		    schema_->metadata()->ToUnorderedMap(&kvmd);
		    return kvmd;
		  }
		
	}
	return kvmd;
}

std::unique_ptr<arrow::ArrayBuilder> ParquetContext::get_builder_from_data_type(std::shared_ptr<arrow::DataType> dtype,
	bool is_list_builder)
{
	switch (dtype->id())
	{
	case arrow::UInt64Type::type_id:
		{
			if (is_list_builder)
				return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::UInt64Builder>(pool_));
			else
				return std::make_unique<arrow::UInt64Builder>(pool_);
		}
	case arrow::Int64Type::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::Int64Builder>(pool_));
		else
			return std::make_unique<arrow::Int64Builder>(pool_);
	}
	case arrow::Int32Type::type_id:
		{
			if (is_list_builder)
				return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::Int32Builder>(pool_));
			else
				return std::make_unique<arrow::Int32Builder>(pool_);
		}
	case arrow::UInt32Type::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::UInt32Builder>(pool_));
		else
			return std::make_unique<arrow::UInt32Builder>(pool_);
	}
	case arrow::UInt16Type::type_id:
		{
			if (is_list_builder)
				return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::UInt16Builder>(pool_));
			return std::make_unique<arrow::UInt16Builder>(pool_);
		}
	case arrow::Int16Type::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::Int16Builder>(pool_));
		return std::make_unique<arrow::Int16Builder>(pool_);
	}
	case arrow::UInt8Type::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::UInt8Builder>(pool_));
		return std::make_unique<arrow::UInt8Builder>(pool_);
	}
	case arrow::Int8Type::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::Int8Builder>(pool_));
		return std::make_unique<arrow::Int8Builder>(pool_);
	}
	case arrow::BooleanType::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::BooleanBuilder>(pool_));
		return std::make_unique<arrow::BooleanBuilder>(pool_);
	}
	case arrow::DoubleType::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::DoubleBuilder>(pool_));
		return std::make_unique<arrow::DoubleBuilder>(pool_);
	}
	case arrow::FloatType::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::FloatBuilder>(pool_));
		return std::make_unique<arrow::FloatBuilder>(pool_);
	}
	case arrow::StringType::type_id:
	{
		if (is_list_builder)
			return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::StringBuilder>(pool_));
		return std::make_unique<arrow::StringBuilder>(pool_);
	}
	
	default:
		return std::make_unique<arrow::NullBuilder>(pool_);
	}
}

void ParquetContext::create_builders()
{
	pool_ = arrow::default_memory_pool();
	ret_ = 0;
	arrow::NullType null_type;
	arrow::Type::type dtype = arrow::Type::NA;
	for (std::map<std::string, Field_Info>::iterator it = field_info_map_.begin(); it != field_info_map_.end(); ++it) {
		std::string fieldName = it->second.fieldName;
		dtype = it->second.type->id();
	
		/*builders_.push_back(get_builder_from_data_type(
			list_fields_dtype_[schema_->field(i)->name()], true));*/
		it->second.builder = get_builder_from_data_type(
			it->second.type, it->second.isList);
		/*printf("created builder for data type %s, field %s\n",
			it->second.type->name().c_str(),it->second.fieldName);*/
	}
}

/*uint8_t ParquetContext::check_col_name(std::string& check_name)
{
	for (std::map<std::string, bool>::iterator it = have_appended_col_map_.begin(); 
		it != have_appended_col_map_.end(); ++it)
	{
		if (it->first == check_name)
			return uint8_t(0);
	}
	return uint8_t(1);
}

uint8_t ParquetContext::check_field_name(std::string& check_name)
{
	for (std::map<std::string, bool>::iterator it = have_appended_col_map_.begin();
		it != have_appended_col_map_.end(); ++it)
	{
		if (it->first == check_name)
			return uint8_t(0);
	}
	return uint8_t(1);
}

template<typename NativeType, typename ArrowType>
uint8_t ParquetContext::append_to_column(std::vector<NativeType>& new_data, std::string col_name)
{
	ret_ = 0;

	// Check for valid column name. 
	if (check_col_name(col_name) == uint8_t(1))
	{
		ret_ = 1;
		printf("no name match\n");
		return ret_;
	}

	// Check if the field with name 'col_name' has the correct data type.
	ArrowType temp_type;




	if (schema_->GetFieldByName(col_name)->type()->id() != temp_type.id())
	{
		ret_ = 2;
		printf("schema/type mismatch: field says type is %s\n", schema_->GetFieldByName(col_name)->type()->name().c_str());
		return ret_;
	}
	//printf("append_to_column name %s\n", col_name.c_str());

	// Get the relevant builder for the data type.
	append_row_count_ = int64_t(new_data.size());


    //pool_ = arrow::default_memory_pool();
	//std:unique_ptr<arrow::ArrayBuilder> bldr = builders_map_[col_name];
	//arrow::ArrayBuilder* bldr = new arrow::NumericBuilder<arrow::FloatType>(pool_);
	//std::make_unique<arrow::DoubleBuilder>(pool_);
	//std::make_unique<arrow::NullBuilder>(pool_);


	std::shared_ptr<arrow::NumericBuilder<ArrowType>> bldr =
		std::dynamic_pointer_cast<arrow::NumericBuilder<ArrowType>>(builders_map_[col_name]);

	// Resize array to allocate space and append data.
	bldr->Resize(append_row_count_);
	bldr->AppendValues(new_data);

	// Make note that this column has been updated by setting the relevant bool.
	have_appended_col_map_[col_name] = true;

	// Write data if all columns have been updated.
	write_cols_if_ready();

	return ret_;
}

template<typename NativeType, typename ArrowType>
uint8_t ParquetContext::append_to_column(NativeType* new_data, int64_t size, 
	std::string col_name, uint8_t* valid)
{
	ret_ = 0;

	// Check for valid column name. 
	if (check_col_name(col_name) == uint8_t(1))
	{
		ret_ = 1;
		printf("no name match\n");
		return ret_;
	}

	// Check if the field with name 'col_name' has the correct data type.
	ArrowType temp_type;
	if (schema_->GetFieldByName(col_name)->type()->id() != temp_type.id())
	{
		ret_ = 2;
		printf("schema/type mismatch: field says type is %s\n", schema_->GetFieldByName(col_name)->type()->name().c_str());
		return ret_;
	}
	//printf("append_to_column name %s\n", col_name.c_str());

	// Get the relevant builder for the data type.
	append_row_count_ = size;
	std::shared_ptr<arrow::NumericBuilder<ArrowType>> bldr =
		std::dynamic_pointer_cast<arrow::NumericBuilder<ArrowType>>(builders_map_[col_name]);
	// Resize array to allocate space and append data.
	bldr->Resize(append_row_count_);
	if(valid == nullptr)
		bldr->AppendValues(new_data, append_row_count_);
	else
		bldr->AppendValues(new_data, append_row_count_, valid);

	// Make note that this column has been updated by setting the relevant bool.
	have_appended_col_map_[col_name] = true;

	// Write data if all columns have been updated.
	write_cols_if_ready();

	return ret_;
}

uint8_t ParquetContext::append_to_bool_column(uint8_t* new_data, int64_t size, 
	std::string col_name, uint8_t* valid)
{
	ret_ = 0;

	// Check for valid column name. 
	if (check_col_name(col_name) == uint8_t(1))
	{
		ret_ = 1;
		printf("no name match\n");
		return ret_;
	}

	// Check if the field with name 'col_name' has the correct data type.
	if (schema_->GetFieldByName(col_name)->type()->id() != arrow::Type::BOOL)
	{
		ret_ = 2;
		printf("schema/type mismatch: field says type is %s\n", schema_->GetFieldByName(col_name)->type()->name().c_str());
		return ret_;
	}
	//printf("append_to_column name %s\n", col_name.c_str());

	// Get the relevant builder for the data type.
	append_row_count_ = size;
	std::shared_ptr<arrow::BooleanBuilder> bldr =
		std::dynamic_pointer_cast<arrow::BooleanBuilder>(builders_map_[col_name]);
	// Resize array to allocate space and append data.
	bldr->Resize(append_row_count_);
	if (valid == nullptr)
		bldr->AppendValues(new_data, append_row_count_);
	else
		bldr->AppendValues(new_data, append_row_count_, valid);

	// Make note that this column has been updated by setting the relevant bool.
	have_appended_col_map_[col_name] = true;

	// Write data if all columns have been updated.
	write_cols_if_ready();

	return ret_;
}

// In this function, the ArrowType is the data type placed in the list, not arrow::ListType.
template<typename NativeType, class ArrowType>
uint8_t ParquetContext::append_lists_to_column(std::vector<NativeType>& new_data, std::string col_name)
{
	ret_ = 0;

	// Check for valid column name. 
	if (check_col_name(col_name) == uint8_t(1))
	{
		ret_ = 1;
		printf("no name match\n");
		return ret_;
	}

	// Check if the field with name 'col_name' has the correct data type.
	if (schema_->GetFieldByName(col_name)->type()->id() != arrow::Type::LIST)
	{
		ret_ = 2;
		printf("schema/type mismatch: field says type is %s\n", schema_->GetFieldByName(col_name)->type()->name().c_str());
		return ret_;
	}
	//printf("append_lists_to_column name %s\n", col_name.c_str());
	// Get the relevant builder for the data type.
	std::shared_ptr<arrow::ListBuilder> bldr =
		std::dynamic_pointer_cast<arrow::ListBuilder>(builders_map_[col_name]);

	// Resize array to allocate space and append data.
	append_row_count_ = int64_t(new_data.size()/ list_fields_count_per_row_map_[col_name]);
	bldr->Resize(append_row_count_);

	// Offsets vector.
	std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, list_fields_count_per_row_map_[col_name]);
	bldr->AppendValues(offsets_vec.data(), append_row_count_);
	arrow::NumericBuilder<ArrowType>* sub_bldr =
		static_cast<arrow::NumericBuilder<ArrowType>*>(bldr->value_builder());
	sub_bldr->AppendValues(new_data.data(), append_row_count_ * list_fields_count_per_row_map_[col_name]);

	// Make note that this column has been updated by setting the relevant bool.
	have_appended_col_map_[col_name] = true;

	// Write data if all columns have been updated.
	write_cols_if_ready();

	return ret_;
}

template<typename NativeType, class ArrowType>
uint8_t ParquetContext::append_lists_to_column(NativeType* new_data, int64_t size, std::string col_name)
{
	ret_ = 0;

	// Check for valid column name. 
	if (check_col_name(col_name) == uint8_t(1))
	{
		ret_ = 1;
		printf("no name match\n");
		return ret_;
	}

	// Check if the field with name 'col_name' has the correct data type.
	if (schema_->GetFieldByName(col_name)->type()->id() != arrow::Type::LIST)
	{
		ret_ = 2;
		printf("schema/type mismatch: field says type is %s\n", schema_->GetFieldByName(col_name)->type()->name().c_str());
		return ret_;
	}
	//printf("append_lists_to_column name %s\n", col_name.c_str());
	// Get the relevant builder for the data type.
	std::shared_ptr<arrow::ListBuilder> bldr =
		std::dynamic_pointer_cast<arrow::ListBuilder>(builders_map_[col_name]);

	// Resize array to allocate space and append data.
	append_row_count_ = size;
	bldr->Resize(append_row_count_);

	// Offsets vector.
	std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, list_fields_count_per_row_map_[col_name]);
	bldr->AppendValues(offsets_vec.data(), append_row_count_);
	arrow::NumericBuilder<ArrowType>* sub_bldr =
		static_cast<arrow::NumericBuilder<ArrowType>*>(bldr->value_builder());
	sub_bldr->AppendValues(new_data, append_row_count_ * list_fields_count_per_row_map_[col_name]);

	// Make note that this column has been updated by setting the relevant bool.
	have_appended_col_map_[col_name] = true;

	// Write data if all columns have been updated.
	write_cols_if_ready();

	return ret_;
}
*/

void ParquetContext::write_cols_if_ready()
{
	
	//printf("all cols updated, ready to write them!\n");

	// Check if the table has been created. If not, then create it.
	if (!have_created_table_)
	{
		if (!have_created_writer_)
		{
			if (is_hdfs_output_)
			{
				//// Determine which HDFS lib.
				//arrow::io::HdfsDriver driver;
				//bool at_least_one_lib = false;
				//st_ = arrow::io::HaveLibHdfs();
				//if (!st_.ok())
				//	printf("HaveLibHdfs() (ID %s): %s\n", st_.CodeAsString().c_str(), st_.message().c_str());
				//else
				//{
				//	at_least_one_lib = true;
				//	driver = arrow::io::HdfsDriver::LIBHDFS;
				//}

				//st_ = arrow::io::HaveLibHdfs3();
				//if (!st_.ok())
				//	printf("HaveLibHdfs3() (ID %s): %s\n", st_.CodeAsString().c_str(), st_.message().c_str());
				//else
				//{
				//	at_least_one_lib = true;
				//	driver = arrow::io::HdfsDriver::LIBHDFS3;
				//}

				//if (!at_least_one_lib)
				//{
				//	printf("No LibHDFS drivers present!\n");
				//	ret_ = 9;
				//	return;
				//}

				//// Configure connection. 
				//arrow::io::HdfsConnectionConfig hdfs_config;
				//hdfs_config.host = host_;
				//hdfs_config.port = port_;
				//hdfs_config.user = user_;
				//hdfs_config.driver = driver;
				//hdfs_config.kerb_ticket = "";
				//std::unordered_map<std::string, std::string> extra_conf_map;
				//hdfs_config.extra_conf = extra_conf_map;
				//printf("after create connection config\n");

				//// Connect to hadoop filesystem.
				//std::shared_ptr<arrow::io::HadoopFileSystem> hdfs;
				//st_ = arrow::io::HadoopFileSystem::Connect(&hdfs_config, &hdfs);
				//if (!st_.ok())
				//{
				//	printf("HadoopFileSystem::Connect error (ID %s): %s\n", st_.CodeAsString().c_str(), st_.message().c_str());
				//	ret_ = 10;
				//	return;
				//}
				//printf("after connect to hdp fs\n");

				//// Open the output stream.
				//std::shared_ptr<arrow::io::HdfsOutputStream> ostream;
				//bool append = false;
				//int32_t buffer_size = 0;  // use default
				//int16_t replication = 0;  // use default
				//int64_t default_block_size = 0;  // use default
				//st_ = hdfs->OpenWritable(path_, append, buffer_size, replication, default_block_size,
				//	&ostream);
				//if (!st_.ok())
				//{
				//	printf("HadoopFileSystem::OpenWritable error (ID %s): %s\n", st_.CodeAsString().c_str(), st_.message().c_str());
				//	ret_ = 11;
				//	return;
				//}
				//printf("after open writable stream\n");

				//// Create the writer.
				//std::shared_ptr<parquet::WriterProperties> props = parquet::default_writer_properties();
				//st_ = parquet::arrow::FileWriter::Open(*schema_, pool_, ostream, props, &writer_);
				//if (!st_.ok())
				//{
				//	printf("parquet::arrow::FileWriter::Open error (ID %s): %s\n", st_.CodeAsString().c_str(), st_.message().c_str());
				//	ret_ = 12;
				//	return;
				//}
				//have_created_writer_ = true;
				//printf("after create writer\n");
			}
			else
			{
				// Output stream and FileWriter.
				//std::shared_ptr<arrow::io::FileOutputStream> ostream;
#ifdef NEWARROW
			  PARQUET_ASSIGN_OR_THROW(ostream_,
						  arrow::io::FileOutputStream::Open(path_));
#else
				st_ = arrow::io::FileOutputStream::Open(path_, &ostream_);
				if (!st_.ok())
				{
					printf("FileOutputStream::Open error (ID %s): %s\n", st_.CodeAsString().c_str(), st_.message().c_str());
					ret_ = 3;
					return;
				}
#endif
				//printf("after open out stream\n");

				// Create properties for the writer.
				//std::shared_ptr<parquet::WriterProperties> props = parquet::default_writer_properties();
				parquet::WriterProperties::Builder props_builder;
				props_builder.compression(parquet::Compression::GZIP);
				props_builder.memory_pool(pool_);
				props_builder.enable_dictionary();
				props_builder.encoding(parquet::Encoding::PLAIN); // the only encoding that works
				props_builder.enable_statistics();
				props_ = props_builder.build();

				// Write file metadata (experimental).
				// As far as I can tell, this code does nothing.
				/*std::shared_ptr<parquet::SchemaDescriptor> schema_desc_ptr;
				st_ = parquet::arrow::ToParquetSchema(schema_.get(), *props_, &schema_desc_ptr);
				std::unique_ptr<parquet::FileMetaDataBuilder> md_builder =
					parquet::FileMetaDataBuilder::Make(schema_desc_ptr.get(), props_, schema_->metadata());
				std::unique_ptr<parquet::FileMetaData> fmd = md_builder->Finish();
				st_ = parquet::arrow::WriteFileMetaData(*fmd, ostream_.get());
				if (!st_.ok())
				{
					printf("parquet::arrow::WriteFileMetaData error (ID %s): %s\n", st_.CodeAsString().c_str(), st_.message().c_str());
					ret_ = 4;
					return;
				}*/
				// End write file metadata.

				st_ = parquet::arrow::FileWriter::Open(*schema_, pool_, ostream_, props_, &writer_);
				if (!st_.ok())
				{
					printf("parquet::arrow::FileWriter::Open error (ID %s): %s\n", st_.CodeAsString().c_str(), st_.message().c_str());
					ret_ = 4;
					return;
				}
				have_created_writer_ = true;
			}
		}
		
#ifdef NEWARROW
		std::vector<std::shared_ptr<arrow::Array>> arr_vec;
#else
		arrow::ArrayVector arr_vec;
#endif
		// Loop over the builders and "Finish" them in order.
		for (int field_ind = 0; field_ind < schema_->num_fields(); field_ind++)
		{
			//printf("Finishing builder for field ind %d, %s\n", field_ind, schema_->field(field_ind)->name().c_str());
			std::shared_ptr<arrow::Array> temp_array_ptr;

			st_ = field_info_map_[schema_->field(field_ind)->name()].builder->Finish(&temp_array_ptr);
			if (!st_.ok())
			{
				printf("\"Finish\" error (ID %s): %s\n", st_.CodeAsString().c_str(), st_.message().c_str());
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
			printf("WriteTable error (ID %s): %s\n", st_.CodeAsString().c_str(), st_.message().c_str());
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
		//std::vector<std::shared_ptr<arrow::Array>> temp_array_vec;
		// Write columns in order.
		writer_->NewRowGroup(append_row_count_);
		for (int field_ind = 0; field_ind < schema_->num_fields(); field_ind++)
		{
			std::shared_ptr<arrow::Array> temp_array_ptr;
			//temp_array_vec.push_back(temp_array_ptr);
			st_ = field_info_map_[schema_->field(field_ind)->name()].builder->Finish(&temp_array_ptr);
			if (!st_.ok())
			{
				printf("\"Finish\" error (ID %s): %s", st_.CodeAsString().c_str(), st_.message().c_str());
				ret_ = 7;
				return;
			}
			//printf("Writing column name %s\n", schema_->field(field_ind)->name().c_str());
			st_ = writer_->WriteColumnChunk(*temp_array_ptr);
			if (!st_.ok())
			{
				printf("WriteColumnChunk error (ID %s): %s", st_.CodeAsString().c_str(), st_.message().c_str());
				ret_ = 8;
				return;
			}
		}
	}
}

// Replace metadata with the new key-value metadata contained in keyval_map.
uint8_t ParquetContext::set_metadata(std::unordered_map<std::string, std::string>& keyval_map)
{
	if (have_created_table_)
	{
		printf("Table already created. Metadata is fixed.\n");
		ret_ = 1;
		return ret_;
	}
	
	
#ifdef NEWARROW
	std::shared_ptr<arrow::KeyValueMetadata> kvmd = arrow::key_value_metadata(keyval_map);
	schema_ = schema_->WithMetadata(kvmd);
#else
	arrow::KeyValueMetadata md(keyval_map);
	schema_ = schema_->AddMetadata(md.Copy());
#endif
	printf("ParquetContext::set_metadata(): %llu key-val pairs\n", schema_->metadata()->size());
	ret_ = 0;
	return ret_;
}

uint8_t ParquetContext::set_field_metadata(std::unordered_map<std::string, std::string>& keyval_map,
	std::string& field_name)
{
	if (have_created_table_)
	{
		printf("Table already created. Metadata is fixed.\n");
		ret_ = 1;
		return ret_;
	}

	arrow::KeyValueMetadata md(keyval_map);
	std::shared_ptr<arrow::Field> field = schema_->GetFieldByName(field_name);
	if (field == NULL)
	{
		printf("There is no field with name \"%s\"\n", field_name.c_str());
		ret_ = 1;
		return ret_;
	}
#ifdef NEWARROW
	field = field->WithMetadata(md.Copy());
#else
	field = field->AddMetadata(md.Copy());
#endif
	st_ = schema_->SetField(schema_->GetFieldIndex(field_name), field, &schema_);
	if (!st_.ok())
	{
		printf("schema_->SetField error (ID %s): %s", st_.CodeAsString().c_str(), st_.message().c_str());
		ret_ = 1;
		return ret_;
	}

	ret_ = 0;
	return ret_;
}

template<typename castType> void ParquetContext::castTo(void* data, std::vector<castType>& buildVec, 
	std::string castFrom, int size, int offset) 
{
	if (castFrom == typeid(int8_t).name()) {
		for (int i = 0; i < size; i++) {
			buildVec[i] = (castType)*(((int8_t*)data) + i + offset);
		}
	}
	else if (castFrom == typeid(uint8_t).name()) {
		for (int i = 0; i < size; i++) {
			buildVec[i] = (castType)*(((uint8_t*)data) + i + offset);
		}
	}
	else if (castFrom == typeid(int16_t).name()) {
		for (int i = 0; i < size; i++) {
			buildVec[i] = (castType)*(((int16_t*)data) + i + offset);
		}
	}
	else if (castFrom == typeid(uint16_t).name()) {
		for (int i = 0; i < size; i++) {
			buildVec[i] = (castType)*(((uint16_t*)data) + i + offset);
		}
	}
	else if (castFrom == typeid(int32_t).name()) {
		for (int i = 0; i < size; i++) {
			buildVec[i] = (castType)*(((int32_t*)data) + i + offset);
		}
	}
	else if (castFrom == typeid(uint32_t).name()) {
		for (int i = 0; i < size; i++) {
			buildVec[i] = (castType)*(((uint32_t*)data) + i + offset);
		}
	}
	else if (castFrom == typeid(int64_t).name()) {
		for (int i = 0; i < size; i++) {
			buildVec[i] = (castType)*((int64_t*)data + i + offset);
		}
	}
	else if (castFrom == typeid(uint64_t).name()) {
		for (int i = 0; i < size; i++) {
			buildVec[i] = (castType)*(((uint64_t*)data) + i + offset);
		}
	}
	else if (castFrom == typeid(bool).name()) {
		for (int i = 0; i < size; i++) {
			buildVec[i] = (castType)*(((bool*)data) + i + offset);
		}
	}
}

void ParquetContext::append_column(Field_Info& fieldInfo, int rows, 
	bool explicit_row_count=false, int offset=0) 
{
	//std::string col_name = schema_->field(col_num)->name();  // Could pass in column number
	std::string col_name = fieldInfo.fieldName;
	//printf("ParquetContext::append_column(): %s\n", col_name.c_str());
	//int datatypeID = schema_->field(col_num)->type()->id();
	//std::string type = schema_->GetFieldByName(col_name)->type()->name().c_str();
	int datatypeID = fieldInfo.type->id(); // schema_->GetFieldByName(col_name)->type()->id();
	bool isList = fieldInfo.isList;

	//int datatypeID = schema_->GetFieldByName(col_name)->type()->id();
	
	bool found = true;
	bool castRequired;

	if (fieldInfo.castFrom == "")
		castRequired = false;
	else
		castRequired = true;
	
	int64_t listCount = 0;

	if (isList) {
		datatypeID = fieldInfo.type->id();
		listCount = fieldInfo.listSize;
	}

	

	// Check if the field with name 'col_name' has the correct data type.
	/*
	if (schema_->GetFieldByName(col_name)->type()->id() != temp_type.id())
	{
		ret_ = 2;
		printf("schema/type mismatch: field says type is %s\n", schema_->GetFieldByName(col_name)->type()->name().c_str());
		return ret_;
	}*/
	
	//printf("append_to_column name %s\n", col_name.c_str());

	// Get the relevant builder for the data type.
	append_row_count_ = rows;

	switch (datatypeID)
	{
	case arrow::UInt64Type::type_id:
	{
		if (isList) {
			// Get the relevant builder for the data type.
			std::shared_ptr<arrow::ListBuilder> bldr =
				std::dynamic_pointer_cast<arrow::ListBuilder>(fieldInfo.builder);

			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);

			// Offsets vector.
			
			if (castRequired) 
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, 0);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::NumericBuilder<arrow::UInt64Type>* sub_bldr =
					static_cast<arrow::NumericBuilder<arrow::UInt64Type>*>(bldr->value_builder());
				std::vector<uint64_t> castVec(append_row_count_ * listCount);
				castTo<uint64_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_ * listCount, offset * listCount);
				sub_bldr->AppendValues(castVec.data(), append_row_count_ * listCount);
			} 
			else
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, offset);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::NumericBuilder<arrow::UInt64Type>* sub_bldr =
					static_cast<arrow::NumericBuilder<arrow::UInt64Type>*>(bldr->value_builder());
				sub_bldr->AppendValues((uint64_t*)fieldInfo.data, append_row_count_ * listCount);
			}
		}
		else {
			std::shared_ptr<arrow::NumericBuilder<arrow::UInt64Type>> bldr = 
				std::dynamic_pointer_cast<arrow::NumericBuilder<arrow::UInt64Type>>(fieldInfo.builder);
			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);
			if (fieldInfo.nullValues == nullptr) {
				if (castRequired) {
					std::vector<uint64_t> castVec(append_row_count_);
					castTo<uint64_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_);
				}
				else
					bldr->AppendValues(((uint64_t*)fieldInfo.data)+offset, append_row_count_); 
			}
			else {
				if (castRequired) {
					std::vector<uint64_t> castVec(append_row_count_);
					castTo<uint64_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_, fieldInfo.nullValues);
				}
				else
					bldr->AppendValues(((uint64_t*)fieldInfo.data)+offset, append_row_count_, fieldInfo.nullValues);
			}
		}
		break;
	}
	case arrow::Int64Type::type_id:
	{
		if (isList) {
			// Get the relevant builder for the data type.
			std::shared_ptr<arrow::ListBuilder> bldr =
				std::dynamic_pointer_cast<arrow::ListBuilder>(fieldInfo.builder);

			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);

			// Offsets vector.
			
			if (castRequired) 
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, 0);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::NumericBuilder<arrow::Int64Type>* sub_bldr =
					static_cast<arrow::NumericBuilder<arrow::Int64Type>*>(bldr->value_builder());
				std::vector<int64_t> castVec(append_row_count_ * listCount);
				castTo<int64_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_ * listCount, offset* listCount);
				sub_bldr->AppendValues(castVec.data(), append_row_count_ * listCount);
			}
			else
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, offset);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::NumericBuilder<arrow::Int64Type>* sub_bldr =
					static_cast<arrow::NumericBuilder<arrow::Int64Type>*>(bldr->value_builder());
				sub_bldr->AppendValues((int64_t*)fieldInfo.data, append_row_count_ * listCount);
			}
		}
		else {
			std::shared_ptr<arrow::NumericBuilder<arrow::Int64Type>> bldr = std::dynamic_pointer_cast<arrow::NumericBuilder<arrow::Int64Type>>(fieldInfo.builder);
			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);
			if (fieldInfo.nullValues == nullptr) {
				if (castRequired) {
					std::vector<int64_t> castVec(append_row_count_);
					castTo<int64_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_);
				}
				else
					bldr->AppendValues(((int64_t*)fieldInfo.data)+offset, append_row_count_);
			}
			else {
				if (castRequired) {
					std::vector<int64_t> castVec(append_row_count_);
					castTo<int64_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_, fieldInfo.nullValues);
				}
				else
					bldr->AppendValues(((int64_t*)fieldInfo.data)+offset, append_row_count_, fieldInfo.nullValues);
			}
		}
		break;
	}
	case arrow::Int32Type::type_id:
	{
		if (isList) {
			// Get the relevant builder for the data type.
			std::shared_ptr<arrow::ListBuilder> bldr =
				std::dynamic_pointer_cast<arrow::ListBuilder>(fieldInfo.builder);

			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);

			// Offsets vector.
			if (castRequired) {
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, 0);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::NumericBuilder<arrow::Int32Type>* sub_bldr =
					static_cast<arrow::NumericBuilder<arrow::Int32Type>*>(bldr->value_builder());
				std::vector<int32_t> castVec(append_row_count_ * listCount);
				castTo<int32_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_ * listCount, offset * listCount);
				sub_bldr->AppendValues(castVec.data(), append_row_count_ * listCount);
			}
			else
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, offset);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::NumericBuilder<arrow::Int32Type>* sub_bldr =
					static_cast<arrow::NumericBuilder<arrow::Int32Type>*>(bldr->value_builder());
				sub_bldr->AppendValues((int32_t*)fieldInfo.data, append_row_count_ * listCount);
			}
		}
		else {
			std::shared_ptr<arrow::NumericBuilder<arrow::Int32Type>> bldr = std::dynamic_pointer_cast<arrow::NumericBuilder<arrow::Int32Type>>(fieldInfo.builder);
			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);
			if (fieldInfo.nullValues == nullptr) {
				if (castRequired) {
					std::vector<int32_t> castVec(append_row_count_);
					castTo<int32_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_);
				}
				else
					bldr->AppendValues(((int32_t*)fieldInfo.data)+offset, append_row_count_);
			}
			else {
				if (castRequired) {
					std::vector<int32_t> castVec(append_row_count_);
					castTo<int32_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_, fieldInfo.nullValues);
				}
				else
					bldr->AppendValues(((int32_t*)fieldInfo.data)+offset, append_row_count_, fieldInfo.nullValues);
			}
		}
		break;
	}
	case arrow::UInt32Type::type_id:
	{
		if (isList) {
			// Get the relevant builder for the data type.
			std::shared_ptr<arrow::ListBuilder> bldr =
				std::dynamic_pointer_cast<arrow::ListBuilder>(fieldInfo.builder);

			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);

			// Offsets vector.
			if (castRequired) 
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, 0);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::NumericBuilder<arrow::UInt32Type>* sub_bldr =
					static_cast<arrow::NumericBuilder<arrow::UInt32Type>*>(bldr->value_builder());
				std::vector<uint32_t> castVec(append_row_count_ * listCount);
				castTo<uint32_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_ * listCount, offset * listCount);
				sub_bldr->AppendValues(castVec.data(), append_row_count_ * listCount);
			}
			else
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, offset);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::NumericBuilder<arrow::UInt32Type>* sub_bldr =
					static_cast<arrow::NumericBuilder<arrow::UInt32Type>*>(bldr->value_builder());
				sub_bldr->AppendValues((uint32_t*)fieldInfo.data, append_row_count_ * listCount);
			}
		}
		else {
			std::shared_ptr<arrow::NumericBuilder<arrow::UInt32Type>> bldr = std::dynamic_pointer_cast<arrow::NumericBuilder<arrow::UInt32Type>>(fieldInfo.builder);
			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);
			if (fieldInfo.nullValues == nullptr) {
				if (castRequired) {
					std::vector<uint32_t> castVec(append_row_count_);
					castTo<uint32_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_);
				}
				else
					bldr->AppendValues(((uint32_t*)fieldInfo.data)+offset, append_row_count_);
			}
			else {
				if (castRequired) {
					std::vector<uint32_t> castVec(append_row_count_);
					castTo<uint32_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_, fieldInfo.nullValues);
				}
				else
					bldr->AppendValues(((uint32_t*)fieldInfo.data)+offset, append_row_count_, fieldInfo.nullValues);
			}
		}
		break;
	}
	case arrow::UInt16Type::type_id:
	{
		if (isList) {
			// Get the relevant builder for the data type.
			std::shared_ptr<arrow::ListBuilder> bldr =
				std::dynamic_pointer_cast<arrow::ListBuilder>(fieldInfo.builder);

			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);

			// Offsets vector.
			if (castRequired) 
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, 0);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::NumericBuilder<arrow::UInt16Type>* sub_bldr =
					static_cast<arrow::NumericBuilder<arrow::UInt16Type>*>(bldr->value_builder());
				std::vector<uint16_t> castVec(append_row_count_ * listCount);
				castTo<uint16_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_ * listCount, offset* listCount);
				sub_bldr->AppendValues(castVec.data(), append_row_count_ * listCount);
			}
			else
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, offset);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::NumericBuilder<arrow::UInt16Type>* sub_bldr =
					static_cast<arrow::NumericBuilder<arrow::UInt16Type>*>(bldr->value_builder());
				sub_bldr->AppendValues((uint16_t*)fieldInfo.data, append_row_count_ * listCount);
			}
		}
		else {
			std::shared_ptr<arrow::NumericBuilder<arrow::UInt16Type>> bldr = std::dynamic_pointer_cast<arrow::NumericBuilder<arrow::UInt16Type>>(fieldInfo.builder);
			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);
			if (fieldInfo.nullValues == nullptr) {
				if (castRequired) {
					std::vector<uint16_t> castVec(append_row_count_);
					castTo<uint16_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_);
				}
				else
					bldr->AppendValues(((uint16_t*)fieldInfo.data)+offset, append_row_count_);
			}
			else {
				if (castRequired) {
					std::vector<uint16_t> castVec(append_row_count_);
					castTo<uint16_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_, fieldInfo.nullValues);
				}
				else
					bldr->AppendValues(((uint16_t*)fieldInfo.data)+offset, append_row_count_, fieldInfo.nullValues);
			}
		}
		break;
	}
	case arrow::Int16Type::type_id:
	{
		if (isList) {
			// Get the relevant builder for the data type.
			std::shared_ptr<arrow::ListBuilder> bldr =
				std::dynamic_pointer_cast<arrow::ListBuilder>(fieldInfo.builder);

			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);

			// Offsets vector.
			if (castRequired) 
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, 0);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::NumericBuilder<arrow::Int16Type>* sub_bldr =
					static_cast<arrow::NumericBuilder<arrow::Int16Type>*>(bldr->value_builder());
				std::vector<int16_t> castVec(append_row_count_ * listCount);
				castTo<int16_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_ * listCount, offset* listCount);
				sub_bldr->AppendValues(castVec.data(), append_row_count_ * listCount);
			}
			else
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, offset);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::NumericBuilder<arrow::Int16Type>* sub_bldr =
					static_cast<arrow::NumericBuilder<arrow::Int16Type>*>(bldr->value_builder());
				sub_bldr->AppendValues((int16_t*)fieldInfo.data, append_row_count_ * listCount);
			}
		}
		else {
			std::shared_ptr<arrow::NumericBuilder<arrow::Int16Type>> bldr = std::dynamic_pointer_cast<arrow::NumericBuilder<arrow::Int16Type>>(fieldInfo.builder);
			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);
			if (fieldInfo.nullValues == nullptr) {
				if (castRequired) {
					std::vector<int16_t> castVec(append_row_count_);
					castTo<int16_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_);
				}
				else
					bldr->AppendValues(((int16_t*)fieldInfo.data)+offset, append_row_count_);
			}
			else {
				if (castRequired) {
					std::vector<int16_t> castVec(append_row_count_);
					castTo<int16_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_, fieldInfo.nullValues);
				}
				else
					bldr->AppendValues(((int16_t*)fieldInfo.data)+offset, append_row_count_, fieldInfo.nullValues);
			}
		}
		break;
	}
	case arrow::UInt8Type::type_id:
	{
		if (isList) {
			// Get the relevant builder for the data type.
			std::shared_ptr<arrow::ListBuilder> bldr =
				std::dynamic_pointer_cast<arrow::ListBuilder>(fieldInfo.builder);

			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);

			// Offsets vector.
			if (castRequired) 
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, 0);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::NumericBuilder<arrow::UInt8Type>* sub_bldr =
					static_cast<arrow::NumericBuilder<arrow::UInt8Type>*>(bldr->value_builder());
				std::vector<uint8_t> castVec(append_row_count_ * listCount);
				castTo<uint8_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_ * listCount, offset* listCount);
				sub_bldr->AppendValues(castVec.data(), append_row_count_ * listCount);
			}
			else
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, offset);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::NumericBuilder<arrow::UInt8Type>* sub_bldr =
					static_cast<arrow::NumericBuilder<arrow::UInt8Type>*>(bldr->value_builder());
				sub_bldr->AppendValues((uint8_t*)fieldInfo.data, append_row_count_ * listCount);
			}
		}
		else {
			std::shared_ptr<arrow::NumericBuilder<arrow::UInt8Type>> bldr = std::dynamic_pointer_cast<arrow::NumericBuilder<arrow::UInt8Type>>(fieldInfo.builder);
			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);
			if (fieldInfo.nullValues == nullptr) {
				if (castRequired) {
					std::vector<uint8_t> castVec(append_row_count_);
					castTo<uint8_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_);
				}
				else
					bldr->AppendValues(((uint8_t*)fieldInfo.data)+offset, append_row_count_);
			}
			else {
				if (castRequired) {
					std::vector<uint8_t> castVec(append_row_count_);
					castTo<uint8_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_, fieldInfo.nullValues);
				}
				else
					bldr->AppendValues(((uint8_t*)fieldInfo.data)+offset, append_row_count_, fieldInfo.nullValues);
			}
		}
		break;
	}
	case arrow::Int8Type::type_id:
	{
		if (isList) {
			// Get the relevant builder for the data type.
			std::shared_ptr<arrow::ListBuilder> bldr =
				std::dynamic_pointer_cast<arrow::ListBuilder>(fieldInfo.builder);

			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);

			// Offsets vector.
			if (castRequired) 
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, 0);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::NumericBuilder<arrow::Int8Type>* sub_bldr =
					static_cast<arrow::NumericBuilder<arrow::Int8Type>*>(bldr->value_builder());
				std::vector<int8_t> castVec(append_row_count_ * listCount);
				castTo<int8_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_ * listCount, offset * listCount);
				sub_bldr->AppendValues(castVec.data(), append_row_count_ * listCount);
			}
			else
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, offset);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::NumericBuilder<arrow::Int8Type>* sub_bldr =
					static_cast<arrow::NumericBuilder<arrow::Int8Type>*>(bldr->value_builder());
				sub_bldr->AppendValues((int8_t*)fieldInfo.data, append_row_count_ * listCount);
			}
		}
		else {
			std::shared_ptr<arrow::NumericBuilder<arrow::Int8Type>> bldr = std::dynamic_pointer_cast<arrow::NumericBuilder<arrow::Int8Type>>(fieldInfo.builder);
			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);
			if (fieldInfo.nullValues == nullptr) {
				if (castRequired) {
					std::vector<int8_t> castVec(append_row_count_);
					castTo<int8_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_);
				}
				else
					bldr->AppendValues(((int8_t*)fieldInfo.data)+offset, append_row_count_);
			}
			else {
				if (castRequired) {
					std::vector<int8_t> castVec(append_row_count_);
					castTo<int8_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_, fieldInfo.nullValues);
				}
				else
					bldr->AppendValues(((int8_t*)fieldInfo.data)+offset, append_row_count_, fieldInfo.nullValues);
			}
		}
		break;
	}
	case arrow::BooleanType::type_id:
	{
		if (isList) {
			// Get the relevant builder for the data type.
			std::shared_ptr<arrow::ListBuilder> bldr =
				std::dynamic_pointer_cast<arrow::ListBuilder>(fieldInfo.builder);

			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);

			// Offsets vector.
			if (castRequired) 
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, 0);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::BooleanBuilder* sub_bldr =
					static_cast<arrow::BooleanBuilder*>(bldr->value_builder());
				std::vector<uint8_t> castVec(append_row_count_ * listCount);
				castTo<uint8_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_ * listCount, offset* listCount);
				sub_bldr->AppendValues(castVec.data(), append_row_count_ * listCount);
			}
			else
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, offset);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::BooleanBuilder* sub_bldr =
					static_cast<arrow::BooleanBuilder*>(bldr->value_builder());
				sub_bldr->AppendValues((uint8_t*)fieldInfo.data, append_row_count_ * listCount);
			}
		}
		else {
			std::shared_ptr<arrow::BooleanBuilder> bldr = std::dynamic_pointer_cast<arrow::BooleanBuilder>(fieldInfo.builder);
			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);
			if (fieldInfo.nullValues == nullptr) {
				if (castRequired) {
					std::vector<uint8_t> castVec(append_row_count_);
					castTo<uint8_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_);
				}
				else
					bldr->AppendValues(((uint8_t*)fieldInfo.data)+offset, append_row_count_);
			}
			else {
				if (castRequired) {
					std::vector<uint8_t> castVec(append_row_count_);
					castTo<uint8_t>(fieldInfo.data, castVec, fieldInfo.castFrom, append_row_count_, offset);
					bldr->AppendValues(castVec.data(), append_row_count_, fieldInfo.nullValues);
				}
				else
					bldr->AppendValues(((uint8_t*)fieldInfo.data)+offset, append_row_count_, fieldInfo.nullValues);
			}
		}
		break;
	}
	case arrow::DoubleType::type_id:
	{
		if (isList) {
			// Get the relevant builder for the data type.
			std::shared_ptr<arrow::ListBuilder> bldr =
				std::dynamic_pointer_cast<arrow::ListBuilder>(fieldInfo.builder);

			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);

			// Offsets vector.
			std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, offset);
			bldr->AppendValues(offsets_vec.data(), append_row_count_);
			arrow::NumericBuilder<arrow::DoubleType>* sub_bldr =
				static_cast<arrow::NumericBuilder<arrow::DoubleType>*>(bldr->value_builder());
			sub_bldr->AppendValues(((double*)fieldInfo.data), append_row_count_* listCount);
		}
		else {
			std::shared_ptr<arrow::NumericBuilder<arrow::DoubleType>> bldr = std::dynamic_pointer_cast<arrow::NumericBuilder<arrow::DoubleType>>(fieldInfo.builder);
			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);
			if (fieldInfo.nullValues == nullptr)
				bldr->AppendValues(((double*)fieldInfo.data)+offset, append_row_count_);
			else
				bldr->AppendValues(((double*)fieldInfo.data)+offset, append_row_count_, fieldInfo.nullValues);
		}
		break;
	}
	case arrow::FloatType::type_id:
	{
		if (isList) {
			// Get the relevant builder for the data type.
			std::shared_ptr<arrow::ListBuilder> bldr =
				std::dynamic_pointer_cast<arrow::ListBuilder>(fieldInfo.builder);

			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);

			// Offsets vector.
			std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, offset);
			bldr->AppendValues(offsets_vec.data(), append_row_count_);
			arrow::NumericBuilder<arrow::FloatType>* sub_bldr =
				static_cast<arrow::NumericBuilder<arrow::FloatType>*>(bldr->value_builder());
			sub_bldr->AppendValues((float*)fieldInfo.data, append_row_count_* listCount);
		}
		else {
			std::shared_ptr<arrow::NumericBuilder<arrow::FloatType>> bldr = std::dynamic_pointer_cast<arrow::NumericBuilder<arrow::FloatType>>(fieldInfo.builder);
			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);
			if (fieldInfo.nullValues == nullptr)
				bldr->AppendValues(((float*)fieldInfo.data)+offset, append_row_count_);
			else
				bldr->AppendValues(((float*)fieldInfo.data)+offset, append_row_count_, fieldInfo.nullValues);
		}
		break;
	}
	case arrow::StringType::type_id:
	{
		if (isList) 
		{
			// !!!!!!!!!!!!!!!!!!! NOTE !!!!!!!!!!!!!!!!!
			// For the list type of string field, the explicit_row_count boolean
			// hasn't been implemented. See the comment below. It means that 
			// lists of strings is likely broken. 

			// Get the relevant builder for the data type.
			std::shared_ptr<arrow::ListBuilder> bldr =
				std::dynamic_pointer_cast<arrow::ListBuilder>(fieldInfo.builder);

			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);

			// Offsets vector.
			if (offset > 0)
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, 0);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::StringBuilder* sub_bldr =
					static_cast<arrow::StringBuilder*>(bldr->value_builder());
				fill_string_vec(fieldInfo.strPtr, append_row_count_ * listCount, offset * listCount);
				sub_bldr->AppendValues(temp_string_vec_);
			}
			else
			{
				std::vector<int32_t> offsets_vec = get_offsets_vector(append_row_count_, listCount, 0);
				bldr->AppendValues(offsets_vec.data(), append_row_count_);
				arrow::StringBuilder* sub_bldr =
					static_cast<arrow::StringBuilder*>(bldr->value_builder());
				sub_bldr->AppendValues(*fieldInfo.strPtr);
			}
		}
		else {
			std::shared_ptr<arrow::StringBuilder> bldr = std::dynamic_pointer_cast<arrow::StringBuilder>(fieldInfo.builder);
			// Resize array to allocate space and append data.
			bldr->Resize(append_row_count_);

			if (offset > 0)
			{
				/*
				If offset is non-zero, then the user is likely buffering data in the vector to which
				strPtr points. The offset conveys the index within that vector from which data should
				be copied and append_row_count_ gives the quantity that must be copied.
				*/
				fill_string_vec(fieldInfo.strPtr, append_row_count_, offset);
				if (fieldInfo.nullValues == nullptr) {
					bldr->AppendValues(temp_string_vec_);
				}
				else {
					bldr->AppendValues(temp_string_vec_, fieldInfo.nullValues);
				}
			}
			else
			{
				if (explicit_row_count && fieldInfo.strPtr->size() < append_row_count_)
				{
					// If the boolean explicit_row_count is true, then the user indicated a specific
					// number of rows by calling the writeColumns(int) function. The only way to 
					// convey to Arrow the row count for the StringBuilder class is via the implicit
					// vector size. So here we resize the vector to match the count of valid rows.
					// The user must be careful to ensure that there is no further use of this vector
					// or, if there is, that the original row count doesn't need to be used.

					// (IM) Changed to include "&& fieldInfo.strPtr->size() < append_row_count_"
					// In general, the user will not give a different row groups size until the end of
					// the data being archived is reached. In this case, the vector needs to be resized
					// based on the argument above. Otherwise, do not resize the vector.
					fieldInfo.strPtr->resize(append_row_count_);
					if (fieldInfo.nullValues == nullptr) {
						bldr->AppendValues(*fieldInfo.strPtr);

					}
					else {
						bldr->AppendValues(*fieldInfo.strPtr, fieldInfo.nullValues);
					}
				}
				else
				{
					fill_string_vec(fieldInfo.strPtr, append_row_count_, offset);
					if (fieldInfo.nullValues == nullptr) {
						bldr->AppendValues(temp_string_vec_);

					}
					else {
						bldr->AppendValues(temp_string_vec_, fieldInfo.nullValues);
					}
				}
			}
		}
		break;
	}
	default:
		printf("Data type not included: %s\n", fieldInfo.type->name().c_str());
		found = false;
		break;
	}

	// Make note that this column has been updated by setting the relevant bool.
	if(found)
		fieldInfo.readyForWrite = true;

}


void ParquetContext::addField(std::shared_ptr<arrow::DataType> type, std::string fieldName, int listSize)
{
	//printf("Add field %s\n", fieldName.c_str());
	bool isList;
	if (listSize == NULL)
		isList = false;
	else
		isList = true;

	// Spark can't read unsigned data types from a parquet file
	if (isUnsigned(type)) {
		printf("Warning!!!!!!!!!!!!Unsigned types are currently not available for writing parquet");
		return;
	}
	int byteSize;
	std::string typeID = getTypeID_fromArrowType(type, byteSize);
	field_info_map_[fieldName] = Field_Info(type, fieldName, typeID, byteSize, listSize);

	std::shared_ptr<arrow::Field> tempField;

	if (isList) {
		tempField = arrow::field(fieldName, arrow::list(type));
		fields.push_back(tempField);
	}
	else {
		tempField = arrow::field(fieldName, type);
		fields.push_back(tempField);
	}
	 /*
	switch (type->id())
	{
	case arrow::UInt64Type::type_id:
	{
		if (isList) {
			tempField = arrow::field(fieldName, arrow::list(arrow::uint64()));
			fields.push_back(tempField);
		}
		else {
			tempField = arrow::field(fieldName, arrow::uint64());
			fields.push_back(tempField);
		}
		break;
	}
	case arrow::Int64Type::type_id:
	{
		if (isList) {
			tempField = arrow::field(fieldName, arrow::list(arrow::int64()));
			fields.push_back(tempField);
		}
		else {
			tempField = arrow::field(fieldName, arrow::int64());
			fields.push_back(tempField);
		}
		break;
	}
	case arrow::Int32Type::type_id:
	{
		if (isList) {
			tempField = arrow::field(fieldName, arrow::list(arrow::int32()));
			fields.push_back(tempField);
		}
		else {
			tempField = arrow::field(fieldName, arrow::int32());
			fields.push_back(tempField);
		}
		break;
	}
	case arrow::UInt32Type::type_id:
	{
		if (isList) {
			tempField = arrow::field(fieldName, arrow::list(arrow::uint32()));
			fields.push_back(tempField);
		}
		else {
			tempField = arrow::field(fieldName, arrow::uint32());
			fields.push_back(tempField);
		}
		break;
	}
	case arrow::UInt16Type::type_id:
	{
		if (isList) {
			tempField = arrow::field(fieldName, arrow::list(arrow::uint16()));
			fields.push_back(tempField);
		}
		else {
			tempField = arrow::field(fieldName, arrow::uint16());
			fields.push_back(tempField);
		}
		break;
	}
	case arrow::Int16Type::type_id:
	{
		if (isList) {
			tempField = arrow::field(fieldName, arrow::list(arrow::int16()));
			fields.push_back(tempField);
		}
		else {
			tempField = arrow::field(fieldName, arrow::int16());
			fields.push_back(tempField);
		}
		break;
	}
	case arrow::UInt8Type::type_id:
	{
		if (isList) {
			tempField = arrow::field(fieldName, arrow::list(arrow::uint8()));
			fields.push_back(tempField);
		}
		else {
			tempField = arrow::field(fieldName, arrow::uint8());
			fields.push_back(tempField);
		}
		break;
	}
	case arrow::Int8Type::type_id:
	{
		if (isList) {
			tempField = arrow::field(fieldName, arrow::list(arrow::int8()));
			fields.push_back(tempField);
		}
		else {
			tempField = arrow::field(fieldName, arrow::int8());
			fields.push_back(tempField);
		}
		break;
	}
	case arrow::BooleanType::type_id:
	{
		if (isList) {
			tempField = arrow::field(fieldName, arrow::list(arrow::boolean()));
			fields.push_back(tempField);
		}
		else {
			tempField = arrow::field(fieldName, arrow::boolean());
			fields.push_back(tempField);
		}
		break;
	}
	case arrow::DoubleType::type_id:
	{
		if (isList) {
			tempField = arrow::field(fieldName, arrow::list(arrow::float64()));
			fields.push_back(tempField);
		}
		else {
			tempField = arrow::field(fieldName, arrow::float64());
			fields.push_back(tempField);
		}
		break;
	}
	case arrow::FloatType::type_id:
	{
		if (isList) {
			tempField = arrow::field(fieldName, arrow::list(arrow::float32()));
			fields.push_back(tempField);
		}
		else {
			tempField = arrow::field(fieldName, arrow::float32());
			fields.push_back(tempField);
		}
		break;
	}
	case arrow::StringType::type_id:
	{
		if (isList) {
			tempField = arrow::field(fieldName, arrow::list(arrow::utf8()));
			fields.push_back(tempField);
		}
		else {
			tempField = arrow::field(fieldName, arrow::utf8());
			fields.push_back(tempField);
		}
		break;
	}

	default:
		//printf("Data type not included: %s\n", type->name()->c_str());
		break;

	}*/
}

bool ParquetContext::isUnsigned(std::shared_ptr<arrow::DataType> type) {
	if (type->id() == arrow::UInt64Type::type_id || type->id() == arrow::UInt32Type::type_id || type->id() == arrow::UInt16Type::type_id || type->id() == arrow::UInt8Type::type_id)
		return true;
	else
		return false;
}

void ParquetContext::writeColumns(int rows, int offset)
{
	
	for (std::map<std::string, Field_Info>::iterator it = field_info_map_.begin(); it != field_info_map_.end(); ++it) {
		if (!it->second.pointerSet) {
			//printf("\033[0; 31m");
			printf("Warning!!!!!!, memory location for field not set: %s \n", it->second.fieldName);
			//printf("\033[0m");
			return;
		}
	}
	if (parquetStop) {
		//printf("\033[0; 31m");
		printf("Warning!!!!!!, parquetStop = true\n");
		//printf("\033[0m");
		return;
	}

	//int64_t n_rows = int64_t(rows);  ///This would need changed to the vector size
	//printf("in writeColumns, input row count %d\n", rows);
	append_row_count_ = rows;
	//printf("writing rows: %lld\n", n_rows);
	//int64_t start_at = 0;
	
	/*int64_t current_count = 0;

	while (start_at < n_rows)
	{
		if (start_at + ROW_GROUP_COUNT_ <= n_rows)
			current_count = ROW_GROUP_COUNT_;
		else
			current_count = n_rows - start_at;

		for (std::map<std::string, Field_Info>::iterator it = field_info_map_.begin(); it != field_info_map_.end(); ++it) {
			append_column(it->second, current_count, start_at);
		}
		start_at += ROW_GROUP_COUNT_;
	}*/
	if (rows > 0)
	{
		for (std::map<std::string, Field_Info>::iterator it = field_info_map_.begin(); it != field_info_map_.end(); ++it)
		{
			//printf("field name %s\n", it->first.c_str());
			append_column(it->second, rows, true, offset);
		}
	}

	write_cols_if_ready();
	// Reset the status of each column.
	for (std::map<std::string, Field_Info>::iterator it = field_info_map_.begin(); it != field_info_map_.end(); ++it)
	{
		it->second.readyForWrite = false;
	}
	//printf("after reset column status\n");
}

void ParquetContext::writeColumns()
{
	for (std::map<std::string, Field_Info>::iterator it = field_info_map_.begin(); it != field_info_map_.end(); ++it) {
		if (!it->second.pointerSet) {
			//printf("\033[0; 31m");
			printf("!!!!!!!!!!!!!!Warning, memory location for field not set: %s \n", it->second.fieldName);
			//printf("\033[0m");
			return;
		}
	}

	if (parquetStop) {
		//printf("\033[0; 31m");
		printf("Warning!!!!!!, parquetStop = true\n");
		//printf("\033[0m");
		return;
	}


	//printf("writing rows: %lld\n", ROW_GROUP_COUNT_);
	

	/*
	int64_t start_at = 0;
	int64_t current_count = 0;
	int64_t n_rows = int64_t(rows);  ///This would need changed to the vector size

	while (start_at < n_rows)
	{
		if (start_at + ROW_GROUP_COUNT_ <= n_rows)
			current_count = ROW_GROUP_COUNT_;
		else
			current_count = n_rows - start_at;

		for (std::map<std::string, Field_Info>::iterator it = field_info_map_.begin(); it != field_info_map_.end(); ++it) {
			append_column(it->second, current_count, start_at);
		}
		start_at += ROW_GROUP_COUNT_;
	}*/

	for (std::map<std::string, Field_Info>::iterator it = field_info_map_.begin(); it != field_info_map_.end(); ++it) {
		append_column(it->second, ROW_GROUP_COUNT_);
	}

	write_cols_if_ready();
	// Reset the status of each column.
	for (std::map<std::string, Field_Info>::iterator it = field_info_map_.begin(); it != field_info_map_.end(); ++it)
	{
		it->second.readyForWrite = false;
	}
}

template<typename NativeType> bool ParquetContext::setMemoryLocation(std::vector<NativeType>& data, std::string fieldName, uint8_t* boolField)
{
	
	for (std::map<std::string, Field_Info>::iterator it = field_info_map_.begin(); it != field_info_map_.end(); ++it){
		if (it->first == fieldName) {
			NativeType a;
			
			// if it is a list and boolField is defined, make sure to reset boolField to null since null lists aren't available
			if (it->second.isList) {
				if (boolField != nullptr) {
					//printf("\033[0; 31m");
					printf("Warning!!!!!!!!!!!!  Null fields for lists are currently unavailable: %s\n", fieldName.c_str());
					//printf("\033[0m");
					boolField = nullptr;
				}
			}
			// Check size of input array for list fields
			//if (it->second.isList && data.size() != ROW_GROUP_COUNT_ * it->second.listSize) {
			//	//printf("\033[0; 31m");
			//	printf("Warning!!!!!!!!!!!!  in setMemoryLocation, vector size is not consistent with row_group_count * list size for: %s\n", 
			//		fieldName.c_str());
			//	//printf("\033[0m");
			//} 
			//// Check size of input array for single value fields
			//else if (!it->second.isList && data.size() != ROW_GROUP_COUNT_ ) {
			//	//printf("\033[0; 31m");
			//	printf("Warning!!!!!!!!!!!!  in setMemoryLocation, vector size (%zu) is not consistent with row_group_count (%d) for: %s\n", 
			//		data.size(), ROW_GROUP_COUNT_, fieldName.c_str());
			//	//printf("\033[0m");
			//}

			// Check if ptr is already set
			if (it->second.pointerSet) {
				//printf("\033[0; 31m");
				printf("Warning!!!!!!!!!!!!  in setMemoryLocation, ptr is already set for: %s\n", fieldName.c_str());
				//printf("\033[0m");	
			}
			// If the casting is required			
			else if (typeid(NativeType).name() != it->second.typeID) {
				// Check if other types are being written to a string (NativeType being cast to second.typeID)
				if (it->second.typeID == typeid(std::string).name() && typeid(std::string).name() != typeid(NativeType).name()) {
					//printf("\033[0; 31m");
					printf("Warning!!!!!!!!!!!!  can't cast from other data type to string for: %s\n", fieldName.c_str());
					//printf("\033[0m");
				}
				// Cast to larger datatype check
				else if (it->second.byteSize < sizeof(a)) {
					printf("Warning!!!!!!!!!!!!  Intended datatype to be cast is smaller than the given datatype for: %s\n", fieldName.c_str());
				}// Check if floating point casting is happening
				else if (it->second.typeID == typeid(float).name() || it->second.typeID == typeid(double).name() || typeid(NativeType).name() == typeid(float).name() || typeid(NativeType).name() == typeid(double).name()) {
					printf("Warning!!!!!!!!!!!!  Can't cast floating point data types: %s, \n", fieldName.c_str());
				}
				// Equal size datatypes check
				else if (it->second.byteSize == sizeof(a)) {
					it->second.set_Field_Info(data.data(), fieldName, typeid(NativeType).name(), boolField);
					printf("Warning!!!!!!!!!!!!  Intended datatype to be cast is equal to the casting type for: %s\n", fieldName.c_str());
				}
				else {
					it->second.set_Field_Info(data.data(), fieldName, typeid(NativeType).name(), boolField);
					printf("Cast from %s planned for: %s, \n", typeid(NativeType).name(), fieldName.c_str());
				}
			}
			// data types are the same and no casting required
			else {
				it->second.set_Field_Info(data.data(), fieldName, "", boolField);
			}
			
			return true;
		}	
	}
	printf("ERROR!!! -> Field name doesn't exist: %s\n", fieldName.c_str());
	parquetStop = true;
	return false;
	
}

template<typename NativeType> bool ParquetContext::setMemoryLocation(std::vector<std::string>& data, std::string fieldName, uint8_t* boolField)
{
	for (std::map<std::string, Field_Info>::iterator it = field_info_map_.begin(); it != field_info_map_.end(); ++it) {
		if (it->first == fieldName) {
			
			//if (it->second.isList && data.size() != ROW_GROUP_COUNT_ * it->second.listSize) {
			//	//printf("\033[0; 31m");
			//	printf("Warning!!!!!!!!!!!!  in setMemoryLocation, vector size is not consistent with row_group_count * list size for: %s\n", fieldName);
			//	//printf("\033[0m");
			//}
			//else if (!it->second.isList && data.size() != ROW_GROUP_COUNT_) {
			//	//printf("\033[0; 31m");
			//	printf("Warning!!!!!!!!!!!!  in setMemoryLocation, vector size is not consistent with row_group_count for: %s\n", fieldName);
			//	//printf("\033[0m");
			//	
			//}
			if (typeid(std::string).name() != it->second.typeID) {
				//printf("\033[0; 31m");
				printf("Warning!!!!!!!!!!!!  in setMemoryLocation, can't cast from string to other types: %s\n", fieldName);
				//printf("\033[0m");
			}
			else if (it->second.pointerSet) {
				//printf("\033[0; 31m");
				printf("Warning!!!!!!!!!!!!  in setMemoryLocation, ptr is already set for: %s\n", fieldName);
				//printf("\033[0m");				
			}
			else if (it->second.isList) {
				if (boolField != nullptr) {
					//printf("\033[0; 31m");
					printf("Warning!!!!!!!!!!!!  Null fields for lists are currently unavailable: %s\n", fieldName);
					//printf("\033[0m");
				}
				it->second.set_Field_Info(data, fieldName, nullptr);
			}
			else {
				printf("setting field info for %s\n", fieldName.c_str());
				it->second.set_Field_Info(data, fieldName, boolField);
			}
			return true;
		}
	}
	printf("ERROR!!! -> Field name doesn't exist: %s\n", fieldName.c_str());
	parquetStop = true;
	return false;
}

std::string ParquetContext::getTypeID_fromArrowType(std::shared_ptr<arrow::DataType> type, int& byteSize){
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

void ParquetContext::fill_string_vec(std::vector<std::string>* str_data_vec_ptr, int64_t count, int offset)
{
	if (temp_string_vec_.size() != count)
		temp_string_vec_.resize(count);
	for (int i = 0; i < count; i++)
	{
		temp_string_vec_[i] = (*str_data_vec_ptr)[i + offset];
	}
}
