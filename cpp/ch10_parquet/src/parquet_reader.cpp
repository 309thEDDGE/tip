#include "parquet_reader.h"

bool ParquetReader::OpenNextParquetFile()
{
    if (arrow_file_ != nullptr)
    {
        if (!arrow_file_->closed())
            arrow_file_->Close();
    }

    if (current_file_ >= input_parquet_paths_.size())
        return false;

    ManagedPath file_path = input_parquet_paths_[current_file_];
    //printf("Reading parquet file: %s\n", file_path.RawString().c_str());
    current_row_group_ = 0;

    // Open file reader.
#ifdef NEWARROW
    try
    {
        PARQUET_ASSIGN_OR_THROW(arrow_file_,
                                arrow::io::ReadableFile::Open(file_path.string(), pool_));
    }
    catch (...)
    {
        printf("arrow::io::ReadableFile::Open error\n");
        return false;
    }
#else
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
#ifndef NEWARROW
    arrow_reader_->set_num_threads(2);
#endif

    std::shared_ptr<arrow::Schema> temp_schema;
    st_ = arrow_reader_->GetSchema(&temp_schema);
    if (!st_.ok())
    {
        printf("GetSchema() error (ID %s): %s\n",
               st_.CodeAsString().c_str(), st_.message().c_str());
        return false;
    }

    // Total count of row groups.
    row_group_count_ = arrow_reader_->num_row_groups();
    //printf("%02d row groups\n", row_group_count_);

    current_file_++;
    return true;
}

