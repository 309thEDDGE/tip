#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "iterable_tools.h"
#include "yaml-cpp/eventhandler.h"

class IterableToolsUniqueElementsTest : public ::testing::Test
{
protected:

	IterableTools it_;
	std::vector<int> test_int_;
	std::vector<int> res_int_;
	std::vector<std::string> test_str_;
	std::vector<std::string> res_str_;

	IterableToolsUniqueElementsTest()
	{

	}
	void SetUp() override
	{

	}

};

TEST_F(IterableToolsUniqueElementsTest, UniqueElementsAcceptsEmpty)
{
	res_int_ = it_.UniqueElements<int>(test_int_);
	ASSERT_EQ(0, res_int_.size());
}

TEST_F(IterableToolsUniqueElementsTest, UniqueElementsIntType)
{
	int input_ints[] = { 34, 6, 7, 45, 2, 8 };
	test_int_.resize(6);
	std::copy(input_ints, input_ints + 6, test_int_.begin());
	res_int_ = it_.UniqueElements<int>(test_int_);
	ASSERT_THAT(res_int_, ::testing::ElementsAre(2, 6, 7, 8, 34, 45));
}

TEST_F(IterableToolsUniqueElementsTest, UniqueElementsStringType)
{
	test_str_.push_back("abc");
	test_str_.push_back("hi my name is ");
	test_str_.push_back("roger");
	test_str_.push_back("Abc");
	res_str_ = it_.UniqueElements<std::string>(test_str_);
	ASSERT_THAT(res_str_, ::testing::ElementsAre("Abc", "abc", "hi my name is ", "roger"));
}


//
// VectorOfMember
//

class T0
{
public:
	int a;
	std::string b;
	std::vector<float> c;
	T0() {}
	T0(int ival, std::string sval, std::vector<float>& fval) : a(ival), b(sval), c(fval) {}
};

class IterableToolsVectorOfMemberTest : public ::testing::Test
{
protected:

	IterableTools it_;
	std::vector<T0> test_obj_;
	std::vector<T0> empty_obj_;
	std::vector<int> res_int_;
	std::vector<std::string> res_str_;
	std::vector<std::vector<float>> res_flt_;

	IterableToolsVectorOfMemberTest() {}
	void SetUp() override
	{
		std::vector<float> temp = { 30.3, 100.0, 0.554 };
		test_obj_.push_back(T0(3, "hi", temp));
		temp[0] = 6.0;
		test_obj_.push_back(T0(45, "anything else", temp));
		temp[1] = 50.0;
		temp[2] = 21.1;
		test_obj_.push_back(T0(2199, "hemisphere", temp));
	}

};

TEST_F(IterableToolsVectorOfMemberTest, VectorOfMemberAcceptsEmpty)
{
	res_int_ = it_.VectorOfMember<T0, int>(empty_obj_, &T0::a);
	ASSERT_THAT(res_int_, ::testing::ElementsAre());
}

TEST_F(IterableToolsVectorOfMemberTest, VectorOfMemberIntType)
{
	res_int_ = it_.VectorOfMember<T0, int>(test_obj_, &T0::a);
	ASSERT_THAT(res_int_, ::testing::ElementsAre(3, 45, 2199));
}

TEST_F(IterableToolsVectorOfMemberTest, VectorOfMemberStringType)
{
	res_str_ = it_.VectorOfMember<T0, std::string>(test_obj_, &T0::b);
	ASSERT_THAT(res_str_, ::testing::ElementsAre("hi", "anything else", "hemisphere"));
}

TEST_F(IterableToolsVectorOfMemberTest, VectorOfMemberVectorType)
{
	res_flt_ = it_.VectorOfMember<T0, std::vector<float>>(test_obj_, &T0::c);
	EXPECT_THAT(res_flt_[0], ::testing::ElementsAre(30.3, 100.0, 0.554));
	EXPECT_THAT(res_flt_[1], ::testing::ElementsAre(6.0, 100.0, 0.554));
	EXPECT_THAT(res_flt_[2], ::testing::ElementsAre(6.0, 50.0, 21.1));
}

// 
// IndicesOfMatching
//

TEST(IterableToolsTest, IndicesOfMatchingInt)
{
	IterableTools it;
	std::vector<int> test = { 3, 5, 8, 6, 11, 23455, 4312, 6 };
	int match = 6;
	std::vector<size_t> res = it.IndicesOfMatching<int>(test, match);
	ASSERT_THAT(res, ::testing::ElementsAre(3, 7));
}

TEST(IterableToolsTest, IndicesOfMatchingString)
{
	IterableTools it;
	std::vector<std::string> test = {"yelp", "hi", "none", "hi", "more"};
	std::string match = "hi";
	std::vector<size_t> res = it.IndicesOfMatching<std::string>(test, match);
	ASSERT_THAT(res, ::testing::ElementsAre(1, 3));
}

//
// CollectByIndices
//

TEST(IterableToolsTest, CollectByIndicesInt)
{
	IterableTools it;
	std::vector<int> test = { 3, 5, 8, 6, 11, 23455, 4312, 6 };
	std::vector<size_t> inds = { 5, 2 };
	std::vector<int> res = it.CollectByIndices<int>(test, inds);
	ASSERT_THAT(res, ::testing::ElementsAre(23455, 8));
}

TEST(IterableToolsTest, CollectByIndicesString)
{
	IterableTools it;
	std::vector<std::string> test = { "yelp", "hi", "none", "hi", "more" };
	std::vector<size_t> inds = { 0, 2, 4 };
	std::vector<std::string> res = it.CollectByIndices<std::string>(test, inds);
	ASSERT_THAT(res, ::testing::ElementsAre("yelp", "none", "more"));
}

//
// GetKeys/Vals
//

class IterableToolsGetKeysValsTest : public ::testing::Test
{
protected:

	IterableTools it_;
	std::map<std::string, int> test_map_;
	std::map<int, T0> test_class_map_;
	std::map<int, std::vector<float>> test_map_vec_;
	std::unordered_map<std::string, int> test_uomap_;
	std::unordered_map<int, T0> test_class_uomap_;
	std::unordered_map<int, std::vector<float>> test_uomap_vec_;

	std::vector<T0> class_vec_;
	std::vector<int> int_vec_;
	std::vector<std::string> str_vec_;
	std::vector<std::vector<float>> float_vec_vec_;
	//std::vector<std::vector<float>> res_flt_;

	IterableToolsGetKeysValsTest() {}
	void SetUp() override
	{
		std::vector<float> temp = { 30.3, 100.0, 0.554 };
		test_map_["abrupt"] = 23;
		test_uomap_["abrupt"] = 23;
		test_class_map_[2] = T0(3, "hi", temp);
		test_class_uomap_[2] = T0(3, "hi", temp);
		test_map_vec_[1] = temp;
		test_uomap_vec_[1] = temp;

		temp[0] = 6.0;
		test_map_["ab"] = 17;
		test_uomap_["ab"] = 17;
		test_class_map_[38] = T0(45, "anything else", temp);
		test_class_uomap_[38] = T0(45, "anything else", temp);
		test_map_vec_[10] = temp;
		test_uomap_vec_[10] = temp;

		temp[1] = 50.0;
		temp[2] = 21.1;
		test_map_["count"] = 600;
		test_uomap_["count"] = 600;
		test_class_map_[2198] = T0(2199, "hemisphere", temp);
		test_class_uomap_[2198] = T0(2199, "hemisphere", temp);
		test_map_vec_[100] = temp;
		test_uomap_vec_[100] = temp;
	}

};

TEST_F(IterableToolsGetKeysValsTest, GetKeysString)
{
	// regular map, map orders alphabetically for string keys.
	str_vec_ = it_.GetKeys<std::string, int>(test_map_);
	EXPECT_THAT(str_vec_, ::testing::ElementsAre("ab", "abrupt", "count"));

	// unordered map
	str_vec_ = it_.GetKeys<std::string, int>(test_uomap_);
	EXPECT_THAT(str_vec_, ::testing::UnorderedElementsAre("abrupt", "ab", "count"));
}

