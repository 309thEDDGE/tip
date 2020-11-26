#include "translation_master.h"

TranslationMaster::TranslationMaster(const ManagedPath& parquet_path, uint8_t n_threads,
	bool select_msgs, std::vector<std::string> select_msg_names, ICDData icd) :
	n_threads_(n_threads), parquet_path_(parquet_path), worker_wait_(200),
	worker_start_offset_(2000), is_multithreaded_(true), output_base_path_(""),
	output_base_name_(""), msg_list_path_(""), parquet_path_is_dir_(false)
{
	// Setup ParquetTranslationManager classes.
	for (uint8_t i = 0; i < n_threads_; i++)
	{
		// Instantiate ParquetTranslationManager objects. One object
		// per thread.
		ptm_vec_.push_back(std::make_unique<ParquetTranslationManager>(i, icd));
		ptm_vec_[i]->set_select_msgs_list(select_msgs, select_msg_names);
	}

	// Determine paths using ParquetTranslationManager. Get paths now in order
	// for call to GetTranslatedDataDir to be of use for metadata recording prior to
	// translation. Metadata may be useful for debuggin.
	ptm_vec_[0]->get_paths(parquet_path_, output_base_path_, output_base_name_, 
		msg_list_path_, input_parquet_paths_, parquet_path_is_dir_);
}

ManagedPath TranslationMaster::GetTranslatedDataDirectory()
{
	return output_base_path_;
}

uint8_t TranslationMaster::translate()
{
	
	if (input_parquet_paths_.size() == 0)
	{
		printf("TranslationMaster::translate(): Failed to get input parquet paths\n");
		return 1;
	}


	// If the input_parquet_paths vector has only one value, the single file input
	// path, and parquet_path_is_dir is false, then execute only a single thread on
	// the input file.
	//
	// Also execute a single thread in the case that the parquet path is a directory
	// with only one file. Otherwise, execute in multithreaded approach.
	uint8_t input_path_count = input_parquet_paths_.size();
	if (input_path_count == 1 || n_threads_ == 1)
	{
		n_threads_ = 1;
		threads_.push_back(std::thread(std::ref(*(ptm_vec_[0])), std::ref(output_base_path_), 
			std::ref(output_base_name_), std::ref(input_parquet_paths_), is_multithreaded_));
		while (!ptm_vec_[0]->completion_status())
		{
			std::this_thread::sleep_for(worker_wait_);
		}
		threads_[0].join();
	}
	else
	{
		is_multithreaded_ = true;

		if (input_path_count < n_threads_)
			n_threads_ = input_path_count;

		// Determine the number of files that each thread should process.
		uint8_t files_per_thread = uint8_t(ceil(float(input_path_count)/ n_threads_));
		//printf("files_per_thread: %hhu\n", files_per_thread);

		// Assign files to each thread.
		std::vector<std::vector<std::string>> thread_parquet_paths;
		uint8_t counter = 0;
		bool should_break = false;
		for (int thread_index = 0; thread_index < n_threads_; thread_index++)
		{
			std::vector<std::string> temp_vec;
			thread_parquet_paths.push_back(temp_vec);
			for (int i = 0; i < files_per_thread; i++)
			{
				//printf("thread %d getting %s\n", thread_index, input_parquet_paths[counter].c_str());
				thread_parquet_paths[thread_index].push_back(input_parquet_paths_[counter]);
				counter++;
				if (counter == input_path_count)
				{
					should_break = true;
					if (thread_index < n_threads_ - 1)
					{
						n_threads_ = thread_index + 1;
						printf("All files have been distributed to first %d threads. "
							"No more threads necessary.\n", thread_index + 1);
					}
					break;
				}
			}
			if (should_break)
				break;
		}

		// Keep track of thread status.
		std::vector<bool> has_joined(n_threads_, false);

		// Start threads.
		bool debug = false;
		for (int thread_index = 0; thread_index < n_threads_; thread_index++)
		{
			printf("\nThread %d processing:\n", thread_index);
			for (int i = 0; i < thread_parquet_paths[thread_index].size(); i++)
				printf("%s\n", thread_parquet_paths[thread_index][i].c_str());
			if (!debug)
			{
				threads_.push_back(std::thread(std::ref(*(ptm_vec_[thread_index])), 
					std::ref(output_base_path_), std::ref(output_base_name_), 
					std::ref(thread_parquet_paths[thread_index]), is_multithreaded_));
			}
			std::this_thread::sleep_for(worker_start_offset_);
		}

		if (!debug)
		{
			// Wait for threads to join.
			bool all_joined = false;
			while (!all_joined)
			{
				all_joined = true;
				for (int thread_index = 0; thread_index < n_threads_; thread_index++)
				{
					if (!has_joined[thread_index])
					{
						if (ptm_vec_[thread_index]->completion_status())
						{
							threads_[thread_index].join();
							has_joined[thread_index] = true;
						}
						else
							all_joined = false;
					}
					std::this_thread::sleep_for(worker_wait_);
				}
			}
		}

	}

	return 0;
}