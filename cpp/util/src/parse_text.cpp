#include "parse_text.h"

// Split the string on delimiter
// Returns vector of string split by delimiter
std::vector<std::string> ParseText::Split(std::string input_string, const char& delim)
{
	std::vector<std::string> return_vec;

	// Check for empty string
	// Returns: empty vector if string
	if (input_string == "")
		return return_vec;

	// If new line character at the end exists, remove it
	if ((input_string.back() == '\n') || (input_string.back() == '\r'))
	  input_string.pop_back();

	/*size_t pos = input_string.find('\n');
	if ((pos + 1) == input_string.length())
		input_string = input_string.substr(0, pos);*/
	size_t pos = 0;

	// Count the number of delimiters
	size_t n = std::count(input_string.begin(), input_string.end(), delim);
	
	// Pushes the string into the vector and returns if the delimiter doesn't exist
	if (n == 0) 
	{
		return_vec.push_back(input_string);
		return return_vec;
	}

	std::map<int, std::string> quoted_sections;
	std::map<int, std::string> unquoted_sections;
	bool have_quoted_sections = ExtractQuotedSections(input_string, quoted_sections,
		unquoted_sections);
	
	/*if (have_quoted_sections)
	{
		IterableTools it;
		std::vector<std::string> cols = { "Index", "SplitElem" };
		it.PrintMapWithHeader_KeyToValue(quoted_sections, cols, "Quoted Sections");
		it.PrintMapWithHeader_KeyToValue(unquoted_sections, cols, "Unquoted Sections");
		}*/

	if (have_quoted_sections)
	{
		// Add each quoted section directly to the output vector and 
		// split unquoted sections by the delimiter.
		int n_sections = quoted_sections.size() + unquoted_sections.size();
		IterableTools it;
		for (int i = 0; i < n_sections; i++)
		{
			if (it.IsKeyInMap(quoted_sections, i))
			{
				return_vec.push_back(quoted_sections[i]);
			}
			else
			{
				std::string curr_string = unquoted_sections[i];

				// Remove any delimiters that may exist at the beginning
				// or end of the string.
				if (curr_string[curr_string.length() - 1] == delim)
					curr_string.erase(curr_string.length() - 1, 1);
				if (curr_string[0] == delim)
					curr_string.erase(0, 1);

				// If the string consists of only a delimiter then the
				// previous erasure will have made an empty string.
				// If the length is zero then do not add it to the vector.
				if (curr_string.size() == 0)
					continue;

				pos = curr_string.find(delim);
				while (pos != std::string::npos)
				{
					return_vec.push_back(curr_string.substr(0, pos));
					// Erase the portion of string consumed
					curr_string.erase(0, pos + 1);
					pos = curr_string.find(delim);
				}

				// Gather the last portion
				return_vec.push_back(curr_string);
			}
		}
	}
	else
	{
		pos = input_string.find(delim);
		while (pos != std::string::npos)
		{
			return_vec.push_back(input_string.substr(0, pos));
			// Erase the portion of string consumed
			input_string.erase(0, pos + 1);
			pos = input_string.find(delim);
		}

		// Gather the last portion
		return_vec.push_back(input_string);
	}

	return return_vec;
}

bool ParseText::ExtractQuotedSections(const std::string& input_string,
	std::map<int, std::string>& quoted_sections,
	std::map<int, std::string>& unquoted_sections)
{
	// Determine if quoted text exists within the input string.
	size_t n_quotes = std::count(input_string.begin(), input_string.end(), '\"');

	// There must be an even count of quote instances to be considered.
	if (n_quotes > 0 && n_quotes % 2 == 0)
	{
		int quoted_section_count = n_quotes / 2;
		std::vector<size_t> quote_pos;

		// Locate all of the quote positions.
		size_t pos = input_string.find('\"');
		while (pos != std::string::npos)
		{
			quote_pos.push_back(pos);
			pos = input_string.find('\"', pos + 1);
		}

		// Extract quoted and non-quoted sections.
		// Process each accordingly.
		pos = 0;
		size_t quote_pos_index = 0;
		int total_index = 0;
		size_t section_len = 0;
		while (pos != std::string::npos)
		{
			if (pos >= quote_pos[quote_pos_index] && pos <= quote_pos[quote_pos_index+1])
			{
				// Get the quoted section and save it in the map.
				section_len = quote_pos[quote_pos_index + 1] - (quote_pos[quote_pos_index] + 1);
				quoted_sections[total_index] = input_string.substr(
					quote_pos[quote_pos_index]+1, section_len);

				// Set pos to the end of the current quoted section, plus one.
				pos = quote_pos[quote_pos_index + 1] + 1;

				// If the end of the current quote section is also the
				// end of the input string then exit out of the while loop.
				if (quote_pos[quote_pos_index + 1] == input_string.size() - 1)
				{
					break;
				}

				// Increment the total index.
				total_index++;
			}
			else
			{
				// If this unquoted section is the end of the input string,
				// i.e., no more quotes, then get the end of the last quote
				// position plus one to the end of the input string.
				if ((quote_pos_index + 2 > quote_pos.size() - 1) && (total_index > 0))
				{
					// Extract the unquoted section.
					section_len = input_string.size() - pos;
					unquoted_sections[total_index] = input_string.substr(pos, section_len);
					break;
				}
				else
				{
					// Increment the quote_pos_index to the next quoted section.
					if(total_index > 0)
						quote_pos_index += 2;

					// Extract the unquoted section.
					section_len = quote_pos[quote_pos_index] - pos;
					unquoted_sections[total_index] = input_string.substr(pos, section_len);

					// Set pos to the beginning of the next quote section.
					pos = quote_pos[quote_pos_index];
				}
				// Increment the total index.
				total_index++;
			}
		}
		return true;
	}
	return false;
}

