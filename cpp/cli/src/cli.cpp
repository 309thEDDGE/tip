#include "cli.h"


CLI::CLI(const std::string& prog_name, const std::string& description) : program_name_(prog_name),
    description_(description), pos_arg_count_(0)
{
    size_t col_count, row_count; 
    if(!GetTerminalSize(row_count, col_count))
        max_width_ = 100;
    else
        max_width_ = col_count;
}

CLI::CLI(const std::string& prog_name, const std::string& description, size_t max_width) : 
    program_name_(prog_name), description_(description), pos_arg_count_(0), max_width_(max_width)
{}


bool CLI::Parse(int argc, char* argv[])
{
    std::vector<std::string> pos_args;
    if(!ValidateUserInput(argc, argv, args_, pos_args))
        return false;
    return Parse(argc, argv, pos_args);
}

bool CLI::Parse(int argc, char* argv[], const std::vector<std::string>& pos_args)
{
    if(!ParsePositionalArgs(pos_args, args_, pos_arg_count_))
        return false;

    // std::string user_args = ConcatenateUserArgs(argc, argv, 0);
    std::string user_args = ConcatenateUserArgs(argc, argv, args_);
    if(!ParseOptionalArgs(user_args, args_))
        return false;

    if(!ParseFlags(user_args, args_))
        return false;

    return true;
}

bool CLI::CheckConfiguration()
{
    args_ = SortArgs(args_, pos_arg_count_);
    for(std::vector<std::shared_ptr<CLIArg>>::iterator arg_it = args_.begin(); 
        arg_it != args_.end(); ++arg_it)
    {
        if(!((*arg_it)->ValidateConfig()))
        {
            printf("CLI::CheckConfiguration: Validation failed for arg: \"%s\"\n", 
                (*arg_it)->GetPrintName().c_str());
            return false;
        }
    }
    return true;
}

ArgsVec CLI::SortArgs(const ArgsVec& args_vec, size_t& pos_arg_count)
{
    ArgsVec sorted_vec;
    pos_arg_count = 0;

    // Positional
    for(ArgsVec::const_iterator it = args_vec.cbegin(); it != args_vec.cend(); ++it)
    {
        if((*it)->ArgType() == CLIArgType::POS)
        {
            pos_arg_count++;
            sorted_vec.push_back(*it);
        }
    }

    // Optional
    for(ArgsVec::const_iterator it = args_vec.cbegin(); it != args_vec.cend(); ++it)
    {
        if((*it)->ArgType() == CLIArgType::OPT)
        {
            sorted_vec.push_back(*it);
        }
    }

    // flags
    for(ArgsVec::const_iterator it = args_vec.cbegin(); it != args_vec.cend(); ++it)
    {
        if((*it)->ArgType() == CLIArgType::FLAG)
        {
            sorted_vec.push_back(*it);
        }
    }

    return sorted_vec;
}

std::unordered_map<std::string, int> CLI::MakeLabelLookupMap(const ArgsVec& args)
{
    std::unordered_map<std::string, int> lookup;
    int arg_count = 0;
    for (ArgsVec::const_iterator it = args.cbegin(); it != args.cend(); ++it)
    {
        if((*it)->ArgType() == CLIArgType::OPT)
            arg_count = 1;
        else if((*it)->ArgType() == CLIArgType::FLAG)
            arg_count = 0;
        else
            continue;

        if((*it)->GetLabel().size() > 0)
        {
            lookup[(*it)->GetLabel()] = arg_count;
        }
        if((*it)->GetShortLabel().size() > 0)
        {
            lookup[(*it)->GetShortLabel()] = arg_count;
        }
       
    }
    return lookup;
}

bool CLI::ParsePositionalArgs(const std::vector<std::string>& pos_args, 
    const ArgsVec& sorted_args, const size_t& pos_argc)
{
    if(pos_argc > 0)
    {
        if(pos_argc < pos_args.size())
        {
            printf("Too many positional arguments, %zu needed:\n", pos_argc);
            for(size_t i = 0; i < pos_args.size(); i++)
                printf("(%zu) \"%s\"\n", i, pos_args.at(i).c_str());
            printf("\n");
            return false;
        }
        else if(pos_argc > pos_args.size())
        {
            printf("Not enough positional arguments, %zu needed.\n\n", pos_argc);
            return false;
        }

        size_t index = 0;
        for(ArgsVec::const_iterator it = sorted_args.cbegin(); it != sorted_args.cend(); ++it)
        {
            if((*it)->ArgType() == CLIArgType::POS)
            {
                if(!(*it)->Parse(pos_args.at(index)))
                    return false;

                index++;
            }        
        }
    }
    return true;
}