bool ParquetReader::SetPQPath(ManagedPath base_path)
{
    current_row_group_ = 0;
    current_file_ = 0;
    row_group_count_ = 0;
    input_parquet_paths_.clear();
    bool first_iteration = true;

    // Get list of files and dirs in base path.
    std::vector<ManagedPath> pq_paths_list;
    bool list_dir_success = false;
    base_path.ListDirectoryEntries(list_dir_success, pq_paths_list);
    if (!list_dir_success)
    {
        printf("Invalid directory %s: \n", base_path.c_str());
        return false;
    }

    // Exclude dirs and include only files with ".parquet" sub strings.
    std::vector<std::string> select_substr({".parquet"});
    pq_paths_list = ManagedPath::SelectPathsWithSubString(
        ManagedPath::SelectFiles(pq_paths_list), select_substr);

    // Iterate over each .parquet file and check schema and
    // row group count information before adding the current
    // file to the list that is to be compared.
    for (std::vector<ManagedPath>::const_iterator it = pq_paths_list.cbegin();
         it != pq_paths_list.cend(); ++it)
    {
        arrow::Status st;
        arrow::MemoryPool* pool = arrow::default_memory_pool();
        std::shared_ptr<arrow::io::ReadableFile> arrow_file;
        std::unique_ptr<parquet::arrow::FileReader> arrow_reader;

        // Check to see if the parquet file is valid and if all the
        // schema line up. It is assumed that the first parquet file
        // contains the correct schema
        if (arrow_file != nullptr)
        {
            if (!arrow_file->closed())
                arrow_file->Close();
        }

        // Open file reader.
#ifdef NEWARROW
        try
        {
            PARQUET_ASSIGN_OR_THROW(arrow_file,
                                    arrow::io::ReadableFile::Open(it->string(), pool));
        }
        catch (...)
        {
            printf("arrow::io::ReadableFile::Open error\n");
            if (arrow_file != nullptr)
            {
                if (!arrow_file->closed())
                    arrow_file->Close();
            }
            return false;
        }
#else
        st = arrow::io::ReadableFile::Open(it->string(), pool, &arrow_file);
        if (!st.ok())
        {
            printf("arrow::io::ReadableFile::Open error (ID %s): %s\n",
                   st.CodeAsString().c_str(), st.message().c_str());
            if (arrow_file != nullptr)
            {
                if (!arrow_file->closed())
                    arrow_file->Close();
            }
            return false;
        }
#endif
        st = parquet::arrow::OpenFile(arrow_file, pool, &arrow_reader);
        if (!st.ok())
        {
            printf("parquet::arrow::OpenFile error (ID %s): %s\n",
                   st.CodeAsString().c_str(), st.message().c_str());
            if (arrow_file != nullptr)
            {
                if (!arrow_file->closed())
                    arrow_file->Close();
            }
            return false;
        }

        arrow_reader->set_use_threads(true);
#ifndef NEWARROW
        arrow_reader->set_num_threads(2);
#endif
        // Get schema from the first parquet file and save
        // for furture use
        std::shared_ptr<arrow::Schema> compare_schema;
        if (first_iteration)
        {
            st = arrow_reader->GetSchema(&schema_);
        }
        // Compare schema to the first files schema
        else
        {
            st = arrow_reader->GetSchema(&compare_schema);
        }

        if (!st.ok())
        {
            printf("GetSchema() error (ID %s): %s\n",
                   st.CodeAsString().c_str(), st.message().c_str());
            if (arrow_file != nullptr)
            {
                if (!arrow_file->closed())
                    arrow_file->Close();
            }
            return false;
        }

        // if schema was read in correctly set
        // first iteration to false
        if (first_iteration)
        {
            first_iteration = false;
        }
        else
        {
            // compare schema count, types and names
            if (schema_->fields().size() != compare_schema->fields().size())
            {
                printf("Error!!! Inconsistent column sizes\n");
                input_parquet_paths_.clear();
                if (arrow_file != nullptr)
                {
                    if (!arrow_file->closed())
                        arrow_file->Close();
                }
                return false;
            }

            for (int i = 0; i < schema_->fields().size(); i++)
            {
                // check data type consistency
                if (schema_->fields()[i]->type()->id() !=
                    compare_schema->fields()[i]->type()->id())
                {
                    printf("Error!!! Inconsistent column types for column %i\n%s,%s\n",
                           i, schema_->fields()[i]->type()->name().c_str(),
                           compare_schema->fields()[i]->type()->name().c_str());
                    input_parquet_paths_.clear();
                    if (arrow_file != nullptr)
                    {
                        if (!arrow_file->closed())
                            arrow_file->Close();
                    }
                    return false;
                }

                // check column name consistency
                if (schema_->fields()[i]->name() !=
                    compare_schema->fields()[i]->name())
                {
                    printf("Error!!! Inconsistent column names for column %i\n%s,%s\n",
                           i, schema_->fields()[i]->name().c_str(),
                           compare_schema->fields()[i]->name().c_str());
                    input_parquet_paths_.clear();
                    if (arrow_file != nullptr)
                    {
                        if (!arrow_file->closed())
                            arrow_file->Close();
                    }
                    return false;
                }
            }
        }

        // Total count of row groups.
        int64_t row_group_count = arrow_reader->num_row_groups();

        // only add the parquet file if the row group count is > 0
        if (row_group_count > 0)
            input_parquet_paths_.push_back(*it);

        if (arrow_file != nullptr)
        {
            if (!arrow_file->closed())
                arrow_file->Close();
        }
    }

    // sort the paths
    //std::sort(input_parquet_paths_.begin(), input_parquet_paths_.end());

    // If a valid parquet file with valid schema was found
    // first_iteration will = false. If first_iteration
    // was never set to false, no valid parquet files were found
    if (first_iteration)
    {
        return false;
    }

    // SetPQPath should return true if there was a valid parquet
    // file even if the file didn't have data. If a parquet file
    // doesn't have data, it is not saved to input_parquet_paths
    // Thus, if first_iteration is false and input_parquet_paths.size == 0
    // SetPQPath should still return true, but it shouldn't
    // initialize the first parquet file (because there isn't
    // a file with data to initialize). OpenNextParuqetFile returns
    // false where there isn't a parquet file to open.
    if (input_parquet_paths_.size() > 0)
        return OpenNextParquetFile();
    else
        return true;
}

bool ParquetReader::SetPQPath(std::string base_path)
{
    ManagedPath mp(base_path);
    return SetPQPath(mp);
}

