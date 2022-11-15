#include "container_arg.h"

bool ContainerArg::CheckFromCharsResultExactMatch(std::from_chars_result& status,
    const std::string& input_str) 
{
    if(status.ec == std::errc::invalid_argument) 
    {
        std::cout << "CheckFromCharsResultExactMatch: invalid_argument" << std::endl;
        std::cout << "pointer: " << std::quoted(status.ptr) << std::endl;
        return false;
    }
    if(status.ec == std::errc::result_out_of_range)
    {
        std::cout << "CheckFromCharsResultExactMatch: result_out_of_range" << std::endl;
        std::cout << "pointer: " << std::quoted(status.ptr) << std::endl;
        return false;
    }

    if (status.ptr != (input_str.data() + input_str.size()))
    {
        std::cout << "pointer: " << std::quoted(status.ptr) << std::endl;
        return false;
    }

    return true;
}

bool ContainerArg::SplitOnColon(const std::string& user_input, std::string& key,
    std::string& val)
{
    if(user_input.find(':') == std::string::npos)
        return false;

    std::vector<std::string> comp = pt_.Split(user_input, ':');
    if(comp.size() != 2)
        return false;

    key = comp.at(0);
    val = comp.at(1);

    if (key == "" || val == "")
        return false;

    return true;
}
