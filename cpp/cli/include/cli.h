#ifndef CLI_H_
#define CLI_H_

#include <string>
#include <memory>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <algorithm>
#include "sysexits.h"
#include "parse_text.h"
#include "cli_conf.h"
#include "cli_arg.h"
#include "cli_positional_arg.h"
#include "cli_optional_arg.h"
#include "cli_flag.h"
#include "terminal.h"

/*
Notes:

- It may be necessary to ensure unique CLIArg labels, short_labels
and/or combination of label + short_label. This is not handled and 
the result of a user defining multiple args with the same label
would likely have unintended results. 

-A secondary concern
is non-unique labels in a group of CLIArgs as exists in CLIGroup.
In that case, the selected/active CLI will not be chosen correctly
and help matter print both args and the accompanying help matter, 
which would further serve to confuse a user. This could be solved
by ensuring arg label (short_label, combination, etc.) uniqueness
among all CLIArgs in a GLIGroup.
*/


using ArgsVec = std::vector<std::shared_ptr<CLIArg>>;

class CLI
{
    protected:

        // Configured argument definitions
        ArgsVec args_;

        // Program name to be used in the help string:
        // "Usage: <program name> [options  . . .]"
        std::string program_name_;

        // CLIGroup high-level description, not relevant
        // to a specific argument ("What does this app do?")
        std::string description_;

        // Count of positional arguments as configured. Determined
        // by SortArgs
        size_t pos_arg_count_;

        // Terminal/console (stdout) width in character unit
        size_t max_width_;

    public:

        CLI(const std::string& prog_name, const std::string& description);
        CLI(const std::string& prog_name, const std::string& description, size_t max_width);
        std::string GetProgramName() { return program_name_; }
        size_t GetArgCount() { return args_.size(); }
        const CLIArg* GetArg(int index)
        {
            if(index < args_.size())
            {
                return args_.at(index).get();
            }
            return nullptr;
        }
        const ArgsVec& GetCLIArgs() { return args_; }
        std::string GetDescription() const { return description_; }

        /*
        Add positional argument. 

        Regardless of when this call is made, positional arguments
        may only exist as the last n arguments to the CLIGroup.

        The order of all calls to this function will determine the 
        order of positional arguments.

        All positional arguments must be present in the user input.

        Args:
            label           --> Name of this argument for reference
                                in print statements
            help_str        --> Help string
            default_value   --> Value to be retrieved if the positional
                                argument is not present
            output_value    --> Value of the positional argument as 
                                interpreted from the user input, else
                                the default_value

        Return:
            Pointer to CLIArg that is the option created and added.
        */
        template <typename ArgDataType>
        std::shared_ptr<CLIArg> AddOption(std::string label, std::string help_str, 
            ArgDataType& output_value);


        
        /*
        Add optional argument.

        Order of creation of optional arguments is irrelevant and may include
        a short_flag in addition to a standard "long" flag. 

        User input must place any optional arguments prior to positional
        arguments, if present.

        Args:
            flag            --> Exact flag which shall indicate the presence
                                of a value immediately following. Must be of form
                                "--data", two hyphens followed by alpha-numeric, 
                                including underscore.            
            short_flag      --> Alternate indicator following form "-d", single
                                hyphen followed by single alphanumeric or underscore
            help_str        --> Help string
            default_value   --> Value to be retrieved if the optional
                                argument is not present
            output_value    --> Value of the optional argument as 
                                interpreted from the user input, else
                                the default_value

        Return:
            Pointer to CLIArg that is the option created and added.
        */
        template <typename ArgDataType>
        std::shared_ptr<CLIArg> AddOption(std::string flag, std::string short_flag, std::string help_str, 
             ArgDataType the_default, ArgDataType& output);

        template <typename ArgDataType>
        std::shared_ptr<CLIArg> AddOption(std::string flag, std::string short_flag, std::string help_str, 
             const std::vector<ArgDataType>& the_default, std::vector<ArgDataType>& output);


