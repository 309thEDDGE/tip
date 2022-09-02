#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "arinc429_data.h"
#include "icd_element.h"
#include <unordered_map>
#include <vector>

class ARINC429DataTest : public ::testing::Test
{
   protected:
    ARINC429Data icd_data_;
    ICDElement elem;
    std::unordered_map<uint16_t,std::unordered_map<uint16_t, std::unordered_map<
            uint16_t,std::unordered_map<int8_t, size_t>>>> organized_lookup_map_;
    std::vector<std::vector<std::vector<ICDElement>>> element_table_;
    std::unordered_map<size_t,std::vector<std::string>> arinc_word_names_;

    size_t table_index_;
    size_t vector_index_;
    uint16_t chanid_ = 21;
    uint16_t subchan_ = 0;  // subchan = 0 to simulate bus number incrementing from 0 in ch10
    uint16_t label_ = 107;
    int8_t sdi_ = 1;

   public:
    ARINC429DataTest(){}
    // in functions below, subchan = 1 to simulate bus number incrementing from 1 in TMATS
    void SetupMap0()
    {
        // chanid=21,subchanid=1,label=107,sdi=1, index = 1
        organized_lookup_map_[21][1][107][1] = 1;
    }
    void SetupMap1()
    {
        // chanid=21,subchanid=1,label=107,sdi=1, index = 1
        organized_lookup_map_[21][1][107][-1] = 1;
    }

    void SetupWordNamesMap0()
    {
        // table_index = 3, vector_index = 0, name = "TestWord"
        arinc_word_names_[3].push_back("TestWord");
    }

    void SetupTable0()
    {
        std::vector<ICDElement> elem_vec;
        elem_vec.push_back(elem);
        std::vector<std::vector<ICDElement>> vec_elem_vec;
        vec_elem_vec.push_back(elem_vec);
        element_table_.push_back(vec_elem_vec);
    }

    void SetupElem()
    {
        elem.label_= 107;
        elem.sdi_= 1;            // 8-bit
        elem.bus_name_="SET1B";
        elem.msg_name_="TestWord";
        elem.rate_= 1.1F;
        elem.description_="Altitude";
        elem.xmit_lru_name_="LRU921";
        elem.elem_name_="107_alt";
        elem.schema_=ICDElementSchema::UNSIGNEDBITS;
        elem.is_bitlevel_=true;
        elem.bcd_partial_=-1;
        elem.msb_val_=1.0;
        elem.bitlsb_= 11;        // 8-bit
        elem.bit_count_= 8;      // 8-bit
        elem.uom_="FT";
        elem.classification_=0;  // 8-bit
    }
};

TEST_F(ARINC429DataTest, IdentifyWordEmptyMap)
{
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);
    bool item_found;

    item_found = icd_data_.IdentifyWord(table_index_,chanid_,subchan_,label_,sdi_);
    EXPECT_FALSE(item_found);
}

TEST_F(ARINC429DataTest, IdentifyWordEmptyTable)
{
    SetupMap0();
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);
    bool item_found;

    item_found = icd_data_.IdentifyWord(table_index_,chanid_,subchan_,label_,sdi_);
    EXPECT_FALSE(item_found);
}


TEST_F(ARINC429DataTest, IdentifyWordMissingChanid)
{
    SetupMap0();
    SetupTable0();
    bool item_found;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);

    // make bad channelid
    chanid_ = 2;

    item_found = icd_data_.IdentifyWord(table_index_,chanid_,subchan_,label_,sdi_);
    EXPECT_FALSE(item_found);
}

TEST_F(ARINC429DataTest, IdentifyWordMissingSubchanid)
{
    SetupMap0();
    SetupTable0();
    bool item_found;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);

    // make bad sub channelid
    subchan_ = 2;

    item_found = icd_data_.IdentifyWord(table_index_,chanid_,subchan_,label_,sdi_);
    EXPECT_FALSE(item_found);
}

TEST_F(ARINC429DataTest, IdentifyWordMissingLabel)
{
    SetupMap0();
    SetupTable0();
    bool item_found;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);

    // make bad sub channelid
    label_ = 2;

    item_found = icd_data_.IdentifyWord(table_index_,chanid_,subchan_,label_,sdi_);
    EXPECT_FALSE(item_found);
}

