#ifndef META_CONFIG_PARAMS_H_
#define META_CONFIG_PARAMS_H_

#include <string>

class MetaCLIConfigParams
{
    public:
        std::string subcommand_;
        std::string translate_subcommand_;
        std::string util_subcommand_;
        bool help_requested_;
        bool show_version_;
        MetaCLIConfigParams() : subcommand_(""), translate_subcommand_(""),
            help_requested_(false), show_version_(false),
            util_subcommand_("")
        {}
};

#endif  // META_CONFIG_PARAMS_H_