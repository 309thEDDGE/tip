#ifndef MANAGED_PATH_H
#define MANAGED_PATH_H

#include <filesystem>
#include <string>
#include <cstdio>

namespace fs = std::filesystem;

class ManagedPath : public fs::path
{
private:
	const fs::path windows_prefix_;

protected:

public:
	
	//
	// User functions
	//

	// Initialize with string path
	ManagedPath(std::string input_path) : fs::path(input_path), windows_prefix_("\\\\?\\") { }

	// Initialize with cwd
	ManagedPath() : fs::path(fs::current_path()), windows_prefix_("\\\\?\\") { }

	// Initialize with fs::path
	ManagedPath(fs::path input_path) : fs::path(input_path), windows_prefix_("\\\\?\\") { }

	// Assignment
	ManagedPath& operator = (const ManagedPath& c);

	// Concatenate, Append
	ManagedPath& operator /= (const ManagedPath& rhs);
	ManagedPath operator / (const ManagedPath& rhs);
	ManagedPath& operator += (const ManagedPath& rhs);

	// Hide, not override these fs::path functions.
	std::string string();
	ManagedPath parent_path();
	bool is_regular_file();
	bool is_directory();

	// Get un-amended raw string. Use for print statements.
	std::string RawString();

	

	// 
	// Functions below not intended to be utilized directly.
	//
	fs::path AmendPath(fs::path input_path);
};

#endif