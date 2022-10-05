#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "organize_429_icd.h"
#include "icd_element.h"
#include <unordered_map>
#include <vector>
#include <string>

class Organize429ICDTest : public ::testing::Test
{
   protected:
    Organize429ICD org;
    ICDElement expected_element;
    std::vector<std::string> md_chan_id_strings =
        {"tmats_chanid_to_429_subchan_and_name:",
        "    34: {1: SET1A, 2: SET1B, 3: SET2A, 4: SET2B}",
        "    35: {1: SET3A}",
        "    36: {1: SET3B}"};

    // builds yaml node from one of yaml_lines_n above
    void BuildNode( const std::vector<std::string>& lines,
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

    void SetupElement()
    {
    expected_element.label_= 107;
    expected_element.sdi_= 2;            // 8-bit
    expected_element.bus_name_="SET1B";
    expected_element.msg_name_="TestWord";
    expected_element.rate_= 1.1F;
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
    }
   public:
    Organize429ICDTest(){}

};


TEST_F(Organize429ICDTest, OrganizeICDMapWordElementsEmpty)
{
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node md_chanid_to_subchan_node;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
            uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;
    std::vector<std::vector<std::vector<ICDElement>>> element_table;

    Organize429ICD icd_org;
    BuildNode(md_chan_id_strings, md_chanid_to_subchan_node);

    EXPECT_FALSE(icd_org.OrganizeICDMap(word_elements, md_chanid_to_subchan_node, organized_lookup_map, element_table));
}


TEST_F(Organize429ICDTest, OrganizeICDMapYAMLNodeNull)
{
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node md_chanid_to_subchan_node;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
            uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;
    std::vector<ICDElement> element_vec;
    std::vector<std::vector<std::vector<ICDElement>>> element_table;

    Organize429ICD icd_org;
    SetupElement();
    element_vec.push_back(expected_element);
    word_elements["TestWord"] = element_vec;

    EXPECT_FALSE(icd_org.OrganizeICDMap(word_elements, md_chanid_to_subchan_node, organized_lookup_map, element_table));
}

TEST_F(Organize429ICDTest, OrganizeICDMapYAMLNodeNotMap)
{
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node md_chanid_to_subchan_node;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
            uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;
    std::vector<std::vector<std::vector<ICDElement>>> element_table;
    std::vector<ICDElement> element_vec;

    SetupElement();
    element_vec.push_back(expected_element);
    word_elements["TestWord"] = element_vec;

    std::vector<std::string> temp_node_input =
        {"'Fail'"};
    Organize429ICD icd_org;
    BuildNode(temp_node_input, md_chanid_to_subchan_node);

    EXPECT_FALSE(icd_org.OrganizeICDMap(word_elements, md_chanid_to_subchan_node, organized_lookup_map, element_table));
}

TEST_F(Organize429ICDTest, ValidateInputsWordElementsEmpty)
{
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node md_chanid_to_subchan_node;

    Organize429ICD icd_org;
    BuildNode(md_chan_id_strings, md_chanid_to_subchan_node);

    EXPECT_FALSE(icd_org.ValidateInputs(word_elements, md_chanid_to_subchan_node));
}


TEST_F(Organize429ICDTest, ValidateInputsYAMLNodeNull)
{
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node md_chanid_to_subchan_node;
    std::vector<ICDElement> element_vec;

    Organize429ICD icd_org;
    SetupElement();
    element_vec.push_back(expected_element);
    word_elements["TestWord"] = element_vec;

    EXPECT_FALSE(icd_org.ValidateInputs(word_elements, md_chanid_to_subchan_node));
}

TEST_F(Organize429ICDTest, ValidateInputsYAMLNodeNotMap)
{
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node md_chanid_to_subchan_node;
    std::vector<ICDElement> element_vec;

    SetupElement();
    element_vec.push_back(expected_element);
    word_elements["TestWord"] = element_vec;

    std::vector<std::string> temp_node_input =
        {"'Fail'"};
    Organize429ICD icd_org;
    BuildNode(temp_node_input, md_chanid_to_subchan_node);

    EXPECT_FALSE(icd_org.ValidateInputs(word_elements, md_chanid_to_subchan_node));
}

TEST_F(Organize429ICDTest, ValidateInputsYAMLNodeVarifyCorrectRoot)
{
    // varify that the root node is tmats_chanid_to_429_subchan_and_name
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node md_chanid_to_subchan_node;
    std::vector<ICDElement> element_vec;

    SetupElement();
    element_vec.push_back(expected_element);
    word_elements["TestWord"] = element_vec;

    std::vector<std::string> temp_node_input =
        {"wrong: {1,'Fail'}"};
    Organize429ICD icd_org;
    BuildNode(temp_node_input, md_chanid_to_subchan_node);
    EXPECT_FALSE(icd_org.ValidateInputs(word_elements, md_chanid_to_subchan_node));

    BuildNode(md_chan_id_strings, md_chanid_to_subchan_node);
    EXPECT_TRUE(icd_org.ValidateInputs(word_elements, md_chanid_to_subchan_node));
}

TEST_F(Organize429ICDTest, AddSubchannelToMapValidateOutput)
{
    Organize429ICD icd_org;
    uint16_t channelid;
    uint16_t subchannelid;
    std::string subchannel_name;
    std::unordered_map<std::string, std::tuple<uint16_t, uint16_t>> output_map;
    std::unordered_map<std::string, std::tuple<uint16_t, uint16_t>> expected_map;


    channelid = 14;
    subchannelid = 1;
    subchannel_name = "ABC1A";
    icd_org.AddSubchannelToMap(channelid, subchannelid, subchannel_name);

    expected_map.insert({subchannel_name, std::make_tuple(channelid, subchannelid)});
    output_map = icd_org.GetBusNameToChannelSubchannelMap();

    EXPECT_TRUE(output_map.size()==expected_map.size());
    EXPECT_TRUE(output_map.size()==1);
    EXPECT_TRUE(output_map.count("ABC1A")==1);
    EXPECT_TRUE(expected_map.count("ABC1A")==1);
    std::tuple<uint16_t, uint16_t> expected_tuple = expected_map["ABC1A"];
    std::tuple<uint16_t, uint16_t> output_tuple = output_map["ABC1A"];
    EXPECT_TRUE(expected_tuple==output_tuple);
    EXPECT_TRUE(std::get<0>(output_tuple)==14);
    EXPECT_TRUE(std::get<1>(output_tuple)==1);
}

TEST_F(Organize429ICDTest, AddSubchannelToMapSubchannelNameCollision)
{
    Organize429ICD icd_org;
    uint16_t channelid;
    uint16_t subchannelid;
    std::string subchannel_name;

    channelid = 14;
    subchannelid = 1;
    subchannel_name = "ABC1A";
    EXPECT_TRUE(icd_org.AddSubchannelToMap(channelid, channelid, subchannel_name));

    channelid = 15;
    EXPECT_FALSE(icd_org.AddSubchannelToMap(channelid, subchannelid, subchannel_name));
}

TEST_F(Organize429ICDTest, BuildBusNameToChannelAndSubchannelMapVarifySubchannelMap)
{
    // varify that a chanid maps to a map with subchannel number and name
    Organize429ICD icd_org;
    YAML::Node md_chanid_to_subchan_node;

    std::vector<std::string> temp_node_input =
        {"tmats_chanid_to_429_subchan_and_name:",
        "    34: {1: 'ABC', 2: 'DEF'}",
        "    35: 'fail'"};

    BuildNode(temp_node_input, md_chanid_to_subchan_node);
    EXPECT_FALSE(icd_org.BuildBusNameToChannelAndSubchannelMap(md_chanid_to_subchan_node));

    BuildNode(md_chan_id_strings, md_chanid_to_subchan_node);
    EXPECT_TRUE(icd_org.BuildBusNameToChannelAndSubchannelMap( md_chanid_to_subchan_node));

}

TEST_F(Organize429ICDTest, BuildBusNameToChannelAndSubchannelMapMultiBusToChanID)
{
    // varify that output is as expectedthat a chanid maps to a map with subchannel number and name
    // even when there are multiple busses mapped to chan id
    Organize429ICD icd_org;
    YAML::Node md_chanid_to_subchan_node;
    std::vector<std::string> temp_node_input =
        {"tmats_chanid_to_429_subchan_and_name:",
        "    34: {1: SET1A, 2: SET1B}",
        "    35: {1: SET2A}"};

    BuildNode(temp_node_input, md_chanid_to_subchan_node);

    ASSERT_TRUE(icd_org.BuildBusNameToChannelAndSubchannelMap(md_chanid_to_subchan_node));

    std::unordered_map<std::string, std::tuple<uint16_t, uint16_t>> output_map =
        icd_org.GetBusNameToChannelSubchannelMap();

    EXPECT_TRUE(output_map.size()==3);
    std::tuple<uint16_t, uint16_t> set1a_tuple = output_map["SET1A"];
    std::tuple<uint16_t, uint16_t> set1b_tuple = output_map["SET1B"];
    std::tuple<uint16_t, uint16_t> set2a_tuple = output_map["SET2A"];
    EXPECT_EQ(output_map.count("SET1A"),1);
    EXPECT_EQ(output_map.count("SET1B"),1);
    EXPECT_EQ(output_map.count("SET2A"),1);
    EXPECT_EQ(std::get<0>(set1a_tuple),34);
    EXPECT_EQ(std::get<1>(set1a_tuple),1);
    EXPECT_EQ(std::get<0>(set1b_tuple),34);
    EXPECT_EQ(std::get<1>(set1b_tuple),2);
    EXPECT_EQ(std::get<0>(set2a_tuple),35);
    EXPECT_EQ(std::get<1>(set2a_tuple),1);
}

TEST_F(Organize429ICDTest, BuildBusNameToChannelAndSubchannelMapCaseSensitivity)
{
    // varify that output is as expectedthat a chanid maps to a map with subchannel number and name
    // even when there are multiple busses mapped to chan id
    Organize429ICD icd_org;
    YAML::Node md_chanid_to_subchan_node;
    std::vector<std::string> temp_node_input =
        {"tmats_chanid_to_429_subchan_and_name:",
        "    34: {1: set1A, 2: SET1B}",
        "    35: {1: SET2A}"};

    BuildNode(temp_node_input, md_chanid_to_subchan_node);

    ASSERT_TRUE(icd_org.BuildBusNameToChannelAndSubchannelMap(md_chanid_to_subchan_node));

    std::unordered_map<std::string, std::tuple<uint16_t, uint16_t>> output_map =
        icd_org.GetBusNameToChannelSubchannelMap();

    EXPECT_TRUE(output_map.size()==3);
    EXPECT_EQ(output_map.count("set1A"),0);
    EXPECT_EQ(output_map.count("SET1A"),1);
}

TEST_F(Organize429ICDTest, GetICDElementComponentsEmptyVector)
{
    std::vector<ICDElement> elements;
    uint16_t label;
    std::string bus_name;
    int8_t sdi;

    ASSERT_FALSE(org.GetICDElementComponents(elements,label,bus_name,sdi));
}

TEST_F(Organize429ICDTest, GetICDElementComponentsWordNameEmptyString)
{
    std::vector<ICDElement> elements;
    uint16_t label;
    std::string bus_name;
    int8_t sdi;
    SetupElement();
    expected_element.bus_name_="";
    elements.push_back(expected_element);

    ASSERT_FALSE(org.GetICDElementComponents(elements,label,bus_name,sdi));
}

TEST_F(Organize429ICDTest, GetICDElementComponentsLabelUnitialized)
{
    // test unitialized element label
    std::vector<ICDElement> elements;
    uint16_t label;
    std::string bus_name;
    int8_t sdi;
    expected_element.bus_name_="Test";  // empty string tested in method
    expected_element.sdi_=-1;
    elements.push_back(expected_element);

    ASSERT_FALSE(org.GetICDElementComponents(elements,label,bus_name,sdi));
}

TEST_F(Organize429ICDTest, GetICDElementComponentsInvalidSDI)
{
    std::vector<ICDElement> elements;
    uint16_t label;
    std::string bus_name;
    int8_t sdi;
    SetupElement();
    expected_element.sdi_=-2;
    elements.push_back(expected_element);
    EXPECT_FALSE(org.GetICDElementComponents(elements,label,bus_name,sdi));

    std::vector<ICDElement> elements2;
    expected_element.sdi_=4;
    elements2.push_back(expected_element);
    EXPECT_FALSE(org.GetICDElementComponents(elements2,label,bus_name,sdi));

    std::vector<ICDElement> elements3;
    expected_element.sdi_=1;
    elements3.push_back(expected_element);
    EXPECT_TRUE(org.GetICDElementComponents(elements3,label,bus_name,sdi));
}

TEST_F(Organize429ICDTest, GetICDElementComponentsValidateOutput)
{
    std::vector<ICDElement> elements;
    uint16_t label;
    std::string bus_name;
    int8_t sdi;
    SetupElement();
    elements.push_back(expected_element);
    int8_t expected_SDI = 2;
    uint16_t expected_label = 107;
    std::string expected_bn = "SET1B";

    ASSERT_TRUE(org.GetICDElementComponents(elements,label,bus_name,sdi));
    EXPECT_EQ(label, expected_label);
    EXPECT_EQ(sdi, expected_SDI);
    EXPECT_EQ(bus_name, expected_bn);
}

TEST_F(Organize429ICDTest, GetChannelSubchannelIDsFromMapEmptyInputMap)
{
    std::string subchan_name = "SET1B";
    uint16_t channelid;
    uint16_t subchan_number;
    std::unordered_map<std::string, std::tuple<uint16_t, uint16_t>> bus_to_ids_map;

    ASSERT_FALSE(org.GetChannelSubchannelIDsFromMap(subchan_name,channelid,
                            subchan_number, bus_to_ids_map));
}

TEST_F(Organize429ICDTest, GetChannelSubchannelIDsFromMapEmptyInputString)
{
    std::string subchan_name;
    uint16_t channelid;
    uint16_t subchan_number;
    std::unordered_map<std::string, std::tuple<uint16_t, uint16_t>> bus_to_ids_map;
    bus_to_ids_map.insert({"SET1B", std::make_tuple(1,3)});

    ASSERT_FALSE(org.GetChannelSubchannelIDsFromMap(subchan_name,channelid,
                            subchan_number, bus_to_ids_map));
}

TEST_F(Organize429ICDTest, GetChannelSubchannelIDsFromMapKeyMissingKey)
{
    std::string subchan_name = "SET1A";
    uint16_t channelid;
    uint16_t subchan_number;
    std::unordered_map<std::string, std::tuple<uint16_t, uint16_t>> bus_to_ids_map;
    bus_to_ids_map.insert({"SET1B", std::make_tuple(1,3)});

    ASSERT_FALSE(org.GetChannelSubchannelIDsFromMap(subchan_name,channelid,
                            subchan_number, bus_to_ids_map));
    std::vector<std::string> misses = org.GetSubchannelNameLookupMisses();
    EXPECT_EQ(misses.size(),1);
    EXPECT_EQ(misses[0],"SET1A");
}

TEST_F(Organize429ICDTest, GetChannelSubchannelIDsFromMapValidateOutput)
{
    std::string subchan_name = "SET1B";
    uint16_t channelid;
    uint16_t subchan_number;
    std::unordered_map<std::string, std::tuple<uint16_t, uint16_t>> bus_to_ids_map;
    bus_to_ids_map.insert({"SET1B", std::make_tuple(1,3)});

    ASSERT_TRUE(org.GetChannelSubchannelIDsFromMap(subchan_name,channelid,
                            subchan_number, bus_to_ids_map));

    EXPECT_EQ(channelid,1);
    EXPECT_EQ(subchan_number,3);
}

TEST_F(Organize429ICDTest, IndexInLookupMapEmtpyMap)
{
    // value five stored at [1][2][3][4]
    uint16_t chan_id = 1;
    uint16_t subchan_id = 2;
    uint16_t label = 3;
    int8_t sdi = 4;
    size_t table_index = 5;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
        uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;

    EXPECT_FALSE(org.IsIndexInLookupMap(chan_id,subchan_id,label,sdi,table_index,organized_lookup_map));
}

TEST_F(Organize429ICDTest, IndexInLookupMapChanIDMissing)
{
    // value five stored at [1][2][3][4]
    uint16_t chan_id = 2;
    uint16_t subchan_id = 2;
    uint16_t label = 3;
    int8_t sdi = 4;
    size_t table_index = 5;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
        uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;

    organized_lookup_map[1][2][3][4] = 5;

    EXPECT_FALSE(org.IsIndexInLookupMap(chan_id,subchan_id,label,sdi,table_index,organized_lookup_map));
}

TEST_F(Organize429ICDTest, IndexInLookupMapSubchanIDMissing)
{
    // value five stored at [1][2][3][4]
    uint16_t chan_id = 1;
    uint16_t subchan_id = 3;
    uint16_t label = 3;
    int8_t sdi = 4;
    size_t table_index = 5;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
        uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;

    organized_lookup_map[1][2][3][4] = 5;

    EXPECT_FALSE(org.IsIndexInLookupMap(chan_id,subchan_id,label,sdi,table_index,organized_lookup_map));
}

TEST_F(Organize429ICDTest, IndexInLookupMapLabelMissing)
{
    // value five stored at [1][2][3][4]
    uint16_t chan_id = 1;
    uint16_t subchan_id = 2;
    uint16_t label = 4;
    int8_t sdi = 4;
    size_t table_index = 5;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
        uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;

    organized_lookup_map[1][2][3][4] = 5;

    EXPECT_FALSE(org.IsIndexInLookupMap(chan_id,subchan_id,label,sdi,table_index,organized_lookup_map));
}

TEST_F(Organize429ICDTest, IndexInLookupMapSDIMissing)
{
    // value five stored at [1][2][3][4]
    uint16_t chan_id = 1;
    uint16_t subchan_id = 2;
    uint16_t label = 3;
    int8_t sdi = 5;
    size_t table_index = 5;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
        uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;

    organized_lookup_map[1][2][3][4] = 5;

    EXPECT_FALSE(org.IsIndexInLookupMap(chan_id,subchan_id,label,sdi,table_index,organized_lookup_map));
}

TEST_F(Organize429ICDTest, IndexInLookupMapValidateOutput)
{
    // value five stored at [1][2][3][4]
    uint16_t chan_id = 1;
    uint16_t subchan_id = 2;
    uint16_t label = 3;
    int8_t sdi = 4;
    size_t table_index = 1;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
        uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;

    organized_lookup_map[1][2][3][4] = 5;

    ASSERT_TRUE(org.IsIndexInLookupMap(chan_id,subchan_id,label,sdi,table_index,organized_lookup_map));
    EXPECT_EQ(table_index,5);
}

TEST_F(Organize429ICDTest, IndexInLookupMapValidateOutputMultipleTableIndexInMap)
{
    // value five stored at [1][2][3][4]
    uint16_t chan_id = 1;
    uint16_t subchan_id = 2;
    uint16_t label = 3;
    int8_t sdi = 4;
    size_t table_index = 1;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
        uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;

    organized_lookup_map[1][2][3][4] = 5;
    organized_lookup_map[7][2][3][4] = 8;

    ASSERT_TRUE(org.IsIndexInLookupMap(chan_id,subchan_id,label,sdi,table_index,organized_lookup_map));
    EXPECT_EQ(table_index,5);

    chan_id = 7;
    EXPECT_TRUE(org.IsIndexInLookupMap(chan_id,subchan_id,label,sdi,table_index,organized_lookup_map));
    EXPECT_EQ(table_index,8);
}

TEST_F(Organize429ICDTest, AddToElementTableEmptyWordElements)
{
    size_t table_index = 1;
    std::vector<ICDElement> word_elements;
    std::vector<std::vector<std::vector<ICDElement>>> element_table;

    ASSERT_FALSE(org.AddToElementTable(table_index, word_elements, element_table));
}


TEST_F(Organize429ICDTest, AddToElementTableInvalidInputIndex)
{
    // table index doesn't exist and isn't the position found if adding to back of element_table
    size_t table_index = 3;
    std::vector<ICDElement> word_elements;
    std::vector<std::vector<std::vector<ICDElement>>> element_table;

    SetupElement();
    word_elements.push_back(expected_element);

    std::vector<std::vector<ICDElement>> temp_vec;
    temp_vec.push_back(word_elements);

    element_table.push_back(temp_vec);

    ASSERT_FALSE(org.AddToElementTable(table_index, word_elements, element_table));
}

// validate output add to new index
TEST_F(Organize429ICDTest, AddToElementTableValidateOutputAddToNewIndex)
{
    // check varify output if adding to new position
    size_t table_index = 1;
    std::vector<ICDElement> word_elements;
    std::vector<std::vector<std::vector<ICDElement>>> element_table;

    SetupElement();
    word_elements.push_back(expected_element);

    std::vector<std::vector<ICDElement>> temp_vec;
    temp_vec.push_back(word_elements);
    element_table.push_back(temp_vec);

    ASSERT_TRUE(org.AddToElementTable(table_index, word_elements, element_table));

    // check size element_table
    EXPECT_EQ(element_table.size(),2);

    // check subvector size in element_table
    EXPECT_EQ(element_table[1].size(), 1);
}

// validate output add to existing position
TEST_F(Organize429ICDTest, AddToElementTableValidateOutputAddToExistingIndex)
{
    // check varify output if adding to new position
    size_t table_index = 0;
    std::vector<ICDElement> word_elements;
    std::vector<std::vector<std::vector<ICDElement>>> element_table;

    SetupElement();
    word_elements.push_back(expected_element);

    std::vector<std::vector<ICDElement>> temp_vec;
    temp_vec.push_back(word_elements);
    element_table.push_back(temp_vec);

    // add such that there are 2 vector<ICDElements> in vector at indexed position
    ASSERT_TRUE(org.AddToElementTable(table_index, word_elements, element_table));
    // check size element_table
    EXPECT_EQ(element_table.size(),1);
    // check subvector size in element_table
    EXPECT_EQ(element_table[0].size(), 2);

    // add such that there are 3 vector<ICDElements> in vector at indexed position
    EXPECT_TRUE(org.AddToElementTable(table_index, word_elements, element_table));
    // check size element_table
    EXPECT_EQ(element_table.size(),1);
    // check subvector size in element_table
    EXPECT_EQ(element_table[0].size(), 3);
}

// // ensure that element added to empty map
// TEST_F(Organize429ICDTest, AddIndexToLookupMapInsertToEmptyMap)
// {
//     // value 0 stored at [1][2][3][4]
//     uint16_t chan_id = 1;
//     uint16_t subchan_id = 2;
//     uint16_t label = 3;
//     int8_t sdi = 4;
//     size_t table_index = 0;
//     std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
//         uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;

//     ASSERT_TRUE(org.AddIndexToLookupMap(chan_id,subchan_id,label,sdi,table_index,organized_lookup_map));

//     EXPECT_TRUE(organized_lookup_map.find(chan_id)!=organized_lookup_map.end());
//     EXPECT_TRUE(organized_lookup_map[chan_id].find(subchan_id)
//         !=organized_lookup_map[chan_id].end());
//     EXPECT_TRUE(organized_lookup_map[chan_id][subchan_id].find(label)
//         !=organized_lookup_map[chan_id][subchan_id].end());
//     EXPECT_TRUE(organized_lookup_map[chan_id][subchan_id][label].find(sdi)
//         !=organized_lookup_map[chan_id][subchan_id][label].end());

//     EXPECT_EQ(organized_lookup_map[chan_id].size(),1);
//     EXPECT_EQ(organized_lookup_map[chan_id][subchan_id].size(),1);
//     EXPECT_EQ(organized_lookup_map[chan_id][subchan_id][label].size(),1);
//     EXPECT_EQ(organized_lookup_map[chan_id][subchan_id][label][sdi],0);
// }

// // ensure that element added to fully built map
// TEST_F(Organize429ICDTest, AddIndexToLookupMapInsertToExistingMap)
// {
//     // value 0 stored at [1][2][3][4]
//     // value 1 stored at [1][2][3][5]
//     uint16_t chan_id = 1;
//     uint16_t subchan_id = 2;
//     uint16_t label = 3;
//     int8_t sdi = 4;
//     size_t table_index = 0;
//     std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
//         uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;

//     ASSERT_TRUE(org.AddIndexToLookupMap(chan_id,subchan_id,label,sdi,table_index,organized_lookup_map));
//     sdi++;
//     table_index++;
//     ASSERT_TRUE(org.AddIndexToLookupMap(chan_id,subchan_id,label,sdi,table_index,organized_lookup_map));

//     EXPECT_TRUE(organized_lookup_map.find(chan_id)!=organized_lookup_map.end());
//     EXPECT_TRUE(organized_lookup_map[chan_id].find(subchan_id)
//         !=organized_lookup_map[chan_id].end());
//     EXPECT_TRUE(organized_lookup_map[chan_id][subchan_id].find(label)
//         !=organized_lookup_map[chan_id][subchan_id].end());
//     EXPECT_TRUE(organized_lookup_map[chan_id][subchan_id][label].find(sdi)
//         !=organized_lookup_map[chan_id][subchan_id][label].end());

//     EXPECT_EQ(organized_lookup_map[chan_id].size(),1);
//     EXPECT_EQ(organized_lookup_map[chan_id][subchan_id].size(),1);
//     EXPECT_EQ(organized_lookup_map[chan_id][subchan_id][label].size(),2);
//     EXPECT_EQ(organized_lookup_map[chan_id][subchan_id][label][sdi],1);
//     EXPECT_EQ(organized_lookup_map[chan_id][subchan_id][label][4],0);
// }

TEST_F(Organize429ICDTest, OrganizeICDMapWordValidateOutputEndToEnd)
{
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node md_chanid_to_subchan_node;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
            uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;
    std::vector<std::vector<std::vector<ICDElement>>> element_table;

    // set up word_elements
    SetupElement();
    std::vector<ICDElement> element_vec;
    element_vec.push_back(expected_element);
    word_elements["TestWord"] = element_vec;
    element_vec.clear();


    // set up md_chanid_to_subchan_node
    BuildNode(md_chan_id_strings, md_chanid_to_subchan_node);

    // build icd_element that will be the "missed subchannel name" below
    ICDElement input_element;
    input_element.label_= 107;
    input_element.sdi_= 1;            // 8-bit
    input_element.bus_name_="MISS1";
    input_element.msg_name_="MissWord";
    input_element.rate_= 1.1F;
    input_element.description_="Altitude";
    input_element.xmit_lru_name_="LRU921";
    input_element.elem_name_="107_alt";
    input_element.schema_=ICDElementSchema::UNSIGNEDBITS;
    input_element.is_bitlevel_=true;
    input_element.bcd_partial_=-1;
    input_element.msb_val_=1.0;
    input_element.bitlsb_= 11;        // 8-bit
    input_element.bit_count_= 8;      // 8-bit
    input_element.uom_="FT";
    input_element.classification_=0;  // 8-bit

    element_vec.push_back(input_element);
    word_elements["MissWord"] = element_vec;

    ASSERT_TRUE(org.OrganizeICDMap(word_elements, md_chanid_to_subchan_node, organized_lookup_map, element_table));

    // check element_table has correct item stored
    EXPECT_EQ(element_table.size(),1);
    EXPECT_EQ(element_table[0].size(),1);
    EXPECT_EQ(element_table[0][0].size(),1);
    EXPECT_EQ(element_table[0][0][0].bus_name_, "SET1B");

    // check organized lookup has correct value stored
    uint16_t chan_id = 34;
    uint16_t subchan_id = 2;
    uint16_t label = 107;
    int8_t sdi = 2;
    EXPECT_TRUE(organized_lookup_map.find(chan_id)!=organized_lookup_map.end());
    EXPECT_TRUE(organized_lookup_map[chan_id].find(subchan_id)
        !=organized_lookup_map[chan_id].end());
    EXPECT_TRUE(organized_lookup_map[chan_id][subchan_id].find(label)
        !=organized_lookup_map[chan_id][subchan_id].end());
    EXPECT_TRUE(organized_lookup_map[chan_id][subchan_id][label].find(sdi)
        !=organized_lookup_map[chan_id][subchan_id][label].end());

    EXPECT_EQ(organized_lookup_map[chan_id].size(),1);
    EXPECT_EQ(organized_lookup_map[chan_id][subchan_id].size(),1);
    EXPECT_EQ(organized_lookup_map[chan_id][subchan_id][label].size(),1);
    EXPECT_EQ(organized_lookup_map[chan_id][subchan_id][label][sdi],0);

    // check for missed subchannel name
    std::vector<std::string> miss_names = org.GetSubchannelNameLookupMisses();
    EXPECT_EQ(miss_names.size(),1);
    EXPECT_EQ(miss_names[0],"MISS1");
}

TEST_F(Organize429ICDTest, OrganizeICDMapWordValidateOutputEndToEndBadElement)
{
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node md_chanid_to_subchan_node;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
            uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;
    std::vector<std::vector<std::vector<ICDElement>>> element_table;

    // set up word_elements
    SetupElement();
    std::vector<ICDElement> element_vec;
    element_vec.clear();


    // set up md_chanid_to_subchan_node
    BuildNode(md_chan_id_strings, md_chanid_to_subchan_node);

    // build icd_element that will have invalid sdi
    ICDElement input_element;
    input_element.label_= 107;
    input_element.sdi_= 5;            // 8-bit
    input_element.bus_name_="MISS1";
    input_element.msg_name_="MissWord";
    input_element.rate_= 1.1F;
    input_element.description_="Altitude";
    input_element.xmit_lru_name_="LRU921";
    input_element.elem_name_="107_alt";
    input_element.schema_=ICDElementSchema::UNSIGNEDBITS;
    input_element.is_bitlevel_=true;
    input_element.bcd_partial_=-1;
    input_element.msb_val_=1.0;
    input_element.bitlsb_= 11;        // 8-bit
    input_element.bit_count_= 8;      // 8-bit
    input_element.uom_="FT";
    input_element.classification_=0;  // 8-bit

    element_vec.push_back(input_element);
    word_elements["TestWord"] = element_vec;

    ASSERT_TRUE(org.OrganizeICDMap(word_elements, md_chanid_to_subchan_node, organized_lookup_map, element_table));

    // check element_table has no item stored because
    // GetICDElementComponents was false and hit continue
    // OrganizeICDMap
    EXPECT_EQ(element_table.size(),0);

}

TEST_F(Organize429ICDTest, OrganizeICDMapWordValidateOutputEndToEndBadSubchannelMap)
{
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node md_chanid_to_subchan_node;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
            uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;
    std::vector<std::vector<std::vector<ICDElement>>> element_table;

    // set up word_elements
    SetupElement();
    std::vector<ICDElement> element_vec;
    element_vec.push_back(expected_element);
    word_elements["TestWord"] = element_vec;
    element_vec.clear();

    std::vector<std::string> bad_chanid_md_strings =
        {"tmats_chanid_to_429_subchan_and_name:",
        "    34: \'FaultyBus\' "};

    // set up md_chanid_to_subchan_node
    BuildNode(bad_chanid_md_strings, md_chanid_to_subchan_node);

    ASSERT_FALSE(org.OrganizeICDMap(word_elements, md_chanid_to_subchan_node, organized_lookup_map, element_table));

    // check element_table has no item stored because
    // GetICDElementComponents was false and hit continue
    // OrganizeICDMap
    EXPECT_EQ(element_table.size(),0);

}

TEST_F(Organize429ICDTest, OrganizeICDMapWordValidateOutputArincWordNames)
{
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node md_chanid_to_subchan_node;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
            uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;
    std::vector<std::vector<std::vector<ICDElement>>> element_table;
    std::unordered_map<size_t,std::vector<std::string>> arinc_word_names;

    // set up word_elements
    SetupElement();
    std::vector<ICDElement> element_vec;
    element_vec.push_back(expected_element);
    word_elements["TestWord"] = element_vec;
    element_vec.clear();


    // set up md_chanid_to_subchan_node
    BuildNode(md_chan_id_strings, md_chanid_to_subchan_node);

    // build icd_element that will be the "missed subchannel name" below
    ICDElement input_element;
    input_element.label_= 107;
    input_element.sdi_= 1;            // 8-bit
    input_element.bus_name_="MISS1";
    input_element.msg_name_="MissWord";
    input_element.rate_= 1.1F;
    input_element.description_="Altitude";
    input_element.xmit_lru_name_="LRU921";
    input_element.elem_name_="107_alt";
    input_element.schema_=ICDElementSchema::UNSIGNEDBITS;
    input_element.is_bitlevel_=true;
    input_element.bcd_partial_=-1;
    input_element.msb_val_=1.0;
    input_element.bitlsb_= 11;        // 8-bit
    input_element.bit_count_= 8;      // 8-bit
    input_element.uom_="FT";
    input_element.classification_=0;  // 8-bit

    element_vec.push_back(input_element);
    word_elements["MissWord"] = element_vec;

    ASSERT_TRUE(org.OrganizeICDMap(word_elements, md_chanid_to_subchan_node, organized_lookup_map, element_table));

    // check element_table has correct item stored
    EXPECT_EQ(element_table.size(),1);
    EXPECT_EQ(element_table[0].size(),1);
    EXPECT_EQ(element_table[0][0].size(),1);
    EXPECT_EQ(element_table[0][0][0].bus_name_, "SET1B");

    // check for ARINC 429 word name in
    arinc_word_names = org.GetArincWordNames();
    EXPECT_EQ(arinc_word_names.size(),1);
    EXPECT_EQ(arinc_word_names[0].size(),1);
    EXPECT_EQ(arinc_word_names[0][0],"TestWord");

}

TEST_F(Organize429ICDTest, OrganizeICDMapIndexInLookupMap)
{
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node md_chanid_to_subchan_node;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
            uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map;
    std::vector<std::vector<std::vector<ICDElement>>> element_table;
    std::unordered_map<size_t,std::vector<std::string>> arinc_word_names;

    // set up word_elements
    SetupElement();
    std::vector<ICDElement> element_vec;
    element_vec.push_back(expected_element);
    word_elements["TestWord"] = element_vec;
    element_vec.clear();


    // set up md_chanid_to_subchan_node
    BuildNode(md_chan_id_strings, md_chanid_to_subchan_node);

    // build icd_element that will be the "missed subchannel name" below
    ICDElement input_element;
    input_element.label_= 107;
    input_element.sdi_= 2;            // 8-bit
    input_element.bus_name_="SET1B";
    input_element.msg_name_="IndexInMapWord";
    input_element.rate_= 1.1F;
    input_element.description_="Altitude";
    input_element.xmit_lru_name_="LRU921";
    input_element.elem_name_="107_alt";
    input_element.schema_=ICDElementSchema::UNSIGNEDBITS;
    input_element.is_bitlevel_=true;
    input_element.bcd_partial_=-1;
    input_element.msb_val_=1.0;
    input_element.bitlsb_= 11;        // 8-bit
    input_element.bit_count_= 8;      // 8-bit
    input_element.uom_="FT";
    input_element.classification_=0;  // 8-bit

    element_vec.push_back(input_element);

    word_elements["OtherTestWord"] = element_vec;
    // word_elements["MissWord"] = element_vec;

    ASSERT_TRUE(org.OrganizeICDMap(word_elements, md_chanid_to_subchan_node, organized_lookup_map, element_table));

    // check element_table correct number of elements
    size_t valid_arinc_word_count = org.GetValidArincWordCount();
    EXPECT_EQ(valid_arinc_word_count, 2);

}