TEST_F(IterableToolsGetKeysValsTest, GetKeysInt)
{
	// regular map, map orders numerically for int keys.
	int_vec_ = it_.GetKeys<int, T0>(test_class_map_);
	EXPECT_THAT(int_vec_, ::testing::ElementsAre(2, 38, 2198));

	// unordered map
	int_vec_ = it_.GetKeys<int, T0>(test_class_uomap_);
	EXPECT_THAT(int_vec_, ::testing::UnorderedElementsAre(2, 38, 2198));
}

TEST_F(IterableToolsGetKeysValsTest, GetValsInt)
{
	// regular map, map orders numerically for int keys.
	int_vec_ = it_.GetVals<std::string, int>(test_map_);
	EXPECT_THAT(int_vec_, ::testing::ElementsAre(17, 23, 600));

	// unordered map
	int_vec_ = it_.GetVals<std::string, int>(test_uomap_);
	EXPECT_THAT(int_vec_, ::testing::UnorderedElementsAre(17, 23, 600));
}

TEST_F(IterableToolsGetKeysValsTest, GetValsVecFloat)
{
	// regular map, map orders numerically for int keys.
	float_vec_vec_ = it_.GetVals<int, std::vector<float>>(test_map_vec_);
	std::vector<float> t0 = test_map_vec_[1];
	std::vector<float> t1 = test_map_vec_[10];
	std::vector<float> t2 = test_map_vec_[100];
	EXPECT_THAT(float_vec_vec_, ::testing::ElementsAreArray({ t0, t1, t2 }));

	// unordered map
	float_vec_vec_ = it_.GetVals<int, std::vector<float>>(test_uomap_vec_);
	EXPECT_THAT(float_vec_vec_, ::testing::UnorderedElementsAreArray({ t0, t1, t2 }));
}

TEST_F(IterableToolsGetKeysValsTest, GetValsClass)
{
	// regular map, map orders numerically for int keys.
	// Note that only the member "a" is used to confirm that the
	// correct object is returned in the vector.
	class_vec_ = it_.GetVals<int, T0>(test_class_map_);
	std::vector<T0> expect_objs = { test_class_map_[2],  test_class_map_[38],  test_class_map_[2198] };
	EXPECT_THAT(class_vec_[0], ::testing::Field(&T0::a, expect_objs[0].a));
	EXPECT_THAT(class_vec_[1], ::testing::Field(&T0::a, expect_objs[1].a));
	EXPECT_THAT(class_vec_[2], ::testing::Field(&T0::a, expect_objs[2].a));

	// unordered map, ints
	class_vec_ = it_.GetVals<int, T0>(test_class_uomap_);
	std::vector<T0> expect_objs2 = { test_class_uomap_[2],  test_class_uomap_[38],  test_class_uomap_[2198] };
	std::vector<int> test_ints = it_.VectorOfMember<T0, int>(class_vec_, &T0::a);
	std::vector<int> expect_ints = it_.VectorOfMember<T0, int>(expect_objs2, &T0::a);
	EXPECT_THAT(test_ints, ::testing::UnorderedElementsAreArray(expect_ints));
}

//
// GetIterablePrintString
//

TEST(IterableToolsTest, GetIterablePrintStringIntVector)
{
	IterableTools it;
	std::string title = "My Vector";
	std::vector<int> test_vec = { 1, 2, 3, 4, 5 };
	std::string fmt = "%02d";
	std::string delim = ", ";
	std::string expect = "My Vector:\n01, 02, 03, 04, 05\n";
	ASSERT_EQ(expect, it.GetIterablePrintString<typename std::vector<int>>(test_vec, title,
		fmt, delim));
}

TEST(IterableToolsTest, GetIterablePrintStringStrVector)
{
	IterableTools it;
	std::string title = "My Vector";
	std::vector<std::string> test_vec = { "1", "2", "3", "4", "5" };
	std::string fmt = "%s";
	std::string delim = ", ";
	std::string expect = "My Vector:\n1, 2, 3, 4, 5\n";
	ASSERT_EQ(expect, it.GetIterablePrintString(test_vec, title, fmt, delim));
}

//
// GroupByMember
//

struct S0
{
	int a;
	std::string b;
	std::vector<float> c;

	S0(int aval, std::string bval, std::vector<float> cval)
	{
		a = aval;
		b = bval;
		c = cval;
	}
};

class IterableToolsGroupByMemberTest : public ::testing::Test
{
protected:

	IterableTools it_;
	std::vector<T0> test_obj_;
	std::vector<T0> empty_obj_;
	std::vector<struct S0> test_struct_;
	std::vector<struct S0> empty_struct_;
	std::unordered_map<int, std::vector<T0>> res_int_;
	std::unordered_map<std::string, std::vector<T0>> res_str_;
	std::unordered_map<int, std::vector<S0>> struct_res_int_;

	IterableToolsGroupByMemberTest() {}
	void SetUp() override
	{
		std::vector<float> temp = { 30.3, 100.0, 0.554 };
		test_obj_.push_back(T0(3, "hi", temp));
		test_struct_.push_back(S0(3, "hi", temp));
		temp[0] = 6.0;
		test_obj_.push_back(T0(45, "anything else", temp));
		test_struct_.push_back(S0(45, "anything else", temp));
		temp[1] = 50.0;
		temp[2] = 21.1;
		test_obj_.push_back(T0(2199, "hemisphere", temp));
		test_struct_.push_back(S0(2199, "hemisphere", temp));
		temp[0] = 33.0;
		test_obj_.push_back(T0(45, "underwater", temp));
		test_obj_.push_back(T0(541, "hi", temp));
		test_struct_.push_back(S0(45, "underwater", temp));
		test_struct_.push_back(S0(541, "hi", temp));
	}
};

TEST_F(IterableToolsGroupByMemberTest, GroupByMemberClassInt)
{
	res_int_ = it_.GroupByMember<T0, int>(test_obj_, &T0::a);

	// There ought to be four groups of objects, with groups 3, 45, 541, 2199.
	EXPECT_EQ(res_int_.size(), 4);

	// The group with key = 45 must have two items in the vector
	EXPECT_EQ(res_int_[45].size(), 2);

	// The other three groups must only have one item each.
	EXPECT_EQ(res_int_[3].size(), 1);
	EXPECT_EQ(res_int_[2199].size(), 1);
	EXPECT_EQ(res_int_[541].size(), 1);

	// The "b" member (string) of the group key = 45 objects must be
	// "anything else" and "underwater".
	EXPECT_EQ(res_int_[45][0].b, "anything else");
	EXPECT_EQ(res_int_[45][1].b, "underwater");
}

TEST_F(IterableToolsGroupByMemberTest, GroupByMemberClassString)
{
	res_str_ = it_.GroupByMember<T0, std::string>(test_obj_, &T0::b);

	// There ought to be four groups of objects, with groups "hi",
	// "anything else", "hemisphere", "underwater"
	EXPECT_EQ(res_str_.size(), 4);

	// The group with key = "hi" must have two items in the vector
	EXPECT_EQ(res_str_["hi"].size(), 2);

	// The other three groups must only have one item each.
	EXPECT_EQ(res_str_["anything else"].size(), 1);
	EXPECT_EQ(res_str_["hemisphere"].size(), 1);
	EXPECT_EQ(res_str_["underwater"].size(), 1);

	// The a member of the group key = "hi" objects must be
	// "anything else" and "underwater".
	EXPECT_EQ(res_str_["hi"][0].a, 3);
	EXPECT_EQ(res_str_["hi"][1].a, 541);
}

TEST_F(IterableToolsGroupByMemberTest, GroupByMemberStructInt)
{
	struct_res_int_ = it_.GroupByMember<S0, int>(test_struct_, &S0::a);

	// There ought to be four groups of objects, with groups 3, 45, 541, 2199.
	EXPECT_EQ(struct_res_int_.size(), 4);

	// The group with key = 45 must have two items in the vector
	EXPECT_EQ(struct_res_int_[45].size(), 2);

	// The other three groups must only have one item each.
	EXPECT_EQ(struct_res_int_[3].size(), 1);
	EXPECT_EQ(struct_res_int_[2199].size(), 1);
	EXPECT_EQ(struct_res_int_[541].size(), 1);

	// The "b" member (string) of the group key = 45 objects must be
	// "anything else" and "underwater".
	EXPECT_EQ(struct_res_int_[45][0].b, "anything else");
	EXPECT_EQ(struct_res_int_[45][1].b, "underwater");
}

