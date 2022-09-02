#ifndef DTS429_H
#define DTS429_H

#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
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
// Currently a vector of strings containing the lines from the 429 DTS and
// a map for newly created ICD elements to be stored (with word name as key)
// is the allowed input. Processing begins with IngestLines().

class DTS429
{
   private:
    // Map the top-level DTS1553 yaml file key string to a DTS1553Component
    const std::map<std::string, DTS429Component> yaml_key_to_component_map_ = {
        {"supplemental_bus_map_labels", DTS429Component::SUPPL_BUSMAP_LABELS},
        {"translatable_word_definitions", DTS429Component::TRANSL_WORD_DEFS}};

   public:
    DTS429(){}
    virtual ~DTS429() {}

    /*
		IngestLines

		dts_path:	Full path to dts file. File name is used determine file type,
		            either yaml or text/csv.

		lines:		All non-newline-terminated lines of text from the dts file.

        word_elements:       Map of ARINC 429 word name to vector of all elements
                                associated with it (as ICDElment)

		return:		True if success, false if failure.

	*/
    virtual bool IngestLines(const std::vector<std::string>& lines,
                     std::unordered_map<std::string, std::vector<ICDElement>>& word_elements);

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
    Perform validation of the root yaml node, which maps to
    'translatable_word_definitions' and 'supplemental_bus_map_labels'

    Args:
        root_node       --> YAML::Node to which word defs and supplemental
                            bus map labels maps.

    Return:
        True if root_node passes validation; false otherwise
    */
    bool ValidateRootNode(const YAML::Node& root_node);

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

};

#endif
