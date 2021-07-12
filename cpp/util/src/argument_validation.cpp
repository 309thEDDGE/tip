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
        printf(
            "ValidateDefaultInputFilePath: Constructed file path (%s) does"
            " not exist or is not a file\n",
            temp_path.RawString().c_str());
        return false;
    }

    full_path = temp_path;
    return true;
}

bool ArgumentValidation::ValidateDefaultOutputDirectory(const ManagedPath& default_output_dir,
                                                        const std::string& user_output_dir, ManagedPath& final_output_dir,
                                                        bool create_dir)
{
    final_output_dir = ManagedPath(std::string(""));

    // Use default directory
    if (user_output_dir == "")
    {
        if (!default_output_dir.is_directory())
        {
            if (create_dir)
            {
                if (!default_output_dir.create_directory())
                {
                    printf("ValidateDefaultOutputDirectory: Failed to create default directory %s\n",
                           default_output_dir.RawString().c_str());
                    return false;
                }

                // Check if directory was created
                if (default_output_dir.is_directory())
                {
                    final_output_dir = default_output_dir.absolute();
                    return true;
                }
            }

            printf("ValidateDefaultOutputDirectory: Default directory (%s) does not exist\n",
                   default_output_dir.RawString().c_str());
            return false;
        }

        final_output_dir = default_output_dir.absolute();
    }
    else
    {
        if (!parse_text_.IsUTF8(user_output_dir))
        {
            printf("ValidateDefaultOutputDirectory: User directory (%s) is not valid UTF-8\n",
                   user_output_dir.c_str());
            return false;
        }

        ManagedPath temp_user_dir(user_output_dir);
        temp_user_dir = temp_user_dir.absolute();
        if (!temp_user_dir.is_directory())
        {
            if (create_dir)
            {
                if (!temp_user_dir.create_directory())
                {
                    printf("ValidateDefaultOutputDirectory: Failed to create user directory %s\n",
                           temp_user_dir.RawString().c_str());
                    return false;
                }

                // Check if directory was created
                if (temp_user_dir.is_directory())
                {
                    final_output_dir = temp_user_dir.absolute();
                    return true;
                }
            }

            printf("ValidateDefaultOutputDirectory: User directory (%s) does not exist\n",
                   temp_user_dir.RawString().c_str());
            return false;
        }
        final_output_dir = temp_user_dir;
    }
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
        printf(
            "ValidateInputFilePath: Input path (%s) does"
            " not exist or is not a file\n",
            temp_path.RawString().c_str());
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

bool ArgumentValidation::ValidateDirectoryPath(std::string dir,
                                               ManagedPath& mp_dir)
{
    mp_dir = ManagedPath(std::string(""));
    if (!parse_text_.IsUTF8(dir))
    {
        printf(
            "ValidateDirectoryPath: Directory (%s) does not conform"
            " with UTF-8 encoding\n",
            dir.c_str());
        return false;
    }

    ManagedPath temp_dir(dir);
    temp_dir = temp_dir.absolute();
    if (!temp_dir.is_directory())
    {
        printf(
            "ValidateDirectoryPath: Directory dir (%s) does not exist"
            " or is not a directory\n",
            temp_dir.RawString().c_str());
        return false;
    }

    mp_dir = temp_dir;
    return true;
}