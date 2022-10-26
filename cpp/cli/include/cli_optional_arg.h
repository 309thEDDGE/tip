#ifndef CLI_OPTIONAL_ARG_H_
#define CLI_OPTIONAL_ARG_H_

#include <string>
#include <memory>
#include <set>
#include <vector>
#include <map>
#include <type_traits>
#include <iostream>
#include <sstream>
#include "parse_text.h"
#include "arg_special_config.h"
#include "cli_arg.h"
#include "default_special_config.h"
#include "validate_special_config.h"
#include "container_arg.h"

class CLIOptionalArgBase : public CLIArg
{
    protected:

        // short label
        std::string short_label_;

        // Regex for configuration and user inputs
        std::string label_config_rgx_;
        std::string short_label_config_rgx_;
        std::string label_user_rgx_;
        std::string short_label_user_rgx_;

        // Matched label/short label and following string
        std::string matched_str_;

    public:
        CLIOptionalArgBase(std::string label, std::string short_label, const std::string& help_str) :
            CLIArg(label, help_str, ""), short_label_(short_label), matched_str_(""), 
            label_config_rgx_("^(--[a-zA-Z0-9/_]+)$"), 
            short_label_config_rgx_("^(-[a-zA-Z0-9]{1})$"),
            // label_user_rgx_("(^|.*\\s)" + label + "\\s([\\w:.\\\\/-]+)(\\s.*|$)"),
            label_user_rgx_("(^|.*\\s)" + label + "\\s([\\w:.\\\\/\\-\\(\\)\\$\\#]+)"),
            short_label_user_rgx_("(^|\\s)" + short_label + "\\s([\\w:.\\\\/\\-\\(\\)\\$\\#]+)")
            // short_label_user_rgx_("(^|.*\\s)" + short_label + "\\s([\\w:.\\\\/-]+)(\\s.*|$)")
        {
            arg_type_ = CLIArgType::OPT;
        }
        ~CLIOptionalArgBase() {}
        virtual std::string GetShortLabel() const;
        // virtual bool Parse(const std::string& input);
        virtual bool ValidateConfig(); 
        virtual bool ValidateConfig(const std::string& label, const std::string& short_label);
        virtual bool ValidateUser(const std::string& user_str);
        virtual std::string GetPrintName();
        virtual std::string GetMatchString();
        virtual std::string GetUsageRepr(); 

};

inline std::string CLIOptionalArgBase::GetShortLabel() const
{ return short_label_; }

inline bool CLIOptionalArgBase::ValidateConfig()
{
    return ValidateConfig(label_, short_label_);
}

inline bool CLIOptionalArgBase::ValidateConfig(const std::string& label, 
    const std::string& short_label)
{
    bool one_valid_label = false;
    if(label.size() != 0)
    {
        std::regex rgx(label_config_rgx_);
        if(!std::regex_match(label, rgx))
            return false;
        one_valid_label = true;
    }

    if(short_label.size() != 0)
    {
        std::regex rgx2(short_label_config_rgx_);
        if(!std::regex_match(short_label, rgx2))
            return false;
        one_valid_label = true;
    }

    if(one_valid_label)
        return true;
    else
        return false;
}

inline bool CLIOptionalArgBase::ValidateUser(const std::string& user_str)
{
    bool found_match = false;
    std::smatch match1, match2;
    if(label_.size() != 0)
    {
        std::regex rgx(label_user_rgx_);
        if(std::regex_search(user_str, match1, rgx))
        {
            // printf("found label match\n");
            found_match = true;
            matched_str_ = label_ + " ";
        }
    }

    if(short_label_.size() != 0)
    {
        std::regex rgx2(short_label_user_rgx_);
        if(std::regex_search(user_str, match2, rgx2))
        {
            // printf("found short label match\n");
            found_match = true;
            matched_str_ = short_label_ + " ";
        }
    }

    if(found_match)
    {
        if(match1.ready() && (match1.size() == 3) && match2.ready() && (match2.size() == 3))
        {
            // printf("found both!, match1 (%zu) \"%s\", match2 (%zu) \"%s\"\n", match1.size(), 
            //     match1[2].str().c_str(), match2.size(), match2[2].str().c_str());
            if(match1.position(2) < match2.position(1))
                rgx_match_ = match1;
            else
                rgx_match_ = match2;
        }
        else if(match1.ready() && match1.size() == 3)
        {
            // printf("match1 %s\"\n\"", match1[2].str().c_str());
            rgx_match_ = match1;
        }
        else
        {
            // printf("match2 \"%s\"\n", match2[2].str().c_str());
            rgx_match_ = match2;
        }

        return true;
    }

    return false;
}

