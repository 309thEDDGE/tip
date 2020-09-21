#include "parquet_translation_manager.h"

ParquetTranslationManager::ParquetTranslationManager(uint8_t id, ICDData icd) :
	parquet_path_(""), icd_(icd), iter_tools_(),
	pool_(arrow::default_memory_pool()), have_created_reader_(false), status_(1),
	table_count_(0), have_consumed_all_row_groups_(false), row_group_index_(0), raw_table_row_group_count_(0),
	raw_table_data_col_index_(0), data_org_(), output_dir_(), output_base_name_(),
	match_index_(UINT16_MAX), row_ind_(0), time_val_(0), raw_data_ptr_(nullptr),
	total_row_count_(0), current_row_count_(0), id_(id), complete_(false),
	select_msgs_(false), table_name_(""), is_multithreaded_(false), 
	table_indices_iterator_end_(table_indices_.end())
{

}

ParquetTranslationManager::ParquetTranslationManager(std::string parquet_path, ICDData icd) :
	parquet_path_(parquet_path), pool_(arrow::default_memory_pool()), have_created_reader_(false), status_(1),
	table_count_(0), have_consumed_all_row_groups_(false), row_group_index_(0), raw_table_row_group_count_(0),
	raw_table_data_col_index_(0), data_org_(), output_dir_(), output_base_name_(),
	match_index_(UINT16_MAX), row_ind_(0), time_val_(0), raw_data_ptr_(nullptr),
	total_row_count_(0), current_row_count_(0), id_(UINT8_MAX), complete_(false),
	select_msgs_(false), table_indices_iterator_end_(table_indices_.end()),
	icd_(icd), iter_tools_(), table_name_(""), is_multithreaded_(false)
{
	if (setup_output_paths() == 1)
	{
		status_ = -1;
		printf("Error setup file paths\n");
		return;
	}
}

void ParquetTranslationManager::set_select_msgs_list(bool select_msgs, std::vector<std::string>& select_msg_list)
{
	select_msgs_ = select_msgs;
	select_msg_vec_ = select_msg_list;
	build_select_messages_indices_set();
}


uint8_t ParquetTranslationManager::setup_output_paths()
{
	// Determine if parquet_path is a directory.
	std::filesystem::path input_parquet_path(parquet_path_);
	parquet_path_is_dir_ = std::filesystem::is_directory(input_parquet_path);

	if (parquet_path_is_dir_)
	{
		printf("Input path IS a directory: %s\n", parquet_path_.c_str());

		/*
		If input path is a directory, then the Parquet "file" represented 
		by the input path is a special case in which a directory can be 
		labeled as .parquet and a Parquet reader can read the directory 
		and all of its contents as if it were a single file. In this case,
		this translation algorithm needs to loop over and consume the 
		contents of each file in that directory.
		*/
		size_t find_result = std::string::npos;
		std::string temp_path = "";
		for (auto& p : std::filesystem::directory_iterator(input_parquet_path))
		{
			temp_path = p.path().string();
			if (!p.is_directory())
			{
				// Do not add the path of the current file if it has the
				// sub-string "metadata" because that file contains only
				// metadata and will have been already consumed in main().
				find_result = temp_path.find("metadata");
				if (find_result == std::string::npos && temp_path.find("_TMATS") == std::string::npos)
				{
#ifdef DEBUG
#if DEBUG > 1
					printf("input path: %s\n", temp_path.c_str());
#endif
#endif
					input_parquet_paths_.push_back(temp_path);
				}
			}
		}

		// Sort the input paths such that they are processed in the order generated
		// by the parser.
		input_parquet_paths_ = iter_tools_.Sort(input_parquet_paths_);

		// Create an output directory at the same level as the Parquet "file"
		// directory.
		output_base_name_ = input_parquet_path.stem(); 
		output_base_name_ += std::filesystem::path("_translated");
		//printf("parent path is %s\n", input_parquet_path.parent_path().string().c_str());
		output_dir_ = input_parquet_path.parent_path() / output_base_name_;
		if (!std::filesystem::exists(output_dir_))
		{
			bool create_dir = std::filesystem::create_directory(output_dir_);
			if (!create_dir)
			{
				printf("Failed to create directory: %s\n", output_dir_.string().c_str());
				return 1;
			}
		}
	}
	else
	{
		printf("Input path IS NOT a directory: %s\n", parquet_path_.c_str());

		// There is only one input path, the parquet_path_.
		input_parquet_paths_.push_back(parquet_path_);
		
		// The output directory is at the same level as the input path.
		output_base_name_ = input_parquet_path.stem();
		output_base_name_ += std::filesystem::path("_translated");
		//printf("parent path is %s\n", input_parquet_path.parent_path().string().c_str());
		output_dir_ = input_parquet_path.parent_path() / output_base_name_;
		if (!std::filesystem::exists(output_dir_))
		{
			bool create_dir = std::filesystem::create_directory(output_dir_);
			if (!create_dir)
			{
				printf("Failed to create directory: %s\n", output_dir_.string().c_str());
				return 1;
			}
		}
		/*else
			printf("Directory already exists: %s\n", output_dir_.string().c_str());*/
	}
	printf("Output base name: %s\n", output_base_name_.string().c_str());
	printf("Output directory: %s\n", output_dir_.string().c_str());

	return 0;
}


