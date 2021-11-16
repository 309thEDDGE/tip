#include "translate_tabular_context_1553.h"

const size_t TranslateTabularContext1553::raw_data_count_per_row_ = 32;

TranslateTabularContext1553::TranslateTabularContext1553(ICDData icd,
                                                         const std::set<std::string>& selected_msg_names) : TranslateTabularContextBase(), icd_data_(icd), pq_reader_(), input_row_group_count_(-1), row_group_index_(-1), should_select_msgs_(false), selected_msg_names_(selected_msg_names), output_dir_(""), output_base_path_(""), current_row_group_row_count_(0), translated_msg_names(translated_msg_names_)
{
    selected_table_indices_ = icd_data_.GetSelectedTableIndicesSet(selected_msg_names_,
                                                                   icd_data_.table_names);
}

bool TranslateTabularContext1553::IsConfigured()
{
    if (!TranslateTabularContextBase::IsConfigured())
    {
        return false;
    }

    return true;
}

std::shared_ptr<TranslateTabularContextBase> TranslateTabularContext1553::Clone()
{
    std::shared_ptr<TranslateTabularContext1553> temp =
        std::make_shared<TranslateTabularContext1553>(this->icd_data_,
                                                      this->selected_msg_names_);
    temp->SetColumnNames(this->ridealong_col_names_, this->data_col_names_);
    return temp;
}

TranslateStatus TranslateTabularContext1553::OpenInputFile(const ManagedPath& input_path,
                                                           const size_t& thread_index)
{
    // Do not increment row group automatically after every call to GetNextRG()
    pq_reader_.SetManualRowgroupIncrementMode();

    // Open file using ParquetReader
    if (!pq_reader_.SetPQPath(input_path))
        return TranslateStatus::CONTINUE;

    // Check if column names are present in schema
    if (!pq_reader_.ColumnsPresent(ridealong_col_names_))
    {
        SPDLOG_ERROR("{:02d} Not all columns in ridealong_col_names_ are present in schema",
                     thread_index);
        return TranslateStatus::FAIL;
    }
    if (!pq_reader_.ColumnsPresent(data_col_names_))
    {
        SPDLOG_ERROR("{:02d} Not all columns in data_col_names_ are present in schema",
                     thread_index);
        return TranslateStatus::FAIL;
    }

    // ParquetReader will return zero on SetPQPath if row group count
    // is zero, so no need to check the value here.
    input_row_group_count_ = pq_reader_.GetRowGroupCount();
    row_group_index_ = 0;
    schema_ = pq_reader_.GetSchema();

    return TranslateStatus::OK;
}

ManagedPath TranslateTabularContext1553::CreateTableOutputPath(const ManagedPath& output_dir,
                                                               const ManagedPath& output_base_name, std::string table_name,
                                                               size_t thread_index)
{
    ManagedPath output = output_dir / table_name;
    output += ".parquet";
    ManagedPath file_name = output_base_name;

    const int buff_size = 100;
    char buffer[100];
    snprintf(buffer, buff_size, "%02zu.parquet", thread_index);
    std::string ext(buffer);

    file_name += ext;
    output /= file_name;

    return output;
}

TranslateStatus TranslateTabularContext1553::OpenOutputFile(const ManagedPath& output_dir,
                                                            const ManagedPath& output_base_name, const size_t& thread_index)
{
    output_dir_ = output_dir;
    output_base_path_ = output_base_name;
    return TranslateStatus::OK;
}

TranslateStatus TranslateTabularContext1553::ConsumeFile(const size_t& thread_index)
{
    TranslateStatus row_group_status;
    while ((row_group_status = ConsumeRowGroup(thread_index)) == TranslateStatus::OK)
    {
    }
    if (row_group_status == TranslateStatus::FAIL)
    {
        SPDLOG_ERROR("{:02d} FAIL status return from ConsumeRowGroup()", thread_index);
        return row_group_status;
    }

    return TranslateStatus::OK;
}

