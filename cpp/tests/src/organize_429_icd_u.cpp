#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "organize_429_icd.h"

class Organize429ICDTest : public ::testing::Test
{
   protected:
    std::vector<std::string> md_chan_id_node =
        {"tmats_chanid_to_429_subchan_and_name:",
        "    34: {1: SET1A, 2: SET1B, 3: SET2A, 4: SET2B}",
        "    35: {1: SET3A}",
        "    36: {1: SET3B}",
        "    37: {1: SET4A}",
        "    38: {1: SET4B}",
        "    39: {1: ARR-1 Channel}"};

    // builds yaml node from one of yaml_lines_n above
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
    build_node(md_chan_id_node, md_chanid_to_subchan_node);

    EXPECT_FALSE(icd_org.OrganizeICDMap(word_elements, md_chanid_to_subchan_node, organized_output_map));
}
