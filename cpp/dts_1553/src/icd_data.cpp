#include "icd_data.h"

ICDData::ICDData() : icd_ingest_success_(false), organize_icd_success_(false), iter_tools_(), yaml_msg_body_keys_({"msg_data", "word_elem", "bit_elem"}), yaml_msg_data_keys_({"lru_addr", "lru_subaddr", "bus", "wrdcnt", "rate"}), yaml_word_elem_keys_({"offset", "cnt", "schema", "msbval"}), yaml_bit_elem_keys_({"offset", "cnt", "schema", "msbval", "msb", "lsb", "bitcnt"}), valid_message_count_(0), valid_message_count(valid_message_count_), uri_percent_encode_(), table_names(table_names_)
{
    MapICDElementSchemaToString();
}

ICDData::~ICDData()
{
}

bool ICDData::ICDSchemaStringToEnum(const std::string& schema_string, ICDElementSchema& schema)
{
    //printf("schema_string = %s\n", schema_string.c_str());
    if (string_to_icdschema_map_.count(schema_string) == 1)
    {
        schema = string_to_icdschema_map_[schema_string];
        return true;
    }
    schema = ICDElementSchema::BAD;
    return false;
}

void ICDData::MapICDElementSchemaToString()
{
    icdschema_to_string_map_[ICDElementSchema::MULTIPLE_BIT] = "MULTIPLE_BIT";
    icdschema_to_string_map_[ICDElementSchema::MODE_CODE] = "MODE_CODE";
    icdschema_to_string_map_[ICDElementSchema::SIGNED16] = "SIGNED16";
    icdschema_to_string_map_[ICDElementSchema::SIGNED32] = "SIGNED32";
    icdschema_to_string_map_[ICDElementSchema::UNSIGNED16] = "UNSIGNED16";
    icdschema_to_string_map_[ICDElementSchema::UNSIGNED32] = "UNSIGNED32";
    icdschema_to_string_map_[ICDElementSchema::FLOAT32_1750] = "FLOAT32_1750";
    icdschema_to_string_map_[ICDElementSchema::FLOAT32_IEEE] = "FLOAT32_IEEE";
    icdschema_to_string_map_[ICDElementSchema::FLOAT64_IEEE] = "FLOAT64_IEEE";
    icdschema_to_string_map_[ICDElementSchema::FLOAT16] = "FLOAT16";
    icdschema_to_string_map_[ICDElementSchema::ASCII] = "ASCII";
    icdschema_to_string_map_[ICDElementSchema::SIGNEDBITS] = "SIGNEDBITS";
    icdschema_to_string_map_[ICDElementSchema::UNSIGNEDBITS] = "UNSIGNEDBITS";
    icdschema_to_string_map_[ICDElementSchema::BAD] = "BAD";
    icdschema_to_string_map_[ICDElementSchema::CAPS] = "CAPS";
    icdschema_to_string_map_[ICDElementSchema::FLOAT32_GPS] = "FLOAT32_GPS";
    icdschema_to_string_map_[ICDElementSchema::FLOAT64_GPS] = "FLOAT64_GPS";

    bool lost_entries = false;
    string_to_icdschema_map_ = iter_tools_.ReverseMap(icdschema_to_string_map_, lost_entries);
}

bool ICDData::IngestICDTextFileLines(const std::vector<std::string>& lines,
                                     std::vector<ICDElement>& icd_elems_output)
{
    // Return false if empty.
    if (lines.size() == 0)
    {
        SPDLOG_WARN("input vector size is zero");
        return false;
    }

    // Return false if destination vector has non-zero element count.
    if (icd_elems_output.size() > 0)
    {
        SPDLOG_WARN("output vector size is non-zero");
        return false;
    }

    // Count the columns (comma-separated) in the first line.
    ParseText pt;
    std::vector<std::string> col_hdr = pt.Split(lines[0], ',');
    if (col_hdr.size() != ICDElement::kFillElementCount)
    {
        std::string split_l0 = iter_tools_.GetIterablePrintString(col_hdr,
                                                                  "First line, split on \',\'", "%s", ",");
        SPDLOG_WARN(
            "first line column count "
            "{:d} (must be {:d})", col_hdr.size(), ICDElement::kFillElementCount);
        SPDLOG_WARN("{:s}", split_l0);
        return false;
    }

    // Check the first two header column names. Should be
    // "msg_name,elem_name,..."
    if (col_hdr[0] != "msg_name" || col_hdr[1] != "elem_name")
    {
        SPDLOG_WARN(
            "the first two column header names must be \"msg_name\" and \"elem_name\"");
        return false;
    }

    // Iterate over all the lines and feed to ICD elements.
    bool fill_result = false;
    int fill_count = 0;
    for (size_t i = 1; i < lines.size(); i++)
    {
        ICDElement temp_icd_elem;
        fill_result = temp_icd_elem.Fill(lines[i]);
        if (!fill_result)
        {
            SPDLOG_ERROR("Failed to fill ICD Element with line: {:s}", lines[i]);
        }
        else
        {
            icd_elems_output.push_back(temp_icd_elem);
            fill_count++;
        }
    }

    // Somewhat arbitrary check on fill count.
    int required_fill_count = 100;
    if (fill_count < required_fill_count)
    {
        SPDLOG_INFO("Fill count < {:d}", required_fill_count);
    }
    SPDLOG_INFO("Fill count = {:d}", fill_count);

    return true;
}

std::vector<ICDElement> ICDData::GetICDElementVector()
{
    return icd_elements_;
}