TranslateStatus TranslateTabularContext1553::ConsumeRowGroup(const size_t& thread_index)
{
    if (row_group_index_ == input_row_group_count_)
        return TranslateStatus::CONTINUE;

    if (!FillRowGroupVectors())
    {
        SPDLOG_WARN("{:02d} Failed to fill row group vectors", thread_index);
        return TranslateStatus::FAIL;
    }
    pq_reader_.IncrementRG();
    row_group_index_++;

    // Iterate over each row in the row group
    std::set<size_t> matching_table_inds;
    std::set<size_t>::const_iterator table_ind_it;
    SPDLOG_DEBUG("{:02d} Iterating over {:d} parsed data rows", thread_index,
                 current_row_group_row_count_);

    bool result = false;
    std::shared_ptr<TranslatableTableBase> table_ptr;
    const uint16_t* raw_data_ptr = raw_data_.data();
    const uint8_t* data_ptr = nullptr;
    const uint8_t* ridealong_data_ptr = nullptr;
    std::string ridealong_col_name = "time";
    ManagedPath output_path;
    std::string table_name = "";
    for (size_t row_ind = 0; row_ind < current_row_group_row_count_; row_ind++)
    {
        // Use the ICD lookup created from the DTS1553 to identify all table
        // indices which match the given criteria.
        matching_table_inds = icd_data_.LookupTableIndex(channelid_[row_ind],
                                                         txrtaddr_[row_ind],
                                                         rxrtaddr_[row_ind],
                                                         txsubaddr_[row_ind],
                                                         rxsubaddr_[row_ind]);

        // Iterate over the matching table indices. Create a TranslatableTable
        // or append data to an existing table.
        for (table_ind_it = matching_table_inds.cbegin();
             table_ind_it != matching_table_inds.cend(); ++table_ind_it)
        {
            if (should_select_msgs_)
            {
                // Skip the current table if it is not in selected_table_indices_
                if (selected_table_indices_.count(*table_ind_it) == 0)
                {
                    SPDLOG_TRACE("{:02d} table index {:d} not a selected table",
                                 thread_index, *table_ind_it);
                    continue;
                }
            }

            table_name = icd_data_.table_names.at(*table_ind_it);

            // if(table_name != "AP03_I")
            //     continue;

            // If the table index is not present in the set of already created table
            // indices, then create the table and its columns and add it to the set.
            if (table_indices_.count(*table_ind_it) == 0)
            {
                // Create the output path and create the directory if it doesn't
                // exist.
                output_path = CreateTableOutputPath(output_dir_, output_base_path_,
                                                    table_name, thread_index);
                SPDLOG_DEBUG("{:02d} Output file path: {:s}", thread_index,
                             output_path.RawString());
                if (!output_path.parent_path().is_directory())
                {
                    if (!output_path.parent_path().create_directory())
                    {
                        SPDLOG_ERROR(
                            "{:02d} Failed to create directory for table \"{:s}\": "
                            "{:s}",
                            thread_index, table_name, output_path.parent_path().RawString());
                        return TranslateStatus::FAIL;
                    }
                }

                SPDLOG_DEBUG("{:02d} Creating table with index {:d}",
                             thread_index, *table_ind_it);
                result = CreateTable(icd_data_, *table_ind_it, table_index_to_table_map_,
                                     output_path, thread_index);

                // Add the index to the set to track table creation.
                table_indices_.insert(*table_ind_it);

                if (!result)
                {
                    SPDLOG_WARN("{:02d} Failed to create table for message: {:s}",
                                thread_index, icd_data_.table_names.at(*table_ind_it));
                    continue;
                }
                translated_msg_names_.insert(table_name);
            }

            // Append data to the table if it is valid.
            table_ptr = table_index_to_table_map_[*table_ind_it];
            if (table_ptr->is_valid)
            {
                // Append data to the ridealong column
                // In the current configuration of the framework, ridealong columns
                // must be appended prior to appendrawdata, which handles the recording
                // of data, if necessary.
                ridealong_data_ptr = reinterpret_cast<const uint8_t*>(time_.data() + row_ind);
                if (!table_ptr->AppendRidealongColumnData(ridealong_data_ptr, 1,
                                                          ridealong_col_name))
                {
                    SPDLOG_WARN("{:02d} Failed to append data for ridealong column \"{:s}\" ",
                                "for message: ", thread_index, ridealong_col_name,
                                icd_data_.table_names.at(*table_ind_it));
                    continue;
                }

                // Append data to all other columns
                data_ptr = reinterpret_cast<const uint8_t*>(
                    raw_data_ptr + row_ind * raw_data_count_per_row_);

                if (!table_ptr->AppendRawData(data_ptr, raw_data_count_per_row_))
                {
                    SPDLOG_WARN(
                        "{:02d} Failed to append raw data to columns of table for message: "
                        "{:s}",
                        thread_index, icd_data_.table_names.at(*table_ind_it));
                    continue;
                }
            }
        }
    }

    return TranslateStatus::OK;
}

