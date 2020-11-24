#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "managed_path.h"
#include <fstream>

TEST(ManagedPathTest, AmendPathInsertsPrefix)
{
	ManagedPath mp;
	std::string prefix = "\\\\?\\";

	// < 260
	std::string test_path_str = R"(C:\User\Is\not-here\ok.txt)";
	std::filesystem::path test_path(test_path_str);

	EXPECT_TRUE(test_path_str.size() < 260);
	EXPECT_EQ(mp.AmendPath(test_path).string(), test_path_str);

	// = 260
	test_path_str = std::string("C:\\Users\\this\\is\\the\\path\\to-the-file\\and_it_must_be"
		"_equivalent_to_260_chars_in_size\\this-is-a-long-part01\\"
		"this-is-a-long-part02\\this-is-a-long-part03\\this-is-a-long-part04\\"
		"this-is-a-long-part05\\this-is-a-long-part06\\this-is-a-long-part07\\"
		"this-is-a-long-aa.txt");
	//printf("string len: %zu\n", test_path_str.size());
	test_path = std::filesystem::path(test_path_str);
	EXPECT_TRUE(test_path_str.size() == 260);

	EXPECT_EQ(mp.AmendPath(test_path).string(), test_path_str);

	// > 260
	test_path_str = std::string("C:\\Users\\this\\is\\the\\path\\to-the-file\\and_it_must_be"
		"_equivalent_to_260_chars_in_size\\this-is-a-long-part01\\"
		"this-is-a-long-part02\\this-is-a-long-part03\\this-is-a-long-part04\\"
		"this-is-a-long-part05\\this-is-a-long-part06\\this-is-a-long-part07\\"
		"this-is-a-long-damn-file.txt");
	test_path = std::filesystem::path(test_path_str);
	EXPECT_TRUE(test_path_str.size() > 260);
#ifdef __WIN64
	EXPECT_EQ(mp.AmendPath(test_path).string(), prefix + test_path_str);
#elif defined __linux__
	EXPECT_EQ(mp.AmendPath(test_path).string(), test_path_str);
#endif

	test_path_str = std::string("/this_is_another/very_long_path/down_the_rabbit_hole01/"
		"down_the_rabbit_hole02/down_the_rabbit_hole03/"
		"down_the_rabbit_hole04/down_the_rabbit_hole05/"
		"down_the_rabbit_hole06/down_the_rabbit_hole07/"
		"down_the_rabbit_hole08/down_the_rabbit_hole09/"
		"down_the_rabbit_hole10/down_the_rabbit_hole11/"
		"to_the_final_dir");
	test_path = std::filesystem::path(test_path_str);
	EXPECT_TRUE(test_path_str.size() > 260);
#ifdef __WIN64
	EXPECT_EQ(mp.AmendPath(test_path).string(), prefix + test_path_str);
#elif defined __linux__
	EXPECT_EQ(mp.AmendPath(test_path).string(), test_path_str);
#endif
}

TEST(ManagedPathTest, string)
{
	
	std::string prefix = "\\\\?\\";

	// < 260
	std::string test_path_str = R"(C:\User\Is\not-here\ok.txt)";
	ManagedPath mp(test_path_str);
	std::filesystem::path test_path(test_path_str);

	EXPECT_TRUE(test_path_str.size() < 260);
	EXPECT_EQ(mp.string(), test_path_str);

	// = 260
	test_path_str = std::string("C:\\Users\\this\\is\\the\\path\\to-the-file\\and_it_must_be"
		"_equivalent_to_260_chars_in_size\\this-is-a-long-part01\\"
		"this-is-a-long-part02\\this-is-a-long-part03\\this-is-a-long-part04\\"
		"this-is-a-long-part05\\this-is-a-long-part06\\this-is-a-long-part07\\"
		"this-is-a-long-aa.txt");
	test_path = std::filesystem::path(test_path_str);
	mp = ManagedPath(test_path_str);
	EXPECT_TRUE(test_path_str.size() == 260);
	EXPECT_EQ(mp.string(), test_path_str);

	// > 260
	test_path_str = std::string("C:\\Users\\this\\is\\the\\path\\to-the-file\\and_it_must_be"
		"_equivalent_to_260_chars_in_size\\this-is-a-long-part01\\"
		"this-is-a-long-part02\\this-is-a-long-part03\\this-is-a-long-part04\\"
		"this-is-a-long-part05\\this-is-a-long-part06\\this-is-a-long-part07\\"
		"this-is-a-long-damn-file.txt");
	test_path = std::filesystem::path(test_path_str);
	mp = ManagedPath(test_path_str);
	EXPECT_TRUE(test_path_str.size() > 260);
#ifdef __WIN64
	EXPECT_EQ(mp.string(), prefix + test_path_str);
#elif defined __linux__
	EXPECT_EQ(mp.string(), test_path_str);
#endif

	test_path_str = std::string("/this_is_another/very_long_path/down_the_rabbit_hole01/"
		"down_the_rabbit_hole02/down_the_rabbit_hole03/"
		"down_the_rabbit_hole04/down_the_rabbit_hole05/"
		"down_the_rabbit_hole06/down_the_rabbit_hole07/"
		"down_the_rabbit_hole08/down_the_rabbit_hole09/"
		"down_the_rabbit_hole10/down_the_rabbit_hole11/"
		"to_the_final_dir");
	test_path = std::filesystem::path(test_path_str);
	mp = ManagedPath(test_path_str);
	EXPECT_TRUE(test_path_str.size() > 260);
#ifdef __WIN64
	EXPECT_EQ(mp.string(), prefix + test_path_str);
#elif defined __linux__
	EXPECT_EQ(mp.string(), test_path_str);
#endif
}

