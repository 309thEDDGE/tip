#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "icd_element.h"

TEST(ICDElementTest, FillEmptyInputString)
{
    // Return false if empty input string.
    ICDElement ice;
    std::string test_str = "";
    ASSERT_FALSE(ice.Fill(test_str));
}

TEST(ICDElementTest, FillNoCommaInputString)
{
    // Return false if no commas in input string.
    ICDElement ice;
    std::string test_str = "val1|val2;val3 va4	VAL6\n";
    ASSERT_FALSE(ice.Fill(test_str));
}

TEST(ICDElementTest, FillIncorrectElementCount)
{
    // Return false if incorrect element count.
    ICDElement ice;

    // 26 elements with trailing comma
    std::string test_str = "A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,";
    ASSERT_FALSE(ice.Fill(test_str));

    // 24 elements without trailing comma
    test_str = "A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X";
    ASSERT_FALSE(ice.Fill(test_str));

    // 24 elements with trailing comma
    test_str = "A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,";
    ASSERT_FALSE(ice.Fill(test_str));
}

// This also tests the private function ICDElement::FillElements().
TEST(ICDElementTest, FillCorrectElementCount)
{
    // Return true if correct element count.
    ICDElement ice;

    // 25 elements without trailing comma
    // Note: This is also an example of a correct string.
    std::string test_str =
        "BL22U,BL22U-20,00000,39030,22,BD3,"
        "MIVU,23,ORT,19,00,03,12.50,19,02,03,0,0,02,00,00,0,"
        "description 5,17374824.000000000000,NONE";
    ASSERT_TRUE(ice.Fill(test_str));

    // 25 elements with trailing comma
    test_str =
        "BL22U,BL22U-20,00000,39030,22,BD3,"
        "MIVU,23,ORT,19,00,03,12.50,19,02,03,0,0,02,00,00,0,"
        "description 5,17374824.000000000000,NONE,";
    ASSERT_TRUE(ice.Fill(test_str));
}

// This also tests the private function ICDElement::FillElements().
TEST(ICDElementTest, FillIncorrectElementTypes)
{
    // Return false if the elements can not be casted into the
    // expected data types.
    ICDElement ice;

    // Everything but 3rd element correct; ought to be uint16.
    std::string test_str =
        "BL22U,BL22U-20,fff,39030,22,BD3,"
        "MIVU,23,ORT,19,00,03,12.50,19,02,03,0,0,02,00,00,0,"
        "description 5,17374824.000000000000,NONE";
    ASSERT_FALSE(ice.Fill(test_str));

    // Element that is integer in the first test (19),
    // is float here, which is incorrect.
    test_str =
        "BL22U,BL22U-20,00000,39030,22,BD3,"
        "MIVU,23,ORT,19,00,03,12.50,19.0,02,03,0,0,02,00,00,0,"
        "description 5,17374824.000000000000,NONE,";
    ASSERT_FALSE(ice.Fill(test_str));

    // Note: test doesn't cover case in which a definite float
    // value is actually an int. But this is hard to test because
    // the msb value can be an int value that needs to be
    // read as a float so we don't want the cast from a value that would
    // normally be read as int to a float to fail.
}

// This also tests the private function ICDElement::FillElements().
TEST(ICDElementTest, FillCorrectElementTypes)
{
    // Return true if the elements can be casted into the
    // expected data types.
    ICDElement ice;

    // Everything correct
    std::string test_str =
        "BL22U,BL22U-20,00000,39030,22,BD3,"
        "MIVU,23,ORT,19,00,03,12.50,19,02,03,0,0,02,00,00,0,"
        "description 5,17374824.000000000000,NONE";
    ASSERT_TRUE(ice.Fill(test_str));
}

TEST(ICDElementTest, FillInvalidBitValues)
{
    ICDElement ice;

    // Everything correct, except bitmsb which is 0 and is_bitlevel = 1.
    // Bitmsb and bitlsb must be in [1, 16].
    std::string test_str =
        "BL22U,BL22U-20,00000,39030,22,BD3,"
        "MIVU,23,ORT,19,00,03,12.50,19,02,03,1,0,00,5,00,0,"
        "description 5,17374824.000000000000,NONE";
    EXPECT_FALSE(ice.Fill(test_str));

    // Everything correct, except bitlsb which is 50 and is_bitlevel = 1.
    // Bitmsb and bitlsb must be in [1, 16].
    test_str =
        "BL22U,BL22U-20,00000,39030,22,BD3,"
        "MIVU,23,ORT,19,00,03,12.50,19,02,03,1,0,12,50,00,0,"
        "description 5,17374824.000000000000,NONE";
    EXPECT_FALSE(ice.Fill(test_str));
}