//
// Sort
//

// Set not tested because sets are sorted by design.
TEST(IterableTools, SortInt)
{
	IterableTools it;
	std::vector<int> test = { 5, 3, 4, 0, 2 };
	std::vector<int> expected = { 0, 2, 3, 4, 5 };
	std::vector<int> res = it.Sort<int>(test);
	ASSERT_THAT(expected, ::testing::ElementsAreArray(res));
}

TEST(IterableTools, SortString)
{
	IterableTools it;
	std::vector<std::string> test = { "a", "dog", "d", "it", "help"};
	std::vector<std::string> expected = { "a", "d", "dog", "help", "it" };
	std::vector<std::string> res = it.Sort<std::string>(test);
	ASSERT_THAT(expected, ::testing::ElementsAreArray(res));
}

//
// Intersection
//

TEST(IterableTools, IntersectionVecInt)
{
	IterableTools it;
	std::vector<int> test1 = { 5, 3, 4, 0, 2 };
	std::vector<int> test2 = {0, 2, 54, 110 };
	std::vector<int> expected = { 0, 2 };
	std::vector<int> res = it.Intersection<int>(test1, test2);
	ASSERT_THAT(expected, ::testing::ElementsAreArray(res));
}

TEST(IterableTools, IntersectionVecString)
{
	IterableTools it;
	std::vector<std::string> test1 = { "a", "b", "c", "d"};
	std::vector<std::string> test2 = { "b", "d" };
	std::vector<std::string> expected = test2;
	std::vector<std::string> res = it.Intersection<std::string>(test1, test2);
	ASSERT_THAT(expected, ::testing::ElementsAreArray(res));
}

TEST(IterableTools, IntersectionSetInt)
{
	IterableTools it;
	std::set<int> test1 = { 5, 3, 4, 0, 2 };
	std::set<int> test2 = { 0, 2, 54, 110 };
	std::set<int> expected = { 0, 2 };
	std::set<int> res = it.Intersection<int>(test1, test2);
	ASSERT_THAT(expected, ::testing::ElementsAreArray(res));
}

TEST(IterableTools, IntersectionSetString)
{
	IterableTools it;
	std::set<std::string> test1 = { "a", "b", "c", "d" };
	std::set<std::string> test2 = { "b", "d" };
	std::set<std::string> expected = test2;
	std::set<std::string> res = it.Intersection<std::string>(test1, test2);
	ASSERT_THAT(expected, ::testing::ElementsAreArray(res));
}

//
// Union
//

TEST(IterableTools, UnionVecInt)
{
	IterableTools it;
	std::vector<int> test1 = { 5, 3, 4, 0, 2 };
	std::vector<int> test2 = { 0, 2, 54, 110 };
	std::vector<int> expected = { 0, 2, 3, 4, 5, 54, 110 };
	std::vector<int> res = it.Union<int>(test1, test2);
	ASSERT_THAT(res, ::testing::ElementsAreArray(expected));
}

TEST(IterableTools, UnionVecString)
{
	IterableTools it;
	std::vector<std::string> test1 = { "a", "b", "c", "d" };
	std::vector<std::string> test2 = { "b", "d" };
	std::vector<std::string> expected = test1;
	std::vector<std::string> res = it.Union<std::string>(test1, test2);
	ASSERT_THAT(res, ::testing::ElementsAreArray(expected));
}

TEST(IterableTools, UnionSetInt)
{
	IterableTools it;
	std::set<int> test1 = { 5, 3, 4, 0, 2 };
	std::set<int> test2 = { 0, 2, 54, 110 };
	std::set<int> expected = { 0, 2, 3, 4, 5, 54, 110 };
	std::set<int> res = it.Union<int>(test1, test2);
	ASSERT_THAT(res, ::testing::ElementsAreArray(expected));
}

TEST(IterableTools, UnionSetString)
{
	IterableTools it;
	std::set<std::string> test1 = { "a", "b", "c", "d" };
	std::set<std::string> test2 = { "b", "d" };
	std::set<std::string> expected = test1;
	std::set<std::string> res = it.Union<std::string>(test1, test2);
	ASSERT_THAT(res, ::testing::ElementsAreArray(expected));
}

//
// IsSubset
//

TEST(IterableTools, IsSubsetVecInt)
{
	// test1 is subset of test2
	IterableTools it;
	std::vector<int> test1 = { 5, 3, 4, 0, 2 };
	std::vector<int> test2 = { 0, 2, 5, 110, 4, 3 };
	bool res = it.IsSubset<int>(test1, test2);
	EXPECT_EQ(true, res);

	// test1a is NOT subset of test2
	std::vector<int> test1a = { 5, 3, 4, 0, 2, 120};
	res = it.IsSubset<int>(test1a, test2);
	EXPECT_EQ(false, res);
}

TEST(IterableTools, IsSubsetSetInt)
{
	// test1 is subset of test2
	IterableTools it;
	std::set<int> test1 = { 5, 3, 4, 0, 2 };
	std::set<int> test2 = { 0, 2, 5, 110, 4, 3 };
	bool res = it.IsSubset<int>(test1, test2);
	EXPECT_EQ(true, res);

	// test1a is NOT subset of test2
	std::set<int> test1a = { 5, 3, 4, 0, 2, 120 };
	res = it.IsSubset<int>(test1a, test2);
	EXPECT_EQ(false, res);
}

//
// ArgSortAscending
//

TEST(IterableTools, ArgSortAscendingPosInt)
{
	IterableTools it;
	std::vector<int> test= { 200, 2, 43, 4, 56, 7 };
	std::vector<size_t> res = it.ArgSortAscending<int>(test);
	ASSERT_THAT(res, ::testing::ElementsAre(1, 3, 5, 2, 4, 0));
}

TEST(IterableTools, ArgSortAscendingPosIntZeros)
{
	IterableTools it;
	std::vector<int> test = { 200, 0, 43, 0, 56, 7 };
	std::vector<size_t> res = it.ArgSortAscending<int>(test);
	ASSERT_THAT(res, ::testing::ElementsAre(1, 3, 5, 2, 4, 0));
}

TEST(IterableTools, ArgSortAscendingPosNegInt)
{
	IterableTools it;
	std::vector<int> test = { 200, 2, 43, 0, 56, -7 };
	std::vector<size_t> res = it.ArgSortAscending<int>(test);
	ASSERT_THAT(res, ::testing::ElementsAre(5, 3, 1, 2, 4, 0));
}

TEST(IterableTools, ArgSortAscendingNegInt)
{
	IterableTools it;
	std::vector<int> test = { -200, -2, -43, -4, -56, -7 };
	std::vector<size_t> res = it.ArgSortAscending<int>(test);
	ASSERT_THAT(res, ::testing::ElementsAre(0, 4, 2, 5, 3, 1));
}

TEST(IterableTools, ArgSortAscendingString)
{
	IterableTools it;
	std::vector<std::string> test = { "my", "test", "hi", "ab", "abdominal"};
	std::vector<size_t> res = it.ArgSortAscending<std::string>(test);
	ASSERT_THAT(res, ::testing::ElementsAre(3, 4, 2, 0, 1));
}

//
// ArgSortDescending
//

TEST(IterableTools, ArgSortDescendingPosInt)
{
	IterableTools it;
	std::vector<int> test = { 200, 2, 43, 4, 56, 7 };
	std::vector<size_t> res = it.ArgSortDescending<int>(test);
	ASSERT_THAT(res, ::testing::ElementsAre(0, 4, 2, 5, 3, 1));
}

TEST(IterableTools, ArgSortDescendingPosIntZeros)
{
	IterableTools it;
	std::vector<int> test = { 200, 0, 43, 0, 56, 7 };
	std::vector<size_t> res = it.ArgSortDescending<int>(test);
	ASSERT_THAT(res, ::testing::ElementsAre(0, 4, 2, 5, 1, 3));
}