bool ParquetReader::GetNextRGBool(int col, std::vector<uint8_t>& data,
                                  int& size,
                                  bool list)
{
    size = 0;

    if (col >= schema_->num_fields())
        return false;

    if (current_row_group_ >= row_group_count_)
    {
        if (!OpenNextParquetFile())
            return false;
    }

    //printf("\rExtracting row group %03d\n", (current_row_group_ + 1));

    // Read row group from first file
    std::shared_ptr<arrow::Table> arrow_table;
    st_ = arrow_reader_->ReadRowGroup(current_row_group_,
                                      std::vector<int>({col}),
                                      &arrow_table);

    if (!st_.ok())
    {
        printf("arrow::io::ReadableFile::ReadRowGroup error (ID %s): %s\n",
               st_.CodeAsString().c_str(), st_.message().c_str());
        return false;
    }

    if (list)
    {
#ifdef NEWARROW
        arrow::ListArray data_list_arr =
            arrow::ListArray(arrow_table->column(0)->chunk(0)->data());
#else
        arrow::ListArray data_list_arr =
            arrow::ListArray(arrow_table->column(0)->data()->chunk(0)->data());
#endif

        arrow::BooleanArray data_array =
            arrow::BooleanArray(data_list_arr.values()->data());

        size = data_array.length();

        if (data.size() < size)
            data.resize(size);

        for (int i = 0; i < size; i++)
        {
            data[i] = data_array.Value(i);
        }
    }
    else
    {
#ifdef NEWARROW
        arrow::BooleanArray data_array =
            arrow::BooleanArray(arrow_table->column(0)->chunk(0)->data());
#else
        arrow::BooleanArray data_array =
            arrow::BooleanArray(arrow_table->column(0)->data()->chunk(0)->data());
#endif

        size = data_array.length();

        if (data.size() < size)
            data.resize(size);

        int null_count = data_array.null_count();

        if (null_count > 0)
        {
            for (int i = 0; i < size; i++)
            {
                data[i] = data_array.Value(i);
                // Arrow doesn't insert consistent
                // data when a row is NULL
                // The value in that row is overridden
                // for this reason to ensure comparisons match
                // later on down the road
                if (data_array.IsNull(i))
                    data[i] = NULL;
            }
        }
        else
        {
            for (int i = 0; i < size; i++)
            {
                data[i] = data_array.Value(i);
            }
        }
    }

    if (!manual_rowgroup_increment_mode_)
        current_row_group_++;
    return true;
}

bool ParquetReader::GetNextRGString(int col, std::vector<std::string>& data,
                                    int& size,
                                    bool list)
{
    size = 0;

    if (col >= schema_->num_fields())
        return false;

    if (current_row_group_ >= row_group_count_)
    {
        if (!OpenNextParquetFile())
            return false;
    }

    //printf("\rExtracting row group %03d\n", (current_row_group_ + 1));

    // Read row group from first file
    std::shared_ptr<arrow::Table> arrow_table;
    st_ = arrow_reader_->ReadRowGroup(current_row_group_,
                                      std::vector<int>({col}),
                                      &arrow_table);

    if (!st_.ok())
    {
        printf("arrow::io::ReadableFile::ReadRowGroup error (ID %s): %s\n",
               st_.CodeAsString().c_str(), st_.message().c_str());
        return false;
    }

    if (list)
    {
#ifdef NEWARROW
        arrow::ListArray data_list_arr =
            arrow::ListArray(arrow_table->column(0)->chunk(0)->data());
#else
        arrow::ListArray data_list_arr =
            arrow::ListArray(arrow_table->column(0)->data()->chunk(0)->data());
#endif

        arrow::StringArray data_array =
            arrow::StringArray(data_list_arr.values()->data());

        size = data_array.length();

        if (data.size() < size)
            data.resize(size);

        // Unlike other data types arrow
        // seems to insert null values consistently
        // into string fields, null value
        // override was not necessary for
        // strings
        for (int i = 0; i < size; i++)
        {
            data[i] = data_array.GetString(i);
        }
    }
    else
    {
#ifdef NEWARROW
        arrow::StringArray data_array =
            arrow::StringArray(arrow_table->column(0)->chunk(0)->data());
#else
        arrow::StringArray data_array =
            arrow::StringArray(arrow_table->column(0)->data()->chunk(0)->data());
#endif

        size = data_array.length();

        if (data.size() < size)
            data.resize(size);

        for (int i = 0; i < size; i++)
        {
            data[i] = data_array.GetString(i);
        }
    }

    if (!manual_rowgroup_increment_mode_)
        current_row_group_++;
    return true;
}

int ParquetReader::GetColumnNumberFromName(std::string col_name)
{
    return schema_->GetFieldIndex(col_name);
}