std::string CLI::ConcatenateUserArgs(int argc, char* argv[], int pos_argc)
{
    std::string user_args = "";
    for(int i = (pos_argc+1); i < argc; i++)
    {
        std::string temp(argv[i]);
        if(i == (pos_argc+1))
            user_args = temp;
        else
            user_args += (" " + temp);
    }
    return user_args;
}

std::string CLI::ConcatenateUserArgs(int argc, char* argv[], const ArgsVec& cli_args)
{
    ParseText pt;
    std::string concat_args = "";
    std::vector<std::string> user_args = VectorizeUserArgs(argc, argv);
    bool encode = false;
    std::string curr_arg = "";
    for(size_t i = 0; i < user_args.size(); i++)
    {
        curr_arg = user_args.at(i);
        if(encode)
        {
            curr_arg = pt.Replace(curr_arg, " ", CLIConf::GetWhitespaceCode());
            encode = false;
        }
        else
        {
            for(ArgsVec::const_iterator it = cli_args.cbegin(); it != cli_args.cend(); ++it)
            {
                if((*it)->ArgType() == CLIArgType::OPT)
                {
                    if((*it)->GetLabel() == curr_arg || (*it)->GetShortLabel() == curr_arg)
                        encode = true;
                }
            }
        }

        if(i == 0)
            concat_args = curr_arg;
        else
            concat_args += (" " + curr_arg);
    }
    return concat_args;
}

bool CLI::ParseOptionalArgs(const std::string& user_args, const ArgsVec& sorted_args)
{
    for(ArgsVec::const_iterator it = sorted_args.cbegin(); it != sorted_args.cend(); ++it)
    {
        if((*it)->ArgType() == CLIArgType::OPT)
        {
            // printf("parsing opt arg %s for: %s\n", (*it)->GetPrintName().c_str(),
            //     user_args.c_str());

            // The only condition which should throw an error is a
            // positive match on the arg AND failure to cast, otherwise
            // a negative match indicates that the optional arg was not
            // utilized.
            if(!(*it)->Parse(user_args))
                return false;
        }        
    }

    return true;
}

bool CLI::ParseFlags(const std::string& user_args, const ArgsVec& sorted_args)
{
    for(ArgsVec::const_iterator it = sorted_args.cbegin(); it != sorted_args.cend(); ++it)
    {
        if((*it)->ArgType() == CLIArgType::FLAG)
        {
            (*it)->Parse(user_args);
        }        
    }

    return true;
}

std::string CLI::MakeHelpString()
{
    // printf("sorted_args size: %zu\n", args_.size());
    size_t usage_len = 0;
    std::string general_usage = MakeGeneralUsageString(program_name_, usage_len);
    general_usage += "\n";

    std::string arg_specific_usage = MakeArgSpecificUsageString(program_name_,
        max_width_, usage_len, args_);
    // printf("Arg spec usage string:\n%s\n", arg_specific_usage.c_str());
    arg_specific_usage += "\n\n";

    size_t max_arg_print_name_width = 0;
    for(ArgsVec::const_iterator it = args_.cbegin(); it != args_.cend(); ++it)
    {
        if((*it)->GetPrintName().size() > max_arg_print_name_width)
            max_arg_print_name_width = (*it)->GetPrintName().size();
    }
    // printf("max name width: %zu\n", max_arg_print_name_width);

    std::string arg_descriptions("");
    for(ArgsVec::const_iterator it = args_.cbegin(); it != args_.cend(); ++it)
    {
        arg_descriptions += (MakeArgHelpString(*it, max_width_, max_arg_print_name_width+1) + "\n\n");
    }

    std::string formatted_desc = "";
    CLIArg::FormatString(description_, max_width_, 0, formatted_desc, " ");

    return general_usage + arg_specific_usage + formatted_desc + "\n\n" + arg_descriptions;
}

std::string CLI::MakeGeneralUsageString(const std::string& prog_name, size_t& indent)
{
    std::string general_usage("Usage: ");
    indent = general_usage.size();
    general_usage += (prog_name + " <positional arguments> [options / flags]");
    return general_usage;
}

