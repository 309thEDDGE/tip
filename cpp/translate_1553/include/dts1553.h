#ifndef DTS1553_H
#define DTS1553_H

#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include "yaml-cpp/yaml.h"
#include "icd_data.h"
#include "managed_path.h"

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
        {"translatable_message_definitions", DTS1553Component::TRANSL_MESSAGE_DEFS}};

    // Fill with supplemental bus map command words data if present in the
    // yaml file. The message key is a integer created by upshifting the tx
    // command word by 16 bits and adding the rx commmand word. The key is
    // the bus name on which the tx and rx command words used to create the
    // mapped set occur.
    std::map<std::string, std::set<uint64_t>> suppl_bus_name_to_message_key_map_;

   public:
    DTS1553() : icd_data_(), icd_data_ptr_(&icd_data_) {}

    ICDData GetICDData() { return icd_data_; }
    ICDData* ICDDataPtr() { return icd_data_ptr_; }
    std::map<std::string, std::set<uint64_t>> GetSupplBusNameToMessageKeyMap()
    {
        return suppl_bus_name_to_message_key_map_;
    }

    /*
		IngestLines

		dts_path:	Full path to dts file. File name is used determine file type,
		            either yaml or text/csv.

		lines:		All non-newline-terminated lines of text from the dts file.


		return:		True if success, false if failure.
	
	*/
    bool IngestLines(const ManagedPath& dts_path, const std::vector<std::string>& lines);

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

    /*
	
		FillSupplBusNameToMsgKeyMap

		suppl_busmap_comm_words_node:			Yaml node containing maps with
												keys corresponding bus names and
												values as sequences of pairs of
												command words.

		output_suppl_busname_to_msg_key_map:	Output maps the bus name to a set
												of message keys, where a message 
												key is an integer created from a 
												uint16_t transmit command word 
												upshifted by 16 bits added to a 
												uint16_t receive command word.

		return:									True if node is empty or has valid
												structure (maps of strings to
												sequences of sequences of two 
												uint16_t values) and false otherwise.
												Output map is empty if node is empty
												or return value is false.
	*/
    bool FillSupplBusNameToMsgKeyMap(const YAML::Node& suppl_busmap_comm_words_node,
                                     std::map<std::string, std::set<uint64_t>>& output_suppl_busname_to_msg_key_map);
};

#endif