bool ICDData::PrepareICDQuery(const std::vector<std::string>& lines)
{
    // Ingest ICD text.
    icd_ingest_success_ = IngestICDTextFileLines(lines, icd_elements_);
    if (!icd_ingest_success_)
    {
        SPDLOG_ERROR("IngestICDTextFileLines() failure. Cannot proceed to PrepareICDQuery()");
        return false;
    }

    // Organize ICDElements into table --> column structure.
    if (!CreateTables(icd_elements_, tables_))
    {
        return false;
    }

    if (!CollectMsgElementsFromTableInds(icd_elements_, tables_, icd_msg_elements_))
    {
        return false;
    }

    if (!CreateTableNameLookup(icd_msg_elements_, table_names_))
    {
        return false;
    }

    if (!CreateLookupMap())
    {
        return false;
    }

    organize_icd_success_ = true;
    return true;
}

bool ICDData::PrepareICDQuery(const YAML::Node& msg_defs_node,
                              std::map<std::string, std::string>& msg_name_substitutions,
                              std::map<std::string, std::string>& elem_name_substitutions)
{
    icd_ingest_success_ = IngestICDYamlNode(msg_defs_node, icd_elements_,
                                            msg_name_substitutions, elem_name_substitutions);
    if (!icd_ingest_success_)
    {
        SPDLOG_ERROR("IngestICDYamlFileLines() failure. Cannot proceed to PrepareICDQuery()");
        return false;
    }

    // Organize ICDElements into table --> column structure.
    if (!CreateTables(icd_elements_, tables_))
    {
        return false;
    }

    if (!CollectMsgElementsFromTableInds(icd_elements_, tables_, icd_msg_elements_))
    {
        return false;
    }

    if (!CreateTableNameLookup(icd_msg_elements_, table_names_))
    {
        return false;
    }

    if (!CreateLookupMap())
    {
        return false;
    }

    organize_icd_success_ = true;
    return true;
}

void ICDData::PrepareMessageKeyMap(std::unordered_map<uint64_t, std::set<std::string>>& message_key_map,
                                   const std::map<std::string, std::set<uint64_t>>& supplemental_map)
{
    uint64_t message_key;
    for (int i = 0; i < icd_elements_.size(); i++)
    {
        // The message key consists of the transmit command word
        // left shifted 16 bits and bitwise ORd with the recieve
        // command word
        message_key = (icd_elements_[i].xmit_word_ << 16) |
                      icd_elements_[i].dest_word_;

        if (message_key_map.count(message_key) == 0)
        {
            std::set<std::string> temp_set;
            temp_set.insert(icd_elements_[i].bus_name_);
            message_key_map[message_key] = temp_set;
        }
        else
        {
            message_key_map[message_key].insert(icd_elements_[i].bus_name_);
        }
    }

    // Append supplemental command words
    for (std::map<std::string, std::set<uint64_t>>::const_iterator it = supplemental_map.begin();
         it != supplemental_map.end();
         ++it)
    {
        for (auto key : it->second)
        {
            if (message_key_map.count(key) == 0)
            {
                std::set<std::string> temp_set;
                temp_set.insert(it->first);
                message_key_map[key] = temp_set;
            }
            else
            {
                message_key_map[key].insert(it->first);
            }
        }
    }
}

std::vector<std::vector<size_t>> ICDData::GetTableOrganizationIndices()
{
    return tables_;
}

bool ICDData::CreateTables(const std::vector<ICDElement>& icd_elems,
                           std::vector<std::vector<size_t>>& output_organized_inds)
{
    if (icd_elems.size() == 0)
        return false;

    // Obtain a vector of ICDElement.msg_name_.
    std::vector<std::string> msg_name_vec = iter_tools_.VectorOfMember(icd_elems,
                                                                       &ICDElement::msg_name_);

    // Find the unique values in msg_name_vec.
    std::vector<std::string> msg_name_unique = iter_tools_.UniqueElements(msg_name_vec);

    // The size of unique messages ought to equal valid_message_count_ if the
    // input ICD format is yaml. If the input format is CSV, then valid_message_count_
    // hasn't been determined yet and it will be set the size of unique message names.
    if (valid_message_count_ != 0)
    {
        if (msg_name_unique.size() != valid_message_count_)
        {
            SPDLOG_ERROR(
                "valid_message_count_ = {:d}, size of "
                "msg_name_unique = {:d}. Must be equal to continue.",
                valid_message_count_, msg_name_unique.size());
            return false;
        }
    }

    valid_message_count_ = msg_name_unique.size();

    // Iterate over the unique message names and find the indices of all matching
    // elements in msg_name_vec. Place the indices in the tables_ vector. A
    // collection (vector) of indices groups elements into a table.
    std::vector<size_t> matching_inds;
    for (size_t i = 0; i < msg_name_unique.size(); i++)
    {
        matching_inds = iter_tools_.IndicesOfMatching(msg_name_vec, msg_name_unique[i]);
        if (matching_inds.size() > 0)
            output_organized_inds.push_back(matching_inds);
    }

    return true;
}

bool ICDData::CollectMsgElementsFromTableInds(const std::vector<ICDElement>& all_icd_elements,
                                              const std::vector<std::vector<size_t>>& table_inds,
                                              std::vector<ICDElement>& output_icd_message_elements)
{
    // For each grouping of indices in tables_, use a single element
    // to determine the message identification parameters.

    // Create a new vector of ICDElements with only one entry per message.
    // The vector created here determines the index for all subsequent
    // lookups. The index can be used to locate an ICD element containing
    // the message information and index to the name of the message.
    //
    // When tables are created prior to translation, use this index
    // to identify each table.

    // Return false if input elements vector or table inds vector
    // are zero size.
    if (all_icd_elements.size() == 0 || table_inds.size() == 0)
        return false;

    // Return false if output message elements already has data.
    if (output_icd_message_elements.size() > 0)
        return false;

    output_icd_message_elements.resize(table_inds.size());
    for (size_t i = 0; i < table_inds.size(); i++)
    {
        output_icd_message_elements[i] = all_icd_elements[table_inds[i][0]];
    }

    return true;
}

