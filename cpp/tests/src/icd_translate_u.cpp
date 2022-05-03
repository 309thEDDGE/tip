#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "icd_translate.h"

class ICDTranslateTest : public ::testing::Test
{
   protected:
    ICDTranslate icdt_;
    ICDElement icde_;
    std::vector<uint16_t> input_words_;
    std::vector<int8_t> output_eu_i8_;
    std::vector<uint8_t> output_eu_ui8_;
    std::vector<float> output_eu_float_;
    std::vector<double> output_eu_double_;
    size_t n_values;
    uint16_t dshift1, dshift2, mask1, mask2,
        mask1_twos, mask2_twos, sign_bit_mask;
    double scale, scale_twos;
    ICDTranslateTest()
    {
    }
    void SetUp() override
    {
    }

    void FillParams()
    {
        icdt_.GetConfigParams(n_values, dshift1, dshift2, mask1,
                              mask2, mask1_twos, mask2_twos, scale, scale_twos, sign_bit_mask);
    }

    void PrintParams()
    {
        printf(
            "n_values %zu, dshift1 %hu, dshift2 %hu, mask1 %hu, mask2 %hu, mask1_twos %hu, "
            "mask2_twos %hu, scale %f, scale_twos %f, sign_bit_mask %hu\n",
            n_values, dshift1, dshift2, mask1,
            mask2, mask1_twos, mask2_twos, scale, scale_twos, sign_bit_mask);
    }

    double LimitSigFigs(double val)
    {
        double output = 0.0;
        int64_t sigfigs = 4;
        double shift = double(sigfigs - int64_t(floor(log10(abs(val)) + 1)));
        output = double(int64_t(val * pow(10.0, shift))) / pow(10, shift);
        return output;
    }
};

TEST_F(ICDTranslateTest, TranslateArrayOfElementUninitializedICDElem)
{
    // ICD Element (icde_) is uninitialized.
    ASSERT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_));
}

TEST_F(ICDTranslateTest, TranslateArrayOfElementInvalidType)
{
    // Setup valid fields.
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.elem_word_count_ = 3;
    icde_.channel_id_ = 10;
    icde_.is_bitlevel_ = true;

    // Test each type.
    icde_.schema_ = ICDElementSchema::ASCII;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_ui8_, icde_));

    icde_.schema_ = ICDElementSchema::SIGNEDBITS;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_ui8_, icde_));
    EXPECT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_));
    EXPECT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));

    icde_.schema_ = ICDElementSchema::UNSIGNEDBITS;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_));
    EXPECT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_));
    EXPECT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    EXPECT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_ui8_, icde_));

    // undefined
    icde_.schema_ = ICDElementSchema::MULTIPLE_BIT;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_));

    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::MODE_CODE;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_));

    icde_.schema_ = ICDElementSchema::SIGNED16;
    icde_.elem_word_count_ = 1;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_));
    EXPECT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_));
    icde_.elem_word_count_ = 2;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_));

    icde_.schema_ = ICDElementSchema::SIGNED32;
    icde_.elem_word_count_ = 2;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_));
    EXPECT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    icde_.elem_word_count_ = 1;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));

    icde_.schema_ = ICDElementSchema::UNSIGNED16;
    icde_.elem_word_count_ = 1;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_));
    EXPECT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_));
    icde_.elem_word_count_ = 2;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_));

    icde_.schema_ = ICDElementSchema::UNSIGNED32;
    icde_.elem_word_count_ = 2;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_));
    EXPECT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    icde_.elem_word_count_ = 3;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));

    icde_.schema_ = ICDElementSchema::FLOAT32_1750;
    icde_.elem_word_count_ = 2;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_));
    EXPECT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_));
    icde_.elem_word_count_ = 3;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_));

    icde_.schema_ = ICDElementSchema::FLOAT32_IEEE;
    icde_.elem_word_count_ = 2;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_));
    EXPECT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_));
    icde_.elem_word_count_ = 3;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_));

    icde_.schema_ = ICDElementSchema::FLOAT64_IEEE;
    icde_.elem_word_count_ = 4;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_));
    EXPECT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    icde_.elem_word_count_ = 3;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));

    icde_.schema_ = ICDElementSchema::FLOAT16;
    icde_.elem_word_count_ = 1;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_));
    EXPECT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_));
    icde_.elem_word_count_ = 3;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_));

    icde_.schema_ = ICDElementSchema::CAPS;
    icde_.elem_word_count_ = 3;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_));
    EXPECT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    icde_.elem_word_count_ = 4;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));

    icde_.schema_ = ICDElementSchema::FLOAT32_GPS;
    icde_.elem_word_count_ = 2;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_));
    EXPECT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_));
    icde_.elem_word_count_ = 3;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_));

    icde_.schema_ = ICDElementSchema::FLOAT64_GPS;
    icde_.elem_word_count_ = 4;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_));
    EXPECT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    icde_.elem_word_count_ = 3;
    EXPECT_FALSE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
}

TEST_F(ICDTranslateTest, ConfigureBitLevelOneWordUnsignedSingleBit)
{
    // Setup ICDElement, single bit.
    icde_.is_bitlevel_ = true;
    icde_.bitlsb_ = 3;
    icde_.bitmsb_ = 3;
    icde_.schema_ = ICDElementSchema::UNSIGNEDBITS;
    icde_.bit_count_ = 1;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = 1.0;

    // Setup input words.
    input_words_.push_back(30);
    input_words_.push_back(40);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureBitLevel(input_words_.size(), output_eu_i8_, icde_);
    FillParams();

    //uint16_t dshift1, dshift2, mask1, mask2,
    //mask1_twos, mask2_twos, scale, scale_twos;
    EXPECT_EQ(n_values, 2);
    EXPECT_EQ(dshift1, 13);
    EXPECT_EQ(mask1, 1);
    EXPECT_EQ(scale, 1.0);
}

TEST_F(ICDTranslateTest, ConfigureBitLevelOneWordUnsignedMultBit)
{
    // Setup ICDElement, single bit.
    icde_.is_bitlevel_ = true;
    icde_.bitlsb_ = 10;
    icde_.bitmsb_ = 4;
    icde_.schema_ = ICDElementSchema::UNSIGNEDBITS;
    icde_.bit_count_ = 7;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = 1324.2;

    // Setup input words.
    input_words_.push_back(30);
    input_words_.push_back(40);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureBitLevel(input_words_.size(), output_eu_i8_, icde_);
    FillParams();

    //uint16_t dshift1, dshift2, mask1, mask2,
    //mask1_twos, mask2_twos, scale, scale_twos;
    EXPECT_EQ(n_values, 2);
    EXPECT_EQ(dshift1, 6);
    EXPECT_EQ(mask1, 127);
    EXPECT_EQ(LimitSigFigs(scale), LimitSigFigs(20.6906));
}

TEST_F(ICDTranslateTest, ConfigureBitLevelOneWordSignedMultBit)
{
    // Setup ICDElement, single bit.
    icde_.is_bitlevel_ = true;
    icde_.bitlsb_ = 9;
    icde_.bitmsb_ = 2;
    icde_.schema_ = ICDElementSchema::SIGNEDBITS;
    icde_.bit_count_ = 8;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = 500.0;

    // Setup input words.
    input_words_.push_back(30);
    input_words_.push_back(40);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureBitLevel(input_words_.size(), output_eu_i8_, icde_);
    FillParams();

    //uint16_t dshift1, dshift2, mask1, mask2,
    //mask1_twos, mask2_twos, scale, scale_twos;
    EXPECT_EQ(n_values, 2);
    EXPECT_EQ(dshift1, 7);
    EXPECT_EQ(mask1_twos, 127);
    EXPECT_EQ(LimitSigFigs(scale_twos), LimitSigFigs(7.8125));
}

TEST_F(ICDTranslateTest, ConfigureBitLevelTwoWordUnsigned)
{
    // Setup ICDElement, single bit.
    icde_.is_bitlevel_ = true;
    icde_.bitlsb_ = 5;
    icde_.bitmsb_ = 3;
    icde_.schema_ = ICDElementSchema::UNSIGNEDBITS;
    icde_.bit_count_ = 19;
    icde_.elem_word_count_ = 2;
    icde_.msb_val_ = 128.0;

    // Setup input words.
    input_words_.push_back(30);
    input_words_.push_back(40);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureBitLevel(input_words_.size(), output_eu_i8_, icde_);
    FillParams();

    //uint16_t dshift1, dshift2, mask1, mask2,
    //mask1_twos, mask2_twos, scale, scale_twos;
    EXPECT_EQ(n_values, 1);
    EXPECT_EQ(dshift1, 0);
    EXPECT_EQ(dshift2, 11);
    EXPECT_EQ(mask1, 16383);
    EXPECT_EQ(mask2, 31);
    EXPECT_EQ(LimitSigFigs(scale), LimitSigFigs(0.00048828));
}

