#include "translate_tabular_context_arinc429.h"

TranslateTabularContextARINC429::TranslateTabularContextARINC429(ARINC429Data icd)
                                                                : TranslateTabularParquet(),
                                                                arinc_data_(icd),
                                                                translated_msg_names(translated_msg_names_)
{
    arinc_data_ = icd;
    arinc_word_names_ = arinc_data_.GetArincWordNamesMap();

    arinc_word_name_to_unique_index_map_ = arinc_data_.GetNamesToUniqueIndexMap();

}

std::shared_ptr<TranslateTabularContextBase> TranslateTabularContextARINC429::Clone()
{
    std::shared_ptr<TranslateTabularContextARINC429> temp =
        std::make_shared<TranslateTabularContextARINC429>(this->arinc_data_);

    temp->SetColumnNames(this->ridealong_col_names_, this->data_col_names_);
    return temp;
}


TranslateStatus TranslateTabularContextARINC429::ConsumeRowGroup(const size_t& thread_index)
{
    TranslateStatus status = ReadRowGroup(thread_index, input_row_group_count_, row_group_index_);
    if(status != TranslateStatus::OK)
        return status;

    // Iterate over each row in the row group
    size_t table_index = 0;;
    size_t word_group_index = 0;
    std::vector<std::vector<ICDElement>> indexed_element_vectors;
    size_t  elem_vec_index;
    ManagedPath output_path;
    std::string table_name = "";
    bool result = false;
    uint16_t subchannel_id;
    int8_t sdi;
    int8_t sign = 0;

    for (size_t row_ind = 0; row_ind < current_row_group_row_count_; row_ind++)
    {
        // Why are these casts necessary? Just because of the argument data types
        // to IdentifyWord? Why is SDI, originally a uint8_t, cast to signed?
        subchannel_id = static_cast<uint16_t>(bus_[row_ind]);
        sdi = static_cast<int8_t>(sdi_[row_ind]);

        // Use the ICD lookup created from the DTS1553 to identify all table
        // indices which match the given criteria.
        if(!arinc_data_.IdentifyWord( word_group_index,
                                      channelid_[row_ind],
                                      subchannel_id,
                                      label_[row_ind],
                                      sdi))
        {
            // SPDLOG_WARN("Failed to identify word with channelid {:d}, subchannel_id {:d}, "
            //     "label {:d}, sdi {:d}", channelid_[row_ind], subchannel_id,
            //     label_[row_ind], sdi);
            continue;
        }

        // get the vector<vector<ICDElement>>. Ensure all possible associated words
        // are processed by looping over vector below.
        if(!arinc_data_.GetWordElements(word_group_index,
                                        indexed_element_vectors))
        {
            SPDLOG_WARN("Failed to get word elements for word_group_index = {:d}",
                word_group_index);
            continue;
        }

        // Iterate over vectors<ICDElement> in indexed_element_vectors
        // Create a TranslatableTable or append data to an existing table.
        for(elem_vec_index = 0; elem_vec_index < indexed_element_vectors.size();
            elem_vec_index++)
        {
            // table_name = name of ARINC 429 word associated with the ICDElement vector
            table_name = arinc_word_names_[word_group_index][elem_vec_index];
            table_index = arinc_word_name_to_unique_index_map_.at(table_name);

            SPDLOG_DEBUG("Table Name: {:s}, channelid: {:d}, subchanid: {:d}, "
                        "table_index: {:d}", table_name, channelid_[row_ind],
                        subchannel_id, table_index);

            // If the table index or the word vector index is not present in the
            // set of already created table indices, then create a the appropriate
            // table and its columns and add it to the sets.
            if (table_indices_.count(table_index) == 0)
            {

                // Create the output path and create the directory if it doesn't
                // exist.
                output_path = CreateTableOutputPath(output_dir_, output_base_path_,
                                                    table_name, thread_index);
                if(!CreateTableOutputDir(thread_index, output_path, table_name))
                    return TranslateStatus::FAIL;

                SPDLOG_DEBUG("{:02d} Creating table with index {:d}",
                            thread_index, table_index);
                result = CreateTable(indexed_element_vectors[elem_vec_index], table_index,
                    table_index_to_table_map_, output_path, thread_index,
                    table_name, elem_vec_index);

                // Add the index to the set to track table creation.
                table_indices_.insert(table_index);

                if (!result)
                {
                    SPDLOG_WARN("{:02d} Failed to create table for ARINC word: {:s}",
                                thread_index, table_name);
                    continue;
                }
                translated_msg_names_.insert(table_name);
            }

            if(!arinc_data_.SSMSignForBCD(ssm_[row_ind], sign))
                sign = 1;

            // Append data to the table if it is valid.
            AppendTimeAndRawDataToTable(thread_index, table_index_to_table_map_[table_index],
                reinterpret_cast<const uint8_t*>(time_.data() + row_ind),
                reinterpret_cast<const uint8_t*>(data_.data() + row_ind),
                1, table_name, sign);
        }
    }
    return TranslateStatus::OK;

}

