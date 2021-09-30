#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "yamlsv_log_item.h"


TEST(YamlSVLogItemTest, PrintToStreamEmptyStream)
{
    LogLevel level = LogLevel::Info;
    std::string msg = "this is my message";
    LogItem item(level, msg);
    std::stringstream stream;
    item.PrintToStream(stream);

    std::string expected = "[INFO ] " + msg + "\n"; 
    EXPECT_EQ(expected, stream.str());
}

TEST(YamlSVLogItemTest, PrintToStreamInitializedStream)
{
    LogLevel level = LogLevel::Info;
    std::string msg = "this is my message";
    LogItem item(level, msg);

    // Initialize stream
    std::stringstream stream;
    stream << "data: ";

    item.PrintToStream(stream);

    std::string expected = "data: [INFO ] " + msg + "\n"; 
    EXPECT_EQ(expected, stream.str());
}