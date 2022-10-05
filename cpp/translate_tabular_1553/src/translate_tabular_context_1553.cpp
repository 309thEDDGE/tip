#include "translate_tabular_context_1553.h"

const size_t TranslateTabularContext1553::raw_data_count_per_row_ = 32;

TranslateTabularContext1553::TranslateTabularContext1553(ICDData icd,
                                                         const std::set<std::string>& selected_msg_names) : 
                                                         TranslateTabularParquet(), icd_data_(icd), 
                                                         should_select_msgs_(false), 
                                                         selected_msg_names_(selected_msg_names), 
                                                         translated_msg_names(translated_msg_names_)
{
    selected_table_indices_ = icd_data_.GetSelectedTableIndicesSet(selected_msg_names_,
                                                                   icd_data_.table_names);
}

std::shared_ptr<TranslateTabularContextBase> TranslateTabularContext1553::Clone()
{
    std::shared_ptr<TranslateTabularContext1553> temp =
        std::make_shared<TranslateTabularContext1553>(this->icd_data_,
                                                      this->selected_msg_names_);
    temp->SetColumnNames(this->ridealong_col_names_, this->data_col_names_);
    return temp;
}

TranslateStatus TranslateTabularContext1553::ConsumeRowGroup(const size_t& thread_index)
{
    TranslateStatus status = ReadRowGroup(thread_index, input_row_group_count_, row_group_index_);
    if(status != TranslateStatus::OK)
        return status;

    // Iterate over each row in the row group
    std::set<size_t> matching_table_inds;
    std::set<size_t>::const_iterator table_ind_it;
    ManagedPath output_path;
    std::string table_name = "";
    bool result = false;
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
            if(!IsSelectedMessage(thread_index, should_select_msgs_, 
                selected_table_indices_, *table_ind_it))
                continue;

            table_name = icd_data_.table_names.at(*table_ind_it);

            // If the table index is not present in the set of already created table
            // indices, then create the table and its columns and add it to the set.
            if (table_indices_.count(*table_ind_it) == 0)
            {
                // Create the output path and create the directory if it doesn't
                // exist.
                output_path = CreateTableOutputPath(output_dir_, output_base_path_,
                                                    table_name, thread_index);
                if(!CreateTableOutputDir(thread_index, output_path, table_name))
                    return TranslateStatus::FAIL;

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
            AppendTimeAndRawDataToTable(thread_index, table_index_to_table_map_[*table_ind_it],
                reinterpret_cast<const uint8_t*>(time_.data() + row_ind),
                reinterpret_cast<const uint8_t*>(raw_data_.data() + row_ind * raw_data_count_per_row_),
                raw_data_count_per_row_, table_name);
        }
    }
    return TranslateStatus::OK;
}

bool TranslateTabularContext1553::FillRowGroupVectors()
{
    int row_count = 0;

    // Read relevant columns into vectors.
    if (!FillRGVector<int64_t, arrow::NumericArray<arrow::Int64Type>>(time_,
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
    if (!AppendColumn<uint16_t, int64_t>(table_ptr, "time", true, icd_elem))
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

bool TranslateTabularContext1553::IsSelectedMessage(const size_t& thread_index, 
    bool should_select_msg, const std::set<size_t>& selected_tables, const size_t& table_index)
{
    if(should_select_msg)
    {
        if (selected_tables.count(table_index) == 0)
        {
            SPDLOG_TRACE("{:02d} table index {:d} not a selected table",
                            thread_index, table_index);
            return false;
        }
    }
    return true;
}