bool TranslateTabularContextARINC429::FillRowGroupVectors()
{
    int row_count = 0;

    // Read relevant columns into vectors.
    if (!FillRGVector<int64_t, arrow::NumericArray<arrow::Int64Type>>(time_,
                                                                       "time", row_count, false))
        return false;

    if (!FillRGVector<uint16_t, arrow::NumericArray<arrow::Int32Type>>(channelid_,
                                                                       "channelid", row_count, false))
        return false;

    if (!FillRGVector<uint8_t, arrow::NumericArray<arrow::Int16Type>>(bus_,
                                                                     "bus", row_count, false))
        return false;

    if (!FillRGVector<uint16_t, arrow::NumericArray<arrow::Int16Type>>(label_,
                                                                     "label", row_count, false))
        return false;

    if (!FillRGVector<uint8_t, arrow::NumericArray<arrow::Int8Type>>(sdi_,
                                                                     "SDI", row_count, false))
        return false;

    if (!FillRGVector<uint32_t, arrow::NumericArray<arrow::Int32Type>>(data_,
                                                                     "data", row_count, false))
        return false;

    if (!FillRGVector<uint8_t, arrow::NumericArray<arrow::Int8Type>>(ssm_,
                                                                     "SSM", row_count, false))
        return false;

    if (!FillRGVector<uint8_t, arrow::NumericArray<arrow::BooleanType>>(parity_,
                                                                     "parity", row_count, false))
        return false;

    current_row_group_row_count_ = row_count;
    return true;
}


bool TranslateTabularContextARINC429::CreateTable(const std::vector<ICDElement>& icd_data, size_t table_index,
                                              std::unordered_map<size_t, std::shared_ptr<TranslatableTableBase>>& table_map,
                                              const ManagedPath& output_path, const size_t& thread_index,
                                              const std::string& table_name, const size_t& vector_index)
{
    // Create the table object relevant to the vector of ICDElements describing the ARINC 429 word
    size_t row_group_size = current_row_group_row_count_;

    if (!CreateTranslatableTable(table_name, row_group_size, table_index,
                                 table_index_to_table_map_, output_path, thread_index))
    {
        SPDLOG_WARN("{:02d} Failed to Create table: {:s}", thread_index, table_name);
        return false;
    }

    // Create the ridealong column(s), which in the case of ARINC 429 is only the
    // time column.
    std::shared_ptr<TranslatableTableBase>& table_ptr = table_index_to_table_map_[table_index];
    ICDElement icd_elem;
    icd_elem.elem_word_count_ = 1;
    if (!AppendColumn<uint32_t, int64_t>(table_ptr, "time", true, icd_elem))
    {
        SPDLOG_WARN("{:02d} Failed to create \"time\" column", thread_index);
        return false;
    }

    // Loop over the ICDElement objects associated with the table and
    // create columns in the table object.
    for (std::vector<ICDElement>::const_iterator it = icd_data.cbegin();
        it != icd_data.cend(); ++it)
    {
        if (!AppendColumnFromICDElement(table_ptr, *it))
        {
            SPDLOG_WARN("{:02d} Failed to create column: {:s}, table: {:s}",
                        thread_index, it->elem_name_, table_name);
            return false;
        }
    }

    // Configure Parquet writer functionality
    if (!table_ptr->ConfigurePqContext())
    {
        SPDLOG_WARN("{:02d} Failed to configure parquet context", thread_index);
        return false;
    }

    table_ptr->LogSchema();

    return true;
}

