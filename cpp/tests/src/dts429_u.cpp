#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "dts429.h"


class Dts429Test
{
   public:
    const std::vector<std::string> yaml_lines_0{"translatable_word_definitions:",
                                " TestWord:",
                                "    wrd_data:",
                                "      label: 107",
                                "      bus: 5",
                                "      sdiusd: false",
                                "      sdi: 2",
                                "      rate: 1.1",
                                "      desc: 'Test'",
                                "      lru_name: 'LRU921'",
                                "    elem:",
                                "      107_alt:",
                                "        schema: UNSIGNEDBITS",
                                "        msbval: 1.0",
                                "        lsb: 11",
                                "        bitcnt: 8",
                                "        desc: 'Altitude'",
                                "        uom: 'FT'",
                                "        class: 0",
                                "      107_speed:",
                                "        schema: UNSIGNEDBITS",
                                "        msbval: 1.0",
                                "        lsb: 11",
                                "        bitcnt: 8",
                                "        desc: 'Airspeed'",
                                "        uom: 'FT/Sec'",
                                "        class: 0",
                                "supplemental_bus_map_labels:",
                                "  A429BusAlpha:",
                                "    - [ 7, 4, 12, 124]"
    };

    const std::vector<std::string> yaml_lines_1{"translatable_word_definitions:",
                                " TestWord:\n",
                                "    wrd_data:\n",
                                "      label: 107\n",
                                "      bus: 5\n",
                                "      sdiusd: false\n",
                                "      sdi: 2\n",
                                "      rate: 1.1\n",
                                "      desc: 'Test'\n",
                                "      lru_name: 'LRU921'\n",
                                "    elem:\n",
                                "      107_alt:\n",
                                "        schema: UNSIGNEDBITS\n",
                                "        msbval: 1.0\n",
                                "        lsb: 11\n",
                                "        bitcnt: 8\n",
                                "        desc: 'Altitude'\n",
                                "        uom: 'FT'\n",
                                "        class: 0\n",
                                "      107_speed:\n",
                                "        schema: UNSIGNEDBITS\n",
                                "        msbval: 1.0\n",
                                "        lsb: 11\n",
                                "        bitcnt: 8\n",
                                "        desc: 'Airspeed'\n",
                                "        uom: 'FT/Sec'\n",
                                "        class: 0\n",
                                "supplemental_bus_map_labels:\n",
                                "  A429BusAlpha:\n",
                                "    - [ 7, 4, 12, 124]\n"
    };

};

TEST(DTS429Test, IngestLinesNonNewlineTerminatedLinesVector)
{
    DTS429 dts;
    std::map<std::string, std::string> wrd_name_substitutions;
    std::map<std::string, std::string> elem_name_substitutions;

    EXPECT_FALSE(dts.IngestLines(yaml_lines_0, elem_name_substitutions, wrd_name_substitutions));

}

TEST(DTS429Test, IngestLines)
{
    DTS429 dts;
    std::map<std::string, std::string> wrd_name_substitutions;
    std::map<std::string, std::string> elem_name_substitutions;

    EXPECT_TRUE(dts.IngestLines(yaml_lines_1, elem_name_substitutions, wrd_name_substitutions));

    // if there are further output checks, add here
}

TEST(DTS429Test, FillSupplBusNameToWordKeyMapValidateInput)
{
    // NOTE: Not spending too much effort on input validation
    DTS429 dts;

    // if the size == zero, the private map should not be filled
    YAML::Node suppl_busmap_node;
    std::map<std::string, std::set<uint32_t>> out_map;

    // If empty return true and leave output map empty.
    EXPECT_TRUE(dts.FillSupplBusNameToWrdKeyMap(suppl_busmap_node, out_map));
    EXPECT_TRUE(out_map.size() == 0);
}

TEST(DTS429Test, FillSupplBusNameToWordKeyMapValidateOutput)
{
    // Tests to ensure output validity

    // expect fillSuppleBusNametoWrdKeyMap True

    EXPECT_FALSE(true);
}

TEST(DTS429Test, ProcessLinesAsYamlValidateInput)
{
    // Test root node entry == size 0 - ensure there are lines passed in or else it fails
    std::vector<std::string> lines;
    YAML::Node wrd_defs_node;
    YAML::Node suppl_busmap_node;

    // Empty lines vector fails
    EXPECT_FALSE(dts.ProcessLinesAsYaml(lines, wrd_defs_node, suppl_busmap_node));

    // test to ensure that yaml lines are all new line terminated on the way in
    Dts429Test test429lines = Dts429Test();

    // non-newline-terminated lines vector fails
    lines = test429lines.yaml_lines_0;
    EXPECT_FALSE(dts.ProcessLinesAsYaml(lines, wrd_defs_node, suppl_busmap_node));

    // newline-terminated lines vector pass
    lines = test429lines.yaml_lines_1;
    EXPECT_TRUE(dts.ProcessLinesAsYaml(lines, wrd_defs_node, suppl_busmap_node));
}

TEST(DTS429Test, ProcessLinesAsYamlValidateOutput)
{
    // Ensure that SupplementalBusMapLabels are stored in the correct
    // map, and the translateable_word_defs are stored in correct map
    YAML::Node wrd_defs_node;
    YAML::Node suppl_busmap_node;
    std::vector<std::string> lines;

    lines = test429lines.yaml_lines_1;
    EXPECT_TRUE(dts.ProcessLinesAsYaml(lines, wrd_defs_node, suppl_busmap_node));
    EXPECT_TRUE(wrd_defs_node["TestWord"]);
    EXPECT_TRUE(suppl_busmap_node["A429BusAlpha"]);
}