TEST_F(ARINC429DataTest, IdentifyWordMissingSDI)
{
    SetupMap0();
    SetupTable0();
    bool item_found;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);

    // make bad sub channelid
    sdi_ = 2;

    item_found = icd_data_.IdentifyWord(table_index_,chanid_,subchan_,label_,sdi_);
    EXPECT_FALSE(item_found);
}

TEST_F(ARINC429DataTest, IdentifyWordMissingSDINegative)
{
    SetupMap0();
    SetupTable0();
    bool item_found;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);

    // make bad sub channelid
    sdi_ = 2;

    item_found = icd_data_.IdentifyWord(table_index_,chanid_,subchan_,label_,sdi_);
    EXPECT_FALSE(item_found);
}

TEST_F(ARINC429DataTest, IdentifyWordPresentSDINegative)
{
    SetupMap1();
    SetupTable0();
    bool item_found;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);

    item_found = icd_data_.IdentifyWord(table_index_,chanid_,subchan_,label_,sdi_);
    EXPECT_TRUE(item_found);
}

TEST_F(ARINC429DataTest, IdentifyWordValidateOutput)
{
    SetupMap0();
    SetupTable0();
    bool item_found;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);

    item_found = icd_data_.IdentifyWord(table_index_,chanid_,subchan_,label_,sdi_);
    EXPECT_TRUE(item_found);
    EXPECT_EQ(table_index_,1);
}

TEST_F(ARINC429DataTest, IdentifyWordValidateOutputSDINegative)
{
    SetupMap1();
    SetupTable0();
    bool item_found;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);

    item_found = icd_data_.IdentifyWord(table_index_,chanid_,subchan_,label_,sdi_);
    EXPECT_TRUE(item_found);
    EXPECT_EQ(table_index_,1);
}

TEST_F(ARINC429DataTest, GetTableSizeEmptyTable)
{
    SetupMap0();
    size_t table_size;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);

    table_size = icd_data_.GetTableSize();
    EXPECT_EQ(table_size,0);
}

TEST_F(ARINC429DataTest, GetTableSizeValidateOutput)
{
    SetupMap0();
    SetupTable0();
    size_t table_size;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);

    table_size = icd_data_.GetTableSize();
    EXPECT_EQ(table_size,1);
}

TEST_F(ARINC429DataTest, GetWordElementsEmptyTable)
{
    SetupMap0();
    std::vector<std::vector<ICDElement>> arinc_elems;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);
    bool item_found;

    item_found = icd_data_.GetWordElements(table_index_,arinc_elems);
    EXPECT_FALSE(item_found);
}

TEST_F(ARINC429DataTest, GetWordElementsEmptyMap)
{
    std::vector<std::vector<ICDElement>> arinc_elems;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);
    bool item_found;

    item_found = icd_data_.GetWordElements(table_index_,arinc_elems);
    EXPECT_FALSE(item_found);
}

TEST_F(ARINC429DataTest, GetWordElementsTableIndexOutOfBounds)
{
    SetupMap0();
    SetupTable0();
    std::vector<std::vector<ICDElement>> arinc_elems;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);
    bool item_found;

    table_index_ = 2;

    item_found = icd_data_.GetWordElements(table_index_,arinc_elems);
    EXPECT_FALSE(item_found);
}

TEST_F(ARINC429DataTest, GetWordElementsValidateOutput)
{
    SetupMap0();
    SetupTable0();
    std::vector<std::vector<ICDElement>> arinc_elems;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);
    bool item_found;
    table_index_ = 0;
    item_found = icd_data_.GetWordElements(table_index_,arinc_elems);
    ASSERT_TRUE(item_found);

    EXPECT_EQ(arinc_elems.size(),element_table_[table_index_].size());
    EXPECT_EQ(arinc_elems[0].size(),element_table_[table_index_][0].size());

}

TEST_F(ARINC429DataTest, GetArincWordNamesEmptyMap)
{
    SetupMap0();
    SetupTable0();
    std::vector<std::vector<ICDElement>> arinc_elems;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_);
    bool item_found;
    table_index_ = 0;
    vector_index_ = 0;
    std::string word_name;
    item_found = icd_data_.GetArincWordNames(table_index_,vector_index_,word_name);
    ASSERT_FALSE(item_found);
}