bool TranslateTabularContextARINC429::AppendColumnFromICDElement(
    std::shared_ptr<TranslatableTableBase>& table_ptr,
    const ICDElement& icd_elem)
{
    bool result = false;
    switch (icd_elem.schema_)
    {
        case ICDElementSchema::SIGNEDBITS:
        {
            if (icd_elem.bit_count_ < 25)
            {
                result = AppendColumn<uint32_t, float>(table_ptr, icd_elem.elem_name_,
                                                       false, icd_elem);
            }
            else
            {
                result = AppendColumn<uint32_t, double>(table_ptr, icd_elem.elem_name_,
                                                        false, icd_elem);
            }
            break;
        }
        case ICDElementSchema::UNSIGNEDBITS:
        {
            if (icd_elem.bit_count_ == 1)
            {
                result = AppendBooleanColumn<uint32_t>(table_ptr, icd_elem.elem_name_,
                                                       false, icd_elem);
            }
            else if (icd_elem.bit_count_ < 25)
            {
                result = AppendColumn<uint32_t, float>(table_ptr, icd_elem.elem_name_,
                                                       false, icd_elem);
            }
            else
            {
                result = AppendColumn<uint32_t, double>(table_ptr, icd_elem.elem_name_,
                                                        false, icd_elem);
            }
            break;
        }
        case ICDElementSchema::SIGNMAG:
        {
            if (icd_elem.bit_count_ < 25)
            {
                result = AppendColumn<uint32_t, float>(table_ptr, icd_elem.elem_name_,
                                                       false, icd_elem);
            }
            else
            {
                result = AppendColumn<uint32_t, double>(table_ptr, icd_elem.elem_name_,
                                                        false, icd_elem);
            }
            break;
        }
        case ICDElementSchema::BCD:
        {
            if (icd_elem.bit_count_ < 25)
            {
                result = AppendExplicitSignColumn<uint32_t, float>(table_ptr, icd_elem.elem_name_,
                                                       false, icd_elem);
            }
            else
            {
                result = AppendExplicitSignColumn<uint32_t, double>(table_ptr, icd_elem.elem_name_,
                                                        false, icd_elem);
            }
            break;
        }
        case ICDElementSchema::ASCII:
        {
            result = AppendASCIIColumn<uint32_t>(table_ptr, icd_elem.elem_name_, false,
                                                 icd_elem);
            break;
        }
        case ICDElementSchema::SIGNED16:
        {
            result = AppendColumn<uint32_t, float>(table_ptr, icd_elem.elem_name_, false,
                                                   icd_elem);
            break;
        }
        case ICDElementSchema::SIGNED32:
        {
            result = AppendColumn<uint32_t, double>(table_ptr, icd_elem.elem_name_, false,
                                                    icd_elem);
            break;
        }
        case ICDElementSchema::UNSIGNED16:
        {
            result = AppendColumn<uint32_t, float>(table_ptr, icd_elem.elem_name_, false,
                                                   icd_elem);
            break;
        }
        case ICDElementSchema::UNSIGNED32:
        {
            result = AppendColumn<uint32_t, double>(table_ptr, icd_elem.elem_name_, false,
                                                    icd_elem);
            break;
        }
        case ICDElementSchema::FLOAT32_1750:
        {
            result = AppendColumn<uint32_t, float>(table_ptr, icd_elem.elem_name_, false,
                                                   icd_elem);
            break;
        }
        case ICDElementSchema::FLOAT32_GPS:
        {
            result = AppendColumn<uint32_t, float>(table_ptr, icd_elem.elem_name_, false,
                                                   icd_elem);
            break;
        }
        case ICDElementSchema::FLOAT32_IEEE:
        {
            result = AppendColumn<uint32_t, float>(table_ptr, icd_elem.elem_name_, false,
                                                   icd_elem);
            break;
        }
        case ICDElementSchema::FLOAT64_GPS:
        {
            result = AppendColumn<uint32_t, double>(table_ptr, icd_elem.elem_name_, false,
                                                    icd_elem);
            break;
        }
        case ICDElementSchema::FLOAT64_IEEE:
        {
            result = AppendColumn<uint32_t, double>(table_ptr, icd_elem.elem_name_, false,
                                                    icd_elem);
            break;
        }
        case ICDElementSchema::FLOAT16:
        {
            result = AppendColumn<uint32_t, float>(table_ptr, icd_elem.elem_name_, false,
                                                   icd_elem);
            break;
        }
        case ICDElementSchema::CAPS:
        {
            result = AppendColumn<uint32_t, double>(table_ptr, icd_elem.elem_name_, false,
                                                    icd_elem);
            break;
        }
        default:
        {
            SPDLOG_WARN("Invalid schema: value {:d}", static_cast<uint8_t>(icd_elem.schema_));
            return false;
        }
    }

    if (!result)
    {
        SPDLOG_WARN("Failed to AppendColumn for element: {:s}, schema: value {:d}",
                    icd_elem.elem_name_, static_cast<uint8_t>(icd_elem.schema_));
        return false;
    }

    return true;
}

