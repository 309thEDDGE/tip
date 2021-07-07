#ifndef MANAGED_PATH_H
#define MANAGED_PATH_H

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <initializer_list>
#include <string>
#include <thread>
#include <vector>
#include "iterable_tools.h"

namespace fs = std::filesystem;

class ManagedPath : public fs::path
{
   private:
    static const inline fs::path windows_prefix_ = "\\\\?\\";
    static const int max_create_dir_attempts_ = 3;

   protected:
   public:
    // Maximum path in windows is 260 chars including a terminating
    // null character, making the maximum string length 259.
    // See https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation
    // Caveats:
    //		- The drive letter is included in the count
    //		- The system will only create directories up to 247 characters in order to allow
    //			the appending of a 12-char DOS-style 8.3 file name:
    //				259 - 12 = 247
    static const int max_unamended_path_len_ = 247;

    //////////////////////////////////////////
    // User functions
    //////////////////////////////////////////

    // Initialize with string path
    ManagedPath(const std::string& input_path);

    // Initialize with cwd
    ManagedPath() : fs::path(fs::current_path()) {}

    // Initialize with fs::path
    ManagedPath(fs::path input_path) : fs::path(input_path) {}

    // Initialize with cwd and each of the arguments appended
    // via "/"
    ManagedPath(std::initializer_list<std::string> path_components);

    // Assignment
    ManagedPath& operator=(const ManagedPath& c);

    // Concatenate, Append
    ManagedPath& operator/=(const ManagedPath& rhs);
    ManagedPath operator/(const ManagedPath& rhs);
    ManagedPath& operator+=(const ManagedPath& rhs);

    //
    // Hide, not override these fs::path functions.
    //

    /*
	Determine if a file or directory represented by a 
	ManagedPath object exists. Same functionality as the 
	std::filesystem::exists function.

	Returns: true if exists, false otherwise.
	*/
    bool exists() const;

    /*
	Get a string that is formatted with special characters
	necessary for long paths in windows, if necessary. 

	Returns: std::string
	*/
    std::string string() const;

    /*
	Get the parent path. Same functionality as the 
	std::filesystem::path::parent_path function.

	Returns: A ManagedPath object containing the parent
	path.
	*/
    ManagedPath parent_path() const;

    /*
	Get the absolute. Same functionality as the
	std::filesystem::absolute function.

	Returns: A ManagedPath object containing the absolute
	path.
	*/
    ManagedPath absolute() const;

    /*
	Check whether the current object is a regular file.
	Same functionality as the std::filesystem::path::is_regular_file
	function.

	Returns: true if is regular file, false otherwise
	*/
    bool is_regular_file() const;

    /*
	Check whether the current object is a directory.
	Same functionality as the std::filesystem::path::is_directory
	function.

	Returns: true if is directory, false otherwise
	*/
    bool is_directory() const;

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

    /*
	Get the extension component of the current object.
	Same functionality as std::filesystem::path::extension

	Return: A ManagedPath object containing the extension
	of the current object.
	*/
    ManagedPath extension() const;

    //
    // Mimic other std::filesystem functions.
    //

    /*
	Create a directory that is represented by the current
	object, similar to std::filesystem::create_directory.

	Returns: true if the directory does not exist and the 
	directory is successfully created or if the directory
	already exists, otherwise false.
	*/
    bool create_directory() const;

    /*
	Remove a file or directory, similar to std::filesystem::remove.

	Returns: true if file/dir is removed, false otherwise.
	*/
    bool remove() const;

    /*
	Remove all files and directories at the location represented
	by the current object. If the current object is a file, then
	only the file shall be removed. Only files and directories
	which extend the current path will be removed.

	Return:
		True if the file (object represents a file path) is removed
		or all files and sub-directories (object represents a directory).
		False if one or more files and/or directories fail to be removed 
		or the file/directory does not exist.
	*/
    bool RemoveTree() const;

    /*
	Get an un-amended raw string. Useful for print statements.
	Does not include the windows magic sequence, even if applicable.

	Returns: the objects representation of the path as a std::string.
	*/
    std::string RawString() const;