TEST_F(ICDTranslateTest, ConfigureBitLevelMultWordUnsigned)
{
    // Setup ICDElement, single bit.
    icde_.is_bitlevel_ = true;
    icde_.bitlsb_ = 6;
    icde_.bitmsb_ = 10;
    icde_.schema_ = ICDElementSchema::UNSIGNEDBITS;
    icde_.bit_count_ = 29;
    icde_.elem_word_count_ = 3;
    icde_.msb_val_ = 5120000.0;

    // Setup input words.
    input_words_.push_back(30);
    input_words_.push_back(40);
    input_words_.push_back(50);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureBitLevel(input_words_.size(), output_eu_i8_, icde_);
    FillParams();

    //uint16_t dshift1, dshift2, mask1, mask2,
    //mask1_twos, mask2_twos, scale, scale_twos;
    EXPECT_EQ(n_values, 1);
    EXPECT_EQ(dshift1, 0);
    EXPECT_EQ(dshift2, 10);
    EXPECT_EQ(mask1, 127);
    EXPECT_EQ(mask2, 63);
    EXPECT_EQ(LimitSigFigs(scale), LimitSigFigs(0.01907));
}

TEST_F(ICDTranslateTest, ConfigureBitLevelTwoWordSigned)
{
    // Setup ICDElement, single bit.
    icde_.is_bitlevel_ = true;
    icde_.bitlsb_ = 5;
    icde_.bitmsb_ = 3;
    icde_.schema_ = ICDElementSchema::SIGNEDBITS;
    icde_.bit_count_ = 19;
    icde_.elem_word_count_ = 2;
    icde_.msb_val_ = 128.0;

    // Setup input words.
    input_words_.push_back(30);
    input_words_.push_back(40);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureBitLevel(input_words_.size(), output_eu_i8_, icde_);
    FillParams();

    //uint16_t dshift1, dshift2, mask1, mask2,
    //mask1_twos, mask2_twos, scale, scale_twos;
    EXPECT_EQ(n_values, 1);
    EXPECT_EQ(dshift1, 0);
    EXPECT_EQ(dshift2, 11);
    EXPECT_EQ(mask1_twos, 8191);
    EXPECT_EQ(mask2_twos, 31);
    EXPECT_EQ(LimitSigFigs(scale_twos), LimitSigFigs(0.0009765));
}

TEST_F(ICDTranslateTest, ConfigureBitLevelMultWordSigned)
{
    // Setup ICDElement, single bit.
    icde_.is_bitlevel_ = true;
    icde_.bitlsb_ = 6;
    icde_.bitmsb_ = 10;
    icde_.schema_ = ICDElementSchema::SIGNEDBITS;
    icde_.bit_count_ = 29;
    icde_.elem_word_count_ = 3;
    icde_.msb_val_ = 5120000.0;

    // Setup input words.
    input_words_.push_back(30);
    input_words_.push_back(40);
    input_words_.push_back(50);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureBitLevel(input_words_.size(), output_eu_i8_, icde_);
    FillParams();

    //uint16_t dshift1, dshift2, mask1, mask2,
    //mask1_twos, mask2_twos, scale, scale_twos;
    EXPECT_EQ(n_values, 1);
    EXPECT_EQ(dshift1, 0);
    EXPECT_EQ(dshift2, 10);
    EXPECT_EQ(mask1_twos, 63);
    EXPECT_EQ(mask2_twos, 63);
    EXPECT_EQ(LimitSigFigs(scale_twos), LimitSigFigs(0.03814));
}

TEST_F(ICDTranslateTest, ConfigureWordLevelUS16)
{
    // Setup ICDElement, single bit.
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::UNSIGNED16;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = 1000.0;

    // Setup input words.
    input_words_.push_back(30);
    input_words_.push_back(40);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    FillParams();

    //uint16_t dshift1, dshift2, mask1, mask2,
    //mask1_twos, mask2_twos, scale, scale_twos;
    EXPECT_EQ(n_values, 2);
    EXPECT_EQ(LimitSigFigs(scale), LimitSigFigs(0.03051));
}

TEST_F(ICDTranslateTest, ConfigureWordLevelS16)
{
    // Setup ICDElement, single bit.
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::SIGNED16;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = 1000.0;

    // Setup input words.
    input_words_.push_back(30);
    input_words_.push_back(40);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    FillParams();

    //uint16_t dshift1, dshift2, mask1, mask2,
    //mask1_twos, mask2_twos, scale, scale_twos;
    EXPECT_EQ(n_values, 2);
    EXPECT_EQ(LimitSigFigs(scale_twos), LimitSigFigs(0.06103));
}

TEST_F(ICDTranslateTest, ConfigureWordLevelUS32)
{
    // Setup ICDElement, single bit.
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::UNSIGNED32;
    icde_.elem_word_count_ = 2;
    icde_.msb_val_ = 515000.0;

    // Setup input words.
    input_words_.push_back(30);
    input_words_.push_back(40);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    FillParams();

    //uint16_t dshift1, dshift2, mask1, mask2,
    //mask1_twos, mask2_twos, scale, scale_twos;
    EXPECT_EQ(n_values, 1);
    EXPECT_EQ(LimitSigFigs(scale), LimitSigFigs(0.0002398));
}

TEST_F(ICDTranslateTest, ConfigureWordLevelS32)
{
    // Setup ICDElement, single bit.
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::SIGNED32;
    icde_.elem_word_count_ = 2;
    icde_.msb_val_ = 515000.0;

    // Setup input words.
    input_words_.push_back(30);
    input_words_.push_back(40);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    FillParams();

    //uint16_t dshift1, dshift2, mask1, mask2,
    //mask1_twos, mask2_twos, scale, scale_twos;
    EXPECT_EQ(n_values, 1);
    EXPECT_EQ(LimitSigFigs(scale_twos), LimitSigFigs(0.0004796));
}

TEST_F(ICDTranslateTest, TranslateUnsignedBitsASCII8Bit)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::ASCII;
    icde_.bitlsb_ = 8;
    icde_.bitmsb_ = 1;
    icde_.bit_count_ = 8;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = 0.0;

    // Setup input words.
    input_words_.push_back(8448);
    input_words_.push_back(14336);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureBitLevel(input_words_.size(), output_eu_i8_, icde_);
    icdt_.TranslateUnsignedBits(input_words_, output_eu_i8_);
    EXPECT_THAT(output_eu_i8_, ::testing::ElementsAre(33, 56));

    // Use the formal translate function.
    output_eu_i8_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_);
    EXPECT_TRUE(res);
    EXPECT_THAT(output_eu_i8_, ::testing::ElementsAre(33, 56));
}

TEST_F(ICDTranslateTest, TranslateUnsignedBitsASCII16Bit)
{
    // CI02_UD02 - 0201@06 | 16 | 32768 | NONE | 0 | A | blah blah |
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::ASCII;
    icde_.bitlsb_ = 16;
    icde_.bitmsb_ = 1;
    icde_.bit_count_ = 16;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = 32768.0;

    // Setup input words.
    input_words_.push_back(33);
    input_words_.push_back(56);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureBitLevel(input_words_.size(), output_eu_i8_, icde_);
    icdt_.TranslateUnsignedBits(input_words_, output_eu_i8_);
    EXPECT_THAT(output_eu_i8_, ::testing::ElementsAre(33, 56));

    // Use the formal translate function.
    output_eu_i8_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_i8_, icde_);
    EXPECT_TRUE(res);
    EXPECT_THAT(output_eu_i8_, ::testing::ElementsAre(33, 56));
}

TEST_F(ICDTranslateTest, TranslateUnsignedBitsUNSIGNEDBITSSingleWord)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::UNSIGNEDBITS;
    icde_.bitlsb_ = 7;
    icde_.bitmsb_ = 4;
    icde_.bit_count_ = 4;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = double(1 << 3);

    // Setup input words.
    input_words_.push_back((13 << 9) + 123);  // 13
    input_words_.push_back((42 << 9) + 342);  // 10

    // Run configure function and get calculated parameters.
    icdt_.ConfigureBitLevel(input_words_.size(), output_eu_ui8_, icde_);
    icdt_.TranslateUnsignedBits(input_words_, output_eu_ui8_);
    EXPECT_THAT(output_eu_ui8_, ::testing::ElementsAre(13, 10));

    // Use the formal translate function, uint8
    output_eu_ui8_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_ui8_, icde_);
    EXPECT_TRUE(res);
    EXPECT_THAT(output_eu_ui8_, ::testing::ElementsAre(13, 10));

    // float
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_TRUE(res);
    EXPECT_THAT(output_eu_float_, ::testing::ElementsAre(13, 10));

    // double
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_TRUE(res);
    EXPECT_THAT(output_eu_double_, ::testing::ElementsAre(13, 10));
}