bool ICDData::CreateTableNameLookup(const std::vector<ICDElement>& icd_message_elements,
                                    std::vector<std::string>& output_table_names)
{
    // Input elements must have nonzero size.
    if (icd_message_elements.size() == 0)
        return false;

    // Output table must have zero elements.
    if (output_table_names.size() > 0)
        return false;

    // Create the table name lookup. Note that it is a lookup in the sense
    // that the index of the matching table can be used to index into the
    // resulting vector to obtain the message name.
    output_table_names = iter_tools_.VectorOfMember(icd_message_elements, &ICDElement::msg_name_);

    // Check if the table_names_ are unique.
    std::vector<std::string> unique_names = iter_tools_.UniqueElements(output_table_names);
    if (output_table_names.size() != unique_names.size())
        return false;

    return true;
}

std::string ICDData::LookupTableNameByIndex(size_t index) const
{
    if (table_names_.size() == 0 || index > table_names_.size() - 1)
        return std::string("");
    else
        return table_names_[index];
}

size_t ICDData::GetTableIndexByName(const std::string& table_name,
                                    const std::vector<std::string>& all_table_names)
{
    std::vector<size_t> match_inds = iter_tools_.IndicesOfMatching(all_table_names, table_name);

    // Return npos if either there are zero or more than one matching index.
    if (match_inds.size() != 1)
        return std::string::npos;
    return match_inds[0];
}

std::set<size_t> ICDData::GetSelectedTableIndicesSet(const std::set<std::string>& selected_msg_names,
                                                     const std::vector<std::string>& all_table_names)
{
    std::set<size_t> indices_set;
    size_t temp_ind;
    for (std::set<std::string>::const_iterator it = selected_msg_names.cbegin();
         it != selected_msg_names.cend(); ++it)
    {
        temp_ind = GetTableIndexByName(*it, all_table_names);
        if (temp_ind != std::string::npos)
            indices_set.insert(temp_ind);
    }
    return indices_set;
}

bool ICDData::CreateLookupMap()
{
    bus_to_elem_map::const_iterator it;
    xlru_to_elem_map::const_iterator it2;
    dlru_to_elem_map::const_iterator it3;
    xsub_to_elem_map::const_iterator it4;
    xlru_to_elem_map xlru_elem_map;
    dlru_to_elem_map dlru_elem_map;
    xsub_to_elem_map xsub_elem_map;
    dsub_to_elem_map dsub_elem_map;

    // Group message elements by bus name.
    bus_to_elem_map bus_elem_map =
        iter_tools_.GroupByMember(icd_msg_elements_, &ICDElement::bus_name_);

    // Loop over groups of message elements, and find further grouping.
    for (it = bus_elem_map.begin(); it != bus_elem_map.end(); ++it)
    {
        // Create a map that groups the elements by the xmit LRU addresses.
        xlru_elem_map = iter_tools_.GroupByMember(it->second, &ICDElement::xmit_lru_addr_);
        if (xlru_elem_map.size() > 0)
        {
            xlru_to_dlru_to_xsub_to_dsub_to_inds xlru_dlru_xsub_dsub_inds_map;
            for (it2 = xlru_elem_map.begin(); it2 != xlru_elem_map.end(); ++it2)
            {
                dlru_elem_map = iter_tools_.GroupByMember(it2->second, &ICDElement::dest_lru_addr_);
                if (dlru_elem_map.size() > 0)
                {
                    dlru_to_xsub_to_dsub_to_inds dlru_xsub_dsub_inds_map;
                    for (it3 = dlru_elem_map.begin(); it3 != dlru_elem_map.end(); ++it3)
                    {
                        xsub_elem_map = iter_tools_.GroupByMember(
                            it3->second, &ICDElement::xmit_lru_subaddr_);
                        if (xsub_elem_map.size() > 0)
                        {
                            xsub_to_dsub_to_inds xsub_dsub_inds_map;
                            for (it4 = xsub_elem_map.begin(); it4 != xsub_elem_map.end(); ++it4)
                            {
                                dsub_elem_map = iter_tools_.GroupByMember(
                                    it4->second, &ICDElement::dest_lru_subaddr_);
                                if (dsub_elem_map.size() > 0)
                                {
                                    // Find the index of each of the ICDElements in
                                    // the mapped vector. Replace each vector of ICDElements
                                    // with a vector of the matching indices. Fill a new
                                    // map (dsub_inds_map) with the key and new indices vector.
                                    dsub_to_inds_map dsub_inds_map;
                                    UpdateMapFromICDElementsToIndices(dsub_elem_map, table_names_,
                                                                      dsub_inds_map);

                                    // Add the map to the greater map.
                                    xsub_dsub_inds_map[it4->first] = dsub_inds_map;
                                }
                            }
                            if (xsub_dsub_inds_map.size() > 0)
                                dlru_xsub_dsub_inds_map[it3->first] = xsub_dsub_inds_map;
                        }
                    }
                    if (dlru_xsub_dsub_inds_map.size() > 0)
                        xlru_dlru_xsub_dsub_inds_map[it2->first] = dlru_xsub_dsub_inds_map;
                }
            }
            if (xlru_dlru_xsub_dsub_inds_map.size() > 0)
                icd_temp_lookup_[it->first] = xlru_dlru_xsub_dsub_inds_map;
        }
    }
    return true;
}