bool TranslateTabularContext1553::FillRowGroupVectors()
{
    int row_count = 0;

    // Read relevant columns into vectors.
    if (!FillRGVector<uint64_t, arrow::NumericArray<arrow::Int64Type>>(time_,
                                                                       "time", row_count, false))
        return false;

    if (!FillRGVector<uint16_t, arrow::NumericArray<arrow::Int32Type>>(raw_data_,
                                                                       "data", row_count, true))
        return false;

    if (!FillRGVector<uint16_t, arrow::NumericArray<arrow::Int32Type>>(channelid_,
                                                                       "channelid", row_count, false))
        return false;

    if (!FillRGVector<uint8_t, arrow::NumericArray<arrow::Int8Type>>(txrtaddr_,
                                                                     "txrtaddr", row_count, false))
        return false;

    if (!FillRGVector<uint8_t, arrow::NumericArray<arrow::Int8Type>>(rxrtaddr_,
                                                                     "rxrtaddr", row_count, false))
        return false;

    if (!FillRGVector<uint8_t, arrow::NumericArray<arrow::Int8Type>>(txsubaddr_,
                                                                     "txsubaddr", row_count, false))
        return false;

    if (!FillRGVector<uint8_t, arrow::NumericArray<arrow::Int8Type>>(rxsubaddr_,
                                                                     "rxsubaddr", row_count, false))
        return false;

    current_row_group_row_count_ = row_count;
    return true;
}

