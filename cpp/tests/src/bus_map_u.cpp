#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "bus_map.h"
#include "file_reader.h"
#include "icd_data.h"


class BusMapTest : public ::testing::Test
{
protected:
	
	std::unordered_map<uint64_t, std::set<std::string>> icd_message_key_to_busnames_map;
	std::unordered_map<uint64_t, std::set<uint64_t>> icd_message_key_to_channelids_map;
	
	std::map<uint64_t, std::string> tmats_chanid_to_source_map;
	

	BusMap b;
	std::map<std::string, std::set<uint64_t>> comet_compare_map;
	std::vector<std::string> term_mux_lines;
	std::set<uint64_t> temp_set;
	std::map<uint64_t, std::set<uint64_t>> ch10_scanned_compare_map;
	
	std::map<uint64_t, std::string> chID_to_busname_compare_map;
	std::map<std::string, std::set<uint64_t>> bus_name_to_lru_addresses_comet_map;
	std::map<uint64_t, std::string> tmats_chanid_to_type_map;
	std::map<uint64_t, std::string> tmats_1553_chanid_compare_map;
	IterableTools iterable_tools_;
	FileReader fr;

	// inputs
	std::map<uint64_t, std::set<uint64_t>> ch10_scanned_chanid_to_lruaddrs_map;

	BusMapTest()
	{
		
	}
	void SetUp() override
	{
	}

	template <typename Map>
	bool map_compare(Map const& lhs, Map const& rhs) 
	{
		return lhs.size() == rhs.size()
			&& std::equal(lhs.begin(), lhs.end(),
				rhs.begin());
	}
	
};

/*
TEST_F(BusMapTest, TryOutUserInterface)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 3,7,10 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // unique LRU
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 3,7,8 }); // unique subset
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 3,8,10,11 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[4] = std::set<uint64_t>({ 3,8,10 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[5] = std::set<uint64_t>({ 3,7 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[6] = std::set<uint64_t>({ 7 }); // trailing subset
	ch10_scanned_chanid_to_lruaddrs_map[7] = std::set<uint64_t>({ 20 }); // none
	ch10_scanned_chanid_to_lruaddrs_map[8] = std::set<uint64_t>({ 20,23 }); // none

	// Build Input Maps
	tmats_chanid_to_source_map[3] = "BUS2"; // should override scanned channel id 3 to BUS1 mapping



	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
	ch10_scanned_chanid_to_lruaddrs_map, 
	tmats_chanid_to_source_map);

	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res, 2, true);
	if (continue_translation)
		printf("returned true\n\n");
	else
		printf("returned false\n\n");
	system("pause");
}*/


TEST_F(BusMapTest, SubsetMappingMatchingSubset)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 5,6,7,8,9 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 6,7,8 });

	// comparison map
	chID_to_busname_compare_map[1] = "BUS1";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("SubsetMapping", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, SubsetMappingMatchingIdenticalSets)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 5,6,7,8,9 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 5,6,7,8,9 });

	// comparison map
	chID_to_busname_compare_map[1] = "BUS1";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("SubsetMapping",
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, SubsetMappingTestLRULengthOrdering1)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 5,6,7,8,9 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 5,6,7,8 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 5,6,7 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 5,6 });
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 5,6,7 });
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 5,6,7,8 });

	// comparison map
	chID_to_busname_compare_map[1] = "BUS1";
	chID_to_busname_compare_map[2] = "BUS2";
	chID_to_busname_compare_map[3] = "BUS3";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("SubsetMapping", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, SubsetMappingTestLRULengthOrdering2)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 5,6,7 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 5,6,7,8 });
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 5,6,7,8,9 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 5,6,7,8 });
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 5,6,7 });
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 5,6 });

	// comparison map
	chID_to_busname_compare_map[1] = "BUS1";
	chID_to_busname_compare_map[2] = "BUS2";
	chID_to_busname_compare_map[3] = "BUS3";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("SubsetMapping", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, SubsetMappingTestLRULengthOrdering3)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 3,7,10 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // Shouldn't be matched
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 3,7,10 }); 
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 3,8,10,11 }); 

	// comparison map
	chID_to_busname_compare_map[2] = "BUS3";
	chID_to_busname_compare_map[3] = "BUS1";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("SubsetMapping", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, SubsetMappingTestDuplicateScannedNotMatchedTwice)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 5,6,7,8,9 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 10,11,12 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 5,6,7,8 });
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 5,6,7 });
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 10,11,12 });

	// comparison map
	chID_to_busname_compare_map[1] = "BUS1";
	chID_to_busname_compare_map[3] = "BUS3";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("SubsetMapping", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, SubsetMappingTestDuplicateCometSkipped)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 5,6,7,8,9,10 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 5,6,7,8,9 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 10,11,12 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 5,6,7,8 });
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 10,11,12 });

	// comparison map
	chID_to_busname_compare_map[1] = "BUS1";
	chID_to_busname_compare_map[3] = "BUS3";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("SubsetMapping", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, SubsetMappingTestDanglingCometMappingSkipped)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 5,6,7,8,9,10 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 5,6,7,8,9 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 10,11,12 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 5,6,7,8 });
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 5,6,7 });

	// comparison map
	chID_to_busname_compare_map[1] = "BUS1";
	chID_to_busname_compare_map[2] = "BUS2";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("SubsetMapping", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, SubsetMappingTestDanglingScannedMappingSkipped)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 5,6,7,8,9,10 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 5,6,7,8,9 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 5,6,7,8 });
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 5,6,7 });
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 10,11 });

	// comparison map
	chID_to_busname_compare_map[1] = "BUS1";
	chID_to_busname_compare_map[2] = "BUS2";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("SubsetMapping", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, SubsetMappingSkips)
{
	// build skipped map
	std::map<uint64_t, std::string> skip;
	skip[2] = "BUS2";
	skip[4] = "BUS4";
	skip[10] = "BUS10"; // Non existant channel id
	skip[5] = "BUS4"; // Duplicate bus name


	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 5,6,7,8,9,10 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 5,6,7,8,9 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 10,11,12 });
	bus_name_to_lru_addresses_comet_map["BUS4"] = std::set<uint64_t>({ 10,11 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 5,6,7,8 });
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 5,6,7 });
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 10,11,12 });
	ch10_scanned_chanid_to_lruaddrs_map[4] = std::set<uint64_t>({ 10,11 });

	// comparison map
	chID_to_busname_compare_map[1] = "BUS1";
	chID_to_busname_compare_map[3] = "BUS3";

	std::set<std::uint64_t> scanned_skipped = iterable_tools_.VecToSet(iterable_tools_.GetKeys(skip));
	std::set<std::string> comet_skipped = iterable_tools_.VecToSet(iterable_tools_.GetVals(skip));

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("SubsetMapping", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map, 
			comet_skipped, scanned_skipped);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}


