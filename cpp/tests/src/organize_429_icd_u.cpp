#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "organize_429_icd.h"
#include "icd_element.h"

class Organize429ICDTest : public ::testing::Test
{
   protected:
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
    }
   public:
    Organize429ICDTest(){}

};


TEST_F(Organize429ICDTest, OrganizeICDMapWordElementsEmpty)
{
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node md_chanid_to_subchan_node;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
            uint16_t,std::unordered_map<int8_t, std::vector<ICDElement>>>>> organized_output_map;

    Organize429ICD icd_org;
    BuildNode(md_chan_id_strings, md_chanid_to_subchan_node);

    EXPECT_FALSE(icd_org.OrganizeICDMap(word_elements, md_chanid_to_subchan_node, organized_output_map));
}


TEST_F(Organize429ICDTest, OrganizeICDMapYAMLNodeNull)
{
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node md_chanid_to_subchan_node;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
            uint16_t,std::unordered_map<int8_t, std::vector<ICDElement>>>>> organized_output_map;
    std::vector<ICDElement> element_vec;

    Organize429ICD icd_org;
    SetupElement();
    element_vec.push_back(expected_element);
    word_elements["TestWord"] = element_vec;

    EXPECT_FALSE(icd_org.OrganizeICDMap(word_elements, md_chanid_to_subchan_node, organized_output_map));
}

TEST_F(Organize429ICDTest, OrganizeICDMapYAMLNodeNotMap)
{
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node md_chanid_to_subchan_node;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
            uint16_t,std::unordered_map<int8_t, std::vector<ICDElement>>>>> organized_output_map;
    std::vector<ICDElement> element_vec;

    SetupElement();
    element_vec.push_back(expected_element);
    word_elements["TestWord"] = element_vec;
    YAML::Node tested_node;

    std::vector<std::string> temp_node_input =
        {"'Fail'"};
    Organize429ICD icd_org;
    BuildNode(temp_node_input, md_chanid_to_subchan_node);

    EXPECT_FALSE(icd_org.OrganizeICDMap(word_elements, md_chanid_to_subchan_node, organized_output_map));
}

TEST_F(Organize429ICDTest, ValidateInputsWordElementsEmpty)
{
    std::unordered_map<std::string, std::vector<ICDElement>> word_elements;
    YAML::Node md_chanid_to_subchan_node;

    Organize429ICD icd_org;
    BuildNode(md_chan_id_strings, md_chanid_to_subchan_node);

    EXPECT_FALSE(icd_org.ValidateInputs(word_elements, md_chanid_to_subchan_node));
}


TEST_F(Organize429ICDTest, OValidateInputsYAMLNodeNull)
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
    YAML::Node tested_node;

    std::vector<std::string> temp_node_input =
        {"'Fail'"};
    Organize429ICD icd_org;
    BuildNode(temp_node_input, md_chanid_to_subchan_node);

    EXPECT_FALSE(icd_org.ValidateInputs(word_elements, md_chanid_to_subchan_node));
}