#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "managed_path.h"
#include <fstream>
#include <iostream>
#include <system_error>

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

std::vector<std::string> CreatePathWithLength(const ManagedPath& mp_relative, size_t desired_len)
{
	ManagedPath mp_new = mp_relative;
	std::vector<std::string> path_components;

	char long_component[] = "this-is-a-very-very-long-path-component%02d"; // 41
	std::string mid_component = "this_is_shorter"; // 15
	std::string short_component = "comp_short"; // 10
	std::string char_component = "a";
	bool first_time_char = true;
	char buff[50];
	int index = 0;
	size_t diff = 0;
	while ((diff = desired_len - mp_new.RawString().size()) > 0)
	{
		printf("diff = %zu\n", diff);
		if (diff >= 41 + 1)
		{
			sprintf(buff, long_component, index);
			index++;
			std::string temp_comp(buff);
			mp_new /= temp_comp;
			path_components.push_back(temp_comp);
		}
		else if (diff >= 15 + 1)
		{
			mp_new /= mid_component;
			path_components.push_back(mid_component);
		}
		else if (diff >= 10 + 1)
		{
			mp_new /= short_component;
			path_components.push_back(short_component);
		}
		else
		{
			if (first_time_char && diff > 1)
			{
				mp_new /= char_component;
				path_components.push_back(char_component);
				first_time_char = false;
			}
			else
			{
				mp_new += char_component;
				std::string temp_comp = path_components.back();
				path_components.pop_back();
				temp_comp += char_component;
				path_components.push_back(temp_comp);
				first_time_char = false;
			}
		}
	}

	return path_components;
}

TEST(ManagedPathTest, AmendPathInsertsPrefix)
{
	ManagedPath mp(std::string("start"));

	// 
	// <= ManagedPath::max_unamended_path_len_
	//
	std::vector<std::string> test_path_str = { "data", "Is", "not-here", "ok.txt" };

	// Build the path in a platform-independent way
	for (auto s : test_path_str)
		mp /= s;
	std::filesystem::path test_path(mp.RawString());

	EXPECT_TRUE(test_path.string().size() <= ManagedPath::max_unamended_path_len_);
	EXPECT_FALSE(HasWindowsPrefix(mp.AmendPath(test_path).string()));

	//
	// = ManagedPath::max_unamended_path_len_ + 1
	//
	mp = ManagedPath();
	test_path_str = CreatePathWithLength(mp, ManagedPath::max_unamended_path_len_ + 1);
	for (auto s : test_path_str)
		mp /= s;
	printf("string len: %zu\n", mp.RawString().size());
	test_path = std::filesystem::path(mp.RawString());
	EXPECT_EQ(test_path.string().size(), ManagedPath::max_unamended_path_len_ + 1);
#ifdef __WIN64
	EXPECT_TRUE(HasWindowsPrefix(mp.AmendPath(test_path).string()));
#elif defined __linux__
	EXPECT_FALSE(HasWindowsPrefix(mp.AmendPath(test_path).string()));
#endif

	// 
	// > ManagedPath::max_unamended_path_len_ + 1
	//
	mp = ManagedPath();
	test_path_str = CreatePathWithLength(mp, ManagedPath::max_unamended_path_len_ + 10);
	for (auto s : test_path_str)
		mp /= s;
	test_path = std::filesystem::path(mp.RawString());
	EXPECT_TRUE(test_path.string().size() > ManagedPath::max_unamended_path_len_ + 1);
#ifdef __WIN64
	EXPECT_TRUE(HasWindowsPrefix(mp.AmendPath(test_path).string()));
#elif defined __linux__
	EXPECT_FALSE(HasWindowsPrefix(mp.AmendPath(test_path).string()));
#endif
}

