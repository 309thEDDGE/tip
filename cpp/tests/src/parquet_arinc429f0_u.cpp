#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parquet_arinc429f0.h"


TEST(ParquetARINC429F0Test, EncodeARINC429Label)
{
    ParquetARINC429F0 parquet429;

    uint32_t raw_label = 83;

    // Expect 83 dec => 312 oct
    EXPECT_EQ(parquet429.EncodeARINC429Label(raw_label), 312);

    raw_label = 154;
    // Expect 154 dec => 131 oct
    EXPECT_EQ(parquet429.EncodeARINC429Label(raw_label), 131);

    raw_label = 153;
    // Expect 153 dec == 231 oct
    EXPECT_EQ(parquet429.EncodeARINC429Label(raw_label), 231);
}