void ParquetTranslationManager::get_paths(std::string parquet_path, std::filesystem::path& output_base_path,
	std::filesystem::path& output_base_name, std::filesystem::path& msg_list_path,
	std::vector<std::string>& input_parquet_paths, bool& parquet_path_is_dir)
{
	parquet_path_ = parquet_path;
	setup_output_paths();
	output_base_path = output_dir_;
	output_base_name = output_base_name_;
	input_parquet_paths = input_parquet_paths_;
	parquet_path_is_dir = parquet_path_is_dir_;
}

int ParquetTranslationManager::get_status()
{
	return status_;
}
std::atomic<bool>& ParquetTranslationManager::completion_status()
{
	return complete_;
}



void ParquetTranslationManager::operator()(std::filesystem::path& output_base_path, 
	std::filesystem::path& output_base_name, std::vector<std::string>& input_parquet_paths, 
	bool is_multithreaded)
{
	// Get start time.
	//auto start_time = std::chrono::high_resolution_clock::now();
	is_multithreaded_ = true;

	// Set path-related variables.
	output_dir_ = output_base_path;
	output_base_name_ = output_base_name;
	input_parquet_paths_ = input_parquet_paths;

	run_translation_loop();

	printf("\n(%02hhu) Total row count: %llu\n", id_, total_row_count_);

	// Get stop time and print duration.
	/*auto stop_time = std::chrono::high_resolution_clock::now();
	printf("(%02hhu) Duration: %zd sec\n", id_, std::chrono::duration_cast<std::chrono::seconds>(stop_time - start_time).count());*/
	complete_ = true;
}

void ParquetTranslationManager::translate()
{
	is_multithreaded_ = false;
	// Get start time.
	auto start_time = std::chrono::high_resolution_clock::now();

	// Create a set of table indices if select_specific_messages
	// has been set to true. Use this set to determine if a given
	// table ought to be created/translated.

	run_translation_loop();

	printf("\nTotal row count: %llu\n", total_row_count_);

	// Get stop time and print duration.
	auto stop_time = std::chrono::high_resolution_clock::now();
	printf("Duration: %zd sec\n", std::chrono::duration_cast<std::chrono::seconds>(stop_time - start_time).count());
}

