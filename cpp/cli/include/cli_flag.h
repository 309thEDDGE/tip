#ifndef CLI_FLAG_H_
#define CLI_FLAG_H_

#include <regex>
#include <memory>
#include <string>
#include "cli_optional_arg.h"


class CLIFlag : public CLIOptionalArg<bool>
{
    private:

    public:
        CLIFlag(std::string label, std::string short_label, const std::string& help_str, 
            bool the_default, bool& output);
        virtual ~CLIFlag() {}
        virtual bool Parse(const std::string& input);
        virtual std::string GetUsageRepr(); 

        /*
        Create a shared pointer to the base class, CLIArg, of
        a new instance of this class.
        */
        static std::shared_ptr<CLIArg> Make(const std::string& label, 
            std::string short_label, const std::string& help_str, 
            bool the_default, bool& output);

};

#endif  // CLI_FLAG_H_