TEST_F(ICDTranslateTest, TranslateUnsignedBitsUNSIGNEDBITSTwoWord)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::UNSIGNEDBITS;
    icde_.bitlsb_ = 12;
    icde_.bitmsb_ = 8;
    icde_.bit_count_ = 21;
    icde_.elem_word_count_ = 2;
    icde_.msb_val_ = double(1 << 20);

    // Setup input words.
    input_words_.push_back(91);   // 372736
    input_words_.push_back(906);  // 56

    // Run configure function and get calculated parameters.
    icdt_.ConfigureBitLevel(input_words_.size(), output_eu_float_, icde_);
    /*FillParams();
	PrintParams();*/
    icdt_.TranslateUnsignedBits(input_words_, output_eu_float_);
    EXPECT_EQ(LimitSigFigs(output_eu_float_[0]), LimitSigFigs(372736 + 56));

    // Use the formal translate function, float
    output_eu_float_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_TRUE(res);
    EXPECT_EQ(LimitSigFigs(output_eu_float_[0]), LimitSigFigs(372736 + 56));

    // double
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_TRUE(res);
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(372736 + 56));
}

TEST_F(ICDTranslateTest, TranslateSignedBitsSingleWord)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNEDBITS;
    icde_.bitlsb_ = 5;
    icde_.bitmsb_ = 2;
    icde_.bit_count_ = 4;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = double(1 << 2);

    // Setup input words.
    input_words_.push_back(13 << 11);  // -3
    input_words_.push_back(3 << 11);   // 3
    input_words_.push_back(11 << 11);  // -5

    // Run configure function and get calculated parameters.
    icdt_.ConfigureBitLevel(input_words_.size(), output_eu_float_, icde_);
    /*FillParams();
	PrintParams();*/
    icdt_.TranslateSignedBits(input_words_, output_eu_float_);
    EXPECT_THAT(output_eu_float_, ::testing::ElementsAre(-3, 3, -5));

    // float
    output_eu_float_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_TRUE(res);
    EXPECT_THAT(output_eu_float_, ::testing::ElementsAre(-3, 3, -5));

    // double
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_TRUE(res);
    EXPECT_THAT(output_eu_double_, ::testing::ElementsAre(-3, 3, -5));
}

TEST_F(ICDTranslateTest, TranslateSignedBitsTwoWord)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNEDBITS;
    icde_.bitlsb_ = 5;
    icde_.bitmsb_ = 12;
    icde_.bit_count_ = 10;
    icde_.elem_word_count_ = 2;
    icde_.msb_val_ = double(1 << 8);

    // Setup input words.
    input_words_.push_back(18);        // 10010
    input_words_.push_back(6 << 11);   // 00110
    input_words_.push_back(10);        // 01010
    input_words_.push_back(25 << 11);  // 11001

    // Run configure function and get calculated parameters.
    icdt_.ConfigureBitLevel(input_words_.size(), output_eu_float_, icde_);
    /*FillParams();
	PrintParams();*/
    icdt_.TranslateSignedBits(input_words_, output_eu_float_);
    EXPECT_THAT(output_eu_float_, ::testing::ElementsAre(-442, 345));

    // float
    output_eu_float_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_TRUE(res);
    EXPECT_THAT(output_eu_float_, ::testing::ElementsAre(-442, 345));

    //// double
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_TRUE(res);
    EXPECT_THAT(output_eu_double_, ::testing::ElementsAre(-442, 345));
}

TEST_F(ICDTranslateTest, TranslateBCDSingleWordAll16)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 16;
    icde_.bitmsb_ = 1;
    icde_.bit_count_ = 16;
    icde_.elem_word_count_ = 1;
    icde_.bcd_partial_ = 0;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bitmsb_ - 1));

    // Setup input words.
    // bits           1          16
    // bcd                8   7   3   
    uint16_t bits = 0b0000100001110011;
    uint16_t bcd = 873;
    input_words_.push_back(bits); 

    std::vector<uint16_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    // double
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(bcd)), LimitSigFigs(output_eu_double_.at(0)));

    // Ensure that bcd_partial does not affect bit_count_ % 4 == 0 case
    output_words.resize(0);
    icde_.bcd_partial_ = -1;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    output_words.resize(0);
    icde_.bcd_partial_ = 1;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));
}

TEST_F(ICDTranslateTest, TranslateBCDSingleWordPartial16)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 10;
    icde_.bitmsb_ = 3;
    icde_.bit_count_ = 8;
    icde_.elem_word_count_ = 1;
    icde_.bcd_partial_ = 0;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bitmsb_ - 1));

    // Setup input words.
    // bits             3   7     
    // bcd              6   2   
    uint16_t bits = 0b1001100010110101;
    uint16_t bcd = 62;
    input_words_.push_back(bits); 

    std::vector<uint16_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    // double
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(bcd)), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateBCDSingleWordIncompleteDigit)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 9;
    icde_.bitmsb_ = 3;
    icde_.bit_count_ = 7;
    icde_.elem_word_count_ = 1;
    icde_.bcd_partial_ = 0;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bitmsb_ - 1));

    // Setup input words.
    // bits             3   7     
    // bcd              6   2 (incomplete)   
    uint16_t bits = 0b1001100010110101;
    uint16_t bcd = 6;
    input_words_.push_back(bits); 

    std::vector<uint16_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    // double
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(bcd)), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateBCDSingleWordIncompleteDigitPartialLS)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 9;
    icde_.bitmsb_ = 3;
    icde_.bit_count_ = 7;
    icde_.elem_word_count_ = 1;
    icde_.bcd_partial_ = 1;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bitmsb_ - 1));

    // Setup input words.
    // bits             3   7     
    // bcd              6   1 (incomplete)   
    uint16_t bits = 0b1001100010110101;
    uint16_t bcd = 61;
    input_words_.push_back(bits); 

    std::vector<uint16_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    // double
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(bcd)), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateBCDSingleWordIncompleteDigitPartialMS)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 9;
    icde_.bitmsb_ = 3;
    icde_.bit_count_ = 7;
    icde_.elem_word_count_ = 1;
    icde_.bcd_partial_ = -1;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bitmsb_ - 1));

    // Setup input words.
    // bits             3  6      
    // bcd              3  4    
    uint16_t bits = 0b1001101000110101;
    uint16_t bcd = 34;
    input_words_.push_back(bits); 

    std::vector<uint16_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    // double
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(bcd)), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateBCDMultWordAll32)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 16;
    icde_.bitmsb_ = 1;
    icde_.bit_count_ = 32;
    icde_.elem_word_count_ = 2;
    icde_.bcd_partial_ = 0;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bitmsb_ - 1));

    // Setup input words.
    // bits            1             16     
    // bcd             9   8   3   5
    uint16_t bits1 = 0b1001100000110101;
    // bits            1             16     
    // bcd             7   4   0   2
    uint16_t bits2 = 0b0111010000000010;
    uint32_t bcd = 98357402;
    input_words_.push_back(bits1); 
    input_words_.push_back(bits2); 

    std::vector<uint32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    // double
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(bcd)), LimitSigFigs(output_eu_double_.at(0)));

    // Confirm that bcd_partial_ does not affect case in which bit_count_ % 4 == 0
    output_words.resize(0);
    icde_.bcd_partial_ = 1;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    output_words.resize(0);
    icde_.bcd_partial_ = -1;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));
}