void ParquetTranslationManager::run_translation_loop()
{
	// Loop over all input Parquet files and process each in turn.
	for (uint16_t file_ind = 0; file_ind < input_parquet_paths_.size(); file_ind++)
	{

		if (open_raw_1553_parquet_file(input_parquet_paths_[file_ind]) == 1)
		{
			status_ = -1;
			printf("Error opening raw 1553 Parquet file\n");
			return;
		}

		// Grab and parse data from raw 1553 Parquet file. Translate as necessary.
		printf("\nRead row groups:\n");
		while (!have_consumed_all_row_groups_)
		{
			/*
			Accumulate data from Parquet file in chunks of row groups. Loop over relevant columns
			from the row group and distribute to the table represented by the message name. Either
			translate raw columns and fill the translated columns of each table after each row group or,
			memory limiting, translate raw columns after all row groups have been read and raw data
			have been distributed to all relevant tables.
			*/
			if (consume_row_group() == 1)
			{
				status_ = -1;
				return;
			}
		}
		printf("\n");

		/*
		Close the raw 1553 Parquet file and reset any necessary variables in
		preparation for reading the next file.
		*/
		if (close_raw_1553_parquet_file() == 1)
		{
			status_ = -1;
			return;
		}
		/*if (file_ind > 2)
			break;*/
	}

	/*
	Translate all un-translated values and finalize output Parquet files.
	Do this only if, in a multi-input file sequence, all row groups
	of the final input file have been consumed.
	*/
	if (finalize_translation() == 1)
	{
		status_ = -1;
		return;
	}
}


ParquetTranslationManager::~ParquetTranslationManager()
{
	if (have_created_reader_)
		arrow_file_->Close();
}

