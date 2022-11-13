#ifndef CLI_ARG_H_
#define CLI_ARG_H_

#include <string>
#include <cstdint>
#include <cstdio>
#include <regex>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include "parse_text.h"
#include "arg_special_config.h"
#include "default_special_config.h"
#include "validate_special_config.h"
#include "container_arg.h"

enum class CLIArgType : uint8_t
{
    BASE = 0,
    POS = 1,
    OPT = 2,
    FLAG = 3
};

using ArgConfVec = std::vector<std::shared_ptr<ArgConfig>>;

// Base class for a CLI command line argument 

class CLIArg : public std::enable_shared_from_this<CLIArg>
{
    protected:

        // Name or label of argument
        std::string label_;

        // Help text associated with argument
        std::string help_str_;

        // Type of CLI argument
        CLIArgType arg_type_;

        // Default value, whether positional, option argument, or flag
        std::string default_;

        // Regex to be used to validate an argument as 
        // as conveyed by the dev using the library to create a CLI.
        std::string config_rgx_;

        // Regex used to validate and select user input configuration
        std::string user_rgx_;

        // Store results of regex matches.
        std::smatch rgx_match_;

        // Minimum user-configurable help message char width
        // static const size_t minimum_help_string_char_width_; 

        // Set to true if argument is present and can be extracted
        // and/or interpreted as expected by parsing the user 
        // string
        bool arg_valid_;

        // Set to true if argument is present as determined by 
        // ValidateUser.
        bool arg_present_;

        // Text parse/manipulation helper
        ParseText pt_;

        // Hold special configuration options for default values
        ArgConfVec default_arg_config_;

        // Hold special configuration options to validate user input values
        ArgConfVec validate_arg_config_;

        // Pointer to ContainerArg base class, if relevant and boolean
        // to indicate if a container argument is used.
        std::shared_ptr<ContainerArg> container_arg_ptr_;
        bool is_container_arg_;

        // Use simple formatting for help string which only prepends
        // a indentation to each line.
        bool use_simple_format_;

    public:
        CLIArg(const std::string& label, const std::string& help_str, 
            const std::string& the_default);
        virtual ~CLIArg() {}
        // static const std::string whitespace_code;
        bool IsValid() const { return arg_valid_; }
        bool IsPresent() const { return arg_present_; }
        CLIArgType ArgType() const { return arg_type_; }
        std::smatch GetSMatch() const { return rgx_match_; }
        virtual std::string GetLabel() const { return label_; }
        virtual std::string GetShortLabel() const { return ""; }
        std::string GetHelpString() const { return help_str_; }

        // Set use_simple_format_ to true.
        std::shared_ptr<CLIArg> SetSimpleHelpFormat()
        { use_simple_format_ = true; return shared_from_this(); }

        /*
        On successful parse completion, if the default value is set
        be used as the output value, instead use the parent path
        of input_path. 

        To be called on the return of the AddOption method. 

        Args:
            input_path      --> String input path

        Return:
            Shared pointer to self
        */
        virtual std::shared_ptr<CLIArg> DefaultUseParentDirOf(const std::string& input_path);



        /*
        On successful parse completion, if the default value is to be used,
        instead use the value of input.

        This is useful if the default value configured for the CLIArg is 
        primarily for help statements and not useful as the actual default
        or a value from another input determined at run-time ought to be 
        used.

        To be called on the return of the AddOption method. 

        Args:
            input       --> Input value to be used

        Return:
            Shared pointer to self
        */
        template<typename T>
        std::shared_ptr<CLIArg> DefaultUseValueOf(const T& input, T& output);
        // template<typename T>
        // std::shared_ptr<CLIArg> DefaultUseValueOf(const T& input);

        virtual std::shared_ptr<CLIArg> DefaultUseValueOf(const std::string& input)
        { 
            printf("CLIArg::DefaultUseValueOf(string): Not overridden for label %s\n", label_.c_str());
            return shared_from_this();
        }
        virtual std::shared_ptr<CLIArg> DefaultUseValueOf(const int& input)
        { 
            printf("CLIArg::DefaultUseValueOf(int): Not overridden for label %s\n", label_.c_str());
            return shared_from_this();
        }
        virtual std::shared_ptr<CLIArg> DefaultUseValueOf(const double& input)
        { 
            printf("CLIArg::DefaultUseValueOf(double): Not overridden for label %s\n", label_.c_str());
            return shared_from_this();
        }
        virtual std::shared_ptr<CLIArg> DefaultUseValueOf(const bool& input)
        { 
            printf("CLIArg::DefaultUseValueOf(bool): Not overridden for label %s\n", label_.c_str());
            return shared_from_this();
        }



        /*
        Compare the user-supplied value against a set of allowed values. 

        To be called on the return of the AddOption method. 

        Args:
            permitted_vals  --> Set of allowed values
            user_val        --> User input value
            default_val     --> Default value to use if user input is not valid

        Return:
            Shared pointer to self
        */
        template <typename T>
        std::shared_ptr<CLIArg> ValidatePermittedValuesAre(const std::set<T>& permitted_vals,
            T& user_val, const T& default_val);

