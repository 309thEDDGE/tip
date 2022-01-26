#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "icd_element.h"
#include "icd_data.h"

// Needed for ICDDataRealICDQueryTest
#include "file_reader.h"

class ICDDataTest : public ::testing::Test
{
   protected:
    ICDData icd_;
    bool res_;
    std::string msg_name_;

    ICDDataTest() : res_(false), msg_name_("")
    {
    }
};

TEST_F(ICDDataTest, FillElementsAcceptsNumberAsStringElement)
{
    // Fields 0, 1, 5, 6, 8, 22, 24 are string data

    std::string test_string =
        "40,3.14,00000,39030,22,"
        "8000,5.5,23,42,19,"
        "00,03,12.50,19,02,"
        "03,0,0,02,00,"
        "00,0,89,17374824.000000000000,80.0";

    ICDElement icd_element;
    ParseText parser;
    std::vector<std::string> fields = parser.Split(test_string, ',');
    ASSERT_TRUE(icd_element.FillElements(fields));
}

TEST_F(ICDDataTest, IngestICDTextFileLinesEmptyVector)
{
    std::vector<std::string> lines;
    std::vector<ICDElement> elems;
    ASSERT_FALSE(icd_.IngestICDTextFileLines(lines, elems));
}

TEST_F(ICDDataTest, IngestICDTextFileLinesNonEmptyOutputVector)
{
    std::vector<std::string> lines;
    std::vector<ICDElement> elems;
    elems.push_back(ICDElement());
    ASSERT_FALSE(icd_.IngestICDTextFileLines(lines, elems));
}

TEST_F(ICDDataTest, IngestICDTextFileLinesHeaderColCount)
{
    // Correct header count, currently 25 elements.
    std::vector<std::string> lines;
    lines.push_back("msg_name,elem_name,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y");
    std::vector<ICDElement> elems;
    EXPECT_TRUE(icd_.IngestICDTextFileLines(lines, elems));

    // Header only and no data. Elems will not be filled.
    EXPECT_EQ(elems.size(), 0);

    // Incorrect header count.
    lines[0] = "msg_name,elem_name,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X";
    ASSERT_FALSE(icd_.IngestICDTextFileLines(lines, elems));
}

TEST_F(ICDDataTest, IngestICDTextFileLinesHeaderColName)
{
    // Only care about correct values for the first two column headers.

    // Here, the second column name is incorrect; it
    // ought to be elem_name.
    std::vector<std::string> lines;
    lines.push_back("msg_name,elemname,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y");
    std::vector<ICDElement> elems;
    EXPECT_FALSE(icd_.IngestICDTextFileLines(lines, elems));

    // Second column is now correct.
    lines[0] = "msg_name,elem_name,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y";
    EXPECT_TRUE(icd_.IngestICDTextFileLines(lines, elems));
}

TEST_F(ICDDataTest, IngestICDTextFileLinesExpectedElementCount)
{
    // Only care about correct values for the first two column headers.

    // Fill actual header line and additional data lines.
    std::vector<std::string> lines;
    lines.push_back(
        "msg_name,elem_name,xmit_word,dest_word,msg_word_count,"
        "bus_name,xmit_lru_name,xmit_lru_addr,dest_lru_name,dest_lru_addr,"
        "xmit_subaddr,dest_subaddr,rate,offset,elem_word_count,schema,isbit,"
        "ismulti,bitmsb,bitlsb,bitcount,classification,description,msbval,uom\n");
    lines.push_back(
        "BL22U,BL22U-17,00000,39030,22,BD3,MIVU,23,ORT,19,00,03,12.50,16,"
        "02,03,0,0,02,00,00,0,description 5,17374824.000000000000,NONE\n");
    lines.push_back(
        "CX53_Z_8,CX53_Z_8-04,00000,18525,29,M1980,MIVU,23,WS9,09,00,02,"
        "0.00,03,01,04,0,0,01,00,00,0,description 6,"
        "376.000000000000,USEC\n");
    lines.push_back(
        "ML00_6,ML00_6-0607,00000,28935,07,BD3,MIVU,23,BLATS,14,00,08,0.00,05,"
        "01,12,1,0,07,07,01,0,description 5,1.000000000000,NONE\n");

    std::vector<ICDElement> elems;
    EXPECT_TRUE(icd_.IngestICDTextFileLines(lines, elems));
    ASSERT_EQ(elems.size(), 3);
}

TEST_F(ICDDataTest, IngestICDTextFileLinesMalformedICDLine)
{
    // Only care about correct values for the first two column headers.

    // Fill actual header line and additional data lines.
    std::vector<std::string> lines;
    lines.push_back(
        "msg_name,elem_name,xmit_word,dest_word,msg_word_count,"
        "bus_name,xmit_lru_name,xmit_lru_addr,dest_lru_name,dest_lru_addr,"
        "xmit_subaddr,dest_subaddr,rate,offset,elem_word_count,schema,isbit,"
        "ismulti,bitmsb,bitlsb,bitcount,classification,description,msbval,uom\n");
    // Destination command word not interpretable as integer.
    lines.push_back(
        "BL22U,BL22U-17,00000,int,22,BD3,MIVU,23,ORT,19,00,03,12.50,16,"
        "02,03,0,0,02,00,00,0,description 5,17374824.000000000000,NONE\n");

    // Rate column not interpretable as float.
    lines.push_back(
        "CX53_Z_8,CX53_Z_8-04,00000,18525,29,M1980,MIVU,23,WS9,09,00,02,"
        "float,03,01,04,0,0,01,00,00,0,description,"
        "376.000000000000,USEC\n");
    lines.push_back(
        "ML00_6,ML00_6-0607,00000,28935,07,BD3,MIVU,23,BLATS,14,00,08,0.00,05,"
        "01,12,1,0,07,07,01,0,description 6,1.000000000000,NONE\n");

    std::vector<ICDElement> elems;
    EXPECT_TRUE(icd_.IngestICDTextFileLines(lines, elems));
    ASSERT_EQ(elems.size(), 1);
}

TEST_F(ICDDataTest, PrepareMessageKeyMap)
{
    IterableTools iterable_tools;
    // Fill ICD lines.
    std::vector<std::string> temp_lines = {
        "msg_name,elem_name,xmit_word,dest_word,msg_word_count,bus_name,xmit_lru_name,xmit_lru_addr,dest_lru_name,dest_lru_addr,xmit_subaddr,dest_subaddr,rate,offset,elem_word_count,schema,isbit,ismulti,bitmsb,bitlsb,bitcount,classification,description,msbval,uom\n",
        "DG6_N,DG6_N-01,00000,39043,03,Bus1,MIVU,23,ORT,19,00,04,1.00,00,02,05,0,0,01,00,00,0,description,21474348.000000000000,SECONDS",
        "DG6_N,DG6_N-0301,00000,39043,03,Bus2,MIVU,23,ORT,19,00,04,1.00,02,01,12,1,0,01,09,09,0,description description,1.000000000000,DAY",
        "DG6_N,DG6_N-0310,00000,39043,03,Bus3,MIVU,23,ORT,19,00,04,1.00,02,01,12,1,0,10,15,06,0,description,3.000000000000,YEAR",
        "LL3_MD09,LL3_MD09-20@09,48192,55360,32,Bus3,MIVU,23,UDTU,27,02,02,0.00,19,02,03,0,1,02,00,00,0,description,17374824.000000000000,FEET",
        "LL3_MD09,LL3_MD09-21@07,48192,55360,32,Bus4,MIVU,23,UDTU,27,02,02,0.00,20,02,05,0,1,01,00,00,0,description,21474348.000000000000,NONE\n",
        "LL3_MD09,LL3_MD09-22@03,48192,55360,32,Bus5,MIVU,23,UDTU,27,02,02,0.00,21,02,03,0,1,02,00,00,0,description description,0.500000000000,SC",
        "DTI_14G,DTI_14G-04@02,00000,55360,32,Bus1,CDU,22,UDTU,27,00,02,0.00,03,01,04,0,1,01,00,00,0,description,37.000000000000,NONE",
        "DTI_14G,DTI_14G-05@01,00000,55360,32,Bus2,CDU,22,UDTU,27,00,02,0.00,04,01,04,0,1,01,00,00,0,description description,37.000000000000,MVOLT",
        "GG01_H,GG01_H-0211,28894,00000,30,Bus1,MIVU,23,BLATS,14,00,06,0.00,01,01,12,1,0,11,11,01,0,description description,1.000000000000,NONE\n",
        "GG01_H,GG01_H-0212,28894,00000,30,Bus1,MIVU,23,BLATS,14,00,06,0.00,01,01,12,1,0,12,12,01,0,description description,1.000000000000,NONE\n",
        "GG01_H,GG01_H-0213,28894,00000,30,Bus2,MIVU,23,BLATS,14,00,06,0.00,01,01,12,1,0,13,13,01,0,description description,1.000000000000,NONE",
        "RPG4_I,RPG4_I-01,00000,39043,20,Bus4,MIVU,23,ORT,19,00,04,1.00,00,02,05,0,0,01,00,00,0,description description,21474348.000000000000,SECONDS"};

    ASSERT_TRUE(icd_.PrepareICDQuery(temp_lines));

    std::unordered_map<uint64_t, std::set<std::string>> message_key_map;
    std::map<std::string, std::set<uint64_t>> suppl_map;
    suppl_map["BusA"] = std::set<uint64_t>({39043, 999});
    suppl_map["BusB"] = std::set<uint64_t>({55360, 888});
    icd_.PrepareMessageKeyMap(message_key_map, suppl_map);
    int64_t key1 = 39043;
    int64_t key2 = 48192 << 16 | 55360;
    int64_t key3 = 55360;
    int64_t key4 = 28894 << 16;

    // Ensure four keys were found
    ASSERT_EQ(message_key_map.size(), 6);

    // Ensure each key was found
    ASSERT_EQ(message_key_map.count(key1), 1);
    ASSERT_EQ(message_key_map.count(key2), 1);
    ASSERT_EQ(message_key_map.count(key3), 1);
    ASSERT_EQ(message_key_map.count(key4), 1);

    // Check for supplement map keys
    ASSERT_EQ(message_key_map.count(999), 1);
    ASSERT_EQ(message_key_map.count(888), 1);

    ASSERT_THAT(message_key_map[key1],
                ::testing::ElementsAre("Bus1", "Bus2", "Bus3", "Bus4", "BusA"));

    ASSERT_THAT(message_key_map[key2],
                ::testing::ElementsAre("Bus3", "Bus4", "Bus5"));

    ASSERT_THAT(message_key_map[key3],
                ::testing::ElementsAre("Bus1", "Bus2", "BusB"));

    ASSERT_THAT(message_key_map[key4],
                ::testing::ElementsAre("Bus1", "Bus2"));

    ASSERT_THAT(message_key_map[999],
                ::testing::ElementsAre("BusA"));

    ASSERT_THAT(message_key_map[888],
                ::testing::ElementsAre("BusB"));
}