void ICDData::UpdateMapFromICDElementsToIndices(const dsub_to_elem_map& dsub_elem_map,
                                                const std::vector<std::string>& table_names, dsub_to_inds_map& dsub_inds_map)
{
    size_t match_ind;
    std::vector<std::string> current_elem_names;
    dsub_to_elem_map::const_iterator it;
    size_t i = 0;
    for (it = dsub_elem_map.begin(); it != dsub_elem_map.end(); ++it)
    {
        // Get a vector of the element names from the vector of ICDElements.
        current_elem_names = iter_tools_.VectorOfMember(it->second, &ICDElement::msg_name_);

        // Loop over each of the element names.
        table_inds_vec all_inds;
        for (i = 0; i < current_elem_names.size(); i++)
        {
            // Get the indices of the matching element names in the input elem_name_vec.
            match_ind = iter_tools_.IndexOf(table_names, current_elem_names[i]);

            if (match_ind == std::string::npos)
            {
                SPDLOG_ERROR("match_inds size != 1!");
                return;
            }
            else
            {
                all_inds.insert(match_ind);
            }
        }
        // Record an entry in the output map, dsub_inds_map, if
        // all_inds has at lease one entry.
        if (all_inds.size() > 0)
            dsub_inds_map[it->first] = all_inds;
        else
        {
            SPDLOG_ERROR("all_inds size == 0!");
            return;
        }
    }
}

std::set<size_t> ICDData::TempLookupTableIndex(const std::string& bus_name,
                                               uint8_t xmit_lru_addr, uint8_t dest_lru_addr, uint8_t xmit_lru_subaddr,
                                               uint8_t dest_lru_subaddr)
{
    try
    {
        temp_table_inds_ = icd_temp_lookup_[bus_name][xmit_lru_addr][dest_lru_addr][xmit_lru_subaddr][dest_lru_subaddr];
    }
    catch (const std::out_of_range& oor)
    {
        
        temp_table_inds_ = table_inds_vec();
    }
    return temp_table_inds_;
}

std::set<size_t> ICDData::LookupTableIndex(uint16_t chanid, uint8_t xmit_lru_addr,
                                           uint8_t dest_lru_addr, uint8_t xmit_lru_subaddr, uint8_t dest_lru_subaddr)
{
    try
    {
        temp_table_inds_ = icd_lookup_[chanid][xmit_lru_addr][dest_lru_addr][xmit_lru_subaddr][dest_lru_subaddr];
    }
    catch (const std::out_of_range& oor)
    {
        temp_table_inds_ = table_inds_vec();
    }
    return temp_table_inds_;
}

bool ICDData::ReplaceBusNameWithChannelIDInLookup(const std::map<std::string,
                                                                 std::set<uint64_t>>& input_map)
{
    if (!organize_icd_success_)
        return false;
    else
    {
        // Get the keys of the temp lookup map.
        std::vector<std::string> temp_lookup_keys = iter_tools_.GetKeys(icd_temp_lookup_);

        // Iterate over the keys. If a key is present in the input_map, then
        // add an entry to the icd_lookup_ map with the input_map value as the
        // the key.
        std::set<uint64_t> chanid_set;
        std::set<uint64_t>::const_iterator setit;
        for (std::vector<std::string>::const_iterator it = temp_lookup_keys.begin();
             it != temp_lookup_keys.end(); ++it)
        {
            if (iter_tools_.IsKeyInMap(input_map, *it))
            {
                // Create a lookup entry for each channel ID in the
                // value set.
                chanid_set = input_map.at(*it);
                for (setit = chanid_set.begin(); setit != chanid_set.end(); ++setit)
                {
                    icd_lookup_[static_cast<uint16_t>(*setit)] = icd_temp_lookup_.at(*it);
                }
            }
        }

        // If the icd_lookup_ has zero entries, then it can't be queried
        // at translate time.
        if (icd_lookup_.size() == 0)
            return false;
    }
    return true;
}

std::vector<uint16_t> ICDData::GetLookupTableChannelIDKeys()
{
    return iter_tools_.GetKeys(icd_lookup_);
}

bool ICDData::CreateBusNameToLRUAddressSetMap(
    const std::vector<ICDElement>& icd_message_elements,
    std::map<std::string, std::set<uint64_t>>& output_bus_to_lru_addrs_map)
{
    // Input icd elements must have nonzero size and
    // output map must have zero size.
    if (icd_message_elements.size() == 0)
        return false;

    if (output_bus_to_lru_addrs_map.size() > 0)
        return false;

    // Use the vector of ICDElements that correspond a unique
    // set of messages. Note that the elements within the vector
    // are arbitrary in which message element is represented. The
    // only requirement is that the message to which each element
    // belongs is unique in the vector and that the elements of
    // the vector represent every message present in the complete ICD.

    // Group the elements in the vector of message elements by the bus
    // name.
    std::unordered_map<std::string, icdelem_vec> group_by_bus = iter_tools_.GroupByMember(
        icd_message_elements, &ICDElement::bus_name_);

    // For each bus group, get the vector of transmitting LRU addresses
    // and the vector of destination LRU addresses and combine them into
    // a set of unique values.
    //
    // Insert the bus name and set of LRU addresses into the map.

    std::unordered_map<std::string, icdelem_vec>::const_iterator it;
    std::vector<uint8_t> xmit_addrs;
    std::vector<uint8_t> dest_addrs;
    std::set<uint64_t> unique_addrs;
    for (it = group_by_bus.begin(); it != group_by_bus.end(); ++it)
    {
        xmit_addrs = iter_tools_.VectorOfMember(it->second, &ICDElement::xmit_lru_addr_);
        dest_addrs = iter_tools_.VectorOfMember(it->second, &ICDElement::dest_lru_addr_);

        // Reset the set to zero.
        unique_addrs.clear();

        // Add both vector of LRU addresses to the set.
        unique_addrs.insert(xmit_addrs.begin(), xmit_addrs.end());
        unique_addrs.insert(dest_addrs.begin(), dest_addrs.end());

        // Add the bus name, LRU addresses set to the map.
        output_bus_to_lru_addrs_map[it->first] = unique_addrs;
    }

    return true;
}