TEST_F(BusMapTest, TrailingSubsetMappingNoOriginalSkips)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 5,6,7,8,9,10 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 5,6,7,8,9 });


	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 5,6,7,8,9,10 });
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 5,6,7,8,9,10 });
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 5,6,7 });

	// comparison map
	chID_to_busname_compare_map[1] = "BUS1";
	chID_to_busname_compare_map[2] = "BUS1";
	chID_to_busname_compare_map[3] = "BUS3";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("TrailingSubsetMapping", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, TrailingSubsetMappingWithOriginalSkips)
{
	// build skipped map
	std::map<uint64_t, std::string> skip;
	skip[4] = "BUS4";
	skip[5] = "BUS5";

	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 5,6,7,8,9,10 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 5,6,7,8,9 });
	bus_name_to_lru_addresses_comet_map["BUS4"] = std::set<uint64_t>({ 15,16,17 });
	bus_name_to_lru_addresses_comet_map["BUS5"] = std::set<uint64_t>({ 18,19,20 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 5,6,7,8,9,10 });
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 5,6,7,8,9,10 });
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 5,6,7 });
	ch10_scanned_chanid_to_lruaddrs_map[4] = std::set<uint64_t>({ 15,16,17 });
	ch10_scanned_chanid_to_lruaddrs_map[5] = std::set<uint64_t>({ 18,19,20 });

	// comparison map
	chID_to_busname_compare_map[1] = "BUS1";
	chID_to_busname_compare_map[2] = "BUS1";
	chID_to_busname_compare_map[3] = "BUS3";

	std::set<std::uint64_t> scanned_skipped = iterable_tools_.VecToSet(iterable_tools_.GetKeys(skip));
	std::set<std::string> comet_skipped = iterable_tools_.VecToSet(iterable_tools_.GetVals(skip));

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("TrailingSubsetMapping", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map, 
			comet_skipped, scanned_skipped);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, UniqueLRUIdentificationNonUniqueExactMatches)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 7,8,9,10 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 7,8,9,10 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 7,8,9,10 });
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 7,8,9,10 });

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("UniqueLRUIdentification", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, UniqueLRUIdentificationNonUniqueSubset)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 7,8,9 });			
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 7,8 });			

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 7,8 });					

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("UniqueLRUIdentification", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, UniqueLRUIdentificationMultipleUniqueOnSameBus)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 7,8,9,10 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 7,8 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 7,8,10 });
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 7,8,9 });
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 7,8 });

	// build expected return map
	chID_to_busname_compare_map[1] = "BUS1";
	chID_to_busname_compare_map[2] = "BUS1";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("UniqueLRUIdentification", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, UniqueLRUIdentificationNonUniqueSuperset)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 7,8,9 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 7,8 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 7,8,10,11 });					

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("UniqueLRUIdentification", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, UniqueLRUIdentificationOneUniqueLRUPresent)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 7,8,9,10,11 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 11 });

	// build expected return map
	chID_to_busname_compare_map[1] = "BUS1";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("UniqueLRUIdentification", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, UniqueLRUIdentificationUniqueSubset)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 7,8,9,15,10,11 });// Unique (15)


	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 7,8,15,10,11 });// Contains 15 (should match)

	// build expected return map
	chID_to_busname_compare_map[1] = "BUS1";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("UniqueLRUIdentification", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, UniqueLRUIdentificationUniqueSuperset)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 7,8,9,10,11,20 });// Unique (20)

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 7,8,9,10,11,20,50,60 });// Unique, with extra LRU (should match)

	// build expected return map
	chID_to_busname_compare_map[1] = "BUS1";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("UniqueLRUIdentification", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, UniqueSubsetMappingOneUnique)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 3,7 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 3,7,8 }); // unique
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 3,8 }); // not unique
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 3,7 }); // not unique

	// comparison map
	chID_to_busname_compare_map[1] = "BUS1";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("UniqueSubsetMapping", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, UniqueSubsetMappingMultipleUniqueSameBus)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 3,7 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 3,7,8 }); // unique
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 3,8,9 }); // unique to the same bus (BUS1)
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 3,7 }); // not unique

	// comparison map
	chID_to_busname_compare_map[1] = "BUS1";
	chID_to_busname_compare_map[2] = "BUS1";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("UniqueSubsetMapping", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, UniqueSubsetMappingMultipleUniqueDifferentBus)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8,11,12 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 3,7 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 3,7,8 }); // unique
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 3,7 }); // not unique
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 3,8,11 }); // unique

	// comparison map
	chID_to_busname_compare_map[1] = "BUS1";
	chID_to_busname_compare_map[3] = "BUS2";

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("UniqueSubsetMapping", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, UniqueSubsetMappingNoUnique)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,7,9 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 3,7,10 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 3,7 }); // not unique
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 3,7 }); // not unique

	std::map<uint64_t, std::string> returned_bus_mapping = 
		b.ReturnMapsForTesting("UniqueSubsetMapping", 
			ch10_scanned_chanid_to_lruaddrs_map, 
			bus_name_to_lru_addresses_comet_map);
	EXPECT_TRUE(map_compare(returned_bus_mapping, chID_to_busname_compare_map));
}

