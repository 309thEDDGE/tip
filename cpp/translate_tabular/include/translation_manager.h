#ifndef TRANSLATION_MANAGER_H_
#define TRANSLATION_MANAGER_H_

// Include first!
#include "translate_tabular_context_base.h"
//
#include <memory>
#include <vector>
#include "spdlog/spdlog.h"
#include "translate_status.h"
#include "managed_path.h"

class TranslationManager
{
   private:
    // Pointer to class derived from TranslateTabularContextBase,
    // which defines the primary functions to read and translate
    // input data.
    std::shared_ptr<TranslateTabularContextBase> context_;

    // Used to uniquely label output files and print/log output
    int thread_index_;

    // Input file paths. The source of raw data to be translated.
    // Operated on sequentially, in order of increasing vector index.
    std::vector<ManagedPath> input_paths_;

    // Output directory and base path name.
    ManagedPath output_dir_;
    ManagedPath output_base_name_;

    // Status, whether complete with or without errors
    std::atomic<bool> complete_;

    // True if no errors, false otherwise
    std::atomic<bool> success_;

    TranslateStatus status_;

   public:
    const int& thread_index;
    const std::atomic<bool>& complete;
    const std::atomic<bool>& success;

    TranslationManager() : context_(nullptr), thread_index_(-1), complete_(false), success_(false), thread_index(thread_index_), complete(complete_), success(success_), status_(TranslateStatus::NONE)
    {
    }

    /*
    Function which is called when the thread is activated. The primary
    loop which executes functions in the context (object derived from
    TranslateTabularContextBase) to accomplish translation.
    */
    void operator()();

    /*
    Assign the TranslateTabularContextBase-derived object which defines
    the standard set of functions calls executed by TranslationManager to
    carry out translation.

    Args:
        context     --> Pointer to TranslateTabularContextBase which points
                        to a derived class which defines the required functions
        thread_index--> Assign an index enrich log output and file labels
        input_paths --> Input file paths, to be operated on sequentially
        output_dir      --> Base output dir
        output_base_name--> base name of output file
    */
    void Configure(std::shared_ptr<TranslateTabularContextBase> context,
                   int thread_index, const std::vector<ManagedPath>& input_paths,
                   const ManagedPath& output_dir, const ManagedPath& output_base_name);

    /*
    Get the pointer to the TranslateTabularContextBase object with which
    this object was configured.

    Return:
        Pointer to context
    */
    std::shared_ptr<TranslateTabularContextBase> GetContext()
    {
        return context_;
    }
};

#endif  // #ifndef TRANSLATION_MANAGER_H_