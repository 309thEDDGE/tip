#ifndef URI_PERCENT_ENCODING_H_
#define URI_PERCENT_ENCODING_H_

#include <string>
#include <set>
#include <cstdint>
#include <iostream>
#include <ios>
#include <iomanip>
#include <sstream>
#include "parse_text.h"

// See https://en.wikipedia.org/wiki/Percent-encoding
class URIPercentEncoding
{

private:

    // For functions such as IsASCII(), IsUTF8()
    ParseText parse_text_;

    // Hold unsigned byte value converted from char.
    uint8_t byte_val_;

public:
    const std::set<char> URI_unreserved_characters_ = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                                                 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
                                                 'q', 'r', 's', 't', 'u', 'v', 'w',
                                                 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E',
                                                 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
                                                 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
                                                 'W', 'X', 'Y', 'Z', '0', '1', '2', '3',
                                                 '4', '5', '6', '7', '8', '9', '-', '_', 
                                                 '.', '~'}; 



    URIPercentEncoding();


    /////////////////////////////////////////////////////////////////////
    //                         User Functions 
    /////////////////////////////////////////////////////////////////////

    /*
    Percent-encode all characters in a given ASCII string that correspond
    to reserved characters and all other characters that are not in
    the unreserved set.

    Args:
        input       --> String for which characters will be encoded
        output      --> Output string with characters encoded

    Return:
        True if all input chars can be encoded, if necessary. False 
        if the input string is not ASCII or a character fails to be
        encoded for some reason.  
    */
    bool PercentEncodeReservedASCIIChars(const std::string& input, std::string& output);



    /////////////////////////////////////////////////////////////////////
    //                         Internal Functions 
    /////////////////////////////////////////////////////////////////////

    /*
    Percent-encode arbitrary char and append it to a stream. Intended to be
    used with ASCII and UTF-8 percent-encoding conversions. Chars are passed
    because it is the default type in std::string. This function handles
    casting of char to unsigned char for possible non-ASCII bytes.

    Args:
        input_char  --> Character to encode. Intended to be char from 
                        std::string which may be ASCII or UTF8
        enc_stream  --> Stream to which encoded chars are appended
    */
    void EncodeChar(const char& input_char, std::stringstream& enc_stream);



    /*
    Append char to stream without encoding.

    Note: This function may be too simple and should be removed. 

    Args:
        input_char  --> Character to encode. Intended to be char from 
                        std::string which may be ASCII or UTF8
        enc_stream  --> Stream to which encoded chars are appended
    */
    void AppendChar(const char& input_char, std::stringstream& enc_stream);

}; // end class URIPercentEncoding
#endif // #ifndef URI_PERCENT_ENCODING_H_