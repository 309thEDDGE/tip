#ifndef CLI_GROUP_H_
#define CLI_GROUP_H_

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <cstdio>
#include "cli_arg.h"
#include "cli.h"
#include "terminal.h"

class GroupIndicator
{
    private:
        // label of CLIArg which this indicator represents
        std::string label_;

        // True if this CLIArg is required to be valid if the 
        // group is to be represented as the active group.
        bool is_indicator_;

    public:
        GroupIndicator() : label_(""), is_indicator_(false) {}
        GroupIndicator(const std::string& label) : label_(label), is_indicator_(false)
        {}
        void SetGroupIndicator() { is_indicator_ = true; }
        bool IsGroupIndicator() { return is_indicator_; }
};

class CLIGroupMember : public CLI
{
    private:
        // Map label/flag to GroupIndicator
        std::map<std::string, GroupIndicator> arg_to_indicator_map_;

    public:
        CLIGroupMember(const std::string& prog_name, const std::string& description,
            size_t terminal_width) : CLI(prog_name, description, terminal_width)
        {}

        bool IsGroupIndicator(const std::string& label) 
        {
            if(arg_to_indicator_map_.count(label) != 0)
                return arg_to_indicator_map_.at(label).IsGroupIndicator();
            return 
                false;
        }

        template <typename ArgDataType>
        std::shared_ptr<CLIArg> AddOption(std::string label, std::string help_str, 
            ArgDataType& output_value, bool is_indicator=false);

        template <typename ArgDataType>
        std::shared_ptr<CLIArg> AddOption(std::string flag, std::string short_flag, std::string help_str, 
             ArgDataType the_default, ArgDataType& output, bool is_indicator=false);
};

template <typename ArgDataType>
std::shared_ptr<CLIArg> CLIGroupMember::AddOption(std::string label, std::string help_str, 
    ArgDataType& output_value, bool is_indicator)
{
    std::shared_ptr<CLIArg> sptr = CLIPositionalArg<ArgDataType>::Make(label, help_str, output_value);
    GroupIndicator gi(label);
    if(is_indicator) { gi.SetGroupIndicator(); }
    arg_to_indicator_map_[label] = gi;
    args_.push_back(sptr);
    return sptr;
}

template <typename ArgDataType>
std::shared_ptr<CLIArg> CLIGroupMember::AddOption(std::string flag, std::string short_flag, std::string help_str, 
        ArgDataType the_default, ArgDataType& output, bool is_indicator)
{
    std::shared_ptr<CLIArg> sptr = CLIOptionalArg<ArgDataType>::Make(
        flag, short_flag, help_str, the_default, output);
    args_.push_back(sptr);
    GroupIndicator gi(flag);
    if(is_indicator) { gi.SetGroupIndicator(); }
    arg_to_indicator_map_[flag] = gi;
    return sptr;
}

template <>
inline std::shared_ptr<CLIArg> CLIGroupMember::AddOption<bool>(std::string flag, std::string short_flag, std::string help_str, 
        bool the_default, bool& output, bool is_indicator)
{
    std::shared_ptr<CLIArg> sptr = CLIFlag::Make(flag, short_flag, help_str, the_default, output);
    args_.push_back(sptr);
    GroupIndicator gi(flag);
    if(is_indicator) { gi.SetGroupIndicator(); }
    arg_to_indicator_map_[flag] = gi;
    return sptr;
}

using GroupLabels = std::vector<std::string>;
using GroupMap = std::map<std::string, std::shared_ptr<CLIGroupMember>>;

class CLIGroup
{
    private:
        GroupLabels arg_group_labels_;
        GroupMap arg_group_;

        bool is_configured_;

        // Terminal/console (stdout) width in character unit
        size_t max_width_;

    public:
        CLIGroup();

        const GroupMap& GetGroupMap() { return arg_group_; }
        size_t GetTerminalWidth() const { return max_width_; } 

        /*
        Add argument CLI in order of precedence. Return a CLIGroupMember, which
        is small overload of CLI, to which options can be added.

        Args:
            prog_name   --> binary name or similar
            description --> description summary
            nickname    --> Used to identify the particular CLI, such as
                            "help", "version", "full", etc. 

        Return:
            False if configure step fails; true otherwise.
        */
        std::shared_ptr<CLIGroupMember> AddCLI(const std::string& prog_name, const std::string& description,
            std::string nickname);

        

        /*
        Similar to CLI::CheckConfiguration applied to all add CLIs
        */
        bool CheckConfiguration();



        /*
        See CLI::Parse.

        Args:
            argc        --> arg count
            argv        --> pointer to array of args
            nickname    --> Nickname of first valid CLIGroupMember
            member      --> Pointer to valid CLIGroupMember

        Return:
            True if at least one of the CLIGroupMembers added via AddCLI is valid,
            false otherwise. A return value of false indicates that a user input
            is not valid according to the defined CLIGroupMembers, that help matter
            should be printed and the program exit.

            A CLIGroupMember is valid if all of the CLIArgs within the member that
            are marked as group indicators (GroupIndicator::SetGroupIndicator()) 
            have the property (IsValid() && IsPresent()).
        */ 
        bool Parse(int argc, char* argv[], std::string& nickname, 
            std::shared_ptr<CLIGroupMember>& member);

        

        /*
        Make string representing help matter for all configured CLIGroupMembers

        Return:
            String containing help matter. 
        */
        std::string MakeHelpString();


};

#endif  // CLI_GROUP_H_