TEST_F(BusMapTest, PerformBusMappingConfidenceLevel1)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 3,7,10 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // unique LRU
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 3,7,8 }); // unique subset
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 3,8,10,11 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[4] = std::set<uint64_t>({ 3,8,10 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[5] = std::set<uint64_t>({ 3,7 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[6] = std::set<uint64_t>({ 7 }); // trailing subset
	ch10_scanned_chanid_to_lruaddrs_map[7] = std::set<uint64_t>({ 20 }); // none
	ch10_scanned_chanid_to_lruaddrs_map[8] = std::set<uint64_t>({ 20,23 }); // none

	// Build Input Maps
	tmats_chanid_to_source_map[1] = "BUS3"; // remap of a unique lru (should not remap)
	tmats_chanid_to_source_map[2] = "BUS3"; // remap of a unique subset (should not remap)
	tmats_chanid_to_source_map[3] = "BUS2"; // valid tmats mapping for confidence level 2 (should override scanned channel id 3 to BUS1 mapping)



	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res,1, false);
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 2);
	EXPECT_TRUE(continue_translation);
	EXPECT_EQ(res[1], "BUS1");
	EXPECT_EQ(res[2], "BUS1");	

	// Also check the source
	std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map_with_source = 
		b.GetFinalBusMap_withSource();
	EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map_with_source).size() == 2);
	EXPECT_EQ(final_bus_map_with_source[1].first, "BUS1");
	EXPECT_EQ(final_bus_map_with_source[1].second, "UniqueLRU");

	EXPECT_EQ(final_bus_map_with_source[2].first, "BUS1");
	EXPECT_EQ(final_bus_map_with_source[2].second, "UniqueSubset");
}

TEST_F(BusMapTest, PerformBusMappingConfidenceLevel2)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 3,7,10 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // unique LRU
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 3,7,8 }); // unique subset
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 3,8,10,11 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[4] = std::set<uint64_t>({ 3,8,10 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[5] = std::set<uint64_t>({ 3,7 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[6] = std::set<uint64_t>({ 7 }); // trailing subset

	// Build Input Maps
	tmats_chanid_to_source_map[1] = "BUS3"; // remap of a unique lru (should not remap)
	tmats_chanid_to_source_map[2] = "BUS3"; // remap of a unique subset (should not remap)
	tmats_chanid_to_source_map[3] = "BUS2"; // valid tmats mapping for confidence level 2 (should override scanned channel id 3 to BUS1 mapping)


	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res, 2, false);
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 3);
	EXPECT_TRUE(continue_translation);
	EXPECT_EQ(res[1], "BUS1");
	EXPECT_EQ(res[2], "BUS1");
	EXPECT_EQ(res[3], "BUS2");

	// Also check the source
	std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map_with_source = 
		b.GetFinalBusMap_withSource();

	EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map_with_source).size() == 3);
	EXPECT_EQ(final_bus_map_with_source[1].first, "BUS1");
	EXPECT_EQ(final_bus_map_with_source[1].second, "UniqueLRU");

	EXPECT_EQ(final_bus_map_with_source[2].first, "BUS1");
	EXPECT_EQ(final_bus_map_with_source[2].second, "UniqueSubset");

	EXPECT_EQ(final_bus_map_with_source[3].first, "BUS2");
	EXPECT_EQ(final_bus_map_with_source[3].second, "TMATS");
}

TEST_F(BusMapTest, PerformBusMappingConfidenceLevel3)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 3,7,10 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // unique LRU
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 3,7,8 }); // unique subset
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 3,8,10,11 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[4] = std::set<uint64_t>({ 3,8,10 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[5] = std::set<uint64_t>({ 3,7 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[6] = std::set<uint64_t>({ 7 }); // trailing subset

	// Build Input Maps
	tmats_chanid_to_source_map[1] = "BUS3"; // remap of a unique lru (should not remap)
	tmats_chanid_to_source_map[2] = "BUS3"; // remap of a unique subset (should not remap)
	tmats_chanid_to_source_map[3] = "BUS2"; // valid tmats mapping for confidence level 2 (should override scanned channel id 3 to BUS1 mapping)


	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res, 3, false);
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 5);
	EXPECT_TRUE(continue_translation);
	EXPECT_EQ(res[1], "BUS1");
	EXPECT_EQ(res[2], "BUS1");
	EXPECT_EQ(res[3], "BUS2");
	EXPECT_EQ(res[4], "BUS2");
	EXPECT_EQ(res[5], "BUS3");

	// Also check the source
	std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map_with_source = 
		b.GetFinalBusMap_withSource();

	EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map_with_source).size() == 5);
	EXPECT_EQ(final_bus_map_with_source[1].first, "BUS1");
	EXPECT_EQ(final_bus_map_with_source[1].second, "UniqueLRU");

	EXPECT_EQ(final_bus_map_with_source[2].first, "BUS1");
	EXPECT_EQ(final_bus_map_with_source[2].second, "UniqueSubset");

	EXPECT_EQ(final_bus_map_with_source[3].first, "BUS2");
	EXPECT_EQ(final_bus_map_with_source[3].second, "TMATS");

	EXPECT_EQ(final_bus_map_with_source[4].first, "BUS2");
	EXPECT_EQ(final_bus_map_with_source[4].second, "Subset");

	EXPECT_EQ(final_bus_map_with_source[5].first, "BUS3");
	EXPECT_EQ(final_bus_map_with_source[5].second, "Subset");
}