uint8_t ParquetTranslationManager::consume_row_group()
{
#ifdef DEBUG
#if DEBUG > 0
	printf("\rConsuming row group %03d                                       ", row_group_index_);
#endif
#endif
	// Read row group with relevant columns only.
	std::shared_ptr<arrow::Table> arrow_table;
	st_ = arrow_reader_->ReadRowGroup(row_group_index_, all_raw_table_indices_vec_, &arrow_table);
	if (!st_.ok())
	{
		printf("arrow::io::ReadableFile::ReadRowGroup error (ID %s): %s\n",
			st_.CodeAsString().c_str(), st_.message().c_str());
		return 1;
	}

	// Get the schema to map column index to data.
	std::shared_ptr<arrow::Schema> arrow_schema = arrow_table->schema();


	// Get arrays of data.
	// Assume that each row group only has 1 chunk. This is true of the first
	// 1553 raw data Parquet file that I tested. Because all Ch10 Parquet files will
	// be created with ParquetContext which writes entire row groups in one pass and 
	// not from chunked arrays, this should be a safe assumption moving forward. 
	//
	// Note that the selected columns below are indexed 0, 1, 2 and 3.
	// This is not the column order in the original parquet files. These 
	// columns have been selected by the ReadRowGroup function above which 
	// reads only selected columns indicated by the values in the 
	// vector all_raw_table_indices_vec_. Unless the order of the selected
	// columns changes in the original table, there is no need to change the
	// indices below. For example, if more columns are appended to the table
	// at parse time the time, mode, etc., column order will not be disrupted
	// and the following can be left unchanged. However, 
	// TODO: set column index in a dynamic way using the schema from arrow_table.

	raw_table_metadata_col_indices_map_["channelid"] = schema_->GetFieldIndex("channelid");
	raw_table_metadata_col_indices_map_["txrtaddr"] = schema_->GetFieldIndex("txrtaddr");
	raw_table_metadata_col_indices_map_["rxrtaddr"] = schema_->GetFieldIndex("rxrtaddr");
	raw_table_metadata_col_indices_map_["txsubaddr"] = schema_->GetFieldIndex("txsubaddr");
	raw_table_metadata_col_indices_map_["rxsubaddr"] = schema_->GetFieldIndex("rxsubaddr");

#ifdef DEBUG
#if DEBUG > 2
	int chunk_count = arrow_table->column(0)->data()->num_chunks();
	printf("chunk_count = %d\n", chunk_count);
#endif
#endif
	
#ifdef NEWARROW
	arrow::NumericArray<arrow::Int32Type> channelid_arr = arrow::NumericArray<arrow::Int32Type>(
		arrow_table->column(arrow_schema->GetFieldIndex("channelid"))->chunk(0)->data());
	arrow::NumericArray<arrow::Int8Type> txrtaddr_arr = arrow::NumericArray<arrow::Int8Type>(
		arrow_table->column(arrow_schema->GetFieldIndex("txrtaddr"))->chunk(0)->data());
	arrow::NumericArray<arrow::Int8Type> rxrtaddr_arr = arrow::NumericArray<arrow::Int8Type>(
		arrow_table->column(arrow_schema->GetFieldIndex("rxrtaddr"))->chunk(0)->data());
	arrow::NumericArray<arrow::Int8Type> txsubaddr_arr = arrow::NumericArray<arrow::Int8Type>(
		arrow_table->column(arrow_schema->GetFieldIndex("txsubaddr"))->chunk(0)->data());
	arrow::NumericArray<arrow::Int8Type> rxsubaddr_arr = arrow::NumericArray<arrow::Int8Type>(
		arrow_table->column(arrow_schema->GetFieldIndex("rxsubaddr"))->chunk(0)->data());
#else
	arrow::NumericArray<arrow::Int32Type> channelid_arr = arrow::NumericArray<arrow::Int32Type>(
		arrow_table->column(arrow_schema->GetFieldIndex("channelid"))->data()->chunk(0)->data());
	arrow::NumericArray<arrow::Int8Type> txrtaddr_arr = arrow::NumericArray<arrow::Int8Type>(
		arrow_table->column(arrow_schema->GetFieldIndex("txrtaddr"))->data()->chunk(0)->data());
	arrow::NumericArray<arrow::Int8Type> rxrtaddr_arr = arrow::NumericArray<arrow::Int8Type>(
		arrow_table->column(arrow_schema->GetFieldIndex("rxrtaddr"))->data()->chunk(0)->data());
	arrow::NumericArray<arrow::Int8Type> txsubaddr_arr = arrow::NumericArray<arrow::Int8Type>(
		arrow_table->column(arrow_schema->GetFieldIndex("txsubaddr"))->data()->chunk(0)->data());
	arrow::NumericArray<arrow::Int8Type> rxsubaddr_arr = arrow::NumericArray<arrow::Int8Type>(
		arrow_table->column(arrow_schema->GetFieldIndex("rxsubaddr"))->data()->chunk(0)->data());
#endif

#ifdef NEWARROW
	arrow::NumericArray<arrow::Int64Type> time_arr = arrow::NumericArray<arrow::Int64Type>(
		arrow_table->column(arrow_schema->GetFieldIndex("time"))->chunk(0)->data());
	arrow::BooleanArray mode_arr = arrow::BooleanArray(
		arrow_table->column(arrow_schema->GetFieldIndex("mode"))->chunk(0)->data());
	arrow::ListArray data_listarr = arrow::ListArray(
		arrow_table->column(arrow_schema->GetFieldIndex("data"))->chunk(0)->data());
	arrow::NumericArray<arrow::Int32Type> data_arr = arrow::NumericArray<arrow::Int32Type>(
		data_listarr.values()->data());
#else
	arrow::NumericArray<arrow::Int64Type> time_arr = arrow::NumericArray<arrow::Int64Type>(
		arrow_table->column(arrow_schema->GetFieldIndex("time"))->data()->chunk(0)->data());
	arrow::BooleanArray mode_arr = arrow::BooleanArray(
		arrow_table->column(arrow_schema->GetFieldIndex("mode"))->data()->chunk(0)->data());
	arrow::ListArray data_listarr = arrow::ListArray(
		arrow_table->column(arrow_schema->GetFieldIndex("data"))->data()->chunk(0)->data());
	arrow::NumericArray<arrow::Int32Type> data_arr = arrow::NumericArray<arrow::Int32Type>(
		data_listarr.values()->data());
#endif
	raw_data_ptr_ = data_arr.raw_values();

#ifdef DEBUG
#if DEBUG > 2
	printf("Row count in row group %02d = %lld\n", row_group_index_, time_arr.length());
	printf("Length of data array = %lld\n", data_arr.length());
#endif
#endif

	// For each row, append the data to the corresponding table.
	current_row_count_ = time_arr.length();
	total_row_count_ += current_row_count_;
	std::set<size_t> matching_table_inds;
	std::set<size_t>::const_iterator it;
	for (row_ind_ = 0; row_ind_ < current_row_count_; row_ind_++)
	{
#ifdef DEBUG
#if DEBUG > 4
		printf("row_ind = %lld\n", row_ind_);
#endif
#endif
		// Isaac Debug
		/*if (channelid_arr.Value(row_ind_) == 33 && txrtaddr_arr.Value(row_ind_) == 0
			&& rxrtaddr_arr.Value(row_ind_) == 6 && txsubaddr_arr.Value(row_ind_) == 0
			&& rxsubaddr_arr.Value(row_ind_) == 0)
		{
			printf("EG17_M appears in this row\n");
		}*/
		// End Isaac Debug

		// Do not include mode word messages.
		/*if (!mode_arr.Value(row_ind_))
		{*/
		// Get the indices of the table(s) that corresponds to the channel ID, TX/RX LRU addrs,
		// TX/RX LRU subaddrs.
		matching_table_inds = icd_.LookupTableIndex(channelid_arr.Value(row_ind_), 
			txrtaddr_arr.Value(row_ind_), rxrtaddr_arr.Value(row_ind_), 
			txsubaddr_arr.Value(row_ind_), rxsubaddr_arr.Value(row_ind_));
#ifdef DEBUG
#if DEBUG > 4
		printf("matching table inds size: %zu\n", matching_table_inds.size());
#endif
#endif
		// Iterate over each of the matching table indices.
		for (it = matching_table_inds.begin(); it != matching_table_inds.end(); ++it)
		{
			// If the table index exists in the set of table_indices, then continue.
			// Otherwise construct a new table and add it to the map.
#ifdef DEBUG
#if DEBUG > 4
			printf("table index %zu\n", *it);
#endif
#endif
			// If select specific messages is set to true, then check if the current
			// table index is in the select table indices set. Do nothing for the current
			// index if it is not in the set.
			if (select_msgs_)
			{
				if (select_table_indices_.count(*it) != 1)
					continue;
#ifdef DEBUG
#if DEBUG > 4
				else
				{
					printf("table is selected message %s\n", 
						icd_.LookupTableNameByIndex(*it).c_str());
				}
#endif
#endif
			}

			// If the table index is not present in the set of already created table 
			// indices, then create the table and its columns and add it to the set.
			if (table_indices_.find(*it) == table_indices_iterator_end_)
			{
#ifdef DEBUG
#if DEBUG > 4
				printf("table not found, creating table\n");
#endif
#endif
					
				// Creates the table, adds it to the table_index_to_table_map_,
				// creates the columns in the table and initializes the ParquetContext.
				if (create_table(*it) == 1)
				{
					// Failed in some way. Do not attempt
					// to add data or translate.
					continue;
				}
			}
					
			// If the table is not bad.
			if (!table_index_to_table_map_[*it]->is_bad())
			{

				time_val_ = time_arr.Value(row_ind_);
				table_index_to_table_map_[*it]->append_data(time_val_, raw_data_ptr_, row_ind_);

				// Update the count of times that raw data has been appended to the given
				// table. If it's full, then translate each column of the table and 
				// reset the various table counters. 
				msg_append_count_map_[*it]++;
				if (msg_append_count_map_[*it] == TABLE_COL_ROW_ALLOC_COUNT)
				{
					// Translate the table.
					if (table_index_to_table_map_[*it]->translate() != 0)
					{
						// Failure of some kind.
						printf("\nParquetTranslationManager::consume_row_group(): Translation failure!\n");
					}
					else
					{
						// Write the translated columns to Parquet file.
						table_index_to_table_map_[*it]->write_to_parquet_file(
							msg_append_count_map_[*it]);
					}

					// Reset the append count.
					msg_append_count_map_[*it] = 0;
				}
			}
#ifdef DEBUG
#if DEBUG > 1
			else
			{
				printf("table %s is BAD!\n", table_index_to_table_map_[*it]->name().c_str());
			}
#endif
#endif
		}
		//} // end if mode code
#ifdef DEBUG
#if DEBUG > 4
		system("pause");
#endif
#endif
	}
	row_group_index_++;
	if(row_group_index_ == raw_table_row_group_count_)
		have_consumed_all_row_groups_ = true; 
	return 0;
}