TEST_F(ICDTranslateTest, TranslateBCDMultWordPartial32)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 7;
    icde_.bitmsb_ = 12;
    icde_.bit_count_ = 12;
    icde_.elem_word_count_ = 2;
    icde_.bcd_partial_ = 0;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bitmsb_ - 1));

    // Setup input words.
    // bits            1         12   16     
    // bcd                        8   9 
    uint16_t bits1 = 0b1001100000110001;
    // bits            1     7       16     
    // bcd                3       
    uint16_t bits2 = 0b0010011000000010;
    uint32_t bcd = 893;
    input_words_.push_back(bits1); 
    input_words_.push_back(bits2); 

    std::vector<uint32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    // double
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(bcd)), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateBCDMultWordIncompleteDigit)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 10;
    icde_.bitmsb_ = 12;
    icde_.bit_count_ = 15;
    icde_.elem_word_count_ = 2;
    icde_.bcd_partial_ = 0;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bitmsb_ - 1));

    // Setup input words.
    // bits            1         12   16     
    // bcd                        8   9 
    uint16_t bits1 = 0b1001100000110001;
    // bits            1     7  10    16     
    // bcd                3       
    uint16_t bits2 = 0b0010011000000010;
    uint32_t bcd = 893;
    input_words_.push_back(bits1); 
    input_words_.push_back(bits2); 

    std::vector<uint32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    // double
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(bcd)), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateBCDMultWordIncompleteDigitPartialLS)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 10;
    icde_.bitmsb_ = 12;
    icde_.bit_count_ = 15;
    icde_.elem_word_count_ = 2;
    icde_.bcd_partial_ = 1;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bitmsb_ - 1));

    // Setup input words.
    // bits            1         12   16     
    // bcd                        8   9 
    uint16_t bits1 = 0b1001100000110001;
    // bits            1     7  10    16     
    // bcd                3       
    uint16_t bits2 = 0b0010011000000010;
    uint32_t bcd = 8930;
    input_words_.push_back(bits1); 
    input_words_.push_back(bits2); 

    std::vector<uint32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    // double
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(bcd)), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateBCDMultWordIncompleteDigitPartialMS)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 10;
    icde_.bitmsb_ = 12;
    icde_.bit_count_ = 15;
    icde_.elem_word_count_ = 2;
    icde_.bcd_partial_ = -1;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bitmsb_ - 1));

    // Setup input words.
    // bits            1         12  15     
    // bcd                        4  4 
    uint16_t bits1 = 0b1001100000110001;
    // bits            1 3   7  10    16     
    // bcd               9   8    
    uint16_t bits2 = 0b0010011000000010;
    uint32_t bcd = 4498;
    input_words_.push_back(bits1); 
    input_words_.push_back(bits2); 

    std::vector<uint32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    // double
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(bcd)), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateBCDExplicitSignNotEnoughElems)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 10;
    icde_.bitmsb_ = 12;
    icde_.bit_count_ = 15;
    icde_.elem_word_count_ = 2;
    icde_.bcd_partial_ = -1;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bitmsb_ - 1));

    // Setup input words.
    // bits            1         12  15     
    // bcd                        4  4 
    uint16_t bits1 = 0b1001100000110001;
    // bits            1 3   7  10    16     
    // bcd               9   8    
    uint16_t bits2 = 0b0010011000000010;
    int32_t bcd = 4498;
    input_words_.push_back(bits1); 
    input_words_.push_back(bits2); 

    // Add twice so there are two identical words
    input_words_.push_back(bits1); 
    input_words_.push_back(bits2); 

    // Only 1 value, not two which are necessary for the two values in input_words_.
    std::vector<int8_t> explicit_sign{-1}; 

    std::vector<int32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_, explicit_sign));
    ASSERT_EQ(output_words.size(), 2);

    // both not negative, because sign not applied
    EXPECT_EQ(bcd, output_words.at(0));
    EXPECT_EQ(bcd, output_words.at(1));
}

TEST_F(ICDTranslateTest, TranslateBCDExplicitSign)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 10;
    icde_.bitmsb_ = 12;
    icde_.bit_count_ = 15;
    icde_.elem_word_count_ = 2;
    icde_.bcd_partial_ = -1;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bitmsb_ - 1));

    // Setup input words.
    // bits            1         12  15     
    // bcd                        4  4 
    uint16_t bits1 = 0b1001100000110001;
    // bits            1 3   7  10    16     
    // bcd               9   8    
    uint16_t bits2 = 0b0010011000000010;
    int32_t bcd = 4498;
    input_words_.push_back(bits1); 
    input_words_.push_back(bits2); 

    // Add three times so there are three identical words
    input_words_.push_back(bits1); 
    input_words_.push_back(bits2); 
    input_words_.push_back(bits1); 
    input_words_.push_back(bits2); 

    // Negative, pos, neg
    std::vector<int8_t> explicit_sign{-1, 1, -1}; 
    std::vector<int32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_, explicit_sign));
    ASSERT_EQ(output_words.size(), 3);

    EXPECT_EQ(-bcd, output_words.at(0));
    EXPECT_EQ(bcd, output_words.at(1));
    EXPECT_EQ(-bcd, output_words.at(2));
}

TEST_F(ICDTranslateTest, TranslateSignMagSingleWordAll16)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNMAG;
    icde_.bitlsb_ = 16;
    icde_.bitmsb_ = 1;
    icde_.bit_count_ = 16;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = static_cast<double>(1 << (icde_.bit_count_ - 2));

    uint16_t bits = 0b0000100001110011;
    int16_t val = bits;
    input_words_.push_back(bits); 
    std::vector<int16_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(val, output_words.at(0));

    bits = 0b1000100001110011;
    val = 0b0000100001110011 * -1;
    input_words_.resize(0);
    input_words_.push_back(bits); 
    output_words.resize(0);
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(val, output_words.at(0));

    // double
    icde_.msb_val_ = icde_.msb_val_ / 10.0;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(val) / 10.0), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateSignMagSingleWordPartial16)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNMAG;
    icde_.bitlsb_ = 9;
    icde_.bitmsb_ = 4;
    icde_.bit_count_ = 6;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = static_cast<double>(1 << (icde_.bit_count_ - 2));

    uint16_t bits = 0b0000100101110011;
    int32_t val = 0b010010;
    input_words_.push_back(bits); 
    std::vector<int32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(val, output_words.at(0));

    bits = 0b0001100101110011;
    val = 0b010010 * -1;
    input_words_.resize(0);
    input_words_.push_back(bits); 
    output_words.resize(0);
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(val, output_words.at(0));

    // double
    icde_.msb_val_ = icde_.msb_val_ / 10.0;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(val) / 10.0), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateSignMagMultWordAll32)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNMAG;
    icde_.bitlsb_ = 16;
    icde_.bitmsb_ = 1;
    icde_.bit_count_ = 32;
    icde_.elem_word_count_ = 2;
    icde_.msb_val_ = static_cast<double>(1 << (icde_.bit_count_ - 2));

    uint32_t bits = 0b00100001010110010011001101001011;
    int32_t val = bits;
    const uint16_t* ui16ptr = reinterpret_cast<const uint16_t*>(&bits);
    input_words_.push_back(ui16ptr[1]); 
    input_words_.push_back(ui16ptr[0]); 
    std::vector<int32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(val, output_words.at(0));

    bits = 0b10100001010110010011001101001011;
    val = 0b00100001010110010011001101001011 * -1;
    input_words_.resize(0);
    input_words_.push_back(ui16ptr[1]); 
    input_words_.push_back(ui16ptr[0]); 
    output_words.resize(0);
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(val, output_words.at(0));

    // double
    icde_.msb_val_ = icde_.msb_val_ / 10.0;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(val) / 10.0), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateSignMagMultWordPartial32)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNMAG;
    icde_.bitlsb_ = 3;
    icde_.bitmsb_ = 5;
    icde_.bit_count_ = 15;
    icde_.elem_word_count_ = 2;
    icde_.msb_val_ = static_cast<double>(1 << (icde_.bit_count_ - 2));

    //                1   5          16 3 
    uint32_t bits = 0b00100001010110010011001101001011;
    int32_t val = 0b000101011001001;
    const uint16_t* ui16ptr = reinterpret_cast<const uint16_t*>(&bits);
    input_words_.push_back(ui16ptr[1]); 
    input_words_.push_back(ui16ptr[0]); 
    std::vector<int32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(val, output_words.at(0));

    bits = 0b00101001010110010011001101001011;
    val = 0b000101011001001 * -1;
    input_words_.resize(0);
    input_words_.push_back(ui16ptr[1]); 
    input_words_.push_back(ui16ptr[0]); 
    output_words.resize(0);
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_words, icde_));
    EXPECT_EQ(val, output_words.at(0));

    // double
    icde_.msb_val_ = icde_.msb_val_ / 10.0;
    ASSERT_TRUE(icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(val) / 10.0), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateUnsigned16)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::UNSIGNED16;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = double(1 << 15);

    // Setup input words.
    input_words_.push_back(1035);
    input_words_.push_back(665);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    /*FillParams();
	PrintParams();*/
    icdt_.TranslateUnsigned16(input_words_, output_eu_float_);
    EXPECT_THAT(output_eu_float_, ::testing::ElementsAre(1035, 665));

    // float
    output_eu_float_.resize(0);
    icde_.msb_val_ = 0.5 * icde_.msb_val_;
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_TRUE(res);
    EXPECT_THAT(output_eu_float_, ::testing::ElementsAre(517.5, 332.5));
}