std::map<std::string, std::set<uint64_t>> ICDData::GetBusNameToLRUAddrsMap()
{
    return bus_name_to_lru_addrs_map_;
}

const std::vector<size_t>& ICDData::GetTableElementIndices(const size_t& table_index) const
{
    return tables_[table_index];
}

const ICDElement& ICDData::GetElementByIndex(const size_t& table_index) const
{
    return icd_elements_[table_index];
}

const ICDElement* ICDData::GetElementPointerByIndex(const size_t& index)
{
    return &icd_elements_[index];
}

bool ICDData::IsYamlFile(const ManagedPath& icd_path)
{
    // Get the extension and cast to lower case.
    std::string extension = icd_path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Does it match yaml or yml?
    if (extension == ".yaml" || extension == ".yml")
        return true;
    else
        return false;
}

bool ICDData::IsYamlFile(const std::string& icd_path)
{
    ManagedPath mp(icd_path);
    return IsYamlFile(mp);
}

bool ICDData::IngestICDYamlNode(const YAML::Node& root_node,
                                std::vector<ICDElement>& icd_elems_output,
                                std::map<std::string, std::string>& msg_name_substitutions,
                                std::map<std::string, std::string>& elem_name_substitutions)
{
    // Iterate over all root-level maps, where each map is a message name to the
    // the message body data.
    size_t element_count = 0;
    valid_message_count_ = 0;
    std::string msg_name = "";
    std::string perc_enc_msg_name = "";
    std::string elem_name = "";
    YAML::Node msg_data_node;
    YAML::Node word_elem_node;
    YAML::Node bit_elem_node;
    bool is_bit = false;
    for (YAML::const_iterator it = root_node.begin(); it != root_node.end(); ++it)
    {
        msg_name = it->first.as<std::string>();
        if (!uri_percent_encode_.PercentEncodeReservedASCIIChars(msg_name, perc_enc_msg_name))
        {
            SPDLOG_ERROR("msg_name is not ASCII: {:s}", msg_name);
            return false;
        }

        if (msg_name.compare(perc_enc_msg_name) != 0)
            msg_name_substitutions[msg_name] = perc_enc_msg_name;

        //printf("Ingesting message: %s\n", msg_name.c_str());

        // Confirm that message body has required keys.
        if (!MapNodeHasRequiredKeys(it->second, yaml_msg_body_keys_))
        {
            SPDLOG_ERROR("Message {:s} body is not a map does not have required keys!",
                perc_enc_msg_name);
            continue;  // or exit?
        }

        // Confirm that key = msg_data maps to map of correct keys.
        msg_data_node = it->second["msg_data"];
        if (!MapNodeHasRequiredKeys(msg_data_node, yaml_msg_data_keys_))
        {
            SPDLOG_ERROR(
                "Message {:s} msg_data node is not a map or does not have required keys!",
                perc_enc_msg_name);
            continue;
        }

        // Fill word elements.
        word_elem_node = it->second["word_elem"];
        is_bit = false;
        element_count += FillElementsFromYamlNodes(perc_enc_msg_name, msg_data_node, word_elem_node,
                                                   is_bit, icd_elems_output, elem_name_substitutions);

        // Fill bit elements.
        bit_elem_node = it->second["bit_elem"];
        is_bit = true;
        element_count += FillElementsFromYamlNodes(perc_enc_msg_name, msg_data_node, bit_elem_node,
                                                   is_bit, icd_elems_output, elem_name_substitutions);

        // Increment valid message count.
        valid_message_count_++;
    }
    SPDLOG_INFO("Ingested {:d} ICD messages", valid_message_count_);
    SPDLOG_INFO("Ingested {:d} ICD elements", element_count);

    return true;
}

bool ICDData::MapNodeHasRequiredKeys(const YAML::Node& node,
                                     const std::vector<std::string>& required_keys)
{
    // Node must be a map.
    if (!node.IsMap())
    {
        SPDLOG_ERROR("Node is not map!");
        return false;
    }

    // Create a map for the required keys.
    std::unordered_map<std::string, bool> keys_present;
    std::vector<std::string>::const_iterator it;
    for (it = required_keys.begin(); it != required_keys.end(); ++it)
    {
        keys_present[*it] = false;
    }

    // Iterate over node, marking if keys are present.
    std::string node_key = "";
    bool current_state = false;
    size_t present_count = 0;
    for (YAML::Node::const_iterator it = node.begin(); it != node.end(); ++it)
    {
        node_key = it->first.as<std::string>();
        //printf("node_key = %s\n", node_key.c_str());
        if (std::count(required_keys.begin(), required_keys.end(), node_key))
        {
            current_state = keys_present[node_key];

            // If the current key has already been found, then error because each
            // required key must be present once.
            if (current_state)
            {
                SPDLOG_ERROR("Key {:s} already found!", node_key);
                return false;
            }
            keys_present[node_key] = true;
            present_count++;
        }
    }

    if (present_count != required_keys.size())
    {
        for(std::unordered_map<std::string, bool>::const_iterator it2 = keys_present.cbegin();
            it2 != keys_present.cend(); ++it2)
        {
            if(!(it2->second))
            {
                SPDLOG_ERROR("Key {:s} not found!", it2->first);
            }
        }
        return false;
    }

    return true;
}

