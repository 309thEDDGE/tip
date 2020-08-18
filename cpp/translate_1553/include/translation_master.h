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
	std::string parquet_path_;
	std::vector<std::thread> threads_;
	std::chrono::milliseconds worker_wait_;
	std::chrono::milliseconds worker_start_offset_;

public:
	TranslationMaster(std::string parquet_path, uint8_t n_threads,
		bool select_msgs, std::vector<std::string> select_msg_names, ICDData icd);
	uint8_t translate();
};

#endif