//
// PrepareICDQuery-relevant functions.
//

class ICDDataPrepareICDQueryTest : public ::testing::Test
{
   protected:
    ICDData icd_;
    bool res_;
    std::string bus_name_;
    std::vector<std::string> icd_lines_;
    std::vector<ICDElement> icd_elems_;
    std::vector<ICDElement> icd_msg_elems_;
    std::vector<size_t> inds_vec_;
    std::vector<std::vector<size_t>> table_indices_;
    std::vector<std::string> table_names_;
    std::map<std::string, std::set<uint64_t>> bus_lruaddrs_map_;
    std::map<std::string, std::set<uint64_t>> update_map_;

    ICDDataPrepareICDQueryTest() : res_(false) {}
    void SetUp() override
    {
        // Fill ICD lines.
        std::vector<std::string> temp_lines = {
            "msg_name,elem_name,xmit_word,dest_word,msg_word_count,bus_name,xmit_lru_name,xmit_lru_addr,dest_lru_name,dest_lru_addr,xmit_subaddr,dest_subaddr,rate,offset,elem_word_count,schema,isbit,ismulti,bitmsb,bitlsb,bitcount,classification,description,msbval,uom\n",
            "DG6_N,DG6_N-01,00000,39043,03,BD3,MIVU,23,ORT,19,00,04,1.00,00,02,05,0,0,01,00,00,0,description,21474348.000000000000,SECONDS",
            "DG6_N,DG6_N-0301,00000,39043,03,BD3,MIVU,23,ORT,19,00,04,1.00,02,01,12,1,0,01,09,09,0,description description,1.000000000000,DAY",
            "DG6_N,DG6_N-0310,00000,39043,03,BD3,MIVU,23,ORT,19,00,04,1.00,02,01,12,1,0,10,15,06,0,description,3.000000000000,YEAR",
            "LL3_MD09,LL3_MD09-20@09,48192,55360,32,BD1,MIVU,23,UDTU,27,02,02,0.00,19,02,03,0,1,02,00,00,0,description,17374824.000000000000,FEET",
            "LL3_MD09,LL3_MD09-21@07,48192,55360,32,BD1,MIVU,23,UDTU,27,02,02,0.00,20,02,05,0,1,01,00,00,0,description,21474348.000000000000,NONE\n",
            "LL3_MD09,LL3_MD09-22@03,48192,55360,32,BD1,MIVU,23,UDTU,27,02,02,0.00,21,02,03,0,1,02,00,00,0,description description,0.500000000000,SC",
            "DTI_14G,DTI_14G-04@02,00000,55360,32,BD1,CDU,22,UDTU,27,00,02,0.00,03,01,04,0,1,01,00,00,0,description,37.000000000000,NONE",
            "DTI_14G,DTI_14G-05@01,00000,55360,32,BD1,CDU,22,UDTU,27,00,02,0.00,04,01,04,0,1,01,00,00,0,description description,37.000000000000,MVOLT",
            "GG01_H,GG01_H-0211,00000,28894,30,BD3,MIVU,23,BLATS,14,00,06,0.00,01,01,12,1,0,11,11,01,0,description description,1.000000000000,NONE\n",
            "GG01_H,GG01_H-0212,00000,28894,30,BD3,MIVU,23,BLATS,14,00,06,0.00,01,01,12,1,0,12,12,01,0,description description,1.000000000000,NONE\n",
            "GG01_H,GG01_H-0213,00000,28894,30,BD3,MIVU,23,BLATS,14,00,06,0.00,01,01,12,1,0,13,13,01,0,description description,1.000000000000,NONE",
            "RPG4_I,RPG4_I-01,00000,39043,20,BD3,MIVU,23,ORT,19,00,04,1.00,00,02,05,0,0,01,00,00,0,description description,21474348.000000000000,SECONDS"};
        icd_lines_ = temp_lines;
        // Ingest lines.
        //res_ = icd_.IngestICDTextFileLines(temp_lines);
    }
};

TEST_F(ICDDataPrepareICDQueryTest, CreateTablesICDElemsNotCreated)
{
    res_ = icd_.CreateTables(icd_elems_, table_indices_);
    ASSERT_EQ(res_, false);
}

TEST_F(ICDDataPrepareICDQueryTest, CreateTablesCorrectInds)
{
    res_ = icd_.IngestICDTextFileLines(icd_lines_, icd_elems_);
    EXPECT_EQ(res_, true);

    res_ = icd_.CreateTables(icd_elems_, table_indices_);
    EXPECT_EQ(res_, true);

    // Check the table_indices.
    EXPECT_THAT(table_indices_[0], ::testing::ElementsAre(0, 1, 2));
    EXPECT_THAT(table_indices_[3], ::testing::ElementsAre(3, 4, 5));
    EXPECT_THAT(table_indices_[1], ::testing::ElementsAre(6, 7));
    EXPECT_THAT(table_indices_[2], ::testing::ElementsAre(8, 9, 10));
}

TEST_F(ICDDataPrepareICDQueryTest, CollectMsgElementsFromTableIndsEmptyICDElems)
{
    res_ = icd_.CollectMsgElementsFromTableInds(icd_elems_, table_indices_, icd_msg_elems_);
    EXPECT_TRUE(icd_elems_.size() == 0);
    EXPECT_TRUE(table_indices_.size() == 0);
    EXPECT_EQ(res_, false);
}

TEST_F(ICDDataPrepareICDQueryTest, CollectMsgElementsFromTableIndsEmptyTableInds)
{
    res_ = icd_.IngestICDTextFileLines(icd_lines_, icd_elems_);
    EXPECT_EQ(res_, true);
    EXPECT_TRUE(icd_elems_.size() > 0);
    EXPECT_TRUE(table_indices_.size() == 0);

    res_ = icd_.CollectMsgElementsFromTableInds(icd_elems_, table_indices_, icd_msg_elems_);
    EXPECT_EQ(res_, false);
}

TEST_F(ICDDataPrepareICDQueryTest, CollectMsgElementsFromTableIndsNonZeroICDMsgElems)
{
    res_ = icd_.IngestICDTextFileLines(icd_lines_, icd_elems_);
    EXPECT_EQ(res_, true);
    EXPECT_EQ(icd_elems_.size(), 12);

    res_ = icd_.CreateTables(icd_elems_, table_indices_);
    EXPECT_EQ(res_, true);
    EXPECT_EQ(table_indices_.size(), 5);

    icd_msg_elems_.push_back(icd_elems_[0]);
    res_ = icd_.CollectMsgElementsFromTableInds(icd_elems_, table_indices_, icd_msg_elems_);
    EXPECT_EQ(res_, false);
}