TEST_F(BusMapTest, PerformBusMappingConfidenceLevel4)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 3,7,10 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // unique LRU
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 3,7,8 }); // unique subset
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 3,8,10,11 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[4] = std::set<uint64_t>({ 3,8,10 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[5] = std::set<uint64_t>({ 3,7 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[6] = std::set<uint64_t>({ 7 }); // trailing subset (along with ids 1 and 2)

	// Build Input Maps
	tmats_chanid_to_source_map[1] = "BUS3"; // remap of a unique lru (should not remap)
	tmats_chanid_to_source_map[2] = "BUS3"; // remap of a unique subset (should not remap)
	tmats_chanid_to_source_map[3] = "BUS2"; // valid tmats mapping for confidence level 2 (should override scanned channel id 3 to BUS1 mapping)


	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res, 4, false);
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 6);
	EXPECT_TRUE(continue_translation);
	EXPECT_EQ(res[1], "BUS1");
	EXPECT_EQ(res[2], "BUS1");
	EXPECT_EQ(res[3], "BUS2");
	EXPECT_EQ(res[4], "BUS2");
	EXPECT_EQ(res[5], "BUS3");
	EXPECT_EQ(res[6], "BUS3");

	// Also check the source
	std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map_with_source = 
		b.GetFinalBusMap_withSource();

	EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map_with_source).size() == 6);
	EXPECT_EQ(final_bus_map_with_source[1].first, "BUS1");
	EXPECT_EQ(final_bus_map_with_source[1].second, "UniqueLRU");

	EXPECT_EQ(final_bus_map_with_source[2].first, "BUS1");
	EXPECT_EQ(final_bus_map_with_source[2].second, "UniqueSubset");

	EXPECT_EQ(final_bus_map_with_source[3].first, "BUS2");
	EXPECT_EQ(final_bus_map_with_source[3].second, "TMATS");

	EXPECT_EQ(final_bus_map_with_source[4].first, "BUS2");
	EXPECT_EQ(final_bus_map_with_source[4].second, "Subset");

	EXPECT_EQ(final_bus_map_with_source[5].first, "BUS3");
	EXPECT_EQ(final_bus_map_with_source[5].second, "Subset");

	EXPECT_EQ(final_bus_map_with_source[6].first, "BUS3");
	EXPECT_EQ(final_bus_map_with_source[6].second, "TrailingSubset");
}


TEST_F(BusMapTest, PerformBusMappingAddNonMappedSourceEntries)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 3,7,10 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // unique LRU
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 3,7,20 }); // non existing
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 30 }); // non existing

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, ch10_scanned_chanid_to_lruaddrs_map, tmats_chanid_to_source_map);
	std::map<uint64_t, std::string> res;
	b.PerformBusMapping(res, 4, false);

	std::map<uint64_t, std::pair<std::string, std::string>> bus_map_suggestions = b.GetBusSuggestionsMap();

	EXPECT_TRUE(iterable_tools_.GetKeys(bus_map_suggestions).size() == 3);
	EXPECT_EQ(bus_map_suggestions[1].first, "BUS1");
	EXPECT_EQ(bus_map_suggestions[1].second, "UniqueLRU");

	EXPECT_EQ(bus_map_suggestions[2].first, "NA");
	EXPECT_EQ(bus_map_suggestions[2].second, "NONE");

	EXPECT_EQ(bus_map_suggestions[3].first, "NA");
	EXPECT_EQ(bus_map_suggestions[3].second, "NONE");
}

TEST_F(BusMapTest, FillSuggestionsMap)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 3,7,10 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // unique LRU
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 3,7,8 }); // unique subset
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 3,8,10,11 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[4] = std::set<uint64_t>({ 3,8,10 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[5] = std::set<uint64_t>({ 3,7 }); // non unique subset
	ch10_scanned_chanid_to_lruaddrs_map[6] = std::set<uint64_t>({ 7 }); // trailing subset (along with ids 1 and 2)

	// Build Input Maps
	tmats_chanid_to_source_map[1] = "BUS3"; // remap of a unique lru (should not remap)
	tmats_chanid_to_source_map[2] = "BUS3"; // remap of a unique subset (should not remap)
	tmats_chanid_to_source_map[3] = "BUS2"; // valid tmats mapping for confidence level 2 (should override scanned channel id 3 to BUS1 mapping)


	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	std::map<uint64_t, std::string> res;
	b.PerformBusMapping(res, 1, false);

	std::map<uint64_t, std::pair<std::string, std::string>> bus_map_suggestions = 
		b.GetBusSuggestionsMap();

	EXPECT_TRUE(iterable_tools_.GetKeys(bus_map_suggestions).size() == 6);
	EXPECT_EQ(bus_map_suggestions[1].first, "BUS1");
	EXPECT_EQ(bus_map_suggestions[1].second, "UniqueLRU");

	EXPECT_EQ(bus_map_suggestions[2].first, "BUS1");
	EXPECT_EQ(bus_map_suggestions[2].second, "UniqueSubset");

	EXPECT_EQ(bus_map_suggestions[3].first, "BUS2");
	EXPECT_EQ(bus_map_suggestions[3].second, "TMATS");

	EXPECT_EQ(bus_map_suggestions[4].first, "BUS2");
	EXPECT_EQ(bus_map_suggestions[4].second, "Subset");

	EXPECT_EQ(bus_map_suggestions[5].first, "BUS3");
	EXPECT_EQ(bus_map_suggestions[5].second, "Subset");

	EXPECT_EQ(bus_map_suggestions[6].first, "BUS3");
	EXPECT_EQ(bus_map_suggestions[6].second, "TrailingSubset");
}

TEST_F(BusMapTest, PerformBusMappingClearExistingMap)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // unique LRU

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	std::map<uint64_t, std::string> res;
	res[10] = "junk";

	b.PerformBusMapping(res, 4, false);
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 1);
	EXPECT_EQ(res[1], "BUS1");
}


TEST_F(BusMapTest, PerformBusMappingInvalidConfidenceLevel)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // unique LRU

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	std::map<uint64_t, std::string> res;

	bool continue_translation = b.PerformBusMapping(res, 0, false); // lower out of bounds
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);
	EXPECT_FALSE(continue_translation);

	continue_translation = b.PerformBusMapping(res, 5, false); // upper out of bounds
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);
	EXPECT_FALSE(continue_translation);

	continue_translation = b.PerformBusMapping(res, 3, false);
	EXPECT_TRUE(continue_translation);

}

TEST_F(BusMapTest, PerformBusMappingTMATSaddedOnlyIfchidExistsInScannedMap)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8,10,11 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // unique LRU

	// Build Input Maps
	tmats_chanid_to_source_map[10] = "BUS2"; // channel id 10 is not in scanned map and should not be added to the final bus map

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map,
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	std::map<uint64_t, std::string> res;
	b.PerformBusMapping(res, 2, false);
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 1);
	EXPECT_EQ(res[1], "BUS1");
}





TEST_F(BusMapTest, PerfomBusMappingReturnsFalseIfNothingMappedAndUserStopIsFalse)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 20 }); // none

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res, 4, false);
	EXPECT_FALSE(continue_translation);
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);
}

