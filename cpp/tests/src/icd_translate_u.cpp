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
		printf("n_values %zu, dshift1 %hu, dshift2 %hu, mask1 %hu, mask2 %hu, mask1_twos %hu, "
			"mask2_twos %hu, scale %llf, scale_twos %llf, sign_bit_mask %hu\n", n_values, dshift1, dshift2, mask1,
			mask2, mask1_twos, mask2_twos, scale, scale_twos, sign_bit_mask);
	}

	double LimitSigFigs(double val)
	{
		double output = 0.0;
		double shift = double(4 - uint64_t(floor(log10(val) + 1)));
		output = double(uint64_t(val * pow(10.0, shift))) / pow(10, shift);
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
	input_words_.push_back((13 << 9) + 123); // 13
	input_words_.push_back((42 << 9) + 342); // 10

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
	input_words_.push_back(91); // 372736
	input_words_.push_back(906); // 56

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

// Note: multiple words for unsigned bits not tested. Also not believed to be present in ICD.

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
	input_words_.push_back(13 << 11); // -3
	input_words_.push_back(3 << 11); // 3
	input_words_.push_back(11 << 11); // -5

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
	input_words_.push_back(18); // 10010
	input_words_.push_back(6 << 11); // 00110
	input_words_.push_back(10); // 01010
	input_words_.push_back(25 << 11); // 11001

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

// No test for SIGNEDBITS, multiple words, though these values are 
// not found in ICD.

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
	input_words_.push_back((1 << 15) + 37); // -32731
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
	input_words_.push_back(1035); // 67829760
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
	EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(-4.15222e-10));
	output_eu_double_.resize(0);
	bool res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
	EXPECT_EQ(LimitSigFigs(output_eu_double_[0]), LimitSigFigs(-4.15222e-10));
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
	input_words_[0] = (1 << 15) + 33; // exponent sign bit high
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
	input_words_[0] = 83; // < 1 << 7
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
	input_words_[0] = 512; // first 7 bits == 0
	input_words_[1] = 897;
	input_words_[2] = (1 << 15) + 4459;
	icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
	icdt_.TranslateCAPS(input_words_, output_eu_double_);
	EXPECT_TRUE(isnan(output_eu_double_[0]));
	output_eu_double_.resize(0);
	res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
	EXPECT_TRUE(isnan(output_eu_double_[0]));

	// Values in [0, 0.5).
	input_words_[0] = 3072; // first 7 bits == 0
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
	input_words_[0] = ((1 << 8) - 1) << 7; // All exponent bits high, all fracion bits low.
	input_words_[1] = 0; // All other fraction bits low.
	icdt_.ConfigureWordLevel(input_words_.size(), output_eu_float_, icde_);
	icdt_.TranslateFloat32IEEE(input_words_, output_eu_float_);
	EXPECT_TRUE(isinf(output_eu_float_[0]));
	output_eu_float_.resize(0);
	res = icdt_.TranslateArrayOfElement(input_words_, output_eu_float_, icde_);
	EXPECT_TRUE(isinf(output_eu_float_[0]));

	output_eu_float_.resize(0);
	input_words_[0] += 1 << 15; // add sign bit for neg inf.
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
	input_words_[0] += (1 << 15); // Add sign bit
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
	input_words_[0] = ((1 << 11) - 1) << 4; // All exponent bits high, all fracion bits low.
	input_words_[1] = 0; // All other fraction bits low.
	input_words_[2] = 0;
	input_words_[3] = 0;
	icdt_.ConfigureWordLevel(input_words_.size(), output_eu_double_, icde_);
	icdt_.TranslateFloat64IEEE(input_words_, output_eu_double_);
	EXPECT_TRUE(isinf(output_eu_double_[0]));
	output_eu_double_.resize(0);
	res = icdt_.TranslateArrayOfElement(input_words_, output_eu_double_, icde_);
	EXPECT_TRUE(isinf(output_eu_double_[0]));

	output_eu_double_.resize(0);
	input_words_[0] += 1 << 15; // add sign bit for neg inf.
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
	input_words_[0] += (1 << 15); // Add sign bit
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