TEST(IterableTools, ArgSortDescendingPosNegInt)
{
	IterableTools it;
	std::vector<int> test = { 200, 2, 43, 0, 56, -7 };
	std::vector<size_t> res = it.ArgSortDescending<int>(test);
	ASSERT_THAT(res, ::testing::ElementsAre(0, 4, 2, 1, 3, 5));
}

TEST(IterableTools, ArgSortDescendingNegInt)
{
	IterableTools it;
	std::vector<int> test = { -200, -2, -43, -4, -56, -7 };
	std::vector<size_t> res = it.ArgSortDescending<int>(test);
	ASSERT_THAT(res, ::testing::ElementsAre(1, 3, 5, 2, 4, 0));
}

TEST(IterableTools, ArgSortDescendingString)
{
	IterableTools it;
	std::vector<std::string> test = { "my", "test", "hi", "ab", "abdominal" };
	std::vector<size_t> res = it.ArgSortDescending<std::string>(test);
	ASSERT_THAT(res, ::testing::ElementsAre(1, 0, 2, 4, 3));
}

//
// SortMapOfIterableByValueSize
//

class IterableToolsSortMapToIterableByValueSize : public ::testing::Test
{
protected:

	IterableTools it_;
	std::map<std::string, std::vector<int>> test_map_intvec_;
	std::map<int, std::vector<std::string>> test_map_strvec_;
	std::map<std::string, std::vector<int>> test_uomap_intvec_;
	std::map<int, std::vector<std::string>> test_uomap_strvec_;
	std::vector<int> res_int_;
	std::vector<std::string> res_str_;

	IterableToolsSortMapToIterableByValueSize()
	{

	}
	void SetUp() override
	{
		std::vector<int> int_vals = { 1, 2 };
		std::vector<std::string> str_vals = { "a", "b" };
		test_map_intvec_["hi"] = int_vals; // 2 elements
		test_uomap_intvec_["hi"] = int_vals;
		test_map_strvec_[20] = str_vals; // 2 elements
		test_uomap_strvec_[20] = str_vals;

		int_vals.push_back(30); 
		int_vals.push_back(40);
		str_vals.push_back("yo");
		test_map_intvec_["my"] = int_vals; // 4 elements
		test_uomap_intvec_["my"] = int_vals;
		test_map_strvec_[25] = str_vals; // 3 elements
		test_uomap_strvec_[25] = str_vals;

		int_vals.push_back(210);
		int_vals.push_back(111);
		str_vals.push_back("ten");
		test_map_intvec_["bid"] = int_vals; // 6 elements
		test_uomap_intvec_["bid"] = int_vals;
		test_map_strvec_[18] = str_vals; // 4 elements
		test_uomap_strvec_[18] = str_vals;

		int_vals.push_back(50);
		str_vals.push_back("mine");
		test_map_intvec_["farewell"] = int_vals; // 7 elements
		test_uomap_intvec_["farewell"] = int_vals;
		test_map_strvec_[70] = str_vals; // 5 elements
		test_uomap_strvec_[70] = str_vals;

		std::vector<int> int_vals2 = { 1230 };
		std::vector<std::string> str_vals2 = { "blimpie" };
		test_map_intvec_["lofi"] = int_vals2; // 1 element
		test_uomap_intvec_["lofi"] = int_vals2;
		test_map_strvec_[8008] = str_vals2; // 1 element
		test_uomap_strvec_[8008] = str_vals2;
	}

};

TEST_F(IterableToolsSortMapToIterableByValueSize, SortMapToIterableByValueSizeIntVec)
{
	res_str_ = it_.SortMapToIterableByValueSize<std::string, 
		typename std::vector<int>>(test_map_intvec_);
	ASSERT_THAT(res_str_, ::testing::ElementsAre("lofi", "hi", "my", "bid", 
		"farewell"));
}

TEST_F(IterableToolsSortMapToIterableByValueSize, SortMapToIterableByValueSizeStrVec)
{
	res_int_ = it_.SortMapToIterableByValueSize<int,
		typename std::vector<std::string>>(test_map_strvec_);
	ASSERT_THAT(res_int_, ::testing::ElementsAre(8008, 20, 25, 18, 70));
}

TEST_F(IterableToolsSortMapToIterableByValueSize, SortMapToIterableByValueSizeUOMapIntVec)
{
	res_str_ = it_.SortMapToIterableByValueSize<std::string,
		typename std::vector<int>>(test_uomap_intvec_);
	ASSERT_THAT(res_str_, ::testing::ElementsAre("lofi", "hi", "my", "bid",
		"farewell"));
}

TEST_F(IterableToolsSortMapToIterableByValueSize, SortMapToIterableByValueSizeUOMapStrVec)
{
	res_int_ = it_.SortMapToIterableByValueSize<int,
		typename std::vector<std::string>>(test_uomap_strvec_);
	ASSERT_THAT(res_int_, ::testing::ElementsAre(8008, 20, 25, 18, 70));
}

TEST_F(IterableToolsSortMapToIterableByValueSize, SortMapToIterableByValueSizeIntVecTwoWithSameSize)
{
	test_map_intvec_["new"] = std::vector<int>({ 55, 66 });
	res_str_ = it_.SortMapToIterableByValueSize<std::string,
		typename std::vector<int>>(test_map_intvec_);

	// We don't know if the order of "new" and "hi", both with two elements,
	// will be the former or "hi", "new". Ensure that "new" is present.
	//EXPECT_THAT(res_str_, ::testing::IsSupersetOf({ "lofi", "hi", "my", "bid",
	//	"farewell" }));
	//EXPECT_THAT(res_str_, ::testing::Contains("new"));
	EXPECT_THAT(res_str_, ::testing::AnyOf(::testing::ElementsAreArray({ "lofi", "hi", "new", "my", "bid", "farewell" }),
					       ::testing::ElementsAreArray({ "lofi", "new", "hi", "my", "bid", "farewell" })));
}

//
// ReverseMap
//

TEST(IterableTools, ReverseMapIntString)
{
	IterableTools it;
	std::map<int, std::string> test = { {1, "hi"}, {2, "mip"}, {3, "do"} };
	bool lost_entries;
	std::map<std::string, int> res = it.ReverseMap<int, std::string>(test, lost_entries);
	EXPECT_EQ(res["hi"], 1);
	EXPECT_EQ(res["mip"], 2);
	EXPECT_EQ(res["do"], 3);
	EXPECT_EQ(lost_entries, false);
}

TEST(IterableTools, ReverseMapStringInt)
{
	IterableTools it;
	std::map<std::string, int> test = { {"hi", 1}, {"mip", 2}, {"do", 3} };
	bool lost_entries;
	std::map<int, std::string> res = it.ReverseMap<std::string, int>(test, lost_entries);
	EXPECT_EQ(res[1], "hi");
	EXPECT_EQ(res[2], "mip");
	EXPECT_EQ(res[3], "do");
	EXPECT_EQ(lost_entries, false);
}

TEST(IterableTools, ReverseMapIntStringLostEntries)
{
	IterableTools it;
	std::map<int, std::string> test = { {1, "hi"}, {2, "mip"}, {3, "do"}, {10, "do"} };
	bool lost_entries;
	std::map<std::string, int> res = it.ReverseMap<int, std::string>(test, lost_entries);
	EXPECT_EQ(res["hi"], 1);
	EXPECT_EQ(res["mip"], 2);
	EXPECT_EQ(res["do"], 10);
	EXPECT_EQ(lost_entries, true);
}

TEST(IterableTools, ReverseMapUOIntString)
{
	IterableTools it;
	std::unordered_map<int, std::string> test = { {1, "hi"}, {2, "mip"}, {3, "do"} };
	bool lost_entries;
	std::unordered_map<std::string, int> res = it.ReverseMap<int, std::string>(test, lost_entries);
	EXPECT_EQ(res["hi"], 1);
	EXPECT_EQ(res["mip"], 2);
	EXPECT_EQ(res["do"], 3);
	EXPECT_EQ(lost_entries, false);
}

