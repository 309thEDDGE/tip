#ifndef PARSE_TEXT_H
#define PARSE_TEXT_H

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <regex>
#include "iterable_tools.h"

class ParseText
{
   private:
   public:
    ParseText() {}
    ~ParseText() {}

    /*
    Split string on a char delimiter.

    Args:
        input_string    --> String to be split
        delim           --> char on which to split

    Return:
        Vector of substrings from input_string which are separated by delim
    */
    std::vector<std::string> Split(std::string input_string, const char& delim);



    /*
    Split on string instead of char. 

    If delim = "] " (right bracket, whitespace), split on all whitespace
    chars preceded by "]". 

    Args:
        input_string    --> String to be split
        delim           --> string on which to split

    Return:
        Vector of substrings from input_string which are separated by delim
    */
    std::vector<std::string> Split(std::string input_string, const std::string& delim);



    /*
    Split on one or more whitespace, removing additional whitespace.

    Transform "hello, this    is   a line with various       whitespace  characters."
        --> vector<string>{"hello,", "this", "is", "a", "line", "with", "various", 
                           "whitespace", "characters."}

    Args:
        input_string    --> String to be split

    Return:
        Vector of substrings from input_string which are separated by delim
    */
    std::vector<std::string> Split(std::string input_string);



    /*
    Join a vector of strings into a single string by inserting a single
    whitespace character between elements of the vector.

    Args:
        input_vec       --> Vector of substrings to be joined

    Return:
        A single string composed of joined elements of a vector of strings.
    */
    std::string Join(const std::vector<std::string>& input_vec);



    /*
    Replace a substring with another substring.

    Args:
        input_string    --> String to be modified by replacement
        find_string     --> Substring in input_string to be replaced
        replace_string  --> String to be substitued in input_string 
                            for find_string
    Return:
        Modified string 
    */
    std::string Replace(std::string input_string, std::string find_string,
        std::string replace_string);
    


    bool ConvertInt(const std::string& convert_string, int& output);
    bool ConvertDouble(const std::string& convert_string, double& output);
    bool CheckForExisitingChar(const std::string& input_string, char character);
    std::string RemoveTrailingChar(const std::string& input_string, char character);
    bool TextIsInteger(const std::string& input_string);
    bool TextIsFloat(const std::string& input_string);

    // Extract quoted (in quotation marks) and non-quoted text into
    // maps with the key indicating the order in which the section originally
    // occurred. Return true if quoted sections present and false otherwise.
    // Input maps are only filled if true is returned.
    //
    // Strings returned in the quoted sections map have the original quotes
    // removed.
    bool ExtractQuotedSections(const std::string& input_string,
                               std::map<int, std::string>& quoted_sections,
                               std::map<int, std::string>& unquoted_sections);

    /*
	Return true if input string qualifies as ASCII; return false otherwise.
	An ASCII character is a byte value in the range [0, 127], i.e., the MSB
	is always zero. If a character is interpreted as a signed integer, then
	a valid ASCII character must not be < 0, or in other words, the sign bit
	must be 0.

	Empty string returns false.

	Args:
		test_str	--> Input string to be checked
	*/
    bool IsASCII(const std::string& test_str) const;

    /*
	Return true if input string satisfies utf-8 encoding characteristics, and
	false otherwise.

	Empty string returns false.

	Args:
		test_str	--> Input string to be checked
	*/
    bool IsUTF8(const std::string& test_str) const;

    /*
	Convert an input string to the lower-case representation of itself.

	Args:
		input_str	--> Input string to be converted to lower case

	Return:
		A string in which all of the characters in the original input string
		have been replaced by the lower-case representation. Does not affect
		non-alphabetical characters.
	*/
    std::string ToLower(const std::string& input_str) const;



    /*
	Convert an input string to the upper-case representation of itself.

	Args:
		input_str	--> Input string to be converted to upper case

	Return:
		A string in which all of the characters in the original input string
		have been replaced by the upper-case representation. Does not affect
		non-alphabetical characters.
	*/
    std::string ToUpper(const std::string& input_str) const;

};

#endif