TEST_F(ICDDataPrepareICDQueryTest, CollectMsgElementsFromTableIndsCorrectElemCount)
{
    res_ = icd_.IngestICDTextFileLines(icd_lines_, icd_elems_);
    EXPECT_EQ(res_, true);

    res_ = icd_.CreateTables(icd_elems_, table_indices_);
    EXPECT_EQ(res_, true);

    res_ = icd_.CollectMsgElementsFromTableInds(icd_elems_, table_indices_, icd_msg_elems_);
    EXPECT_EQ(res_, true);
    // There must be a message corresponding to each table.
    EXPECT_EQ(icd_msg_elems_.size(), table_indices_.size());
}

TEST_F(ICDDataPrepareICDQueryTest, CollectMsgElementsFromTableIndsCorrectElemName)
{
    res_ = icd_.IngestICDTextFileLines(icd_lines_, icd_elems_);
    EXPECT_EQ(res_, true);

    res_ = icd_.CreateTables(icd_elems_, table_indices_);
    EXPECT_EQ(res_, true);

    res_ = icd_.CollectMsgElementsFromTableInds(icd_elems_, table_indices_, icd_msg_elems_);
    EXPECT_EQ(res_, true);
    EXPECT_EQ(icd_msg_elems_[0].msg_name_, icd_elems_[table_indices_[0][0]].msg_name_);
    EXPECT_EQ(icd_msg_elems_[1].msg_name_, icd_elems_[table_indices_[1][0]].msg_name_);
    EXPECT_EQ(icd_msg_elems_[2].msg_name_, icd_elems_[table_indices_[2][0]].msg_name_);
    EXPECT_EQ(icd_msg_elems_[3].msg_name_, icd_elems_[table_indices_[3][0]].msg_name_);
    EXPECT_EQ(icd_msg_elems_[4].msg_name_, icd_elems_[table_indices_[4][0]].msg_name_);
}

TEST_F(ICDDataPrepareICDQueryTest, CreateTableNameLookupValidInput)
{
    // Empty input elements.
    res_ = icd_.CreateTableNameLookup(icd_elems_, table_names_);
    EXPECT_EQ(res_, false);

    res_ = icd_.IngestICDTextFileLines(icd_lines_, icd_elems_);
    EXPECT_EQ(res_, true);

    // Non-zero table names output vector.
    table_names_.push_back("test");
    res_ = icd_.CreateTableNameLookup(icd_elems_, table_names_);
    EXPECT_EQ(res_, false);
}

TEST_F(ICDDataPrepareICDQueryTest, CreateTableNameLookupCorrectNames)
{
    res_ = icd_.IngestICDTextFileLines(icd_lines_, icd_elems_);
    EXPECT_EQ(res_, true);

    res_ = icd_.CreateTables(icd_elems_, table_indices_);
    EXPECT_EQ(res_, true);

    res_ = icd_.CollectMsgElementsFromTableInds(icd_elems_, table_indices_, icd_msg_elems_);
    EXPECT_EQ(res_, true);

    res_ = icd_.CreateTableNameLookup(icd_msg_elems_, table_names_);
    EXPECT_EQ(res_, true);

    EXPECT_EQ(icd_msg_elems_[0].msg_name_, table_names_[0]);
    EXPECT_EQ(icd_msg_elems_[1].msg_name_, table_names_[1]);
    EXPECT_EQ(icd_msg_elems_[2].msg_name_, table_names_[2]);
    EXPECT_EQ(icd_msg_elems_[3].msg_name_, table_names_[3]);
    EXPECT_EQ(icd_msg_elems_[4].msg_name_, table_names_[4]);
}

TEST_F(ICDDataPrepareICDQueryTest, CreateBusNameToLRUAddressSetMapValidInput)
{
    // Empty input elements.
    res_ = icd_.CreateBusNameToLRUAddressSetMap(icd_msg_elems_, bus_lruaddrs_map_);
    EXPECT_EQ(res_, false);

    res_ = icd_.IngestICDTextFileLines(icd_lines_, icd_elems_);
    EXPECT_EQ(res_, true);

    res_ = icd_.CreateTables(icd_elems_, table_indices_);
    EXPECT_EQ(res_, true);

    res_ = icd_.CollectMsgElementsFromTableInds(icd_elems_, table_indices_, icd_msg_elems_);
    EXPECT_EQ(res_, true);

    // Non-zero output map size.
    bus_lruaddrs_map_["test"] = std::set<uint64_t>({10, 100});
    res_ = icd_.CreateBusNameToLRUAddressSetMap(icd_msg_elems_, bus_lruaddrs_map_);
    EXPECT_EQ(res_, false);
}

TEST_F(ICDDataPrepareICDQueryTest, CreateBusNameToLRUAddressSetMapCorrectMap)
{
    res_ = icd_.IngestICDTextFileLines(icd_lines_, icd_elems_);
    EXPECT_EQ(res_, true);

    res_ = icd_.CreateTables(icd_elems_, table_indices_);
    EXPECT_EQ(res_, true);

    res_ = icd_.CollectMsgElementsFromTableInds(icd_elems_, table_indices_, icd_msg_elems_);
    EXPECT_EQ(res_, true);

    res_ = icd_.CreateBusNameToLRUAddressSetMap(icd_msg_elems_, bus_lruaddrs_map_);
    EXPECT_EQ(res_, true);

    EXPECT_THAT(bus_lruaddrs_map_["BD1"], ::testing::ElementsAre(22, 23, 27));
    EXPECT_THAT(bus_lruaddrs_map_["BD3"], ::testing::ElementsAre(14, 19, 23));
}

TEST_F(ICDDataPrepareICDQueryTest, UpdateMapFromICDElementsToIndices)
{
    res_ = icd_.IngestICDTextFileLines(icd_lines_, icd_elems_);
    EXPECT_EQ(res_, true);

    res_ = icd_.CreateTables(icd_elems_, table_indices_);
    EXPECT_EQ(res_, true);

    res_ = icd_.CollectMsgElementsFromTableInds(icd_elems_, table_indices_, icd_msg_elems_);
    EXPECT_EQ(res_, true);

    res_ = icd_.CreateTableNameLookup(icd_msg_elems_, table_names_);

    std::unordered_map<uint8_t, std::vector<ICDElement>> dsub_elem_map;
    std::vector<ICDElement> temp_elems;
    temp_elems.push_back(icd_msg_elems_[1]);
    temp_elems.push_back(icd_msg_elems_[2]);
    dsub_elem_map[2] = temp_elems;
    std::unordered_map<uint8_t, std::set<size_t>> dsub_inds_map;
    icd_.UpdateMapFromICDElementsToIndices(dsub_elem_map, table_names_, dsub_inds_map);
    EXPECT_EQ(dsub_inds_map.size(), 1);
    EXPECT_THAT(dsub_inds_map[2], ::testing::ElementsAre(1, 2));
}

TEST_F(ICDDataPrepareICDQueryTest, CreateLookupMapCorrectMap)
{
    // Call public function PrepareICDQuery() which calls the
    // the necessary functions in order prior to calling
    // CreateLookupMap();
    res_ = icd_.PrepareICDQuery(icd_lines_);
    ASSERT_EQ(res_, true);

    // Use TempLookupTableIndex to query the map.
    bus_name_ = "BD3";
    EXPECT_THAT(icd_.TempLookupTableIndex(bus_name_, 23, 19, 0, 4), ::testing::ElementsAre(0, 4));
    EXPECT_THAT(icd_.TempLookupTableIndex(bus_name_, 23, 14, 0, 6), ::testing::ElementsAre(2));
    bus_name_ = "BD1";
    EXPECT_THAT(icd_.TempLookupTableIndex(bus_name_, 22, 27, 0, 2), ::testing::ElementsAre(1));
    EXPECT_THAT(icd_.TempLookupTableIndex(bus_name_, 23, 27, 2, 2), ::testing::ElementsAre(3));
}

TEST_F(ICDDataPrepareICDQueryTest, ReplaceBusNameWithChannelIDInLookupFailIfNotPrepared)
{
    update_map_["BD1"] = std::set<uint64_t>({13});
    update_map_["BD2"] = std::set<uint64_t>({14});
    ASSERT_EQ(icd_.ReplaceBusNameWithChannelIDInLookup(update_map_), false);
}

TEST_F(ICDDataPrepareICDQueryTest, ReplaceBusNameWithChannelIDInLookupFailIfBadMap)
{
    // If the initial result is not true, the test is meaningless.
    res_ = icd_.PrepareICDQuery(icd_lines_);
    ASSERT_EQ(res_, true);

    // These map names don't match those in the ICD.
    update_map_["BUSA"] = std::set<uint64_t>({13});
    update_map_["BUSB"] = std::set<uint64_t>({14});
    ASSERT_EQ(icd_.ReplaceBusNameWithChannelIDInLookup(update_map_), false);
}