TEST(IterableTools, ReverseMapUOStringInt)
{
	IterableTools it;
	std::unordered_map<std::string, int> test = { {"hi", 1}, {"mip", 2}, {"do", 3} };
	bool lost_entries;
	std::unordered_map<int, std::string> res = it.ReverseMap<std::string, int>(test, lost_entries);
	EXPECT_EQ(res[1], "hi");
	EXPECT_EQ(res[2], "mip");
	EXPECT_EQ(res[3], "do");
	EXPECT_EQ(lost_entries, false);
}

TEST(IterableTools, ReverseMapUOIntStringLostEntries)
{
	IterableTools it;
	std::unordered_map<int, std::string> test = { {1, "hi"}, {2, "mip"}, {3, "do"}, {4, "hi"} };
	bool lost_entries;
	std::unordered_map<std::string, int> res = it.ReverseMap<int, std::string>(test, lost_entries);
	//EXPECT_EQ(res["hi"], 4);
	EXPECT_THAT(res["hi"], ::testing::AnyOf(::testing::Eq(4), ::testing::Eq(1)));
	EXPECT_EQ(res["mip"], 2);
	EXPECT_EQ(res["do"], 3);
	EXPECT_EQ(lost_entries, true);
}


//
// IsKeyInMap
//

TEST(IterableTools, IsKeyInMapIntKey)
{
	IterableTools it;
	std::map<int, std::string> test_map = { {1, "hi"}, {2, "mip"}, {3, "do"} };
	int test_key = 4;
	bool res = it.IsKeyInMap<int, std::string>(test_map, test_key);
	EXPECT_EQ(res, false);

	test_key = 3;
	res = it.IsKeyInMap<int, std::string>(test_map, test_key);
	EXPECT_EQ(res, true);
}

TEST(IterableTools, IsKeyInMapStrKey)
{
	IterableTools it;
	std::map<std::string, int> test_map = { {"hi", 1}, {"mip", 2}, {"do", 3} };
	std::string test_key = "hello";
	bool res = it.IsKeyInMap<std::string, int>(test_map, test_key);
	EXPECT_EQ(res, false);

	test_key = "mip";
	res = it.IsKeyInMap<std::string>(test_map, test_key);
	EXPECT_EQ(res, true);
}

TEST(IterableTools, IsKeyInMapUOIntKey)
{
	IterableTools it;
	std::unordered_map<int, std::string> test_map = { {1, "hi"}, {2, "mip"}, {3, "do"} };
	int test_key = 4;
	bool res = it.IsKeyInMap<int, std::string>(test_map, test_key);
	EXPECT_EQ(res, false);

	test_key = 3;
	res = it.IsKeyInMap<int, std::string>(test_map, test_key);
	EXPECT_EQ(res, true);
}

TEST(IterableTools, IsKeyInMapUOStrKey)
{
	IterableTools it;
	std::unordered_map<std::string, int> test_map = { {"hi", 1}, {"mip", 2}, {"do", 3} };
	std::string test_key = "hello";
	bool res = it.IsKeyInMap<std::string, int>(test_map, test_key);
	EXPECT_EQ(res, false);

	test_key = "mip";
	res = it.IsKeyInMap<std::string>(test_map, test_key);
	EXPECT_EQ(res, true);
}

//
// UpdateMapKeys
//

TEST(IterableTools,UpdateMapKeysIntKey)
{
	IterableTools it;
	std::map<int, std::string> test_map = { {1, "hi"}, {2, "mip"}, {3, "do"} };
	std::map<int, int> update_map = { {1, 500}, {200, 4333}, {2, 7} };
	std::map<int, std::string> res = it.UpdateMapKeys<int, std::string>(test_map, 
		update_map);
	EXPECT_EQ(res[500], "hi");
	EXPECT_EQ(res[7], "mip");
	EXPECT_EQ(res[3], "do");
}

TEST(IterableTools, UpdateMapKeysStrKey)
{
	IterableTools it;
	std::map<std::string, int> test_map = { {"hi", 1}, {"mip", 2}, {"do", 3} };
	std::map<std::string, std::string> update_map = { {"dog", "cat"}, {"do", "map"}, 
		{"chueck", "bill"} };
	std::map<std::string, int> res = it.UpdateMapKeys<std::string, int>(test_map, update_map);
	EXPECT_EQ(res["hi"], 1);
	EXPECT_EQ(res["map"], 3);
	EXPECT_EQ(res["mip"], 2);
}

TEST(IterableTools, UpdateMapKeysUOIntKey)
{
	IterableTools it;
	std::unordered_map<int, std::string> test_map = { {1, "hi"}, {2, "mip"}, {3, "do"} };
	std::unordered_map<int, int> update_map = { {1, 500}, {200, 4333}, {2, 7} };
	std::unordered_map<int, std::string> res = it.UpdateMapKeys<int, std::string>(test_map,
		update_map);
	EXPECT_EQ(res[500], "hi");
	EXPECT_EQ(res[7], "mip");
	EXPECT_EQ(res[3], "do");
}

TEST(IterableTools, UpdateMapKeysUOStrKey)
{
	IterableTools it;
	std::unordered_map<std::string, int> test_map = { {"hi", 1}, {"mip", 2}, {"do", 3} };
	std::unordered_map<std::string, std::string> update_map = { {"dog", "cat"}, {"do", "map"},
		{"chueck", "bill"} };
	std::unordered_map<std::string, int> res = it.UpdateMapKeys<std::string, int>(test_map, update_map);
	EXPECT_EQ(res["hi"], 1);
	EXPECT_EQ(res["map"], 3);
	EXPECT_EQ(res["mip"], 2);
}

//
// UpdateMapVals
//

TEST(IterableTools, UpdateMapValsStrVal)
{
	IterableTools it;
	std::map<int, std::string> test_map = { {1, "hi"}, {2, "mip"}, {3, "do"} };
	std::map<std::string, std::string> update_map = { {"dog", "cat"}, {"do", "map"},
		{"chueck", "bill"} };
	std::map<int, std::string> res = it.UpdateMapVals<int, std::string>(test_map,
		update_map);
	EXPECT_EQ(res[1], "hi");
	EXPECT_EQ(res[2], "mip");
	EXPECT_EQ(res[3], "map");
}

TEST(IterableTools, UpdateMapValsIntVal)
{
	IterableTools it;
	std::map<std::string, int> test_map = { {"hi", 1}, {"mip", 2}, {"do", 3} };
	std::map<int, int> update_map = { {1, 500}, {200, 4333}, {2, 7} };
	std::map<std::string, int> res = it.UpdateMapVals<std::string, int>(test_map, update_map);
	EXPECT_EQ(res["hi"], 500);
	EXPECT_EQ(res["mip"], 7);
	EXPECT_EQ(res["do"], 3);
}

TEST(IterableTools, UpdateMapValsUOStrVal)
{
	IterableTools it;
	std::unordered_map<int, std::string> test_map = { {1, "hi"}, {2, "mip"}, {3, "do"} };
	std::unordered_map<std::string, std::string> update_map = { {"dog", "cat"}, {"do", "map"},
		{"chueck", "bill"} };
	std::unordered_map<int, std::string> res = it.UpdateMapVals<int, std::string>(test_map,
		update_map);
	EXPECT_EQ(res[1], "hi");
	EXPECT_EQ(res[2], "mip");
	EXPECT_EQ(res[3], "map");
}

TEST(IterableTools, UpdateMapValsUOIntVal)
{
	IterableTools it;
	std::unordered_map<std::string, int> test_map = { {"hi", 1}, {"mip", 2}, {"do", 3} };
	std::unordered_map<int, int> update_map = { {1, 500}, {200, 4333}, {2, 7} };
	std::unordered_map<std::string, int> res = it.UpdateMapVals<std::string, int>(test_map, update_map);
	EXPECT_EQ(res["hi"], 500);
	EXPECT_EQ(res["mip"], 7);
	EXPECT_EQ(res["do"], 3);
}