TEST_F(BusMapTest, PerfomBusMappingReturnsTrueIfEverythingMatchedAndSkipsUserInput)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); 

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res, 4, true);
	EXPECT_TRUE(continue_translation);
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 1);
	EXPECT_EQ(res[1], "BUS1");
}

// User decides to run translation
TEST_F(BusMapTest, UserAdjustmentsUserSpecifiesContinue)
{

  std::vector<std::string> adj_vec = { "1" };
	bool continue_translation = 
		b.UserAdjustments(&adj_vec);
	EXPECT_TRUE(continue_translation);
}

// User decides to quit translation
TEST_F(BusMapTest, UserAdjustmentsUserSpecifiesQuit)
{
  std::vector<std::string> adj_vec = { "q" };
	bool continue_translation = 
		b.UserAdjustments(&adj_vec);
	EXPECT_FALSE(continue_translation);
}

// Junk then continue
TEST_F(BusMapTest, UserAdjustmentsJunkInputThenContinue)
{
  std::vector<std::string> adj_vec = { "4","0","a","1" };
	bool continue_translation = 
		b.UserAdjustments(&adj_vec);
	EXPECT_TRUE(continue_translation);
}

// Junk then quit
TEST_F(BusMapTest, UserAdjustmentsJunkInputThenQuit)
{
  std::vector<std::string> adj_vec = { "4","0","a","q" };
	bool continue_translation = 
		b.UserAdjustments(&adj_vec);
	EXPECT_FALSE(continue_translation);
}

// Junk -> Adjust then quit
TEST_F(BusMapTest, UserAdjustmentsJunkInputThenAdjustThenQuit)
{
  
  std::vector<std::string> adj_vec = { "4","0","2","q" };
	bool continue_translation = 
		b.UserAdjustments(&adj_vec);
	EXPECT_FALSE(continue_translation);
}

// Adjust -> make a change to channel ID -> then quit
TEST_F(BusMapTest, UserAdjustmentsAdjustThenValidChannelIDThenQuit)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8,10,11 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // matching set
	ch10_scanned_chanid_to_lruaddrs_map[5] = std::set<uint64_t>({ 99 }); // non matching set

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map,
		ch10_scanned_chanid_to_lruaddrs_map,
		tmats_chanid_to_source_map);

	std::vector<std::string> adj_vec = { "2","1","q" };

	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res, 4, true, &adj_vec);
	EXPECT_FALSE(continue_translation);
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);
}

// Adjust -> invalid channel id -> then quit
TEST_F(BusMapTest, UserAdjustmentsAdjustThenInvalidChannelIDThenQuit)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8,10,11 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // matching set
	ch10_scanned_chanid_to_lruaddrs_map[5] = std::set<uint64_t>({ 99 }); // non matching set

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map,
		ch10_scanned_chanid_to_lruaddrs_map,
		tmats_chanid_to_source_map);

	std::vector<std::string> adj_vec = { "2","invalid","q" };
	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res, 4, true, &adj_vec);
	EXPECT_FALSE(continue_translation);
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);
}

// Adjust -> valid channel id -> invalid bus names -> then quit
TEST_F(BusMapTest, UserAdjustmentsAdjustThenValidChannelIDThenInvalidBusNameThenQuit)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8,10,11 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // matching set
	ch10_scanned_chanid_to_lruaddrs_map[5] = std::set<uint64_t>({ 99 }); // non matching set

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map,
		ch10_scanned_chanid_to_lruaddrs_map,
		tmats_chanid_to_source_map);

	std::vector<std::string> adj_vec = { "2","1","invalid","q" };
	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res, 4, true, &adj_vec);
	EXPECT_FALSE(continue_translation);
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);
}

TEST_F(BusMapTest, UserAdjustmentsAdjustThenInvalidChannelIDThenValidChannelIDThenInvalidBusNameThenValidBusName)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 3,7,10 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // unique LRU
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 3,7,8 }); // unique Subset
	ch10_scanned_chanid_to_lruaddrs_map[5] = std::set<uint64_t>({ 99 }); // non matching set

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);

	std::vector<std::string> adj_vec = { "2","invalid","2","invalid","BUS3","q" };
	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res, 4, true, &adj_vec);
	EXPECT_FALSE(continue_translation);
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);

	std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map = 
		b.GetFinalBusMap_withSource();

	EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map).size() == 2);
	EXPECT_EQ(final_bus_map[1].first, "BUS1");
	EXPECT_EQ(final_bus_map[1].second, "UniqueLRU");

	EXPECT_EQ(final_bus_map[2].first, "BUS3"); // Previous mapping was chid 2 -> BUS2
	EXPECT_EQ(final_bus_map[2].second, "USER");
	EXPECT_FALSE(continue_translation);

}

TEST_F(BusMapTest, UserAdjustmentsOverrideExistingWithNewNameAndSource)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 20});

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // unique LRU
	ch10_scanned_chanid_to_lruaddrs_map[5] = std::set<uint64_t>({ 99 }); // non matching set

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);

	std::vector<std::string> adj_vec = { "2","1","BUS2","q" };
	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res, 4, true, &adj_vec);
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);

	std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map = 
		b.GetFinalBusMap_withSource();

	EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map).size() == 1);
	EXPECT_EQ(final_bus_map[1].first, "BUS2");
	EXPECT_EQ(final_bus_map[1].second, "USER");
	EXPECT_FALSE(continue_translation);
}

TEST_F(BusMapTest, UserAdjustmentsAddFromNonMappedToFinalMap)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 3,7,10 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // unique LRU
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 3,7,8 }); // unique subset
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 20 }); // none
	ch10_scanned_chanid_to_lruaddrs_map[4] = std::set<uint64_t>({ 20,23 }); // none

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);

	std::vector<std::string> adj_vec = { "2","3","BUS3","q" };
	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res, 4, true, &adj_vec);
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);

	std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map = 
		b.GetFinalBusMap_withSource();
	EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map).size() == 3);
	EXPECT_FALSE(continue_translation);
	EXPECT_EQ(final_bus_map[1].first, "BUS1");
	EXPECT_EQ(final_bus_map[1].second, "UniqueLRU");
	EXPECT_EQ(final_bus_map[2].first, "BUS1");
	EXPECT_EQ(final_bus_map[2].second, "UniqueSubset");
	EXPECT_EQ(final_bus_map[3].first, "BUS3");
	EXPECT_EQ(final_bus_map[3].second, "USER");
	EXPECT_FALSE(continue_translation);
}

