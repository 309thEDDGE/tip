#include "gtest/gtest.h"
#include "parse_manager.h"
#include "parser_config_params.h"
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

class ParseManagerTest : public ::testing::Test
{
protected:

	
	ParserConfigParams config;
	std::string input_path = "";
	std::string output_path = "";
	ParseManager pm = ParseManager(input_path, output_path, &config);
	std::string filename = "_TMATS.txt";
	std::ifstream file;
	std::string line;
	

	ParseManagerTest() 
	{
		// delete previous file if it exists
		file.open(filename);
		if (file.good())
		{
			file.close();
			remove(filename.c_str());
		}
		file.close();
		
	}
	~ParseManagerTest()
	{
		// delete previous file if it exists
		file.open(filename);
		if (file.good())
		{
			file.close();
			remove(filename.c_str());
		}
		file.close();
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

TEST_F(ParseManagerTest, NoTMATSLeftFromPriorTests)
{
	file.open(filename);
	ASSERT_FALSE(file.good());
}

TEST_F(ParseManagerTest, NoTMATSPresent)
{
	std::vector<std::string> tmats;
	pm.ProcessTMATsTest(tmats);
	file.open(filename);
	// file shouldn't exist if tmats did
	// not exist
	ASSERT_FALSE(file.good());
}

TEST_F(ParseManagerTest, TMATSWritten)
{
	std::vector<std::string> tmats = { "line1\nline2\n", "line3\nline4\n" };
	pm.ProcessTMATsTest(tmats);
	file.open(filename);
	ASSERT_TRUE(file.good());
	std::ostringstream ss;
	ss << file.rdbuf(); 
	std::string test = ss.str();
	ASSERT_EQ(test, "line1\nline2\nline3\nline4\n");
}

TEST_F(ParseManagerTest, TMATSParsed)
{
	// R-x\TK1-n:channelID
	// R-x\DSI-n:Source
	// R-x\CDT-n:Type
	std::vector<std::string> tmats = { "line2;\n\n", 
										"R-1\\TK1-1:1;\n\n",
										"R-2\\TK1-2:2;\n\n",
										"R-2\\DSI-2:Bus2;\n\n",
										"R-1\\DSI-1:Bus1;\n",
										"R-3\\TK1-3:3;\n\n",
										"R-1\\CDT-1:type1;\n\n",
										"R-2\\CDT-2:type2;\n\n",
										"R-3\\CDT-3:type3;\n",
										"R-3\\DSI-3:Bus3;\n\n",
										"comment;\n",
										"Junk-1\\Junk1-1;\n\n",
									 };
	pm.ProcessTMATsTest(tmats);
	std::map<std::string,std::string> source_map_test = 
		pm.GetTMATsChannelIDToSourceMap();

	std::map<std::string, std::string> type_map_test =
		pm.GetTMATsChannelIDToTypeMap();

	std::map<std::string, std::string> source_map_truth = 
	{ {"1" , "Bus1"}, {"2" , "Bus2"}, {"3" , "Bus3"} };

	std::map<std::string, std::string> type_map_truth = 
	{ {"1" , "type1"}, {"2" , "type2"}, {"3" , "type3"} };

	ASSERT_TRUE(map_compare(source_map_test, source_map_truth));
	ASSERT_TRUE(map_compare(type_map_test, type_map_truth));
}
