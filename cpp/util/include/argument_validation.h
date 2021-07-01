
#ifndef ARGUMENT_VALIDATION_H_
#define ARGUMENT_VALIDATION_H_

#include "managed_path.h"
#include "parse_text.h"
#include "file_reader.h"

class ArgumentValidation
{
private:
	ParseText parse_text_;
	FileReader file_reader_;

public:
	ArgumentValidation() {}

	/*
	TODO: If the print statements in any of these functions are important enough to log,
	then pass a fifth argument, a string containing the name of a logger. Make
	this a default arg with empty string. If non-empty use spdlog to get the logger
	and log the message with the appropriate log level. There are probably other
	ways to handle this needed functionality. Can't find much on the web. Note
	that the main motivation is the different cases in which this function,
	or others in this header file, may be used, at both parse and translate time,
	in which different loggers will be in use.
	*/


	/*
	Validate a potential input file path by checking that the path string conforms
	to utf-8 and that it exists.

	Args:
		input_path		--> User input path passed as an argument
		mp_input_path	--> ManagedPath object to be set as the file
							path if the path is validated, otherwise set as an
							emptry string

	Return:
		True if the path is validated, i.e., conforms to utf-8 and the file exists,
		otherwise false.
	*/
	bool ValidateInputFilePath(std::string input_path, ManagedPath& mp_input_path);

	/*
	Create default file path consisting of a default base path joined
	with the file name if the user-input base path is an empty string, otherwise
	create a path from the user-input base path and the file name.

	Do not proceed if any strings do not comply with utf8 pattern or final file
	path does not exist.

	Args:
		default_base_path	--> default directory to be used if user-supplied directory
								string is empty
		user_base_path		--> User-input base path will only be used if it is not
								an empty string. If so, it will be used to construct
								the final return path.
		file_name			--> name of file joined with either the default or user-input
								path
		full_path			--> Full path of base / file name, validated to be utf8
								conformant and confirmed to exist if true is returned
								by function, otherwise an empty path

	Return:
		True if the path is validated utf8 and confirmed to exist, false otherwise

	*/
	bool ValidateDefaultInputFilePath(const ManagedPath& default_base_path,
		const std::string& user_base_path, std::string file_name,
		ManagedPath& full_path);

	/*
	Create an output directory object if the user output directory is an 
	empty string, or use the default directory object. Confirm that the
	directory exists and return false if create_dir is set to false. Otherwise,
	attempt to create the directory and return true if successful, and false
	otherwise. Set the final_output_dir to the directory which is either
	created or confirmed to exist if the return is true, otherwise set it
	equal to an empty string.

	Args:
		default_output_dir	--> Directory to check or create if the
								user_output_dir is empty
		user_output_dir		--> String representing the user's choice of 
								output directory, which may be an empty string
		final_output_dir	--> Object in which the directory which is created 
								or confirmed to exist is stored
		create_dir			--> Set to true to create the directory if it doesn't
								exist

	Return:
		True if the directory is confirmed to exist or created successfully if 
		create_dir is set to true, false otherwise.
	*/
	bool ValidateDefaultOutputDirectory(const ManagedPath& default_output_dir,
		const std::string& user_output_dir, ManagedPath& final_output_dir,
		bool create_dir);

	/*
	Read a file into a string object and test for UTF-8 conformity.

	Args:
		doc_path	--> ManagedPath object with the full path to the file
						which ought to be read. 
		doc_string	--> Output string containing the entire text of the document,
						newlines included.

	Return:
		True if the document can be read and tested to conform with UTF-8, 
		otherwise false.
	*/
	bool ValidateDocument(const ManagedPath& doc_path, std::string& doc_string);

	/*
	Validate a potential directory by checking that the path string conforms
	to utf-8 and that it exists.

	Args:
		output_dir		--> User-input path passed as a string argument
		mp_output_dir	--> ManagedPath object to be set as the dir
							path if the path is validated, otherwise set as an
							emptry string

	Return:
		True if the path is validated, i.e., conforms to utf-8 and the dir exists,
		otherwise false.
	*/
	bool ValidateDirectoryPath(std::string dir, ManagedPath& mp_dir);

	/*
	Check that a string has one of the n extensions passed as arguments.
	Ignore case of extension on input string and extensions passed as 
	arguments.

	Args:
		input_path	--> An input path which may or may not have the expected
						extension
		exts			--> N many string arguments listing the possible extensions,
						ex: (input_path, "txt", "data", "csv")

	Return:
		True if the extension found on the input_path is equal to one of 
		the extensions supplied as arguments, otherwise false.
	*/
	template <typename ...T>
	bool CheckExtension(const std::string& input_path, T... exts);

};

template <typename ...T>
bool ArgumentValidation::CheckExtension(const std::string& input_path, T... exts)
{
	ManagedPath temp_path(input_path);
	if (temp_path.extension().RawString() == "")
	{
		printf("CheckExtension: Input file %s does not have an extension\n",
			input_path.c_str());
		return false;
	}

	// Get lower case extension from input string with the '.' removed.
	std::string path_ext = parse_text_.ToLower(
		temp_path.extension().RawString().substr(1));

	// Iterate over argument pack and compare.
	std::string lower_opt_ext;
	for (auto opt_ext : { exts... })
	{
		lower_opt_ext = parse_text_.ToLower(opt_ext);
		if (path_ext == lower_opt_ext)
			return true;
	}

	std::string ext_list = "";
	for (auto opt_ext : { exts... })
	{
		ext_list += opt_ext;
		ext_list += " ";
	}
	printf("CheckExtension: Input file %s does not have one of the "
		   "(case-insensitive) extensions: %s\n", input_path.c_str(), ext_list.c_str());

	return false;
}


#endif