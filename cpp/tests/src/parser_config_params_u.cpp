#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parser_config_params.h"
#include <thread>

class ParserConfigParamsTest : public ::testing::Test
{
protected:

	ParserConfigParams config;
	std::ofstream file;
	std::string filepath = "testfile.yaml";
	ParserConfigParamsTest()
	{
		std::ifstream infile(filepath);
		if (infile.good())
			remove(filepath.c_str());
		file.open(filepath);
	}
	~ParserConfigParamsTest() 
	{
		file.close();
		remove(filepath.c_str());
	}

	void SetUp() override
	{

	}

};

TEST_F(ParserConfigParamsTest, NonexistantFile)
{
	bool status = config.Initialize("badpath.yaml");
	ASSERT_FALSE(status);
}

TEST_F(ParserConfigParamsTest, ValidEntries)
{
	uint64_t thread_count_estimate = std::thread::hardware_concurrency();
	file << "ch10_packet_type:\n";
	file << "  MILSTD1553_FORMAT1: true\n";
	file << "  VIDEO_FORMAT0: true\n";
	file << "parse_chunk_bytes : 150\n";
	file << "parse_thread_count : " << thread_count_estimate << "\n";
	file << "max_chunk_read_count : 5\n";
	file << "worker_offset_wait_ms : 200\n";
	file << "worker_shift_wait_ms : 300\n";
	file.close();

	bool status = config.Initialize(filepath);

	ASSERT_TRUE(status);
	ASSERT_EQ(config.parse_chunk_bytes_, 150);
	ASSERT_EQ(config.parse_thread_count_, thread_count_estimate);
	ASSERT_EQ(config.max_chunk_read_count_, 5);
	ASSERT_EQ(config.worker_offset_wait_ms_, 200);
	ASSERT_EQ(config.worker_shift_wait_ms_, 300);
}