TEST_F(ARINC429DataTest, GetArincWordNamesBadTableIndex)
{
    SetupMap0();
    SetupTable0();
    SetupWordNamesMap0();
    std::vector<std::vector<ICDElement>> arinc_elems;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_,arinc_word_names_);
    bool item_found;
    table_index_ = 0;
    vector_index_ = 0;
    std::string word_name;
    item_found = icd_data_.GetArincWordNames(table_index_,vector_index_,word_name);
    ASSERT_FALSE(item_found);
}

TEST_F(ARINC429DataTest, GetArincWordNamesBadVectorIndex)
{
    SetupMap0();
    SetupTable0();
    SetupWordNamesMap0();
    std::vector<std::vector<ICDElement>> arinc_elems;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_,arinc_word_names_);
    bool item_found;
    table_index_ = 3;
    vector_index_ = 3;
    std::string word_name;
    item_found = icd_data_.GetArincWordNames(table_index_,vector_index_,word_name);
    ASSERT_FALSE(item_found);
}

TEST_F(ARINC429DataTest, GetArincWordNamesValidateOutput)
{
    SetupMap0();
    SetupTable0();
    SetupWordNamesMap0();
    std::vector<std::vector<ICDElement>> arinc_elems;
    icd_data_ = ARINC429Data(organized_lookup_map_, element_table_,arinc_word_names_);
    bool item_found;
    table_index_ = 3;
    vector_index_ = 0;
    std::string word_name;
    item_found = icd_data_.GetArincWordNames(table_index_,vector_index_,word_name);
    ASSERT_TRUE(item_found);
    EXPECT_EQ(word_name, "TestWord");
}

TEST_F(ARINC429DataTest, SSMSignForBCDNoComputedData)
{
    uint8_t ssm = 1;  // NCD
    int8_t sign = 8;

    ASSERT_FALSE(icd_data_.SSMSignForBCD(ssm, sign));
    EXPECT_EQ(sign, 0);
}

TEST_F(ARINC429DataTest, SSMSignForBCDFunctionalTest)
{
    uint8_t ssm = 2;  // Functional test
    int8_t sign = 8;

    ASSERT_FALSE(icd_data_.SSMSignForBCD(ssm, sign));
    EXPECT_EQ(sign, 0);
}

TEST_F(ARINC429DataTest, SSMSignForBCDPositive)
{
    uint8_t ssm = 0;  // positive
    int8_t sign = 8;

    ASSERT_TRUE(icd_data_.SSMSignForBCD(ssm, sign));
    EXPECT_EQ(sign, 1);
}

TEST_F(ARINC429DataTest, SSMSignForBCDNegative)
{
    uint8_t ssm = 3;  // negative
    int8_t sign = 8;

    ASSERT_TRUE(icd_data_.SSMSignForBCD(ssm, sign));
    EXPECT_EQ(sign, -1);
}

TEST_F(ARINC429DataTest, SetNamesToUniqueIndexMapEmptyMap)
{
    std::unordered_map<size_t, std::vector<std::string>> word_names;
    std::unordered_map<std::string, size_t> word_name_to_unique_index_map;

    ASSERT_FALSE(icd_data_.SetNamesToUniqueIndexMap(word_names, word_name_to_unique_index_map));

}


TEST_F(ARINC429DataTest, SetNamesToUniqueIndexMapOutputValid)
{
    std::unordered_map<size_t, std::vector<std::string>> word_names;
    std::vector<std::string> temp_vector;
    std::unordered_map<std::string, size_t> word_name_to_unique_index_map;

    temp_vector.push_back("One");
    temp_vector.push_back("Two");
    word_names[1] = temp_vector;
    temp_vector.clear();
    temp_vector.push_back("Three");
    word_names[2] = temp_vector;

    ASSERT_TRUE(icd_data_.SetNamesToUniqueIndexMap(word_names, word_name_to_unique_index_map));
    ASSERT_EQ(3, word_name_to_unique_index_map.size());

}