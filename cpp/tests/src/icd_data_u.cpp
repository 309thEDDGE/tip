#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "icd_element.h"
#include "icd_data.h"

// Needed for ICDDataRealICDQueryTest
#include "file_reader.h"

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
	std::string test_str = "BL22U,BL22U-20,00000,39030,22,BD3,"
		"MIVU,23,ORT,19,00,03,12.50,19,02,03,0,0,02,00,00,0,"
		"description 5,17374824.000000000000,NONE";
	ASSERT_TRUE(ice.Fill(test_str));

	// 25 elements with trailing comma
	test_str = "BL22U,BL22U-20,00000,39030,22,BD3,"
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
	std::string test_str = "BL22U,BL22U-20,fff,39030,22,BD3," 
		"MIVU,23,ORT,19,00,03,12.50,19,02,03,0,0,02,00,00,0,"
		"description 5,17374824.000000000000,NONE";
	ASSERT_FALSE(ice.Fill(test_str));

	// Second element should be a string and not convertible
	// to an int or float
	test_str = "BL22U,20,00000,39030,22,BD3,"
		"MIVU,23,ORT,19,00,03,12.50,19,02,03,0,0,02,00,00,0,"
		"description 5,17374824.000000000000,NONE,";
	ASSERT_FALSE(ice.Fill(test_str));

	// Element that is integer in the first test (19),
	// is float here, which is incorrect.
	test_str = "BL22U,BL22U-20,00000,39030,22,BD3,"
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
	std::string test_str = "BL22U,BL22U-20,00000,39030,22,BD3,"
		"MIVU,23,ORT,19,00,03,12.50,19,02,03,0,0,02,00,00,0,"
		"description 5,17374824.000000000000,NONE";
	ASSERT_TRUE(ice.Fill(test_str));
}

TEST(ICDElementTest, FillInvalidBitValues)
{
	ICDElement ice;

	// Everything correct, except bitmsb which is 0 and is_bitlevel = 1.
	// Bitmsb and bitlsb must be in [1, 16].
	std::string test_str = "BL22U,BL22U-20,00000,39030,22,BD3,"
		"MIVU,23,ORT,19,00,03,12.50,19,02,03,1,0,00,5,00,0,"
		"description 5,17374824.000000000000,NONE";
	EXPECT_FALSE(ice.Fill(test_str));

	// Everything correct, except bitlsb which is 50 and is_bitlevel = 1.
	// Bitmsb and bitlsb must be in [1, 16].
	test_str = "BL22U,BL22U-20,00000,39030,22,BD3,"
		"MIVU,23,ORT,19,00,03,12.50,19,02,03,1,0,12,50,00,0,"
		"description 5,17374824.000000000000,NONE";
	EXPECT_FALSE(ice.Fill(test_str));
}

TEST(ICDDataTest, IngestICDTextFileLinesEmptyVector)
{
	ICDData icd;
	std::vector<std::string> lines;
	std::vector<ICDElement> elems;
	ASSERT_FALSE(icd.IngestICDTextFileLines(lines, elems));
}

TEST(ICDDataTest, IngestICDTextFileLinesNonEmptyOutputVector)
{
	ICDData icd;
	std::vector<std::string> lines;
	std::vector<ICDElement> elems;
	elems.push_back(ICDElement());
	ASSERT_FALSE(icd.IngestICDTextFileLines(lines, elems));
}

TEST(ICDDataTest, IngestICDTextFileLinesHeaderColCount)
{
	// Correct header count, currently 25 elements.
	std::vector<std::string> lines;
	lines.push_back("msg_name,elem_name,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y");
	std::vector<ICDElement> elems;
	ICDData icd;
	EXPECT_TRUE(icd.IngestICDTextFileLines(lines, elems));

	// Header only and no data. Elems will not be filled.
	EXPECT_EQ(elems.size(), 0);

	// Incorrect header count.
	lines[0] = "msg_name,elem_name,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X";
	ASSERT_FALSE(icd.IngestICDTextFileLines(lines, elems));
}