        /*
        Add optional flag (with no following argument).

        Args:
            flag            --> Exact flag which shall indicate that the non-default
                                value shall be set as the output. Must be of form
                                "--data", two hyphens followed by alpha-numeric, 
                                including underscore.            
            short_flag      --> Alternate indicator following form "-d", single
                                hyphen followed by single alphanumeric or underscore
            help_str        --> Help string
            default_value   --> Value to be retrieved if the flag is not present
            output_value    --> Value of the default argument if the flag or short_flag
                                is not present, otherwise NOT default
        */
        // User specialization of AddOption
        // void AddOption(std::string flag, std::string short_flag, std::string help_str, 
        //     bool the_default, bool& output);


        
        /*
        Check each of the configured arguments for errors. The program should 
        be halted if this returns false.

        Return:
            False if at least argument has an error; true otherwise.
        */
        bool CheckConfiguration();



        /*
        Parse the configured arguments and the user input string.

        Args:
            argc        --> arg count
            argv        --> pointer to array of args

        Return:
            False if one of the configured CLI arguments is malformed
            or the user input contains values which can't be parsed
            as intended; true otherwise. 
        */
        bool Parse(int argc, char* argv[]);

        // Version which allows separate consideration for ValidateUserInput
        // Args:
        //     pos_args     --> Vector of positional argument candidates, as
        //                      as returned from ValidateUserInput
        bool Parse(int argc, char* argv[], const std::vector<std::string>& pos_args);



        ///////////////////////////////////////////////////////////////////////
        //                        Internal functions 
        ///////////////////////////////////////////////////////////////////////

        /*
        Sort created arguments such that positional arguments occupy the highest
        indicies.

        Args:
            args_vec        --> Vector to CLIArg 
            pos_arg_count   --> Count of positional arguments, as output parameter

        Return:
            Sorted vector of pointer to CLIArg.
        */
        static ArgsVec SortArgs(const ArgsVec& args_vec, size_t& pos_arg_count);



        /*
        Build a map to use a lookup table for parsing of positional arguments.

        Map optional args and flags (strings) to count of variables that ought
        to follow if used correctly: 1 for optional args, 0 for flags.

        Args:
            args    --> ArgsVec of all configured CLIArgs

        Return:
            Map of label or short_label to expected count of the following
            arguments.
        */
        static std::unordered_map<std::string, int> MakeLabelLookupMap(const ArgsVec& args);



        /*
        Convert argc, argv into vector of user input.

        Args:
            argc        --> arg count
            argv        --> arg values, array of char*

        Return:
            Vector of user arguments.
        */
        static std::vector<std::string> VectorizeUserArgs(int argc, char* argv[]);


        
        /*
        Identify unrecognized user inputs by deduction and obtain a
        list of potential positional arguments.

        Args:
            argc        --> arg count
            argv        --> arg values, array of char*
            cli_args    --> ArgsVec of all configured CLIArgs
            pos_args    --> Vector of potential user positional arguments
                            as output variable

        Return:
            True if no errors occur, false otherwise.
        */
        static bool ValidateUserInput(int argc, char* argv[], const ArgsVec& cli_args,
            std::vector<std::string>& pos_args);


        
        /*
        Check if input matches potential optional arg or flag, i.e,
        either "-x" or "--xxx".

        Args:
            input       --> Potential flag or optional arg

        Return:
            True if may be candidate flag or optional arg; false
            otherwise.
        */
        static bool IsFlagOrOptArgCandidate(const std::string& input);



        /*
        Parse positional args directly from argc, argv. 

        Args:
            pos_args    --> positional arg values as determined by ValidateUserInput
            sorted_args --> Vector of CLIArg 
            pos_argc    --> Count of positional args as determined by SortArgs

        Return:
            True if positional args are present and can be parsed without error or
            if positional args are not present. False otherwise.
        */
        bool ParsePositionalArgs(const std::vector<std::string>& pos_args, 
            const ArgsVec& sorted_args, const size_t& pos_argc);

        

        /*
        Concatenate the user input arguments, neglecting positional arguments,
        into a single string.

        Args:
            argc        --> arg count
            argv        --> arg values, array of char*
            pos_argc    --> Count of positional args in sorted_args

        Return:
            String of concatenated arguments.
        */
        static std::string ConcatenateUserArgs(int argc, char* argv[], int pos_argc);



        /*
        Concatenate the user input arguments into a single string, replacing 
        optional arg user inputs with whitespace-converted versions.

        Args:
            argc        --> arg count
            argv        --> arg values, array of char*
            cli_args    --> Vector of CLIArg to determine which arguments 
                            are optional

        Return:
            String of concatenated arguments.
        */
        static std::string ConcatenateUserArgs(int argc, char* argv[], const ArgsVec& cli_args);



