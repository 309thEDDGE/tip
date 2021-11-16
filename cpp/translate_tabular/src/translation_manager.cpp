#include "translation_manager.h"

void TranslationManager::Configure(std::shared_ptr<TranslateTabularContextBase> context,
                                   int thread_index, const std::vector<ManagedPath>& input_paths,
                                   const ManagedPath& output_dir, const ManagedPath& output_base_name)
{
    context_ = context;
    thread_index_ = thread_index;
    input_paths_ = input_paths;
    output_dir_ = output_dir;
    output_base_name_ = output_base_name;
}

void TranslationManager::operator()()
{
    success_ = true;
    complete_ = false;

    bool is_final_file = false;

    // Iterate over input files assigned to this thread
    for (size_t input_file_ind = 0; input_file_ind < input_paths_.size(); input_file_ind++)
    {
        if (input_file_ind == (input_paths_.size() - 1))
            is_final_file = true;

        SPDLOG_INFO("Thread {:d} working on input file: {:s}",
                    thread_index_, input_paths_.at(input_file_ind).RawString());

        status_ = context_->OpenInputFile(input_paths_.at(input_file_ind), thread_index_);
        if (status_ == TranslateStatus::FAIL)
        {
            success_ = false;
            break;
        }
        else if (status_ == TranslateStatus::CONTINUE)
            continue;

        status_ = context_->OpenOutputFile(output_dir_, output_base_name_,
                                           thread_index_);
        if (status_ == TranslateStatus::FAIL)
        {
            success_ = false;
            break;
        }
        else if (status_ == TranslateStatus::CONTINUE)
            continue;

        status_ = context_->ConsumeFile(thread_index_);
        if (status_ == TranslateStatus::FAIL)
        {
            success_ = false;
            break;
        }
        else if (status_ == TranslateStatus::CONTINUE)
            continue;

        status_ = context_->CloseInputFile(thread_index_);
        if (status_ == TranslateStatus::FAIL)
        {
            success_ = false;
            break;
        }
        else if (status_ == TranslateStatus::CONTINUE)
            continue;

        status_ = context_->CloseOutputFile(thread_index_, is_final_file);
        if (status_ == TranslateStatus::FAIL)
        {
            success_ = false;
            break;
        }
        else if (status_ == TranslateStatus::CONTINUE)
            continue;
    }

    complete_ = true;
}
