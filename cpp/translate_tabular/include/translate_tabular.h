#ifndef TRANSLATE_TABULAR_H_
#define TRANSLATE_TABULAR_H_

// Include first!
#include "translate_tabular_context_base.h"
//
#include <thread>
#include <chrono>
#include <cstdint>
#include <cmath>
#include <memory>
#include <vector>
#include <string>
#include "spdlog/spdlog.h"
#include "managed_path.h"
#include "translation_manager.h"

class TranslateTabular
{
   private:
    // Count of maximum concurrent threads
    size_t n_threads_;

    // Vector to hold thread objects. Each activated thread
    // will process at least one input file.
    // std::vector<std::thread> thread_vec_;

    // List of files which contain raw data and intend to be translated
    std::vector<ManagedPath> file_list_;
    bool is_file_list_valid_;

    // User-assigned output directory and base name
    ManagedPath output_dir_;
    ManagedPath output_base_name_;

    // Pointer to TranslateTabularContextBase which points to derived object
    // relevant to the data being translated.
    std::shared_ptr<TranslateTabularContextBase> ctx_;

    // TranslationManager executes the functions defined in the class
    // derived from TranslateTabularContextBase. It executes on the primary
    // functions which check configuration, open and read files, create tables
    // translate data into tables, etc. One instance of TranslationManager per
    // thread.
    std::vector<std::shared_ptr<TranslationManager>> manager_vec_;

   public:
    const std::vector<ManagedPath>& file_list;
    const bool& is_file_list_valid;

    TranslateTabular(size_t n_threads, std::shared_ptr<TranslateTabularContextBase> ctx);

    /*
    Set input files from which input raw (read: un-tranlsated) data will be read. 
    User ought to validate that files exist prior to submission.
    
    Args:
        input_files             --> Vector of file paths 
        input_file_extension    --> Extension of input data files which are to
                                    be read and translated. Includes
                                    leading period.

    Return:
        True if input_files vector has size greater than zero and each
        file has the correct extension; false otherwise.
    */
    bool SetInputFiles(const std::vector<ManagedPath>& file_list,
                       std::string input_file_extension);

    /*
    Set output directory for translated results. User is responsible to confirm
    that output dir has been created. Also set the base name of the output files.

    Args:
        output_dir          --> Output directory
        output_base_name    --> Base name of output files (without postfix or extension)
    */
    void SetOutputDir(const ManagedPath& output_dir,
                      const ManagedPath& output_base_name);

    /*
    Primary function which executes preparation and translation.

    Return:
        False if a failure of some kind occurred. True if no failures occur.
    */
    bool Translate();

    /*
    Get the vector of TranslationManager objects. Can be used to access
    data in the Context (TranslateTabularContextBase or a derived class).

    Return:
        manager_vec_, vector of TranslationManager objects.
    */
    std::vector<std::shared_ptr<TranslationManager>> GetManagers()
    {
        return manager_vec_;
    }

    ///////////////////////////////////////////////////////////////////////////
    //                          Internal functions
    ///////////////////////////////////////////////////////////////////////////

    /*
    Check context is configured and that input files exist and have correct
    extensions.

    Args:
        context     --> Configured derived class pointing to TranslateTabularContextBase

    Return:
        True if configured correctly. False otherwise.
    */
    bool CheckConfiguration(std::shared_ptr<TranslateTabularContextBase> context);

    /*
    Allocate input files to each of the threads.

    Args:
        thread_count            --> Count of threads to be use for translation
        input_files             --> Vector of input file paths 
        thread_paths            --> Vector of vector of paths in which the index 
                                    is the thread index. This is the product of 
                                    this function.
        required_thread_count   --> Count of the threads required to translate all 
                                    input paths. Equal to thread_paths.size(). May
                                    be less than or equal to thread_count. 
            
    */
    void AssignFilesToThreads(size_t thread_count, const std::vector<ManagedPath>& input_files,
                              std::vector<std::vector<ManagedPath>>& thread_paths, size_t& required_thread_count);

    /*
    Create a vector of TranslationManager objects and it's target object, a shared_ptr to
    object derived from TranslateTabularContextBase for each thread. This function
    assumes the context object has a Clone() method.

    Args:
        context         --> Configured derived class pointing to TranslateTabularContextBase
        thread_count    --> Count of threads to be used for translation
        file_paths      --> Input files to be operated on sequentially by TranslationManager 
                            in order of increasing vector index. The vector associated with
                            each index corresponds with the files to be ingested by the 
                            thread of the same index.
        output_dir      --> Base output dir
        output_base_name--> base name of output file
        tm_obj          --> Vector of created TranslationManager objects

    Return:
        True if no errors occur; false otherwise.
    */
    bool CreateTranslationManagerObjects(std::shared_ptr<TranslateTabularContextBase> context,
                                         size_t thread_count, const std::vector<std::vector<ManagedPath>>& file_paths,
                                         const ManagedPath& output_dir, const ManagedPath& output_base_name,
                                         std::vector<std::shared_ptr<TranslationManager>>& tm_obj_vec);

    /*
    Execute the operator() function associated with each of the TranslationManager
    objects represented in the tm_obj_vec vector in a thread and record the
    newly created thread objects in the threads vector.

    Args:
        threads     --> Container in which activated threads will be stored
        tm_obj_vec  --> TranslationManager objects which will be activated 
                        in each thread. Assumes that each object has an
                        operator() function.

    */
    void StartThreads(std::vector<std::thread>& threads,
                      std::vector<std::shared_ptr<TranslationManager>>& tm_obj_vec);

    /*
    Wait for each TranslationManager object to indicate completion 
    and join the relevant thread.

    Args:
        threads     --> Container in which activated threads are stored
        tm_obj_vec  --> TranslationManager objects for which the
                        operator() function has been called.

    */
    void JoinThreads(std::vector<std::thread>& threads,
                     std::vector<std::shared_ptr<TranslationManager>>& tm_obj_vec);
};

#endif  // #define TRANSLATE_TABULAR_H_