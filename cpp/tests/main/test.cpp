
#include "gmock/gmock.h"
#include "../src/spdlog_setup_helper_funcs.h"

int main(int argc, char** argv) {

    //Create a logger with the name used in ParseManager.
    std::string logger_name = "pm_logger";
    CreateNullLoggerWithName(logger_name);

    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}