TEST_F(BusMapTest, UserAdjustmentsMapMultipleAndContinueAdjustsFinalMap)
{
	// build map from comet
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 3,7,8,9,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 3,8,10,11 });
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>({ 3,7,10 });

	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // unique LRU
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>({ 3,7,8 }); // unique subset
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>({ 20 }); // none
	ch10_scanned_chanid_to_lruaddrs_map[4] = std::set<uint64_t>({ 20,23 }); // none

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map,
		ch10_scanned_chanid_to_lruaddrs_map,
		tmats_chanid_to_source_map);

	std::vector<std::string> adj_vec = { "2","invalid_chid","3","invalid_bus",
						  "BUS3","2","invalid_chid","4",
						  "invalid_bus","BUS1","1" };
	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res, 4, true, &adj_vec);

	std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map = 
		b.GetFinalBusMap_withSource();
	EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map).size() == 4);
	EXPECT_EQ(final_bus_map[1].first, "BUS1");
	EXPECT_EQ(final_bus_map[1].second, "UniqueLRU");
	EXPECT_EQ(final_bus_map[2].first, "BUS1");
	EXPECT_EQ(final_bus_map[2].second, "UniqueSubset");
	EXPECT_EQ(final_bus_map[3].first, "BUS3");
	EXPECT_EQ(final_bus_map[3].second, "USER");
	EXPECT_EQ(final_bus_map[4].first, "BUS1");
	EXPECT_EQ(final_bus_map[4].second, "USER");
	ASSERT_TRUE(iterable_tools_.GetKeys(res).size() == 4);
	EXPECT_EQ(res[1], "BUS1");
	EXPECT_EQ(res[2], "BUS1");
	EXPECT_EQ(res[3], "BUS3");
	EXPECT_EQ(res[4], "BUS1");
	EXPECT_TRUE(continue_translation);
}

TEST_F(BusMapTest, UserAdjustmentsEnsureNoMapInputDoesnotAllowChangesToFinalMap)
{
	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);

	std::vector<std::string> adj_vec = { "2","invalid","q" };
	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res, 4, true, &adj_vec);
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);

	std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map = 
		b.GetFinalBusMap_withSource();

	EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map).size() == 0);
	EXPECT_FALSE(continue_translation);
}

TEST_F(BusMapTest, UserAdjustmentsEnsureNoMapInputDoesnotAllowChangesToFinalMap2)
{
	// build ch10 scanned map
	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>({ 9 }); // unique LRU

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);

	std::vector<std::string> adj_vec = { "2", "1", "invalid", "q" };
	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res, 4, true, &adj_vec);
	EXPECT_TRUE(iterable_tools_.GetKeys(res).size() == 0);

	std::map<uint64_t, std::pair<std::string, std::string>> final_bus_map = 
		b.GetFinalBusMap_withSource();

	EXPECT_TRUE(iterable_tools_.GetKeys(final_bus_map).size() == 0);
	EXPECT_FALSE(continue_translation);
}

/*
//// OLD TMATS type tests

TEST_F(BusMapTest, CleanTmatsMaps1553AndExtraTypes)
{
	// Build Input Maps
	tmats_chanid_to_source_map[1] = "BUS1";
	tmats_chanid_to_source_map[2] = "BUS2";
	tmats_chanid_to_source_map[3] = "BUS3";
	tmats_chanid_to_source_map[4] = "BUS4";

	tmats_chanid_to_type_map[1] = "1553";
	tmats_chanid_to_type_map[2] = "1553";
	tmats_chanid_to_type_map[3] = "Video";
	tmats_chanid_to_type_map[4] = "Other";

	// Build Expected Maps
	tmats_1553_chanid_compare_map[1] = "BUS1";
	tmats_1553_chanid_compare_map[2] = "BUS2";

	b.InitializeMaps(term_mux_lines, ch10_scanned_chanid_to_lruaddrs_map, tmats_chanid_to_source_map);
	EXPECT_TRUE(map_compare(b.GetTmats1553ChanID_ToBusNameMap(), tmats_1553_chanid_compare_map));
}


TEST_F(BusMapTest, CleanTmatsMaps1553WithExtraInsertionsAroundAndPartial1553)
{
	// Build Input Maps
	tmats_chanid_to_source_map[1] = "BUS1";
	tmats_chanid_to_source_map[2] = "BUS2";
	tmats_chanid_to_source_map[3] = "BUS3";

	tmats_chanid_to_type_map[1] = "IN1553";
	tmats_chanid_to_type_map[2] = "1553OUT";
	tmats_chanid_to_type_map[3] = "155A"; // partial 1553 shouldn't make it through

	// Build Expected Maps
	tmats_1553_chanid_compare_map[1] = "BUS1";
	tmats_1553_chanid_compare_map[2] = "BUS2";

	b.InitializeMaps(term_mux_lines, ch10_scanned_chanid_to_lruaddrs_map, tmats_chanid_to_type_map, tmats_chanid_to_source_map );
	EXPECT_TRUE(map_compare(b.GetTmats1553ChanID_ToBusNameMap(), tmats_1553_chanid_compare_map));
}

TEST_F(BusMapTest, CleanTmatsMaps1553ChannelIDNotInSourceMap)
{
	// Build Input Maps
	tmats_chanid_to_source_map[1] = "BUS1";
	tmats_chanid_to_source_map[3] = "BUS3";

	tmats_chanid_to_type_map[1] = "1553";
	tmats_chanid_to_type_map[2] = "1553";
	tmats_chanid_to_type_map[3] = "1553";

	// Build Expected Maps
	tmats_1553_chanid_compare_map[1] = "BUS1";
	tmats_1553_chanid_compare_map[3] = "BUS3";

	b.InitializeMaps(term_mux_lines, ch10_scanned_chanid_to_lruaddrs_map, tmats_chanid_to_source_map);
	EXPECT_TRUE(map_compare(b.GetTmats1553ChanID_ToBusNameMap(), tmats_1553_chanid_compare_map));
}
*/




