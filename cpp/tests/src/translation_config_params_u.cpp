#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "translation_config_params.h"

class TranslationConfigParamsTest : public ::testing::Test
{
protected:

	TranslationConfigParams config;
	std::ofstream file;
	std::string filepath = "testfile.yaml";
	TranslationConfigParamsTest()
	{
		std::ifstream infile(filepath);
		if (infile.good())
			remove(filepath.c_str());
		file.open(filepath);
	}
	~TranslationConfigParamsTest()
	{
		file.close();
		remove(filepath.c_str());
	}

	void SetUp() override
	{

	}

	template <typename Map>
	bool map_compare(Map const& lhs, Map const& rhs) {
		return lhs.size() == rhs.size()
			&& std::equal(lhs.begin(), lhs.end(),
				rhs.begin());
	}

};

TEST_F(TranslationConfigParamsTest, NonexistantFile)
{
	bool status = config.Initialize("badpath.yaml");
	ASSERT_FALSE(status);
}

TEST_F(TranslationConfigParamsTest, InitializeValidEntry)
{
	file << "use_tmats_busmap : true\n";
	file << "tmats_busname_corrections :\n  {\n  1553a: ABUS,\n  1553b: BBUS\n  }\n";
	file << "select_specific_messages :\n  [\n   message1, \n   message2\n  ]\n";
	file << "exit_after_table_creation : false\n";
	file << "stop_after_bus_map : false\n";
	file << "vote_threshold : 1\n";
	file << "vote_method_checks_tmats : false\n";
	file << "bus_name_exclusions :\n  [\n   BusA, \n   BusB\n  ]\n";
	file << "prompt_user : false\n";
	file << "translate_thread_count: 1\n";
	file.close();
	
	bool status = config.Initialize(filepath);
	
	
	std::map<std::string, std::string> expected_tmats_busname_corrections_ = { {"1553a", "ABUS"}, {"1553b", "BBUS"} };
	std::map<std::string, std::set<uint64_t>> expected_comet_busname_replacement_ = { {"ABUS", std::set<uint64_t>({1,2,3,4})}, {"BBUS", std::set<uint64_t>({5,6,7})} };
	ASSERT_TRUE(status);
	EXPECT_EQ(config.use_tmats_busmap_, true);
	EXPECT_TRUE(map_compare(expected_tmats_busname_corrections_, config.tmats_busname_corrections_));
	EXPECT_THAT(config.select_specific_messages_, ::testing::ElementsAre("message1", "message2"));
	EXPECT_EQ(config.exit_after_table_creation_, false);
	EXPECT_EQ(config.stop_after_bus_map_, false);
	EXPECT_EQ(config.prompt_user_, false);
	EXPECT_THAT(config.bus_name_exclusions_, ::testing::ElementsAre("BusA", "BusB"));
	EXPECT_EQ(config.translate_thread_count_, 1);
}

TEST_F(TranslationConfigParamsTest, InitializeEmptyList)
{
	file << "use_tmats_busmap : true\n";
	file << "tmats_busname_corrections :\n  {}\n";
	file << "select_specific_messages :\n  []\n";
	file << "exit_after_table_creation : false\n";
	file << "stop_after_bus_map : false\n";
	file << "vote_threshold : 1\n";
	file << "vote_method_checks_tmats : false\n";
	file << "prompt_user : false\n";
	file << "bus_name_exclusions :\n  [\n   BusA, \n   BusB\n  ]\n";
	file << "comet_busmap_replacement :\n  {}\n";
	file << "translate_thread_count: 1\n";
	file.close();

	bool status = config.Initialize(filepath);

	ASSERT_TRUE(status);
	EXPECT_TRUE(config.select_specific_messages_.empty());
}

TEST_F(TranslationConfigParamsTest, InitializeWithConfigStringValidConfig)
{
	std::string yaml_matter = {
		"use_tmats_busmap : true\n"
		"tmats_busname_corrections :\n  {\n  1553a: ABUS,\n  1553b: BBUS\n  }\n"
		"select_specific_messages :\n  [\n   message1, \n   message2\n  ]\n"
		"exit_after_table_creation : false\n"
		"stop_after_bus_map : false\n"
		"vote_threshold : 1\n"
		"vote_method_checks_tmats : false\n"
		"bus_name_exclusions :\n  [\n   BusA, \n   BusB\n  ]\n"
		"prompt_user : false\n"
		"translate_thread_count: 1\n"
	};

	bool status = config.InitializeWithConfigString(yaml_matter);

	std::map<std::string, std::string> expected_tmats_busname_corrections_ = { {"1553a", "ABUS"}, {"1553b", "BBUS"} };
	std::map<std::string, std::set<uint64_t>> expected_comet_busname_replacement_ = { {"ABUS", std::set<uint64_t>({1,2,3,4})}, {"BBUS", std::set<uint64_t>({5,6,7})} };
	ASSERT_TRUE(status);
	EXPECT_EQ(config.use_tmats_busmap_, true);
	EXPECT_TRUE(map_compare(expected_tmats_busname_corrections_, config.tmats_busname_corrections_));
	EXPECT_THAT(config.select_specific_messages_, ::testing::ElementsAre("message1", "message2"));
	EXPECT_EQ(config.exit_after_table_creation_, false);
	EXPECT_EQ(config.stop_after_bus_map_, false);
	EXPECT_EQ(config.prompt_user_, false);
	EXPECT_THAT(config.bus_name_exclusions_, ::testing::ElementsAre("BusA", "BusB"));
	EXPECT_EQ(config.translate_thread_count_, 1);
}

TEST_F(TranslationConfigParamsTest, InitializeWithConfigStringInvalidYaml)
{
	std::string yaml_matter = {
		"use_tmats_busmap : true\n"
		"tmats_busname_corrections :\n  {\n  1553a: ABUS,\n  1553b: BBUS\n  }\n"
		"select_specific_messages :\n  [\n   message1, \n   message2\n  ]\n"
		"exit_after_table_creation : false\n"
		"stop_after_bus_map : false" // no newline
		"vote_threshold : 1\n"
		"vote_method_checks_tmats : false\n"
		"bus_name_exclusions :\n  [\n   BusA, \n   BusB\n  ]\n"
		"prompt_user : false\n"
		"translate_thread_count: 1\n"
	};

	bool status = config.InitializeWithConfigString(yaml_matter);
	EXPECT_FALSE(status);
}

TEST_F(TranslationConfigParamsTest, InitializeWithConfigStringInvalidConfig)
{
	std::string yaml_matter = {
		"use_tmats_busmap : true\n"
		"tmats_busname_corrections :\n  {\n  1553a: ABUS,\n  1553b: BBUS\n  }\n"
		"select_specific_messages :\n  [\n   message1, \n   message2\n  ]\n"
		"exit_after_table_creation : false\n"
		"stop_after_bus_map : false\n"
		"vote_threshold : one\n" // can't be interpreted as an integer
		"vote_method_checks_tmats : false\n"
		"bus_name_exclusions :\n  [\n   BusA, \n   BusB\n  ]\n"
		"prompt_user : yes\n"
		"translate_thread_count: 1\n"
	};

	bool status = config.InitializeWithConfigString(yaml_matter);
	EXPECT_FALSE(status);
}