uint8_t ParquetTranslationManager::open_raw_1553_parquet_file(std::string& current_path)
{
	// Open file reader.
#ifdef DEBUG
#if DEBUG > 0
	printf("\nReading parquet file: %s\n", current_path.c_str());
#endif
#endif

#ifdef NEWARROW
	try
	{
		PARQUET_ASSIGN_OR_THROW(arrow_file_, arrow::io::ReadableFile::Open(current_path, pool_));  
	}
	catch (...)
	{
		printf("ReadableFile::Open error\n");
		return false;
	}
	   
#else
	st_ = arrow::io::ReadableFile::Open(current_path, pool_, &arrow_file_);
	if (!st_.ok())
	{
		printf("arrow::io::ReadableFile::Open error (ID %s): %s\n", 
			st_.CodeAsString().c_str(), st_.message().c_str());
		return 1;
	}
#endif
	st_ = parquet::arrow::OpenFile(arrow_file_, pool_, &arrow_reader_);
	if (!st_.ok())
	{
		printf("parquet::arrow::OpenFile error (ID %s): %s\n",
			st_.CodeAsString().c_str(), st_.message().c_str());
		return 1;
	}

	arrow_reader_->set_use_threads(true);

	// Get schema.
	st_ = arrow_reader_->GetSchema(&schema_);
	if (!st_.ok())
	{
		printf("GetSchema() error (ID %s): %s\n",
			st_.CodeAsString().c_str(), st_.message().c_str());
		return 1;
	}
	//printf("Schema:\n%s\n", schema_->ToString().c_str());

	have_created_reader_ = true;

	// Total count of row groups.
	raw_table_row_group_count_ = arrow_reader_->num_row_groups();
#ifdef DEBUG
#if DEBUG > 1
	printf("%02d row groups\n", raw_table_row_group_count_);
#endif
#endif

	// Reset control logic for single Parquet file row group consumption.
	have_consumed_all_row_groups_ = false;
	row_group_index_ = 0;

	// Assemble indices of relevant columns from the raw 1553 data
	// table.
	raw_table_ridealong_col_indices_map_["time"] = schema_->GetFieldIndex("time");
	raw_table_metadata_col_indices_map_["mode"] = schema_->GetFieldIndex("mode");
	raw_table_metadata_col_indices_map_["channelid"] = schema_->GetFieldIndex("channelid");
	raw_table_metadata_col_indices_map_["txrtaddr"] = schema_->GetFieldIndex("txrtaddr");
	raw_table_metadata_col_indices_map_["rxrtaddr"] = schema_->GetFieldIndex("rxrtaddr");
	raw_table_metadata_col_indices_map_["txsubaddr"] = schema_->GetFieldIndex("txsubaddr");
	raw_table_metadata_col_indices_map_["rxsubaddr"] = schema_->GetFieldIndex("rxsubaddr");
	raw_table_data_col_index_ = schema_->GetFieldIndex("data");

	for (std::unordered_map<std::string, int>::iterator it = raw_table_ridealong_col_indices_map_.begin();
		it != raw_table_ridealong_col_indices_map_.end(); ++it)
	{
		all_raw_table_indices_vec_.push_back(it->second);
	}
	for (std::unordered_map<std::string, int>::iterator it = raw_table_metadata_col_indices_map_.begin();
		it != raw_table_metadata_col_indices_map_.end(); ++it)
	{
		all_raw_table_indices_vec_.push_back(it->second);
	}
	all_raw_table_indices_vec_.push_back(raw_table_data_col_index_);

#ifdef DEBUG
#if DEBUG > 1
	printf("Relevant raw table indices:\n");
	for (int i = 0; i < all_raw_table_indices_vec_.size(); i++)
		printf("%02d, ", all_raw_table_indices_vec_[i]);
	printf("\n");
#endif
#endif

	return 0;
}

