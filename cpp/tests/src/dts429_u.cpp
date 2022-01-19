#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "dts429.h"


class Dts429Test
{
   public:
    std::vector<std::string> yaml_lines_0{"translatable_word_definitions:",
                                " TestWord:",
                                "    wrd_data:",
                                "      label: 107",
                                "      bus: 5",
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

    std::vector<std::string> yaml_lines_1{"translatable_word_definitions:\n",
                                " TestWord:\n",
                                "    wrd_data:\n",
                                "      label: 107\n",
                                "      bus: 5\n",
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

    std::vector<std::string> yaml_lines_2{};

    std::vector<std::string> yaml_lines_3{"translatable_word_definitions: {}\n",
                                "supplemental_bus_map_labels: {}\n",
    };

    std::vector<std::string> yaml_lines_4{"translatable_word_definitions: 'Empty'\n",
    };

    std::vector<std::string> yaml_lines_5{"translatable_word_definitions: 'Empty'\n",
                                "supplemental_bus_map_labels: {}\n",
    };

    std::vector<std::string> yaml_lines_6{"translatable_word_definitions:",
                                " TestWord: 'wrd_data'",
                                "supplemental_bus_map_labels:",
                                "  A429BusAlpha:",
                                "    - [ 7, 4, 12, 124]"
    };



    // builds yaml root node from one of yaml_lines_n above
    void build_node( const std::vector<std::string>& lines,
                          YAML::Node& root_node)
    {
        std::stringstream ss;
        std::for_each(lines.begin(), lines.end(),
                    [&ss](const std::string& s) {
                        ss << s;
                        ss << "\n";
                    });
        std::string all_lines = ss.str();

        root_node = YAML::Load(all_lines.c_str());
    }

};

TEST(DTS429Test, IngestLinesNoStringsInVector)
{
    DTS429 dts;
    Dts429Test input;
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;

    EXPECT_FALSE(dts.IngestLines(input.yaml_lines_2, word_elements));
}

// root node must have entry for translateable_word_defintions and supplemental_bus_maps
TEST(DTS429Test, IngestLinesTwoMapsInRootNode)
{
    DTS429 dts;
    Dts429Test input;
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;

    EXPECT_FALSE(dts.IngestLines(input.yaml_lines_4, word_elements));
}

// Root Node's translateable_word_defintions and supplemental_bus_maps are Maps
TEST(DTS429Test, IngestLinesElementsAreMaps)
{
    DTS429 dts;
    Dts429Test input;
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;

    EXPECT_TRUE(dts.IngestLines(input.yaml_lines_3, word_elements));

    EXPECT_FALSE(dts.IngestLines(input.yaml_lines_5, word_elements));
}

TEST(DTS429Test, ProcessLinesAsYamlValidateOutput)
{
    // Ensure that SupplementalBusMapLabels are stored in the correct
    // map, and the translateable_word_defs are stored in correct map
    YAML::Node wrd_defs_node;
    YAML::Node suppl_busmap_node;
    YAML::Node root_node;
    Dts429Test input;
    DTS429 dts;

    input.build_node(input.yaml_lines_0, root_node);

    EXPECT_TRUE(dts.ProcessLinesAsYaml(root_node, wrd_defs_node, suppl_busmap_node));
    EXPECT_TRUE(wrd_defs_node["TestWord"]);
    EXPECT_TRUE(suppl_busmap_node["A429BusAlpha"]);
}

TEST(DTS429Test, ValidateWordNodeNotAMap)
{
    YAML::Node word_node(YAML::NodeType::Scalar);
    DTS429 dts;
    ASSERT_FALSE(dts.ValidateWordNode(word_node));
}

TEST(DTS429Test, ValidateWordNodeMissingRequiredKey)
{
    // Ought to have "elem" as key
    YAML::Node word_node = YAML::Load(
        "wrd_data: \n"
        "element: \n"
    );

    DTS429 dts;
    ASSERT_FALSE(dts.ValidateWordNode(word_node));

    // Ought to have "wrd_data" as key
    YAML::Node word_node2 = YAML::Load(
        "elem: \n"
    );
    ASSERT_FALSE(dts.ValidateWordNode(word_node));
}


TEST(DTS429Test, CreateICDElementFromWordNodesTestOutput)
{
    Dts429Test input;
    DTS429 dts;

    YAML::Node root_node;
    input.build_node(input.yaml_lines_0, root_node);
    YAML::Node transl_wrd_defs_node = root_node["translatable_word_definitions"];
    YAML::Node word_name_node = transl_wrd_defs_node["TestWord"];

    // build nodes to build a specific element
    YAML::Node wrd_data_node = word_name_node["wrd_data"];
    YAML::Node elem_node = word_name_node["elem"]["107_alt"];
    ICDElement output_element;

    // create expected element
    ICDElement expected_element;
    expected_element.label_= 107;
    expected_element.sdi_=2;
    expected_element.bus_name_=5;
    expected_element.msg_name_="TestWord";
    expected_element.rate_=1.1;
    expected_element.description_="Altitude";
    expected_element.xmit_lru_name_="LRU921";
    expected_element.elem_name_="107_alt";
    expected_element.schema_=ICDElementSchema::UNSIGNEDBITS;
    expected_element.is_bitlevel_=true;
    expected_element.bcd_partial_=-1;
    expected_element.msb_val_=1.0;
    expected_element.bitlsb_=11;
    expected_element.bit_count_=8;
    expected_element.uom_="FT";
    expected_element.classification_=0;

    dts.CreateICDElementFromWordNodes("TestWord",wrd_data_node, elem_node, output_element);

    EXPECT_EQ(expected_element.label_, output_element.label_);
    EXPECT_EQ(expected_element.sdi_, output_element.sdi_);
    EXPECT_EQ(expected_element.bus_name_, output_element.bus_name_);
    EXPECT_EQ(expected_element.msg_name_, output_element.msg_name_);
    EXPECT_EQ(expected_element.rate_, output_element.rate_);
    EXPECT_EQ(expected_element.description_, output_element.description_);
    EXPECT_EQ(expected_element.xmit_lru_name_, output_element.xmit_lru_name_);
    EXPECT_EQ(expected_element.elem_name_, output_element.elem_name_);
    EXPECT_EQ(expected_element.schema_, output_element.schema_);
    EXPECT_EQ(expected_element.is_bitlevel_, output_element.is_bitlevel_);
    EXPECT_EQ(expected_element.bcd_partial_, output_element.bcd_partial_);
    EXPECT_EQ(expected_element.msb_val_, output_element.msb_val_);
    EXPECT_EQ(expected_element.bitlsb_, output_element.bitlsb_);
    EXPECT_EQ(expected_element.bit_count_, output_element.bit_count_);
    EXPECT_EQ(expected_element.uom_, output_element.uom_);
    EXPECT_EQ(expected_element.classification_, output_element.classification_);

}

TEST(DTS429Test, BuildNameToICDElementMapTestNullNode)
{
    DTS429 dts;
    YAML::Node root_node;
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;

    EXPECT_FALSE(dts.BuildNameToICDElementMap(root_node, word_elements));
}

TEST(DTS429Test, BuildNameToICDElementMapValidateYamlNodeIsMap)
{
    // Ensure that the input maps to a map
    YAML::Node root_node;
    YAML::Node transl_wrd_defs;
    Dts429Test input;
    DTS429 dts;
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;

    input.build_node(input.yaml_lines_5, root_node);
    transl_wrd_defs = root_node["translatable_word_definitions"];

    EXPECT_FALSE(dts.BuildNameToICDElementMap(transl_wrd_defs, word_elements));
}

TEST(DTS429Test, BuildNameToICDElementMapValidateOutputVectorSize)
{
    YAML::Node root_node;
    YAML::Node transl_wrd_defs;
    Dts429Test input;
    DTS429 dts;
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;

    input.build_node(input.yaml_lines_0, root_node);
    transl_wrd_defs = root_node["translatable_word_definitions"];

    dts.BuildNameToICDElementMap(transl_wrd_defs, word_elements);
    std::vector<ICDElement> output_elements = word_elements["TestWord"];

    EXPECT_EQ(2,output_elements.size());

}

// TEST(DTS429Test, FillSupplBusNameToWordKeyMapValidateInput)
// {
//     // NOTE: Not spending too much effort on input validation
//     DTS429 dts;

//     // if the size == zero, the private map should not be filled
//     YAML::Node suppl_busmap_node;
//     std::map<std::string, std::set<uint32_t>> out_map;

//     // If empty return true and leave output map empty.
//     EXPECT_TRUE(dts.FillSupplBusNameToWordKeyMap(suppl_busmap_node, out_map));
//     EXPECT_TRUE(out_map.size() == 0);
// }

// TEST(DTS429Test, FillSupplBusNameToWordKeyMapValidateOutput)
// {
//     DTS429 dts;
//     Dts429Test input;

//     // Tests to ensure output validity
//     std::map<std::string, std::set<uint32_t>> out_map;
//     std::map<std::string, std::set<uint32_t>> expected_map;
//     YAML::Node suppl_busmap_node = YAML::Load(
//         "supplemental_bus_map_labels:\n  {A429BusAlpha:\n    - [ 7, 4, 12, 124] }");

//     std::set<uint32_t> alpha = {7, 4, 12, 124};
//     expected_map["A429BusAlpha"] = alpha;

//     // fill output map
//     dts.FillSupplBusNameToWordKeyMap(suppl_busmap_node, out_map);

//     // expect fillSuppleBusNametoWrdKeyMap True
//     bool map_equality = expected_map.size() == out_map.size()
//         && std::equal(expected_map.begin(), expected_map.end(),
//                       out_map.begin());

//     EXPECT_TRUE(map_equality);
// }