TEST_F(BusMapTest, InitializeMapsInitialMapsAssigned)
{
	// Empty Map
	b.InitializeMaps(&icd_message_key_to_busnames_map,
		std::set<uint64_t>(),
		tmats_chanid_to_source_map);

	EXPECT_EQ(b.GetICD_MessageKeyToBusNamesMap()->size(),0);

	// Map with entries
	icd_message_key_to_busnames_map[1] = std::set<std::string>({ "BusA", "BusB", "BusC" });
	icd_message_key_to_busnames_map[2] = std::set<std::string>({ "BusA" });

	b.InitializeMaps(&icd_message_key_to_busnames_map,
		std::set<uint64_t>({5,6,7,8}),
		tmats_chanid_to_source_map);
	EXPECT_EQ(*b.GetICD_MessageKeyToBusNamesMap(), icd_message_key_to_busnames_map);
	EXPECT_THAT(b.GetChannelIDs(), ::testing::ElementsAre(5, 6, 7, 8));
}

TEST_F(BusMapTest, InitializeMapsMessageKeyToChannelIDCreationAndUniqueBuses)
{
	// Map with entries
	icd_message_key_to_busnames_map[1] = std::set<std::string>({ "BusA", "BusB", "BusC" });
	icd_message_key_to_busnames_map[2] = std::set<std::string>({ "BusA" });
	icd_message_key_to_busnames_map[3] = std::set<std::string>({ "BusD", "BusB" });

	b.InitializeMaps(&icd_message_key_to_busnames_map,
		std::set<uint64_t>(),
		tmats_chanid_to_source_map);

	icd_message_key_to_channelids_map[1] = std::set<uint64_t>();
	icd_message_key_to_channelids_map[2] = std::set<uint64_t>();
	icd_message_key_to_channelids_map[3] = std::set<uint64_t>();

	EXPECT_EQ(b.GetICD_MessageKeyToChannelIDSMap(), icd_message_key_to_channelids_map);
	EXPECT_THAT(b.GetUniqueBuses(), ::testing::ElementsAre("BusA", "BusB", "BusC", "BusD"));
}

// If tmats source map is empty, bus map is considered to be given no tmats data
TEST_F(BusMapTest, InitializeMapsTMATSCheck)
{
	// No tmats provided
	b.InitializeMaps(&icd_message_key_to_busnames_map,
		std::set<uint64_t>());
	EXPECT_FALSE(b.TmatsPresent());

	// empty tmats provided
	b.InitializeMaps(&icd_message_key_to_busnames_map,
		std::set<uint64_t>(),
		tmats_chanid_to_source_map);
	EXPECT_FALSE(b.TmatsPresent());

	tmats_chanid_to_source_map[1] = "BUS1";
	tmats_chanid_to_source_map[2] = "BUS2";
	tmats_chanid_to_source_map[3] = "BUS3";
	tmats_chanid_to_source_map[4] = "BUS4";

	// tmats provided
	b.InitializeMaps(&icd_message_key_to_busnames_map,
		std::set<uint64_t>(),
		tmats_chanid_to_source_map);
	EXPECT_TRUE(b.TmatsPresent());

	EXPECT_EQ(b.GetTMATSchannelidToSourceMap(), tmats_chanid_to_source_map);
}

TEST_F(BusMapTest, InitializeMapsTMATSReplacements)
{
	// No tmats provided
	b.InitializeMaps(&icd_message_key_to_busnames_map,
		std::set<uint64_t>());
	EXPECT_FALSE(b.TmatsPresent());

	// empty tmats provided
	b.InitializeMaps(&icd_message_key_to_busnames_map,
		std::set<uint64_t>(),
		tmats_chanid_to_source_map);
	EXPECT_FALSE(b.TmatsPresent());

	tmats_chanid_to_source_map[1] = "BUS1";
	tmats_chanid_to_source_map[2] = "BUS2";
	tmats_chanid_to_source_map[3] = "BUS3";
	tmats_chanid_to_source_map[4] = "BUS4";

	std::map<std::string, std::string> tmats_replacements;

	tmats_replacements["BUS3"] = "BUS3Replacement";
	tmats_replacements["BUS1"] = "BUS1Replacement";
	tmats_replacements["BUSB"] = "NonExistantReplacement";

	std::map<uint64_t, std::string> compare_map;
	compare_map[1] = "BUS1Replacement";
	compare_map[2] = "BUS2";
	compare_map[3] = "BUS3Replacement";
	compare_map[4] = "BUS4";

	// tmats provided with tmats replacements
	b.InitializeMaps(&icd_message_key_to_busnames_map,
		std::set<uint64_t>(),
		tmats_chanid_to_source_map,
		tmats_replacements);

	EXPECT_TRUE(b.TmatsPresent());
	EXPECT_EQ(b.GetTMATSchannelidToSourceMap(), compare_map);
}


