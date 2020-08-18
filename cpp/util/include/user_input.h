#ifndef USERINPUT_H
#define USERINPUT_H

#include <string>
#include <iostream>
#include "parse_text.h"
#include "iterable_tools.h"

class UserInput
{
private:
	ParseText parse_text_;
	IterableTools iterable_tools_;

public:
	UserInput() {};
	~UserInput() {};

	// Gets an unsigned integer from the user
	// Continues to prompt the user until the input is valid or
	// the user enters "q" to quit
	//
	// Returns true:  When the user input is a positive integer (if valid_options not provided)
	//                When the user input exists in valid_options (if valid_options provided)
	// Returns false: When the user enters "q" to quit
	//
	// Inputs:	output		  -> returned unsigned integer from user
	//			prompt		  -> prompt shown to the user when asking for input
	//			valid_options -> If provided, the user is required to enter
	//                           one of the values specified
	//			test_input	  -> used for unit tests to bypass user input
	bool GetUnsignedInt(uint64_t& output, 
		std::string prompt, 
		const std::set<uint64_t> * const valid_options = NULL,
		std::vector<std::string> * const test_input = NULL);

	// Gets a string from the user
	// Continues to prompt the user until the input is valid or
	// the user enters "q" to quit
	//
	// Returns true:  if the user enters one of the valid_options,
	//				  or if the user enters anything but "q" when
	//				  valid_options are not provided
	// Returns false: if the user enters "q"
	//
	// Inputs:	output		  -> returned string from user
	//			prompt		  -> prompt shown to the user when asking for input
	//			valid_options -> If provided, the user is required to enter
	//                           one of the values specified
	//			test_input	  -> used for unit tests to bypass user input
	bool GetString(std::string& output, 
		std::string prompt, 
		const std::set<std::string> * const valid_options = NULL,
		std::vector<std::string> * const test_input = NULL);

};

#endif