TEST(IterableTools, VecToSet)
{
	IterableTools it;
	std::vector<int> in_vec;
	in_vec.push_back(1);
	in_vec.push_back(2);
	in_vec.push_back(2);
	in_vec.push_back(3);
	in_vec.push_back(4);

	std::set<int> compare_set;
	compare_set.insert(1);
	compare_set.insert(2);
	compare_set.insert(3);
	compare_set.insert(4);

	std::set<int> return_set = it.VecToSet(in_vec);
	EXPECT_TRUE(return_set == compare_set);
}
//
// GetKeysByValue
//

TEST(IterableTools, GetKeysByValueIntKeyStrVal)
{
	IterableTools it;
	std::map<int, std::string> test_map = { {1, "hi"}, {2, "mip"}, {3, "do"}, {10, "hi"} };
	std::string match_val = "hi";
	std::set<int>res = it.GetKeysByValue<int, std::string>(test_map, match_val);
	ASSERT_THAT(res, ::testing::UnorderedElementsAre( 1, 10 ));
}

TEST(IterableTools, GetKeysByValueStrKeyIntVal)
{
	IterableTools it;
	std::map<std::string, int> test_map = { {"hi", 1}, {"mip", 2}, {"do", 3}, {"dog", 2} };
	int match_val = 2;
	std::set<std::string> res = it.GetKeysByValue<std::string, int>(test_map, match_val);
	ASSERT_THAT(res, ::testing::UnorderedElementsAre("dog", "mip"));
}

TEST(IterableTools, GetKeysByValueUOIntKeyStrVal)
{
	IterableTools it;
	std::unordered_map<int, std::string> test_map = { {1, "hi"}, {2, "mip"}, {3, "do"}, {10, "hi"} };
	std::string match_val = "hi";
	std::set<int>res = it.GetKeysByValue<int, std::string>(test_map, match_val);
	ASSERT_THAT(res, ::testing::UnorderedElementsAre(1, 10));
}

TEST(IterableTools, GetKeysByValueUOStrKeyIntVal)
{
	IterableTools it;
	std::unordered_map<std::string, int> test_map = { {"hi", 1}, {"mip", 2}, {"do", 3}, {"dog", 2} };
	int match_val = 2;
	std::set<std::string> res = it.GetKeysByValue<std::string, int>(test_map, match_val);
	ASSERT_THAT(res, ::testing::UnorderedElementsAre("dog", "mip"));
}

TEST(IterableTools, EqualSetsBackwardOrder)
{
	IterableTools it;
	std::set<int> test1 = std::set<int>({ 1,2,3 });
	std::set<int> test2 = std::set<int>({ 3,2,1 });

	EXPECT_TRUE(it.EqualSets(test1, test2));
}

TEST(IterableTools, EqualSetsNonMatchingSubsets)
{
	IterableTools it;
	std::set<int> test1 = std::set<int>({ 1,2,3,4 });
	std::set<int> test2 = std::set<int>({ 1,2,3 });

	EXPECT_FALSE(it.EqualSets(test1, test2));
}

TEST(IterableTools, EqualSetsNonMatchingSubsets2)
{
	IterableTools it;
	std::set<int> test1 = std::set<int>({ 1,2,3 });
	std::set<int> test2 = std::set<int>({ 1,2,3,4 });

	EXPECT_FALSE(it.EqualSets(test1, test2));
}

TEST(IterableTools, EqualSets)
{
	IterableTools it;
	std::set<int> test1 = std::set<int>({ 1,2,3 });
	std::set<int> test2 = std::set<int>({ 1,2,3 });

	EXPECT_TRUE(it.EqualSets(test1, test2));

	test1.insert(4);
	EXPECT_FALSE(it.EqualSets(test1, test2));
}

TEST(IterableTools, DeleteFromSet)
{
	IterableTools it;
	std::set<int> deletion = std::set<int>({ 1 });
	std::set<int> original = std::set<int>({ 1,2,3,4 });

	EXPECT_THAT(it.DeleteFromSet(original,deletion), ::testing::ElementsAre(2,3,4));
	
	deletion.insert(2);
	EXPECT_THAT(it.DeleteFromSet(original, deletion), ::testing::ElementsAre(3, 4));

	deletion.insert(3);
	deletion.insert(4);
	deletion.insert(6);
	EXPECT_THAT(it.DeleteFromSet(original, deletion), ::testing::IsEmpty());
}

TEST(IterableTools, DeleteValueFromSet)
{
	IterableTools it;
	int deletion = 1;
	std::set<int> original = std::set<int>({ 1,2,3,4 });

	EXPECT_THAT(it.DeleteFromSet(original, deletion), ::testing::ElementsAre(2, 3, 4));

	deletion = 6;
	EXPECT_THAT(it.DeleteFromSet(original, deletion), ::testing::ElementsAre(1, 2, 3, 4));
}

TEST(IterableTools, CombineMapsEnsureNoOverride)
{
	IterableTools it;
	std::map<std::string, int> original_map = { {"hi", 1}, {"mip", 2}, {"do", 3} };
	std::map<std::string, int> update_map = { {"hi", 500}, {"jack", 4333}, {"do", 7} };
	std::map<std::string, int> res = it.CombineMaps(original_map, update_map);

	EXPECT_TRUE(it.GetKeys(res).size() == 4);
	EXPECT_EQ(res["hi"], 1);
	EXPECT_EQ(res["mip"], 2);
	EXPECT_EQ(res["do"], 3);
	EXPECT_EQ(res["jack"], 4333);
}

TEST(IterableTools, CombineMapsUMEnsureNoOverride)
{
	IterableTools it;
	std::unordered_map<std::string, int> original_map = { {"hi", 1}, {"mip", 2}, {"do", 3} };
	std::unordered_map<std::string, int> update_map = { {"hi", 500}, {"jack", 4333}, {"do", 7} };
	std::unordered_map<std::string, int> res = it.CombineMaps(original_map, update_map);

	EXPECT_TRUE(it.GetKeys(res).size() == 4);
	EXPECT_EQ(res["hi"], 1);
	EXPECT_EQ(res["mip"], 2);
	EXPECT_EQ(res["do"], 3);
	EXPECT_EQ(res["jack"], 4333);
}

TEST(IterableTools, CombineMapsEmptyUpdateMap)
{
	IterableTools it;
	std::map<std::string, int> original_map = { {"hi", 1}, {"mip", 2}, {"do", 3} };
	std::map<std::string, int> update_map;
	std::map<std::string, int> res = it.CombineMaps(original_map, update_map);

	EXPECT_TRUE(it.GetKeys(res).size() == 3);
	EXPECT_EQ(res["hi"], 1);
	EXPECT_EQ(res["mip"], 2);
	EXPECT_EQ(res["do"], 3);
}

TEST(IterableTools, CombineMapsUMEmptyUpdateMap)
{
	IterableTools it;
	std::unordered_map<std::string, int> original_map = { {"hi", 1}, {"mip", 2}, {"do", 3} };
	std::unordered_map<std::string, int> update_map;
	std::unordered_map<std::string, int> res = it.CombineMaps(original_map, update_map);

	EXPECT_TRUE(it.GetKeys(res).size() == 3);
	EXPECT_EQ(res["hi"], 1);
	EXPECT_EQ(res["mip"], 2);
	EXPECT_EQ(res["do"], 3);
}

TEST(IterableTools, CombineMapsEmptyOriginalMap)
{
	IterableTools it;
	std::unordered_map<std::string, int> original_map;
	std::unordered_map<std::string, int> update_map = { {"hi", 1}, {"mip", 2}, {"do", 3} };
	std::unordered_map<std::string, int> res = it.CombineMaps(original_map, update_map);

	EXPECT_TRUE(it.GetKeys(res).size() == 3);
	EXPECT_EQ(res["hi"], 1);
	EXPECT_EQ(res["mip"], 2);
	EXPECT_EQ(res["do"], 3);
}

