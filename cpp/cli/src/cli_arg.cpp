#include "cli_arg.h"

const int CLIArg::minimum_help_string_char_width_ = 40;
const std::string CLIArg::whitespace_code = ContainerArg::whitespace_code;

CLIArg::CLIArg(const std::string& label, const std::string& help_str, 
    const std::string& the_default) : label_(label), 
    arg_type_(CLIArgType::BASE), pt_(), default_(the_default), arg_valid_(false),
    config_rgx_(""), user_rgx_(""), arg_present_(false), help_str_(help_str),
    container_arg_ptr_(nullptr), is_container_arg_(false), use_simple_format_(false)
{}

bool CLIArg::FormatString(const std::string& help_str, int max_char_width,
    const size_t& indent, std::string& fmt_str, std::string sep)
{

    if (max_char_width < minimum_help_string_char_width_)
    {
        printf("CLIArg::FormatHelpString: Require max_char_width >= %d\n", minimum_help_string_char_width_);
        return false;
    }

    // printf("help_str size: %zu\n", help_str.size());
    ParseText pt;
    std::vector<std::string> components;
    std::string cleaned = help_str;
    if(pt.CheckForExisitingChar(help_str, '\n'))
    {
        components = pt.Split(help_str, '\n', true);
        cleaned = pt.Join(components);
    }
    components = pt.Split(cleaned);
    cleaned = pt.Join(components);

    std::string indent_str = "";
    size_t cleaned_size = 0;
    if(indent > 0)
    {
        for(size_t i = 0; i < indent; i++)
            indent_str += " ";
        std::string temp_cleaned = indent_str + cleaned;
        cleaned_size = temp_cleaned.size();
    }
    else
        cleaned_size = cleaned.size();

    // printf("cleaned size is %zu, cleaned str is:\n%s\n", cleaned_size, (indent_str + cleaned).c_str());
    if (cleaned_size <= max_char_width)
        fmt_str = (indent_str + cleaned);
    else
    {
        fmt_str = "";
        if (sep.size() == 1)
        {
            char sep_char = sep.at(0);
            components = pt.Split(cleaned, sep_char, true);
        }
        else
            components = pt.Split(cleaned, sep, true);

        std::string curr_line = (indent_str + components.at(0));
        for (size_t i = 1; i < components.size(); i++)
        {
            // printf("curr_line: \"%s\"\n", curr_line.c_str());
            if((curr_line.size() + 1 + components.at(i).size()) < max_char_width)
                curr_line += (" " + components.at(i));
            else
            {
                fmt_str += (curr_line + "\n");
                curr_line = (indent_str + components.at(i));
            }
        }
        fmt_str += curr_line;
    }

    return true;
}

bool CLIArg::FormatString(const std::string& help_str, const size_t& indent,
    std::string& fmt_str)
{
    fmt_str = "";

    std::string indent_str = "";
    if(indent > 0)
    {
        for(size_t i = 0; i < indent; i++)
            indent_str += " ";
    }

    ParseText pt;
    std::vector<std::string> components;
    if(pt.CheckForExisitingChar(help_str, '\n'))
    {
        components = pt.Split(help_str, '\n', true);
    }
    else
    {
        fmt_str = (indent_str + help_str);
        return true;
    }
        
    std::string curr_line = (indent_str + components.at(0));
    for (size_t i = 1; i < components.size(); i++)
    {
        fmt_str += (curr_line + "\n");
        curr_line = (indent_str + components.at(i));
    }
    fmt_str += curr_line;

    return true;
}

bool CLIArg::GetHelpString(int max_char_width, const size_t& indent, 
    std::string& fmt_str)
{
    std::map<int, std::string> nonescaped;
    std::map<int, std::string> escaped;
    if(FindEscapedComponents(help_str_, nonescaped, escaped))
    {
        size_t component_count = nonescaped.size() + escaped.size();
        std::string temp = "";
        std::string formatted = "";
        for(size_t i = 0; i < component_count; i++)
        {
            if(escaped.count(i) == 1)
            {
                if(!FormatString(escaped.at(i), indent, temp))
                    return false;
            }
            else
            {
                if(!FormatString(nonescaped.at(i), max_char_width, indent, temp, " "))
                    return false;
            }

            if(i > 0)
                formatted += ("\n" + temp);
            else
                formatted += temp;
        }
        
        fmt_str = formatted;
        return true;
    }

    if(use_simple_format_)
        return FormatString(help_str_, indent, fmt_str);
    else
        return FormatString(help_str_, max_char_width, indent, fmt_str, " ");
}

