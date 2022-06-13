#ifndef PARSE_MANAGER_MOCK_H_
#define PARSE_MANAGER_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parse_manager.h"
#include "managed_path.h"
#include "parser_metadata.h"
#include "parser_config_params.h"

class MockParseManager : public ParseManager
{
   public:
    MockParseManager() : ParseManager() {}
    MOCK_METHOD3(Configure, bool(ManagedPath input_ch10_file_path, ManagedPath output_dir,
                   const ParserConfigParams& user_config));
    MOCK_METHOD1(Parse, bool(const ParserConfigParams& user_config));
    MOCK_METHOD2(RecordMetadata, bool(ManagedPath input_ch10_file_path,
                  const ParserConfigParams& user_config));
};


#endif  // PARSE_MANAGER_MOCK_H_