TEST(ICDDataTest, IngestICDTextFileLinesHeaderColName)
{
	// Only care about correct values for the first two column headers.

	// Here, the second column name is incorrect; it
	// ought to be elem_name.
	std::vector<std::string> lines;
	lines.push_back("msg_name,elemname,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y");
	std::vector<ICDElement> elems;
	ICDData icd;
	EXPECT_FALSE(icd.IngestICDTextFileLines(lines, elems));

	// Second column is now correct.
	lines[0] = "msg_name,elem_name,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y";
	EXPECT_TRUE(icd.IngestICDTextFileLines(lines, elems));
}

TEST(ICDDataTest, IngestICDTextFileLinesExpectedElementCount)
{
	// Only care about correct values for the first two column headers.

	// Fill actual header line and additional data lines.
	std::vector<std::string> lines;
	lines.push_back("msg_name,elem_name,xmit_word,dest_word,msg_word_count,"
		"bus_name,xmit_lru_name,xmit_lru_addr,dest_lru_name,dest_lru_addr,"
		"xmit_subaddr,dest_subaddr,rate,offset,elem_word_count,schema,isbit,"
		"ismulti,bitmsb,bitlsb,bitcount,classification,description,msbval,uom\n");
	lines.push_back("BL22U,BL22U-17,00000,39030,22,BD3,MIVU,23,ORT,19,00,03,12.50,16,"
		"02,03,0,0,02,00,00,0,description 5,17374824.000000000000,NONE\n");
	lines.push_back("CX53_Z_8,CX53_Z_8-04,00000,18525,29,M1980,MIVU,23,WS9,09,00,02,"
		"0.00,03,01,04,0,0,01,00,00,0,description 6,"
		"376.000000000000,USEC\n");
	lines.push_back("ML00_6,ML00_6-0607,00000,28935,07,BD3,MIVU,23,BLATS,14,00,08,0.00,05,"
		"01,12,1,0,07,07,01,0,description 5,1.000000000000,NONE\n");

	ICDData icd;
	std::vector<ICDElement> elems;
	EXPECT_TRUE(icd.IngestICDTextFileLines(lines, elems));
	ASSERT_EQ(elems.size(), 3);
}

TEST(ICDDataTest, IngestICDTextFileLinesMalformedICDLine)
{
	// Only care about correct values for the first two column headers.

	// Fill actual header line and additional data lines.
	std::vector<std::string> lines;
	lines.push_back("msg_name,elem_name,xmit_word,dest_word,msg_word_count,"
		"bus_name,xmit_lru_name,xmit_lru_addr,dest_lru_name,dest_lru_addr,"
		"xmit_subaddr,dest_subaddr,rate,offset,elem_word_count,schema,isbit,"
		"ismulti,bitmsb,bitlsb,bitcount,classification,description,msbval,uom\n");
	// Destination command word not interpretable as integer.
	lines.push_back("BL22U,BL22U-17,00000,int,22,BD3,MIVU,23,ORT,19,00,03,12.50,16,"
		"02,03,0,0,02,00,00,0,description 5,17374824.000000000000,NONE\n");

	// Rate column not interpretable as float.
	lines.push_back("CX53_Z_8,CX53_Z_8-04,00000,18525,29,M1980,MIVU,23,WS9,09,00,02,"
		"float,03,01,04,0,0,01,00,00,0,description,"
		"376.000000000000,USEC\n");
	lines.push_back("ML00_6,ML00_6-0607,00000,28935,07,BD3,MIVU,23,BLATS,14,00,08,0.00,05,"
		"01,12,1,0,07,07,01,0,description 6,1.000000000000,NONE\n");

	ICDData icd;
	std::vector<ICDElement> elems;
	EXPECT_TRUE(icd.IngestICDTextFileLines(lines, elems));
	ASSERT_EQ(elems.size(), 1);
}