size_t ICDData::FillElementsFromYamlNodes(const std::string& msg_name, const YAML::Node& msg_data_node,
                                          const YAML::Node& elem_node, bool is_bit_node, std::vector<ICDElement>& icd_elems,
                                          std::map<std::string, std::string>& elem_name_subs)
{
    // word_elem and bit_elem nodes must be maps.
    size_t fill_count = 0;
    if (!elem_node.IsMap())
    {
        SPDLOG_ERROR(
            "Message {:s} elem node is not a map!", msg_name);
        return fill_count;
    }

    // Confirm that each key in the elem map is a map to
    // a map with the correct keys.
    std::string elem_name = "";
    std::string perc_enc_elem_name = "";
    bool is_map_and_has_keys = false;
    if (elem_node.size() > 0)
    {
        for (YAML::const_iterator it = elem_node.begin(); it != elem_node.end(); ++it)
        {
            elem_name = it->first.as<std::string>();
            if (!uri_percent_encode_.PercentEncodeReservedASCIIChars(elem_name, perc_enc_elem_name))
            {
                SPDLOG_ERROR("elem_name {:s} is not ASCII", elem_name);
                return 0;
            }

            if (!(elem_name.compare(perc_enc_elem_name) == 0))
                elem_name_subs[elem_name] = perc_enc_elem_name;

            // Item must be a map and have the correct keys.
            if (is_bit_node)
                is_map_and_has_keys = MapNodeHasRequiredKeys(it->second, yaml_bit_elem_keys_);
            else
                is_map_and_has_keys = MapNodeHasRequiredKeys(it->second, yaml_word_elem_keys_);
            if (!is_map_and_has_keys)
            {
                SPDLOG_ERROR("Message {:s} elem node {:s} is not a map or does not have the correct keys!",
                    msg_name, perc_enc_elem_name);
                continue;
            }

            // Create the ICD element.
            ICDElement temp_icd_elem;
            if (is_bit_node)
            {
                if (!ConfigureBitElementFromYamlNodes(temp_icd_elem, msg_name,
                                                      msg_data_node, perc_enc_elem_name,
                                                      it->second))
                {
                    SPDLOG_ERROR(
                        "Failed to configure "
                        "ICDElement for msg \"{:s}\", bit element \"{:s}\"",
                        msg_name, perc_enc_elem_name);
                    continue;
                }
            }
            else
            {
                if (!ConfigureWordElementFromYamlNodes(temp_icd_elem, msg_name,
                                                       msg_data_node, perc_enc_elem_name,
                                                       it->second))
                {
                    SPDLOG_ERROR(
                        "Failed to configure "
                        "ICDElement for msg \"{:s}\", word element \"{:s}\"",
                        msg_name, perc_enc_elem_name);
                    continue;
                }
            }

            icd_elems.push_back(temp_icd_elem);
            fill_count++;
        }
    }
    return fill_count;
}

bool ICDData::CreateVectorOfStringICDComponents(const std::string& msg_name, const YAML::Node& msg_data_node,
                                                const std::string& elem_name, const YAML::Node& elem_data_node, bool is_bit_elem,
                                                std::vector<std::string>& output_vec)
{
    if (output_vec.size() != ICDElement::kFillElementCount)
    {
        SPDLOG_ERROR("Output vector has size zero!");
        return false;
    }

    if (!SequenceNodeHasCorrectSize(msg_data_node["command"], icd_sequence_size_))
    {
        SPDLOG_ERROR(
            "{:s}/{:s} msg_data[command] node "
            "is not a sequence or does not have size = {:d}!",
            msg_name, elem_name, icd_sequence_size_);
        return false;
    }

    if (!SequenceNodeHasCorrectSize(msg_data_node["lru_addr"], icd_sequence_size_))
    {
        SPDLOG_ERROR(
            "{:s}/{:s} msg_data[lru_addr] node "
            "is not a sequence or does not have size = {:d}!",
            msg_name, elem_name, icd_sequence_size_);
        return false;
    }

    if (!SequenceNodeHasCorrectSize(msg_data_node["lru_subaddr"], icd_sequence_size_))
    {
        SPDLOG_ERROR(
            "{:s}/{:s} msg_data[lru_subaddr] node "
            "is not a sequence or does not have size = {:d}!",
            msg_name, elem_name, icd_sequence_size_);
        return false;
    }
    if (!SequenceNodeHasCorrectSize(msg_data_node["lru_name"], icd_sequence_size_))
    {
        SPDLOG_ERROR(
            "{:s}/{:s} msg_data[lru_name] node "
            "is not a sequence or does not have size = {:d}!",
            msg_name, elem_name, icd_sequence_size_);
        return false;
    }

    output_vec[0] = msg_name;
    output_vec[1] = elem_name;

    output_vec[2] = msg_data_node["command"][0].as<std::string>();
    output_vec[3] = msg_data_node["command"][1].as<std::string>();

    output_vec[4] = msg_data_node["wrdcnt"].as<std::string>();
    output_vec[5] = msg_data_node["bus"].as<std::string>();

    output_vec[6] = msg_data_node["lru_name"][0].as<std::string>();
    output_vec[7] = msg_data_node["lru_addr"][0].as<std::string>();

    output_vec[8] = msg_data_node["lru_name"][1].as<std::string>();
    output_vec[9] = msg_data_node["lru_addr"][1].as<std::string>();

    output_vec[10] = msg_data_node["lru_subaddr"][0].as<std::string>();
    output_vec[11] = msg_data_node["lru_subaddr"][1].as<std::string>();

    //output_vec[12] = std::to_string(msg_data_node["rate"].as<double>());
    output_vec[12] = msg_data_node["rate"].as<std::string>();
    output_vec[13] = elem_data_node["offset"].as<std::string>();
    output_vec[14] = elem_data_node["cnt"].as<std::string>();
    ICDElementSchema schema;
    std::string schema_string = elem_data_node["schema"].as<std::string>();
    if (ICDSchemaStringToEnum(schema_string, schema))
    {
        output_vec[15] = std::to_string(static_cast<int>(schema));
    }
    else
    {
        SPDLOG_ERROR("Failed ICDSchemaStringToEnum()");
        return false;
    }
    output_vec[17] = std::to_string(static_cast<int>(elem_data_node["multifmt"].as<bool>()));
    output_vec[21] = elem_data_node["class"].as<std::string>();
    output_vec[22] = elem_data_node["desc"].as<std::string>();
    output_vec[23] = elem_data_node["msbval"].as<std::string>();
    output_vec[24] = elem_data_node["uom"].as<std::string>();

    if (is_bit_elem)
    {
        output_vec[16] = "1";
        output_vec[18] = elem_data_node["msb"].as<std::string>();
        output_vec[19] = elem_data_node["lsb"].as<std::string>();
        output_vec[20] = elem_data_node["bitcnt"].as<std::string>();
    }
    else
    {
        output_vec[16] = "0";
        output_vec[18] = "0";
        output_vec[19] = "0";
        output_vec[20] = "0";
    }
    return true;
}