        virtual std::shared_ptr<CLIArg> ValidatePermittedValuesAre(
            const std::set<std::string>& permitted_vals)
        {
            printf("CLIArg::ValidatePermittedValuesAre(string): Not overriden for label %s\n",
                label_.c_str());
            return shared_from_this();
        }
        virtual std::shared_ptr<CLIArg> ValidatePermittedValuesAre(
            const std::set<int>& permitted_vals)
        {
            printf("CLIArg::ValidatePermittedValuesAre(int): Not overriden for label %s\n",
                label_.c_str());
            return shared_from_this();
        }
        virtual std::shared_ptr<CLIArg> ValidatePermittedValuesAre(
            const std::set<double>& permitted_vals)
        {
            printf("CLIArg::ValidatePermittedValuesAre(double): Not overriden for label %s\n",
                label_.c_str());
            return shared_from_this();
        }



        /*
        Ensure that the user-supplied value lies within a range, inclusively.

        To be called on the return of the AddOption method. 

        Args:
            range_low       --> Lowest allowed value
            range_high      --> Highest allowed value
            user_val        --> User input value
            default_val     --> Default value to use if user input is not valid

        Return:
            Shared pointer to self
        */
        template <typename T>
        std::shared_ptr<CLIArg> ValidateInclusiveRangeIs(const T& range_low, const T& range_high,
            T& user_val, const T& default_val);

        virtual std::shared_ptr<CLIArg> ValidateInclusiveRangeIs(int range_low, int range_high)
        { return shared_from_this(); }
        virtual std::shared_ptr<CLIArg> ValidateInclusiveRangeIs(double range_low, double range_high)
        { return shared_from_this(); }
        virtual std::shared_ptr<CLIArg> ValidateInclusiveRangeIs(size_t range_low, size_t range_high)
        { return shared_from_this(); }
     

        ///////////////////////////////////////////////////////////////////////
        //                        Internal functions 
        ///////////////////////////////////////////////////////////////////////

        /*
        Format an input string with newline chars inserted to maintain a 
        maximum character width. Remove newline chars that perscribe
        lines with char counts substantially lower or greater than the maximum.

        Args:
            help_str        --> Input string to be formatted
            max_char_width  --> Maximum character count in a line,
                                not including newline char
            indent          --> Count of whitespace chars to indent
                                each line
            fmt_str         --> Formatted string
            sep             --> String which indicates the separator on which
                                to break lines, i.e., " " (whitespace), "] " 
                                (split on whitespace following ']')

        Return:
            True if no errors occurred; false otherwise.
        */
        static bool FormatString(const std::string& help_str, size_t max_char_width,
            const size_t& indent, std::string& fmt_str, std::string sep);

        /*
        Format input string by padding with indent only. Do not truncate
        lines or otherwise modify new lines.
        */
        static bool FormatString(const std::string& help_str, const size_t& indent,
            std::string& fmt_str);

        /*
        Meta handler for returning appropriately formatted help string. 
        Similar input as FormatString.
        */
        bool GetHelpString(size_t max_char_width, const size_t& indent, 
            std::string& fmt_str);


        
        /*
        Identify and parse a CLI argument from a user input string of 
        arguments.

        Args:
            input       --> User input string
            label       --> Label of option or flag to match 
            parsed      --> Parsed string value of identified user argument.
                            Empty string if argument is not identified.

        Return:
            True if argument is identified and parsed; false otherwise.
        */
        bool IdentifyArgument(const std::string& input, const std::string& label, 
            std::string& parsed);



        /*
        Cast argument to correct output type.

        Args:
            input       --> User input string
            output      --> Casted output value

        Return:
            False if cast fails; true otherwise.
        */
        virtual bool CastArgument(const std::string& input, double& output);
        virtual bool CastArgument(const std::string& input, int& output);
        virtual bool CastArgument(const std::string& input, std::string& output);

        template<typename ArgDataType>
        bool CastArgument(const std::string& input, ArgDataType& output);



        /*
        Parse user input string and set the value of the output.
        Also set arg_present_ to true if argument is present, as 
        determined by ValidateUser(), and arg_valid_ to true if 
        argument can be casted, if applicable. 

        Args:
            input       --> User input string

        Return:
            False if one of the sub-functions fails and which indicates
            that the argument is malformed or can't be casted. Return value
            of false does not necessarily indicate that the arg is not 
            present and should be used to indicate that the host
            program should print help information and exit. Otherwise
            true. 
        */
        virtual bool Parse(const std::string& input)
        { return true; }


        
        /*
        Check if label conforms to standard: 
         - leading double hyphens
         - no quotes or whitespace 
         - no other hyphens
         - underscore ok

        Args:
            label       --> User defined label. Ex: "--data"

        Return:
            True if label conforms, false otherwise.
        */
        virtual bool CheckLabel(const std::string& label);


        /*
        Check if short label conforms to standard: 
         - leading single hyphen
         - no quotes or whitespace
         - single character following hyphen

        Args:
            short_label       --> User defined label. Ex: "-d"

        Return:
            True if label conforms, false otherwise.
        */
        virtual bool CheckShortLabel(const std::string& short_label);



