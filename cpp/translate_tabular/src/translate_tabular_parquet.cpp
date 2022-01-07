#include "translate_tabular_parquet.h"

TranslateTabularParquet::TranslateTabularParquet() : TranslateTabularContextBase(), 
    pq_reader_(), input_row_group_count_(-1), row_group_index_(-1), output_dir_(""), 
    output_base_path_(""), current_row_group_row_count_(0) 

{ }

TranslateStatus TranslateTabularParquet::OpenInputFile(const ManagedPath& input_path,
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

ManagedPath TranslateTabularParquet::CreateTableOutputPath(const ManagedPath& output_dir,
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

TranslateStatus TranslateTabularParquet::OpenOutputFile(const ManagedPath& output_dir,
                                                            const ManagedPath& output_base_name, const size_t& thread_index)
{
    output_dir_ = output_dir;
    output_base_path_ = output_base_name;
    return TranslateStatus::OK;
}

TranslateStatus TranslateTabularParquet::ConsumeRowGroup(const size_t& thread_index)
{
    return TranslateStatus::OK;
}

TranslateStatus TranslateTabularParquet::ConsumeFile(const size_t& thread_index)
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

TranslateStatus TranslateTabularParquet::CloseOutputFile(
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


TranslateStatus TranslateTabularParquet::ReadRowGroup(const size_t& thread_index,
        const size_t& row_group_count, size_t& row_group_index)
{    
    if (row_group_index == row_group_count)
        return TranslateStatus::CONTINUE;

    if (!FillRowGroupVectors())
    {
        SPDLOG_WARN("{:02d} Failed to fill row group vectors", thread_index);
        return TranslateStatus::FAIL;
    }
    pq_reader_.IncrementRG();
    row_group_index++;
    SPDLOG_DEBUG("{:02d} Read {:d} parsed data rows", thread_index,
        current_row_group_row_count_);

    return TranslateStatus::OK;
}

bool TranslateTabularParquet::CreateTableOutputDir(const size_t& thread_index, 
    const ManagedPath& table_path, const std::string& table_name)
{
    SPDLOG_DEBUG("{:02d} Output file path: {:s}", thread_index,
        table_path.RawString());
    if (!table_path.parent_path().is_directory())
    {
        if (!table_path.parent_path().create_directory())
        {
            SPDLOG_ERROR(
                "{:02d} Failed to create directory for table \"{:s}\": "
                "{:s}",
                thread_index, table_name, table_path.parent_path().RawString());
            return false;
        }
    }
    return true;
}

bool TranslateTabularParquet::AppendTimeAndRawDataToTable(const size_t& thread_index, 
    std::shared_ptr<TranslatableTableBase> table, const uint8_t* time_data,
    const uint8_t* raw_data, const size_t& raw_data_count, const std::string& table_name)
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
        if (!table->AppendRawData(raw_data, raw_data_count))
        {
            SPDLOG_WARN(
                "{:02d} Failed to append raw data to columns of table for message: "
                "{:s}", thread_index, table_name);
        }
    }

    return true;
}
