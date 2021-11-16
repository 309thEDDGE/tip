#include "translate_tabular.h"

TranslateTabular::TranslateTabular(size_t n_threads,
                                   std::shared_ptr<TranslateTabularContextBase> ctx) : n_threads_(n_threads), ctx_(ctx), file_list(file_list_), is_file_list_valid_(false), is_file_list_valid(is_file_list_valid_)
{
}

bool TranslateTabular::SetInputFiles(const std::vector<ManagedPath>& file_list,
                                     std::string input_file_extension)
{
    if (file_list.size() == 0)
    {
        SPDLOG_WARN("Input file vector has size zero");
        return false;
    }

    if (input_file_extension[0] != '.')
    {
        SPDLOG_WARN("Input file extension string must have leading period");
        return false;
    }

    for (std::vector<ManagedPath>::const_iterator it = file_list.cbegin();
         it != file_list.cend(); ++it)
    {
        if (it->extension() != input_file_extension)
        {
            SPDLOG_WARN("Input file \"{:s}\" does not have extension \"{:s}\"",
                        it->RawString(), input_file_extension);
            return false;
        }
    }

    file_list_ = file_list;
    is_file_list_valid_ = true;
    return true;
}

void TranslateTabular::SetOutputDir(const ManagedPath& output_dir,
                                    const ManagedPath& output_base_name)
{
    output_dir_ = output_dir;
    output_base_name_ = output_base_name;
}

bool TranslateTabular::Translate()
{
    if (!CheckConfiguration(ctx_))
        return false;

    size_t required_thread_count = 0;
    std::vector<std::vector<ManagedPath>> thread_file_paths;
    AssignFilesToThreads(n_threads_, file_list_, thread_file_paths, required_thread_count);

    // It's possible that required_that_count is less than n_threads_, else it is
    // equal. required_thread_count is equal to thread_file_paths.size(), so files
    // may not exist for ingestion for thread index = n_threads_ - 1. Set n_threads_
    // equal to required_thread_count to avoid this problem.
    SPDLOG_INFO("Thread count adjusted: {:d}", required_thread_count);
    n_threads_ = required_thread_count;

    if (!CreateTranslationManagerObjects(ctx_, n_threads_, thread_file_paths,
                                         output_dir_, output_base_name_, manager_vec_))
        return false;

    std::vector<std::thread> thread_vec;
    StartThreads(thread_vec, manager_vec_);

    JoinThreads(thread_vec, manager_vec_);

    // Print thread success results
    std::string status_string = "";
    for (std::vector<std::shared_ptr<TranslationManager>>::const_iterator it =
             manager_vec_.cbegin();
         it != manager_vec_.cend(); ++it)
    {
        if ((*it)->success)
            status_string = "SUCCESS";
        else
            status_string = "FAIL";

        SPDLOG_INFO("TranslationManager associated with thread {:d} status: {:s}",
                    (*it)->thread_index, status_string);
    }
    return true;
}

bool TranslateTabular::CheckConfiguration(std::shared_ptr<TranslateTabularContextBase> context)
{
    if (!context->IsConfigured())
    {
        SPDLOG_WARN("Context is not configured");
        return false;
    }

    if (!is_file_list_valid_)
    {
        SPDLOG_WARN("File list has not been set or is not valid. Use SetInputFiles()");
        return false;
    }

    // File is present
    for (std::vector<ManagedPath>::const_iterator it = file_list_.cbegin();
         it != file_list_.cend(); ++it)
    {
        if (!it->is_regular_file())
        {
            SPDLOG_WARN("Input file {:s} does not exist", it->RawString());
            return false;
        }
    }
    return true;
}

void TranslateTabular::AssignFilesToThreads(size_t thread_count,
                                            const std::vector<ManagedPath>& input_files,
                                            std::vector<std::vector<ManagedPath>>& thread_paths,
                                            size_t& required_thread_count)
{
    if (input_files.size() < thread_count)
        thread_count = static_cast<size_t>(input_files.size());

    required_thread_count = thread_count;

    // Determine the number of files that each thread should process.
    int files_per_thread = static_cast<int>(
        ceil(static_cast<float>(input_files.size()) / thread_count));

    int counter = 0;
    bool should_break = false;
    for (int thread_index = 0; thread_index < thread_count; thread_index++)
    {
        std::vector<ManagedPath> temp_vec;
        thread_paths.push_back(temp_vec);
        for (int i = 0; i < files_per_thread; i++)
        {
            thread_paths[thread_index].push_back(input_files[counter]);
            counter++;
            if (counter == input_files.size())
            {
                should_break = true;
                if (thread_index < thread_count - 1)
                {
                    required_thread_count = thread_index + 1;
                }
                break;
            }
        }
        if (should_break)
            break;
    }
    SPDLOG_INFO("Input files allotted to threads");
}

bool TranslateTabular::CreateTranslationManagerObjects(
    std::shared_ptr<TranslateTabularContextBase> context,
    size_t thread_count, const std::vector<std::vector<ManagedPath>>& file_paths,
    const ManagedPath& output_dir, const ManagedPath& output_base_name,
    std::vector<std::shared_ptr<TranslationManager>>& tm_obj_vec)
{
    // Context must be initialized
    if (!context->IsConfigured())
    {
        SPDLOG_WARN("Context is not initialized");
        return false;
    }

    // file_paths size and thread_count must be equal
    if (thread_count != file_paths.size())
    {
        SPDLOG_WARN("thread_count not equal to file_paths.size()");
        return false;
    }

    // Clone context objects, create and configure TranslationManager objects, and
    // add them to vector.
    for (size_t i = 0; i < thread_count; i++)
    {
        std::shared_ptr<TranslationManager> temp_tm = std::make_shared<TranslationManager>();
        std::shared_ptr<TranslateTabularContextBase> temp_context = context->Clone();
        temp_tm->Configure(temp_context, i, file_paths[i], output_dir, output_base_name);
        tm_obj_vec.push_back(temp_tm);
    }
    SPDLOG_INFO("TranslationManager objects created");
    return true;
}

void TranslateTabular::StartThreads(std::vector<std::thread>& threads,
                                    std::vector<std::shared_ptr<TranslationManager>>& tm_obj_vec)
{
    for (size_t thread_index = 0; thread_index < tm_obj_vec.size(); thread_index++)
    {
        SPDLOG_INFO("Thread {:d} started", thread_index);
        threads.push_back(std::thread(std::ref(*(tm_obj_vec.at(thread_index)))));
    }
    SPDLOG_INFO("Threads started");
}

void TranslateTabular::JoinThreads(std::vector<std::thread>& threads,
                                   std::vector<std::shared_ptr<TranslationManager>>& tm_obj_vec)
{
    bool all_joined = false;
    while (!all_joined)
    {
        all_joined = true;
        for (size_t thread_index = 0; thread_index < threads.size(); thread_index++)
        {
            if (threads[thread_index].joinable())
            {
                if (tm_obj_vec[thread_index]->complete)
                {
                    threads[thread_index].join();
                    SPDLOG_INFO("Joined thread {:d}", thread_index);
                }
                else
                    all_joined = false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    SPDLOG_INFO("Joined threads");
}
