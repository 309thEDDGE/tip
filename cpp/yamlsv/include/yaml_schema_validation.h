#ifndef YAML_SCHEMA_VALIDATION_H_
#define YAML_SCHEMA_VALIDATION_H_

#include <vector>
#include <cstdarg>
#include <string>
#include <algorithm>
#include <ostream>
#include "yaml-cpp/yaml.h"
#include "parse_text.h"
#include "yamlsv_log_item.h"
#include "yamlsv_schema.h"

class YamlSV
{
   private:
    ParseText parse_text_;

    static const int buff_size_ = 512;
    char buffer_[buff_size_];

    std::string bool_tolower_;
    bool is_opt_;

    // Passed to CheckDataTypeString as the indicator
    // that user-defined allowed values are present
    bool has_allowed_vals_operator_;

    // Parsed user-defined allowed values represented
    // by strings
    std::vector<std::string> allowed_values_;

    // String representation of a scalar value under test
    std::string scalar_test_str_;

    // String representation of a schema string
    std::string schema_str_;

    std::string str_type_;

   public:
    YamlSV();
    ~YamlSV();

    /*
	Validate a yaml node against a schema node.

	Args:
		test_node	--> root node of yaml matter under test
		schema_node	--> root node of yaml schema, see schema readme
		log_output	--> vector of LogItem filled during validation. Facilitates use
						of arbitrary logging mechanism.

	Return:
		True if yaml matter under test is validated, false otherwise.
	*/
    bool Validate(const YAML::Node& test_node, const YAML::Node& user_schema_node,
                  std::vector<LogItem>& log_output);

    /*
	Validate a yaml document against a schema document.

	Args:
		test_node	--> string containing yaml matter under test
		schema_node	--> string containing yaml schema, see schema readme
		log_output	--> vector of LogItem filled during validation. Facilitates use
						of arbitrary logging mechanism.

	Return:
		True if yaml matter under test is validated, false otherwise.
	*/
    bool Validate(const std::string& test_doc, const std::string& schema_doc,
                  std::vector<LogItem>& log_output);

    /*
	Print the last print_count log items to a stringstream.

	Args:
		log_output	--> vector of LogItem to be printed
		print_count --> Print up to the last print_count log items, 
						total count permitting
		stream		--> iostream or derived class object to which log items
						information is printed
	*/
    static void PrintLogItems(const std::vector<LogItem>& log_items, int print_count,
                              std::ostream& stream);

    /////////////////////////////////////////////////////////////////////////////////
    // Internal use functions below
    /////////////////////////////////////////////////////////////////////////////////

    /*
	Create a LogItem object with a given LogLevel and message.

	Args: 
		log_output	--> vector of LogItem into which the new LogItem
						will be added
		level		--> LogLevel specified for the new LogItem
		message		--> String message specified for the new LogItem
	*/
    void AddLogItem(std::vector<LogItem>& log_output, LogLevel level,
                    std::string message);

    /*
		Create a LogItem object with a given LogLevel and message,
		which is created by a format string and variable argument
		count.

	Args: 
		log_output	--> vector of LogItem into which the new LogItem
						will be added
		level		--> LogLevel specified for the new LogItem
		fmt			--> A printf style format C-string to be used 
						with the values passed in the remaining arguments
		...			--> N arguments passed to the format string
	*/
    void AddLogItem(std::vector<LogItem>& log_output, LogLevel level,
                    const char* fmt, ...);

    /*
	Compare a schema node against a test node. Handles maps, sequences, and 
	scalars. Assumes the two nodes have the same structure. Intended to be 
	used recursively.

	Args:
		test_node	--> Node to be tested against the schema
		schema_node	--> Schema defined in YAML and read into a node
		log_output	--> Vector of log items to which will be added any 
						log entries generated during this function call

	Return:
		False if test matter does not conform to schema or an error occurs, 
		true otherwise.
	*/
    bool ProcessNode(const YAML::Node& test_node, const YAML::Node& schema_node,
                     std::vector<LogItem>& log_output);