bool ICDData::SequenceNodeHasCorrectSize(const YAML::Node& node, const size_t& required_size)
{
    if (!node.IsSequence())
    {
        return false;
    }
    else if (node.size() != required_size)
    {
        return false;
    }
    return true;
}

bool ICDData::ConfigureMsgDataFromYamlNode(ICDElement& icd_elem, const std::string& msg_name,
                                           const YAML::Node& msg_data_node)
{
    /*
    Note that mode_code and desc are not copied here because those are not
    fields relevant to ICDElement.
    */
    icd_elem.msg_name_ = msg_name;
    std::string param_name = "command";
    if (!GetSequenceValuesFromNode(msg_data_node, param_name, yaml_msg_data_keys_,
                                   temp_comm_word_vec_))
        return false;
    if (temp_comm_word_vec_.size() > 0)
    {
        icd_elem.xmit_word_ = static_cast<uint16_t>(temp_comm_word_vec_[0]);
        icd_elem.dest_word_ = static_cast<uint16_t>(temp_comm_word_vec_[1]);
    }

    param_name = "lru_addr";
    if (!GetSequenceValuesFromNode(msg_data_node, param_name, yaml_msg_data_keys_,
                                   temp_lru_addr_vec_))
        return false;
    if (temp_lru_addr_vec_.size() > 0)
    {
        icd_elem.xmit_lru_addr_ = static_cast<uint8_t>(temp_lru_addr_vec_[0]);
        icd_elem.dest_lru_addr_ = static_cast<uint8_t>(temp_lru_addr_vec_[1]);
    }

    param_name = "lru_subaddr";
    if (!GetSequenceValuesFromNode(msg_data_node, param_name, yaml_msg_data_keys_,
                                   temp_lru_subaddr_vec_))
        return false;
    if (temp_lru_subaddr_vec_.size() > 0)
    {
        icd_elem.xmit_lru_subaddr_ = static_cast<uint8_t>(temp_lru_subaddr_vec_[0]);
        icd_elem.dest_lru_subaddr_ = static_cast<uint8_t>(temp_lru_subaddr_vec_[1]);
    }

    param_name = "lru_name";
    if (!GetSequenceValuesFromNode(msg_data_node, param_name, yaml_msg_data_keys_,
                                   temp_lru_name_vec_))
        return false;
    if (temp_lru_name_vec_.size() > 0)
    {
        icd_elem.xmit_lru_name_ = temp_lru_name_vec_[0];
        icd_elem.dest_lru_name_ = temp_lru_name_vec_[1];
    }

    param_name = "bus";
    if (!GetMappedValueFromNode(msg_data_node, param_name, yaml_msg_data_keys_,
                                icd_elem.bus_name_))
        return false;

    param_name = "wrdcnt";
    if (!GetMappedValueFromNode(msg_data_node, param_name, yaml_msg_data_keys_,
                                temp_int_))
        return false;
    icd_elem.msg_word_count_ = static_cast<uint8_t>(temp_int_);

    param_name = "rate";
    if (!GetMappedValueFromNode(msg_data_node, param_name, yaml_msg_data_keys_,
                                icd_elem.rate_))
        return false;

    return true;
}