TEST_F(ICDTranslateTest, TranslateSigned16)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::SIGNED16;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = double(1 << 14);

    // Setup input words.
    input_words_.push_back((1 << 15) + 37);  // -32731
    input_words_.push_back(665);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    /*FillParams();
	PrintParams();*/
    icdt_.TranslateSigned16(input_words_, output_eu_float_);
    EXPECT_THAT(output_eu_float_, ::testing::ElementsAre(-32731, 665));

    // float
    output_eu_float_.resize(0);
    icde_.msb_val_ = 0.5 * icde_.msb_val_;
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_TRUE(res);
    EXPECT_THAT(output_eu_float_, ::testing::ElementsAre(-16365.5, 332.5));
}

TEST_F(ICDTranslateTest, TranslateUnsigned32)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::UNSIGNED32;
    icde_.elem_word_count_ = 2;
    icde_.msb_val_ = double(uint64_t(1) << 31);

    // Setup input words.
    input_words_.push_back(1035);  // 67829760
    input_words_.push_back(765);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    /*FillParams();
	PrintParams();*/
    icdt_.TranslateUnsigned32(input_words_, output_eu_double_);
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(67829750 + 765));

    // double
    output_eu_double_.resize(0);
    icde_.msb_val_ = 0.1 * icde_.msb_val_;
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_TRUE(res);
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(6782975.0 + 76.5));
}

TEST_F(ICDTranslateTest, TranslateSigned32)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::SIGNED32;
    icde_.elem_word_count_ = 2;
    icde_.msb_val_ = double(uint64_t(1) << 30);

    // Setup input words.
    input_words_.push_back((1 << 15) + 41);
    input_words_.push_back(2210);
    input_words_.push_back(200);
    input_words_.push_back(4532);

    // Run configure function and get calculated parameters.
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    /*FillParams();
	PrintParams();*/
    icdt_.TranslateSigned32(input_words_, output_eu_double_);
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(-2144794462));
    EXPECT_EQ(LimitSigFigs(output_eu_double_[1]), LimitSigFigs(13111732));

    // double
    output_eu_double_.resize(0);
    icde_.msb_val_ = 0.1 * icde_.msb_val_;
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_TRUE(res);
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(-214479446.2));
    EXPECT_EQ(LimitSigFigs(output_eu_double_[1]), LimitSigFigs(1311173.2));
}

TEST_F(ICDTranslateTest, TranslateFloat32GPS)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::FLOAT32_GPS;
    icde_.elem_word_count_ = 2;

    input_words_.push_back(1432);
    input_words_.push_back(44201);
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    icdt_.TranslateFloat32GPS(input_words_, output_eu_float_);
    EXPECT_FLOAT_EQ(LimitSigFigs(output_eu_float_[0]), LimitSigFigs(3.58935e-36));
    output_eu_float_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_FLOAT_EQ(LimitSigFigs(output_eu_float_[0]), LimitSigFigs(3.58935e-36));
}

TEST_F(ICDTranslateTest, TranslateFloat32GPSCornerCases)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::FLOAT32_GPS;
    icde_.elem_word_count_ = 2;

    // Identical zeros.
    input_words_.push_back(0);
    input_words_.push_back(0);
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    icdt_.TranslateFloat32GPS(input_words_, output_eu_float_);
    EXPECT_FLOAT_EQ(output_eu_float_[0], 0.0);
    output_eu_float_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_FLOAT_EQ(output_eu_float_[0], 0.0);

    // Sign bit high and all exponent bits = zero.
    input_words_[0] = 1 << 15;
    input_words_[1] = 0;
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    icdt_.TranslateFloat32GPS(input_words_, output_eu_float_);
    EXPECT_TRUE(isnan(output_eu_float_[0]));
    output_eu_float_.resize(0);
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_TRUE(isnan(output_eu_float_[0]));

    // Case in which value is in [0, 0.5), exponent bits zeros.
    input_words_[0] = 32;
    input_words_[1] = 553;
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    icdt_.TranslateFloat32GPS(input_words_, output_eu_float_);
    EXPECT_EQ(LimitSigFigs(output_eu_float_[0]), LimitSigFigs(0.12503));
    output_eu_float_.resize(0);
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_EQ(LimitSigFigs(output_eu_float_[0]), LimitSigFigs(0.12503));
}

TEST_F(ICDTranslateTest, TranslateFloat64GPS)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::FLOAT64_GPS;
    icde_.elem_word_count_ = 4;

    input_words_.push_back(44906);
    input_words_.push_back(8876);
    input_words_.push_back(3567);
    input_words_.push_back(4096);
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    icdt_.TranslateFloat64GPS(input_words_, output_eu_double_);
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(-5.32362e-11));
    output_eu_double_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(-5.32362e-11));
}

TEST_F(ICDTranslateTest, TranslateFloat64GPSCornerCases)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::FLOAT64_GPS;
    icde_.elem_word_count_ = 4;

    // Identical zero.
    input_words_.push_back(0);
    input_words_.push_back(0);
    input_words_.push_back(0);
    input_words_.push_back(0);
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    icdt_.TranslateFloat64GPS(input_words_, output_eu_double_);
    EXPECT_EQ(output_eu_double_[0], 0.0);
    output_eu_double_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_EQ(output_eu_double_[0], 0.0);

    // NaN. Sign bit high all exponent bits zero.
    input_words_[0] = (1 << 15) + 33;  // exponent sign bit high
    input_words_[1] = 3542;
    input_words_[2] = 11112;
    input_words_[3] = 11112;
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    icdt_.TranslateFloat64GPS(input_words_, output_eu_double_);
    EXPECT_TRUE(isnan(output_eu_double_[0]));
    output_eu_double_.resize(0);
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_TRUE(isnan(output_eu_double_[0]));

    // Values in [0, 0.5).
    input_words_[0] = 83;  // < 1 << 7
    input_words_[1] = 22106;
    input_words_[2] = 2049;
    input_words_[3] = 33311;
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    icdt_.TranslateFloat64GPS(input_words_, output_eu_double_);
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(0.325536));
    output_eu_double_.resize(0);
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(0.325536));
}

TEST_F(ICDTranslateTest, TranslateCAPS)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::CAPS;
    icde_.elem_word_count_ = 3;

    input_words_.push_back(44906);
    input_words_.push_back(8876);
    input_words_.push_back(3567);
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    icdt_.TranslateCAPS(input_words_, output_eu_double_);
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(1.3218e-7));
    output_eu_double_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(1.3218e-7));
}

TEST_F(ICDTranslateTest, TranslateCAPSCornerCases)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::CAPS;
    icde_.elem_word_count_ = 3;

    // Identical zero.
    input_words_.push_back(0);
    input_words_.push_back(0);
    input_words_.push_back(0);
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    icdt_.TranslateCAPS(input_words_, output_eu_double_);
    EXPECT_EQ(output_eu_double_[0], 0.0);
    output_eu_double_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_EQ(output_eu_double_[0], 0.0);

    // Sign bit is high, exponent bits zero --> undefined.
    input_words_[0] = 512;  // first 7 bits == 0
    input_words_[1] = 897;
    input_words_[2] = (1 << 15) + 4459;
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    icdt_.TranslateCAPS(input_words_, output_eu_double_);
    EXPECT_TRUE(isnan(output_eu_double_[0]));
    output_eu_double_.resize(0);
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_TRUE(isnan(output_eu_double_[0]));

    // Values in [0, 0.5).
    input_words_[0] = 3072;  // first 7 bits == 0
    input_words_[1] = 897;
    input_words_[2] = 4459;
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    icdt_.TranslateCAPS(input_words_, output_eu_double_);
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(0.068039));
    output_eu_double_.resize(0);
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(0.068039));
}

TEST_F(ICDTranslateTest, TranslateFloat32IEEE)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::FLOAT32_IEEE;
    icde_.elem_word_count_ = 2;

    float val = -3.78632e-5;
    uint16_t* ui16ptr = (uint16_t*)&val;
    input_words_.push_back(ui16ptr[1]);
    input_words_.push_back(ui16ptr[0]);
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    icdt_.TranslateFloat32IEEE(input_words_, output_eu_float_);
    EXPECT_EQ(LimitSigFigs(output_eu_float_[0]), LimitSigFigs(val));
    output_eu_float_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_EQ(LimitSigFigs(output_eu_float_[0]), LimitSigFigs(val));
}

