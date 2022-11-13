#include "yaml_schema_validation.h"
#include "file_reader.h"
#include "managed_path.h"
#include "cli_group.h"

bool ConfigureCLI(CLIGroup& cli_group, bool& help_requested, std::string& yaml_path_str,
    std::string& schema_path_str);

int main(int argc, char* argv[])
{
    CLIGroup cli_group;
    bool help_requested = false;
    std::string yaml_path_str("");
    std::string schema_path_str("");

    if(!ConfigureCLI(cli_group, help_requested, yaml_path_str, schema_path_str))
        return -1;

    std::string nickname = "";
    std::shared_ptr<CLIGroupMember> cli;
    if (!cli_group.Parse(argc, argv, nickname, cli) || help_requested)
    {
        printf("%s", cli_group.MakeHelpString().c_str());
        return 0;
    }

    ManagedPath yaml_path(yaml_path_str);
    ManagedPath schema_path(schema_path_str);
    FileReader fr_test;
    if (fr_test.ReadFile(yaml_path.string()) == 1)
    {
        printf("Failed to read YAML file under test: %s\n", yaml_path.RawString().c_str());
        return -1;
    }
    FileReader fr_schema;
    if (fr_schema.ReadFile(schema_path.string()) == 1)
    {
        printf("Failed to read YAML schema file: %s\n", schema_path.RawString().c_str());
        return -1;
    }

    // Concatenate all lines into a single string. It is requisite to append
    // to each line the newline character. Yaml loader must see new lines to
    // understand context.
    std::vector<std::string> test_lines = fr_test.GetLines();
    std::stringstream ss_test;
    std::for_each(test_lines.begin(), test_lines.end(),
                  [&ss_test](const std::string& s) {
                      ss_test << s;
                      ss_test << "\n";
                  });
    std::string all_test_lines = ss_test.str();

    std::vector<std::string> schema_lines = fr_schema.GetLines();
    std::stringstream ss_schema;
    std::for_each(schema_lines.begin(), schema_lines.end(),
                  [&ss_schema](const std::string& s) {
                      ss_schema << s;
                      ss_schema << "\n";
                  });
    std::string all_schema_lines = ss_schema.str();

    YAML::Node test_node = YAML::Load(all_test_lines.c_str());
    YAML::Node schema_node = YAML::Load(all_schema_lines.c_str());

    std::vector<LogItem> log;
    YamlSV ysv;
    bool res = ysv.Validate(test_node, schema_node, log);

    for (std::vector<LogItem>::const_iterator it = log.begin();
         it != log.end(); ++it)
    {
        if (it->log_value >= static_cast<uint8_t>(LogLevel::Info))
            it->Print();
    }

    if (res)
    {
        printf("\nValidation result: PASS\n");
        return 0;
    }
    else
    {
        printf("\nValidation result: FAIL\n");
        return 1;
    }
}

bool ConfigureCLI(CLIGroup& cli_group, bool& help_requested, std::string& yaml_path_str,
    std::string& schema_path_str)
{
    std::string exe_name = "validate_yaml";
    std::string description = "Validate an input yaml file given in YAML_PATH with "
        "a yaml schema given in SCHEMA_PATH. For schema examples, see \"tip_translate_1553 --dts_help\" or "
        "\"tip_translate_arinc429 --dts_help\". The text printed to stdout contains yaml schema matter "
        "and an explanation of the fields in the corresponding 1553 or ARINC429 ICD configuration "
        "documents, respectively, which are to be validated by the aforementioned schema.\n\nPrint "
        "\"PASS\" (0) to stdout if YAML_PATH is valid according to SCHEMA_PATH, or \"FAIL\" (1) "
        "if invalid and return the value shown in parentheses. A NULL result returns -1.";
    std::shared_ptr<CLIGroupMember> cli_help = cli_group.AddCLI(exe_name, 
    description, "clihelp");
    cli_help->AddOption("--help", "-h", "Show usage information", false, help_requested, true);

    std::shared_ptr<CLIGroupMember> cli_full = cli_group.AddCLI(exe_name,
        description, "clifull");

    std::string yaml_path_help = "Full path to yaml file which is to be validated against SCHEMA_PATH";
    cli_full->AddOption("yaml_path", yaml_path_help, yaml_path_str, true);

    std::string schema_path_help = "Full path to schema, also in yaml format, with which to validate YAML_PATH";
    cli_full->AddOption("schema_path", schema_path_help, schema_path_str, true);

    if(!cli_group.CheckConfiguration())
        return false;

    return true;
}
