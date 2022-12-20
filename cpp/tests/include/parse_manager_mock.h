#ifndef PARSE_MANAGER_MOCK_H_
#define PARSE_MANAGER_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parse_manager.h"
#include "managed_path.h"
#include "parser_metadata.h"
#include "parser_config_params.h"

class MockParseManagerFunctions : public ParseManagerFunctions
{
   public:
      MockParseManagerFunctions() : ParseManagerFunctions() {}
      MOCK_METHOD5(ActivateWorker, bool(WorkUnit* work_unit, std::vector<uint16_t>& active_workers,
			const uint16_t& worker_index, bool append_mode, const uint64_t& read_pos));
      MOCK_METHOD3(JoinWorker, bool(WorkUnit* work_unit, std::vector<uint16_t>& active_workers,
			const uint16_t& active_worker_index));
      MOCK_METHOD4(IngestUserConfig, void(const ParserConfigParams& user_config,
			const uint64_t& ch10_file_size, uint64_t& chunk_bytes, uint16_t& worker_count));

      MOCK_METHOD7(MakeWorkUnits, bool(std::vector<WorkUnit>& work_units, const uint16_t& worker_count,
			const uint64_t& read_size, const uint32_t& append_read_size,
			const uint64_t& total_size, std::ifstream& ch10_input_stream, const ParserPaths* parser_paths));

      MOCK_METHOD2(OpenCh10File, bool(const ManagedPath& input_path, std::ifstream& input_stream));
};


class MockParseManager : public ParseManager
{
   public:
    MockParseManager() : ParseManager() {}
    MOCK_METHOD3(Configure, int(ManagedPath input_ch10_file_path, ManagedPath output_dir,
                   const ParserConfigParams& user_config));
    MOCK_METHOD1(Parse, int(const ParserConfigParams& user_config));
    MOCK_METHOD2(RecordMetadata, bool(ManagedPath input_ch10_file_path,
                  const ParserConfigParams& user_config));
    MOCK_METHOD6(StartThreads, int(bool append_mode, std::vector<uint16_t>& active_workers_vec,
         const uint16_t& effective_worker_count, const ParserConfigParams& user_config, 
			std::vector<WorkUnit*>& work_units, ParseManagerFunctions* pfm));
    MOCK_METHOD4(StopThreads, bool(std::vector<WorkUnit*>& work_units,
         std::vector<uint16_t>& active_workers_vec, int worker_shift_wait, ParseManagerFunctions* pfm));
};

class MockWorkUnit : public WorkUnit
{
   public:
      MockWorkUnit() : WorkUnit() {}

      MOCK_METHOD2(CheckConfiguration, bool(bool append_mode, const uint64_t& read_pos));
      MOCK_METHOD0(Activate, void());
      MOCK_METHOD0(Join, void());
      MOCK_CONST_METHOD0(GetReadBytes, const uint64_t&());
      MOCK_CONST_METHOD0(IsComplete, bool());
};


#endif  // PARSE_MANAGER_MOCK_H_