TEST_F(ICDTranslateTest, TranslateFloat32IEEECornerCases)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::FLOAT32_IEEE;
    icde_.elem_word_count_ = 2;

    // Identical zero.
    float val = 0.0;
    uint16_t* ui16ptr = (uint16_t*)&val;
    input_words_.push_back(ui16ptr[1]);
    input_words_.push_back(ui16ptr[0]);
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    icdt_.TranslateFloat32IEEE(input_words_, output_eu_float_);
    EXPECT_EQ(output_eu_float_[0], val);
    output_eu_float_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_EQ(output_eu_float_[0], val);

    // Positive and negative infinities.
    input_words_[0] = ((1 << 8) - 1) << 7;  // All exponent bits high, all fracion bits low.
    input_words_[1] = 0;                    // All other fraction bits low.
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    icdt_.TranslateFloat32IEEE(input_words_, output_eu_float_);
    EXPECT_TRUE(isinf(output_eu_float_[0]));
    output_eu_float_.resize(0);
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_TRUE(isinf(output_eu_float_[0]));

    output_eu_float_.resize(0);
    input_words_[0] += 1 << 15;  // add sign bit for neg inf.
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    icdt_.TranslateFloat32IEEE(input_words_, output_eu_float_);
    EXPECT_TRUE(isinf(output_eu_float_[0]));
    output_eu_float_.resize(0);
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_TRUE(isinf(output_eu_float_[0]));

    // NaN. Sign bit either high or low, exponent all high, mantissa anything
    // but all zeros (all zero mantissa is inf).
    input_words_[0] = (((1 << 8) - 1) << 7) + 81;
    input_words_[1] = 4432;
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    icdt_.TranslateFloat32IEEE(input_words_, output_eu_float_);
    EXPECT_TRUE(isnan(output_eu_float_[0]));
    output_eu_float_.resize(0);
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_TRUE(isnan(output_eu_float_[0]));

    output_eu_float_.resize(0);
    input_words_[0] += (1 << 15);  // Add sign bit
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    icdt_.TranslateFloat32IEEE(input_words_, output_eu_float_);
    EXPECT_TRUE(isnan(output_eu_float_[0]));
    output_eu_float_.resize(0);
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_TRUE(isnan(output_eu_float_[0]));
}

TEST_F(ICDTranslateTest, TranslateFloat64IEEE)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::FLOAT64_IEEE;
    icde_.elem_word_count_ = 4;

    double val = -3.78632e-5;
    uint16_t* ui16ptr = (uint16_t*)&val;
    input_words_.push_back(ui16ptr[3]);
    input_words_.push_back(ui16ptr[2]);
    input_words_.push_back(ui16ptr[1]);
    input_words_.push_back(ui16ptr[0]);
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    icdt_.TranslateFloat64IEEE(input_words_, output_eu_double_);
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(val));
    output_eu_double_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(val));
}

TEST_F(ICDTranslateTest, TranslateFloat64IEEECornerCases)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::FLOAT64_IEEE;
    icde_.elem_word_count_ = 4;

    // Identical zero.
    double val = 0.0;
    uint16_t* ui16ptr = (uint16_t*)&val;
    input_words_.push_back(ui16ptr[3]);
    input_words_.push_back(ui16ptr[2]);
    input_words_.push_back(ui16ptr[1]);
    input_words_.push_back(ui16ptr[0]);
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    icdt_.TranslateFloat64IEEE(input_words_, output_eu_double_);
    EXPECT_EQ(output_eu_double_[0], val);
    output_eu_double_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_EQ(output_eu_double_[0], val);

    // Positive and negative infinities.
    input_words_[0] = ((1 << 11) - 1) << 4;  // All exponent bits high, all fracion bits low.
    input_words_[1] = 0;                     // All other fraction bits low.
    input_words_[2] = 0;
    input_words_[3] = 0;
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    icdt_.TranslateFloat64IEEE(input_words_, output_eu_double_);
    EXPECT_TRUE(isinf(output_eu_double_[0]));
    output_eu_double_.resize(0);
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_TRUE(isinf(output_eu_double_[0]));

    output_eu_double_.resize(0);
    input_words_[0] += 1 << 15;  // add sign bit for neg inf.
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    icdt_.TranslateFloat64IEEE(input_words_, output_eu_double_);
    EXPECT_TRUE(isinf(output_eu_double_[0]));
    output_eu_double_.resize(0);
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_TRUE(isinf(output_eu_double_[0]));

    // NaN. Sign bit either high or low, exponent all high, mantissa anything
    // but all zeros (all zero mantissa is inf).
    input_words_[0] = (((1 << 11) - 1) << 4) + 12;
    input_words_[1] = 4432;
    input_words_[2] = 123;
    input_words_[3] = 8873;
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    icdt_.TranslateFloat64IEEE(input_words_, output_eu_double_);
    EXPECT_TRUE(isnan(output_eu_double_[0]));
    output_eu_double_.resize(0);
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_TRUE(isnan(output_eu_double_[0]));

    output_eu_double_.resize(0);
    input_words_[0] += (1 << 15);  // Add sign bit
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
    icdt_.TranslateFloat64IEEE(input_words_, output_eu_double_);
    EXPECT_TRUE(isnan(output_eu_double_[0]));
    output_eu_double_.resize(0);
    res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
    EXPECT_TRUE(isnan(output_eu_double_[0]));
}

TEST_F(ICDTranslateTest, TranslateFloat321750)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::FLOAT32_1750;
    icde_.elem_word_count_ = 2;

    input_words_.push_back(4476);
    input_words_.push_back(27);
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    icdt_.TranslateFloat321750(input_words_, output_eu_float_);
    EXPECT_EQ(LimitSigFigs(output_eu_float_[0]), LimitSigFigs(1.8333e7));
    output_eu_float_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_EQ(LimitSigFigs(output_eu_float_[0]), LimitSigFigs(1.8333e7));
}

TEST_F(ICDTranslateTest, TranslateFloat321750CornerCases)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::FLOAT32_1750;
    icde_.elem_word_count_ = 2;

    // Identical zero.
    input_words_.push_back(0);
    input_words_.push_back(0);
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    icdt_.TranslateFloat321750(input_words_, output_eu_float_);
    EXPECT_EQ(output_eu_float_[0], 0.0);
    output_eu_float_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_EQ(output_eu_float_[0], 0.0);

    // According to the following comment by Justin, this may be all the corner
    // cases necessary for this data type:
    // "According to Jeff Walton from MMC. He said there are:
    // "no hidden bits (ever), no NAN, no INF, exact zero is all zeros".
    // So I think you have the corner cases covered here
}

TEST_F(ICDTranslateTest, TranslateFloat16)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::FLOAT16;
    icde_.elem_word_count_ = 1;

    input_words_.push_back(5384);
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    icdt_.TranslateFloat16(input_words_, output_eu_float_);
    EXPECT_EQ(LimitSigFigs(output_eu_float_[0]), LimitSigFigs(1.443109e12));
    output_eu_float_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_EQ(LimitSigFigs(output_eu_float_[0]), LimitSigFigs(1.443109e12));
}

TEST_F(ICDTranslateTest, TranslateFloat16CornerCases)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::FLOAT16;
    icde_.elem_word_count_ = 1;

    // Identical zero.
    input_words_.push_back(0);
    icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
    icdt_.TranslateFloat16(input_words_, output_eu_float_);
    EXPECT_EQ(output_eu_float_[0], 0.0);
    output_eu_float_.resize(0);
    bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
    EXPECT_EQ(output_eu_float_[0], 0.0);

    // Need tests for NaNs, Infs, other special cases?
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32TranslateU32)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::UNSIGNED32;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = double(uint64_t(1) << 31);
    uint32_t val = 6413001;
    std::vector<uint32_t> input_words32{val};

    // Run configure function and get calculated parameters.
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(val));

    // double
    output_eu_double_.resize(0);
    icde_.msb_val_ = 0.1 * icde_.msb_val_;
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(val * 0.1));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32TranslateU32ExpectWordCount1)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::UNSIGNED32;
    // For an input raw type of uint32_t, and schema uint32_t only 
    // one elem is necessary--ought to be 1
    icde_.elem_word_count_ = 2;
    std::vector<uint32_t> input_words32;

    // Run configure function and get calculated parameters.
    ASSERT_FALSE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32TranslateS32)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::SIGNED32;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = double(uint64_t(1) << 30);
    int32_t val = -6413001;
    const uint32_t* val_ptr = reinterpret_cast<const uint32_t*>(&val);
    std::vector<uint32_t> input_words32{*val_ptr};

    // Run configure function and get calculated parameters.
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(val));

    // double
    output_eu_double_.resize(0);
    icde_.msb_val_ = 0.1 * icde_.msb_val_;
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(val * 0.1));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32TranslateS32ExpectWordCount1)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = false;
    icde_.schema_ = ICDElementSchema::SIGNED32;
    // For an input raw type of uint32_t, and schema uint32_t only 
    // one elem is necessary--ought to be 1
    icde_.elem_word_count_ = 2;
    std::vector<uint32_t> input_words32;

    // Run configure function and get calculated parameters.
    ASSERT_FALSE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32SignedBitsSingleWordInvalidWordCount)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNEDBITS;

    // For single word, word count must be 1. 
    icde_.elem_word_count_ = 2;
    icde_.msb_val_ = double(1 << 8);
    std::vector<uint32_t> input_words32;

    ASSERT_FALSE(icdt_.TranslateArray(input_words32, output_eu_float_, icde_));
}