inline std::string CLIOptionalArgBase::GetPrintName()
{
    std::string name = ""; 
    if(label_.size() != 0)
    {
        name = label_;
        if(short_label_.size() != 0)
            name += (", " + short_label_);
    }
    else
    {
        name = short_label_;
    }    
    return name; 
}

inline std::string CLIOptionalArgBase::GetMatchString()
{
    return matched_str_;
}

inline std::string CLIOptionalArgBase::GetUsageRepr()
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

    repr += " <value>]";
    return repr;
}

/////////////////////////////////////////////////////////////////////
//                      general 
/////////////////////////////////////////////////////////////////////

template<typename ArgDataType, typename ArgDataType2 = void>
class CLIOptionalArg : public CLIOptionalArgBase
{
    public:
        CLIOptionalArg(std::string label, std::string short_label, const std::string& help_str, 
            ArgDataType the_default, ArgDataType& output);
        ~CLIOptionalArg() {}
};

/////////////////////////////////////////////////////////////////////
//                      vector 
/////////////////////////////////////////////////////////////////////

template<typename ArgDataType>
class CLIOptionalArg<std::vector<ArgDataType>, void> : public CLIOptionalArgBase
{
    protected:
        std::vector<ArgDataType>& parsed_value_;
        std::vector<ArgDataType> default_value_;
        ArgDataType temp_parsed_value_;

    public:
        CLIOptionalArg(std::string label, std::string short_label, const std::string& help_str, 
            const std::vector<ArgDataType>& the_default, std::vector<ArgDataType>& output);
        ~CLIOptionalArg() {}
        virtual bool Parse(const std::string& input);
        virtual std::string GetDefaultString();

        /*
        Create a shared pointer to the base class, CLIArg, of
        a new instance of this class.
        */
        static std::shared_ptr<CLIArg> Make(const std::string& label, 
            std::string short_label, const std::string& help_str, 
            const std::vector<ArgDataType>& the_default, std::vector<ArgDataType>& output);
};

template<typename ArgDataType>
CLIOptionalArg<std::vector<ArgDataType>>::CLIOptionalArg(std::string label, std::string short_label,
    const std::string& help_str, const std::vector<ArgDataType>& the_default, 
    std::vector<ArgDataType>& output) : 
    CLIOptionalArgBase(label, short_label, help_str), parsed_value_(output), 
    default_value_(the_default), temp_parsed_value_{}
{ 
    this->is_container_arg_ = true;
    this->container_arg_ptr_ = MakeContainerArg(parsed_value_, temp_parsed_value_);
}

template<typename ArgDataType>
std::shared_ptr<CLIArg> CLIOptionalArg<std::vector<ArgDataType>>::Make(const std::string& label, 
    std::string short_label, const std::string& help_str, 
    const std::vector<ArgDataType>& the_default, std::vector<ArgDataType>& output)
{
    std::shared_ptr<CLIArg> sptr = std::make_shared<CLIOptionalArg<std::vector<ArgDataType>>>(
        label, short_label, help_str, the_default, output);
    return sptr;
}

