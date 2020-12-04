#ifndef TRANSLATION_MASTER_H
#define TRANSLATION_MASTER_H

#include "parquet_translation_manager.h"
#include <thread>
#include <cmath>

class TranslationMaster
{
private:
	std::vector <std::unique_ptr<ParquetTranslationManager>> ptm_vec_;
	uint8_t n_threads_;
	ManagedPath parquet_path_;
	std::vector<std::thread> threads_;
	std::chrono::milliseconds worker_wait_;
	std::chrono::milliseconds worker_start_offset_;

	ManagedPath output_base_path_;
	ManagedPath output_base_name_;
	ManagedPath msg_list_path_;
	std::vector<ManagedPath> input_parquet_paths_;
	bool parquet_path_is_dir_;
	bool is_multithreaded_;

public:
	TranslationMaster(const ManagedPath& parquet_path, uint8_t n_threads,
		bool select_msgs, std::vector<std::string> select_msg_names, ICDData icd);
	uint8_t translate();
	ManagedPath GetTranslatedDataDirectory();
};

#endif