TEST_F(ICDDataPrepareICDQueryTest, ReplaceBusNameWithChannelIDInLookupChanIDKeysCorrect)
{
    res_ = icd_.PrepareICDQuery(icd_lines_);
    ASSERT_EQ(res_, true);

    update_map_["BD1"] = std::set<uint64_t>({13});
    update_map_["BD3"] = std::set<uint64_t>({14, 21});
    ASSERT_EQ(icd_.ReplaceBusNameWithChannelIDInLookup(update_map_), true);
    ASSERT_THAT(icd_.GetLookupTableChannelIDKeys(), ::testing::UnorderedElementsAre(13, 14, 21));
}

TEST_F(ICDDataPrepareICDQueryTest, ReplaceBusNameWithChannelIDInLookupOneChanIDKeyCorrect)
{
    res_ = icd_.PrepareICDQuery(icd_lines_);
    ASSERT_EQ(res_, true);

    // These map names don't match those in the ICD.
    update_map_["BD1"] = std::set<uint64_t>({13, 17});
    update_map_["BD4"] = std::set<uint64_t>({14});
    ASSERT_EQ(icd_.ReplaceBusNameWithChannelIDInLookup(update_map_), true);
    ASSERT_THAT(icd_.GetLookupTableChannelIDKeys(), ::testing::UnorderedElementsAre(13, 17));
}

TEST_F(ICDDataPrepareICDQueryTest, LookupTableIndex)
{
    // The final adjusted map can be queried directly using
    // LookupTableIndex().

    // Call public function PrepareICDQuery() which calls the
    // private function CreateTables() followed by CreateLookupMap();
    res_ = icd_.PrepareICDQuery(icd_lines_);
    ASSERT_EQ(res_, true);

    // Use TempLookupTableIndex to query the map.
    update_map_["BD1"] = std::set<uint64_t>({13});
    update_map_["BD3"] = std::set<uint64_t>({14, 21});
    ASSERT_EQ(icd_.ReplaceBusNameWithChannelIDInLookup(update_map_), true);
    EXPECT_THAT(icd_.LookupTableIndex(14, 23, 19, 0, 4), ::testing::ElementsAre(0, 4));
    EXPECT_THAT(icd_.LookupTableIndex(14, 23, 14, 0, 6), ::testing::ElementsAre(2));
    EXPECT_THAT(icd_.LookupTableIndex(21, 23, 19, 0, 4), ::testing::ElementsAre(0, 4));
    EXPECT_THAT(icd_.LookupTableIndex(21, 23, 14, 0, 6), ::testing::ElementsAre(2));
    EXPECT_THAT(icd_.LookupTableIndex(13, 22, 27, 0, 2), ::testing::ElementsAre(1));
    EXPECT_THAT(icd_.LookupTableIndex(13, 23, 27, 2, 2), ::testing::ElementsAre(3));
}

// Real ICD query below. Comment out for general use.
//class ICDDataRealICDQueryTest : public ::testing::Test
//{
//protected:
//
//	ICDData icd_;
//	bool res_;
//	std::string bus_name_;
//	std::vector<std::string> icd_lines_;
//	std::set<size_t> matched_table_inds_;
//	std::string table_name_;
//	std::vector<size_t> table_elem_indices_;
//
//	ICDDataRealICDQueryTest() : res_(false) {}
//	void SetUp() override
//	{
//
//		// Read real digital ICD.
//		std::string icd_path = "";
//		FileReader fr;
//		int read_result = fr.ReadFile(icd_path);
//		if (read_result == 0)
//			res_ = true;
//		else
//		{
//			res_ = false;
//			return;
//		}
//
//		// Ingest lines.
//		icd_lines_ = fr.GetLines();
//		res_ = icd_.PrepareICDQuery(icd_lines_);
//	}
//};

/*
* Tests below are related specifically to processing of YAML files. 
*/

class ICDDataIngestYamlTest : public ::testing::Test
{
   protected:
    ICDData icd_;
    bool res_;
    std::string msg_name_;
    std::vector<std::string> icd_lines_;
    std::vector<std::string> bad_icd_lines1_;
    std::vector<std::string> bad_icd_lines2_;
    std::vector<std::string> bad_icd_lines3_;
    YAML::Node msg_data_node_;
    bool is_bit_node_;
    YAML::Node icd_node_;
    YAML::Node good_icd_node1_;
    YAML::Node bad_icd_node1_;
    YAML::Node bad_icd_node2_;
    YAML::Node bad_icd_node3_;
    std::vector<ICDElement> icd_elems_;
    std::vector<std::string> msg_body_keys_;
    std::vector<std::string> string_icd_components1_;
    std::vector<std::string> string_icd_components2_;
    std::vector<std::string> string_icd_components3_;
    std::map<std::string, std::string> msg_name_subs_;
    std::map<std::string, std::string> elem_name_subs_;

    ICDDataIngestYamlTest() : res_(false),
                              msg_body_keys_({"msg_data", "word_elem", "bit_elem"}) {}

    void FillYamlBasicStructureLines()
    {
        icd_lines_ = {
            "msg1:\n",
            "    msg_data: {}\n",
            "    word_elem: []\n",
            "    bit_elem: []\n",
            "msg2:\n",
            "    msg_data: {}\n",
            "    word_elem: []\n",
            "    bit_elem: []\n"};

        // Root-level sequence, not map
        bad_icd_lines1_ = {
            "- msg1:\n",
            "    msg_data: {}\n",
            "    word_elem: []\n",
            "    bit_elem: []\n",
            "- msg2:\n",
            "    msg_data: {}\n",
            "    word_elem: []\n",
            "    bit_elem: []\n"};
    }

    void FillYamlMsgBodyNodes()
    {
        // correct msg body
        icd_node_ = YAML::Load(
            "    msg_data: {}\n"
            "    word_elem: []\n"
            "    bit_elem: []\n");

        // incorrect, repeat msg_data entry
        bad_icd_node1_ = YAML::Load(
            "    msg_data: {}\n"
            "    word_elem: []\n"
            "    bit_elem: []\n"
            "    msg_data: {}\n");

        // incorrect, missing bit_elem
        bad_icd_node2_ = YAML::Load(
            "    msg_data: {}\n"
            "    word_elem: []\n");

        // incorrect, not map
        bad_icd_node3_ = YAML::Load(
            "    - msg_data: {}\n"
            "    - word_elem: []\n"
            "    - bit_elem: []\n");
    }

