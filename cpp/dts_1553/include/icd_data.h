#ifndef ICD_DATA_H
#define ICD_DATA_H

#include <map>
#include <set>
#include <cstdint>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>
#include "spdlog/spdlog.h"
#include "icd_element.h"
#include "parse_text.h"
#include "uri_percent_encoding.h"
#include "managed_path.h"
#include "iterable_tools.h"
#include "yaml_reader.h"
#include "yaml-cpp/yaml.h"

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

    // Count of valid messages ingested from the DTS yaml or csv.
    size_t valid_message_count_;

    // Map a unique index to a vector of indices to ICDElement objects
    // in icd_elements_ that groups elements into tables.
    // The final lookup returns the unique index which is
    // the key in this table.
    std::vector<std::vector<size_t>> tables_;

    // Map of bus name to the set of LRU addresses which are present
    // on the bus.
    std::map<std::string, std::set<uint64_t>> bus_name_to_lru_addrs_map_;

    // For conversion of message and element names to percent-encoded strings
    URIPercentEncoding uri_percent_encode_;

    // Yaml icd members.
    std::vector<std::string> yaml_msg_body_keys_;
    std::vector<std::string> yaml_msg_data_keys_;
    std::vector<std::string> yaml_word_elem_keys_;
    std::vector<std::string> yaml_bit_elem_keys_;
    const size_t icd_sequence_size_ = 2;

    // Vectors to hold data retrieved from sequence nodes.
    std::vector<int> temp_comm_word_vec_;
    std::vector<std::string> temp_lru_name_vec_;
    std::vector<int> temp_lru_addr_vec_;
    std::vector<int> temp_lru_subaddr_vec_;

    // Hold data retrieved from mapped parameters.
    int temp_int_;
    float temp_float_;

    /*
	* Private high-level functions.
	*/
    void MapICDElementSchemaToString();
    bool CreateLookupMap();

   public:
    const size_t& valid_message_count;
    const std::vector<std::string>& table_names;

    ICDData();
    ~ICDData();
    bool PrepareICDQuery(const std::vector<std::string>& lines);
    bool PrepareICDQuery(const YAML::Node& msg_defs_node,
                         std::map<std::string, std::string>& msg_name_substitutions,
                         std::map<std::string, std::string>& elem_name_substitutions);

    void PrepareMessageKeyMap(std::unordered_map<uint64_t, std::set<std::string>>& message_key_map,
                              const std::map<std::string, std::set<uint64_t>>& supplemental_map);
    std::vector<std::vector<size_t>> GetTableOrganizationIndices();
    std::set<size_t> TempLookupTableIndex(const std::string& bus_name, uint8_t xmit_lru_addr,
                                          uint8_t dest_lru_addr, uint8_t xmit_lru_subaddr, uint8_t dest_lru_subaddr);
    std::set<size_t> LookupTableIndex(uint16_t chanid, uint8_t xmit_lru_addr,
                                      uint8_t dest_lru_addr, uint8_t xmit_lru_subaddr, uint8_t dest_lru_subaddr);
    std::string LookupTableNameByIndex(size_t index) const;
    bool ReplaceBusNameWithChannelIDInLookup(const std::map<std::string, std::set<uint64_t>>& input_map);
    std::map<std::string, std::set<uint64_t>> GetBusNameToLRUAddrsMap();
    const std::vector<size_t>& GetTableElementIndices(const size_t& table_index) const;
    const ICDElement& GetElementByIndex(const size_t& table_index) const;
    const ICDElement* GetElementPointerByIndex(const size_t& table_index);

    /*
    Get the index of the table name in the vector of names.

    Args:
        table_name      --> Name of table for which index is desired
        all_table_names --> Vector of table names.

    Return:
        std::string::npos if table name is not found, and the index
        of the table name within the vector otherwise.
    */
    size_t GetTableIndexByName(const std::string& table_name,
                               const std::vector<std::string>& all_table_names);

    /*
    Get a set of indices of the tables which are represented by
    a set of message names. 

    Args:
        selected_msg_names  --> Set of string message names which 
                                will be used to create a set of indices.
                                Each of the names is mapped to a table
                                represented by the corresponding index in tables_.
        all_table_names     --> Vector of table names from which indices
                                will be determined.
    
    Return:
        A set of indices which map to tables in the tables_ vector. 
        Only message names which are found in the table_names_ vector
        will be included.
    */
    std::set<size_t> GetSelectedTableIndicesSet(const std::set<std::string>& selected_msg_names,
                                                const std::vector<std::string>& all_table_names);

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
    bool IsYamlFile(const ManagedPath& icd_path);
    bool IsYamlFile(const std::string& icd_path);
    bool IngestICDYamlNode(const YAML::Node& root_node,
                           std::vector<ICDElement>& icd_elems_output,
                           std::map<std::string, std::string>& msg_name_substitutions,
                           std::map<std::string, std::string>& elem_name_substitutions);
    bool MapNodeHasRequiredKeys(const YAML::Node& node,
                                const std::vector<std::string>& required_keys);
    size_t FillElementsFromYamlNodes(const std::string& msg_name, const YAML::Node& msg_data_node,
                                     const YAML::Node& elem_node, bool is_bit_node,
                                     std::vector<ICDElement>& icd_elems,
                                     std::map<std::string, std::string>& elem_name_subs);
    bool CreateVectorOfStringICDComponents(const std::string& msg_name, const YAML::Node& msg_data_node,
                                           const std::string& elem_name, const YAML::Node& elem_data_node, bool is_bit_elem,
                                           std::vector<std::string>& output_vec);
    bool SequenceNodeHasCorrectSize(const YAML::Node& node, const size_t& required_size);
    bool ICDSchemaStringToEnum(const std::string& schema_string, ICDElementSchema& schema);

    /*
	Set the ICDElement data member values from Yaml nodes which represent
    a word element.

	Args:
        icd_elem        --> ICDElement object to be configured
		msg_name		--> Name of message to which the ICD Element
							is associated
		msg_data_node   --> Yaml::Node which satisfies the criterion that
                            MapNodeHasRequiredKeys(msg_data_node, yaml_msg_data_keys_)
                            == true
        word_elem_name  --> Name of element 
        word_elem_node  --> Yaml::Node with word element data which satisfies
                            the criteriion that 
                            MapNodeHasRequiredKeys(word_elem_node, yaml_word_elem_keys_)
                            == true
    
    Return:
        True if all required member variables of the icd_elem can be set. False
        if a required key-value pair is missing or a data component can't be 
        converted or casted as expected.
	*/
    bool ConfigureWordElementFromYamlNodes(ICDElement& icd_elem, const std::string& msg_name,
                                           const YAML::Node& msg_data_node,
                                           const std::string& word_elem_name,
                                           const YAML::Node& word_elem_node);

    /*
	Set the ICDElement data member values from Yaml nodes which represent
    a bit element.

	Args:
        icd_elem        --> ICDElement object to be configured
		msg_name		--> Name of message to which the ICD Element
							is associated
		msg_data_node   --> Yaml::Node which satisfies the criterion that
                            MapNodeHasRequiredKeys(msg_data_node, yaml_msg_data_keys_)
                            == true
        bit_elem_name  --> Name of element 
        bit_elem_node  --> Yaml::Node with bit element data which satisfies
                            the criteriion that 
                            MapNodeHasRequiredKeys(bit_elem_node, yaml_bit_elem_keys_)
                            == true
    
    Return:
        True if all required member variables of the icd_elem can be set. False
        if a required key-value pair is missing or a data component can't be 
        converted or casted as expected.
	*/
    bool ConfigureBitElementFromYamlNodes(ICDElement& icd_elem, const std::string& msg_name,
                                          const YAML::Node& msg_data_node,
                                          const std::string& bit_elem_name,
                                          const YAML::Node& bit_elem_node);

    /*
    Set the ICDElement data member values that pertain to message-level
    data from a message data node.

	Args:
        icd_elem        --> ICDElement object to be configured
		msg_name		--> Name of message to which the ICD Element
							is associated
		msg_data_node   --> Yaml::Node which satisfies the criterion that
                            MapNodeHasRequiredKeys(msg_data_node, yaml_msg_data_keys_)
                            == true

    Return:
        True if all required fields of the msg_data_node are present and
        can be casted to the correct values. False if a required field is
        not present or any field fails to be casted.
    */
    bool ConfigureMsgDataFromYamlNode(ICDElement& icd_elem, const std::string& msg_name,
                                      const YAML::Node& msg_data_node);

    /*
    Set the ICDElement data member values that pertain to word-level
    data from a word element node.

	Args:
        icd_elem        --> ICDElement object to be configured
		word_elem_node  --> Yaml::Node which satisfies the criterion that
                            MapNodeHasRequiredKeys(word_elem_node, yaml_word_elem_keys_)
                            == true
        word_elem_name  --> Name of element 
                           

    Return:
        True if all required fields of the word_elem_node are present and
        can be casted to the correct values. False if a required field is
        not present or any field fails to be casted.
    */
    bool ConfigureWordElemDataFromYamlNode(ICDElement& icd_elem,
                                           const YAML::Node& word_elem_node,
                                           const std::string& word_elem_name);

    /*
    Set the ICDElement data member values that pertain to bit-level
    data from a bit element node.

	Args:
        icd_elem        --> ICDElement object to be configured
		bit_elem_node  --> Yaml::Node which satisfies the criterion that
                            MapNodeHasRequiredKeys(bit_elem_node, yaml_bit_elem_keys_)
                            == true
        bit_elem_name  --> Name of element 
                           

    Return:
        True if all required fields of the bit_elem_node are present and
        can be casted to the correct values. False if a required field is
        not present or any field fails to be casted.
    */
    bool ConfigureBitElemDataFromYamlNode(ICDElement& icd_elem,
                                          const YAML::Node& bit_elem_node,
                                          const std::string& bit_elem_name);

    /*
    Fill an output variable from a yaml node using the parameter name as key. 
    If the parameter name is not in a vector of required parameters, then the 
    key and value are not required to be present.

    Args:
        node            --> YAML::Node from which key/val data are retrieved 
        param_name      --> Name of the parameter which is being retrieved
        required_params --> Vector of parameter names. Inclusion of a name in 
                            the vector implies that the parameter is required.
        output          --> Variable into which will be placed the mapped value

    Return:
        True if the mapped value is present and can be cast to the data type 
        of the output variable or the param_name is not in required_params. 
        False otherwise.
    */
    template <typename T>
    bool GetMappedValueFromNode(const YAML::Node& node, std::string param_name,
                                const std::vector<std::string> required_params,
                                T& output);

    /*
    Fill an output vector from a yaml sequence node. 
    If the parameter name is not in a vector of required parameters, then the 
    key and value are not required to be present.

    Args:
        node            --> YAML::Node which contains a sequence which is 
                            mapped by the key param_name. The sequence which
                            is mapped contains the values that will fill
                            the output vector.
        param_name      --> Name of the parameter which is being retrieved
        required_params --> Vector of parameter names. Inclusion of a name in 
                            the vector implies that the parameter is required.
        output          --> Vector into which will be placed the mapped value

    Return:
        True if the mapped node is present and is a sequence node and 
        the values in the sequence can be casted to the data type of 
        the output vector OR the param_name is not in required_params. 
        False otherwise.
    */
    template <typename T>
    bool GetSequenceValuesFromNode(const YAML::Node& node, std::string param_name,
                                   const std::vector<std::string> required_params,
                                   std::vector<T>& output);
};

template <typename T>
bool ICDData::GetMappedValueFromNode(const YAML::Node& node, std::string param_name,
                                     const std::vector<std::string> required_params,
                                     T& output)
{
    if (!YamlReader::GetMapNodeParameter(node, param_name, output))
    {
        SPDLOG_ERROR("Error obtaining mapped value with parameter name \"{:s}\"",
            param_name);
        if (std::find(required_params.begin(), required_params.end(), param_name) != required_params.end())
            return false;
    }
    return true;
}

template <typename T>
bool ICDData::GetSequenceValuesFromNode(const YAML::Node& node, std::string param_name,
                                        const std::vector<std::string> required_params,
                                        std::vector<T>& output)
{
    output.clear();
    if (node[param_name])
    {
        if (!YamlReader::GetSequenceNodeVector(node[param_name], output))
        {
            SPDLOG_ERROR("Error obtaining sequence from \"{:s}\" key", param_name);
            return false;
        }
    }
    else
    {
        if (std::find(required_params.begin(), required_params.end(), param_name) != required_params.end())
            return false;
    }

    return true;
}

#endif