    /*
	Create a file/dir path using the current object path and the final
	component of an input ManagedPath object. If the input object is 
	is a file path, then the full file name will be used unless 
	the extension_replacement argument is not the default value, in which
	case the file name will be modifed by the extension replacement.

	If the input object is a directory path, then the final component 
	of the input path will be used and the extension_replacement argument 
	will be concatenated without a file path separator to the final component.

	Input: 

		output_fname			- ManagedPath object from which the file name
		will be used to construct the output file name

		extension_replacement	- String to replace the extension,
		ex: if extension_replacement = "_abc.123", "a.txt" --> "a_abc.123"
		ex: extension_replacement = "_append-this", "/a/b/base" --> "a/b/base_append-this"

	Return: ManagedPath object representative of the new path
	*/
    ManagedPath CreatePathObject(const ManagedPath& output_fname,
                                 const std::string& extension_replacement = "") const;

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
    void GetFileSize(bool& success, uint64_t& result) const;

    /*
	Fill the output_list vector with ManagedPath objects corresponding
	to the contents of the directory of the current object. If the current
	object is not a directory or does not exist, the output_list will be empty
	and the success bool will be false.

	Sort the objects in the output_list alphanumerically by the string paths
	represented by each object.

	Input:

		success				- Output bool is true if the current object represents a 
		directory and contents can be successfully iterated over using 
		std::filesystem::directory_iterator. Otherwise false.

		output_list			- Vector of ManagedPath objects representative of the
		directory contents.

		exclude_matching	- Vector of strings used to exclude files from 
		the output_list which contain sub-strings matching any of the files
		in this list.
	*/
    void ListDirectoryEntries(bool& success, std::vector<ManagedPath>& output_list) const;

    /*
	Filter a vector of ManagedPath objects by removing all objects from the vector
	that represent paths which contain a substring (in the filename() component) 
	equal to one of the substrings in the substrings vector.

	Input: 

		input_paths	- Vector of ManagedPath objects

		substrings	- Vector of strings used to exclude files from 
		the output list which contain sub-strings matching any of the files
		in this list.

	Return:

		Vector of ManagedPath objects which represent paths that do not contain
		substrings matching any of those in the substrings vector.
	*/
    static std::vector<ManagedPath> ExcludePathsWithSubString(const std::vector<ManagedPath>&
                                                                  input_paths,
                                                              const std::vector<std::string>& substrings);

    /*
	Filter a vector of ManagedPath objects by selecting/keeping all objects from the vector
	that represent paths which contain (in the filename() component) a substring equal 
	to one of the substrings in the substrings vector.

	Input:

		input_paths	- Vector of ManagedPath objects

		substrings	- Vector of strings used to select files from
		the input list which contain sub-strings matching any of the files
		in this list.

	Return:

		Vector of ManagedPath objects which represent paths that contain
		substrings matching at least one of those in the substrings vector.
	*/
    static std::vector<ManagedPath> SelectPathsWithSubString(const std::vector<ManagedPath>&
                                                                 input_paths,
                                                             const std::vector<std::string>& substrings);

    /*
	Filter a vector of ManagedPath objects by removing all objects which correspond
	to directories or do not exist.

	Input:

		input_paths	- Vector of ManagedPath objects

	Return:

		Vector of ManagedPath objects which represent files.
	*/
    static std::vector<ManagedPath> SelectFiles(const std::vector<ManagedPath>& input_paths);

    /*
	Filter a vector of ManagedPath objects by removing all objects which correspond
	to files or do not exist.

	Input:

		input_paths	- Vector of ManagedPath objects

	Return:

		Vector of ManagedPath objects which represent directories.
	*/
    static std::vector<ManagedPath> SelectDirectories(const std::vector<ManagedPath>& input_paths);

    /*
	Create a directory object specific to a given
	output directory, base file name and string to be appended to the
	base file name. Optionally create the directory in the file system.

	Log the name of the directory created.

	Args:
		base_dir			--> ManagedPath object giving the directory into
								which the output directory will be created
		base_file_name		--> ManagedPath object with the base file name on which
								to build the output directory name. May be a complete
								path to a file, in which case the parent path will be
								stripped and only the file name used, or a file name
								only.
		append_str			--> String to be appended to the base_file_name
		output_dir			--> ManagedPath object with the final output directory object
		create_dir			--> True if the directory ought to be created, false otherwise

	Return:
		True if create_dir is true and the output_dir output path is
		created in the filesystem. Also true if create_dir is false and there
		were no errors creating output_dir. False otherwise.
	*/
    static bool CreateDirectoryFromComponents(const ManagedPath& base_dir,
                                              const ManagedPath& base_file_name, const std::string& append_str,
                                              ManagedPath& output_dir, bool create_dir);

    //////////////////////////////////////////
    // Functions below not intended to be utilized directly.
    //////////////////////////////////////////

    /*
	Get a std:filesystem::path object amended to include
	the windows magic characters if necessary.

	Returns: fs::path object
	*/
    fs::path AmendPath(fs::path input_path) const;
};

#endif