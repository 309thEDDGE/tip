#ifndef DTS429_H
#define DTS429_H

#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include "yaml-cpp/yaml.h"
#include "icd_data.h"
#include "icd_element.h"
#include "managed_path.h"
#include "spdlog/spdlog.h"

// Explicit indication of DTS429 components
enum class DTS429Component : uint8_t
{
    BAD = 0,
    TRANSL_WORD_DEFS = 1,
    SUPPL_BUSMAP_LABELS = 2
};

const std::map<std::string, ICDElementSchema> StringToICDElementSchemaMap = {
	{"ASCII", ICDElementSchema::ASCII},
	{"SIGNEDBITS", ICDElementSchema::SIGNEDBITS},
	{"UNSIGNEDBITS", ICDElementSchema::UNSIGNEDBITS},
	{"BCD", ICDElementSchema::BCD},
	{"SIGNMAG", ICDElementSchema::SIGNMAG}
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
   /*
    // Ingest and manipulate ICD data
    ICDData icd_data_;
    ICDData* icd_data_ptr_;
    std::vector<std::string> yaml_lines_;
    std::unordered_map<std::string, std::vector<ICDElement>> word_name_to_elements_map_;
    */
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
    DTS429(){}
    /*
    DTS429() : icd_data_(), icd_data_ptr_(&icd_data_), yaml_lines_() {}
    ICDData GetICDData() { return icd_data_; }
    ICDData* ICDDataPtr() { return icd_data_ptr_; }
    const std::vector<std::string>& GetYamlLines() { return yaml_lines_; }
    */
    std::map<std::string, std::set<uint32_t>> GetSupplBusNameToWordKeyMap()
    {
        return suppl_bus_name_to_word_key_map_;
    }

    /*
		IngestLines

		dts_path:	Full path to dts file. File name is used determine file type,
		            either yaml or text/csv.

		lines:		All non-newline-terminated lines of text from the dts file.

        word_elements:       Map of ARINC 429 word name to vector of all elements
                                associated with it (as ICDElment)

		return:		True if success, false if failure.

	*/
    bool IngestLines(const std::vector<std::string>& lines,
                     std::unordered_map<std::string, std::vector<ICDElement>> word_elements);

    /*
		ProcessLinesAsYaml

		transl_wrd_defs_node:			Output root node for translated word
										definitions map.

		suppl_busmap_labels_node:	Output root node for supplemental bus
										map labels.

		return:							True if success, otherwise false.

	*/
    bool ProcessLinesAsYaml(const YAML::Node& root_node,
                            YAML::Node& transl_wrd_defs_node,
                            YAML::Node& suppl_busmap_labels_node);

    /*
		BuildNameToICDElementMap

		transl_wrd_defs_node:		Output root node for translated word
									definitions map.

        word_elements:              Map of ARINC 429 word name to vector of all elements
                                    associated with it (as ICDElment)

		return:					    True if success, otherwise false.

	*/
    bool BuildNameToICDElementMap(YAML::Node& transl_wrd_defs_node,
                     std::unordered_map<std::string, std::vector<ICDElement>>& word_elements);

    /*
    Perform validation of single word_node, the key:value YAML::Node in which
    the key is a 429 word name and the value is a map with keys 'wrd_data' and
    'elem'.

    Args:
        word_node       --> YAML::Node that is expected to the value which
                            is mapped to the word name/label

    Return:
        True if word_node is validated; false otherwise
    */
    bool ValidateWordNode(const YAML::Node& word_node);

    /*
    Create a new ICDElement from wrd_data and an elem as defined in
    tip_dts429_schema.yaml

    Args:
        msg_name        --> std::string - the name of the message with
                            which the element is associated. Alias in
                            tip_dts429_schema.yaml is wrd_name.

        elem_name       --> std::string - the name of the element being
                            created by this method. Alias in
                            tip_dts429_schema.yaml is elem_name.

        wrd_data        --> YAML::Node containing ARINC 429 word level
                            information.

        elem_data       --> YAML::Node which stores information specific
                            to a single parameter tied to the word defined
                            in wrd_data

        arinc_param     --> ICDElement object being created.

    Return:
        True if new ICDElement is created; false otherwise

    */
    bool CreateICDElementFromWordNodes(const std::string& msg_name,
                                       const std::string& elem_name,
                                       const YAML::Node& wrd_data,
                                       const YAML::Node& elem_data,
                                       ICDElement& arinc_param);

    /*
		OpenYamlFile

		dts_path:	Full path to dts file. File name is used determine file type,
		            either yaml or text/csv. The input file is opened and all data
                    is stored individually as new line terminated strings in
                    yaml_lines_.

        word_elements:       Map of ARINC 429 word name to vector of all elements
                                associated with it (as ICDElment)

		return:		True if success, false if failure.

	*/
    // bool OpenYamlFile(const ManagedPath& dts_path,
    //                   std::unordered_map<std::string, std::vector<ICDElement>> word_elements);

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
    // bool FillSupplBusNameToWordKeyMap(const YAML::Node& suppl_busmap_labels_node,
    //                                  std::map<std::string, std::set<uint32_t>>& output_suppl_busname_to_wrd_key_map);

};

#endif