bool TranslateTabularContextARINC429::CreateTranslatableTable(const std::string& name,
                                                          size_t row_group_size, size_t index,
                                                          std::unordered_map<size_t, std::shared_ptr<TranslatableTableBase>>& table_map,
                                                          const ManagedPath& output_path, const size_t& thread_index)
{
    if (!IsConfigured())
    {
        SPDLOG_WARN("{:02d} Not configured, use Configure()", thread_index);
        return false;
    }

    // Parent dir of output file must exist.
    if (!output_path.parent_path().is_directory())
    {
        SPDLOG_WARN("{:02d} Parent path of complete output file path must exist: {:s}",
                    thread_index, output_path.parent_path().RawString());
        return false;
    }

    std::shared_ptr<TranslatableTableBase> table = std::make_shared<TranslatableARINC429Table>();
    if (!table->Configure(name, row_group_size, output_path, thread_index))
    {
        SPDLOG_WARN("{:02d} Table is not configured", thread_index);
        return false;
    }

    table_map[index] = table;

    return true;
}

bool TranslateTabularContextARINC429::AppendTimeAndRawDataToTable(const size_t& thread_index,
        std::shared_ptr<TranslatableTableBase> table, const uint8_t* time_data,
        const uint8_t* raw_data, const size_t& raw_data_count,
        const std::string& table_name, int8_t sign)
{
    if (table->is_valid)
    {
        // Append data to the ridealong column
        // In the current configuration of the framework, ridealong columns
        // must be appended prior to appendrawdata, which handles the recording
        // of data, if necessary.
        if (!table->AppendRidealongColumnData(time_data, 1, "time"))
        {
            SPDLOG_WARN("{:02d} Failed to append data for ridealong column \"time\" ",
                        "for message: ", thread_index, table_name);
            return true;
        }

        // Append data to all other columns
        if (!table->AppendRawData(raw_data, raw_data_count, sign))
        {
            SPDLOG_WARN(
                "{:02d} Failed to append raw data to columns of table for message: "
                "{:s}", thread_index, table_name);
        }
    }

    return true;
}
