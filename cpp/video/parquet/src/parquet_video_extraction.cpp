#include "parquet_video_extraction.h"

bool ParquetVideoExtraction::OpenParquetFile(ManagedPath file_path)
{
    std::string data_col_name = "data";
    std::string channel_id_col_name = "channelid";

#ifdef NEWARROW
    try
    {
        PARQUET_ASSIGN_OR_THROW(arrow_file_, arrow::io::ReadableFile::Open(file_path.string(), pool_));
    }
    catch (...)
    {
        printf("ReadableFile::Open error\n");
        return false;
    }
#else
    // Open file reader.
    st_ = arrow::io::ReadableFile::Open(file_path.string(), pool_, &arrow_file_);
    if (!st_.ok())
    {
        printf("arrow::io::ReadableFile::Open error (ID %s): %s\n",
               st_.CodeAsString().c_str(), st_.message().c_str());
        return false;
    }
#endif
    st_ = parquet::arrow::OpenFile(arrow_file_, pool_, &arrow_reader_);
    if (!st_.ok())
    {
        printf("parquet::arrow::OpenFile error (ID %s): %s\n",
               st_.CodeAsString().c_str(), st_.message().c_str());
        return false;
    }

    arrow_reader_->set_use_threads(true);

    // Get schema.
    st_ = arrow_reader_->GetSchema(&schema_);
    if (!st_.ok())
    {
        printf("GetSchema() error (ID %s): %s\n",
               st_.CodeAsString().c_str(), st_.message().c_str());
        return false;
    }

    // Total count of row groups.
    row_group_count_ = arrow_reader_->num_row_groups();
    printf("%02d row groups\n", row_group_count_);

    // Get the column index for data
    int temp_index = schema_->GetFieldIndex(data_col_name);

    if (temp_index == -1)
    {
        printf("GetFieldIndex() error: field name %s does not exist\n",
               data_col_name.c_str());
        return false;
    }
    else
        data_column_index_ = temp_index;

    // Get the column index for channel id
    temp_index = schema_->GetFieldIndex(channel_id_col_name);

    if (temp_index == -1)
    {
        printf("GetFieldIndex() error: field name %s does not exist\n",
               channel_id_col_name.c_str());
        return false;
    }
    else
        channel_id_index_ = temp_index;

    return true;
}

bool ParquetVideoExtraction::ExtractFileTS()
{
    for (int row_group = 0; row_group < row_group_count_; row_group++)
    {
        printf("\rExtracting row group %03d", (row_group + 1));

        // Read row group with only the data column
        std::shared_ptr<arrow::Table> arrow_table;
        st_ = arrow_reader_->ReadRowGroup(row_group,
                                          std::vector<int>({data_column_index_, channel_id_index_}),
                                          &arrow_table);

        if (!st_.ok())
        {
            printf("arrow::io::ReadableFile::ReadRowGroup error (ID %s): %s\n",
                   st_.CodeAsString().c_str(), st_.message().c_str());
            return false;
        }

#ifdef NEWARROW
        arrow::ListArray data_list_arr =
            arrow::ListArray(arrow_table->column(0)->chunk(0)->data());
#else
        arrow::ListArray data_list_arr =
            arrow::ListArray(arrow_table->column(0)->data()->chunk(0)->data());
#endif

        arrow::NumericArray<arrow::Int32Type> data_arr =
            arrow::NumericArray<arrow::Int32Type>(data_list_arr.values()->data());

#ifdef NEWARROW
        arrow::NumericArray<arrow::Int32Type> channel_ids =
            arrow::NumericArray<arrow::Int32Type>(arrow_table->column(1)->chunk(0)->data());
#else
        arrow::NumericArray<arrow::Int32Type> channel_ids =
            arrow::NumericArray<arrow::Int32Type>(arrow_table->column(1)->data()->chunk(0)->data());
#endif

        WriteRowGroup(data_arr, channel_ids);
    }

    return true;
}

