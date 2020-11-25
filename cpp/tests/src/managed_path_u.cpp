#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "managed_path.h"
#include <fstream>

bool HasWindowsPrefix(std::string input_str)
{
	char backslash = '\\';
	char q_mark = '?';
	if (input_str[0] == backslash && input_str[1] == backslash
		&& input_str[2] == q_mark && input_str[3] == backslash)
	{
		return true;
	}
	return false;
}

TEST(ManagedPathTest, AmendPathInsertsPrefix)
{
	ManagedPath mp;

	// < 260
	std::vector<std::string> test_path_str = { "data", "Is", "not-here", "ok.txt" };

	// Build the path in a platform-independent way
	for (auto s : test_path_str)
		mp /= s;
	std::filesystem::path test_path(mp.RawString());

	EXPECT_TRUE(test_path.string().size() < 260);
	EXPECT_FALSE(HasWindowsPrefix(mp.AmendPath(test_path).string()));

	// = 260
	test_path_str = std::vector<std::string>({ "this","path","to-the-file","must_be",
		"_equivalent_to_260_chars_in_size", "this-is-a-long-part01",
		"this-is-a-long-part02", "this-is-a-long-part03\\this-is-a-long-part04",
		"this-is-a-long-part05", "this-is-a-long-part06", "this-is-a"});
	mp = ManagedPath();
	for (auto s : test_path_str)
		mp /= s;
	//printf("string len: %zu\n", mp.RawString().size());
	test_path = std::filesystem::path(mp.RawString());
	EXPECT_TRUE(test_path.string().size() == 260);
	EXPECT_FALSE(HasWindowsPrefix(mp.AmendPath(test_path).string()));

	// > 260
	test_path_str = std::vector<std::string>({ "this","path","to-the-file","must_be",
		"_equivalent_to_260_chars_in_size", "this-is-a-long-part01",
		"this-is-a-long-part02", "this-is-a-long-part03\\this-is-a-long-part04",
		"this-is-a-long-part05", "this-is-a-long-part06", "this-is-another-very-long-part" });
	mp = ManagedPath();
	for (auto s : test_path_str)
		mp /= s;
	test_path = std::filesystem::path(mp.RawString());
	EXPECT_TRUE(test_path.string().size() > 260);
	EXPECT_TRUE(HasWindowsPrefix(mp.AmendPath(test_path).string()));
}

TEST(ManagedPathTest, string)
{
	
	ManagedPath mp;

	// < 260
	std::vector<std::string> test_path_str = { "data", "Is", "not-here", "ok.txt" };

	// Build the path in a platform-independent way
	for (auto s : test_path_str)
		mp /= s;
	EXPECT_TRUE(mp.RawString().size() < 260);
	EXPECT_FALSE(HasWindowsPrefix(mp.string()));

	// = 260
	test_path_str = std::vector<std::string>({ "this","path","to-the-file","must_be",
		"_equivalent_to_260_chars_in_size", "this-is-a-long-part01",
		"this-is-a-long-part02", "this-is-a-long-part03\\this-is-a-long-part04",
		"this-is-a-long-part05", "this-is-a-long-part06", "this-is-a" });
	mp = ManagedPath();
	for (auto s : test_path_str)
		mp /= s;
	EXPECT_TRUE(mp.RawString().size() == 260);
	EXPECT_FALSE(HasWindowsPrefix(mp.string()));

	// > 260
	test_path_str = std::vector<std::string>({ "this","path","to-the-file","must_be",
		"_equivalent_to_260_chars_in_size", "this-is-a-long-part01",
		"this-is-a-long-part02", "this-is-a-long-part03\\this-is-a-long-part04",
		"this-is-a-long-part05", "this-is-a-long-part06", "this-is-another-very-long-part" });
	mp = ManagedPath();
	for (auto s : test_path_str)
		mp /= s;
	EXPECT_TRUE(mp.RawString().size() > 260);
	EXPECT_TRUE(HasWindowsPrefix(mp.string()));
}

TEST(ManagedPathTest, CreateDirectoryAlreadyExists)
{
	fs::path temp_path("test_dir");
	EXPECT_TRUE(fs::create_directory(temp_path));

	ManagedPath mp;
	mp /= ManagedPath(temp_path);

	EXPECT_TRUE(mp.create_directory());

	// Remove test dir.
	EXPECT_TRUE(fs::remove(temp_path));
}

TEST(ManagedPathTest, CreateDirectoryParentMustExist)
{
	ManagedPath mp;
	mp = mp / "test_dir" / "second-dir";

	ASSERT_FALSE(mp.create_directory());
}

