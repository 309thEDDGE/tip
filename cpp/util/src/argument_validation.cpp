#include "argument_validation.h"

bool ArgumentValidation::ValidateDefaultInputFilePath(
	const ManagedPath& default_base_path, 
	const std::string& user_base_path, std::string file_name, ManagedPath& full_path)
{
	ParseText pt;
	full_path = ManagedPath(std::string(""));
	ManagedPath temp_path;

	// Check that file_name is utf-8
	if (!pt.IsUTF8(file_name))
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
		if (!pt.IsUTF8(user_base_path))
		{
			printf("ValidateDefaultInputFilePath: User base path (%s) is not valid UTF-8\n",
				user_base_path.c_str());
			return false;
		}
		temp_path = ManagedPath(user_base_path) / file_name;
	}

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
	return true;
}