TEST(ICDDataTest, PrepareMessageKeyMap)
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
		"RPG4_I,RPG4_I-01,00000,39043,20,Bus4,MIVU,23,ORT,19,00,04,1.00,00,02,05,0,0,01,00,00,0,description description,21474348.000000000000,SECONDS"
	};

	ICDData icd;
	ASSERT_TRUE(icd.PrepareICDQuery(temp_lines, false));

	std::unordered_map<uint64_t, std::set<std::string>> message_key_map;
	icd.PrepareMessageKeyMap(message_key_map);
	int64_t key1 = 39043;
	int64_t key2 = 48192 << 16 | 55360;
	int64_t key3 = 55360;
	int64_t key4 = 28894 << 16;

	// Ensure four keys were found
	ASSERT_EQ(message_key_map.size(), 4);
	
	// Ensure each key was found
	ASSERT_EQ(message_key_map.count(key1), 1);
	ASSERT_EQ(message_key_map.count(key2), 1);
	ASSERT_EQ(message_key_map.count(key3), 1);
	ASSERT_EQ(message_key_map.count(key4), 1);

	ASSERT_THAT(message_key_map[key1], 
		::testing::ElementsAre("Bus1", "Bus2", "Bus3", "Bus4" ));

	ASSERT_THAT(message_key_map[key2], 
		::testing::ElementsAre("Bus3", "Bus4", "Bus5" ));

	ASSERT_THAT(message_key_map[key3], 
		::testing::ElementsAre("Bus1", "Bus2" ));

	ASSERT_THAT(message_key_map[key4], 
		::testing::ElementsAre("Bus1", "Bus2" ));
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
			"RPG4_I,RPG4_I-01,00000,39043,20,BD3,MIVU,23,ORT,19,00,04,1.00,00,02,05,0,0,01,00,00,0,description description,21474348.000000000000,SECONDS"
		};
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
	bus_lruaddrs_map_["test"] = std::set<uint64_t>({ 10, 100 });
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
	update_map_["BD1"] = std::set<uint64_t>({ 13 });
	update_map_["BD2"] = std::set<uint64_t>({ 14 });
	ASSERT_EQ(icd_.ReplaceBusNameWithChannelIDInLookup(update_map_), false);
}

TEST_F(ICDDataPrepareICDQueryTest, ReplaceBusNameWithChannelIDInLookupFailIfBadMap)
{
	// If the initial result is not true, the test is meaningless.
	res_ = icd_.PrepareICDQuery(icd_lines_);
	ASSERT_EQ(res_, true);

	// These map names don't match those in the ICD.
	update_map_["BUSA"] = std::set<uint64_t>({ 13 });
	update_map_["BUSB"] = std::set<uint64_t>({ 14 });
	ASSERT_EQ(icd_.ReplaceBusNameWithChannelIDInLookup(update_map_), false);
}

TEST_F(ICDDataPrepareICDQueryTest, ReplaceBusNameWithChannelIDInLookupChanIDKeysCorrect)
{
	res_ = icd_.PrepareICDQuery(icd_lines_);
	ASSERT_EQ(res_, true);

	update_map_["BD1"] = std::set<uint64_t>({ 13 });
	update_map_["BD3"] = std::set<uint64_t>({ 14, 21 });
	ASSERT_EQ(icd_.ReplaceBusNameWithChannelIDInLookup(update_map_), true);
	ASSERT_THAT(icd_.GetLookupTableChannelIDKeys(), ::testing::UnorderedElementsAre(13, 14, 21));
}