    void FillYamlElementsNodes()
    {
        /*
		* Test ndoes defined here are used for structure: Is the node a map? Does each key map
		* to a map? Does the secondary map have the correct keys? It is not meant to check
		* if the values can be interpreted as the correct data types.
		*/

        msg_data_node_ = YAML::Load(
            "{command: [40023, 0], lru_addr: [1, 2], lru_subaddr: [3, 0], lru_name: [LRUA, LRUB], bus: BUSB, wrdcnt: 20, rate: 12.5, mode_code: False, desc: \"description\"}\n");

        // correct bit elems
        icd_node_ = YAML::Load(
            "APP_2-0301: {offset: 2, cnt: 1, schema: UNSIGNEDBITS, msbval: 18, desc: \"description\", uom: \"DAY\", multifmt: False, class: 0, msb: 1, lsb: 9, bitcnt: 9}\n"
            "APP_2-0310: {offset: 2, cnt: 1, schema: UNSIGNEDBITS, msbval: 323, desc: description, uom: YEAR, multifmt: False, class: 0, msb: 10, lsb: 15, bitcnt: 6}\n"
            "APP_2-0316: {offset: 2, cnt: 1, schema: UNSIGNEDBITS, msbval: 1, desc: \"description / description\", uom: ' ', multifmt: False, class: 0, msb: 16, lsb: 16, bitcnt: 1}\n");

        // For msg_data_node_ and icd_node_, element 2.
        string_icd_components1_ = {
            "APP_2", "APP_2-0310", "40023", "0", "20", "BUSB", "LRUA", "1", "LRUB", "2", "3", "0", "12.5",
            "2", "1", "12", "1", "0", "10", "15", "6", "0", "description", "323", "YEAR"};

        // For msg_data_node_ and icd_node_, element 3.
        string_icd_components2_ = {
            "APP_2", "APP_2-0316", "40023", "0", "20", "BUSB", "LRUA", "1", "LRUB", "2", "3", "0", "12.5",
            "2", "1", "12", "1", "0", "16", "16", "1", "0", "description / description", "1", " "};

        // correct word elems
        good_icd_node1_ = YAML::Load(
            "APP_2-03 : {offset: 2, cnt : 1, schema : UNSIGNED16, msbval : 18, "
            "desc : description, uom : DAY, multifmt : False, class : 0}\n"
            "APP_2-04 : {offset: 2, cnt : 1, schema : UNSIGNED16, msbval : 323, "
            "desc : description, uom : YEAR, multifmt : False, class : 0}\n"
            "APP_2-05 : {offset: 2, cnt : 1, schema : UNSIGNED16, msbval : 1000, "
            "desc : description / description, uom : ' ', multifmt : False, class : 0}\n");

        // For msg_data_node_ and good_icd_node1_, element 3, APP_2-05.
        string_icd_components3_ = {
            "APP_2", "APP_2-05", "40023", "0", "20", "BUSB", "LRUA", "1", "LRUB", "2", "3", "0", "12.5",
            "2", "1", "4", "0", "0", "0", "0", "0", "0", "description / description", "1000", " "};

        // incorrect, is sequence of maps
        bad_icd_node1_ = YAML::Load(
            "- APP_2-03 : {offset: 2, cnt : 1, schema : UNSIGNEDBITS, msbval : 18, "
            "desc : description, uom : DAY, multifmt : False, class : 0}\n"
            "- APP_2-04 : {offset: 2, cnt : 1, schema : UNSIGNEDBITS, msbval : 323, "
            "desc : description, uom : YEAR, multifmt : False, class : 0}\n"
            "- APP_2-05 : {offset: 2, cnt : 1, schema : UNSIGNEDBITS, msbval : 1, "
            "desc : description / description, uom : ' ', multifmt : False, class : 0}\n");

        // one incorrect elem, value is not a map
        bad_icd_node2_ = YAML::Load(
            "APP_2-03 :\n"
            "    - offset: 2\n"
            "    - cnt : 1\n"
            "    - schema : UNSIGNEDBITS\n"
            "    - msbval : 18\n"
            "    - desc : description\n"
            "    - uom : DAY\n"
            "    - multifmt : False\n"
            "    - class : 0\n"
            "APP_2-04 : {offset: 2, cnt : 1, schema : UNSIGNEDBITS, msbval : 323, "
            "desc : description, uom : YEAR, multifmt : False, class : 0}\n"
            "APP_2-05 : {offset: 2, cnt : 1, schema : UNSIGNEDBITS, msbval : 1, "
            "desc : description / description, uom : ' ', multifmt : False, class : 0}\n");

        // two incorrect elems, first is missing "schema", third is
        // missing "off".
        bad_icd_node3_ = YAML::Load(
            "APP_2-03 : {offset: 2, cnt : 1, msbval : 18, "
            "desc : description, uom : DAY, multifmt : False, class : 0}\n"
            "APP_2-04 : {offset: 2, cnt : 1, schema : UNSIGNEDBITS, msbval : 323, "
            "desc : description, uom : YEAR, multifmt : False, class : 0}\n"
            "APP_2-05 : {cnt : 1, schema : UNSIGNEDBITS, msbval : 1, "
            "desc : description / description, uom : ' ', multifmt: False, class : 0}\n");
    }

    void FillSequenceNodes()
    {
        // sequence, 3 elements
        icd_node_ = YAML::Load("[a, b, c]\n");

        // sequence, 2 elements
        good_icd_node1_ = YAML::Load("[a, b]\n");

        // not sequence
        bad_icd_node1_ = YAML::Load("key: [a, b]\n");
    }
};

TEST_F(ICDDataIngestYamlTest, IsYamlFile)
{
    std::string path;

    // Windows path
    path = "C:\\User\\foo\\bar\\icd_file.txt";
    EXPECT_EQ(false, icd_.IsYamlFile(path));

    path = "C:\\User\\foo\\bar\\icd_file.YML";
    EXPECT_EQ(true, icd_.IsYamlFile(path));

    path = "C:\\User\\foo\\bar\\icd_file.yml";
    EXPECT_EQ(true, icd_.IsYamlFile(path));

    path = "C:\\User\\foo\\bar\\icd_file.YAML";
    EXPECT_EQ(true, icd_.IsYamlFile(path));

    path = "C:\\User\\foo\\bar\\icd_file.yaml";
    EXPECT_EQ(true, icd_.IsYamlFile(path));

    // Linux path
    path = "/home/user/Documents/foo/bar/icd_file.txt";
    EXPECT_EQ(false, icd_.IsYamlFile(path));

    path = "/home/user/Documents/foo/bar/icd_file.YML";
    EXPECT_EQ(true, icd_.IsYamlFile(path));

    path = "/home/user/Documents/foo/bar/icd_file.yml";
    EXPECT_EQ(true, icd_.IsYamlFile(path));

    path = "/home/user/Documents/foo/bar/icd_file.YAML";
    EXPECT_EQ(true, icd_.IsYamlFile(path));

    EXPECT_EQ(true, icd_.IsYamlFile(path));
}

TEST_F(ICDDataIngestYamlTest, IngestICDYamlNodeValidMsgCount)
{
    // Initial message count ought to be zero.
    ASSERT_EQ(icd_.valid_message_count, 0);

    icd_node_ = YAML::Load(
        "Add:\n"
        "  msg_data: {command: [0, 390], lru_addr: [0, 9], lru_subaddr: [0, 3], lru_name: [m1, m2], bus: b1, wrdcnt: 22, rate: 12.5, mode_code: False, desc: CONTROL}\n"
        "  word_elem:\n"
        "    Add-17: {offset: 16, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE, uom: NONE, multifmt: False, class: 0}\n"
        "    Add-20: {offset: 19, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE5, uom: NONE, multifmt: False, class: 0}\n"
        "  bit_elem:\n"
        "    Add-0201: {offset: 1, cnt: 1, schema: UNSIGNEDBITS, msbval: 8, desc: MODE4, uom: NONE, multifmt: False, class: 0, msb: 1, lsb: 4, bitcnt: 4}\n");

    EXPECT_TRUE(icd_node_.IsMap());
    EXPECT_TRUE(icd_.IngestICDYamlNode(icd_node_, icd_elems_, msg_name_subs_,
                                       elem_name_subs_));
    EXPECT_EQ(icd_.valid_message_count, 1);

    icd_node_ = YAML::Load(
        "Add:\n"
        "  msg_data: {command: [0, 390], lru_addr: [0, 9], lru_subaddr: [0, 3], lru_name: [m1, m2], bus: b1, wrdcnt: 22, rate: 12.5, mode_code: False, desc: CONTROL}\n"
        "  word_elem:\n"
        "    Add-17: {offset: 16, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE, uom: NONE, multifmt: False, class: 0}\n"
        "    Add-20: {offset: 19, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE5, uom: NONE, multifmt: False, class: 0}\n"
        "  bit_elem:\n"
        "    Add-0201: {offset: 1, cnt: 1, schema: UNSIGNEDBITS, msbval: 8, desc: MODE4, uom: NONE, multifmt: False, class: 0, msb: 1, lsb: 4, bitcnt: 4}\n"
        "Aee:\n"
        "  msg_data: {command: [0, 2190], lru_addr: [3, 3], lru_subaddr: [0, 3], lru_name: [m1, m2], bus: b1, wrdcnt: 22, rate: 12.5, mode_code: False, desc: CONTROL}\n"
        "  word_elem:\n"
        "    Aee-17: {offset: 16, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE, uom: NONE, multifmt: False, class: 0}\n"
        "    Aee-20: {offset: 19, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE5, uom: NONE, multifmt: False, class: 0}\n"
        "  bit_elem:\n"
        "    Aee-0201: {offset: 1, cnt: 1, schema: UNSIGNEDBITS, msbval: 8, desc: MODE4, uom: NONE, multifmt: False, class: 0, msb: 1, lsb: 4, bitcnt: 4}\n");

    EXPECT_TRUE(icd_.IngestICDYamlNode(icd_node_, icd_elems_, msg_name_subs_,
                                       elem_name_subs_));
    EXPECT_EQ(icd_.valid_message_count, 2);
}

