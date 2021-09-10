#include "uri_percent_encoding.h"

URIPercentEncoding::URIPercentEncoding() : parse_text_() 
{
}

bool URIPercentEncoding::PercentEncodeReservedASCIIChars(const std::string& input, std::string& output)
{
    std::stringstream encoded;

    if(!parse_text_.IsASCII(input))
    {
        output = "";
        return false;
    }

    for (std::string::const_iterator it = input.cbegin(); it != input.cend(); ++it)
    {
        if(URI_unreserved_characters_.count(*it) == 0)
            EncodeChar(*it, encoded);
        else
            AppendChar(*it, encoded);
    }

    output = encoded.str();
    return true;
}

void URIPercentEncoding::EncodeChar(const char& input_char, std::stringstream& enc_stream)
{
    byte_val_ = input_char;
    enc_stream << "%" 
        << std::setfill('0') << std::setw(2) 
        << std::hex << static_cast<int>(byte_val_);
}

void URIPercentEncoding::AppendChar(const char& input_char, std::stringstream& enc_stream)
{
    enc_stream << input_char;
}


