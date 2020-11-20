#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "path_manager.h"
#include <fstream>

TEST(PathManagerTest, AmendPathInsertsPrefix)
{
	PathManager pm;
	std::string prefix = "\\\\?\\";

	// < 260
	std::string test_path_str = R"(C:\User\Is\not-here\ok.txt)";
	std::filesystem::path test_path(test_path_str);

	EXPECT_TRUE(test_path_str.size() < 260);
	EXPECT_EQ(pm.AmendPath(test_path).string(), test_path_str);

	// = 260
	test_path_str = std::string("C:\\Users\\this\\is\\the\\path\\to-the-file\\and_it_must_be"
								"_equivalent_to_260_chars_in_size\\this-is-a-long-part01\\"
								"this-is-a-long-part02\\this-is-a-long-part03\\this-is-a-long-part04\\"
								"this-is-a-long-part05\\this-is-a-long-part06\\this-is-a-long-part07\\"
								"this-is-a-long-aa.txt");
	//printf("string len: %zu\n", test_path_str.size());
	test_path = std::filesystem::path(test_path_str);
	EXPECT_TRUE(test_path_str.size() == 260);

	EXPECT_EQ(pm.AmendPath(test_path).string(), test_path_str);

	// > 260
	test_path_str = std::string("C:\\Users\\this\\is\\the\\path\\to-the-file\\and_it_must_be"
		"_equivalent_to_260_chars_in_size\\this-is-a-long-part01\\"
		"this-is-a-long-part02\\this-is-a-long-part03\\this-is-a-long-part04\\"
		"this-is-a-long-part05\\this-is-a-long-part06\\this-is-a-long-part07\\"
		"this-is-a-long-damn-file.txt");
	test_path = std::filesystem::path(test_path_str);
	EXPECT_TRUE(test_path_str.size() > 260);
#ifdef __WIN64
	EXPECT_EQ(pm.AmendPath(test_path).string(), prefix+test_path_str);
#elif defined __linux__
	EXPECT_EQ(pm.AmendPath(test_path).string(), test_path_str);
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
	EXPECT_EQ(pm.AmendPath(test_path).string(), prefix + test_path_str);
#elif defined __linux__
	EXPECT_EQ(pm.AmendPath(test_path).string(), test_path_str);
#endif
}

TEST(PathManagerTest, JoinStringDir)
{
	std::string init_path = "C:\\Documents\\input\\data";
	PathManager pm(init_path);

	std::string join_path = "things\\file1.ch10";
	EXPECT_EQ(pm.Join(join_path).AsString(), init_path + "\\" + join_path);

	join_path = "\\" + join_path;
	EXPECT_EQ(pm.AsString(), init_path + join_path);
}

TEST(PathManagerTest, Parent)
{
	std::string init_path = "C:\\Documents\\input\\data";
	PathManager pm(init_path);

	std::string expected_path = "C:\\Documents\\input";
	EXPECT_EQ(pm.Parent().AsString(), expected_path);

	expected_path = init_path;
	init_path = init_path + "\\file.out";
	PathManager pm1(init_path);
	EXPECT_EQ(pm1.Parent().AsString(), expected_path);

}

TEST(PathManagerTest, IsDirectory)
{

	// Confirm non-existent directory fails.
	std::string false_dir = "C:\\This_does\\not\\exist";
	PathManager pm(false_dir);
	EXPECT_FALSE(pm.IsDirectory());

	// Partial path of directory to create
	std::string temp_dir_name = "test_dir";
	
	// Create path at cwd
	pm = PathManager();

	// Join the temp dir
	pm.Join(temp_dir_name);

	// Create the directory using std::filesystem native functions
	fs::path temp_dir_path(pm.AsString());
	EXPECT_TRUE(fs::create_directory(temp_dir_path));

	// Confirm directory exists
	EXPECT_TRUE(pm.IsDirectory());

	// Delete directory
	fs::remove(temp_dir_path);
}

TEST(PathManagerTest, IsFile)
{

	// Confirm non-existent file fails.
	std::string false_file = "C:\\This_does\\not\\exist\\my_file.txt";
	PathManager pm(false_file);
	EXPECT_FALSE(pm.IsFile());

	// File name to create
	std::string temp_file_name = "test_file.out";

	// Create path at cwd
	pm = PathManager();

	// Join the temp file
	pm.Join(temp_file_name);

	// Create the file using std::filesystem native functions
	fs::path temp_file_path(pm.AsString());
	std::ofstream(temp_file_path).put('a');

	// Confirm file exists
	EXPECT_TRUE(pm.IsFile());

	// Delete file
	fs::remove(temp_file_path);
}