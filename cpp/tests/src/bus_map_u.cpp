#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "bus_map.h"
#include "file_reader.h"
#include "icd_data.h"


class BusMapTest : public ::testing::Test
{
protected:
	
	BusMap b;
	std::map<std::string, std::set<uint64_t>> comet_compare_map;
	std::vector<std::string> term_mux_lines;
	std::set<uint64_t> temp_set;
	std::map<uint64_t, std::set<uint64_t>> ch10_scanned_compare_map;
	std::map<uint64_t, std::string> tmats_1553_chanid_compare_map;
	std::map<uint64_t, std::string> chID_to_busname_compare_map;
	std::map<std::string, std::set<uint64_t>> bus_name_to_lru_addresses_comet_map;
	IterableTools iterable_tools_;
	FileReader fr;

	// inputs
	std::map<uint64_t, std::set<uint64_t>> ch10_scanned_chanid_to_lruaddrs_map;
	std::map<uint64_t, std::string> tmats_chanid_to_source_map;
	std::map<uint64_t, std::string> tmats_chanid_to_type_map;

	BusMapTest()
	{
		
	}
	void SetUp() override
	{
	}

	template <typename Map>
	bool map_compare(Map const& lhs, Map const& rhs) {
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

TEST_F(BusMapTest, InitializeMapsCometMapImport)
{
	// Empty Map
	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	EXPECT_TRUE(map_compare(b.GetBusName_ToLRUAddressesCometMap(), comet_compare_map));

	std::map<uint64_t, std::string> res;
	bool continue_translation = b.PerformBusMapping(res, 1, false);
	EXPECT_EQ(res.size(), 0);

	// Map with entries
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>({ 1,2,3 });
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>({ 1 });

	comet_compare_map["BUS1"] = std::set<uint64_t>({ 1,2,3 });
	comet_compare_map["BUS2"] = std::set<uint64_t>({ 1 });
	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	EXPECT_TRUE(map_compare(b.GetBusName_ToLRUAddressesCometMap(), comet_compare_map));
}

TEST_F(BusMapTest, InitializeMapsScannedChannedIDS)
{	
	// Empty input map
	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	EXPECT_TRUE(map_compare(b.GetCH10ScannedChanID_ToLRUAddressesMap(), ch10_scanned_compare_map));

	temp_set.insert(1);
	temp_set.insert(2);
	temp_set.insert(3);
	ch10_scanned_chanid_to_lruaddrs_map[1] = temp_set;
	ch10_scanned_compare_map[1] = temp_set;

	temp_set.insert(4);
	temp_set.insert(5);
	temp_set.insert(5);
	ch10_scanned_chanid_to_lruaddrs_map[2] = temp_set;
	ch10_scanned_compare_map[2] = temp_set;

	// Non empty input
	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	ASSERT_TRUE(map_compare(b.GetCH10ScannedChanID_ToLRUAddressesMap(), ch10_scanned_compare_map));

	// rewrite don't add
	temp_set.clear();
	temp_set.insert(1);
	temp_set.insert(2);
	temp_set.insert(3);
	ch10_scanned_chanid_to_lruaddrs_map.clear();
	ch10_scanned_compare_map.clear();
	ch10_scanned_chanid_to_lruaddrs_map[1] = temp_set;
	ch10_scanned_compare_map[1] = temp_set;
	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	EXPECT_TRUE(map_compare(b.GetCH10ScannedChanID_ToLRUAddressesMap(), ch10_scanned_compare_map));
}

// If tmats source map is empty, bus map is considered to be given no tmats data
TEST_F(BusMapTest, InitializeMapsNoTMATSCheck)
{
	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map);
	EXPECT_FALSE(b.TmatsPresent());

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	EXPECT_FALSE(b.TmatsPresent());

	tmats_chanid_to_source_map[1] = "BUS1";

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	EXPECT_TRUE(b.TmatsPresent());
}

// If the bus name in comet is a substring of a bus name in tmats, 
// replace tmats bus name with comet bus name
TEST_F(BusMapTest, CleanTmatsMapsRemoveExtraMapsFromTMATS)
{
	// Built comet input
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>();
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>();

	// Build Input Maps
	tmats_chanid_to_source_map[1] = "BUS1";  
	tmats_chanid_to_source_map[2] = "BUS2";
	tmats_chanid_to_source_map[3] = "BUS3"; 

	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>();
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>();
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>();

	// Build Expected Maps
	tmats_1553_chanid_compare_map[1] = "BUS1"; 
	tmats_1553_chanid_compare_map[2] = "BUS2";

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);

