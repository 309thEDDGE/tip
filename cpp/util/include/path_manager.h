#ifndef PATHMANAGER_H
#define PATHMANAGER_H

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class PathManager
{
private:
	fs::path path_;
	fs::path windows_prefix_;

protected:

public:

	// Initialize with current execution path
	PathManager() : path_(fs::current_path()), windows_prefix_("\\\\?\\") { }

	// Initialize with input path.
	PathManager(std::string input_path) : path_(input_path), windows_prefix_("\\\\?\\") { }
	std::string AsString();

	PathManager& Join(const std::string& input_path);
	PathManager& Parent();
	bool IsDirectory();
	bool IsFile();

	// Functions below not intended to be utilized directly.
	fs::path AmendPath(const fs::path& input_path);
};

#endif
