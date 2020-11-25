#ifndef MANAGED_PATH_H
#define MANAGED_PATH_H

#include <thread>
#include <chrono>
#include <filesystem>
#include <string>
#include <cstdio>
#include <cstdint>

namespace fs = std::filesystem;

class ManagedPath : public fs::path
{
private:
	static const inline fs::path windows_prefix_ = "\\\\?\\";
	static const int max_create_dir_attempts_ = 5;

protected:

public:
	
	//////////////////////////////////////////
	// User functions
	//////////////////////////////////////////

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

	//
	// Hide, not override these fs::path functions.
	//

	/*
	Get a string that is formatted with special characters
	necessary for long paths in windows, if necessary. 

	Returns: std::string
	*/
	std::string string();

	/*
	Get the parent path. Same functionality as the 
	std::filesystem::path::parent_path function.

	Returns: A ManagedPath object containing the parent
	path.
	*/
	ManagedPath parent_path();

	/*
	Check whether the current object is a regular file.
	Same functionality as the std::filesystem::path::is_regular_file
	function.

	Returns: true if is regular file, false otherwise
	*/
	bool is_regular_file();

	/*
	Check whether the current object is a directory.
	Same functionality as the std::filesystem::path::is_directory
	function.

	Returns: true if is directory, false otherwise
	*/
	bool is_directory();

	/*
	Get the filename component of the current object.
	Same functionality as the std::filesystem::path::filename
	function.

	Returns: A ManagedPath object containing the filename portion
	of the current object.
	*/
	ManagedPath filename() const;

	/*
	Get the stem component of the current object.
	Same functionality as the std::filesystem::path::stem
	function.

	Returns: A ManagedPath object containing the stem portion
	of the current object.
	*/
	ManagedPath stem() const;

	//
	// Mimic other std::filesystem functions.
	//

	/*
	Create a directory that is represented by the current
	object, similar to std::filesystem::create_directory.

	Returns: true if the directory does not exist and the 
	directory is successfully created, false otherwise.
	*/
	bool create_directory();

	/*
	Remove a file or directory, similar to std::filesystem::remove.

	Returns: true if file/dir is removed, false otherwise.
	*/
	bool remove();

	/*
	Get an un-amended raw string. Useful for print statements.
	Does not include the windows magic sequence, even if applicable.

	Returns: the objects representation of the path as a std::string.
	*/
	std::string RawString();

	/*
	Create a file/dir path using the current object path and the final
	component of an input ManagedPath object. If the input object is 
	is a file path, then the full file name will be used unless 
	the extension_replacement argument is not the default value, in which
	case the file name will be modifed by the extension replacement.

	If the input object is a directory path, then the final component 
	of the input path will be used and the extension_replacement argument 
	will be concatenated without a file path separator to the final component.

	This function creates a directory path if the 

	Input: 

		output_fname			- ManagedPath object from which the file name
		will be used to construct the output file name

		extension_replacement	- String to replace the extension,
		ex: if extension_replacement = "_abc.123", "a.txt" --> "a_abc.123"
		ex: extension_replacement = "_append-this", "/a/b/base" --> "a/b/base_append-this"

	Return: ManagedPath object representative of the new file path
	*/
	ManagedPath CreatePathObject(const ManagedPath& output_fname,
		const std::string& extension_replacement = "");


	/*
	Get the size of the file represented by the current object.

	Input: 

		success	- Output bool indicator if the file size was
		correctly obtained. Set to false if the current object
		is a path to a file that does not exist or the file does
		exist but cannot be opened.

		result	- Output result indicating file size in bytes.
		Set to 0 if success is false.
	*/
	void GetFileSize(bool& success, uint64_t& result);


	//////////////////////////////////////////
	// Functions below not intended to be utilized directly.
	//////////////////////////////////////////

	/*
	Get a std:filesystem::path object amended to include
	the windows magic characters if necessary.

	Returns: fs::path object
	*/
	fs::path AmendPath(fs::path input_path);
};

#endif