	EXPECT_TRUE(map_compare(b.GetTmats1553ChanID_ToBusNameMap(), 
		tmats_1553_chanid_compare_map));
}

TEST_F(BusMapTest, CleanTmatsMaps1553DoNotAddIfMatchesCometNameButChannelIDisNotInScannedMap)
{
	// Built comet input
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>();
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>();
	bus_name_to_lru_addresses_comet_map["BUS3"] = std::set<uint64_t>();

	// Build Input Maps
	tmats_chanid_to_source_map[1] = "BUS1";
	tmats_chanid_to_source_map[2] = "BUS2";
	tmats_chanid_to_source_map[3] = "BUS3"; // should not add this

	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>();
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>();

	// Build Expected Maps
	tmats_1553_chanid_compare_map[1] = "BUS1";
	tmats_1553_chanid_compare_map[2] = "BUS2";


	b.InitializeMaps(bus_name_to_lru_addresses_comet_map,
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	EXPECT_TRUE(map_compare(b.GetTmats1553ChanID_ToBusNameMap(), tmats_1553_chanid_compare_map));
}

TEST_F(BusMapTest, CleanTmatsMapsTmatsEmptyNoCorrectionMap)
{
	// Empty Correction Map
	std::map<std::string, std::string> correction_map;
	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map,  
		tmats_chanid_to_source_map, correction_map);
	EXPECT_TRUE(map_compare(b.GetTmats1553ChanID_ToBusNameMap(), 
		tmats_1553_chanid_compare_map));

	// No Correction Map given
	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	EXPECT_TRUE(map_compare(b.GetTmats1553ChanID_ToBusNameMap(), tmats_1553_chanid_compare_map));
}

// Override tmats bus namesby specification from the config file
TEST_F(BusMapTest, CleanTmatsMapsTmatsCorrectionMapGiven)
{
	// Built comet input
	bus_name_to_lru_addresses_comet_map["RENAME1"] = std::set<uint64_t>();
	bus_name_to_lru_addresses_comet_map["RENAME2"] = std::set<uint64_t>();

	// Build Input Maps
	tmats_chanid_to_source_map[1] = "JUNK_TMATS1"; 
	tmats_chanid_to_source_map[2] = "JUNK_TMATS2";
	tmats_chanid_to_source_map[3] = "JUNK_TMATS3";
	tmats_chanid_to_source_map[4] = "JUNK_TMATS4";
	tmats_chanid_to_source_map[5] = "JUNK_TMATS2"; // duplicate correction

	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>();
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>();
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>();
	ch10_scanned_chanid_to_lruaddrs_map[4] = std::set<uint64_t>();
	ch10_scanned_chanid_to_lruaddrs_map[5] = std::set<uint64_t>();

	// Build tmats correction vector
	std::map<std::string, std::string> correction_map;
	correction_map["JUNK_TMATS1"] = "RENAME1";
	correction_map["JUNK_TMATS2"] = "RENAME2";

	// Unfound correction
	correction_map["JJ"] = "RENAME2";

	// Build Expected Maps
	tmats_1553_chanid_compare_map[1] = "RENAME1";
	tmats_1553_chanid_compare_map[2] = "RENAME2"; 
	tmats_1553_chanid_compare_map[5] = "RENAME2";

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map, correction_map);
	EXPECT_TRUE(map_compare(b.GetTmats1553ChanID_ToBusNameMap(), tmats_1553_chanid_compare_map));
}