void ParquetVideoExtraction::WriteRowGroup(const arrow::NumericArray<arrow::Int32Type>& data_arr,
                                           const arrow::NumericArray<arrow::Int32Type>& channel_ids)
{
    int row_count = channel_ids.length();

    // Array size, is the length of the full array divided by
    // the total number of rows in the current row group
    int max_array_size = data_arr.length();
    int array_size = max_array_size / row_count;

    const int32_t* raw_data_ptr = data_arr.raw_values();
    int32_t last_channel_id = -1;
    int32_t current_channel_id = -1;
    int* buffer_length;
    uint16_t* write_buffer_ptr;

    // Ensure maps are large enough
    for (std::unordered_map<int32_t, std::vector<uint16_t>>::iterator it = write_buffer_.begin();
         it != write_buffer_.end();
         ++it)
    {
        if (it->second.size() < max_array_size)
        {
            it->second.resize(max_array_size);
        }
    }

    for (int row = 0; row < row_count; row++)
    {
        current_channel_id = channel_ids.Value(row);

        // If the file stream has not been created for a specific channel ID
        // create it
        if (current_channel_id != last_channel_id)
        {
            if (video_streams_.count(current_channel_id) < 1)
            {
                std::ofstream* os = new std::ofstream();

                // Create the file name with the channel ID
                std::string temp;
                temp = "video_channel_id_" +
                       std::to_string(current_channel_id) + ".ts";

                std::string file_path = (output_path_ / temp).string();

                // Open the file stream
                os->open(file_path, std::ios_base::binary);

                // Set the video stream map
                video_streams_[current_channel_id] = os;

                // Set buffer maps
                write_buffer_[current_channel_id] =
                    std::vector<uint16_t>(max_array_size);

                buffer_lengths_[current_channel_id] = 0;
            }

            last_channel_id = current_channel_id;

            // Pointer switch to a different channel id length
            buffer_length = &buffer_lengths_[current_channel_id];

            // Pointer switch to a different write buffer
            write_buffer_ptr = write_buffer_[current_channel_id].data();
        }

        // Copy transport stream to buffer
        std::copy(raw_data_ptr + row * array_size,
                  raw_data_ptr + (row + 1) * array_size,
                  write_buffer_ptr + *buffer_length);

        // Add the amount copied to the write buffer for the channel id
        *buffer_length += array_size;
    }

    // Iterate over maps and write to file
    for (std::unordered_map<int32_t, std::vector<uint16_t>>::iterator it = write_buffer_.begin();
         it != write_buffer_.end();
         ++it)
    {
        int buffer_length = buffer_lengths_[it->first] / 2;
        uint32_t* temp_ptr = reinterpret_cast<uint32_t*>(it->second.data());

        for (int i = 0; i < buffer_length; i++)
        {
            // 16 bit shift
            //temp_ptr[i] = temp_ptr[i] << 8 | temp_ptr[i] >> 8;

            // Equivalent of a 16 bit endian swap, but 32 bits at a time
            temp_ptr[i] =
                ((temp_ptr[i] & 0b11111111000000001111111100000000) >> 8) | 
                ((temp_ptr[i] & 0b00000000111111110000000011111111) << 8);
        }

        video_streams_[it->first]->write(reinterpret_cast<char*>(temp_ptr), buffer_length * sizeof(uint32_t));
        buffer_lengths_[it->first] = 0;
    }
}

int ParquetVideoExtraction::Initialize(ManagedPath video_path, ManagedPath output_dir)
{
    if (!video_path.is_directory())
    {
        printf("Video directory %s does not exist\n", video_path.RawString().c_str());
        return 66;
    }

    if(!output_dir.is_directory())
    {
        printf("Output directory %s does not exist\n", output_dir.RawString().c_str());
        return 66;
    }

    parquet_path_ = video_path;
    std::string ext_replacement = "_TS";
    output_path_ = output_dir.CreatePathObject(parquet_path_, ext_replacement);

    printf("\nTransport Stream Extractor--\nInput parquet path: %s\n", parquet_path_.RawString().c_str());
    printf("Output directory: %s\n", output_path_.RawString().c_str());

    if (!output_path_.create_directory())
    {
        printf("Creation of directory %s failed\n", output_path_.RawString().c_str());
        return 73;
    }

    return 0;
}

int ParquetVideoExtraction::ExtractTS()
{
    // Get list of entries in the parquet_path_ directory and
    // select only those which are files and contain the substring
    // ".parquet".
    std::vector<std::string> substr({".parquet"});
    std::vector<ManagedPath> dir_entries;
    bool success = false;
    parquet_path_.ListDirectoryEntries(success, dir_entries);

    if (!success)
    {
        printf("Failed to get list of directory entries from %s\n",
               parquet_path_.RawString().c_str());
        return 74;
    }

    std::vector<ManagedPath> matching_files = ManagedPath::SelectPathsWithSubString(
        ManagedPath::SelectFiles(dir_entries), substr);

    for (std::vector<ManagedPath>::const_iterator it = matching_files.cbegin();
         it != matching_files.cend(); ++it)
    {
        printf("\n\n--Reading parquet file: \n%s\n", it->stem().RawString().c_str());
        if (!OpenParquetFile(*it))
        {
            return 74;
        }

        if (!ExtractFileTS())
        {
            return 70;
        }
    }

    printf("\n\n --Finished\nTransport Stream Output Written to: %s\n",
           output_path_.RawString().c_str());
    return 0;
}
