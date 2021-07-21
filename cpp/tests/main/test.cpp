#include "gmock/gmock.h"
#include "logger_setup.h"

int main(int argc, char** argv)
{
    EnsureLoggersAreAvailable();

    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