std::string CLI::MakeArgSpecificUsageString(const std::string& prog_name,
    const size_t& max_width, const size_t& indent, const ArgsVec& sorted_args)
{
    std::string arg_specific_usage = "";
    arg_specific_usage += prog_name;
    size_t updated_indent = indent + prog_name.size() + 1;
    for(ArgsVec::const_iterator it = sorted_args.cbegin(); it != sorted_args.cend(); ++it)
    {
        arg_specific_usage += (" " + (*it)->GetUsageRepr());
    }
    bool res = CLIArg::FormatString(arg_specific_usage, max_width, updated_indent, 
        arg_specific_usage, "] ");
    
    // printf("post-intmdt arg spec usage:\n%s\n", arg_specific_usage.c_str());
    return arg_specific_usage.substr(prog_name.size()+1);
}

std::string CLI::MakeArgHelpString(const std::shared_ptr<CLIArg>& arg,
    const size_t& max_width, const size_t& indent)
{
    std::string arg_label   = (arg)->GetPrintName() + " ";

    std::string arg_help = "";
    arg->GetHelpString(max_width, indent, arg_help);

    std::string arg_default = (arg)->GetDefaultString();
    CLIArg::FormatString(arg_default, max_width, indent, arg_default, " ");

    return (arg_label + arg_help.substr(arg_label.size()) + "\n" + arg_default);
}

std::vector<std::string> CLI::VectorizeUserArgs(int argc, char* argv[])
{
    std::vector<std::string> user_args;
    for (size_t i = 1; i < argc; i++)
    {
        std::string temp_arg(argv[i]);
        user_args.push_back(temp_arg);
    }
    return user_args;
}

bool CLI::ValidateUserInput(int argc, char* argv[], const ArgsVec& cli_args,
    std::vector<std::string>& pos_args)
{
    std::unordered_map<std::string, int> lookup = MakeLabelLookupMap(cli_args);
    std::vector<std::string> user_args = VectorizeUserArgs(argc, argv);

    std::string label;
    std::string short_label;
    std::vector<std::string>::const_iterator user_arg_it;
    std::vector<std::string> matched;
    std::vector<std::string>::const_iterator match_it;
    int remove_count = 0;
    std::vector<std::string>::iterator erase_begin_it;
    std::vector<std::string>::iterator erase_end_it;
    for(ArgsVec::const_iterator it = cli_args.cbegin(); it != cli_args.cend(); ++it)
    {
        if((*it)->ArgType() != CLIArgType::POS)
        {
            label = (*it)->GetLabel();
            short_label = (*it)->GetShortLabel();

            for(user_arg_it = user_args.cbegin(); user_arg_it != user_args.cend(); ++user_arg_it)
            {
                if(label == *user_arg_it)  
                    matched.push_back(label);
                else if(short_label == *user_arg_it)
                    matched.push_back(short_label);
            }

            for(match_it = matched.cbegin(); match_it != matched.cend(); ++match_it)
            {
                remove_count = lookup.at(*match_it);
                erase_begin_it = std::find(user_args.begin(), user_args.end(), *match_it);
                erase_end_it = erase_begin_it;

                size_t increment = 0;
                while(increment < (1 + remove_count))
                {
                    if(erase_end_it == user_args.end())
                        break;
                    erase_end_it++;
                    increment++;
                }

                user_args.erase(erase_begin_it, erase_end_it);
                // lookup.erase(*match_it); 
            }

            matched.clear();
        }
    }

    // Check if remaining match "-x", "--xxx".
    for(user_arg_it = user_args.cbegin(); user_arg_it != user_args.cend(); ++user_arg_it)
    {
        if(IsFlagOrOptArgCandidate(*user_arg_it))
        {
            printf("\nUnrecognized argument \"%s\"\n", user_arg_it->c_str());
            return false;
        }
    }
    pos_args = user_args;
    return true;
}

bool CLI::IsFlagOrOptArgCandidate(const std::string& input)
{
    if(input.size() < 2)
        return false;
    
    if(input.at(0) == '-')
    {
        if(input.at(1) == '-')
        {
            if(input.size() < 4)
                return false;
            for(size_t i = 2; i < input.size(); i++)
            {
                if(input.at(i) == '-')
                    return false;
            }
            return true;
        }
        else if(input.size() > 2)
            return false;
        else if(input.at(1) != '-')
            return true;
    }
    return false;
}
