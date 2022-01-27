#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "dts429.h"


class DTS429Test : public ::testing::Test
{
   protected:
    DTS429 dts;
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node root_node;
    YAML::Node transl_wrd_defs;
    YAML::Node suppl_busmap_node;

   public:
    std::vector<std::string> yaml_lines_0{"translatable_word_definitions:",
                                " TestWord:",
                                "    wrd_data:",
                                "      label: 107",
                                "      bus: 'bus5'",
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
                                "      bus: 'bus5'\n",
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
    // properly structured "TestWord", empty suppl_busmap map
    std::vector<std::string> yaml_lines_7{"translatable_word_definitions:",
                                " TestWord:",
                                "    wrd_data:",
                                "      label: 107",
                                "      bus: 'bus5'",
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
                                "supplemental_bus_map_labels: {}"
    };

    std::vector<std::string> yaml_lines_8{"translatable_word_definitions:",
                                " TestWord: 'wrd_data'",
                                "supplemental_bus_map_labels: 'empty'"
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

TEST_F(DTS429Test, IngestLinesNoStringsInVector)
{
    EXPECT_FALSE(dts.IngestLines(yaml_lines_2, word_elements));
}

// root node must have entry for translateable_word_defintions and supplemental_bus_maps
TEST_F(DTS429Test, IngestLinesTwoMapsInRootNode)
{
    EXPECT_FALSE(dts.IngestLines(yaml_lines_4, word_elements));
}

// Root Node's translateable_word_defintions and supplemental_bus_maps map to maps
TEST_F(DTS429Test, IngestLinesElementsAreMaps)
{
    EXPECT_FALSE(dts.IngestLines(yaml_lines_5, word_elements));
    EXPECT_FALSE(dts.IngestLines(yaml_lines_8, word_elements));
}

// NBD if supplemental_bus_maps is empty
TEST_F(DTS429Test, IngestLinesSupplBusmapMapEmpty)
{
    EXPECT_TRUE(dts.IngestLines(yaml_lines_7, word_elements));
}

// Expect translateable_word_defintions map not empty
TEST_F(DTS429Test, IngestLinesTranslatableWordDefsMapEmpty)
{
    EXPECT_FALSE(dts.IngestLines(yaml_lines_3, word_elements));
}

TEST_F(DTS429Test, ProcessLinesAsYamlValidateOutputIdealConditions)
{
    // Ensure that SupplementalBusMapLabels are stored in the correct
    // map, and the translateable_word_defs are stored in correct map
    build_node(yaml_lines_0, root_node);

    EXPECT_TRUE(dts.ProcessLinesAsYaml(root_node, transl_wrd_defs, suppl_busmap_node));
    EXPECT_TRUE(transl_wrd_defs["TestWord"]);
    EXPECT_TRUE(suppl_busmap_node["A429BusAlpha"]);
}

TEST_F(DTS429Test, ProcessLinesAsYamlValidateOutputEmptySupplBusmap)
{
    // Test results when supple busmap node empty
    build_node(yaml_lines_7, root_node);

    EXPECT_TRUE(dts.ProcessLinesAsYaml(root_node, transl_wrd_defs, suppl_busmap_node));
    EXPECT_TRUE(transl_wrd_defs["TestWord"]);
    EXPECT_EQ(suppl_busmap_node.size(), 0);
}

TEST_F(DTS429Test, ValidateWordNodeNotAMap)
{
    YAML::Node word_node(YAML::NodeType::Scalar);
    ASSERT_FALSE(dts.ValidateWordNode(word_node));
}

TEST_F(DTS429Test, ValidateWordNodeMissingRequiredKey)
{
    // Ought to have "elem" as key
    YAML::Node word_node = YAML::Load(
        "wrd_data: \n"
        "element: \n"
    );

    ASSERT_FALSE(dts.ValidateWordNode(word_node));

    // Ought to have "wrd_data" as key
    YAML::Node word_node2 = YAML::Load(
        "elem: \n"
    );
    ASSERT_FALSE(dts.ValidateWordNode(word_node));
}


TEST_F(DTS429Test, CreateICDElementFromWordNodesTestOutput)
{
    build_node(yaml_lines_0, root_node);
    transl_wrd_defs = root_node["translatable_word_definitions"];
    YAML::Node word_name_node = transl_wrd_defs["TestWord"];

    // build nodes to build a specific element
    YAML::Node wrd_data_node = word_name_node["wrd_data"];
    YAML::Node elem_node = word_name_node["elem"]["107_alt"];
    ICDElement output_element;

    // create expected element
    ICDElement expected_element;
    expected_element.label_= 107;
    expected_element.sdi_= 2;            // 8-bit
    expected_element.bus_name_="bus5";
    expected_element.msg_name_="TestWord";
    expected_element.rate_=1.1;
    expected_element.description_="Altitude";
    expected_element.xmit_lru_name_="LRU921";
    expected_element.elem_name_="107_alt";
    expected_element.schema_=ICDElementSchema::UNSIGNEDBITS;
    expected_element.is_bitlevel_=true;
    expected_element.bcd_partial_=-1;
    expected_element.msb_val_=1.0;
    expected_element.bitlsb_= 11;        // 8-bit
    expected_element.bit_count_= 8;      // 8-bit
    expected_element.uom_="FT";
    expected_element.classification_=0;  // 8-bit

    dts.CreateICDElementFromWordNodes("TestWord","107_alt",wrd_data_node, elem_node, output_element);

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

TEST_F(DTS429Test, BuildNameToICDElementMapTestNullNode)
{
    EXPECT_FALSE(dts.BuildNameToICDElementMap(root_node, word_elements));
}

TEST_F(DTS429Test, BuildNameToICDElementMapValidateYamlNodeIsMap)
{
    // Ensure that the input maps to a map
    build_node(yaml_lines_5, root_node);
    transl_wrd_defs = root_node["translatable_word_definitions"];

    EXPECT_FALSE(dts.BuildNameToICDElementMap(transl_wrd_defs, word_elements));
}

TEST_F(DTS429Test, BuildNameToICDElementMapValidateOutputVectorSize)
{
    build_node(yaml_lines_0, root_node);
    transl_wrd_defs = root_node["translatable_word_definitions"];

    dts.BuildNameToICDElementMap(transl_wrd_defs, word_elements);
    std::vector<ICDElement> output_elements = word_elements["TestWord"];

    EXPECT_EQ(2,output_elements.size());
}

TEST_F(DTS429Test, BuildNameToICDElementMapValidateOutputMapFeatures)
{
    build_node(yaml_lines_0, root_node);
    transl_wrd_defs = root_node["translatable_word_definitions"];

    dts.BuildNameToICDElementMap(transl_wrd_defs, word_elements);

    EXPECT_TRUE(word_elements.count("TestWord"));
}