TEST_F(ICDDataIngestYamlTest, IngestICDYamlNodeMsgNameSubstitutions)
{
    // No subtitutions required
    icd_node_ = YAML::Load(
        "Add:\n"
        "  msg_data: {command: [0, 390], lru_addr: [0, 9], lru_subaddr: [0, 3], lru_name: [m1, m2], bus: b1, wrdcnt: 22, rate: 12.5, mode_code: False, desc: CONTROL}\n"
        "  word_elem:\n"
        "    Add-17: {offset: 16, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE, uom: NONE, multifmt: False, class: 0}\n"
        "    Add-20: {offset: 19, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE5, uom: NONE, multifmt: False, class: 0}\n"
        "  bit_elem:\n"
        "    Add-0201: {offset: 1, cnt: 1, schema: UNSIGNEDBITS, msbval: 8, desc: MODE4, uom: NONE, multifmt: False, class: 0, msb: 1, lsb: 4, bitcnt: 4}\n"
        "Acc:\n"
        "  msg_data: {command: [0, 330], lru_addr: [3, 9], lru_subaddr: [1, 3], lru_name: [m1, m2], bus: b1, wrdcnt: 22, rate: 12.5, mode_code: False, desc: CONTROL}\n"
        "  word_elem:\n"
        "    Acc-17: {offset: 16, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE, uom: NONE, multifmt: False, class: 0}\n"
        "    Acc-20: {offset: 19, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE5, uom: NONE, multifmt: False, class: 0}\n"
        "  bit_elem:\n"
        "    Acc-0201: {offset: 1, cnt: 1, schema: UNSIGNEDBITS, msbval: 8, desc: MODE4, uom: NONE, multifmt: False, class: 0, msb: 1, lsb: 4, bitcnt: 4}\n"

    );

    EXPECT_TRUE(icd_node_.IsMap());
    EXPECT_TRUE(icd_.IngestICDYamlNode(icd_node_, icd_elems_, msg_name_subs_,
                                       elem_name_subs_));
    EXPECT_TRUE(msg_name_subs_.size() == 0);

    // Unreserved chars in msg names ok
    icd_node_ = YAML::Load(
        "Ad-d:\n"
        "  msg_data: {command: [0, 390], lru_addr: [0, 9], lru_subaddr: [0, 3], lru_name: [m1, m2], bus: b1, wrdcnt: 22, rate: 12.5, mode_code: False, desc: CONTROL}\n"
        "  word_elem:\n"
        "    Add-17: {offset: 16, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE, uom: NONE, multifmt: False, class: 0}\n"
        "    Add-20: {offset: 19, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE5, uom: NONE, multifmt: False, class: 0}\n"
        "  bit_elem:\n"
        "    Add-0201: {offset: 1, cnt: 1, schema: UNSIGNEDBITS, msbval: 8, desc: MODE4, uom: NONE, multifmt: False, class: 0, msb: 1, lsb: 4, bitcnt: 4}\n"
        "A.~cc:\n"
        "  msg_data: {command: [0, 330], lru_addr: [3, 9], lru_subaddr: [1, 3], lru_name: [m1, m2], bus: b1, wrdcnt: 22, rate: 12.5, mode_code: False, desc: CONTROL}\n"
        "  word_elem:\n"
        "    Acc-17: {offset: 16, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE, uom: NONE, multifmt: False, class: 0}\n"
        "    Acc-20: {offset: 19, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE5, uom: NONE, multifmt: False, class: 0}\n"
        "  bit_elem:\n"
        "    Acc-0201: {offset: 1, cnt: 1, schema: UNSIGNEDBITS, msbval: 8, desc: MODE4, uom: NONE, multifmt: False, class: 0, msb: 1, lsb: 4, bitcnt: 4}\n"

    );

    EXPECT_TRUE(icd_node_.IsMap());
    EXPECT_TRUE(icd_.IngestICDYamlNode(icd_node_, icd_elems_, msg_name_subs_,
                                       elem_name_subs_));
    EXPECT_TRUE(msg_name_subs_.size() == 0);

    // Expect subs in this case
    icd_node_ = YAML::Load(
        "Ad*d:\n"
        "  msg_data: {command: [0, 390], lru_addr: [0, 9], lru_subaddr: [0, 3], lru_name: [m1, m2], bus: b1, wrdcnt: 22, rate: 12.5, mode_code: False, desc: CONTROL}\n"
        "  word_elem:\n"
        "    Add-17: {offset: 16, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE, uom: NONE, multifmt: False, class: 0}\n"
        "    Add-20: {offset: 19, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE5, uom: NONE, multifmt: False, class: 0}\n"
        "  bit_elem:\n"
        "    Add-0201: {offset: 1, cnt: 1, schema: UNSIGNEDBITS, msbval: 8, desc: MODE4, uom: NONE, multifmt: False, class: 0, msb: 1, lsb: 4, bitcnt: 4}\n"
        "A/c+c:\n"
        "  msg_data: {command: [0, 330], lru_addr: [3, 9], lru_subaddr: [1, 3], lru_name: [m1, m2], bus: b1, wrdcnt: 22, rate: 12.5, mode_code: False, desc: CONTROL}\n"
        "  word_elem:\n"
        "    Acc-17: {offset: 16, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE, uom: NONE, multifmt: False, class: 0}\n"
        "    Acc-20: {offset: 19, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE5, uom: NONE, multifmt: False, class: 0}\n"
        "  bit_elem:\n"
        "    Acc-0201: {offset: 1, cnt: 1, schema: UNSIGNEDBITS, msbval: 8, desc: MODE4, uom: NONE, multifmt: False, class: 0, msb: 1, lsb: 4, bitcnt: 4}\n"

    );

    std::map<std::string, std::string> expected = {
        {"Ad*d", "Ad%2ad"},
        {"A/c+c", "A%2fc%2bc"}};
    EXPECT_TRUE(icd_node_.IsMap());
    EXPECT_TRUE(icd_.IngestICDYamlNode(icd_node_, icd_elems_, msg_name_subs_,
                                       elem_name_subs_));
    ASSERT_TRUE(msg_name_subs_.size() == 2);
    ASSERT_TRUE(msg_name_subs_.count("Ad*d") == 1);
    ASSERT_TRUE(msg_name_subs_.count("A/c+c") == 1);
    EXPECT_EQ(msg_name_subs_.at("Ad*d"), expected.at("Ad*d"));
    EXPECT_EQ(msg_name_subs_.at("A/c+c"), expected.at("A/c+c"));
}

TEST_F(ICDDataIngestYamlTest, IngestICDYamlNodeElemNameSubstitutions)
{
    // No subtitutions required
    icd_node_ = YAML::Load(
        "Add:\n"
        "  msg_data: {command: [0, 390], lru_addr: [0, 9], lru_subaddr: [0, 3], lru_name: [m1, m2], bus: b1, wrdcnt: 22, rate: 12.5, mode_code: False, desc: CONTROL}\n"
        "  word_elem:\n"
        "    Add-17: {offset: 16, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE, uom: NONE, multifmt: False, class: 0}\n"
        "    Add-20: {offset: 19, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE5, uom: NONE, multifmt: False, class: 0}\n"
        "  bit_elem:\n"
        "    Add-0201: {offset: 1, cnt: 1, schema: UNSIGNEDBITS, msbval: 8, desc: MODE4, uom: NONE, multifmt: False, class: 0, msb: 1, lsb: 4, bitcnt: 4}\n"
        "Acc:\n"
        "  msg_data: {command: [0, 330], lru_addr: [3, 9], lru_subaddr: [1, 3], lru_name: [m1, m2], bus: b1, wrdcnt: 22, rate: 12.5, mode_code: False, desc: CONTROL}\n"
        "  word_elem:\n"
        "    Acc-17: {offset: 16, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE, uom: NONE, multifmt: False, class: 0}\n"
        "    Acc-20: {offset: 19, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE5, uom: NONE, multifmt: False, class: 0}\n"
        "  bit_elem:\n"
        "    Acc-0201: {offset: 1, cnt: 1, schema: UNSIGNEDBITS, msbval: 8, desc: MODE4, uom: NONE, multifmt: False, class: 0, msb: 1, lsb: 4, bitcnt: 4}\n");

    EXPECT_TRUE(icd_node_.IsMap());
    EXPECT_TRUE(icd_.IngestICDYamlNode(icd_node_, icd_elems_, msg_name_subs_,
                                       elem_name_subs_));
    EXPECT_TRUE(elem_name_subs_.size() == 0);

    // Unreserved chars ok, no subs necessary
    icd_node_ = YAML::Load(
        "Add:\n"
        "  msg_data: {command: [0, 390], lru_addr: [0, 9], lru_subaddr: [0, 3], lru_name: [m1, m2], bus: b1, wrdcnt: 22, rate: 12.5, mode_code: False, desc: CONTROL}\n"
        "  word_elem:\n"
        "    A.d-17: {offset: 16, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE, uom: NONE, multifmt: False, class: 0}\n"
        "    Add-20: {offset: 19, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE5, uom: NONE, multifmt: False, class: 0}\n"
        "  bit_elem:\n"
        "    A_dd-0201: {offset: 1, cnt: 1, schema: UNSIGNEDBITS, msbval: 8, desc: MODE4, uom: NONE, multifmt: False, class: 0, msb: 1, lsb: 4, bitcnt: 4}\n"
        "Acc:\n"
        "  msg_data: {command: [0, 330], lru_addr: [3, 9], lru_subaddr: [1, 3], lru_name: [m1, m2], bus: b1, wrdcnt: 22, rate: 12.5, mode_code: False, desc: CONTROL}\n"
        "  word_elem:\n"
        "    Acc-17: {offset: 16, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE, uom: NONE, multifmt: False, class: 0}\n"
        "    A~cc-20: {offset: 19, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE5, uom: NONE, multifmt: False, class: 0}\n"
        "  bit_elem:\n"
        "    Acc-0201: {offset: 1, cnt: 1, schema: UNSIGNEDBITS, msbval: 8, desc: MODE4, uom: NONE, multifmt: False, class: 0, msb: 1, lsb: 4, bitcnt: 4}\n");

    EXPECT_TRUE(icd_node_.IsMap());
    EXPECT_TRUE(icd_.IngestICDYamlNode(icd_node_, icd_elems_, msg_name_subs_,
                                       elem_name_subs_));
    EXPECT_TRUE(elem_name_subs_.size() == 0);

    // Subs expected
    icd_node_ = YAML::Load(
        "Add:\n"
        "  msg_data: {command: [0, 390], lru_addr: [0, 9], lru_subaddr: [0, 3], lru_name: [m1, m2], bus: b1, wrdcnt: 22, rate: 12.5, mode_code: False, desc: CONTROL}\n"
        "  word_elem:\n"
        "    A/d-17: {offset: 16, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE, uom: NONE, multifmt: False, class: 0}\n"
        "    Add-20: {offset: 19, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE5, uom: NONE, multifmt: False, class: 0}\n"
        "  bit_elem:\n"
        "    A#dd-0201: {offset: 1, cnt: 1, schema: UNSIGNEDBITS, msbval: 8, desc: MODE4, uom: NONE, multifmt: False, class: 0, msb: 1, lsb: 4, bitcnt: 4}\n"
        "Acc:\n"
        "  msg_data: {command: [0, 330], lru_addr: [3, 9], lru_subaddr: [1, 3], lru_name: [m1, m2], bus: b1, wrdcnt: 22, rate: 12.5, mode_code: False, desc: CONTROL}\n"
        "  word_elem:\n"
        "    Acc-17: {offset: 16, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE, uom: NONE, multifmt: False, class: 0}\n"
        "    A*cc-20: {offset: 19, cnt: 2, schema: SIGNED32, msbval: 1, desc: MODE5, uom: NONE, multifmt: False, class: 0}\n"
        "  bit_elem:\n"
        "    Acc-02\\01: {offset: 1, cnt: 1, schema: UNSIGNEDBITS, msbval: 8, desc: MODE4, uom: NONE, multifmt: False, class: 0, msb: 1, lsb: 4, bitcnt: 4}\n");

    std::map<std::string, std::string> expected = {
        {"A/d-17", "A%2fd-17"},
        {"A#dd-0201", "A%23dd-0201"},
        {"A*cc-20", "A%2acc-20"},
        {"Acc-02\\01", "Acc-02%5c01"}};
    EXPECT_TRUE(icd_node_.IsMap());
    EXPECT_TRUE(icd_.IngestICDYamlNode(icd_node_, icd_elems_, msg_name_subs_,
                                       elem_name_subs_));
    ASSERT_TRUE(elem_name_subs_.size() == 4);
    ASSERT_TRUE(elem_name_subs_.count("A/d-17") == 1);
    ASSERT_TRUE(elem_name_subs_.count("A#dd-0201") == 1);
    ASSERT_TRUE(elem_name_subs_.count("A*cc-20") == 1);
    ASSERT_TRUE(elem_name_subs_.count("Acc-02\\01") == 1);
    EXPECT_THAT(expected, ::testing::ContainerEq(elem_name_subs_));
}