template<typename ArgDataType>
bool CLIOptionalArg<std::vector<ArgDataType>>::Parse(const std::string& input)
{
    std::string input_str = input;
    std::string fail_msg("Optional argument " + GetPrintName());
    std::string arg_raw_value = "";
    while(input_str.size() > 0)
    {
        // printf("input_str: %s\n", input_str.c_str());
        if(!ValidateUser(input_str))
        {
            // printf("Input \"%s\" contains character(s) which are not valid for "
            //     "optional argument %s\n", input.c_str(), GetUsageRepr().c_str());
            ComputeDefaultSpecialConfig();
            parsed_value_ = default_value_;
            return true;
        }
        arg_present_ = true; 

        arg_raw_value = GetSMatch()[2].str();
        // printf("Got match: %s\n", arg_raw_value.c_str());
        if(!(this->container_arg_ptr_->Parse(arg_raw_value)))
        {
            ComputeDefaultSpecialConfig();
            parsed_value_ = default_value_;
            return false;
        }

        if(!ComputeValidateSpecialConfig(fail_msg))
        {
            return false;
        }

        input_str = GetSMatch().suffix().str();
        // printf("later input_str: %s\n", input_str.c_str());
    }
    arg_valid_ = true;
    return true;
} 

template<typename ArgDataType>
std::string CLIOptionalArg<std::vector<ArgDataType>>::GetDefaultString()
{
    std::stringstream def;
    def << "default: [";
    if (default_value_.size() > 0)
    {
        def << default_value_.at(0); 
        for(size_t i = 1; i < default_value_.size(); i++)
        {
            def << ", " << default_value_.at(i);
        }
    }
    def << "]";
    return def.str();
}

/////////////////////////////////////////////////////////////////////
//                      map 
/////////////////////////////////////////////////////////////////////

template<typename ArgDataType, typename ArgDataType2>
class CLIOptionalArg<std::map<ArgDataType, ArgDataType2>> : public CLIOptionalArgBase
{
    protected: 
        std::map<ArgDataType, ArgDataType2>& parsed_value_;
        std::map<ArgDataType, ArgDataType2> default_value_;
        ArgDataType2 temp_parsed_value_;
        ArgDataType temp_parsed_key_;

    public:
        CLIOptionalArg(std::string label, std::string short_label, const std::string& help_str, 
            const std::map<ArgDataType, ArgDataType2>& the_default, 
            std::map<ArgDataType, ArgDataType2>& output);
        ~CLIOptionalArg() {}
        virtual bool Parse(const std::string& input);
        virtual std::string GetDefaultString();

        /*
        Create a shared pointer to the base class, CLIArg, of
        a new instance of this class.
        */
        static std::shared_ptr<CLIArg> Make(const std::string& label, 
            std::string short_label, const std::string& help_str, 
            const std::map<ArgDataType, ArgDataType2>& the_default, 
            std::map<ArgDataType, ArgDataType2>& output);

};

template<typename ArgDataType, typename ArgDataType2>
CLIOptionalArg<std::map<ArgDataType, ArgDataType2>>::CLIOptionalArg(std::string label, 
    std::string short_label, const std::string& help_str, 
    const std::map<ArgDataType, ArgDataType2>& the_default, 
    std::map<ArgDataType, ArgDataType2>& output) : 
    CLIOptionalArgBase(label, short_label, help_str), parsed_value_(output), 
    default_value_(the_default), temp_parsed_value_{}, temp_parsed_key_{}

{
    this->is_container_arg_ = true;
    this->container_arg_ptr_ = MakeContainerArg(parsed_value_, temp_parsed_key_, 
        temp_parsed_value_);
}

template<typename ArgDataType, typename ArgDataType2>
bool CLIOptionalArg<std::map<ArgDataType, ArgDataType2>>::Parse(const std::string& input) 
{
    std::string input_str = input;
    std::string fail_msg("Optional argument " + GetPrintName());
    std::string arg_raw_value = "";
    while(input_str.size() > 0)
    {
        // printf("input_str: %s\n", input_str.c_str());
        if(!ValidateUser(input_str))
        {
            // printf("Input \"%s\" contains character(s) which are not valid for "
            //     "optional argument %s\n", input.c_str(), GetUsageRepr().c_str());
            ComputeDefaultSpecialConfig();
            parsed_value_ = default_value_;
            return true;
        }
        arg_present_ = true; 

        arg_raw_value = GetSMatch()[2].str();
        // printf("Got match: %s\n", arg_raw_value.c_str());
        if(!(this->container_arg_ptr_->Parse(arg_raw_value)))
        {
            ComputeDefaultSpecialConfig();
            parsed_value_ = default_value_;
            return false;
        }

        if(!ComputeValidateSpecialConfig(fail_msg))
        {
            return false;
        }

        input_str = GetSMatch().suffix().str();
        // printf("later input_str: %s\n", input_str.c_str());
    }
    arg_valid_ = true;
    return true;
}

