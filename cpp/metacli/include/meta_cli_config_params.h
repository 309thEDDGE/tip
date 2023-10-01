#ifndef META_CONFIG_PARAMS_H_
#define META_CONFIG_PARAMS_H_

class MetaCLIConfigParams
{
    public:
        std::string subcommand_;
        std::string translate_subcommand_;
        MetaCLIConfigParams() : subcommand_(""), translate_subcommand_("")
        {}
};

#endif  // META_CONFIG_PARAMS_H_