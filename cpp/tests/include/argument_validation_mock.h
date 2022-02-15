#ifndef ARGUMENT_VALIDATION_MOCK_H_
#define ARGUMENT_VALIDATION_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "argument_validation.h"

class MockArgumentValidation : public ArgumentValidation
{
   public:
    MockArgumentValidation() : ArgumentValidation() {}
    MOCK_CONST_METHOD2(CheckExtension, bool(const std::string& input_path, 
      std::vector<std::string> exts));
    MOCK_CONST_METHOD2(ValidateInputFilePath, bool(std::string input_path, ManagedPath& mp_input_path));
    MOCK_CONST_METHOD2(ValidateDirectoryPath, bool(std::string dir, ManagedPath& mp_dir));
    MOCK_CONST_METHOD4(ValidateDefaultInputFilePath, bool(const ManagedPath& default_base_path,
                                      const std::string& user_base_path, std::string file_name,
                                      ManagedPath& full_path));
    MOCK_CONST_METHOD4(ValidateDefaultOutputDirectory, bool(const ManagedPath& default_output_dir,
                                        const std::string& user_output_dir, ManagedPath& final_output_dir,
                                        bool create_dir));
};


#endif  // ARGUMENT_VALIDATION_MOCK_H_