        /*
        Check for a character that is outside the allowed set: alpha-numeric,
        including upper/lower case, decimals and underscore.

        Args:
            c       --> Input char under test

        Return:
            False if char is not in allowed set; true otherwise.
        */
        virtual bool IsAllowedChar(char c);



        /*
        Apply config validation regex as relevant to the CLIArgType


        Return:
            True if relevant data are not discriminated, i.e., 
            the developer-configured input is validated 
        */
        virtual bool ValidateConfig() { return true; }
        virtual bool ValidateConfig(const std::string& label, const std::string& short_label)
        { return true; }



        /*
        Apply user validation regex as relevant to the CLIArgType


        Return:
            True if relevant data are not discriminated, i.e., 
            the user-configured input is validated
        */
        virtual bool ValidateUser(const std::string& user_str) { return true; }



        /*
        Create a nice-looking name based off the label.

        Return a string representing the name of the argument to be used in 
        print statements, etc.
        */
        virtual std::string GetPrintName() { return label_; }



        /*
        Create a string representation for the usage description, i.e.,
        "[--flag]", "[-f]", "[--flag <path>]", "[-f <path>]", "INPUT1"

        Return:
            String
        */
        virtual std::string GetUsageRepr() { return label_; }


        /*
        Get the matched string and following argument (if relevant)
        for printing.

        Return:
            Substring of the user string containing the matched
            portion and following argument (if relevant).
        */ 
        virtual std::string GetMatchString() { std::string temp(""); return temp; }



        /*
        Get a string to use in the help matter which defines the
        default value. Example: "default: 54"

        Return:
            String representation of default label and value.
        */
        virtual std::string GetDefaultString() 
        { 
            std::string def("default: none");
            return def;
        }



        /*
        Iterate over each ArgConfig in default_arg_config_ and call Compute()
        */
        void ComputeDefaultSpecialConfig();



        /*
        Iterate over each ArgConfig in validate_arg_config_ and call Compute()

        Args:
            fail_msg    --> String which identifies the argument in a fail
                            message

        Return:
            False if one of the ArgConfig objects returns false when ArgConfig::Validate()
            function is called; true otherwise.
        */
        bool ComputeValidateSpecialConfig(const std::string& fail_msg);



        /*
        Obtain specially-escaped and un-escaped sections from an input string,
        retaining ordering of sections. 

        Used to allow user input of help strings which have highly formatted
        sections which are not modified by the typical GetHelpString use,
        indicated by special escape sequence, 
        in addition to non-escaped sections which are formatted to indent
        and terminal width. 

        This function is ought to be called by GetHelpString() and is not
        to be used independently. 

        Args:
            input_str       --> Input string to be decomposed by escape
                                sequence
            nonescaped      --> map<int, std::string> which maps order of 
                                nonescaped string section in original 
                                string to section value
            escaped         --> map<int, std::string> which maps order of 
                                escaped string section in original 
                                string to section value

        Return:
            True if escape chars are present and occur in pairs and
            output params nonescaped and escaped are populated correctly;
            false otherwise.
        */
        bool FindEscapedComponents(const std::string& input_str, 
            std::map<int, std::string>& nonescaped, std::map<int, std::string>& escaped);

};

template<typename ArgDataType>
bool CLIArg::CastArgument(const std::string& input, ArgDataType& output)
{
    printf("CLIArg::CastArgument not defined.\n");
    return false;
}

template<typename T>
std::shared_ptr<CLIArg> CLIArg::DefaultUseValueOf(const T& input, T& output)
{
    std::shared_ptr<ArgUseValueConfig<T>> conf = std::make_shared<ArgUseValueConfig<T>>();
    conf->Set(&input, &output);
    default_arg_config_.push_back(conf);
    return shared_from_this();
}

// template<typename T>
// std::shared_ptr<CLIArg> CLIArg::DefaultUseValueOf(const T& input)
// { 
//     printf("CLIArg::DefaultUseValueOf: Using template function, not overridden for label %s\n", label_.c_str());
//     return shared_from_this();
// }

template <typename T>
std::shared_ptr<CLIArg> CLIArg::ValidatePermittedValuesAre(const std::set<T>& permitted_vals,
    T& user_val, const T& default_val)
{
    std::shared_ptr<ArgPermittedValuesConfig<T>> conf = 
        std::make_shared<ArgPermittedValuesConfig<T>>(permitted_vals);
    conf->Set(&default_val, &user_val);
    validate_arg_config_.push_back(conf);
    return shared_from_this();
}

template <typename T>
std::shared_ptr<CLIArg> CLIArg::ValidateInclusiveRangeIs(const T& range_low, const T& range_high,
    T& user_val, const T& default_val)
{
    std::shared_ptr<ArgPermittedInclusiveRangeConfig<T>> conf = 
        std::make_shared<ArgPermittedInclusiveRangeConfig<T>>(range_low, range_high);
    conf->Set(&default_val, &user_val);
    validate_arg_config_.push_back(conf);
    return shared_from_this();
}

#endif  // CLI_ARG_H_
