
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

};

#endif