TEST(ManagedPathTest, CreateDirAlreadyExists)
{
	fs::path temp_path("test_dir");
	EXPECT_TRUE(fs::create_directory(temp_path));

	ManagedPath mp;
	mp /= ManagedPath(temp_path);

	EXPECT_TRUE(mp.CreateDir());

	// Remove test dir.
	EXPECT_TRUE(fs::remove(temp_path));
}

TEST(ManagedPathTest, CreateDirParentMustExist)
{
	ManagedPath mp;
	mp = mp / "test_dir" / "second-dir";

	ASSERT_FALSE(mp.CreateDir());
}

TEST(ManagedPathTest, CreateDirLongPath)
{
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
	// end section of failing code

	EXPECT_TRUE(mp.CreateDir());

	// Remove directories.
	fs::path root_path(root_dir_name);
	EXPECT_TRUE(fs::remove_all(root_path));
}


#ifdef __WIN64
TEST(ManagedPathTest, AppendOperator)
{
	// /=
	std::string s1 = "C:\\Users\\my";
	std::string s2 = "in\\here-you-go";
	ManagedPath mp1(s1);
	ManagedPath mp2(s2);
	mp1 /= mp2;
	EXPECT_EQ(mp1.RawString(), s1 + "\\" + s2);
}

TEST(ManagedPathTest, ConcatenateOperator)
{
	// /
	std::string s1 = "C:\\Users\\my";
	std::string s2 = "in\\here-you-go";
	ManagedPath mp1(s1);
	ManagedPath mp2(s2);
	ManagedPath mp = mp1 / mp2;
	EXPECT_EQ(mp.RawString(), s1 + "\\" + s2);
}

TEST(ManagedPathTest, AppendNoSeparator)
{
	// /
	std::string s1 = "C:\\Users\\my";
	std::string s2 = "in\\here-you-go";
	ManagedPath mp1(s1);
	ManagedPath mp2(s2);
	mp1 += mp2;
	EXPECT_EQ(mp1.RawString(), s1 + s2);
}



#elif defined __linux__
TEST(ManagedPathTest, AppendOperator)
{
	// /=
	std::string s1 = "/Users/my";
	std::string s2 = "in/here-you-go";
	ManagedPath mp1(s1);
	ManagedPath mp2(s2);
	mp1 /= mp2;
	EXPECT_EQ(mp1.RawString(), s1 + "/" + s2);
}

TEST(ManagedPathTest, ConcatenateOperator)
{
	// /
	std::string s1 = "/Users/my";
	std::string s2 = "in/here-you-go";
	ManagedPath mp1(s1);
	ManagedPath mp2(s2);
	ManagedPath mp = mp1 / mp2;
	EXPECT_EQ(mp.RawString(), s1 + "/" + s2);
}

TEST(ManagedPathTest, AppendNoSeparator)
{
	// /
	std::string s1 = "/Users/my";
	std::string s2 = "in/here-you-go";
	ManagedPath mp1(s1);
	ManagedPath mp2(s2);
	mp1 += mp2;
	EXPECT_EQ(mp1.RawString(), s1 + s2);
}
#endif