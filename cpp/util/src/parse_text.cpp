#include "parse_text.h"

// Split the string on delimiter
// Returns vector of string split by delimiter
std::vector<std::string> ParseText::Split(std::string input_string, const char& delim,
    bool keep_quotes) const
{
    std::vector<std::string> return_vec;

    // Check for empty string
    // Returns: empty vector if string
    if (input_string == "")
        return return_vec;

    // If new line character at the end exists, remove it
    if ((input_string.back() == '\n') || (input_string.back() == '\r'))
        input_string.pop_back();

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

    if (have_quoted_sections)
    {
        // Add each quoted section directly to the output vector and
        // split unquoted sections by the delimiter.
        size_t n_sections = quoted_sections.size() + unquoted_sections.size();
        IterableTools it;
        bool new_element = false;
        bool preceding_is_quoted = false;
        std::string quoted = "";
        for (int i = 0; i < n_sections; i++)
        {
            if (it.IsKeyInMap(quoted_sections, i))
            {
                if(keep_quotes)
                    quoted = "\"" + quoted_sections.at(i) + "\"";
                else
                    quoted = quoted_sections.at(i);

                if(new_element)
                    return_vec.push_back(quoted);
                else
                {
                    if(return_vec.size() == 0)
                        return_vec.push_back(quoted);
                    else
                    {
                        size_t elem_pos = return_vec.size() - 1;
                        return_vec[elem_pos] = return_vec.at(elem_pos) + quoted;
                    }
                }
                preceding_is_quoted = true;
                new_element = false;
            }
            else
            {
                std::string curr_string = unquoted_sections.at(i);

                // Remove any delimiters that may exist at the beginning
                // or end of the string.
                if (curr_string[curr_string.length() - 1] == delim)
                {
                    curr_string.erase(curr_string.length() - 1, 1);
                    new_element = true;
                }
                if (curr_string.size() == 0)
                    continue;
                if (curr_string[0] == delim)
                {
                    curr_string.erase(0, 1);
                    preceding_is_quoted = false;
                }

                // If the string consists of only a delimiter then the
                // previous erasure will have made an empty string.
                // If the length is zero then do not add it to the vector.
                if (curr_string.size() == 0)
                    continue;

                pos = curr_string.find(delim);
                while (pos != std::string::npos)
                {
                    if(preceding_is_quoted)
                    {
                        size_t elem_pos = return_vec.size() - 1;
                        return_vec[elem_pos] = return_vec.at(elem_pos) + curr_string.substr(0, pos);
                        preceding_is_quoted = false;
                    }
                    else
                        return_vec.push_back(curr_string.substr(0, pos));

                    // Erase the portion of string consumed
                    curr_string.erase(0, pos + 1);
                    pos = curr_string.find(delim);
                }

                // Gather the last portion
                if(preceding_is_quoted)
                {
                        size_t elem_pos = return_vec.size() - 1;
                        return_vec[elem_pos] = return_vec.at(elem_pos) + curr_string;
                }
                else
                    return_vec.push_back(curr_string);

                preceding_is_quoted = false;
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


std::vector<std::string> ParseText::Split(std::string input_string, const std::string& delim,
    bool keep_delim)
{
    std::vector<std::string> split;
    if(input_string.size() == 0)
        return split;

    size_t pos = 0; 
    size_t start_pos = 0;
    std::string keep_part = delim.substr(0, delim.size()-1);
    std::string found = "";
    while((pos = input_string.find(delim, start_pos)) != std::string::npos)
    {
        found = input_string.substr(start_pos, pos-start_pos);
        if (keep_delim)
        {
            if((found.size() + keep_part.size()) > 0)
                split.push_back(found + keep_part);
        }
        else
        {
            if(found.size() > 0)
                split.push_back(found);
        }
        start_pos = pos + delim.size();
    }
    if(start_pos < input_string.size())
        split.push_back(input_string.substr(start_pos, input_string.size() - start_pos));

    return split;
}

std::vector<std::string> ParseText::Split(std::string input_string)
{
    std::vector<std::string> split;
    std::regex rgx("(^|[^ ]+)[ ]+");
    std::smatch match;
    std::string input = input_string;
    std::string matched_substr = "";
    while(std::regex_search(input, match, rgx))
    {
        matched_substr = match[1].str();
        if(matched_substr.size() > 0)
            split.push_back(matched_substr);

        input = match.suffix().str();
    }

    if(input.size() > 0)
        split.push_back(input);

    return split;
}

std::string ParseText::Join(const std::vector<std::string>& input_vec)
{
    std::string join = "";
    if(input_vec.size() > 0)
    {
        join = input_vec.at(0);
        for (size_t i = 1; i < input_vec.size(); i++)
            join += (" " + input_vec.at(i));
    }
    return join;
}

bool ParseText::ExtractQuotedSections(const std::string& input_string,
                                      std::map<int, std::string>& quoted_sections,
                                      std::map<int, std::string>& unquoted_sections) const
{
    // Determine if quoted text exists within the input string.
    size_t n_quotes = std::count(input_string.begin(), input_string.end(), '\"');

    // There must be an even count of quote instances to be considered.
    if (n_quotes > 0 && n_quotes % 2 == 0)
    {
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
            if (pos >= quote_pos[quote_pos_index] && pos <= quote_pos[quote_pos_index + 1])
            {
                // Get the quoted section and save it in the map.
                section_len = quote_pos[quote_pos_index + 1] - (quote_pos[quote_pos_index] + 1);
                quoted_sections[total_index] = input_string.substr(
                    quote_pos[quote_pos_index] + 1, section_len);

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
                    if (total_index > 0)
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

bool ParseText::IsASCII(const std::string& test_str) const
{
    // Return false for empty strings.
    if (test_str.length() == 0)
        return false;

    // Check each character in test_str
    for (int i = 0; i < test_str.length(); i++)
    {
        if (test_str.at(i) < 0)
            return false;
    }
    return true;
}

bool ParseText::IsUTF8(const std::string& test_str) const
{
    // See table 3-7, "Well-Formed UTF-8 Byte Sequences"
    // in http://www.unicode.org/versions/Unicode6.2.0/ch03.pdf

    // Return false for empty strings.
    if (test_str.length() == 0)
        return false;

    uint8_t byte_val = 0;
    uint8_t seq_count = 0;
    uint8_t seq_ind = 0;
    for (int i = 0; i < test_str.length(); i++)
    {
        byte_val = test_str.at(i);

        // Check for single byte (1B) sequence
        if (byte_val >= 0x00 && byte_val <= 0x7F)
            seq_count = 1;
        // 2B sequence
        else if (byte_val >= 0xC2 && byte_val <= 0xDF)
            seq_count = 2;
        // 3B sequence
        else if (byte_val >= 0xE0 && byte_val <= 0xEF)
        {
            seq_count = 3;

            // 0xE0 case
            if (byte_val == 0xE0 && (uint8_t)test_str.at(i + 1) < 0xA0)
                return false;

            // 0xED case
            if (byte_val == 0xED && (uint8_t)test_str.at(i + 1) > 0x9F)
                return false;
        }
        // 4B sequence
        else if (byte_val >= 0xF0 && byte_val <= 0xF4)
        {
            seq_count = 4;

            // 0xF0 case
            if (byte_val == 0xF0 && (uint8_t)test_str.at(i + 1) < 0x90)
                return false;

            // 0xF4 case
            if (byte_val == 0xF4 && (uint8_t)test_str.at(i + 1) > 0x8F)
                return false;
        }
        else
            return false;

        for (seq_ind = 1; seq_ind < seq_count; seq_ind++)
        {
            i++;
            byte_val = test_str.at(i);
            if (byte_val < 0x80 || byte_val > 0xBF)
                return false;
        }
    }
    return true;
}

std::string ParseText::ToLower(const std::string& input_str) const
{
    std::string lower_str = input_str;
    for (size_t i = 0; i < lower_str.length(); i++)
    {
        if (!std::islower(lower_str[i]))
            lower_str[i] = std::tolower(lower_str[i]);
    }
    return lower_str;
}

std::string ParseText::ToUpper(const std::string& input_str) const
{
    std::string upper_str = input_str;
    for (size_t i = 0; i < upper_str.length(); i++)
    {
        if (!std::isupper(upper_str[i]))
            upper_str[i] = std::toupper(upper_str[i]);
    }

    return upper_str;
}

std::string ParseText::Replace(std::string input_string, std::string find_string,
    std::string replace_string)
{
    std::string mod = input_string;
    size_t pos = 0;
    size_t find_str_len = find_string.size();
    size_t repl_str_len = replace_string.size();
    while((pos = mod.find(find_string, pos)) != std::string::npos)
    {
        mod = mod.replace(pos, find_str_len, replace_string);
        pos += repl_str_len;
    }
    return mod;
}
