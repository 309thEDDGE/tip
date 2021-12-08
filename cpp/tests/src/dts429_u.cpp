#include "gtest/gtest.h"
#include "gmock/gmock.h"
// #include "dts429.h"


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

TEST(DTS429Test, NonNewlineTerminatedLinesVector)
{
    DTS429 dts;
    std::map<std::string, std::string> wrd_name_substitutions;
    std::map<std::string, std::string> elem_name_substitutions;

    EXPECT_FALSE(dts.IngestLines(yaml_lines_1, elem_name_substitutions, wrd_name_substitutions));

}


TEST(DTS429Test, IngestLines)
{
    DTS429 dts;
    std::map<std::string, std::string> wrd_name_substitutions;
    std::map<std::string, std::string> elem_name_substitutions;

    EXPECT_TRUE(dts.IngestLines(yaml_lines_0, elem_name_substitutions, wrd_name_substitutions));

    EXPECT_FALSE(true);
}



TEST(DTS429Test, YamlParsedCorrectly)
{

    EXPECT_FALSE(true);
}

TEST(DTS429Test, StoreParsedInMap)
{

    EXPECT_FALSE(true);
}
