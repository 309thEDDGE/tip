#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "metadata.h"

using fspath = std::filesystem::path;

class MetadataTest : public ::testing::Test
{
protected:

	Metadata md_;
	std::string base_file_name_;
	fspath md_path_;
	std::string expected_md_string_;
	MetadataTest() : base_file_name_(""), md_(), md_path_("/home/usr/lib64"),
		expected_md_string_("")
	{

	}
	void SetUp() override
	{

	}

};

TEST_F(MetadataTest, GetYamlMetadataPathEmptyFileName)
{
	base_file_name_ = "";
	fspath outdir1("C:\\Users\\Admin\\Documents");
	md_path_ = md_.GetYamlMetadataPath(outdir1, base_file_name_);
	EXPECT_EQ(md_path_.parent_path().string(), outdir1.string());
	EXPECT_EQ(md_path_.filename().string(), "_metadata.yaml");

	fspath outdir2("/home/user/blah");
	md_path_ = md_.GetYamlMetadataPath(outdir2, base_file_name_);
	EXPECT_EQ(md_path_.parent_path().string(), outdir2.string());
	EXPECT_EQ(md_path_.filename().string(), "_metadata.yaml");
}	

TEST_F(MetadataTest, GetYamlMetadataPathEmptyOutDir)
{
	base_file_name_ = "metadata";
	fspath outdir1("");
	md_path_ = md_.GetYamlMetadataPath(outdir1, base_file_name_);
	EXPECT_EQ(md_path_.string(), "metadata.yaml");
}

TEST_F(MetadataTest, GetYamlMetadataPathEmptyOutDirAndFileName)
{
	base_file_name_ = "";
	fspath outdir1("");
	md_path_ = md_.GetYamlMetadataPath(outdir1, base_file_name_);
	EXPECT_EQ(md_path_.string(), "_metadata.yaml");
}

TEST_F(MetadataTest, GetMetadataStringSingleKeyValuePair)
{
	std::string key("document_1");
	int val = 100231;
	md_.RecordSingleKeyValuePair(key, val);

	expected_md_string_ = 
		"---\n"
		"document_1: 100231\n"
		"...\n";
	EXPECT_EQ(md_.GetMetadataString(), expected_md_string_);
}

TEST_F(MetadataTest, GetMetadataStringSimpleMap)
{
	std::string map_name("this is the map name");
	std::map<uint64_t, int32_t> input_map = {
		{33, -10},
		{90, 2111}
	};
	md_.RecordSimpleMap(input_map, map_name);

	expected_md_string_ =
		"---\n"
		"this is the map name:\n"
		"  33: -10\n"
		"  90: 2111\n"
		"...\n";
	EXPECT_EQ(md_.GetMetadataString(), expected_md_string_);
}

TEST_F(MetadataTest, GetMetadataStringCompoundMapToVector)
{
	std::string map_name("this is the map name");
	std::map<int, std::vector<std::string>> input_map = {
		{33, {"hello", "it's", "me"}},
		{-90, {"Sprocket is", "my dog"}}
	};
	md_.RecordCompoundMapToVector(input_map, map_name);

	// Ensure that the output is in the order the map will place
	// the values, likely in ascending numerical order.
	expected_md_string_ =
		"---\n"
		"this is the map name:\n"
		"  -90: [Sprocket is, my dog]\n"
		"  33: [hello, it\'s, me]\n"
		"...\n";
	EXPECT_EQ(md_.GetMetadataString(), expected_md_string_);
}

TEST_F(MetadataTest, GetMetadataStringCompoundMapToSet)
{
	std::string map_name("this is the map name");
	std::map<uint16_t, std::set<int>> input_map = {
		{33, {123, 80, -10}},
		{9, {10000, 981, -11}}
	};
	md_.RecordCompoundMapToSet(input_map, map_name);

	/// Ensure set is in ascending order.
	expected_md_string_ =
		"---\n"
		"this is the map name:\n"
		"  9: [-11, 981, 10000]\n"
		"  33: [-10, 80, 123]\n"
		"...\n";
	EXPECT_EQ(md_.GetMetadataString(), expected_md_string_);
}

TEST_F(MetadataTest, GetMetadataStringAllRecordTypes)
{
	std::string map_name1("ui16_to_setstr_map");
	std::map<uint16_t, std::set<std::string>> input_map1 = {
		{33, {"one two three", "maybe", "again"}},
		{9, {"c", "d", "e", "f"}}
	};
	md_.RecordCompoundMapToSet(input_map1, map_name1);

	std::string map_name2("int_to_vecbool_map");
	std::map<int, std::vector<bool>> input_map2 = {
		{-331, {true, true, false, true}},
		{-20, {false, true, true}},
		{65, {true, true}}
	};
	md_.RecordCompoundMapToVector(input_map2, map_name2);

	std::string map_name3("simple_map_of_str_to_uint32_t");
	std::map<std::string, uint32_t> input_map3 = {
		{"val1", 3343},
		{"val 2", 55}
	};
	md_.RecordSimpleMap(input_map3, map_name3);

	int key = 1900;
	int64_t val = -2193933;
	md_.RecordSingleKeyValuePair(key, val);

	// Ensure set is in ascending order and map keys
	// are in ascending order. 
	expected_md_string_ =
		"---\n"
		"ui16_to_setstr_map:\n"
		"  9: [c, d, e, f]\n"
		"  33: [again, maybe, one two three]\n"
		"\n"
		"int_to_vecbool_map:\n"
		"  -331: [true, true, false, true]\n"
		"  -20: [false, true, true]\n"
		"  65: [true, true]\n"
		"\n"
		"simple_map_of_str_to_uint32_t:\n"
		"  val 2: 55\n"
		"  val1: 3343\n"
		"\n"
		"1900: -2193933\n"
		"...\n";
	EXPECT_EQ(md_.GetMetadataString(), expected_md_string_);
}

TEST_F(MetadataTest, GetMetadataStringCompoundMapToVectorOfSet)
{
	std::string map_name("this is the map name");
	std::map<uint16_t, std::vector<std::set<int>>> input_map = {
		{33, {{123, 80}, {-10, -12, 13}}},
		{9, {{10000, 981}, {-11, 233, 3}, {50, 55}}}
	};
	md_.RecordCompoundMapToVectorOfSet<uint16_t, int>(input_map, map_name);

	YAML::Node node = YAML::Load(md_.GetMetadataString());

	// Check map name
	EXPECT_TRUE(node[map_name]);

	// Check sub map names.
	YAML::Node input_map_node = node[map_name];
	EXPECT_TRUE(input_map_node["9"] && input_map_node["33"]);

	// Check vector of sets.
	EXPECT_THAT(input_map_node["9"][0].as<std::vector<int>>(), 
		::testing::UnorderedElementsAreArray(input_map[9][0]));
	EXPECT_THAT(input_map_node["9"][1].as<std::vector<int>>(),
		::testing::UnorderedElementsAreArray(input_map[9][1]));
	EXPECT_THAT(input_map_node["9"][2].as<std::vector<int>>(),
		::testing::UnorderedElementsAreArray(input_map[9][2]));

	EXPECT_THAT(input_map_node["33"][0].as<std::vector<int>>(),
		::testing::UnorderedElementsAreArray(input_map[33][0]));
	EXPECT_THAT(input_map_node["33"][1].as<std::vector<int>>(),
		::testing::UnorderedElementsAreArray(input_map[33][1]));
}