uint8_t ParquetTranslationManager::close_raw_1553_parquet_file()
{
	if (have_created_reader_)
		arrow_file_->Close();

	have_created_reader_ = false;
	raw_table_row_group_count_ = 0;
	all_raw_table_indices_vec_.resize(0);

	return 0;
}

uint8_t ParquetTranslationManager::create_table(size_t table_ind)
{
	table_name_ = icd_.LookupTableNameByIndex(table_ind);
#ifdef DEBUG
#if DEBUG > 1
	printf("\nCreating Translatable1553Table: %s\n", table_name_.c_str());
#elif DEBUG > 0
	printf("Creating Translatable1553Table: %s\n", table_name_.c_str());
#endif
#endif

	// Create the new table.
	std::shared_ptr<Translatable1553Table> table_ptr = std::make_shared<Translatable1553Table>(
		table_name_, &icd_, table_ind);

	// Add the table to the map and update the map of append count
	// and table_indices vector. Even if the subsequent functions
	// fail, we don't want to attempt to create the table each
	// time a row in the raw 1553 table matches this particular
	// table/message type.
	table_index_to_table_map_[table_ind] = table_ptr;
	msg_append_count_map_[table_ind] = 0;
	table_indices_.insert(table_ind);
	table_indices_iterator_end_ = table_indices_.end();

	// Create all translatable columns.
#ifdef DEBUG
#if DEBUG > 1
	printf("Creating translatable columns for table: %s\n", table_name_.c_str());
#endif
#endif
	uint8_t ret_val = table_ptr->create_translatable_columns(TABLE_COL_ROW_ALLOC_COUNT);
	if (ret_val == 1)
	{
		printf("create_translatable_columns() : Failed to create translated EU columns in table %s\n", 
			table_name_.c_str());
		return ret_val;
	}

	// Create ridealong columns, one by one.
#ifdef DEBUG
#if DEBUG > 1
	printf("Creating ridealong columns for table: %s\n", table_name_.c_str());
#endif
#endif
	if (!table_ptr->is_bad())
	{
		ret_val = table_ptr->create_ridealong_column(TABLE_COL_ROW_ALLOC_COUNT, schema_->GetFieldByName("time"));
		if (ret_val == 1)
		{
			printf("create_ridealong_column() : Failed to create ridealong column in table %s\n", "time");
			return ret_val;
		}
	}

	// Initialize table output context.
#ifdef DEBUG
#if DEBUG > 2
	printf("Initializing ParquetContext for table: %s\n", table_name_.c_str());
#endif
#endif
	if (!table_ptr->is_bad())
	{
		ret_val = table_ptr->configure_parquet_context(output_dir_, output_base_name_,
			is_multithreaded_, id_);
		if (ret_val != 0)
		{
			printf("configure_parquet_context() error!\n");
			return ret_val;
		}
	}

	ret_val = 0;
	return ret_val;
}


