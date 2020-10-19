#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "dts1553.h"

TEST(DTS1553ProcessLinesAsYaml, ValidateInput)
{
	DTS1553 dts;
	std::vector<std::string> lines;
	YAML::Node msg_defs_node;
	YAML::Node suppl_busmap_node;

	// Empty lines vector fails
	EXPECT_FALSE(dts.ProcessLinesAsYaml(lines, msg_defs_node, suppl_busmap_node));

	// Yaml root node must be a map
	lines.push_back("- a:b\n");
	lines.push_back("- c:d\n"); // sequence, not map
	EXPECT_FALSE(dts.ProcessLinesAsYaml(lines, msg_defs_node, suppl_busmap_node));
}

TEST(DTS1553ProcessLinesAsYaml, RequiredNodes)
{
	DTS1553 dts;
	std::vector<std::string> lines;
	YAML::Node msg_defs_node;
	YAML::Node suppl_busmap_node;

	// Message definitions must be present
	lines.clear();
	lines.push_back("translatable_messagedefinitions:"); // misspelled
	EXPECT_FALSE(dts.ProcessLinesAsYaml(lines, msg_defs_node, suppl_busmap_node));

	lines.clear();
	lines.push_back("translatable_message_definitions:"); // correct
	EXPECT_TRUE(dts.ProcessLinesAsYaml(lines, msg_defs_node, suppl_busmap_node));
}

TEST(DTS1553ProcessLinesAsYaml, CorrectNodeData)
{
	DTS1553 dts;
	std::vector<std::string> lines;
	YAML::Node msg_defs_node;
	YAML::Node suppl_busmap_node;

	// message defs only
	lines.push_back("translatable_message_definitions:");
	lines.push_back("  A: B");
	lines.push_back("  C: D");
	EXPECT_TRUE(dts.ProcessLinesAsYaml(lines, msg_defs_node, suppl_busmap_node));
	EXPECT_TRUE(msg_defs_node["A"]);
	EXPECT_TRUE(msg_defs_node["C"]);

	// message defs and supplemental bus map comm words
	lines.push_back("supplemental_bus_map_command_words:");
	lines.push_back("  BusA: {}");
	lines.push_back("  otherbus: 2");
	EXPECT_TRUE(dts.ProcessLinesAsYaml(lines, msg_defs_node, suppl_busmap_node));
	EXPECT_TRUE(msg_defs_node["A"]);
	EXPECT_TRUE(msg_defs_node["C"]);
	EXPECT_TRUE(suppl_busmap_node["BusA"]);
	EXPECT_TRUE(suppl_busmap_node["otherbus"]);
}