TEST(ManagedPathTest, CreateDirectoryLongPath)
{
	/*
	This test was created using TDD. See the comments below that
	describe how failures were initially observed.
	*/
	ManagedPath mp;
	std::string root_dir_name = "test_dir";

	// Create the part of the path that's less than 261 chars using std::filesystem.
	mp /= ManagedPath(root_dir_name);
	mp = mp / "this_is_part_of_a_very_long_path_section00" / "this_is_part_of_a_very_long_path_section01";
	mp = mp / "this_is_part_of_a_very_long_path_section02" / "this_is_part_of_a_very_long_path_section03";
	EXPECT_TRUE(mp.RawString().size() < 261);

	fs::path small_path(mp.RawString());
	EXPECT_TRUE(fs::create_directories(small_path));

	// Extend the path beyond 260 and create the final path.
	mp = mp / "this_is_part_of_a_very_long_path_section04";

	// Code below fails with an exception on Windows.
	/*fs::path long_path(mp.RawString());
	EXPECT_TRUE(fs::create_directory(long_path));*/
	
	// This code, created to handle long paths, must succeed
	// for the test to pass.
	EXPECT_TRUE(mp.create_directory());

	// Remove long directory using ManagedPath remove function.
	// Note that if this function is not called to take the 
	// full path length to below 261 chars, the following call
	// to std::filesystem::remove_all will fail.
	// This test also tests ManagedPath::remove.
	EXPECT_TRUE(mp.remove());

	// Remove all remaining dirs using std::filesystem::remove_all.
	fs::path root_path(root_dir_name);
	EXPECT_TRUE(fs::remove_all(root_path));
}

//TEST(ManagedPathTest, CreateDirectoryMultipleRapidCalls)
//{
//	std::string root_dir_name = "test_dir";
//	
//	// Create current working directory ManagedPath object
//	ManagedPath root_dir;
//
//	// Append the test root dir.
//	root_dir /= root_dir_name;
//
//	// Create the base dir.
//	EXPECT_TRUE(root_dir.create_directory());
//
//	// Create a bunch of ManagedPath objects in preparation for
//	// rapid directory creation.
//	std::vector<ManagedPath> sub_dirs;
//	std::string sub_dir_base_name = "subdir";
//	int subdir_count = 500;
//	for (int i = 0; i < subdir_count; i++)
//	{
//		sub_dirs.push_back(root_dir / (sub_dir_base_name + std::to_string(i)));
//	}
//
//	// Create directories
//	for (int i = 0; i < subdir_count; i++)
//	{
//		// I expected this to fail, but it didn't
//		EXPECT_TRUE(fs::create_directory(fs::path(sub_dirs[i].RawString())));
//
//		// I expected that this call would not fail only if the 
//		// code in ManagedPath::create_directory that allows multiple
//		// fs::create_directory attempts is uncommented. This is not the case.
//		//EXPECT_TRUE(sub_dirs[i].create_directory());
//	}
//
//	// Remove all remaining dirs using std::filesystem::remove_all.
//	fs::path root_path(root_dir_name);
//	EXPECT_TRUE(fs::remove_all(root_path));
//}

TEST(ManagedPathTest, AppendOperator)
{
	// /=
	std::string s1 = "data";
	std::string s2 = "path";
	ManagedPath mp1(s1);
	ManagedPath mp2(s2);
	std::filesystem::path p1(s1);
	std::filesystem::path p2(s2);
	p1 /= p2;
	mp1 /= mp2;
	EXPECT_EQ(mp1.RawString(), p1.string());
}

TEST(ManagedPathTest, ConcatenateOperator)
{
	// /
	std::string s1 = "data";
	std::string s2 = "path";
	ManagedPath mp1(s1);
	ManagedPath mp2(s2);
	std::filesystem::path p1(s1);
	std::filesystem::path p2(s2);
	ManagedPath mp = mp1 / mp2;
	std::filesystem::path p = p1 / p2;
	EXPECT_EQ(mp.RawString(), p.string());
}

TEST(ManagedPathTest, AppendNoSeparator)
{
	// /
	std::string s1 = "data";
	std::string s2 = "path";
	ManagedPath mp1(s1);
	ManagedPath mp2(s2);
	std::filesystem::path p1(s1);
	std::filesystem::path p2(s2);
	mp1 += mp2;
	p1 += p2;
	EXPECT_EQ(mp1.RawString(), p1.string());
}
