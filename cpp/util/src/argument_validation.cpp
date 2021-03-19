#include "argument_validation.h"

bool ArgumentValidation::ValidateDefaultInputFilePath(
	const ManagedPath& default_base_path, 
	const std::string& user_base_path, std::string file_name, ManagedPath& full_path)
{
	full_path = ManagedPath(std::string(""));
	ManagedPath temp_path;

	// Check that file_name is utf-8
	if (!parse_text_.IsUTF8(file_name))
	{
		printf("ValidateDefaultInputFilePath: file name (%s) is not valid UTF-8\n",
			file_name.c_str());
		return false;
	}
	
	if (user_base_path == "")
		temp_path = default_base_path / file_name;
	else
	{
		// Confirm that the user base path is valid utf-8
		if (!parse_text_.IsUTF8(user_base_path))
		{
			printf("ValidateDefaultInputFilePath: User base path (%s) is not valid UTF-8\n",
				user_base_path.c_str());
			return false;
		}
		temp_path = ManagedPath(user_base_path) / file_name;
	}
	temp_path = temp_path.absolute();

	// Confirm that the constructed path exists.
	if (!temp_path.is_regular_file())
	{
		printf("ValidateDefaultInputFilePath: Constructed file path (%s) does"
			" not exist or is not a file\n", temp_path.RawString().c_str());
		return false;
	}

	full_path = temp_path;
	return true;
}

bool ArgumentValidation::ValidateInputFilePath(std::string input_path, ManagedPath& mp_input_path)
{
	mp_input_path = ManagedPath(std::string(""));

	// Check that input_path is utf-8
	if (!parse_text_.IsUTF8(input_path))
	{
		printf("ValidateInputFilePath: Input path (%s) is not valid UTF-8\n",
			input_path.c_str());
		return false;
	}

	ManagedPath temp_path(input_path);
	if (!temp_path.is_regular_file())
	{
		printf("ValidateInputFilePath: Input path (%s) does"
			" not exist or is not a file\n", temp_path.RawString().c_str());
		return false;
	}

	mp_input_path = temp_path;
	return true;
}

bool ArgumentValidation::ValidateDocument(const ManagedPath& doc_path, std::string& doc_string)
{
	doc_string = "";

	// Attempt to read file.
	if (!(file_reader_.ReadFile(doc_path.string()) == 0))
	{
		printf("ValidateDocument: Document path (%s) does not exist or is not a file\n",
			doc_path.RawString().c_str());
		return false;
	}

	std::string temp_doc_string = file_reader_.GetDocumentAsString();
	if (!parse_text_.IsUTF8(temp_doc_string))
	{
		printf("ValidateDocument: Document (%s) does not conform with UTF-8 encoding\n",
			doc_path.RawString().c_str());
		return false;
	}

	doc_string = temp_doc_string;
	return true;
}

bool ArgumentValidation::ValidateOutputDirPath(std::string output_dir, 
	ManagedPath& mp_output_dir)
{
	mp_output_dir = ManagedPath(std::string(""));
	if (!parse_text_.IsUTF8(output_dir))
	{
		printf("ValidateOutputDirPath: Output dir (%s) does not conform"
			" with UTF-8 encoding\n", output_dir.c_str());
		return false;
	}

	ManagedPath temp_output_dir(output_dir);
	temp_output_dir = temp_output_dir.absolute();
	if (!temp_output_dir.is_directory())
	{
		printf("ValidateOutputDirPath: Output dir (%s) does not exist"
			" or is not a directory\n", temp_output_dir.RawString().c_str());
		return false;
	}

	mp_output_dir = temp_output_dir;
	return true;
}