// Extra tmats trimmed
TEST_F(BusMapTest, CleanTmatsDuplicateTmatsBusses)
{
	// Built comet input
	bus_name_to_lru_addresses_comet_map["BUS1"] = std::set<uint64_t>();
	bus_name_to_lru_addresses_comet_map["BUS2"] = std::set<uint64_t>();

	// Build Input Maps
	tmats_chanid_to_source_map[1] = "JUNK_TMATS1";
	tmats_chanid_to_source_map[2] = "BUS1";
	tmats_chanid_to_source_map[3] = "BUS2";
	tmats_chanid_to_source_map[4] = "JUNK_TMATS4";
	tmats_chanid_to_source_map[5] = "JUNK_TMATS2"; 
	tmats_chanid_to_source_map[6] = "BUS2";

	ch10_scanned_chanid_to_lruaddrs_map[1] = std::set<uint64_t>();
	ch10_scanned_chanid_to_lruaddrs_map[2] = std::set<uint64_t>();
	ch10_scanned_chanid_to_lruaddrs_map[3] = std::set<uint64_t>();
	ch10_scanned_chanid_to_lruaddrs_map[4] = std::set<uint64_t>();
	ch10_scanned_chanid_to_lruaddrs_map[5] = std::set<uint64_t>();
	ch10_scanned_chanid_to_lruaddrs_map[6] = std::set<uint64_t>();

	// Build Expected Maps
	tmats_1553_chanid_compare_map[2] = "BUS1";
	tmats_1553_chanid_compare_map[3] = "BUS2";
	tmats_1553_chanid_compare_map[6] = "BUS2";

	b.InitializeMaps(bus_name_to_lru_addresses_comet_map, 
		ch10_scanned_chanid_to_lruaddrs_map, 
		tmats_chanid_to_source_map);
	EXPECT_TRUE(map_compare(b.GetTmats1553ChanID_ToBusNameMap(), tmats_1553_chanid_compare_map));
}

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




///// OLD term mux tests

TEST_F(BusMapTest, AddLinesToBusnameLRUAddressMapRequiresThreeDelimiters)
{
	term_mux_lines.push_back("Non Delimiter Line");
	b.AddLinesTo_BusnameLRUAddressMap(term_mux_lines);
	ASSERT_TRUE(map_compare(b.GetBusName_ToLRUAddressesCometMap(), comet_compare_map));

	// One Delimiter
	term_mux_lines[0] ="A|";
	b.AddLinesTo_BusnameLRUAddressMap(term_mux_lines);
	ASSERT_TRUE(map_compare(b.GetBusName_ToLRUAddressesCometMap(), comet_compare_map));

	// Three Delimiters
	term_mux_lines[0] = "|B|B|";
	b.AddLinesTo_BusnameLRUAddressMap(term_mux_lines);
	ASSERT_TRUE(map_compare(b.GetBusName_ToLRUAddressesCometMap(), comet_compare_map));
}

TEST_F(BusMapTest, AddLinesToBusnameLRUAddressMapSkipsPound)
{
	term_mux_lines.push_back("# Comment Line");
	b.AddLinesTo_BusnameLRUAddressMap(term_mux_lines);
	ASSERT_TRUE(map_compare(b.GetBusName_ToLRUAddressesCometMap(), comet_compare_map));
}

TEST_F(BusMapTest, AddLinesToBusnameLRUAddressMapSkipsEmptyStringLine)
{
	term_mux_lines.push_back("");
	b.AddLinesTo_BusnameLRUAddressMap(term_mux_lines);
	ASSERT_TRUE(map_compare(b.GetBusName_ToLRUAddressesCometMap(), comet_compare_map));
}

TEST_F(BusMapTest, AddLinesToBusnameLRUAddressMapSkipsEmptyStringBusNameOrLRUAddress)
{
	term_mux_lines.push_back("AA|BB|");
	term_mux_lines.push_back("AA||CC");
	b.AddLinesTo_BusnameLRUAddressMap(term_mux_lines);
	ASSERT_TRUE(map_compare(b.GetBusName_ToLRUAddressesCometMap(), comet_compare_map));
}