TEST_F(ICDDataIngestYamlTest, MapNodeHasRequiredKeys)
{
    FillYamlMsgBodyNodes();
    EXPECT_EQ(true, icd_.MapNodeHasRequiredKeys(icd_node_, msg_body_keys_));
    EXPECT_EQ(false, icd_.MapNodeHasRequiredKeys(bad_icd_node1_, msg_body_keys_));
    EXPECT_EQ(false, icd_.MapNodeHasRequiredKeys(bad_icd_node2_, msg_body_keys_));
    EXPECT_EQ(false, icd_.MapNodeHasRequiredKeys(bad_icd_node3_, msg_body_keys_));
}

TEST_F(ICDDataIngestYamlTest, FillElementsFromYamlNodes)
{
    FillYamlElementsNodes();
    msg_name_ = "msgA";
    is_bit_node_ = true;
    size_t expected_fill_count = 3;
    size_t reported_fill_count = 0;
    reported_fill_count = icd_.FillElementsFromYamlNodes(msg_name_, msg_data_node_,
                                                         icd_node_, is_bit_node_, icd_elems_,
                                                         elem_name_subs_);
    EXPECT_EQ(expected_fill_count, reported_fill_count);
    EXPECT_EQ(expected_fill_count, icd_elems_.size());

    is_bit_node_ = false;
    icd_elems_.clear();
    reported_fill_count = icd_.FillElementsFromYamlNodes(msg_name_, msg_data_node_,
                                                         good_icd_node1_, is_bit_node_, icd_elems_,
                                                         elem_name_subs_);
    EXPECT_EQ(expected_fill_count, reported_fill_count);
    EXPECT_EQ(expected_fill_count, icd_elems_.size());

    icd_elems_.clear();
    expected_fill_count = 0;
    reported_fill_count = icd_.FillElementsFromYamlNodes(msg_name_, msg_data_node_,
                                                         bad_icd_node1_, is_bit_node_, icd_elems_,
                                                         elem_name_subs_);
    EXPECT_EQ(expected_fill_count, reported_fill_count);
    EXPECT_EQ(expected_fill_count, icd_elems_.size());

    icd_elems_.clear();
    expected_fill_count = 2;
    reported_fill_count = icd_.FillElementsFromYamlNodes(msg_name_, msg_data_node_,
                                                         bad_icd_node2_, is_bit_node_, icd_elems_,
                                                         elem_name_subs_);
    EXPECT_EQ(expected_fill_count, reported_fill_count);
    EXPECT_EQ(expected_fill_count, icd_elems_.size());

    icd_elems_.clear();
    expected_fill_count = 1;
    reported_fill_count = icd_.FillElementsFromYamlNodes(msg_name_, msg_data_node_,
                                                         bad_icd_node3_, is_bit_node_, icd_elems_,
                                                         elem_name_subs_);
    EXPECT_EQ(expected_fill_count, reported_fill_count);
    EXPECT_EQ(expected_fill_count, icd_elems_.size());
}

TEST_F(ICDDataIngestYamlTest, SequenceNodeHasCorrectSize)
{
    FillSequenceNodes();
    size_t required_size = 3;
    EXPECT_EQ(true, icd_.SequenceNodeHasCorrectSize(icd_node_, required_size));

    required_size = 1;
    EXPECT_EQ(false, icd_.SequenceNodeHasCorrectSize(icd_node_, required_size));

    required_size = 2;
    EXPECT_EQ(true, icd_.SequenceNodeHasCorrectSize(good_icd_node1_, required_size));

    required_size = 3;
    EXPECT_EQ(false, icd_.SequenceNodeHasCorrectSize(good_icd_node1_, required_size));

    required_size = 2;
    EXPECT_EQ(false, icd_.SequenceNodeHasCorrectSize(bad_icd_node1_, required_size));
}

TEST_F(ICDDataIngestYamlTest, CreateVectorOfStringICDComponents)
{
    FillYamlElementsNodes();
    std::vector<std::string> output_vec;
    std::string msg_name = "APP_2";
    std::string elem_name = "APP_2-0310";
    bool is_bit_elem = true;

    // Output vector hasn't been resized, ought to return false.
    res_ = icd_.CreateVectorOfStringICDComponents(msg_name, msg_data_node_, elem_name,
                                                  icd_node_[elem_name], is_bit_elem, output_vec);
    EXPECT_FALSE(res_);

    output_vec.resize(ICDElement::kFillElementCount);
    res_ = icd_.CreateVectorOfStringICDComponents(msg_name, msg_data_node_, elem_name,
                                                  icd_node_[elem_name], is_bit_elem, output_vec);
    EXPECT_TRUE(res_);
    EXPECT_THAT(string_icd_components1_, ::testing::ElementsAreArray(output_vec));

    elem_name = "APP_2-0316";
    res_ = icd_.CreateVectorOfStringICDComponents(msg_name, msg_data_node_, elem_name,
                                                  icd_node_[elem_name], is_bit_elem, output_vec);
    EXPECT_TRUE(res_);
    EXPECT_THAT(string_icd_components2_, ::testing::ElementsAreArray(output_vec));

    elem_name = "APP_2-05";
    is_bit_elem = false;
    res_ = icd_.CreateVectorOfStringICDComponents(msg_name, msg_data_node_, elem_name,
                                                  good_icd_node1_[elem_name], is_bit_elem, output_vec);
    EXPECT_TRUE(res_);
    EXPECT_THAT(string_icd_components3_, ::testing::ElementsAreArray(output_vec));
}

