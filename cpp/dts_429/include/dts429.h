#ifndef DTS429_H
#define DTS429_H

#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include "yaml-cpp/yaml.h"
#include "icd_data.h"
#include "managed_path.h"
#include "tip_md_document.h"
#include "subchannel_map.h"

// Explicit indication of DTS429 components
enum class DTS429Component : uint8_t
{
    BAD = 0,
    TRANSL_WORD_DEFS = 1,
    SUPPL_BUSMAP_LABELS = 2
};

// DTS429 - Data Translation Specification, 429
//
// This class manages the parsing, manipulation and processing of all data
// relevant to translation of ARINC 429 bus data payloads to engineering units.
// The primary data of interest is a representation of the Interface Control
// Document (ICD). These data are managed by the ICDData class.
//
// Data supplemental to bus mapping and which include 429 Labels of
// words which may be present in the raw 429 parquet tables that are not
// included in the 429 message descriptions are processed by the
// SupplementalBusMapLabels class.
//
// Currently a yaml file, containing message definitions and supplemental bus
// map labels, is allowed as input.

class DTS429
{
   private:
    // Ingest and manipulate ICD data
    ICDData icd_data_;
    ICDData* icd_data_ptr_;
    std::vector<std::string> yaml_lines_;
    TIPMDDocument parse_metadata_doc_;
    TIPMDDocument* parse_metadata_doc_ptr_;
    SubchannelMap subchannel_map_;

    // Map the top-level DTS1553 yaml file key string to a DTS1553Component
    const std::map<std::string, DTS429Component> yaml_key_to_component_map_ = {
        {"supplemental_bus_map_labels", DTS429Component::SUPPL_BUSMAP_LABELS},
        {"translatable_word_definitions", DTS429Component::TRANSL_WORD_DEFS}};

    // Fill with supplemental bus map labels data if present in the
    // yaml file. The word key is an integer created by upshifting the 429
    // label by 8 bits and adding bus number from the IPDH. The key is
    // the bus name on which the 429 labels + bus numbers used to create the
    // mapped set occur.
    std::map<std::string, std::set<uint32_t>> suppl_bus_name_to_word_key_map_;

   public:
    DTS429() : icd_data_(), icd_data_ptr_(&icd_data_), yaml_lines_(), parse_metadata_doc_(), parse_metadata_doc_ptr_(&parse_metadata_doc_),  {}

    ICDData GetICDData() { return icd_data_; }
    ICDData* ICDDataPtr() { return icd_data_ptr_; }
    SubchannelMap GetSubchannelMap() { return subchannel_map_;}
    const std::vector<std::string>& GetYamlLines() { return yaml_lines_; }
    std::map<std::string, std::set<uint32_t>> GetSupplBusNameToWordKeyMap()
    {
        return suppl_bus_name_to_word_key_map_;
    }


    /*
		OpenYamlFile

		dts_path:	Full path to dts file. File name is used determine file type,
		            either yaml or text/csv. The input file is opened and all data
                    is stored individually as new line terminated strings in
                    yaml_lines_.


		return:		True if success, false if failure.

	*/
    bool OpenYamlFile(const ManagedPath& dts_path,
                        std::map<std::string, std::string>& wrd_name_substitutions,
                        std::map<std::string, std::string>& elem_name_substitutions);

    /*
		IngestLines

		dts_path:	Full path to dts file. File name is used determine file type,
		            either yaml or text/csv.

		lines:		All non-newline-terminated lines of text from the dts file.

        wrd_name_substitution:   Map of original word name to substituted name.

        elem_name_substitution: Map of original elem name to substituted elem name.


		return:		True if success, false if failure.

	*/
    bool IngestLines(const std::vector<std::string>& lines,
                     std::map<std::string, std::string>& wrd_name_substitutions,
                     std::map<std::string, std::string>& elem_name_substitutions);

    /*
		ProcessLinesAsYaml

		lines:							All non-newline-terminated lines of
										text from the dts file.

		transl_wrd_defs_node:			Output root node for translated word
										definitions map.

		suppl_busmap_comm_words_node:	Output root node for supplemental bus
										map labels.

		return:							True if success, otherwise false.

	*/
    bool ProcessLinesAsYaml(const std::vector<std::string>& lines,
                            YAML::Node& transl_wrd_defs_node,
                            YAML::Node& suppl_busmap_labels_node);

    /*

		FillSupplBusNameToWordKeyMap

		suppl_busmap_labels_node:			    Yaml node containing maps with
												keys corresponding bus names and
												values as sequences of integers
												representing ARINC 429 labels.

		output_suppl_busname_to_wrd_key_map:	Output maps the bus name to a set
												of 429 word keys, where a word
												key is an integer representing an
                                                ARINC label that is associated with
                                                the bus name.

		return:									True if node is empty or has valid
												structure (maps of strings to
												sequences of integer values)
                                                and false otherwise.
												Output map is empty if node is empty
												or return value is false.
	*/
    bool FillSupplBusNameToWordKeyMap(const YAML::Node& suppl_busmap_labels_node,
                                     std::map<std::string, std::set<uint32_t>>& output_suppl_busname_to_wrd_key_map);


    /*

		OrganizeParseMetadata

		parser_md_doc:	TIPMDDocument which read in string representation of
                        the ARINC 429 metadata file, containing new line
                        character at the end of each line.

		return:	  True if success. False if fails at any step.
	*/
    bool ManageParseMetadata(TIPMDDocument& parser_md_doc);

};

#endif
