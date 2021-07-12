
#include <gtest/gtest.h>
#include "tmats_parser.h"

const std::string TMATS =
    R"(G\TA:PIT_1(14-002);)"
    "\n"
    R"(G\106:07;)"
    "\n"
    R"(G\DSI\N:1;)"
    "\n"
    R"(G\DSI-1:DATASOURCE;)"
    "\n"
    R"(G\DST-1:OTH;)"
    "\n"
    R"(G\OD:05/15/2015;)"
    "\n"
    R"(G\UD:05/15/2015;)"
    "\n"
    R"(G\POC\N:1;)"
    "\n"
    R"(G\POC1-1:ILIAD HeimRecorderGen;)"
    "\n"
    R"(G\COM:Generated by ILIAD Unit Generator build 802.12.0.284 on 2015/05/15 11:35:20;)"
    "\n"
    R"(R-1\ID:DATASOURCE;)"
    "\n"
    R"(R-1\TC1:SSR;)"
    "\n"
    R"(R-1\RI1:Heim;)"
    "\n"
    R"(R-1\RI2:D200F;)"
    "\n"
    R"(R-1\RI3:Y;)"
    "\n"
    R"(R-1\N:28;)"
    "\n"
    R"(R-1\EV\E:F;)"
    "\n"
    R"(R-1\IDX\E:F;)"
    "\n"
    R"(R-1\RID:HeimSystems;)"
    "\n"
    R"(R-1\NSB:4;)"
    "\n"
    R"(V-1\ID:DATASOURCE;)"
    "\n"
    R"(V-1\VN:HDS;)"
    "\n"
    R"(R-1\DSI-1:Time;)"
    "\n"
    R"(R-1\TK1-1:1;)"
    "\n"
    R"(R-1\CHE-1:T;)"
    "\n"
    R"(R-1\CDT-1:TIMEIN;)"
    "\n"
    R"(R-1\TK4-1:1;)"
    "\n"
    R"(R-1\TTF-1:1;)"
    "\n"
    R"(R-1\CDLN-1:TIMECHANNEL;)"
    "\n"
    R"(R-1\TFMT-1:B;)"
    "\n"
    R"(R-1\TSRC-1:E;)"
    "\n"
    R"(V-1\HDS\SYS:sY1a-;)"
    "\n"
    R"(V-1\HDS\SYS:solRmRa5b5e3hNiBk1pAr0u-n+sBy-z6;)"
    "\n"
    R"(V-1\HDS\SYS:sco3;)"
    "\n"
    R"(V-1\HDS\SYS:sfh5o5r-t-;)"
    "\n"
    R"(V-1\HDS\SYS:ssb25000c+e-g-hNiIkIn0q2a0;)"
    "\n"
    R"(V-1\HDS\SYS:ssr38400;)"
    "\n"
    R"(V-1\HDS\SYS:sam;)"
    "\n"
    R"(R-1\DSI-2:UAR-1;)"
    "\n"
    R"(R-1\TK1-2:2;)"
    "\n"
    R"(R-1\CHE-2:T;)"
    "\n"
    R"(R-1\CDT-2:1553IN;)"
    "\n"
    R"(R-1\TK4-2:2;)"
    "\n"
    R"(R-1\CDLN-2:UAR-1;)"
    "\n"
    R"(R-1\BTF-2:1;)"
    "\n"
    R"(B-2\DLN:UAR-1;)"
    "\n"
    R"(B-2\NBS\N:1;)"
    "\n"
    R"(B-2\BID-1:00000000;)"
    "\n"
    R"(B-2\BNA-1:F1;)"
    "\n"
    R"(B-2\BT-1:1553;)"
    "\n"
    R"(V-1\HDS\SYS:sB1cTdTWHw20x0v0t+u+;)"
    "\n"
    R"(R-1\DSI-3:UAR-2;)"
    "\n"
    R"(R-1\TK1-3:3;)"
    "\n"
    R"(R-1\CHE-3:T;)"
    "\n"
    R"(R-1\CDT-3:1553IN;)"
    "\n"
    R"(R-1\TK4-3:3;)"
    "\n"
    R"(R-1\CDLN-3:UAR-2;)"
    "\n"
    R"(R-1\BTF-3:1;)"
    "\n"
    R"(B-3\DLN:UAR-2;)"
    "\n"
    R"(B-3\NBS\N:1;)"
    "\n"
    R"(R-1\ASN-18-6:6;)"
    "\n"
    R"(R-1\ANM-18-6:EEC3B;)"
    "\n"
    R"(V-1\HDS\SYS:sR6WH;)"
    "\n"
    R"(R-1\ASN-18-7:7;)"
    "\n"
    R"(R-1\ANM-18-7:EEC4A;)"
    "\n"
    R"(V-1\HDS\SYS:sR7WH;)"
    "\n"
    R"(R-1\ASN-18-8:8;)"
    "\n"
    R"(R-1\ANM-18-8:EEC4B;)"
    "\n"
    R"(V-1\HDS\SYS:sR8WH;)"
    "\n"
    R"(R-1\DSI-19:ARR40-1-2;)"
    "\n"
    R"(R-1\TK1-19:19;)"
    "\n"
    R"(R-1\CHE-19:T;)"
    "\n"
    R"(R-1\CDT-19:429IN;)"
    "\n"
    R"(R-1\TK4-19:19;)"
    "\n"
    R"(R-1\CDLN-19:ARR40-1-2;)"
    "\n";

