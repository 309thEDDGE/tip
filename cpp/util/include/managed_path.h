#ifndef MANAGED_PATH_H
#define MANAGED_PATH_H

#include <thread>
#include <chrono>
#include <filesystem>
#include <string>
#include <cstdio>


static const std::string WINDOWS_PREFIX("\\\\?\\");

namespace fs = std::filesystem;

class ManagedPath : public fs::path
{
private:
	static const inline fs::path windows_prefix_ = WINDOWS_PREFIX;
	static const int max_create_dir_attempts_ = 5;

protected:

public:
	
	//
	// User functions
	//

	// Initialize with string path
	ManagedPath(std::string input_path) : fs::path(input_path){ }

	// Initialize with cwd
	ManagedPath() : fs::path(fs::current_path()) { }

	// Initialize with fs::path
	ManagedPath(fs::path input_path) : fs::path(input_path) { }

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

	// Use instead of std::filesystem::create_directory().
	bool CreateDir();

	// Get un-amended raw string. Use for print statements.
	std::string RawString();
	

	// 
	// Functions below not intended to be utilized directly.
	//
	fs::path AmendPath(fs::path input_path);
};

#endif