// Converts a string to int and assigns the value to output (0 if unsuccessful)
// Returns: false if unsuccessful, true if successful
bool ParseText::ConvertInt(const std::string& convert_string, int& output)
{
	//// Return invalid if empty string
	//if (convert_string.length() == 0)
	//{
	//	output = 0;
	//	return false;
	//}

	//// Return invalid if first character is '-' and length is one
	//if (convert_string[0] == '-' && convert_string.length() == 1)
	//{
	//	output = 0;
	//	return false;
	//}

	if (!TextIsInteger(convert_string))
	{
		output = 0;
		return false;
	}
	
	output = stoi(convert_string);
	return true;
}

// Checks for a character in a string
// Returns: false if the character doesn't exist in the string, true if it does
bool ParseText::CheckForExisitingChar(const std::string& input_string, char character)
{
	if (input_string.find(character) != std::string::npos)
		return true;
	else
		return false;
}


// Removes a trailing character if the character specified is 
// the actual trailing character. Otherwise returns the original string.
std::string ParseText::RemoveTrailingChar(const std::string& input_string, char character)
{
	// Is the character present? 
	if (CheckForExisitingChar(input_string, character))
	{
		// Is the character the last in the string?
		size_t pos = 0;
		if ((pos = input_string.find_last_of(character)) == input_string.length() - 1)
		{
			std::string ret_str = input_string.substr(0, pos);
			return ret_str;
		}
	}
	return input_string;
}

// Converts a string to a double and assigns the value to output (0.0 if unsuccessful)
// Returns: false if unsuccessful, true if successful
bool ParseText::ConvertDouble(const std::string& convert_string, double& output)
{
	//// Return invalid if empty string
	//if (convert_string.length() == 0)
	//{
	//	output = 0;
	//	return false;
	//}

	//// Return invalid if first character is '-' and length is one
	//if (convert_string[0] == '-' && convert_string.length() == 1)
	//{
	//	output = 0;
	//	return false;
	//}

	if (!TextIsFloat(convert_string))
	{
		output = 0.0;
		return false;
	}

	output = stod(convert_string);
	return true;
}

bool ParseText::TextIsInteger(const std::string& input_string)
{
	if (input_string.length() == 0)
		return false;
	for (int i = 0; i < input_string.length(); i++)
	{

		// If the first character is '-', skip it
		if (i == 0 && input_string[0] == '-')
		{
			if (input_string.length() == 1)
				return false;
			else
				continue;
		}

		// If any character is not a digit return false
		if (!isdigit(input_string[i]))
		{
			return false;
		}

	}
	return true;
}

bool ParseText::TextIsFloat(const std::string& input_string)
{
	if (input_string.length() == 0)
		return false;

	int e_count = 0;
	for (int i = 0; i < input_string.length(); i++)
	{
		// If the first character is '-', skip it.
		// If the current character is a period, skip it.
		if (i == 0 && input_string[0] == '-')
		{
			if (input_string.length() == 1)
				return false;
			else
				continue;
		}
		else if (input_string[i] == '.')
			continue;
		else if (std::tolower(input_string[i]) == 'e')
		{
			e_count++;
			if (e_count > 1)
				return false;
			if (input_string[i + 1] == '+' || input_string[i + 1] == '-')
			{
				i++;
				continue;
			}
		}

		// If any character is not a digit return false
		if (!isdigit(input_string[i]))
		{
			return false;
		}
	}
	return true;
}