TEST(ManagedPathTest, string)
{
	ManagedPath mp(std::string("start"));

	// 
	// <= ManagedPath::max_unamended_path_len_
	//
	std::vector<std::string> test_path_str = { "data", "Is", "not-here", "ok.txt" };

	// Build the path in a platform-independent way
	for (auto s : test_path_str)
		mp /= s;
	EXPECT_TRUE(mp.RawString().size() <= ManagedPath::max_unamended_path_len_);
	EXPECT_FALSE(HasWindowsPrefix(mp.string()));

	// 
	// = ManagedPath::max_unamended_path_len_ + 1
	//
	mp = ManagedPath();
	test_path_str = CreatePathWithLength(mp, ManagedPath::max_unamended_path_len_ + 1);
	for (auto s : test_path_str)
		mp /= s;
	EXPECT_EQ(mp.RawString().size(), ManagedPath::max_unamended_path_len_ + 1);
#ifdef __WIN64
	EXPECT_TRUE(HasWindowsPrefix(mp.string()));
#elif defined __linux__
	EXPECT_FALSE(HasWindowsPrefix(mp.string()));
#endif

	// 
	// > ManagedPath::max_unamended_path_len_ + 1
	//
	mp = ManagedPath();
	test_path_str = CreatePathWithLength(mp, ManagedPath::max_unamended_path_len_ + 10);
	for (auto s : test_path_str)
		mp /= s;
	EXPECT_TRUE(mp.RawString().size() > ManagedPath::max_unamended_path_len_ + 1);
#ifdef __WIN64
	EXPECT_TRUE(HasWindowsPrefix(mp.string()));
#elif defined __linux__
	EXPECT_FALSE(HasWindowsPrefix(mp.string()));
#endif
}

void TestCreateDirectoryFailsWithoutCorrection(bool use_absolute_path)
{
	uint16_t max_length = ManagedPath::max_unamended_path_len_;
	uint16_t initial_length = max_length - 10;

	// Create relative-path object.
	ManagedPath mp;

	// Get the absolute path.
	mp = mp.absolute();
	printf("\nabsolute: %s\n", mp.RawString().c_str());

	// Start with a path a little less than max, then test all
	// lengths up to max for success and max+1 for failure.
	std::vector<std::string> test_path_components = CreatePathWithLength(mp, 
		initial_length);
	
	// Append test_path_str components as file path components.
	ManagedPath fullpath = mp; 
	for (auto s : test_path_components)
		fullpath /= s;
	printf("\nabsolute extended to max + 1: %s\n", fullpath.RawString().c_str());
	EXPECT_EQ(fullpath.RawString().size(), initial_length);

	// Create all but the last directory.
	ManagedPath currdir = mp;
	printf("\nstr vec size is %zu\n", test_path_components.size());
	for (size_t i = 0; i < test_path_components.size() - 1; i++)
	{
		printf("\nappending to path: %s\n", test_path_components[i].c_str());
		currdir /= test_path_components[i];
		
		EXPECT_TRUE(currdir.create_directory());
		printf("after append length = %zu\n", currdir.RawString().size());
	}

	// Extend by one char at a time and verify create succeeds
	ManagedPath base_dir(currdir);
	std::string postfix = "";
	for (size_t i = base_dir.RawString().size(); i <= max_length; i++)
	{
		postfix += 'a';
		currdir = base_dir / postfix;
		
		EXPECT_TRUE(currdir.create_directory());
		printf("after append length = %zu\n", currdir.RawString().size());
	}

	// Attempt to create the last directory using standard fs::create_directory. 
	// This ought to fail because the path is too long.
	postfix += 'a';
	currdir = base_dir / postfix;

	/*
	Catch the error. Also, one way to fix the create_directory
	impl is to catch the error, amend path, and try again. 
	This is an ugly approach but will likely solve the problem, 
	though it may need to be implemented in several functions
	that operate on the real file system.
	*/
	fs::path raw_path(currdir.RawString());
	std::error_code ec;
	bool result = fs::create_directory(raw_path, ec); 
	
	// If the error code is zero, then an error was not thrown.
	// If zero, 
#ifdef __WIN64
	EXPECT_TRUE((ec.value() != 0) || (!result));
#elif defined __linux__
	EXPECT_TRUE((ec.value() == 0) && result);
#endif
	

	// Attempt to create the directory using ManagedPath.
	// Ought to succeed.
	EXPECT_TRUE(currdir.create_directory());

	// Delete all directories created during this test.
	fs::path remove_path(test_path_components[0]);
	EXPECT_TRUE(fs::remove_all(remove_path));

}

TEST(ManagedPathTest, CreateDirectoryFailsWithoutCorrectionAbsPath)
{
	TestCreateDirectoryFailsWithoutCorrection(true);
}

