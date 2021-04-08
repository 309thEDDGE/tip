#include "yaml_schema_validation.h"
#include "file_reader.h"
#include "managed_path.h"

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("Args not present\n");
		return 0;
	}

	ManagedPath yaml_test_path = std::string(argv[1]);
	ManagedPath yaml_schema_path = std::string(argv[2]);

	FileReader fr_test;
	if (fr_test.ReadFile(yaml_test_path.string()) == 1)
	{
		printf("Failed to read YAML file under test: %s\n", yaml_test_path.RawString().c_str());
		return 0;
	}
	FileReader fr_schema;
	if (fr_schema.ReadFile(yaml_schema_path.string()) == 1)
	{
		printf("Failed to read YAML schema file: %s\n", yaml_schema_path.RawString().c_str());
		return 0;
	}

	// Concatenate all lines into a single string. It is requisite to append
	// to each line the newline character. Yaml loader must see new lines to
	// understand context.
	std::vector<std::string> test_lines = fr_test.GetLines();
	std::stringstream ss_test;
	std::for_each(test_lines.begin(), test_lines.end(),
		[&ss_test](const std::string& s) { ss_test << s; ss_test << "\n"; });
	std::string all_test_lines = ss_test.str();

	std::vector<std::string> schema_lines = fr_schema.GetLines();
	std::stringstream ss_schema;
	std::for_each(schema_lines.begin(), schema_lines.end(),
		[&ss_schema](const std::string& s) { ss_schema << s; ss_schema << "\n"; });
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
		printf("\nValidation result: PASS\n");
	else
		printf("\nValidation result: FAIL\n");

	return 0;
}