bool ICDData::ConfigureWordElemDataFromYamlNode(ICDElement& icd_elem,
                                                const YAML::Node& word_elem_node,
                                                const std::string& word_elem_name)
{
    icd_elem.is_bitlevel_ = false;
    icd_elem.elem_name_ = word_elem_name;

    std::string param_name = "offset";
    if (!GetMappedValueFromNode(word_elem_node, param_name, yaml_word_elem_keys_,
                                temp_int_))
        return false;
    icd_elem.offset_ = static_cast<uint8_t>(temp_int_);

    param_name = "cnt";
    if (!GetMappedValueFromNode(word_elem_node, param_name, yaml_word_elem_keys_,
                                temp_int_))
        return false;
    icd_elem.elem_word_count_ = static_cast<uint8_t>(temp_int_);

    param_name = "schema";
    std::string schema_str;
    ICDElementSchema schema_enum = ICDElementSchema::BAD;
    if (!GetMappedValueFromNode(word_elem_node, param_name, yaml_word_elem_keys_,
                                schema_str))
        return false;
    if (!ICDSchemaStringToEnum(schema_str, schema_enum))
        return false;
    icd_elem.schema_ = schema_enum;

    param_name = "msbval";
    if (!GetMappedValueFromNode(word_elem_node, param_name, yaml_word_elem_keys_,
                                icd_elem.msb_val_))
        return false;

    param_name = "desc";
    if (!GetMappedValueFromNode(word_elem_node, param_name, yaml_word_elem_keys_,
                                icd_elem.description_))
        return false;

    param_name = "uom";
    if (!GetMappedValueFromNode(word_elem_node, param_name, yaml_word_elem_keys_,
                                icd_elem.uom_))
        return false;

    param_name = "multifmt";
    if (!GetMappedValueFromNode(word_elem_node, param_name, yaml_word_elem_keys_,
                                icd_elem.is_multiformat_))
        return false;

    param_name = "class";
    if (!GetMappedValueFromNode(word_elem_node, param_name, yaml_word_elem_keys_,
                                temp_int_))
        return false;
    icd_elem.classification_ = static_cast<uint8_t>(temp_int_);

    return true;
}

bool ICDData::ConfigureBitElemDataFromYamlNode(ICDElement& icd_elem,
                                               const YAML::Node& bit_elem_node,
                                               const std::string& bit_elem_name)
{
    icd_elem.is_bitlevel_ = true;
    icd_elem.elem_name_ = bit_elem_name;

    std::string param_name = "offset";
    if (!GetMappedValueFromNode(bit_elem_node, param_name, yaml_bit_elem_keys_,
                                temp_int_))
        return false;
    icd_elem.offset_ = static_cast<uint8_t>(temp_int_);

    param_name = "cnt";
    if (!GetMappedValueFromNode(bit_elem_node, param_name, yaml_bit_elem_keys_,
                                temp_int_))
        return false;
    icd_elem.elem_word_count_ = static_cast<uint8_t>(temp_int_);

    param_name = "schema";
    std::string schema_str;
    ICDElementSchema schema_enum = ICDElementSchema::BAD;
    if (!GetMappedValueFromNode(bit_elem_node, param_name, yaml_bit_elem_keys_,
                                schema_str))
        return false;
    if (!ICDSchemaStringToEnum(schema_str, schema_enum))
        return false;
    icd_elem.schema_ = schema_enum;

    param_name = "msbval";
    if (!GetMappedValueFromNode(bit_elem_node, param_name, yaml_bit_elem_keys_,
                                icd_elem.msb_val_))
        return false;

    param_name = "desc";
    if (!GetMappedValueFromNode(bit_elem_node, param_name, yaml_bit_elem_keys_,
                                icd_elem.description_))
        return false;

    param_name = "uom";
    if (!GetMappedValueFromNode(bit_elem_node, param_name, yaml_bit_elem_keys_,
                                icd_elem.uom_))
        return false;

    param_name = "multifmt";
    if (!GetMappedValueFromNode(bit_elem_node, param_name, yaml_bit_elem_keys_,
                                icd_elem.is_multiformat_))
        return false;

    param_name = "class";
    if (!GetMappedValueFromNode(bit_elem_node, param_name, yaml_bit_elem_keys_,
                                temp_int_))
        return false;
    icd_elem.classification_ = static_cast<uint8_t>(temp_int_);

    param_name = "msb";
    if (!GetMappedValueFromNode(bit_elem_node, param_name, yaml_bit_elem_keys_,
                                temp_int_))
        return false;
    icd_elem.bitmsb_ = static_cast<uint8_t>(temp_int_);

    param_name = "lsb";
    if (!GetMappedValueFromNode(bit_elem_node, param_name, yaml_bit_elem_keys_,
                                temp_int_))
        return false;
    icd_elem.bitlsb_ = static_cast<uint8_t>(temp_int_);

    param_name = "bitcnt";
    if (!GetMappedValueFromNode(bit_elem_node, param_name, yaml_bit_elem_keys_,
                                temp_int_))
        return false;
    icd_elem.bit_count_ = static_cast<uint8_t>(temp_int_);

    return true;
}

bool ICDData::ConfigureWordElementFromYamlNodes(ICDElement& icd_elem, const std::string& msg_name,
                                                const YAML::Node& msg_data_node,
                                                const std::string& word_elem_name,
                                                const YAML::Node& word_elem_node)
{
    if (!ConfigureMsgDataFromYamlNode(icd_elem, msg_name, msg_data_node))
        return false;

    if (!ConfigureWordElemDataFromYamlNode(icd_elem, word_elem_node, word_elem_name))
        return false;

    return true;
}

bool ICDData::ConfigureBitElementFromYamlNodes(ICDElement& icd_elem, const std::string& msg_name,
                                               const YAML::Node& msg_data_node,
                                               const std::string& bit_elem_name,
                                               const YAML::Node& bit_elem_node)
{
    if (!ConfigureMsgDataFromYamlNode(icd_elem, msg_name, msg_data_node))
        return false;

    if (!ConfigureBitElemDataFromYamlNode(icd_elem, bit_elem_node, bit_elem_name))
        return false;

    return true;
}
