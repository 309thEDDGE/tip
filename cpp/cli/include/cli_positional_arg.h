#ifndef CLI_POSITIONAL_ARG_H_
#define CLI_POSITIONAL_ARG_H_

#include <string>
#include <memory>
#include "cli_arg.h"

template<typename ArgDataType>
class CLIPositionalArg : public CLIArg
{
    private:

        // Parsed output variable
        ArgDataType& parsed_value_;

    public:
        CLIPositionalArg(std::string label, std::string help_str, 
            ArgDataType& output);
        ~CLIPositionalArg();

        virtual bool Parse(const std::string& input);
        virtual bool ValidateUser(const std::string& user_str);
        virtual std::string GetUsageRepr(); 
        virtual std::string GetPrintName();
        virtual std::shared_ptr<CLIArg> DefaultUseParentDirOf(const std::string& input_path);

        /*
        Create a shared pointer to the base class, CLIArg, of
        a new instance of this class.
        */
        static std::shared_ptr<CLIArg> Make(const std::string& label, 
            const std::string& help_str, ArgDataType& output);
};

template<typename ArgDataType>
CLIPositionalArg<ArgDataType>::CLIPositionalArg(std::string label, std::string help_str, 
    ArgDataType& output) : 
    CLIArg(label, help_str, ""), parsed_value_(output)
{ 
    arg_type_ = CLIArgType::POS;
    user_rgx_ = "^([a-zA-Z0-9\\\\\\/_:\\. \\-\\(\\)\\$\\#]+)$";
}

template<typename ArgDataType>
CLIPositionalArg<ArgDataType>::~CLIPositionalArg()
{ }

template<typename ArgDataType>
bool CLIPositionalArg<ArgDataType>::Parse(const std::string& input)
{
    if(!ValidateUser(input))
    {
        printf("Input \"%s\" contains character(s) which are not valid for "
            "positional argument %s\n", input.c_str(), GetUsageRepr().c_str());
        return false;
    }
    arg_present_ = true;

    std::string output = rgx_match_[1].str();
    if(!CastArgument(output, parsed_value_))
    {
        printf("Input \"%s\" can't be casted for positional argument %s\n",
            input.c_str(), GetUsageRepr().c_str());
        return false;
    }

    // Positional args must always be present, so if the 
    // indicated position is not a label-type and
    // it can be casted correctly, assume it is 
    // valid. 
    arg_valid_ = true;
    return true;
}

template<typename ArgDataType>
bool CLIPositionalArg<ArgDataType>::ValidateUser(const std::string& user_str)
{
    std::regex rgx(user_rgx_);
    if(!std::regex_match(user_str, rgx_match_, rgx))
        return false;
    return true;
}

template<typename ArgDataType>
std::shared_ptr<CLIArg> CLIPositionalArg<ArgDataType>::Make(const std::string& label, 
    const std::string& help_str, ArgDataType& output)
{
    std::shared_ptr<CLIArg> sptr = std::make_shared<CLIPositionalArg<ArgDataType>>(
        label, help_str, output);
    return sptr;
}

template<typename ArgDataType>
std::string CLIPositionalArg<ArgDataType>::GetUsageRepr()
{
    return pt_.ToUpper(label_);
}

template<typename ArgDataType>
std::string CLIPositionalArg<ArgDataType>::GetPrintName()
{
    return GetUsageRepr();
}

template <typename ArgDataType>
std::shared_ptr<CLIArg> CLIPositionalArg<ArgDataType>::DefaultUseParentDirOf(
    const std::string& input_path)
{
    printf("CLIOptionalArg::DefaultUseParentDirOf: Not defined for positional args!\n"); 
    return shared_from_this();
}
#endif  // CLI_POSITIONAL_ARG_H_