TEST(IterableTools, CombineMapsUMEmptyOriginalMap)
{
	IterableTools it;
	std::unordered_map<std::string, int> original_map;
	std::unordered_map<std::string, int> update_map = { {"hi", 1}, {"mip", 2}, {"do", 3} };
	std::unordered_map<std::string, int> res = it.CombineMaps(original_map, update_map);

	EXPECT_TRUE(it.GetKeys(res).size() == 3);
	EXPECT_EQ(res["hi"], 1);
	EXPECT_EQ(res["mip"], 2);
	EXPECT_EQ(res["do"], 3);
}

TEST(IterableTools, ReverseMapSet)
{
	IterableTools it;
	std::map<std::string, std::set<int>> original_map = { {"hi", std::set<int>({1,2,3})}, {"mip", std::set<int>({2,3,4})}, {"do", std::set<int>({3,4,5})} };
	std::map<int, std::set<std::string>> res = it.ReverseMapSet(original_map);

	
	EXPECT_TRUE(it.GetKeys(res).size() == 5);
	EXPECT_THAT(res[1], ::testing::ElementsAre("hi"));
	EXPECT_THAT(res[2], ::testing::UnorderedElementsAre("hi","mip"));
	EXPECT_THAT(res[3], ::testing::UnorderedElementsAre("hi","mip","do"));
	EXPECT_THAT(res[4], ::testing::UnorderedElementsAre("mip","do"));
	EXPECT_THAT(res[5], ::testing::UnorderedElementsAre("do"));
}

TEST(IterableTools, ReverseMapSetUM)
{
	IterableTools it;
	std::unordered_map<std::string, std::set<int>> original_map = { {"hi", std::set<int>({1,2,3})}, {"mip", std::set<int>({2,3,4})}, {"do", std::set<int>({3,4,5})} };
	std::unordered_map<int, std::set<std::string>> res = it.ReverseMapSet(original_map);


	EXPECT_TRUE(it.GetKeys(res).size() == 5);
	EXPECT_THAT(res[1], ::testing::ElementsAre("hi"));
	EXPECT_THAT(res[2], ::testing::UnorderedElementsAre("hi", "mip"));
	EXPECT_THAT(res[3], ::testing::UnorderedElementsAre("hi", "mip", "do"));
	EXPECT_THAT(res[4], ::testing::UnorderedElementsAre("mip", "do"));
	EXPECT_THAT(res[5], ::testing::UnorderedElementsAre("do"));
}

//
// IndexOf
//

TEST(IterableTools, IndexOfVectorInt)
{
	IterableTools it;
	std::vector<int> test = { 10, 23, 24, 1032 };
	int search = 24;
	EXPECT_EQ(it.IndexOf(test, search), 2);
	search = 22;
	EXPECT_EQ(it.IndexOf(test, search), std::string::npos);
}

TEST(IterableTools, IndexOfVectorStr)
{
	IterableTools it;
	std::vector<std::string> test = { "alpha", "beta", "uniform", "violet" };
	std::string search = "beta";
	EXPECT_EQ(it.IndexOf(test, search), 1);
	search = "none";
	EXPECT_EQ(it.IndexOf(test, search), std::string::npos);
}

TEST(IterableTools, IndexOfSetInt)
{
	IterableTools it;
	std::set<int> test = { 10, 23, 24, 1032 };
	int search = 24;
	EXPECT_EQ(it.IndexOf(test, search), 2);
	search = 22;
	EXPECT_EQ(it.IndexOf(test, search), std::string::npos);
}

TEST(IterableTools, IndexOfSetStr)
{
	IterableTools it;
	std::set<std::string> test = { "alpha", "beta", "uniform", "violet" };
	std::string search = "beta";
	EXPECT_EQ(it.IndexOf(test, search), 1);
	search = "none";
	EXPECT_EQ(it.IndexOf(test, search), std::string::npos);
}

TEST(UserInputOutput, PrintMapWithHeader_KeyToSetTwoColumns)
{
	IterableTools iterable_tools_;
	std::map<uint64_t, std::set<uint64_t>> map_to_print;
	map_to_print[1] = std::set<uint64_t>({ 4,5,6 });
	map_to_print[2] = std::set<uint64_t>({}); // ensure no comma at the end
	map_to_print[3] = std::set<uint64_t>({ 7,8 }); // same size ensure print
	map_to_print[4] = std::set<uint64_t>({ 9,10 }); //same size ensure print
	map_to_print[5] = std::set<uint64_t>({ 10,11,12,13 }); // ensure this biggest set is printed first

	std::vector<std::string> columns;
	columns.push_back("column1");
	columns.push_back("column2");

	std::string output = iterable_tools_.PrintMapWithHeader_KeyToSet(map_to_print, columns, "map name");

	std::string compare_output = "\n  map name\n (column1) | (column2) \n" + iterable_tools_.GetPrintBar() + "\n 5:\t10,11,12,13\n 1:\t4,5,6\n 4:\t9,10\n 3:\t7,8\n 2:\t\n" + iterable_tools_.GetPrintBar() + "\n\n";

	EXPECT_EQ(compare_output, output);
}

TEST(UserInputOutput, PrintMapWithHeader_KeyToSetOneColumn)
{
	IterableTools iterable_tools_;
	std::map<uint64_t, std::set<uint64_t>> map_to_print;
	map_to_print[1] = std::set<uint64_t>({}); 


	std::vector<std::string> columns;
	columns.push_back("column1");

	std::string output = iterable_tools_.PrintMapWithHeader_KeyToSet(map_to_print, columns, "map name");

	std::string compare_output = "\n  map name\n (column1) \n" + iterable_tools_.GetPrintBar() + "\n 1:\t\n" + iterable_tools_.GetPrintBar() + "\n\n";

	EXPECT_EQ(compare_output, output);
}

TEST(UserInputOutput, PrintMapWithHeader_KeyToValue)
{
	IterableTools iterable_tools_;
	std::map<uint64_t, uint64_t> map_to_print;
	map_to_print[1] = 4;
	map_to_print[2] = 5;
	map_to_print[3] = 6;
	map_to_print[4] = 7;

	std::vector<std::string> columns;
	columns.push_back("column1");
	columns.push_back("column2");

	std::string output = iterable_tools_.PrintMapWithHeader_KeyToValue(map_to_print, columns, "map name");

	std::string compare_output = "\n  map name\n (column1) | (column2) \n" + iterable_tools_.GetPrintBar() + "\n 1:\t4\n 2:\t5\n 3:\t6\n 4:\t7\n" + iterable_tools_.GetPrintBar() + "\n\n";

	EXPECT_EQ(compare_output, output);
}

// Test fixture and NullEventHandler class stolen from yaml-cpp tests:
// https://github.com/jbeder/yaml-cpp/blob/master/test/integration/emitter_test.cpp

class NullEventHandler : public YAML::EventHandler {
	virtual void OnDocumentStart(const YAML::Mark&) {}
	virtual void OnDocumentEnd() {}

	virtual void OnNull(const YAML::Mark&, YAML::anchor_t) {}
	virtual void OnAlias(const YAML::Mark&, YAML::anchor_t) {}
	virtual void OnScalar(const YAML::Mark&, const std::string&, YAML::anchor_t,
		const std::string&) {}

	virtual void OnSequenceStart(const YAML::Mark&, const std::string&, YAML::anchor_t,
		YAML::EmitterStyle::value /* style */) {}
	virtual void OnSequenceEnd() {}

	virtual void OnMapStart(const YAML::Mark&, const std::string&, YAML::anchor_t,
		YAML::EmitterStyle::value /* style */) {}
	virtual void OnMapEnd() {}
};

class EmitterTest : public ::testing::Test {
protected:

	IterableTools it_;
	YAML::Emitter out_;

	void ExpectEmit(const std::string& expected, YAML::Emitter& out) {
		EXPECT_EQ(expected, out.c_str());
		EXPECT_TRUE(out.good()) << "Emitter raised: " << out.GetLastError();
		if (expected == out.c_str()) {
			std::stringstream stream(expected);
			YAML::Parser parser;
			NullEventHandler handler;
			parser.HandleNextDocument(handler);
		}
	}
	
};

