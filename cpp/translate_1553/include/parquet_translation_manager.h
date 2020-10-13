#ifndef PARQUET_TRANSLATION_MANAGER_H
#define PARQUET_TRANSLATION_MANAGER_H

#include <cstdio>
#include <fstream>
#include <iostream>
#include <cstring>
#include <filesystem>
#include <chrono>
#include <atomic>
#include "data_organization.h"
#include <vector>
#include <set>
#include "translatable_1553_table.h"
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/schema.h>
#include "iterable_tools.h"
#include "icd_data.h"

const int TABLE_COL_ROW_ALLOC_COUNT = 10000;

class ParquetTranslationManager
{
private:
	DataOrg data_org_;
	int status_;
	std::string parquet_path_;
	bool parquet_path_is_dir_;
	std::vector<std::string> input_parquet_paths_;
	std::filesystem::path output_dir_;
	std::filesystem::path output_base_name_;
	bool select_msgs_;
	std::vector<std::string> select_msg_vec_;
	bool have_created_reader_;
	bool have_consumed_all_row_groups_;
	int row_group_index_;
	uint32_t table_count_;
	int raw_table_row_group_count_;
	std::unordered_map<std::string, int> raw_table_ridealong_col_indices_map_;
	int raw_table_data_col_index_;
	std::unordered_map<std::string, int> raw_table_metadata_col_indices_map_;
	std::vector<int> all_raw_table_indices_vec_;
	bool is_multithreaded_;
	IterableTools iter_tools_;

	ICDData icd_;
	std::unordered_map<size_t, std::shared_ptr<Translatable1553Table>> table_index_to_table_map_;
	std::set<size_t> table_indices_;
	std::set<size_t>::iterator table_indices_iterator_end_;
	std::unordered_map<size_t, uint16_t> msg_append_count_map_;
	std::string table_name_;
	std::set<size_t> select_table_indices_;

	// Arrow variables.
	arrow::Status st_;
	arrow::MemoryPool* pool_;
	std::shared_ptr<arrow::io::ReadableFile> arrow_file_;
	std::unique_ptr<parquet::arrow::FileReader> arrow_reader_;
	std::shared_ptr<arrow::Schema> schema_;

	// Vars for consume_row_group().
	uint16_t match_index_;
	int64_t row_ind_;
	int64_t time_val_;
	const int32_t* raw_data_ptr_;
	int64_t current_row_count_;
	uint64_t total_row_count_;

	// Multithreading-related vars.
	uint8_t id_;
	std::atomic<bool> complete_;

	// Private functions
	uint8_t setup_output_paths();
	uint8_t open_raw_1553_parquet_file(std::string& current_path);
	uint8_t close_raw_1553_parquet_file();
	uint8_t consume_row_group();
	uint8_t finalize_translation();
	void run_translation_loop();
	
	uint8_t create_table(size_t table_ind);
	void build_select_messages_indices_set();
	

public:
	ParquetTranslationManager(std::string parquet_path, ICDData icd);
	ParquetTranslationManager(uint8_t id, ICDData icd);
	~ParquetTranslationManager();
	int get_status();
	void translate();
	
	void set_select_msgs_list(bool select_msgs, std::vector<std::string>& select_msg_list);
	

	// Functions for use with multithreaded parsing only.
	void operator()(std::filesystem::path& output_base_path, std::filesystem::path& output_base_name,
		std::vector<std::string>& input_parquet_paths, bool is_multithreaded);
	void get_paths(std::string parquet_path, std::filesystem::path& output_base_path, 
		std::filesystem::path& output_base_name, std::filesystem::path& msg_list_path,
		std::vector<std::string>& input_parquet_paths, bool& parquet_path_is_dir);
	void get_message_list(std::vector<std::string>& msg_names_list, bool& message_list_exists);
	void operator()(std::filesystem::path& output_base_path, std::filesystem::path& output_base_name,
		std::vector<std::string>& input_parquet_paths, std::filesystem::path& msg_list_path, 
		std::vector<std::string>& msg_names_list, bool is_multithreaded);
	std::atomic<bool>& completion_status();
	std::filesystem::path GetTranslatedDataDirectory();
};

#endif 