TEST_F(ICDDataIngestYamlTest, ConfigureMsgDataFromYamlNodeCorrectVals)
{
    FillYamlElementsNodes();
    // msg_data_node_ = YAML::Load(
    //     "{command: [40023, 0], lru_addr: [1, 2], lru_subaddr: [3, 0], lru_name: [LRUA, LRUB], bus: BUSB, wrdcnt: 20, rate: 12.5, mode_code: False, desc: \"description\"}\n");
    ICDElement icdelem;
    msg_name_ = "MSG";

    res_ = icd_.ConfigureMsgDataFromYamlNode(icdelem, msg_name_, msg_data_node_);
    EXPECT_TRUE(res_);
    EXPECT_EQ(msg_name_, icdelem.msg_name_);

    EXPECT_EQ(msg_data_node_["command"][0].as<int>(), icdelem.xmit_word_);
    EXPECT_EQ(msg_data_node_["command"][1].as<int>(), icdelem.dest_word_);

    EXPECT_EQ(msg_data_node_["wrdcnt"].as<int>(), icdelem.msg_word_count_);

    EXPECT_EQ(msg_data_node_["bus"].as<std::string>(), icdelem.bus_name_);

    EXPECT_EQ(msg_data_node_["lru_name"][0].as<std::string>(), icdelem.xmit_lru_name_);
    EXPECT_EQ(msg_data_node_["lru_name"][1].as<std::string>(), icdelem.dest_lru_name_);

    EXPECT_EQ(msg_data_node_["lru_addr"][0].as<int>(), icdelem.xmit_lru_addr_);
    EXPECT_EQ(msg_data_node_["lru_addr"][1].as<int>(), icdelem.dest_lru_addr_);

    EXPECT_EQ(msg_data_node_["lru_subaddr"][0].as<int>(), icdelem.xmit_lru_subaddr_);
    EXPECT_EQ(msg_data_node_["lru_subaddr"][1].as<int>(), icdelem.dest_lru_subaddr_);

    EXPECT_EQ(msg_data_node_["rate"].as<float>(), icdelem.rate_);
}

TEST_F(ICDDataIngestYamlTest, ConfigureMsgDataFromYamlNodeMissingRequiredMappedValue)
{
    // Missing "bus" key.
    msg_data_node_ = YAML::Load(
        "{command: [40023, 0], lru_addr: [1, 2], lru_subaddr: [3, 0], lru_name: [LRUA, LRUB], wrdcnt: 20, rate: 12.5, mode_code: False, desc: \"description\"}\n");
    ICDElement icdelem;
    msg_name_ = "MSG";

    res_ = icd_.ConfigureMsgDataFromYamlNode(icdelem, msg_name_, msg_data_node_);
    ASSERT_FALSE(res_);
}

TEST_F(ICDDataIngestYamlTest, ConfigureMsgDataFromYamlNodeMissingRequiredSequence)
{
    // Missing "lru_addr" key.
    msg_data_node_ = YAML::Load(
        "{command: [40023, 0], bus: BUSB, lru_subaddr: [3, 0], lru_name: [LRUA, LRUB], wrdcnt: 20, rate: 12.5, mode_code: False, desc: \"description\"}\n");
    ICDElement icdelem;
    msg_name_ = "MSG";

    res_ = icd_.ConfigureMsgDataFromYamlNode(icdelem, msg_name_, msg_data_node_);
    ASSERT_FALSE(res_);
}

TEST_F(ICDDataIngestYamlTest, ConfigureMsgDataFromYamlNodeMissingOptionalSequence)
{
    // Missing "lru_name" key.
    msg_data_node_ = YAML::Load(
        "{command: [40023, 0], lru_addr: [1, 2], bus: BUSB, lru_subaddr: [3, 0], wrdcnt: 20, rate: 12.5, mode_code: False, desc: \"description\"}\n");
    ICDElement icdelem;
    msg_name_ = "MSG";

    res_ = icd_.ConfigureMsgDataFromYamlNode(icdelem, msg_name_, msg_data_node_);
    // Note that return should be true indicating good configuration.
    ASSERT_TRUE(res_);
    EXPECT_EQ("", icdelem.xmit_lru_name_);
    EXPECT_EQ("", icdelem.dest_lru_name_);
}

TEST_F(ICDDataIngestYamlTest, ConfigureWordElemDataFromYamlNodeCorrectVals)
{
    YAML::Node node = YAML::Load(
        "{offset: 2, cnt : 1, schema : UNSIGNED16, msbval : 18, "
        "desc : description, uom : DAY, multifmt : False, class : 0}\n");
    ICDElement icdelem;
    std::string elem_name = "ytt644";

    res_ = icd_.ConfigureWordElemDataFromYamlNode(icdelem, node, elem_name);
    EXPECT_TRUE(res_);

    EXPECT_FALSE(icdelem.is_bitlevel_);
    EXPECT_EQ(elem_name, icdelem.elem_name_);
    EXPECT_EQ(node["offset"].as<int>(), icdelem.offset_);
    EXPECT_EQ(node["cnt"].as<int>(), icdelem.elem_word_count_);

    ICDElementSchema schema = ICDElementSchema::BAD;
    std::string schema_str = node["schema"].as<std::string>();
    res_ = icd_.ICDSchemaStringToEnum(schema_str, schema);
    EXPECT_TRUE(res_);

    EXPECT_EQ(schema, icdelem.schema_);
    EXPECT_EQ(node["msbval"].as<double>(), icdelem.msb_val_);
    EXPECT_EQ(node["desc"].as<std::string>(), icdelem.description_);
    EXPECT_EQ(node["uom"].as<std::string>(), icdelem.uom_);
    EXPECT_EQ(node["multifmt"].as<bool>(), icdelem.is_multiformat_);
    EXPECT_EQ(node["class"].as<int>(), icdelem.classification_);
}

TEST_F(ICDDataIngestYamlTest, ConfigureBitElemDataFromYamlNodeCorrectVals)
{
    YAML::Node node = YAML::Load(
        "{offset: 2, cnt: 1, schema: UNSIGNEDBITS, msbval: 18, "
        "desc: \"description\", uom: \"DAY\", multifmt: False, "
        "class: 0, msb: 1, lsb: 9, bitcnt: 9}\n");
    ICDElement icdelem;
    std::string elem_name = "532-xx";

    res_ = icd_.ConfigureBitElemDataFromYamlNode(icdelem, node, elem_name);
    EXPECT_TRUE(res_);

    EXPECT_TRUE(icdelem.is_bitlevel_);
    EXPECT_EQ(elem_name, icdelem.elem_name_);
    EXPECT_EQ(node["offset"].as<int>(), icdelem.offset_);
    EXPECT_EQ(node["cnt"].as<int>(), icdelem.elem_word_count_);

    ICDElementSchema schema = ICDElementSchema::BAD;
    std::string schema_str = node["schema"].as<std::string>();
    res_ = icd_.ICDSchemaStringToEnum(schema_str, schema);
    EXPECT_TRUE(res_);
    EXPECT_EQ(schema, icdelem.schema_);

    EXPECT_EQ(node["msbval"].as<double>(), icdelem.msb_val_);
    EXPECT_EQ(node["desc"].as<std::string>(), icdelem.description_);
    EXPECT_EQ(node["uom"].as<std::string>(), icdelem.uom_);
    EXPECT_EQ(node["multifmt"].as<bool>(), icdelem.is_multiformat_);
    EXPECT_EQ(node["class"].as<int>(), icdelem.classification_);
    EXPECT_EQ(node["msb"].as<int>(), icdelem.bitmsb_);
    EXPECT_EQ(node["lsb"].as<int>(), icdelem.bitlsb_);
    EXPECT_EQ(node["bitcnt"].as<int>(), icdelem.bit_count_);
}

TEST_F(ICDDataTest, GetTableIndexByName)
{
    std::vector<std::string> table_names{"a", "b", "c"};

    // not in vector
    msg_name_ = "d";
    size_t index = 0;
    index = icd_.GetTableIndexByName(msg_name_, table_names);
    EXPECT_EQ(std::string::npos, index);

    msg_name_ = "b";
    index = icd_.GetTableIndexByName(msg_name_, table_names);
    EXPECT_EQ(1, index);
}

TEST_F(ICDDataTest, GetSelectedTableIndicesSet)
{
    std::vector<std::string> table_names{"a", "b", "c"};
    std::set<size_t> selected_indices;

    // None present
    std::set<std::string> selected_names{"d", "e"};
    selected_indices = icd_.GetSelectedTableIndicesSet(selected_names, table_names);
    EXPECT_EQ(0, selected_indices.size());

    // Some present, others not
    std::set<std::string> selected_names2{"d", "a", "e"};
    selected_indices = icd_.GetSelectedTableIndicesSet(selected_names2, table_names);
    EXPECT_EQ(1, selected_indices.size());
    EXPECT_EQ(1, selected_indices.count(0));
}