TEST_F(ICDTranslateTest, Swap16)
{
    std::vector<uint16_t> expected{5431, 17, 901, 54};
    std::vector<uint32_t> input_words32(2, 0);
    uint16_t* u16ptr = reinterpret_cast<uint16_t*>(input_words32.data());
    u16ptr[0] = 17;
    u16ptr[1] = 5431;
    u16ptr[2] = 54;
    u16ptr[3] = 901;

    std::vector<uint16_t> output_words;
    icdt_.Swap16(input_words32, output_words);
    EXPECT_THAT(expected, ::testing::ElementsAreArray(output_words));
}

TEST_F(ICDTranslateTest, KeepMS16)
{
    std::vector<uint16_t> expected{5431, 17, 901, 54};
    std::vector<uint32_t> input_words32(4, 0);
    input_words32[1] = 17 << 16;
    input_words32[0] = 5431 << 16;
    input_words32[3] = 54 << 16;
    input_words32[2] = 901 << 16;

    std::vector<uint16_t> output_words;
    icdt_.KeepMS16(input_words32, output_words);
    EXPECT_THAT(expected, ::testing::ElementsAreArray(output_words));
}

TEST_F(ICDTranslateTest, KeepLS16)
{
    std::vector<uint16_t> expected{5431, 17, 901, 54};
    std::vector<uint32_t> input_words32(4, 0);
    input_words32[1] = (908 << 16) + 17;
    input_words32[0] = (836 << 16) + 5431;
    input_words32[3] = (4511 << 16) + 54;
    input_words32[2] = (47 << 16) + 901;

    std::vector<uint16_t> output_words;
    icdt_.KeepLS16(input_words32, output_words);
    EXPECT_THAT(expected, ::testing::ElementsAreArray(output_words));
}

TEST_F(ICDTranslateTest, ConfigureU32ElemForU16MS16)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNEDBITS;
    icde_.bitlsb_ = 9;
    icde_.bitmsb_ = 5;
    icde_.bit_count_ = 5;
    icde_.elem_word_count_ = 1;

    std::vector<uint32_t> input_words{555 << 16};
    std::vector<uint16_t> output_words{555}; 
    icdt_.ConfigureU32BitElemForU16(icde_, input_words, output_words);
    EXPECT_EQ(1, icde_.elem_word_count_);
    EXPECT_EQ(9, icde_.bitlsb_);
    EXPECT_EQ(5, icde_.bitmsb_);
    EXPECT_EQ(5, icde_.bit_count_);
    ASSERT_TRUE(output_words.size() == 1);
    EXPECT_EQ(555, output_words.at(0));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32SignedBitsSingleWordMS16)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNEDBITS;
    icde_.bitlsb_ = 9;
    icde_.bitmsb_ = 5;
    icde_.bit_count_ = 5;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = double(1 << 3);
    
    //                   5   9  = 5
    uint32_t raw = 0b00001011000000000000000000000000;
    int8_t val = -16 + 6;
    double expected = static_cast<double>(val);
    std::vector<uint32_t> input_words32{raw};

    // float
    output_eu_float_.resize(0);
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_float_, icde_));
    EXPECT_EQ(LimitSigFigs(output_eu_float_[0]), LimitSigFigs(expected));

    // double
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(expected));

    // positive
    raw = 0b00000011000000000000000000000000;
    val = 6;
    expected = static_cast<double>(val);
    input_words32[0] = raw;
    output_eu_double_.resize(0);
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(expected));
}

TEST_F(ICDTranslateTest, ConfigureU32ElemForU16LS16)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNEDBITS;
    icde_.bitlsb_ = 24;
    icde_.bitmsb_ = 18;
    icde_.bit_count_ = 7;
    icde_.elem_word_count_ = 1;

    std::vector<uint32_t> input_words{(248 << 16) + 555};
    std::vector<uint16_t> output_words{555}; 
    icdt_.ConfigureU32BitElemForU16(icde_, input_words, output_words);
    EXPECT_EQ(1, icde_.elem_word_count_);
    EXPECT_EQ(8, icde_.bitlsb_);
    EXPECT_EQ(2, icde_.bitmsb_);
    EXPECT_EQ(7, icde_.bit_count_);
    ASSERT_TRUE(output_words.size() == 1);
    EXPECT_EQ(555, output_words.at(0));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32SignedBitsSingleWordLS16)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNEDBITS;
    icde_.bitlsb_ = 24;
    icde_.bitmsb_ = 18;
    icde_.bit_count_ = 7;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = double(1 << 5);
    
    //                               18    24  = 7
    uint32_t raw = 0b00000000000000000110001100000000;
    int8_t val = -64 + 35;
    double expected = static_cast<double>(val);
    std::vector<uint32_t> input_words32{raw};

    // float
    output_eu_float_.resize(0);
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_float_, icde_));
    EXPECT_EQ(LimitSigFigs(output_eu_float_[0]), LimitSigFigs(expected));

    // double
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(expected));

    // positive
    raw = 0b00000000000000000010001100000000;
    val = 35;
    expected = static_cast<double>(val);
    input_words32[0] = raw;
    output_eu_double_.resize(0);
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(expected));
}