void ParquetTranslationManager::build_select_messages_indices_set()
{
	size_t temp_ind = 0;
	for (size_t i = 0; i < select_msg_vec_.size(); i++)
	{
		temp_ind = icd_.GetTableIndexByName(select_msg_vec_[i]);
		if (temp_ind != std::string::npos)
		{
			select_table_indices_.insert(temp_ind);
#ifdef DEBUG
#if DEBUG > 0
			printf("ParquetTranslationManager::build_select_messages_indices_set(): "
				"select message %s has table index %zu\n", select_msg_vec_[i].c_str(),
				temp_ind);
#endif
#endif
		}
#ifdef DEBUG
#if DEBUG > 0
		else
		{
			printf("ParquetTranslationManager::build_select_messages_indices_set(): "
				"select message %s not found\n", select_msg_vec_[i].c_str());
		}
#endif
#endif

	}
}

uint8_t ParquetTranslationManager::finalize_translation()
{
	printf("ParquetTranslationManager::finalize_translation(): Finalizing translation and file writing\n");
	// Check the vector which contains the count of raw messages appended
	// to each table (msg_append_count_vec_) for non-zero elements. If
	// the count is greater than zero, then there are elements that remain
	// to be translated. Call the translation routine to translate the 
	// remaining elements.
	for (std::set<size_t>::const_iterator it = table_indices_.begin(); it != table_indices_.end(); ++it)
	{
		if (msg_append_count_map_[*it] > 0)
		{
			if (!table_index_to_table_map_[*it]->is_bad())
			{
				// Translate the table.
				if (table_index_to_table_map_[*it]->translate() != 0)
				{
					// Failure of some kind.
					printf("ParquetTranslationManager::finalize_translation(): Translation failure!\n");
				}

				table_index_to_table_map_[*it]->write_to_parquet_file(msg_append_count_map_[*it]);
			}
		}
	}
	return 0;
}

