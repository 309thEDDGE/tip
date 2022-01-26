#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "provenance_data.h"

TEST(ProvenanceDataTest, GetGMTString)
{
    time_t rawtime;
    struct tm* tm_ptr = nullptr;
    time(&rawtime);
#if defined __WIN64
    struct tm tm_data;
    tm_ptr = &tm_data;
    gmtime_s(tm_ptr, &rawtime);
#else
    tm_ptr = gmtime(&rawtime);
#endif
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
