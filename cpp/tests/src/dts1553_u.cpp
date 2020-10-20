#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "dts1553.h"

TEST(DTS1553Test, ProcessLinesAsYamlValidateInput)
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

TEST(DTS1553Test, ProcessLinesAsYamlRequiredNodes)
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

TEST(DTS1553Test, ProcessLinesAsYamlCorrectNodeData)
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

TEST(DTS1553Test, FillSupplBusNameToMsgKeyMapValidate)
{
	DTS1553 dts;
	YAML::Node suppl_busmap_node;
	std::map<std::string, std::set<uint64_t>> out_map;
	
	// If empty return true and leave output map empty.
	EXPECT_TRUE(dts.FillSupplBusNameToMsgKeyMap(suppl_busmap_node, out_map));
	EXPECT_TRUE(out_map.size() == 0);

	// Root is sequence, fail
	suppl_busmap_node = YAML::Load({
		"- BUSA: [1, 2]\n"
		"- ABBD: [4211, 8202]\n"
		});
	EXPECT_FALSE(dts.FillSupplBusNameToMsgKeyMap(suppl_busmap_node, out_map));

	// Root is map, not all values are sequences, fail
	suppl_busmap_node = YAML::Load({
		"BUSA:\n"
		"  - [1, 2]\n"
		"  - [3432, 31223, 6673]\n"
		"ABBD: {a: b, c: d}\n"
		});
	EXPECT_FALSE(dts.FillSupplBusNameToMsgKeyMap(suppl_busmap_node, out_map));

	// Mapped value is sequence, but does not contain two items, fail
	suppl_busmap_node = YAML::Load({
		"BUSA:\n"
		"  - [1, 2]\n"
		"  - [3432, 31223, 6673]\n"
		"ABBD:\n"
		"  - [4545, 7212]\n"
		"  - [7632, 124]\n"
		"  - [852, 831]\n"
		});
	EXPECT_FALSE(dts.FillSupplBusNameToMsgKeyMap(suppl_busmap_node, out_map));

}