TEST(CodeNameTest, RegexCorrect)
{
    CodeName c = CodeName(R"(R-x\DSI-n)");

    EXPECT_EQ("R-([0-9]+)\\\\DSI-([0-9]+)", c.regex_string);
}

TEST(TMATSParserTest, MapAttrsMatchesGroups)
{
    std::map<std::string, std::string> expected;
    std::map<std::string, std::string> result;
    expected["ARR40-1-2"] = "19";
    expected["Time"] = "1";
    expected["UAR-1"] = "2";
    expected["UAR-2"] = "3";

    TMATSParser parser = TMATSParser(TMATS);
    result = parser.MapAttrs("R-x\\DSI-n", "R-x\\TK1-n");

    EXPECT_EQ(expected, result);
}

TEST(TMATSParserTest, MapAttrsSkipsMismatch)
{
    std::map<std::string, std::string> expected;
    std::map<std::string, std::string> result;
    expected["ARR40-1-2"] = "19";
    expected["Time"] = "1";
    expected["UAR-1"] = "2";
    expected["UAR-2"] = "3";

    TMATSParser parser = TMATSParser(TMATS);
    result = parser.MapAttrs("R-x\\DSI-n", "R-x\\TK1-n");

    EXPECT_EQ(expected, result);
}

TEST(TMATSParserTest, MapAttrs3Subattrs)
{
    std::map<std::string, std::string> expected;
    std::map<std::string, std::string> result;
    expected["6"] = "EEC3B";
    expected["7"] = "EEC4A";
    expected["8"] = "EEC4B";

    TMATSParser parser = TMATSParser(TMATS);
    result = parser.MapAttrs("R-x\\ASN-n-m", "R-x\\ANM-n-m");

    EXPECT_EQ(expected, result);
}

TEST(TMATSParserTest, MapAttrsHandleRepeatData)
{
    // Note about creating test lines from a tmats text file:
    // 1) Read in with Python via with open(...), a = f.readlines();
    // 2) b = ['R\"(' + x.rstrip('\n') + ')\" \"\\r\\n\"\n' for x in a]
    // 3) Write to temp output file via with open(...), f.writelines(b)
    // 4) paste into C++ file
    //
    std::string custom_tmats =
        R"(G\COM:********************** Time Channel ************************;)"
        "\r\n"
        R"(G\COM:************************************************************;)"
        "\r\n"
        R"(R-1\CDT-1:TIMEIN;)"
        "\r\n"
        R"(R-1\TK1-1:1;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(G\COM:************************************************************;)"
        "\r\n"
        R"(G\COM:********************* Video Channels ***********************;)"
        "\r\n"
        R"(G\COM:************************************************************;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(R-1\DSI-2:VIDEO_1;)"
        "\r\n"
        R"(R-1\CDT-2:VIDIN;)"
        "\r\n"
        R"(R-1\TK1-2:2;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(R-1\DSI-3:VIDEO_2;)"
        "\r\n"
        R"(R-1\CDT-3:VIDIN;)"
        "\r\n"
        R"(R-1\TK1-3:3;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(R-1\DSI-4:VIDEO_3;)"
        "\r\n"
        R"(R-1\CDT-4:VIDIN;)"
        "\r\n"
        R"(R-1\TK1-4:4;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(R-1\DSI-5:VIDEO_4;)"
        "\r\n"
        R"(R-1\CDT-5:VIDIN;)"
        "\r\n"
        R"(R-1\TK1-5:5;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(G\COM:************************************************************;)"
        "\r\n"
        R"(G\COM:***************** MIL-STD-1553B Channels *******************;)"
        "\r\n"
        R"(G\COM:************************************************************;)"
        "\r\n"
        R"(R-1\DSI-6:1553_1;)"
        "\r\n"
        R"(R-1\CDT-6:1553IN;)"
        "\r\n"
        R"(R-1\TK1-6:6;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(R-1\DSI-7:1553_2;)"
        "\r\n"
        R"(R-1\CDT-7:1553IN;)"
        "\r\n"
        R"(R-1\TK1-7:7;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(R-1\DSI-8:1553_3;)"
        "\r\n"
        R"(R-1\CDT-8:1553IN;)"
        "\r\n"
        R"(R-1\TK1-8:8;)"
        "\r\n"
        R"()"
        "\r\n"
        R"(R-1\DSI-9:1553_4;)"
        "\r\n"
        R"(R-1\CDT-9:1553IN;)"
        "\r\n"
        R"(R-1\TK1-9:9;)"
        "\r\n";

    std::map<std::string, std::string> expected = {
        {"1", "TIMEIN"},
        {"2", "VIDIN"},
        {"3", "VIDIN"},
        {"4", "VIDIN"},
        {"5", "VIDIN"},
        {"6", "1553IN"},
        {"7", "1553IN"},
        {"8", "1553IN"},
        {"9", "1553IN"}};
    std::map<std::string, std::string> result;

    TMATSParser parser = TMATSParser(custom_tmats, false);
    result = parser.MapAttrs("R-x\\TK1-n", "R-x\\CDT-n");
    EXPECT_EQ(expected, result);
}
