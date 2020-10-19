#ifndef DTS1553_H
#define DTS1553_H

#include <sstream>
#include "yaml-cpp/yaml.h"
#include "icd_data.h"

// Explicit indication of DTS1553 components
enum class DTS1553Component : uint8_t
{
	BAD = 0,
	TRANSL_MESSAGE_DEFS = 1,
	SUPPL_BUSMAP_COMM_WORDS = 2
};

// DTS1553 - Data Translation Specification, 1553
//
// This class manages the parsing, manipulation and processing of all data
// relevant to translation of 1553 bus data payloads to engineering units.
// The primary data of interest is a representation of the Interface Control
// Document (ICD). These data are managed by the ICDData class. 
//
// Data supplemental to bus mapping and which include 1553 command words of 
// messages which may be present in the raw 1553 parquet tables that are not
// included in the 1553 message descriptions are processed by the 
// SupplementalBusMapCommWords class.
//
// Currently a CSV file, containing only translatable message definitions,
// or a yaml file, containing message definitions and supplemental bus
// map command words, are allowed as input. 

class DTS1553
{
private:

	// Ingest and manipulate ICD data
	ICDData icd_data_;
	ICDData* icd_data_ptr_;

	// Map the top-level DTS1553 yaml file key string to a DTS1553Component
	const std::map<std::string, DTS1553Component> yaml_key_to_component_map_ = {
		{"supplemental_bus_map_command_words", DTS1553Component::SUPPL_BUSMAP_COMM_WORDS},
		{"translatable_message_definitions", DTS1553Component::TRANSL_MESSAGE_DEFS}
	};

public:
	DTS1553() : icd_data_(), icd_data_ptr_(&icd_data_) { }

	ICDData GetICDData() { return icd_data_; }
	ICDData* ICDDataPtr() { return icd_data_ptr_; }

	/*
		IngestLines

		icd_path:	Full path to dts file. File name is used determine file type,
		            either yaml or text/csv.

		lines:		All non-newline-terminated lines of text from the dts file.


		return:		True if success, false if failure.
	
	*/
	bool IngestLines(const std::string& dts_path, const std::vector<std::string>& lines);

	/*
		ProcessLinesAsYaml

		lines:							All non-newline-terminated lines of 
										text from the dts file.

		transl_msg_defs_node:			Output root node for translated message
										definitions map.

		suppl_busmap_comm_words_node:	Output root node for supplemental bus
										map command words.

		return:							True if success, otherwise false.

	*/
	bool ProcessLinesAsYaml(const std::vector<std::string>& lines,
		YAML::Node& transl_msg_defs_node,
		YAML::Node& suppl_busmap_comm_words_node);


};


#endif