TEST_F(BusMapTest, SubmitMessages)
{
	// Map with entries
	// keys are transmit command word bit shifted left 16 bits
	// followed by the recieve command word
	icd_message_key_to_busnames_map[10 << 16 | 10] = std::set<std::string>();
	icd_message_key_to_busnames_map[11 << 16 | 11] = std::set<std::string>();
	icd_message_key_to_busnames_map[12 << 16 | 12] = std::set<std::string>();
	icd_message_key_to_busnames_map[13 << 16 | 13] = std::set<std::string>();
	icd_message_key_to_busnames_map[14 << 16 | 14] = std::set<std::string>();
	icd_message_key_to_busnames_map[15 << 16 | 15] = std::set<std::string>();
	icd_message_key_to_busnames_map[16 << 16 | 16] = std::set<std::string>();
	icd_message_key_to_busnames_map[17 << 16 | 17] = std::set<std::string>();
	icd_message_key_to_busnames_map[18 << 16 | 18] = std::set<std::string>();
	icd_message_key_to_busnames_map[19 << 16 | 19] = std::set<std::string>();
	icd_message_key_to_busnames_map[20 << 16 | 20] = std::set<std::string>();

	b.InitializeMaps(&icd_message_key_to_busnames_map,
		std::set<uint64_t>());

	std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({10,10,11,12,13,14,15,16,17,18,19,20,20});
	std::vector<uint64_t> recieve_cmds =  std::vector<uint64_t>({10,10,11,12,13,20,15,16,17,10,19,20,20});
	std::vector<uint64_t> channel_ids  =  std::vector<uint64_t>({ 0, 1, 1, 2, 3, 4, 5, 5, 7, 8, 9,10,10});

	std::unordered_map<uint64_t, std::set<uint64_t>> compare_map;
	compare_map[10 << 16 | 10] = std::set<uint64_t>({0,1}); // multiple matches with different channel IDs
	compare_map[11 << 16 | 11] = std::set<uint64_t>({1});
	compare_map[12 << 16 | 12] = std::set<uint64_t>({2});
	compare_map[13 << 16 | 13] = std::set<uint64_t>({3});
	compare_map[14 << 16 | 14] = std::set<uint64_t>();      // missing from input
	compare_map[15 << 16 | 15] = std::set<uint64_t>({5});
	compare_map[16 << 16 | 16] = std::set<uint64_t>({5});   // duplicate channel ids with different keys
	compare_map[17 << 16 | 17] = std::set<uint64_t>({7});
	compare_map[18 << 16 | 18] = std::set<uint64_t>();      // missing from input
	compare_map[19 << 16 | 19] = std::set<uint64_t>({9});
	compare_map[20 << 16 | 20] = std::set<uint64_t>({10});   // duplicate channel ids with same keys

	EXPECT_TRUE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));
	EXPECT_EQ(b.GetICD_MessageKeyToChannelIDSMap(), compare_map);
}

TEST_F(BusMapTest, SubmitMessagesNonEqualSizedVectors)
{
	b.InitializeMaps(&icd_message_key_to_busnames_map,
		std::set<uint64_t>());

	std::vector<uint64_t> transmit_cmds = std::vector<uint64_t>({ 10,10 });
	std::vector<uint64_t> recieve_cmds = std::vector<uint64_t>({ 10,10,11 });
	std::vector<uint64_t> channel_ids = std::vector<uint64_t>({ 0, 1, 1, 2});

	EXPECT_FALSE(b.SubmitMessages(transmit_cmds, recieve_cmds, channel_ids));
}

TEST_F(BusMapTest, VoteMappingNoMatches)
{
	icd_message_key_to_busnames_map[10] = std::set<std::string>({ "BusA","BusB" });
	icd_message_key_to_busnames_map[11] = std::set<std::string>({ "BusB", "BusC" });
	icd_message_key_to_busnames_map[12] = std::set<std::string>({ "BusC", "BusD" });

	b.InitializeMaps(&icd_message_key_to_busnames_map,
		std::set<uint64_t>());

	icd_message_key_to_channelids_map[10] = std::set<uint64_t>();
	icd_message_key_to_channelids_map[11] = std::set<uint64_t>();
	icd_message_key_to_channelids_map[12] = std::set<uint64_t>();

	std::map<uint64_t, std::string> compare_map;

	EXPECT_EQ(b.TestVoteMapping(icd_message_key_to_channelids_map), compare_map);
}

TEST_F(BusMapTest, VoteMappingMatchHighestVotes)
{
	icd_message_key_to_busnames_map[10] = std::set<std::string>({ "BusA","BusB" });
	icd_message_key_to_busnames_map[11] = std::set<std::string>({ "BusB", "BusC" });
	icd_message_key_to_busnames_map[12] = std::set<std::string>({ "BusC", "BusD" });
	icd_message_key_to_busnames_map[13] = std::set<std::string>({ "BusD", "BusE" });
	icd_message_key_to_busnames_map[14] = std::set<std::string>({ "BusD", "BusE" });
	icd_message_key_to_busnames_map[15] = std::set<std::string>({ "BusE" });
	icd_message_key_to_busnames_map[16] = std::set<std::string>({ "BusG" });

	b.InitializeMaps(&icd_message_key_to_busnames_map,
		std::set<uint64_t>({ 1,2,4 }));

	icd_message_key_to_channelids_map[10] = std::set<uint64_t>({ 1});
	icd_message_key_to_channelids_map[11] = std::set<uint64_t>({ 1 });
	icd_message_key_to_channelids_map[12] = std::set<uint64_t>({ 2 });
	icd_message_key_to_channelids_map[13] = std::set<uint64_t>({ 2, 4 });
	icd_message_key_to_channelids_map[14] = std::set<uint64_t>({ 4 });
	icd_message_key_to_channelids_map[15] = std::set<uint64_t>({ 4 });
	icd_message_key_to_channelids_map[16] = std::set<uint64_t>({ });

	std::map<uint64_t, std::string> compare_map;
	compare_map[1] = "BusB";
	compare_map[2] = "BusD";
	compare_map[4] = "BusE";

	EXPECT_EQ(b.TestVoteMapping(icd_message_key_to_channelids_map), compare_map);
}

TEST_F(BusMapTest, VoteMappingMatchNoMatchesWhenVoteCountIsTheSame)
{
	icd_message_key_to_busnames_map[10] = std::set<std::string>({ "BusA","BusB" });
	// Bus A and B are a tie
	icd_message_key_to_busnames_map[11] = std::set<std::string>({ "BusA", "BusB", "BusC" });
	// Channel ID 2 will tie Bus C 
	icd_message_key_to_busnames_map[12] = std::set<std::string>({ "BusC" });
	// Channel ID 3 should break the tie with BusC
	icd_message_key_to_busnames_map[13] = std::set<std::string>({ "BusC" });

	b.InitializeMaps(&icd_message_key_to_busnames_map,
		std::set<uint64_t>({ 1,2,4 }));

	icd_message_key_to_channelids_map[10] = std::set<uint64_t>({ 1, 2, 3 });
	icd_message_key_to_channelids_map[11] = std::set<uint64_t>({ 1, 2, 3 });
	icd_message_key_to_channelids_map[12] = std::set<uint64_t>({ 2, 3 });
	icd_message_key_to_channelids_map[13] = std::set<uint64_t>({ 3 });


	std::map<uint64_t, std::string> compare_map;
	compare_map[3] = "BusC";

	EXPECT_EQ(b.TestVoteMapping(icd_message_key_to_channelids_map), compare_map);
}