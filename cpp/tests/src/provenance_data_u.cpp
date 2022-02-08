#include <fstream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "managed_path.h"
#include "version_info.h"
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

TEST(ProvenanceDataTest, GetProvenanceData)
{
    // Create temp output file
    ManagedPath temp_file;
    temp_file /= "temp_hash_file";

    std::ofstream outf(temp_file.string());
    ASSERT_TRUE(outf.is_open() && outf.good());

    outf << "write this stuff in to my hash test file";
    outf.close();

    ASSERT_TRUE(temp_file.is_regular_file());

    std::string expected_time = GetGMTString("%F %T");
    expected_time = expected_time.substr(0, expected_time.find(" "));
    printf("expected_time: %s\n", expected_time.c_str());
    std::string exp_hash = "1be959da502e848018feb680121bcbc8de40ed792fd97387b537ad1a1adc3e78";
    std::string exp_version = GetVersionString();

    ProvenanceData prov_data;
    ASSERT_TRUE(GetProvenanceData(temp_file, 0, prov_data));
    std::string obs_time = prov_data.time.substr(0, prov_data.time.find(" "));

    EXPECT_EQ(expected_time, obs_time);
    EXPECT_EQ(exp_hash, prov_data.hash);
    EXPECT_EQ(exp_version, prov_data.tip_version);
    temp_file.remove();
}

TEST(ProvenanceDataTest, GetProvenanceDataFileNotExist)
{
    // Create temp output file
    ManagedPath temp_file;
    temp_file /= "temp_hash_file";
    ASSERT_FALSE(temp_file.is_regular_file());

    ProvenanceData prov_data;
    EXPECT_FALSE(GetProvenanceData(temp_file, 0, prov_data));
}

TEST(ProvenanceDataTest, ClassInstantiation)
{
    ProvenanceData prov_data;
    EXPECT_EQ("", prov_data.tip_version);
    EXPECT_EQ("", prov_data.hash);
    EXPECT_EQ("", prov_data.time);
}