TEST(ManagedPathTest, CreateDirectoryFailsWithoutCorrectionRelPath)
{
	TestCreateDirectoryFailsWithoutCorrection(false);
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

	// Create the part of the path that's less than ManagedPath::max_unamended_path_len_ + 1
	// chars using std::filesystem.
	mp /= ManagedPath(root_dir_name);
	mp = mp / "this_is_part_of_a_very_long_path_section00" / "this_is_part_of_a_very_long_path_section01";
	mp = mp / "this_is_part_of_a_very_long_path_section02" / "this_is_part_of_a_very_long_path_section03";
	EXPECT_TRUE(mp.RawString().size() <= ManagedPath::max_unamended_path_len_);

	fs::path small_path(mp.RawString());
	EXPECT_TRUE(fs::create_directories(small_path));

	// Extend the path beyond ManagedPath::max_unamended_path_len_ and 
	// create the final path.
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

TEST(ManagedPathTest, CreatePathObjectNoExtReplacement)
{
	std::string base_path = "base-path";
	ManagedPath mp_base(base_path);

	// multi-component file path
	std::string file_path1 = "file_base_path";
	std::string file_path2 = "actual_FileName.ext";
	ManagedPath mp_file(file_path1);
	mp_file /= file_path2;

	ManagedPath mp_result = mp_base.CreatePathObject(mp_file);

	std::filesystem::path p_base(base_path);
	std::filesystem::path p_file(file_path1);
	p_file /= file_path2;
	std::filesystem::path p_result = p_base / p_file.filename();

	EXPECT_EQ(mp_result.RawString(), p_result.string());

	// single-component file path
	mp_file = ManagedPath(file_path2);
	mp_result = mp_base.CreatePathObject(mp_file);
	p_result = p_base / std::filesystem::path(file_path2);
	EXPECT_EQ(mp_result.RawString(), p_result.string());
}

TEST(ManagedPathTest, CreatePathObjectWithExtReplacement)
{
	std::string base_path = "base-path";
	ManagedPath mp_base(base_path);

	// multi-component file path
	std::string file_path1 = "file_base_path";
	std::string file_path2 = "actual_FileName.ext";
	std::string ext_repl = ".int";
	ManagedPath mp_file(file_path1);
	mp_file /= file_path2;

	ManagedPath mp_result = mp_base.CreatePathObject(mp_file, ext_repl);

	std::filesystem::path p_base(base_path);
	std::filesystem::path p_file(file_path1);
	p_file /= file_path2;
	std::filesystem::path p_result = p_base / (p_file.stem() += ext_repl);

	EXPECT_EQ(mp_result.RawString(), p_result.string());

	// single-component file path
	mp_file = ManagedPath(file_path2);
	mp_result = mp_base.CreatePathObject(mp_file, ext_repl);
	p_result = p_base / (std::filesystem::path(file_path2).stem() += ext_repl);
	EXPECT_EQ(mp_result.RawString(), p_result.string());
}

TEST(ManagedPathTest, CreatePathObjectDirsNoExtReplacement)
{
	std::string base_path = "base-path";
	ManagedPath mp_base(base_path);

	// multi-component file path
	std::string file_path1 = "input_base_path";
	std::string file_path2 = "final_input_path";
	ManagedPath mp_file(file_path1);
	mp_file /= file_path2;

	ManagedPath mp_result = mp_base.CreatePathObject(mp_file);

	std::filesystem::path p_base(base_path);
	std::filesystem::path p_file(file_path1);
	p_file /= file_path2;
	std::filesystem::path p_result = p_base / p_file.filename();
	EXPECT_EQ(mp_result.RawString(), p_result.string());
}

TEST(ManagedPathTest, CreatePathObjectDirsWithExtReplacement)
{
	std::string base_path = "base-path";
	ManagedPath mp_base(base_path);

	// multi-component file path
	std::string file_path1 = "input_base_path";
	std::string file_path2 = "final_input_path";
	std::string ext_repl = "_modified";
	ManagedPath mp_file(file_path1);
	mp_file /= file_path2;

	ManagedPath mp_result = mp_base.CreatePathObject(mp_file, ext_repl);

	std::filesystem::path p_base(base_path);
	std::filesystem::path p_file(file_path1);
	p_file /= file_path2;
	std::filesystem::path p_result = p_base / (p_file.filename() += ext_repl);
	EXPECT_EQ(mp_result.RawString(), p_result.string());
}

TEST(ManagedPathTest, GetFileSize)
{
	// Create relative path and simple file.
	std::string test_fname = "my_file.data";
	ManagedPath mp;
	mp /= test_fname;

	uint64_t n_bytes = 5;
	char c[] = "blah blah blah";
	std::ofstream(mp.RawString()).write(c, n_bytes);

	bool success = false;
	uint64_t result = 100;
	mp.GetFileSize(success, result);
	EXPECT_TRUE(success);
	EXPECT_EQ(result, n_bytes);

	EXPECT_TRUE(std::filesystem::remove(std::filesystem::path(test_fname)));
}

TEST(ManagedPathTest, GetFileSizeNonExistentFile)
{
	// Create relative path and simple file.
	std::string test_fname = "my_file.data";
	ManagedPath mp;
	mp /= test_fname;

	bool success = true;
	uint64_t result = 100;
	mp.GetFileSize(success, result);
	EXPECT_FALSE(success);
	EXPECT_EQ(result, 0);
}

TEST(ManagedPathTest, GetFileSizeNonFile)
{
	// Create relative path and simple file.
	std::string test_fname = "my_dir";
	ManagedPath mp;
	mp /= test_fname;

	EXPECT_TRUE(mp.create_directory());

	bool success = true;
	uint64_t result = 100;
	mp.GetFileSize(success, result);
	EXPECT_FALSE(success);
	EXPECT_EQ(result, 0);

	EXPECT_TRUE(std::filesystem::remove(std::filesystem::path(test_fname)));
}

TEST(ManagedPathTest, ListDirectoryEntriesDirNotExist)
{
	// Create object representative of dir that does not exist.
	std::string test_fname = "my_dir";
	ManagedPath mp;
	mp /= test_fname;

	bool success = true;
	std::vector<ManagedPath> file_list({ mp });

	mp.ListDirectoryEntries(success, file_list);

	EXPECT_FALSE(success);
	EXPECT_EQ(file_list.size(), 0);
}

TEST(ManagedPathTest, ListDirectoryEntriesCorrectList)
{
	std::string test_fname = "my_dir";
	ManagedPath mp;
	mp /= test_fname;

	// Create dir
	EXPECT_TRUE(mp.create_directory());

	std::string file_name1 = "the-file.txt";
	std::string file_name2 = "1other.data";

	ManagedPath file_path1 = mp / file_name1;
	ManagedPath file_path2 = mp / file_name2;

	// Create files
	std::ofstream(file_path1.RawString()).put('a');
	std::ofstream(file_path2.RawString()).put('a');

	bool success = false;
	std::vector<ManagedPath> file_list({ mp });

	mp.ListDirectoryEntries(success, file_list);

	EXPECT_TRUE(success);
	EXPECT_EQ(file_list.size(), 2);
	
	// Check correct files name in alphanumeric order
	EXPECT_EQ(file_list[0].filename().RawString(), file_name2);
	EXPECT_EQ(file_list[1].filename().RawString(), file_name1);

	std::filesystem::path rm_path(mp.RawString());
	std::filesystem::remove_all(rm_path);
}

TEST(ManagedPathTest, ExcludePathsWithSubString)
{
	std::string test_fname = "my_dir";
	ManagedPath mp;
	mp /= test_fname;

	// Create dir
	EXPECT_TRUE(mp.create_directory());

	std::string file_name1 = "the-file.txt";
	std::string file_name2 = "1other.data";
	std::string file_name3 = "b-files.out";

	ManagedPath file_path1 = mp / file_name1;
	ManagedPath file_path2 = mp / file_name2;
	ManagedPath file_path3 = mp / file_name3;

	// Create files
	std::ofstream(file_path1.RawString()).put('a');
	std::ofstream(file_path2.RawString()).put('a');
	std::ofstream(file_path3.RawString()).put('a');

	bool success = false;
	std::vector<ManagedPath> dir_entries;
	std::vector<std::string> substrings({ "the-" });

	mp.ListDirectoryEntries(success, dir_entries);
	EXPECT_TRUE(success);
	EXPECT_EQ(dir_entries.size(), 3);

	// 
	// Single exclusion
	//
	std::vector<ManagedPath> result = ManagedPath::ExcludePathsWithSubString(
		dir_entries, substrings);

	// Removed the "the-file.txt" entry.
	EXPECT_EQ(result.size(), 2);

	// Check correct files name in alphanumeric order
	EXPECT_EQ(result[0].filename().RawString(), file_name2);
	EXPECT_EQ(result[1].filename().RawString(), file_name3);

	//
	// Multiple exclusion
	//
	substrings = std::vector<std::string>({ "the-", "files" });
	result = ManagedPath::ExcludePathsWithSubString(
		dir_entries, substrings);

	// Removed the "the-file.txt" and "b-files.out" entries.
	EXPECT_EQ(result.size(), 1);

	// Check correct files name in alphanumeric order
	EXPECT_EQ(result[0].filename().RawString(), file_name2);

	//
	// Multiple substrings for single exclusion
	//
	substrings = std::vector<std::string>({ "the-", "txt", "files" });
	result = ManagedPath::ExcludePathsWithSubString(
		dir_entries, substrings);

	// Removed the "the-file.txt".
	EXPECT_EQ(result.size(), 1);

	// Check correct files name in alphanumeric order
	EXPECT_EQ(result[0].filename().RawString(), file_name2);

	std::filesystem::path rm_path(mp.RawString());
	std::filesystem::remove_all(rm_path);
}

TEST(ManagedPathTest, ExcludePathsWithSubStringNotInFilenameComponent)
{
	std::string test_fname = "my_dir";
	ManagedPath mp;
	mp /= test_fname;

	std::string file_name1 = "the-file.txt";
	std::string file_name2 = "1other.data";

	ManagedPath file_path1 = mp / file_name1;
	ManagedPath file_path2 = mp / file_name2;

	std::vector<ManagedPath> dir_entries({ file_path1, file_path2 });
	std::vector<std::string> substrings({ "dir" });

	std::vector<ManagedPath> result = ManagedPath::ExcludePathsWithSubString(
		dir_entries, substrings);

	// Ensure that both entries are NOT removed due to having "dir" in 
	// "my_dir" portion of the path.
	EXPECT_EQ(result.size(), 2);
}

TEST(ManagedPathTest, SelectPathsWithSubString)
{
	std::string test_fname = "my_dir";
	ManagedPath mp;
	mp /= test_fname;

	// Create dir
	EXPECT_TRUE(mp.create_directory());

	std::string file_name1 = "the-file.txt";
	std::string file_name2 = "1fileother.data";
	std::string file_name3 = "b-files.out";

	ManagedPath file_path1 = mp / file_name1;
	ManagedPath file_path2 = mp / file_name2;
	ManagedPath file_path3 = mp / file_name3;

	// Create files
	std::ofstream(file_path1.RawString()).put('a');
	std::ofstream(file_path2.RawString()).put('a');
	std::ofstream(file_path3.RawString()).put('a');

	bool success = false;
	std::vector<ManagedPath> dir_entries;

	mp.ListDirectoryEntries(success, dir_entries);
	EXPECT_TRUE(success);
	EXPECT_EQ(dir_entries.size(), 3);

	// 
	// Select all
	//
	std::vector<std::string> substrings({ "file" });

	std::vector<ManagedPath> result = ManagedPath::SelectPathsWithSubString(
		dir_entries, substrings);

	// Removed the "the-file.txt" entry.
	EXPECT_EQ(result.size(), 3);

	// Check correct files name in alphanumeric order
	EXPECT_EQ(result[0].filename().RawString(), file_name2);
	EXPECT_EQ(result[1].filename().RawString(), file_name3);
	EXPECT_EQ(result[2].filename().RawString(), file_name1);

	//
	// Select single entry
	//
	substrings = std::vector<std::string>({ "other" });
	result = ManagedPath::SelectPathsWithSubString(
		dir_entries, substrings);

	// Removed the "the-file.txt" and "b-files.out" entries.
	EXPECT_EQ(result.size(), 1);

	// Check correct files name in alphanumeric order
	EXPECT_EQ(result[0].filename().RawString(), file_name2);

	//
	// Select multiple
	//
	substrings = std::vector<std::string>({ "-file" });
	result = ManagedPath::SelectPathsWithSubString(
		dir_entries, substrings);

	// Removed the "the-file.txt".
	EXPECT_EQ(result.size(), 2);

	// Check correct files name in alphanumeric order
	EXPECT_EQ(result[0].filename().RawString(), file_name3);
	EXPECT_EQ(result[1].filename().RawString(), file_name1);

	std::filesystem::path rm_path(mp.RawString());
	std::filesystem::remove_all(rm_path);
}

TEST(ManagedPathTest, SelectPathsWithSubStringNotInFilenameComponent)
{
	std::string test_fname = "my_dir";
	ManagedPath mp;
	mp /= test_fname;

	std::string file_name1 = "the-file.txt";
	std::string file_name2 = "1fileother.data";

	ManagedPath file_path1 = mp / file_name1;
	ManagedPath file_path2 = mp / file_name2;

	std::vector<ManagedPath> dir_entries({ file_path1, file_path2 });

	std::vector<std::string> substrings({ "dir" });

	std::vector<ManagedPath> result = ManagedPath::SelectPathsWithSubString(
		dir_entries, substrings);

	// Ensure that zero entries are selected. The "dir" in "my_dir" portion
	// of the path should not be used, only the filename() componenet.
	EXPECT_EQ(result.size(), 0);
}

TEST(ManagedPathTest, SelectFiles)
{
	std::string test_fname = "my_dir";
	ManagedPath mp;
	mp /= test_fname;

	// Create dir
	EXPECT_TRUE(mp.create_directory());

	std::string file_name1 = "the-file.txt";
	std::string file_name2 = "1other.data";
	std::string file_name3 = "b-files.out";

	ManagedPath file_path1 = mp / file_name1;
	ManagedPath file_path2 = mp / file_name2;
	ManagedPath file_path3 = mp / file_name3;

	// Create files and dirs
	std::ofstream(file_path1.RawString()).put('a');
	std::ofstream(file_path2.RawString()).put('a');
	EXPECT_TRUE(file_path3.create_directory());

	bool success = false;
	std::vector<ManagedPath> dir_entries;

	mp.ListDirectoryEntries(success, dir_entries);
	EXPECT_TRUE(success);
	EXPECT_EQ(dir_entries.size(), 3);

	// Remove "b-files.out" dir
	std::vector<ManagedPath> files_only = ManagedPath::SelectFiles(dir_entries);
	EXPECT_EQ(files_only.size(), 2);

	EXPECT_EQ(files_only[0].filename().RawString(), file_name2);
	EXPECT_EQ(files_only[1].filename().RawString(), file_name1);

	std::filesystem::path rm_path(mp.RawString());
	std::filesystem::remove_all(rm_path);
}

TEST(ManagedPathTest, SelectDirectories)
{
	std::string test_fname = "my_dir";
	ManagedPath mp;
	mp /= test_fname;

	// Create dir
	EXPECT_TRUE(mp.create_directory());

	std::string file_name1 = "the-file.txt";
	std::string file_name2 = "1other.data";
	std::string file_name3 = "b-files.out";

	ManagedPath file_path1 = mp / file_name1;
	ManagedPath file_path2 = mp / file_name2;
	ManagedPath file_path3 = mp / file_name3;

	// Create files and dirs
	std::ofstream(file_path1.RawString()).put('a');
	EXPECT_TRUE(file_path2.create_directory());
	EXPECT_TRUE(file_path3.create_directory());

	bool success = false;
	std::vector<ManagedPath> dir_entries;

	mp.ListDirectoryEntries(success, dir_entries);
	EXPECT_TRUE(success);
	EXPECT_EQ(dir_entries.size(), 3);

	// Remove "the-file.txt" file
	std::vector<ManagedPath> files_only = ManagedPath::SelectDirectories(dir_entries);
	EXPECT_EQ(files_only.size(), 2);

	EXPECT_EQ(files_only[0].filename().RawString(), file_name2);
	EXPECT_EQ(files_only[1].filename().RawString(), file_name3);

	std::filesystem::path rm_path(mp.RawString());
	std::filesystem::remove_all(rm_path);
}



