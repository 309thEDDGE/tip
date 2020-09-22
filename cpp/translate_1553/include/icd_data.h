#ifndef ICD_DATA_H
#define ICD_DATA_H

#include <map>
#include <set>
#include <cstdint>
#include "icd_element.h"
#include "parse_text.h"
#include "iterable_tools.h"
#include <filesystem>
#include <cctype>
#include <algorithm>
#include "yaml-cpp/yaml.h"
#include <sstream>

class ICDData
{
private:
	IterableTools iter_tools_;
	bool icd_ingest_success_;
	bool organize_icd_success_; 
	std::vector<ICDElement> icd_elements_;
	std::unordered_map<ICDElementSchema, std::string> icdschema_to_string_map_;
	std::unordered_map<std::string, ICDElementSchema> string_to_icdschema_map_;
	using icdelem_vec = std::vector<ICDElement>;
	using table_inds_vec = std::set<size_t>;
	using bus_to_elem_map = std::unordered_map<std::string, icdelem_vec>;
	using xlru_to_elem_map = std::unordered_map<uint8_t, icdelem_vec>;
	using dlru_to_elem_map = std::unordered_map<uint8_t, icdelem_vec>;
	using xsub_to_elem_map = std::unordered_map<uint8_t, icdelem_vec>;
	using dsub_to_elem_map = std::unordered_map<uint8_t, icdelem_vec>;
	using dsub_to_inds_map = std::unordered_map<uint8_t, table_inds_vec>;

	using xsub_to_dsub_to_inds = std::unordered_map<uint8_t, dsub_to_inds_map>;
	using dlru_to_xsub_to_dsub_to_inds = std::unordered_map<uint8_t, xsub_to_dsub_to_inds>;
	using xlru_to_dlru_to_xsub_to_dsub_to_inds = std::unordered_map<uint8_t, dlru_to_xsub_to_dsub_to_inds>;
	using complete_temp_lookup = std::unordered_map<std::string, xlru_to_dlru_to_xsub_to_dsub_to_inds>;
	using complete_lookup = std::unordered_map<uint16_t, xlru_to_dlru_to_xsub_to_dsub_to_inds>;
	complete_temp_lookup icd_temp_lookup_;
	complete_lookup icd_lookup_;
	table_inds_vec temp_table_inds_;

	icdelem_vec icd_msg_elements_;
	std::vector<std::string> table_names_;

	// Map a unique index to a vector of indices to ICDElement objects
	// in icd_elements_ that groups elements into tables. 
	// The final lookup returns the unique index which is 
	// the key in this table.
	std::vector<std::vector<size_t>> tables_;

	// Map of bus name to the set of LRU addresses which are present
	// on the bus.
	std::map<std::string, std::set<uint64_t>> bus_name_to_lru_addrs_map_;

	/*
	* Private high-level functions.
	*/
	void MapICDElementSchemaToString();
	bool CreateLookupMap();

	// Yaml icd members.
	std::vector<std::string> yaml_msg_body_keys_;
	std::vector<std::string> yaml_msg_data_keys_;
	std::vector<std::string> yaml_word_elem_keys_;
	std::vector<std::string> yaml_bit_elem_keys_;
	const size_t icd_sequence_size_ = 2;

public:
	ICDData();
	~ICDData();
	bool PrepareICDQuery(const std::vector<std::string>& lines, bool is_yaml_file=false);
	void PrepareMessageKeyMap(std::unordered_map<uint64_t, std::set<std::string>>& message_key_map);
	std::vector<std::vector<size_t>> GetTableOrganizationIndices();
	std::set<size_t> TempLookupTableIndex(const std::string& bus_name, uint8_t xmit_lru_addr, 
		uint8_t dest_lru_addr, uint8_t xmit_lru_subaddr, uint8_t dest_lru_subaddr);
	std::set<size_t> LookupTableIndex(uint16_t chanid, uint8_t xmit_lru_addr,
		uint8_t dest_lru_addr, uint8_t xmit_lru_subaddr, uint8_t dest_lru_subaddr);
	std::string LookupTableNameByIndex(size_t index);
	bool ReplaceBusNameWithChannelIDInLookup(const std::map<std::string, std::set<uint64_t>>& input_map);
	std::map<std::string, std::set<uint64_t>> GetBusNameToLRUAddrsMap();
	const std::vector<size_t>& GetTableIndices(const size_t& table_index);
	const ICDElement& GetElementByIndex(const size_t& table_index);
	const ICDElement* GetElementPointerByIndex(const size_t& table_index);
	size_t GetTableIndexByName(const std::string& table_name);

	/*
	* Below are primarily for testing and shouldn't be called by the user. 
	*/
	
	std::vector<uint16_t> GetLookupTableChannelIDKeys();

	// Note: return by value. Unless this class receives additional
	// manipulation functions. It should probably be instantiated in
	// a narrow scope, vector of elements retrieved, then garbage
	// collected to avoid duplication of this vector.
	std::vector<ICDElement> GetICDElementVector();

	// These functions should not be called invidually by the user.
	// Use PrepareICDQuery() to call all of them in the correct order,
	// using the correct data.
	bool IngestICDTextFileLines(const std::vector<std::string>& lines,
		std::vector<ICDElement>& icd_elems_output);
	bool CreateTables(const std::vector<ICDElement>& icd_elems,
		std::vector<std::vector<size_t>>& output_organized_inds);
	bool CollectMsgElementsFromTableInds(const std::vector<ICDElement>& all_icd_elements,
		const std::vector<std::vector<size_t>>& table_inds,
		std::vector<ICDElement>& output_icd_message_elements);
	bool CreateTableNameLookup(const std::vector<ICDElement>& icd_message_elements,
		std::vector<std::string>& output_table_names);
	bool CreateBusNameToLRUAddressSetMap(const std::vector<ICDElement>& icd_message_elements,
		std::map<std::string, std::set<uint64_t>>& output_bus_to_lru_addrs_map);
	void UpdateMapFromICDElementsToIndices(const dsub_to_elem_map& dsub_elem_map,
		const std::vector<std::string>& table_names, dsub_to_inds_map& dsub_inds_map);

	// These functions should be considered as part of the same category
	// as those immediately above, with the exception that the following
	// are related specifically to processing of yaml ICD input.
	bool IsYamlFile(const std::string& icd_path);
	bool IngestICDYamlFileLines(const std::vector<std::string>& lines,
		std::vector<ICDElement>& icd_elems_output);
	bool MapNodeHasRequiredKeys(const YAML::Node& node, 
		const std::vector<std::string>& required_keys);
	size_t FillElementsFromYamlNodes(const std::string& msg_name, const YAML::Node& msg_data_node,
		const YAML::Node& elem_node, bool is_bit_node, std::vector<ICDElement>& icd_elems);
	bool CreateVectorOfStringICDComponents(const std::string& msg_name, const YAML::Node& msg_data_node,
		const std::string& elem_name, const YAML::Node& elem_data_node, bool is_bit_elem, 
		std::vector<std::string>& output_vec);
	bool SequenceNodeHasCorrectSize(const YAML::Node& node, const size_t& required_size);
	bool ICDSchemaStringToEnum(const std::string& schema_string, ICDElementSchema& schema);
	
};


#endif