TEST_F(EmitterTest, EmitKeyValuePairStrings)
{
	std::string key = "the_key_is";
	std::string val = "GitLab Project Management Demo";
	it_.EmitKeyValuePair(out_, key, val);

	std::string expect = "the_key_is: GitLab Project Management Demo\n";
	ExpectEmit(expect, out_);
}

TEST_F(EmitterTest, EmitKeyValuePairStringInt)
{
	std::string key = "integer for use";
	int val = 1232;
	it_.EmitKeyValuePair(out_, key, val);

	std::string expect = "integer for use: 1232\n";
	ExpectEmit(expect, out_);
}

TEST_F(EmitterTest, EmitKeyValuePairIntString)
{
	std::string val = "integer for use";
	int key = 1232;
	it_.EmitKeyValuePair(out_, key, val);

	std::string expect = "1232: integer for use\n";
	ExpectEmit(expect, out_);
}

TEST_F(EmitterTest, EmitKeyValuePairIntInt)
{
	uint16_t key = 80;
	int val = -32;
	it_.EmitKeyValuePair(out_, key, val);

	std::string expect = "80: -32\n";
	ExpectEmit(expect, out_);
}

TEST_F(EmitterTest, EmitSimpleMapStrings)
{
	std::map<std::string, std::string> test_map =
		{ {"1", "big"}, {"22", "ten"}, {"fifty", "proton"} };
	it_.EmitSimpleMap(out_, test_map, "my_map");

	std::string expect = "my_map:\n  1: big\n  22: ten\n  fifty: proton\n";
	ExpectEmit(expect, out_);
}

TEST_F(EmitterTest, EmitSimpleMapIntString)
{
	std::map<int, std::string> test_map =
	{ {1, "big"}, {22, "ten"}, {-50, "proton"} };
	it_.EmitSimpleMap(out_, test_map, "my map");

	// Note the order of expected. Maps sort based on key type, apparently
	// in ascending order for int.
	std::string expect = "my map:\n  -50: proton\n  1: big\n  22: ten\n";
	ExpectEmit(expect, out_);
}

TEST_F(EmitterTest, EmitSimpleMapIntInt)
{
	std::map<int, uint16_t> test_map =
	{ {1, 55}, {22, 10}, {-50, 1380} };
	it_.EmitSimpleMap(out_, test_map, "my map");

	// Note the order of expected. Maps sort based on key type, apparently
	// in ascending order for int.
	std::string expect = "my map:\n  -50: 1380\n  1: 55\n  22: 10\n";
	ExpectEmit(expect, out_);
}

// Handle both EmitCompoundMapToVector and EmitCompountMapToSet
TEST_F(EmitterTest, EmitCompoundMapStringString)
{
	// Ensure that vector/set of strings are written in alphabetical
	// order so we can use the same expect string for both the vector
	// test, which will keep the order, and the set test, which will
	// place the values in alphabetical order.
	std::map<std::string, std::vector<std::string>> test_map =
		{ 
			{"1", {"big", "daddy"}}, 
			{"22", {"hi", "speed", "ten"}}, 
			{"fifty", {"bunch", "map", "proton"}} 
		};
	it_.EmitCompoundMapToVector(out_, test_map, "my_map");

	std::string expect = "my_map:\n  1: [big, daddy]\n  22: [hi, speed, ten]\n  "
		"fifty: [bunch, map, proton]\n";
	ExpectEmit(expect, out_);

	
	// Create new emitter for second map.
	YAML::Emitter e;
	std::map<std::string, std::set<std::string>> test_map2;
	for (std::map<std::string, std::vector<std::string>>::const_iterator it = test_map.begin();
		it != test_map.end(); ++it)
	{
		std::set<std::string> add_set(it->second.begin(), it->second.end());
		test_map2[it->first] = add_set;
	}
	it_.EmitCompoundMapToSet(e, test_map2, "my_map");
	ExpectEmit(expect, e);
	
}

// Handle both EmitCompoundMapToVector and EmitCompoundMapToSet
TEST_F(EmitterTest, EmitCompoundMapIntInt)
{
	// Ensure that vector/set of strings are written in alphabetical
	// order so we can use the same expect string for both the vector
	// test, which will keep the order, and the set test, which will
	// place the values in alphabetical order.
	std::map<int, std::vector<int64_t>> test_map =
	{
		{-33, {3, 4, 5}},
		{22, {-6, -2, 43}},
		{634, {-1023, 1023, 5000}}
	};
	it_.EmitCompoundMapToVector(out_, test_map, "the other map");

	std::string expect = "the other map:\n  -33: [3, 4, 5]\n  22: [-6, -2, 43]\n  "
		"634: [-1023, 1023, 5000]\n";
	ExpectEmit(expect, out_);


	// Create new emitter for second map.
	YAML::Emitter e;
	std::map<int, std::set<int64_t>> test_map2;
	for (std::map<int, std::vector<int64_t>>::const_iterator it = test_map.begin();
		it != test_map.end(); ++it)
	{
		std::set<int64_t> add_set(it->second.begin(), it->second.end());
		test_map2[it->first] = add_set;
	}
	it_.EmitCompoundMapToSet(e, test_map2, "the other map");
	ExpectEmit(expect, e);

}

TEST(IterableTools, CombineCompoundMapsToSetEmptyUpdateMap)
{
	IterableTools it;
	using maptype = std::map<std::string, std::set<int>>;
	maptype input_map = {
		{"hi", {3, 6, 88}},
		{"test", {3, 55, 78, 88}}
	};
	maptype update_map;
	maptype result_map = it.CombineCompoundMapsToSet(input_map, update_map);
	EXPECT_THAT(result_map["hi"], ::testing::ElementsAreArray(input_map["hi"]));
	EXPECT_THAT(result_map["test"], ::testing::ElementsAreArray(input_map["test"]));
}

TEST(IterableTools, CombineCompoundMapsToSetEmptyInputMap)
{
	IterableTools it;
	using maptype = std::map<std::string, std::set<int>>;
	maptype update_map = {
		{"hi", {3, 6, 88}},
		{"test", {3, 55, 78, 88}}
	};
	maptype input_map;
	maptype result_map = it.CombineCompoundMapsToSet(input_map, update_map);
	EXPECT_THAT(result_map["hi"], ::testing::ElementsAreArray(update_map["hi"]));
	EXPECT_THAT(result_map["test"], ::testing::ElementsAreArray(update_map["test"]));
}

TEST(IterableTools, CombineCompoundMapsToSetMergeSets)
{
	IterableTools it;
	using maptype = std::map<std::string, std::set<int>>;
	maptype input_map = {
		{"hi", {3, 6, 88}},
		{"test", {3, 55, 78, 88}}
	};
	maptype update_map = {
		{"hi", {3, 23, 88}}
	};
	maptype expected_map = {
		{"hi", {3, 6, 23, 88}},
		{"test", {3, 55, 78, 88}}
	};
	maptype result_map = it.CombineCompoundMapsToSet(input_map, update_map);
	EXPECT_THAT(result_map["hi"], ::testing::ElementsAreArray(expected_map["hi"]));
	EXPECT_THAT(result_map["test"], ::testing::ElementsAreArray(expected_map["test"]));
}

TEST(IterableTools, CombineCompoundMapsToSetNewKeyValPair)
{
	IterableTools it;
	using maptype = std::map<std::string, std::set<int>>;
	maptype input_map = {
		{"hi", {3, 6, 88}},
		{"test", {3, 55, 78, 88}}
	};
	maptype update_map = {
		{"hi", {3, 23, 88}},
		{"dude", {12, 44}}
	};
	maptype expected_map = {
		{"hi", {3, 6, 23, 88}},
		{"test", {3, 55, 78, 88}},
		{"dude", {12, 44}}
	};
	maptype result_map = it.CombineCompoundMapsToSet(input_map, update_map);
	EXPECT_THAT(result_map["hi"], ::testing::ElementsAreArray(expected_map["hi"]));
	EXPECT_THAT(result_map["test"], ::testing::ElementsAreArray(expected_map["test"]));
	EXPECT_THAT(result_map["dude"], ::testing::ElementsAreArray(expected_map["dude"]));
}