template<typename ArgDataType, typename ArgDataType2>
std::string CLIOptionalArg<std::map<ArgDataType, ArgDataType2>>::GetDefaultString() 
{
    std::stringstream def;
    def << "default: [";
    if (default_value_.size() > 0)
    {
        typename std::map<ArgDataType, ArgDataType2>::const_iterator it = default_value_.cbegin();
        size_t ind = 0;
        for(it = default_value_.cbegin(); it != default_value_.cend(); ++it)
        {
            if(ind == 0)
                def << "(" << it->first << ", " << it->second << ")"; 
            else
                def << ", (" << it->first << ", " << it->second << ")";
            ind++;
        }
    }
    def << "]";
    return def.str();
}

template<typename ArgDataType, typename ArgDataType2>
std::shared_ptr<CLIArg> CLIOptionalArg<std::map<ArgDataType, ArgDataType2>>::Make(
    const std::string& label, std::string short_label, const std::string& help_str, 
    const std::map<ArgDataType, ArgDataType2>& the_default, std::map<ArgDataType, ArgDataType2>& output)
{
    std::shared_ptr<CLIArg> sptr = std::make_shared<CLIOptionalArg<std::map<ArgDataType, ArgDataType2>>>(
        label, short_label, help_str, the_default, output);
    return sptr;
}

/////////////////////////////////////////////////////////////////////
//                      scalar 
/////////////////////////////////////////////////////////////////////

template<typename ArgDataType>
class CLIOptionalArg<ArgDataType, void> : public CLIOptionalArgBase
{
    protected:
        // Parsed output variable
        ArgDataType& parsed_value_;

        // Default value
        ArgDataType default_value_;

    public:
        CLIOptionalArg(std::string label, std::string short_label, const std::string& help_str, 
            ArgDataType the_default, ArgDataType& output);
        ~CLIOptionalArg() {}
        virtual bool Parse(const std::string& input);
        virtual std::string GetDefaultString();
        virtual std::shared_ptr<CLIArg> DefaultUseValueOf(const ArgDataType& input);
        virtual std::shared_ptr<CLIArg> ValidatePermittedValuesAre(
            const std::set<ArgDataType>& permitted_vals);  
        virtual std::shared_ptr<CLIArg> ValidateInclusiveRangeIs(ArgDataType range_low, 
            ArgDataType range_high);
        virtual std::shared_ptr<CLIArg> DefaultUseParentDirOf(const std::string& input_path);

        /*
        Create a shared pointer to the base class, CLIArg, of
        a new instance of this class.
        */
        static std::shared_ptr<CLIArg> Make(const std::string& label, 
            std::string short_label, const std::string& help_str, 
            ArgDataType the_default, ArgDataType& output);

};


template<typename ArgDataType>
CLIOptionalArg<ArgDataType>::CLIOptionalArg(std::string label, std::string short_label,
    const std::string& help_str, ArgDataType the_default, ArgDataType& output) : 
    CLIOptionalArgBase(label, short_label, help_str), parsed_value_(output), 
    default_value_(the_default)
{ }

template<typename ArgDataType>
bool CLIOptionalArg<ArgDataType>::Parse(const std::string& input)
{
    if(!ValidateUser(input))
    {
        // printf("Input \"%s\" contains character(s) which are not valid for "
        //     "optional argument %s\n", input.c_str(), GetUsageRepr().c_str());
        ComputeDefaultSpecialConfig();
        parsed_value_ = default_value_;
        return true;
    }
    arg_present_ = true; 

    if(!CastArgument(GetSMatch()[2].str(), parsed_value_))
    {
        matched_str_ += GetSMatch()[2].str();
        printf("Value \"%s\" can't be casted for optional argument %s\n",
            matched_str_.c_str(), GetUsageRepr().c_str());
        ComputeDefaultSpecialConfig();
        parsed_value_ = default_value_;
        return false;
    }

    std::string fail_msg("Optional argument " + GetPrintName());
    if(!ComputeValidateSpecialConfig(fail_msg))
    {
        return false;
    }

    arg_valid_ = true;
    return true;
}