TEST_F(BusMapTest, AddLinesToBusnameLRUAddressMapDoesNotDuplicate)
{
	term_mux_lines.push_back("AA|B1|20");
	term_mux_lines.push_back("AA|B1|20");
	b.AddLinesTo_BusnameLRUAddressMap(term_mux_lines);
	temp_set.insert(20);
	comet_compare_map["B1"] = temp_set;
	ASSERT_TRUE(map_compare(b.GetBusName_ToLRUAddressesCometMap(), comet_compare_map));
}

TEST_F(BusMapTest, AddLinesToBusnameLRUAddressMapAddMultiple)
{
	term_mux_lines.push_back("AA|B1|20");
	term_mux_lines.push_back("AA|B1|21");
	term_mux_lines.push_back("AA|B2|22");
	term_mux_lines.push_back("AA|B2|20");
	term_mux_lines.push_back("AA|B2|25");
	term_mux_lines.push_back("AA|B3|20");
	b.AddLinesTo_BusnameLRUAddressMap(term_mux_lines);
	temp_set.insert(20);
	temp_set.insert(21);
	comet_compare_map["B1"] = temp_set;
	temp_set.clear();
	temp_set.insert(22);
	temp_set.insert(20);
	temp_set.insert(25);
	comet_compare_map["B2"] = temp_set;
	temp_set.clear();
	temp_set.insert(20);
	comet_compare_map["B3"] = temp_set;
	ASSERT_TRUE(map_compare(b.GetBusName_ToLRUAddressesCometMap(), comet_compare_map));
}


TEST_F(BusMapTest, AddLinesToBusnameLRUAddressMapDoNotAddNegativeLRUAddress)
{
	term_mux_lines.push_back("AA|B1|-1");
	b.AddLinesTo_BusnameLRUAddressMap(term_mux_lines);
	ASSERT_TRUE(map_compare(b.GetBusName_ToLRUAddressesCometMap(), comet_compare_map));
}

TEST_F(BusMapTest, AddLinesToBusnameLRUAddressMapDoNotAddNonIntegerLRUAddresses)
{
	term_mux_lines.push_back("AA|B1|-1a");
	term_mux_lines.push_back("AA|B1|-");
	term_mux_lines.push_back("AA|B1|CC");
	b.AddLinesTo_BusnameLRUAddressMap(term_mux_lines);
	ASSERT_TRUE(map_compare(b.GetBusName_ToLRUAddressesCometMap(), comet_compare_map));
}


TEST_F(BusMapTest, PrepareUniqueElementMap)
{
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

	BusMap bm;
	bm.InitializeMaps(icd);
	std::unordered_map<uint64_t, UniqueKeyData> unique_key_map = bm.GetUniqueKeyMap();
	int64_t key1 = 39043;
	int64_t key2 = 48192 << 16 | 55360;
	int64_t key3 = 55360;
	int64_t key4 = 28894 << 16;

	std::set<size_t> truth_keys;
	truth_keys.insert(key1);
	truth_keys.insert(key2);
	truth_keys.insert(key3);
	truth_keys.insert(key4);

	//ASSERT_EQ(iterable_tools_.GetKeys(unique_key_map), truth_keys);
	ASSERT_THAT(iterable_tools_.GetKeys(unique_key_map), ::testing::ElementsAre(key1, key2, key3, key4));

	ASSERT_THAT(unique_key_map[key1] = UniqueKeyData(std::set<size_t>(), std::set<std::string>({ "Bus1", "Bus2", "Bus3", "Bus4" }));

	unique_key_map[key] = UniqueKeyData(std::set<size_t>(), std::set<std::string>({ "Bus3", "Bus4", "Bus5" }));

	unique_key_map[key] = UniqueKeyData(std::set<size_t>(), std::set<std::string>({ "Bus1", "Bus2" }));

	unique_key_map[key] = UniqueKeyData(std::set<size_t>(), std::set<std::string>({ "Bus1", "Bus2" }));
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