bool CLIArg::IdentifyArgument(const std::string& input, const std::string& label, 
    std::string& parsed)
{
    if (input.size() == 0)
    {
        printf("CLIArg::IdentifyArgument: User input string is empty\n");
        parsed = "";
        return false;
    }

    // single word string without spaces
    std::string regex_str = ".*" + label + "\\s(\\w+).*";
    // printf("input: %s\n", input.c_str());
    // printf("regex_str: %s\n", regex_str.c_str());
    std::regex rgx(regex_str);
    std::smatch sm;
    if(std::regex_match(input, sm, rgx))
    {
        // printf("found: %s\n", sm[1].str().c_str());
        // printf("found size: %zu\n", sm.size());
        parsed = sm[1].str();
        return true;
    }
    
    // quoted string
    regex_str = ".*" + label + "\\s\"([^\"]*)\".*";
    std::regex rgx2(regex_str);
    std::smatch sm2;
    if(std::regex_match(input, sm2, rgx2))
    {
        // printf("found: %s\n", sm2[1].str().c_str());
        // printf("found size: %zu\n", sm2.size());
        parsed = sm2[1].str();
        return true;
    }

    parsed = "";
    printf("CLIArg::IdentifyArgument: No match for \"%s\"\n", label.c_str());
    return false;
}

bool CLIArg::CheckLabel(const std::string& label)
{
    size_t pos = std::string::npos;
    if((pos = label.find("--")) == std::string::npos)
        return false;

    if (pos != 0)
        return false;

    char c;
    for(size_t i = pos+2; i < label.size(); i++)
    {
        c = label.at(i);
        if(!IsAllowedChar(c))
            return false;
    }
    return true;
}

bool CLIArg::CheckShortLabel(const std::string& short_label)
{
    size_t pos = std::string::npos;
    if((pos = short_label.find('-')) == std::string::npos)
        return false;

    if (pos != 0)
        return false;

    if (short_label.size() > 2)
        return false;

    if (short_label.at(1) == '_')
        return false;

    if(!IsAllowedChar(short_label.at(1)))
        return false;

    return true;
}

bool CLIArg::CastArgument(const std::string& input, int& output)
{
    if(!pt_.TextIsInteger(input))
    {
        // printf("CLIArg::CastArgument(int): \"%s\" is not an integer\n", input.c_str());
        return false;
    }

    output = std::stoi(input);
    return true;
}

bool CLIArg::CastArgument(const std::string& input, double& output)
{
    if(!pt_.TextIsFloat(input))
    {
        // printf("CLIArg::CastArgument(double): \"%s\" is not a float\n", input.c_str());
        return false;
    }

    output = std::stod(input);
    return true;
}

bool CLIArg::CastArgument(const std::string& input, std::string& output)
{
    output = input;
    return true;
}

bool CLIArg::IsAllowedChar(char c)
{
    if ((c == 95) || ((c > 96) && (c < 123)) || ((c > 64) && (c < 91)) || ((c > 47) && (c < 58)))
        return true;

    return false;
}

std::shared_ptr<CLIArg> CLIArg::DefaultUseParentDirOf(const std::string& input_path)
{
    return shared_from_this();
}

void CLIArg::ComputeDefaultSpecialConfig()
{
    for (ArgConfVec::iterator it = default_arg_config_.begin(); it != default_arg_config_.end(); ++it)
        (*it)->Compute();
}

bool CLIArg::ComputeValidateSpecialConfig(const std::string& fail_msg)
{
    for (ArgConfVec::iterator it = validate_arg_config_.begin(); it != validate_arg_config_.end(); ++it)
    {
        if(!(*it)->Validate(fail_msg))
            return false;
    }
    return true;
}

bool CLIArg::FindEscapedComponents(const std::string& input_str, 
    std::map<int, std::string>& nonescaped, std::map<int, std::string>& escaped)
{
    std::string escape_seq = "**||";
    if(input_str.find(escape_seq) == std::string::npos) 
        return false;

    size_t pos = 0;
    size_t escape_seq_len = escape_seq.size();
    std::vector<size_t> escape_pos;
    while((pos = input_str.find(escape_seq, pos)) != std::string::npos)
    {
        escape_pos.push_back(pos);
        pos += escape_seq_len;
    }
    if((escape_pos.size() % 2) != 0)
        return false;

    std::vector<std::string> components = pt_.Split(input_str, escape_seq);
    if(escape_pos.at(0) == 0)
    {
        for(size_t i = 0; i < components.size(); i++)
        {
            if(i % 2 == 0)
                escaped[i] = components.at(i);
            else
                nonescaped[i] = components.at(i);
        }
    }
    else
    {
        for(size_t i = 0; i < components.size(); i++)
        {
            if(i % 2 == 0)
                nonescaped[i] = components.at(i);
            else
                escaped[i] = components.at(i);
        }
    }

    return true;
}