        /*
        Parse optional args from a single string which represents a series of optional
        args and flags given by user as arguments to the program.

        Optional args are of the key, value form: "--data blahblah".

        Args:
            user_args       --> Single string representing user optional args and flags
            sorted_args     --> Vector of CLIArg sorted such that positional args
                                occupy the highest indices

        Return:
            True if optional args are present and can be parsed without error
            or optional args are not present. False otherwise.
        */
        bool ParseOptionalArgs(const std::string& user_args, const ArgsVec& sorted_args);


        /*
        Parse flags from a single string which represents a series of optional args and
        flags given by the user.

        Flags are of the form "--verbose" or "-v" and do not have accompanying data.

        Args:
            user_args       --> Single string representing user optional args and flags
            sorted_args     --> Vector of CLIArg sorted such that positional args
                                occupy the highest indices

        Return:
            True if flags are or ARE NOT present -- true always. Leaving
            bool return type for future consideration.
        */
        bool ParseFlags(const std::string& user_args, const ArgsVec& sorted_args);



        /*
        Make string representing help matter for all configured args
        */
        std::string MakeHelpString();

        

        /*
        Make first line of help output indicating order of arguments, 
        "Usage: <program name> [options] ...".

        Args:
            prog_name   --> Program name 
            indent      --> Character count for following lines to align
                            with program name, used as return argument
        Return:
            The first help line
        */
        static std::string MakeGeneralUsageString(const std::string& prog_name, size_t& indent);



        /*
        Make arg-specific usage string indented to 'indent' characters.

        Args:
            prog_name       --> Program name 
            max_width       --> mac character width
            indent          --> Character count for following lines to align
                                with program name
            sorted_args     --> Vector of CLIArg sorted such that positional args
                                occupy the highest indices

        Return:
            The arg-specific usage string
        */
        static std::string MakeArgSpecificUsageString(const std::string& prog_name,
            const size_t& max_width, const size_t& indent, const ArgsVec& sorted_args);

        

        /*
        Make a string with an argument description, including the argument
        label (short_label) and new line characters if necessary.

        Args:
            args        --> CLIArg for which help string will be generated
            max_width       --> mac character width
            indent          --> Character count for following lines to align
                                with program name

        Return:
            The help string
        */
        static std::string MakeArgHelpString(const std::shared_ptr<CLIArg>& arg,
            const size_t& max_width, const size_t& indent);

        

        /*
        Make a formatted program description, accounting for terminal width.
        
        Args:
            description     --> Description string.
            max_width       --> mac character width

        Return:
            Formatted description with newlines placed to account for
            terminal width.
        */
        // static std::string FormatDescriptionString
};

template <typename ArgDataType>
std::shared_ptr<CLIArg> CLI::AddOption(std::string label, std::string help_str, 
    ArgDataType& output_value)
{
    std::shared_ptr<CLIArg> sptr = CLIPositionalArg<ArgDataType>::Make(label, help_str, output_value);
    args_.push_back(sptr);
    return sptr;
}

template <typename ArgDataType>
std::shared_ptr<CLIArg> CLI::AddOption(std::string flag, std::string short_flag, std::string help_str, 
        ArgDataType the_default, ArgDataType& output)
{
    std::shared_ptr<CLIArg> sptr = CLIOptionalArg<ArgDataType>::Make(flag, short_flag, 
        help_str, the_default, output);
    args_.push_back(sptr);
    return sptr;
}

template <typename ArgDataType>
std::shared_ptr<CLIArg> CLI::AddOption(std::string flag, std::string short_flag, std::string help_str, 
        const std::vector<ArgDataType>& the_default, std::vector<ArgDataType>& output)
{
    std::shared_ptr<CLIArg> sptr = CLIOptionalArg<std::vector<ArgDataType>>::Make(flag, short_flag, 
        help_str, the_default, output);
    args_.push_back(sptr);
    return sptr;
}

template <>
inline std::shared_ptr<CLIArg> CLI::AddOption<bool>(std::string flag, std::string short_flag, std::string help_str, 
        bool the_default, bool& output)
{
    std::shared_ptr<CLIArg> sptr = CLIFlag::Make(flag, short_flag, help_str, the_default, output);
    args_.push_back(sptr);
    return sptr;
}

#endif  // CLI_H_ 