bool TranslateTabularContext1553::CreateTable(const ICDData& icd_data, size_t table_index,
                                              std::unordered_map<size_t, std::shared_ptr<TranslatableTableBase>>& table_map,
                                              const ManagedPath& output_path, const size_t& thread_index)
{
    // Create the table object relevant to the ICDData object
    size_t row_group_size = current_row_group_row_count_;
    std::string table_name = icd_data.LookupTableNameByIndex(table_index);
    if (!CreateTranslatableTable(table_name, row_group_size, table_index,
                                 table_index_to_table_map_, output_path, thread_index))
    {
        SPDLOG_WARN("{:02d} Failed to Create table: {:s}", thread_index, table_name);
        return false;
    }

    // Create the ridealong column(s), which in the case of 1553 is only the
    // time column.
    std::shared_ptr<TranslatableTableBase>& table_ptr = table_index_to_table_map_[table_index];
    ICDElement icd_elem;
    icd_elem.elem_word_count_ = 1;
    if (!AppendColumn<uint16_t, uint64_t>(table_ptr, "time", true, icd_elem))
    {
        SPDLOG_WARN("{:02d} Failed to create \"time\" column", thread_index);
        return false;
    }

    // Loop over the ICDElement objects associated with the table and
    // create columns in the table object.
    const std::vector<size_t>& elem_inds = icd_data.GetTableElementIndices(table_index);
    for (std::vector<size_t>::const_iterator elem_ind_it = elem_inds.cbegin();
         elem_ind_it != elem_inds.cend(); ++elem_ind_it)
    {
        if (!AppendColumnFromICDElement(table_ptr, icd_data.GetElementByIndex(*elem_ind_it)))
        {
            SPDLOG_WARN("{:02d} Failed to create column: {:s}, table: {:s}",
                        thread_index, icd_data.GetElementByIndex(*elem_ind_it).elem_name_, table_name);
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

bool TranslateTabularContext1553::AppendColumnFromICDElement(
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
                result = AppendColumn<uint16_t, float>(table_ptr, icd_elem.elem_name_,
                                                       false, icd_elem);
            }
            else
            {
                result = AppendColumn<uint16_t, double>(table_ptr, icd_elem.elem_name_,
                                                        false, icd_elem);
            }
            break;
        }
        case ICDElementSchema::UNSIGNEDBITS:
        {
            if (icd_elem.bit_count_ == 1)
            {
                result = AppendBooleanColumn<uint16_t>(table_ptr, icd_elem.elem_name_,
                                                       false, icd_elem);
            }
            else if (icd_elem.bit_count_ < 25)
            {
                result = AppendColumn<uint16_t, float>(table_ptr, icd_elem.elem_name_,
                                                       false, icd_elem);
            }
            else
            {
                result = AppendColumn<uint16_t, double>(table_ptr, icd_elem.elem_name_,
                                                        false, icd_elem);
            }
            break;
        }
        case ICDElementSchema::ASCII:
        {
            result = AppendASCIIColumn<uint16_t>(table_ptr, icd_elem.elem_name_, false,
                                                 icd_elem);
            break;
        }
        case ICDElementSchema::SIGNED16:
        {
            result = AppendColumn<uint16_t, float>(table_ptr, icd_elem.elem_name_, false,
                                                   icd_elem);
            break;
        }
        case ICDElementSchema::SIGNED32:
        {
            result = AppendColumn<uint16_t, double>(table_ptr, icd_elem.elem_name_, false,
                                                    icd_elem);
            break;
        }
        case ICDElementSchema::UNSIGNED16:
        {
            result = AppendColumn<uint16_t, float>(table_ptr, icd_elem.elem_name_, false,
                                                   icd_elem);
            break;
        }
        case ICDElementSchema::UNSIGNED32:
        {
            result = AppendColumn<uint16_t, double>(table_ptr, icd_elem.elem_name_, false,
                                                    icd_elem);
            break;
        }
        case ICDElementSchema::FLOAT32_1750:
        {
            result = AppendColumn<uint16_t, float>(table_ptr, icd_elem.elem_name_, false,
                                                   icd_elem);
            break;
        }
        case ICDElementSchema::FLOAT32_GPS:
        {
            result = AppendColumn<uint16_t, float>(table_ptr, icd_elem.elem_name_, false,
                                                   icd_elem);
            break;
        }
        case ICDElementSchema::FLOAT32_IEEE:
        {
            result = AppendColumn<uint16_t, float>(table_ptr, icd_elem.elem_name_, false,
                                                   icd_elem);
            break;
        }
        case ICDElementSchema::FLOAT64_GPS:
        {
            result = AppendColumn<uint16_t, double>(table_ptr, icd_elem.elem_name_, false,
                                                    icd_elem);
            break;
        }
        case ICDElementSchema::FLOAT64_IEEE:
        {
            result = AppendColumn<uint16_t, double>(table_ptr, icd_elem.elem_name_, false,
                                                    icd_elem);
            break;
        }
        case ICDElementSchema::FLOAT16:
        {
            result = AppendColumn<uint16_t, float>(table_ptr, icd_elem.elem_name_, false,
                                                   icd_elem);
            break;
        }
        case ICDElementSchema::CAPS:
        {
            result = AppendColumn<uint16_t, double>(table_ptr, icd_elem.elem_name_, false,
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

TranslateStatus TranslateTabularContext1553::CloseOutputFile(
    const size_t& thread_index, bool is_final_file)
{
    if (is_final_file)
    {
        for (std::unordered_map<size_t, std::shared_ptr<TranslatableTableBase>>::iterator it =
                 table_index_to_table_map_.begin();
             it != table_index_to_table_map_.end(); ++it)
        {
            if (it->second->is_valid)
            {
                SPDLOG_DEBUG("{:02d} Closing output file for table {:s}",
                             thread_index, it->second->name);
                it->second->CloseOutputFile();
            }
        }
    }
    return TranslateStatus::OK;
}

void TranslateTabularContext1553::PrintPayloadVals(std::string table_name,
                                                   const uint16_t* data_ptr)
{
    std::stringstream s;
    for (size_t i = 0; i < raw_data_count_per_row_; i++)
    {
        if (i == raw_data_count_per_row_ - 1)
            s << i << ":" << data_ptr[i];
        else
            s << i << ":" << data_ptr[i] << ", ";
    }
    SPDLOG_INFO("{:s} payload - {:s}", table_name, s.str());
}