TEST_F(ICDTranslateTest, ConfigureU32ElemForU16All32)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNEDBITS;
    icde_.bitlsb_ = 21;
    icde_.bitmsb_ = 12;
    icde_.bit_count_ = 10;
    icde_.elem_word_count_ = 1;

    // 0b10010 = 18, 0b0101000000000000 ==> actual bits 0b1001001010 
    std::vector<uint32_t> input_words{(18 << 16) + 0b0101000000000000};
    const uint16_t* ui16ptr = reinterpret_cast<const uint16_t*>(input_words.data());
    std::vector<uint16_t> expected_words{ui16ptr[1], ui16ptr[0]}; 
    std::vector<uint16_t> output_words;
    icdt_.ConfigureU32BitElemForU16(icde_, input_words, output_words);
    EXPECT_EQ(2, icde_.elem_word_count_);
    EXPECT_EQ(5, icde_.bitlsb_);
    EXPECT_EQ(12, icde_.bitmsb_);
    EXPECT_EQ(10, icde_.bit_count_);
    ASSERT_TRUE(output_words.size() == 2);
    EXPECT_THAT(expected_words, ::testing::ElementsAreArray(output_words));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32SignedBitsSingleWordAll32)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNEDBITS;
    icde_.bitlsb_ = 21;
    icde_.bitmsb_ = 12;
    icde_.bit_count_ = 10;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = double(1 << 8);
    
    // 0b10010 = 18, 0b0101000000000000 ==> actual bits 0b1001001010 
    uint32_t raw = (18 << 16) + 0b0101000000000000;
    int16_t val = -512 + 74;
    double expected = static_cast<double>(val);
    std::vector<uint32_t> input_words32{raw};

    // float
    output_eu_float_.resize(0);
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_float_, icde_));
    EXPECT_EQ(LimitSigFigs(output_eu_float_[0]), LimitSigFigs(expected));

    // double
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(expected));

    // positive
    raw = (2 << 16) + 0b0101000000000000;
    val = 74;
    expected = static_cast<double>(val);
    input_words32[0] = raw;
    output_eu_double_.resize(0);
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(expected));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32SignedBitsSingleWord19Bits)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNEDBITS;
    icde_.bitlsb_ = 32;
    icde_.bitmsb_ = 14;
    icde_.bit_count_ = 19;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = double(1 << 17);
    
    // high bits 0b100 = 4, actual bits 0b1000000000000000000
    uint32_t raw = (1 << 18);
    int32_t val = -262144;
    double expected = static_cast<double>(val);
    std::vector<uint32_t> input_words32{raw};

    // float
    output_eu_float_.resize(0);
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_float_, icde_));
    EXPECT_EQ(output_eu_float_[0], expected);

    // double
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(output_eu_double_[0], expected);

    // positive
    raw = (1 << 17);
    val = 131072;
    expected = static_cast<double>(val);
    input_words32[0] = raw;
    output_eu_double_.resize(0);
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(output_eu_double_[0], expected);
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32UnsignedBitsSingleWordAll32)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::UNSIGNEDBITS;
    icde_.bitlsb_ = 21;
    icde_.bitmsb_ = 12;
    icde_.bit_count_ = 10;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = double(1 << 9);
    
    // 0b00010 = 2, 0b0101000000000000 ==> actual bits 0b1001001010 
    uint32_t raw = (2 << 16) + 0b0101000000000000;
    int16_t val = 74;
    double expected = static_cast<double>(val);
    std::vector<uint32_t> input_words32{raw};

    // float
    output_eu_float_.resize(0);
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_float_, icde_));
    EXPECT_EQ(LimitSigFigs(output_eu_float_[0]), LimitSigFigs(expected));

    // double
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(expected));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32BCDMS16)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 9;
    icde_.bitmsb_ = 2;
    icde_.bit_count_ = 8;
    icde_.elem_word_count_ = 1;
    icde_.bcd_partial_ = 0;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bit_count_ - 1));

    // Setup input words.
    // bits            12      9   16     
    // bcd              3      0        
    uint32_t bits = 0b1001100000110001 << 16;
    uint32_t bcd = 30;
    std::vector<uint32_t> input_words32{bits};
    std::vector<uint32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    // double
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(bcd)), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32BCDLS16)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 30;  // bit 14 in LS word
    icde_.bitmsb_ = 20;  // bit 4 in LS word
    icde_.bit_count_ = 11;
    icde_.elem_word_count_ = 1;
    icde_.bcd_partial_ = 0;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bit_count_ - 1));

    // Setup input words.
    // bits           1  4        14      
    // bcd               9   1           
    uint32_t bits = 0b1001001000110001;
    uint32_t bcd = 91;
    std::vector<uint32_t> input_words32{bits};
    std::vector<uint32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    // double
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(bcd)), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32BCDPartial32)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 23;
    icde_.bitmsb_ = 12;
    icde_.bit_count_ = 12;
    icde_.elem_word_count_ = 1;
    icde_.bcd_partial_ = 0;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bit_count_ - 1));

    // Setup input words.
    // bits            1         12   16     
    // bcd                        8   9 
    uint16_t bits1 = 0b1001100000110001;
    // bits            1     7       16     
    // bcd                3       
    uint16_t bits2 = 0b0010011000000010;
    uint32_t bcd = 893;
    std::vector<uint32_t> input_words32(1);
    uint16_t* ui16ptr = reinterpret_cast<uint16_t*>(input_words32.data());
    ui16ptr[0] = bits2;
    ui16ptr[1] = bits1;

    std::vector<uint32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    // double
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(bcd)), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32BCDPartialLS)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 25;
    icde_.bitmsb_ = 12;
    icde_.bit_count_ = 14;
    icde_.elem_word_count_ = 1;
    icde_.bcd_partial_ = 1;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bit_count_ - 1));

    // Setup input words.
    // bits            1         12   16     
    // bcd                        8   9 
    uint16_t bits1 = 0b1001100000110001;
    // bits           17       25     16     
    // bcd                3   2    
    uint16_t bits2 = 0b0010011100000010;
    uint32_t bcd = 8932;
    std::vector<uint32_t> input_words32(1);
    uint16_t* ui16ptr = reinterpret_cast<uint16_t*>(input_words32.data());
    ui16ptr[0] = bits2;
    ui16ptr[1] = bits1;

    std::vector<uint32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    // double
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(bcd)), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32BCDPartialMS)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 25;
    icde_.bitmsb_ = 12;
    icde_.bit_count_ = 14;
    icde_.elem_word_count_ = 1;
    icde_.bcd_partial_ = -1;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bit_count_ - 1));

    // Setup input words.
    // bits            1         12 14 16     
    // bcd                        2 2  
    uint16_t bits1 = 0b1001100000110001;
    // bits           1718       25     16     
    // bcd              4   6    
    uint16_t bits2 = 0b0010001100000010;
    uint32_t bcd = 2246;
    std::vector<uint32_t> input_words32(1);
    uint16_t* ui16ptr = reinterpret_cast<uint16_t*>(input_words32.data());
    ui16ptr[0] = bits2;
    ui16ptr[1] = bits1;

    std::vector<uint32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    // double
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(bcd)), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32BCD429Case)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::BCD;
    icde_.bitlsb_ = 32;
    icde_.bitmsb_ = 14;
    icde_.bit_count_ = 19;
    icde_.elem_word_count_ = 1;
    icde_.bcd_partial_ = -1;
    // MSB is not relevant for BCD
    icde_.msb_val_ = double(1 << (icde_.bit_count_ - 1));

    // Setup input words.
    // bits            1           14 16     
    // bcd                          5 
    uint16_t bits1 = 0b1001100000110101;
    // bits           17              32     
    // bcd             2   3   0   7 
    uint16_t bits2 = 0b0010001100000111;
    uint32_t bcd = 52307;
    std::vector<uint32_t> input_words32(1);
    uint16_t* ui16ptr = reinterpret_cast<uint16_t*>(input_words32.data());
    ui16ptr[0] = bits2;
    ui16ptr[1] = bits1;

    std::vector<uint32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_words, icde_));
    EXPECT_EQ(bcd, output_words.at(0));

    // double
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(bcd)), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32SignMagMS16)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNMAG;
    icde_.bitlsb_ = 16;
    icde_.bitmsb_ = 5;
    icde_.bit_count_ = 12;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = static_cast<double>(1 << (icde_.bit_count_ - 2));

    //                1   5         16  
    uint32_t bits = 0b0010000101011001 << 16;
    int32_t val = 0b000101011001;
    std::vector<uint32_t> input_words32{bits};
    std::vector<int32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_words, icde_));
    EXPECT_EQ(val, output_words.at(0));

    bits = 0b0010100101011001 << 16;
    val = 0b000101011001 * -1;
    input_words32[0] = bits;
    output_words.resize(0);
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_words, icde_));
    EXPECT_EQ(val, output_words.at(0));

    // double
    icde_.msb_val_ = icde_.msb_val_ / 10.0;
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(val) / 10.0), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32SignMagLS16)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNMAG;
    icde_.bitlsb_ = 23;  // bit 7 in LS word
    icde_.bitmsb_ = 19;  // bit 3 in LS word
    icde_.bit_count_ = 5;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = static_cast<double>(1 << (icde_.bit_count_ - 2));

    //                1 3   7        16  
    uint32_t bits = 0b0000011101011001;
    int32_t val = 0b00011;
    std::vector<uint32_t> input_words32{bits};
    std::vector<int32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_words, icde_));
    EXPECT_EQ(val, output_words.at(0));

    bits = 0b0010011101011001;
    val = 0b00011 * -1;
    input_words32[0] = bits;
    output_words.resize(0);
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_words, icde_));
    EXPECT_EQ(val, output_words.at(0));

    // double
    icde_.msb_val_ = icde_.msb_val_ / 10.0;
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(val) / 10.0), LimitSigFigs(output_eu_double_.at(0)));
}

TEST_F(ICDTranslateTest, TranslateArrayRawTypeU32SignMagPartial32)
{
    icde_.msg_name_ = "A";
    icde_.elem_name_ = "A-1";
    icde_.is_bitlevel_ = true;
    icde_.schema_ = ICDElementSchema::SIGNMAG;
    icde_.bitlsb_ = 19;
    icde_.bitmsb_ = 5;
    icde_.bit_count_ = 15;
    icde_.elem_word_count_ = 1;
    icde_.msb_val_ = static_cast<double>(1 << (icde_.bit_count_ - 2));

    //                1   5          16 3 
    uint32_t bits = 0b00100001010110010011001101001011;
    int32_t val = 0b000101011001001;
    std::vector<uint32_t> input_words32{bits};
    std::vector<int32_t> output_words;
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_words, icde_));
    EXPECT_EQ(val, output_words.at(0));

    bits = 0b00101001010110010011001101001011;
    val = 0b000101011001001 * -1;
    input_words32[0] = bits;
    output_words.resize(0);
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_words, icde_));
    EXPECT_EQ(val, output_words.at(0));

    // double
    icde_.msb_val_ = icde_.msb_val_ / 10.0;
    ASSERT_TRUE(icdt_.TranslateArray(input_words32, output_eu_double_, icde_));
    EXPECT_EQ(LimitSigFigs(static_cast<double>(val) / 10.0), LimitSigFigs(output_eu_double_.at(0)));
}
