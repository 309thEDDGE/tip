#include "cli_flag.h"

CLIFlag::CLIFlag(std::string label, std::string short_label,
    const std::string& help_str, bool the_default, bool& output) : 
    CLIOptionalArg(label, short_label, help_str, the_default, output)
{ 
    arg_type_ = CLIArgType::FLAG;
    label_user_rgx_ = "(^|.*\\s)" + label + "(\\s.*|$)";
    short_label_user_rgx_ = "(^|.*\\s)" + short_label + "(\\s.*|$)";

    // CLIFlags are always valid because a flag is either not present (default value)
    // or present (non-default value).
    arg_valid_ = true;
}

bool CLIFlag::Parse(const std::string& input)
{
    if(!ValidateUser(input))
    {
        parsed_value_ = default_value_;
        return true;
    }
    arg_present_ = true;
    parsed_value_ = !default_value_;
    return true;
}

std::shared_ptr<CLIArg> CLIFlag::Make(const std::string& label, std::string short_label, 
    const std::string& help_str, bool the_default, bool& output)
{
    std::shared_ptr<CLIArg> sptr = std::make_shared<CLIFlag>(
        label, short_label, help_str, the_default, output);
    return sptr;
}

std::string CLIFlag::GetUsageRepr()
{
    std::string repr = "[";
    if(label_.size() != 0)
    {
        repr += label_;
        if(short_label_.size() != 0)
            repr += (" | " + short_label_);
    }
    else
        repr += short_label_;
    repr += "]";

    return repr;
}