template<>
inline bool CLIOptionalArg<std::string>::Parse(const std::string& input)
{
    if(!ValidateUser(input))
    {
        // printf("Input \"%s\" contains character(s) which are not valid for "
        //     "optional argument %s\n", input.c_str(), GetUsageRepr().c_str());
        ComputeDefaultSpecialConfig();
        parsed_value_ = default_value_;
        return true;
    }
    arg_present_ = true; 

    if(!CastArgument(GetSMatch()[2].str(), parsed_value_))
    {
        matched_str_ += GetSMatch()[2].str();
        printf("Value \"%s\" can't be casted for optional argument %s\n",
            matched_str_.c_str(), GetUsageRepr().c_str());
        ComputeDefaultSpecialConfig();
        parsed_value_ = default_value_;
        return false;
    }

    std::string fail_msg("Optional argument " + GetPrintName());
    if(!ComputeValidateSpecialConfig(fail_msg))
    {
        return false;
    }

    if(parsed_value_.find(CLIArg::whitespace_code) != std::string::npos)
    {
        ParseText pt;
        parsed_value_ = pt.Replace(parsed_value_, CLIArg::whitespace_code, " ");
    }

    arg_valid_ = true;
    return true;
}

template<typename ArgDataType>
std::shared_ptr<CLIArg> CLIOptionalArg<ArgDataType>::Make(const std::string& label, 
    std::string short_label, const std::string& help_str, 
    ArgDataType the_default, ArgDataType& output)
{
    std::shared_ptr<CLIArg> sptr = std::make_shared<CLIOptionalArg<ArgDataType>>(
        label, short_label, help_str, the_default, output);
    return sptr;
}

template<typename ArgDataType>
std::string CLIOptionalArg<ArgDataType>::GetDefaultString()
{
    std::string def = "default: ";
    def += std::to_string(default_value_);
    return def;
}

template<>
inline std::string CLIOptionalArg<std::string>::GetDefaultString()
{
    std::string def = "default: ";
    def += default_value_;
    return def;
}

template<>
inline std::string CLIOptionalArg<bool>::GetDefaultString()
{
    std::string def = "default: ";
    if(default_value_)
        def += "true";
    else
        def += "false";
    return def;
}

template <typename ArgDataType>
std::shared_ptr<CLIArg> CLIOptionalArg<ArgDataType>::DefaultUseParentDirOf(
    const std::string& input_path)
{
    printf("CLIPositionalArg::DefaultUseParentDirOf: Not defined for non-string template args!!\n"); 
    return shared_from_this();
}

template <>
inline std::shared_ptr<CLIArg> CLIOptionalArg<std::string>::DefaultUseParentDirOf(
    const std::string& input_path)
{
    std::shared_ptr<ArgParentPathConfig> conf = std::make_shared<ArgParentPathConfig>();
    conf->Set(&input_path, &default_value_);
    default_arg_config_.push_back(conf);
    return shared_from_this();
}

template <typename ArgDataType>
std::shared_ptr<CLIArg> CLIOptionalArg<ArgDataType>::DefaultUseValueOf(const ArgDataType& input)
{
    return CLIArg::DefaultUseValueOf(input, default_value_);
}

template <typename ArgDataType>
std::shared_ptr<CLIArg> CLIOptionalArg<ArgDataType>::ValidatePermittedValuesAre(
    const std::set<ArgDataType>& permitted_vals)
{
    return CLIArg::ValidatePermittedValuesAre(permitted_vals, parsed_value_, default_value_);
}

template<typename ArgDataType>
std::shared_ptr<CLIArg> CLIOptionalArg<ArgDataType>::ValidateInclusiveRangeIs(
    ArgDataType range_low, ArgDataType range_high)
{
    return CLIArg::ValidateInclusiveRangeIs(range_low, range_high, parsed_value_,
        default_value_);
}

#endif  // CLI_OPTIONAL_ARG_H_
