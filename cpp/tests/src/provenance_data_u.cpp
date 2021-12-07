#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "provenance_data.h"

TEST(ProvenanceDataTest, GetGMTString)
{
    time_t rawtime;
    struct tm* tm_ptr;
    time(&rawtime);
    tm_ptr = gmtime(&rawtime);
    std::string strftime_fmt("%F");

    std::stringstream ss;
    ss.width(4);
    ss.fill('0');
    ss << (1900 + tm_ptr->tm_year);
    ss << "-";
    ss.width(2);
    ss << (tm_ptr->tm_mon + 1);
    ss << "-";
    ss.width(2);
    ss << tm_ptr->tm_mday;

    EXPECT_EQ(ss.str(), GetGMTString(strftime_fmt));
}
