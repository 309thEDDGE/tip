#ifndef PATHMANAGER_H
#define PATHMANAGER_H

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class PathManager
{
private:
	fs::path path_;
	const fs::path windows_prefix_;

protected:

public:

	// Initialize with current execution path
	PathManager() : path_(fs::current_path()), windows_prefix_("\\\\?\\") { }

	// Initialize with input path string.
	PathManager(std::string input_path) : path_(input_path), windows_prefix_("\\\\?\\") { }

	// Initialize with input std::filesystem::path
	PathManager(fs::path input_path) : path_(input_path), windows_prefix_("\\\\?\\") { }

	// Return the path_ private member, un-modified. PATH IS NOT AMENDED!
	//fs::path RawPath();

	PathManager& operator = (const PathManager& c);

	std::string AsString();
	fs::path AsPath();
	PathManager& Join(const std::string& input_path);
	PathManager& Parent();
	bool IsDirectory();
	bool IsFile();
	bool CreateDirectory();

	// 
	// Functions below not intended to be utilized directly.
	//
	fs::path AmendPath(const fs::path& input_path);
};

#endif
