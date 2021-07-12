#include "iterable_tools.h"

IterableTools::IterableTools()
{
}

IterableTools::~IterableTools()
{
}

std::string IterableTools::print(std::string print_string)
{
    printf("%s", print_string.c_str());
    return print_string;
}

std::string IterableTools::GetHeader(std::vector<std::string> columns, std::string title)
{
    std::stringstream ss;
    // Name of the header
    ss << "\n  " << title << "\n";

    for (int i = 0; i < columns.size(); i++)
    {
        ss << " (" << columns[i] << ") |";
    }
    if (columns.size() > 0)
        ss.seekp(-1, std::ios_base::end);
    ss << "\n"
       << GetPrintBar() << "\n";
    return ss.str();
}

std::string IterableTools::GetPrintBar()
{
    return "-------------------------------";
}

template <>
std::string IterableTools::GetIterablePrintString<std::vector<std::string>>(
    const std::vector<std::string>& input_iterable, std::string title,
    std::string format_spec, std::string delim)
{
    int i = 0;
    std::string ret_val = title + ":\n";
    std::string format_and_delim(delim + format_spec);
    char buff[100];
    for (std::vector<std::string>::const_iterator it = input_iterable.begin();
         it != input_iterable.end(); ++it)
    {
        if (i == 0)
        {
            snprintf(buff, format_spec.size(), format_spec.c_str(), it->c_str());
        }
        else
        {
            snprintf(buff, format_and_delim.size(), format_and_delim.c_str(), it->c_str());
        }
        ret_val += buff;
        i++;
    }
    ret_val += "\n";
    return ret_val;
}

//template<typename Key>
//std::string IterableTools::GetPrintableMapElements_KeyToValue<Key, bool>(const std::map<Key, bool>& input_map)
//{
//	std::stringstream ss;
//	for (typename std::map<Key, Val>::const_iterator it = input_map.begin(); it != input_map.end(); it++)
//	{
//		ss << " " << it->first << ":\t";
//		ss << std::boolalpha << it->second;
//		ss << "\n";
//	}
//
//	return ss.str();
//}