#include "cli_group.h"


CLIGroup::CLIGroup() : is_configured_(false), max_width_(0)
{
    size_t rows, cols;
    if(!GetTerminalSize(rows, cols))
    {
        printf("CLIGroup(): Failed to get terminal size\n");
        max_width_ = 100;
    }
    else
    {
        max_width_ = cols;
        // printf("CLIGroup(): Terminal width = %zu\n", max_width_);
    }
}

std::shared_ptr<CLIGroupMember> CLIGroup::AddCLI(const std::string& prog_name, const std::string& description,
    std::string nickname)
{
    std::shared_ptr<CLIGroupMember> clig = std::make_shared<CLIGroupMember>(prog_name, 
        description, max_width_);
    arg_group_[nickname] = clig;
    arg_group_labels_.push_back(nickname);
    return clig;
}

bool CLIGroup::CheckConfiguration()
{

    for(GroupMap::iterator it = arg_group_.begin(); it != arg_group_.end(); ++it)
    {
        if(it->second->GetArgCount() == 0)
        {
            printf("CLIGroup::CheckConfiguration: CLIGroupMember \"%s\" has zero args\n", 
                it->first.c_str());
            return false;
        }

        // printf("CLIGroup::CheckConfiguration of \"%s\"\n", it->first.c_str());
        if(!(it->second->CheckConfiguration()))
        {
            printf("CLIGroup::CheckConfiguration: CLIGroupMember \"%s\" configuration fail\n",
                it->first.c_str());
            return false;
        }
    }
    is_configured_ = true;
    return true;
}

bool CLIGroup::Parse(int argc, char* argv[], std::string& nickname, 
    std::shared_ptr<CLIGroupMember>& member)
{
    bool parse_fail = false;
    if(!is_configured_)
    {
        printf("CLIGroup::Parse: Not configured\n");
        parse_fail = true;
        return false;
    }

    if(argc == 1 && !parse_fail)
    {
        printf("No arguments present.\n\n");
        parse_fail = true;
    }

    if(!parse_fail)
    {
        std::shared_ptr<CLIGroupMember> curr_member;

        // Assemble all CLIArgs in a single vector for user input validation.
        ArgsVec all_args;
        for(GroupLabels::const_iterator it = arg_group_labels_.cbegin(); 
            it != arg_group_labels_.cend(); ++it)
        {
            curr_member = arg_group_.at(*it);
            // printf("curr_member cliarg count: %zu\n", curr_member->GetArgCount());
            all_args.insert(all_args.end(), curr_member->GetCLIArgs().begin(), 
                curr_member->GetCLIArgs().end());
            // printf("all_args size: %zu\n", all_args.size());
        }

        std::vector<std::string> pos_args;
        if(!CLI::ValidateUserInput(argc, argv, all_args, pos_args))
            return false;

        // printf("after validateuserinput\n");
        // fflush(stdout);
        bool indicators_found = false;
        for(GroupLabels::const_iterator it = arg_group_labels_.cbegin(); 
            it != arg_group_labels_.cend(); ++it)
        {
            indicators_found = false;
            curr_member = arg_group_.at(*it);
            // printf("Checking member with nickname %s\n", it->c_str());
            if(!(curr_member->Parse(argc, argv, pos_args)))
            {
                // printf("member with nickname %s fail\n", it->c_str());
                parse_fail = true;
                break;
            }
            // fflush(stdout);

            // bool print_on = false;
            // if(*it == "clifull")
                // print_on = true; 
            // Iterate over CLIArgs in the CLIGroupMember
            const ArgsVec& args = curr_member->GetCLIArgs();
            for(ArgsVec::const_iterator arg_it = args.cbegin(); arg_it != args.cend(); ++arg_it)
            {
                // if(print_on)
                    // printf("arg %s\n", (*arg_it)->GetUsageRepr().c_str());
                if(curr_member->IsGroupIndicator((*arg_it)->GetLabel()))
                {
                    indicators_found = true;
                    // if(print_on)
                    //     printf("is indicator for label: %s\n", (*arg_it)->GetLabel().c_str());
                    // if(print_on)
                    //     printf("ispresent: %d, isvalid: %d\n", (*arg_it)->IsPresent(), (*arg_it)->IsValid());
                    if((*arg_it)->IsPresent() && (*arg_it)->IsValid())
                    {
                        nickname = *it;
                        member = curr_member;
                        return true;
                    }
                }
            }
            // fflush(stdout);
        }
    }

    // printf("Input \"%s\" did not match any arguments\n\n", 
        // CLI::ConcatenateUserArgs(argc, argv, 0).c_str());
    // printf("%s", MakeHelpString().c_str());
    nickname = "";
    member = nullptr;
    return false;
}

std::string CLIGroup::MakeHelpString()
{
    std::string help = "";
    if(!is_configured_)
        return help;

    size_t indent = 0;
    bool first_iteration = true;
    std::string usage = "";
    std::string fmt_usage = "";
    std::string description;
    std::shared_ptr<CLIGroupMember> member;
    for(GroupLabels::const_iterator it = arg_group_labels_.cbegin(); 
        it != arg_group_labels_.cend(); ++it)
    {
        member = arg_group_.at(*it);
        if(first_iteration)
        {
            usage = (CLI::MakeGeneralUsageString(member->GetProgramName(), indent) + "\n");
            description = member->GetDescription();
        }

        fmt_usage = CLI::MakeArgSpecificUsageString(member->GetProgramName(), max_width_, 
            indent, member->GetCLIArgs());
        usage += (fmt_usage + "\n"); 
        first_iteration = false;
    }
    usage += "\n";

    size_t max_arg_width = 0;
    for(GroupLabels::const_iterator it = arg_group_labels_.cbegin(); 
        it != arg_group_labels_.cend(); ++it)
    {
        member = arg_group_.at(*it);
        const ArgsVec& args = member->GetCLIArgs();
        for (ArgsVec::const_iterator arg_it = args.cbegin(); arg_it != args.cend(); ++arg_it)
        {
            if((*arg_it)->GetPrintName().size() > max_arg_width)
                max_arg_width = (*arg_it)->GetPrintName().size();
        }
    }

    std::string arg_help = "";
    for(GroupLabels::const_iterator it = arg_group_labels_.cbegin(); 
        it != arg_group_labels_.cend(); ++it)
    {
        member = arg_group_.at(*it);
        const ArgsVec& args = member->GetCLIArgs();
        for (ArgsVec::const_iterator arg_it = args.cbegin(); arg_it != args.cend(); ++arg_it)
        {
            arg_help += (CLI::MakeArgHelpString(*arg_it, max_width_, max_arg_width+1) + "\n\n");
        }
    }

    std::string formatted_description = "";
    CLIArg::FormatString(description, max_width_, 0, formatted_description, " ");

    help = usage + formatted_description + "\n\n" + arg_help;
    return help;
}