    /*
	Test a value passed as a string against one of the schema data types.
	Based on the ParseText::TextIsInteger/Float functions.

	Args:
		str_type	--> The data type retrieved from the schema to be used
						to verify the input value. One of the elements 
						in YamlSVSchemaType.
		test_val	--> String representation of the raw value to be tested
						for compatibility with the schema data type.

	Return:
		False if the str_type is invalid or the test_val is not compatible
		with the schema data type represented by str_type, true otherwise.
	*/
    bool VerifyType(const std::string& str_type, const std::string& test_val);

    /*
	Test a map element (key-value pair) against a schema element (key-value pair
	which contains the schema data type) via YAML::const_iterator (node iterators).

	Args:
		schema_it	--> Iterator to a schema map element
		test_it		--> Iterator to a test map element
		log_output	--> Vector of log items to which will be added any 
						log entries generated during this function call

	Return:
		False if the mapped test element is not compatible with the mapped
		schema data type or the mapped type is a map or sequence which
		has data not compatible with the schema data type indicated in the 
		schema-mapped sequence or map, otherwise true.
	*/
    bool TestMapElement(YAML::const_iterator& schema_it,
                        YAML::const_iterator& test_it, std::vector<LogItem>& log_output);

    /*
	Test a sequence node against a test node.

	Args:
		schema_node	--> Node containing a sequence with a single value, which
						is a string YamlSVSchemaType
		test_node	--> Node containing a sequence of n values, each of which
						is to be tested for compatibility with the schema data type
		log_output	--> Vector of log items to which will be added any 
						log entries generated during this function call

	Return:
		True if each value in the test_node is compatible with the schema 
		data type, false otherwise of if there is an error.
	*/
    bool TestSequence(const YAML::Node& schema_node, const YAML::Node& test_node,
                      std::vector<LogItem>& log_output);

    /*
	Get a string representing the standard schema type, after checking for 
	any modifier characters.

	Args:
		test_type	--> String representing the schema type as read from the 
						schema yaml.
		str_type	--> The string representation of the schema type if the 
						test_type is valid.
		is_opt		--> Set the value to true if the schema is valid
		has_allowed_vals_operator	--> Set to true if the schema string contains
		                            the arrow characters ("-->") in the format
									[OPT][string type]-->{list, of, allowed, vals}

	Return:
		True if the test_type includes a valid schema type with or without
		valid modifier characters.
	*/
    bool CheckDataTypeString(const std::string& test_type, std::string& str_type,
                             bool& is_opt, bool& has_allowed_vals_operator);

    /*
	Extract the allowed values indicated by the values in braces that follow 
	the allowed vals operator ("-->") as a vector of strings.

	Example: Input is the schema string: "INT-->{0, 1, 2}", get vector of
	string values ["0", "1", "2"]

	Args:
		test_type	--> String representing the schema type as read from the 
						schema yaml.
		allowed_vals--> Vector of string representation of allowed values
		log_output	--> Vector of log items to which will be added any 
						log entries generated during this function call

	Return:
		False if a brace does not immediately follow the allowed vals operator
		or if the closing brace is not present or if the string values cannot
		be separated by comma delimiter, if present. Otherwise true.
	*/
    bool ParseAllowedValues(const std::string& test_type,
                            std::vector<std::string>& allowed_vals,
                            std::vector<LogItem>& log_output);

    /*
	Compare a test string against a vector of possible matches. Used
	to check if the test string is one of the possible allowed values 
	which the schema can indicate via the allowed values operator. 
	To be used in conjunction with ParseAllowedValues().

	Args:
		test_string		--> String to be compared against all possible
							allowed values
		allowed_vals	--> Vector of string representation of allowed values

	Return:
		True if the test_string matches one of the allowed_vals and false if
		there is no match.

	*/
    bool CompareToAllowedValues(const std::string& test_string,
                                const std::vector<std::string>& allowed_vals);
};

#endif