TEST_F(ICDDataPrepareICDQueryTest, ReplaceBusNameWithChannelIDInLookupOneChanIDKeyCorrect)
{
	res_ = icd_.PrepareICDQuery(icd_lines_);
	ASSERT_EQ(res_, true);

	// These map names don't match those in the ICD.
	update_map_["BD1"] = std::set<uint64_t>({ 13, 17 });
	update_map_["BD4"] = std::set<uint64_t>({ 14 });
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
	update_map_["BD1"] = std::set<uint64_t>({ 13 });
	update_map_["BD3"] = std::set<uint64_t>({ 14, 21 });
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
	/*std::vector<ICDElement> icd_msg_elems_;
	std::vector<size_t> inds_vec_;
	std::vector<std::vector<size_t>> table_indices_;
	std::vector<std::string> table_names_;
	std::map<std::string, std::set<uint64_t>> bus_lruaddrs_map_;
	std::map<std::string, std::set<uint64_t>> update_map_;*/

	ICDDataIngestYamlTest() : res_(false), 
		msg_body_keys_({ "msg_data", "word_elem", "bit_elem" }) {}

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
			"    bit_elem: []\n"
		};

		// Root-level sequence, not map
		bad_icd_lines1_ = {
			"- msg1:\n",
			"    msg_data: {}\n",
			"    word_elem: []\n",
			"    bit_elem: []\n",
			"- msg2:\n",
			"    msg_data: {}\n",
			"    word_elem: []\n",
			"    bit_elem: []\n"
		};

	}

	void FillYamlMsgBodyNodes()
	{
		// correct msg body
		icd_node_ = YAML::Load({
			"    msg_data: {}\n"
			"    word_elem: []\n"
			"    bit_elem: []\n"
		});

		// incorrect, repeat msg_data entry
		bad_icd_node1_ = YAML::Load({
			"    msg_data: {}\n"
			"    word_elem: []\n"
			"    bit_elem: []\n"
			"    msg_data: {}\n"
		});

		// incorrect, missing bit_elem
		bad_icd_node2_ = YAML::Load( {
			"    msg_data: {}\n"
			"    word_elem: []\n"
		});

		// incorrect, not map
		bad_icd_node3_ = YAML::Load({
			"    - msg_data: {}\n"
			"    - word_elem: []\n"
			"    - bit_elem: []\n"
		});
	}

	void FillYamlElementsNodes()
	{
		/*
		* Test ndoes defined here are used for structure: Is the node a map? Does each key map
		* to a map? Does the secondary map have the correct keys? It is not meant to check
		* if the values can be interpreted as the correct data types.
		*/
		
		msg_data_node_ = YAML::Load(
			"{command: [40023, 0], lru_addr: [1, 2], lru_subaddr: [3, 0], lru_name: [LRUA, LRUB], bus: BUSB, wrdcnt: 20, rate: 12.5, mode_code: False, desc: \"description\"}\n"
			);

		// correct bit elems
		icd_node_ = YAML::Load(
			"APP_2-0301: {off: 2, cnt: 1, schema: UNSIGNEDBITS, msbval: 18, desc: \"description\", uom: \"DAY\", multifmt: False, class: 0, msb: 1, lsb: 9, bitcnt: 9}\n"
			"APP_2-0310: {off: 2, cnt: 1, schema: UNSIGNEDBITS, msbval: 323, desc: description, uom: YEAR, multifmt: False, class: 0, msb: 10, lsb: 15, bitcnt: 6}\n"
			"APP_2-0316: {off: 2, cnt: 1, schema: UNSIGNEDBITS, msbval: 1, desc: \"description / description\", uom: ' ', multifmt: False, class: 0, msb: 16, lsb: 16, bitcnt: 1}\n"
			);

		// For msg_data_node_ and icd_node_, element 2.
		string_icd_components1_ = {
			"APP_2","APP_2-0310","40023","0","20","BUSB","LRUA","1","LRUB","2","3","0","12.5",
			"2","1","12","1","0","10","15","6","0","description","323","YEAR"
		};

		// For msg_data_node_ and icd_node_, element 3.
		string_icd_components2_ = {
			"APP_2","APP_2-0316","40023","0","20","BUSB","LRUA","1","LRUB","2","3","0","12.5",
			"2","1","12","1","0","16","16","1","0","description / description","1"," "
		};

		// correct word elems
		good_icd_node1_ = YAML::Load(
			"APP_2-03 : {off: 2, cnt : 1, schema : UNSIGNED16, msbval : 18, "
				"desc : description, uom : DAY, multifmt : False, class : 0}\n"
			"APP_2-04 : {off: 2, cnt : 1, schema : UNSIGNED16, msbval : 323, "
				"desc : description, uom : YEAR, multifmt : False, class : 0}\n"
			"APP_2-05 : {off: 2, cnt : 1, schema : UNSIGNED16, msbval : 1000, "
				"desc : description / description, uom : ' ', multifmt : False, class : 0}\n"
			);

		// For msg_data_node_ and good_icd_node1_, element 3, APP_2-05.
		string_icd_components3_ = {
			"APP_2","APP_2-05","40023","0","20","BUSB","LRUA","1","LRUB","2","3","0","12.5",
			"2","1","4","0","0","0","0","0","0","description / description","1000"," "
		};

		// incorrect, is sequence of maps
		bad_icd_node1_ = YAML::Load(
			"- APP_2-03 : {off: 2, cnt : 1, schema : UNSIGNEDBITS, msbval : 18, "
				"desc : description, uom : DAY, multifmt : False, class : 0}\n"
			"- APP_2-04 : {off: 2, cnt : 1, schema : UNSIGNEDBITS, msbval : 323, "
				"desc : description, uom : YEAR, multifmt : False, class : 0}\n"
			"- APP_2-05 : {off: 2, cnt : 1, schema : UNSIGNEDBITS, msbval : 1, "
				"desc : description / description, uom : ' ', multifmt : False, class : 0}\n"
			);

		// one incorrect elem, value is not a map
		bad_icd_node2_ = YAML::Load(
			"APP_2-03 :\n" 
			"    - off: 2\n"
			"    - cnt : 1\n"
			"    - schema : UNSIGNEDBITS\n"
			"    - msbval : 18\n"
			"    - desc : description\n"
			"    - uom : DAY\n"
			"    - multifmt : False\n"
			"    - class : 0\n"
			"APP_2-04 : {off: 2, cnt : 1, schema : UNSIGNEDBITS, msbval : 323, "
				"desc : description, uom : YEAR, multifmt : False, class : 0}\n"
			"APP_2-05 : {off: 2, cnt : 1, schema : UNSIGNEDBITS, msbval : 1, "
				"desc : description / description, uom : ' ', multifmt : False, class : 0}\n"
			);

		// two incorrect elems, first is missing "schema", third is 
		// missing "multifmt".
		bad_icd_node3_ = YAML::Load(
			"APP_2-03 : {off: 2, cnt : 1, msbval : 18, "
				"desc : description, uom : DAY, multifmt : False, class : 0}\n"
			"APP_2-04 : {off: 2, cnt : 1, schema : UNSIGNEDBITS, msbval : 323, "
				"desc : description, uom : YEAR, multifmt : False, class : 0}\n"
			"APP_2-05 : {off: 2, cnt : 1, schema : UNSIGNEDBITS, msbval : 1, "
				"desc : description / description, uom : ' ', class : 0}\n"
			);
	}

	void FillSequenceNodes()
	{
		// sequence, 3 elements
		icd_node_ = YAML::Load({
			"[a, b, c]\n"
			});

		// sequence, 2 elements
		good_icd_node1_ = YAML::Load({
			"[a, b]\n"
			});

		// not sequence
		bad_icd_node1_ = YAML::Load({
			"key: [a, b]\n"
			});
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

TEST_F(ICDDataIngestYamlTest, IngestICDYamlFileLinesEmptyReturnsFalse)
{
	ASSERT_EQ(false, icd_.IngestICDYamlFileLines(icd_lines_, icd_elems_));
}

TEST_F(ICDDataIngestYamlTest, IngestICDYamlFileLinesRootIsMap)
{
	FillYamlBasicStructureLines();
	EXPECT_EQ(true, icd_.IngestICDYamlFileLines(icd_lines_, icd_elems_));
	EXPECT_EQ(false, icd_.IngestICDYamlFileLines(bad_icd_lines1_, icd_elems_));
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
		icd_node_, is_bit_node_, icd_elems_);
	EXPECT_EQ(expected_fill_count, reported_fill_count);
	EXPECT_EQ(expected_fill_count, icd_elems_.size());

	is_bit_node_ = false;
	icd_elems_.clear();
	reported_fill_count = icd_.FillElementsFromYamlNodes(msg_name_, msg_data_node_,
		good_icd_node1_, is_bit_node_, icd_elems_);
	EXPECT_EQ(expected_fill_count, reported_fill_count);
	EXPECT_EQ(expected_fill_count, icd_elems_.size());

	icd_elems_.clear();
	expected_fill_count = 0;
	reported_fill_count = icd_.FillElementsFromYamlNodes(msg_name_, msg_data_node_,
		bad_icd_node1_, is_bit_node_, icd_elems_);
	EXPECT_EQ(expected_fill_count, reported_fill_count);
	EXPECT_EQ(expected_fill_count, icd_elems_.size());

	icd_elems_.clear();
	expected_fill_count = 2;
	reported_fill_count = icd_.FillElementsFromYamlNodes(msg_name_, msg_data_node_,
		bad_icd_node2_, is_bit_node_, icd_elems_);
	EXPECT_EQ(expected_fill_count, reported_fill_count);
	EXPECT_EQ(expected_fill_count, icd_elems_.size());

	icd_elems_.clear();
	expected_fill_count = 1;
	reported_fill_count = icd_.FillElementsFromYamlNodes(msg_name_, msg_data_node_,
		bad_icd_node3_, is_bit_node_, icd_elems_);
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