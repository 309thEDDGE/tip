#include "parquet_context.h"

ParquetContext::ParquetContext() : ROW_GROUP_COUNT_(10000),
                                   have_created_table_(false),
                                   path_(""),
                                   have_created_writer_(false),
                                   pool_(nullptr),
                                   schema_(nullptr),
                                   append_row_count_(0),
                                   host_(""),
                                   user_(""),
                                   port_(-1),
                                   have_created_schema_(false),
                                   writer_(nullptr),
                                   parquet_stop_(false),
                                   truncate_(true),
                                   appended_row_count_(0),
                                   max_temp_element_count_(0),
                                   row_group_count_multiplier_(1),
                                   ready_for_automatic_tracking_(false),
                                   print_activity_(false),
                                   print_msg_(""),
                                   did_write_columns_(false),
                                   empty_file_deletion_enabled_(false),
                                   row_group_count(ROW_GROUP_COUNT_),
                                   parquet_stop(parquet_stop_)
{
}

ParquetContext::ParquetContext(int rgSize) : ROW_GROUP_COUNT_(rgSize),
                                             have_created_table_(false),
                                             path_(""),
                                             have_created_writer_(false),
                                             pool_(nullptr),
                                             schema_(nullptr),
                                             append_row_count_(0),
                                             host_(""),
                                             user_(""),
                                             port_(-1),
                                             have_created_schema_(false),
                                             writer_(nullptr),
                                             parquet_stop_(false),
                                             truncate_(true),
                                             appended_row_count_(0),
                                             max_temp_element_count_(0),
                                             row_group_count_multiplier_(1),
                                             ready_for_automatic_tracking_(false),
                                             print_activity_(false),
                                             print_msg_(""),
                                             did_write_columns_(false),
                                             empty_file_deletion_enabled_(false),
                                             row_group_count(ROW_GROUP_COUNT_),
                                             parquet_stop(parquet_stop_)
{
}

ParquetContext::~ParquetContext()
{
    Close();
}

void ParquetContext::Close(const uint16_t& thread_id)
{
    // If automatic row count tracking and writing
    // has been turned on, write the data remaining in the
    // buffers.
    if (ready_for_automatic_tracking_)
    {
        Finalize(thread_id);
    }

    if (have_created_writer_)
    {
        writer_->Close();
        ostream_->Close();
        have_created_writer_ = false;

        if (ready_for_automatic_tracking_ && !did_write_columns_)
        {
            SPDLOG_INFO("({:02d}) {:s}, Empty file: Zero rows written", thread_id,
                        print_msg_);

            if (empty_file_deletion_enabled_)
            {
                SPDLOG_INFO("({:02d}) {:s}, Automatic deletion enabled, deleting {:s}",
                            thread_id, print_msg_, path_.c_str());

                std::filesystem::path fspath(path_);
                if (!std::filesystem::remove(fspath))
                {
                    SPDLOG_ERROR("({:02d}) {:s}, Automatic deletion failed for {:s}",
                                 thread_id, print_msg_, path_.c_str());
                }
            }
        }
    }
    ready_for_automatic_tracking_ = false;
}

std::vector<int32_t>
ParquetContext::GetOffsetsVector(const int& n_rows,
                                 const int& elements_per_row,
                                 const int offset)
{
    std::vector<int32_t> offsets_vec(n_rows, 0);
    int _offset_ = offset * elements_per_row;
    for (int32_t i = 0; i < n_rows; i++)
    {
        offsets_vec[i] = i * elements_per_row + _offset_;
    }
    return offsets_vec;
}

bool ParquetContext::OpenForWrite(const std::string path, const bool truncate)
{
    // must have at least one column set with AddField
    // and SetMemoryLocation before writing
    if (fields_.size() == 0)
    {
        SPDLOG_CRITICAL("Must call AddField and SetMemoryLocation before calling OpenForWrite");
        return false;
    }

    // Check to see if all memory locations are set for each column
    for (std::map<std::string, ColumnData>::iterator
             it = column_data_map_.begin();
         it != column_data_map_.end();
         ++it)
    {
        if (!it->second.pointer_set_)
        {
            SPDLOG_CRITICAL("memory location for field not set: {:s}",
                            it->second.field_name_);
            parquet_stop_ = true;
            return false;
        }
    }

    if (parquet_stop_)
        return false;

    if (!have_created_writer_)
    {
#ifdef NEWARROW
        try
        {
            PARQUET_ASSIGN_OR_THROW(ostream_,
                                    arrow::io::FileOutputStream::Open(path));
        }
        catch (...)
        {
            SPDLOG_CRITICAL("FileOutputStream::Open error");
            return false;
        }
#else
        st_ = arrow::io::FileOutputStream::Open(path,
                                                !truncate_,
                                                &ostream_);

        if (!st_.ok())
        {
            SPDLOG_CRITICAL("FileOutputStream::Open error (ID {:s}): {:s}",
                            st_.CodeAsString(), st_.message());
            return false;
        }
#endif

        path_ = path;
        truncate_ = true;
        schema_ = arrow::schema(fields_);
        have_created_schema_ = true;
        CreateBuilders();
        // Create properties for the writer.
        parquet::WriterProperties::Builder props_builder;
        props_builder.compression(parquet::Compression::GZIP);
        props_builder.memory_pool(pool_);
        props_builder.enable_dictionary();

        // The only encoding that works
        props_builder.encoding(parquet::Encoding::PLAIN);
        props_builder.enable_statistics();
        props_ = props_builder.build();

        st_ = parquet::arrow::FileWriter::Open(*schema_, pool_,
                                               ostream_,
                                               props_,
                                               &writer_);

        if (!st_.ok())
        {
            SPDLOG_CRITICAL("parquet::arrow::FileWriter::Open error (ID {:s}): {:s}",
                            st_.CodeAsString(), st_.message());
            return false;
        }
        have_created_writer_ = true;
    }
    return true;
}

std::unique_ptr<arrow::ArrayBuilder>
ParquetContext::GetBuilderFromDataType(const std::shared_ptr<arrow::DataType> dtype,
                                       const bool& is_list_builder)
{
    switch (dtype->id())
    {
        case arrow::UInt64Type::type_id:
        {
            if (is_list_builder)
                return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::UInt64Builder>(pool_));
            else
                return std::make_unique<arrow::UInt64Builder>(pool_);
        }
        case arrow::Int64Type::type_id:
        {
            if (is_list_builder)
                return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::Int64Builder>(pool_));
            else
                return std::make_unique<arrow::Int64Builder>(pool_);
        }
        case arrow::Int32Type::type_id:
        {
            if (is_list_builder)
                return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::Int32Builder>(pool_));
            else
                return std::make_unique<arrow::Int32Builder>(pool_);
        }
        case arrow::UInt32Type::type_id:
        {
            if (is_list_builder)
                return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::UInt32Builder>(pool_));
            else
                return std::make_unique<arrow::UInt32Builder>(pool_);
        }
        case arrow::UInt16Type::type_id:
        {
            if (is_list_builder)
                return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::UInt16Builder>(pool_));
            else
                return std::make_unique<arrow::UInt16Builder>(pool_);
        }
        case arrow::Int16Type::type_id:
        {
            if (is_list_builder)
                return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::Int16Builder>(pool_));
            else
                return std::make_unique<arrow::Int16Builder>(pool_);
        }
        case arrow::UInt8Type::type_id:
        {
            if (is_list_builder)
                return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::UInt8Builder>(pool_));
            else
                return std::make_unique<arrow::UInt8Builder>(pool_);
        }
        case arrow::Int8Type::type_id:
        {
            if (is_list_builder)
                return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::Int8Builder>(pool_));
            else
                return std::make_unique<arrow::Int8Builder>(pool_);
        }
        case arrow::BooleanType::type_id:
        {
            if (is_list_builder)
                return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::BooleanBuilder>(pool_));
            else
                return std::make_unique<arrow::BooleanBuilder>(pool_);
        }
        case arrow::DoubleType::type_id:
        {
            if (is_list_builder)
                return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::DoubleBuilder>(pool_));
            else
                return std::make_unique<arrow::DoubleBuilder>(pool_);
        }
        case arrow::FloatType::type_id:
        {
            if (is_list_builder)
                return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::FloatBuilder>(pool_));
            else
                return std::make_unique<arrow::FloatBuilder>(pool_);
        }
        case arrow::StringType::type_id:
        {
            if (is_list_builder)
                return std::make_unique<arrow::ListBuilder>(pool_, std::make_shared<arrow::StringBuilder>(pool_));
            else
                return std::make_unique<arrow::StringBuilder>(pool_);
        }

        default:
            return std::make_unique<arrow::NullBuilder>(pool_);
    }
}

void ParquetContext::CreateBuilders()
{
    pool_ = arrow::default_memory_pool();
    arrow::NullType null_type;
    arrow::Type::type dtype = arrow::Type::NA;
    for (std::map<std::string, ColumnData>::iterator
             it = column_data_map_.begin();
         it != column_data_map_.end();
         ++it)
    {
        std::string fieldName = it->second.field_name_;
        dtype = it->second.type_->id();

        it->second.builder_ = GetBuilderFromDataType(
            it->second.type_, it->second.is_list_);
    }
}

bool ParquetContext::WriteColsIfReady()
{
    // Check if the table has been created. If not, then create it.
    if (!have_created_table_)
    {
#ifdef NEWARROW
        std::vector<std::shared_ptr<arrow::Array>> arr_vec;
#else
        arrow::ArrayVector arr_vec;
#endif
        // Loop over the builders and "Finish" them in order.
        for (int field_ind = 0; field_ind < schema_->num_fields(); field_ind++)
        {
            std::shared_ptr<arrow::Array> temp_array_ptr;

            st_ = column_data_map_[schema_->field(field_ind)->name()]
                      .builder_->Finish(&temp_array_ptr);

            if (!st_.ok())
            {
                SPDLOG_ERROR("\"Finish\" error (ID {:s}): {:s}",
                             st_.CodeAsString(), st_.message());
                return false;
            }
            arr_vec.push_back(temp_array_ptr);
        }

        // Make the Table and write it.
        std::shared_ptr<arrow::Table> table = arrow::Table::Make(schema_, arr_vec);
        st_ = writer_->WriteTable(*table, append_row_count_);
        if (!st_.ok())
        {
            SPDLOG_ERROR("WriteTable error (ID {:s}): {:s}",
                         st_.CodeAsString(), st_.message());
            return false;
        }
        have_created_table_ = true;

        // Debug, check if table has metadata.
        if (table->schema()->HasMetadata())
            SPDLOG_DEBUG("after being written, table has metadata");
#ifdef NEWARROW
        if (writer_->schema()->HasMetadata())
            SPDLOG_DEBUG("after writetable, writer has metadata");
#endif
    }
    else
    {
        // Write columns in order.
        writer_->NewRowGroup(append_row_count_);
        for (int field_ind = 0; field_ind < schema_->num_fields(); field_ind++)
        {
            std::shared_ptr<arrow::Array> temp_array_ptr;
            st_ = column_data_map_[schema_->field(field_ind)->name()].builder_->Finish(&temp_array_ptr);

            if (!st_.ok())
            {
                SPDLOG_ERROR("\"Finish\" error (ID {:s}): {:s}",
                             st_.CodeAsString(), st_.message());
                return false;
            }
            st_ = writer_->WriteColumnChunk(*temp_array_ptr);
            if (!st_.ok())
            {
                SPDLOG_ERROR("WriteColumnChunk error (ID {:s}): {:s}",
                             st_.CodeAsString(), st_.message());

                return false;
            }
        }
    }
    return true;
}

bool ParquetContext::AppendColumn(ColumnData& columnData,
                                  const int& rows,
                                  const int offset)
{
    int datatypeID = columnData.type_->id();
    bool isList = columnData.is_list_;
    bool castRequired;

    if (columnData.cast_from_ == CastFromType::TypeNONE)
        castRequired = false;
    else
    {
        SPDLOG_WARN("Cast required for column \"{:s}\"",
            columnData.field_name_);
        castRequired = true;
    }

    int listCount = 0;

    if (isList)
        listCount = columnData.list_size_;

    SPDLOG_TRACE("AppendColumn(): name {:s}, datatypename {:s}", columnData.field_name_,
                 columnData.type_->name());

    append_row_count_ = rows;

    if (rows + offset > columnData.initial_max_row_size_)
    {
        SPDLOG_CRITICAL(
            "initial vector does not"
            " contain enough information to write"
            " {:d} rows with a {:d} offset for column {:s}",
            rows, offset, columnData.type_->name());
        return false;
    }

    switch (datatypeID)
    {
        case arrow::Int64Type::type_id:
        {
            Append<int64_t, arrow::NumericBuilder<arrow::Int64Type>>(isList,
                                                                     castRequired,
                                                                     listCount,
                                                                     offset,
                                                                     columnData);
            break;
        }

        case arrow::UInt64Type::type_id:
        {
            Append<uint64_t, arrow::NumericBuilder<arrow::UInt64Type>>(isList,
                                                                       castRequired,
                                                                       listCount,
                                                                       offset,
                                                                       columnData);
            break;
        }

        case arrow::Int32Type::type_id:
        {
            Append<int32_t, arrow::NumericBuilder<arrow::Int32Type>>(isList,
                                                                     castRequired,
                                                                     listCount,
                                                                     offset,
                                                                     columnData);
            break;
        }

        case arrow::UInt32Type::type_id:
        {
            Append<uint32_t, arrow::NumericBuilder<arrow::UInt32Type>>(isList,
                                                                       castRequired,
                                                                       listCount,
                                                                       offset,
                                                                       columnData);
            break;
        }

        case arrow::Int16Type::type_id:
        {
            Append<int16_t, arrow::NumericBuilder<arrow::Int16Type>>(isList,
                                                                     castRequired,
                                                                     listCount,
                                                                     offset,
                                                                     columnData);
            break;
        }

        case arrow::UInt16Type::type_id:
        {
            Append<uint16_t, arrow::NumericBuilder<arrow::UInt16Type>>(isList,
                                                                       castRequired,
                                                                       listCount,
                                                                       offset,
                                                                       columnData);
            break;
        }

        case arrow::Int8Type::type_id:
        {
            Append<int8_t, arrow::NumericBuilder<arrow::Int8Type>>(isList,
                                                                   castRequired,
                                                                   listCount,
                                                                   offset,
                                                                   columnData);
            break;
        }

        case arrow::UInt8Type::type_id:
        {
            Append<uint8_t, arrow::NumericBuilder<arrow::UInt8Type>>(isList,
                                                                     castRequired,
                                                                     listCount,
                                                                     offset,
                                                                     columnData);
            break;
        }

        case arrow::DoubleType::type_id:
        {
            castRequired = false;
            Append<double, arrow::NumericBuilder<arrow::DoubleType>>(isList,
                                                                     castRequired,
                                                                     listCount,
                                                                     offset,
                                                                     columnData);
            break;
        }

        case arrow::FloatType::type_id:
        {
            castRequired = false;
            Append<float, arrow::NumericBuilder<arrow::FloatType>>(isList,
                                                                   castRequired,
                                                                   listCount,
                                                                   offset,
                                                                   columnData);
            break;
        }

        case arrow::BooleanType::type_id:
        {
            castRequired = false;
            Append<uint8_t, arrow::BooleanBuilder>(isList,
                                                   castRequired,
                                                   listCount,
                                                   offset,
                                                   columnData);
            break;
        }

        case arrow::StringType::type_id:
        {
            if (isList)
            {
                // Get the relevant builder for the data type.
                std::shared_ptr<arrow::ListBuilder> bldr =
                    std::dynamic_pointer_cast<arrow::ListBuilder>(columnData.builder_);

                // Resize array to allocate space and append data.
                bldr->Resize(append_row_count_);

                std::vector<int32_t> offsets_vec =
                    GetOffsetsVector(append_row_count_, listCount, 0);

                bldr->AppendValues(offsets_vec.data(), append_row_count_);
                arrow::StringBuilder* sub_bldr =
                    static_cast<arrow::StringBuilder*>(bldr->value_builder());

                // The only way to convey to Arrow the row count for the
                // StringBuilder class is via the implicit vector size.
                //
                // If the desired append row count is not equivalent
                // to initial_max_row_size_, a new string vector is built
                // with a size consistent with the desired row count.
                // For lists initial_max_row_size_ = initial vector size / list size
                // For non list columns initial_max_row_size_ = initial vector size
                if (append_row_count_ == columnData.initial_max_row_size_)
                {
                    sub_bldr->AppendValues(*columnData.str_ptr_);
                }
                else
                {
                    FillStringVec(columnData.str_ptr_,
                                  append_row_count_ * listCount,
                                  offset * listCount);

                    sub_bldr->AppendValues(temp_string_vec_);
                }
            }
            else
            {
                std::shared_ptr<arrow::StringBuilder> bldr =
                    std::dynamic_pointer_cast<arrow::StringBuilder>(columnData.builder_);

                // Resize array to allocate space and append data.
                bldr->Resize(append_row_count_);

                if (append_row_count_ == columnData.initial_max_row_size_)
                {
                    if (columnData.null_values_ == nullptr)
                    {
                        bldr->AppendValues(*columnData.str_ptr_);
                    }
                    else
                    {
                        bldr->AppendValues(*columnData.str_ptr_, columnData.null_values_->data() + offset);
                    }
                }
                else
                {
                    FillStringVec(columnData.str_ptr_, append_row_count_, offset);
                    if (columnData.null_values_ == nullptr)
                    {
                        bldr->AppendValues(temp_string_vec_);
                    }
                    else
                    {
                        bldr->AppendValues(temp_string_vec_, columnData.null_values_->data() + offset);
                    }
                }
            }
            break;
        }
        default:
            SPDLOG_CRITICAL("Data type not included: {:s}", columnData.type_->name());
            return false;
            break;
    }

    // Make note that this column has been updated by setting the relevant bool.
    columnData.ready_for_write_ = true;

    return true;
}

bool ParquetContext::AddField(const std::shared_ptr<arrow::DataType> type,
                              const std::string& fieldName,
                              int listSize)
{
    //printf("Add field %s\n", fieldName.c_str());
    bool isList;
    if (listSize == 0)
        isList = false;
    else
        isList = true;

    // Spark can't read unsigned data types from a parquet file
    if (IsUnsigned(type))
    {
        SPDLOG_CRITICAL("Unsigned types are currently not available for writing parquet");
        return false;
    }
    int byteSize;
    std::string typeID = GetTypeIDFromArrowType(type, byteSize);
    column_data_map_[fieldName] = ColumnData(type,
                                             fieldName,
                                             typeID,
                                             byteSize,
                                             listSize);

    std::shared_ptr<arrow::Field> tempField;

    if (isList)
    {
        tempField = arrow::field(fieldName, arrow::list(type));
        fields_.push_back(tempField);
    }
    else
    {
        tempField = arrow::field(fieldName, type);
        fields_.push_back(tempField);
    }

    return true;
}

bool ParquetContext::IsUnsigned(const std::shared_ptr<arrow::DataType> type)
{
    if (type->id() == arrow::UInt64Type::type_id ||
        type->id() == arrow::UInt32Type::type_id ||
        type->id() == arrow::UInt16Type::type_id ||
        type->id() == arrow::UInt8Type::type_id)
        return true;
    else
        return false;
}

bool ParquetContext::WriteColumns(const int& rows, const int offset)
{
    if (parquet_stop_)
    {
        SPDLOG_ERROR("parquetStop = true");
        return false;
    }

    if (!have_created_writer_)
    {
        SPDLOG_CRITICAL("must call OpenForWrite before calling WriteColumns");
        return false;
    }
    append_row_count_ = rows;
    bool ret_val = true;

    try
    {
        if (rows > 0)
        {
            for (std::map<std::string, ColumnData>::iterator
                     it = column_data_map_.begin();
                 it != column_data_map_.end();
                 ++it)
            {
                ret_val = AppendColumn(it->second, rows, offset);
                if (!ret_val)
                {
                    SPDLOG_CRITICAL("AppendColumn() failure");
                    return false;
                }
            }
        }

        if (!WriteColsIfReady())
        {
            SPDLOG_CRITICAL("WriteColsIfReady() failure");
            return false;
        }

        // Reset the status of each column.
        for (std::map<std::string, ColumnData>::iterator
                 it = column_data_map_.begin();
             it != column_data_map_.end();
             ++it)
        {
            it->second.ready_for_write_ = false;
        }
        return true;
    }

    catch (...)
    {
        SPDLOG_CRITICAL("WriteColumns(): Caught Exception");
        return false;
    }
}

bool ParquetContext::WriteColumns()
{
    return WriteColumns(ROW_GROUP_COUNT_, 0);
}

std::string ParquetContext::GetTypeIDFromArrowType(const std::shared_ptr<arrow::DataType> type,
                                                   int& byteSize)
{
    switch (type->id())
    {
        case arrow::UInt64Type::type_id:
        {
            uint64_t a;
            byteSize = sizeof(a);
            return typeid(a).name();
            break;
        }
        case arrow::Int64Type::type_id:
        {
            int64_t a;
            byteSize = sizeof(a);
            return typeid(a).name();
            break;
        }
        case arrow::Int32Type::type_id:
        {
            int32_t a;
            byteSize = sizeof(a);
            return typeid(a).name();
            break;
        }
        case arrow::UInt32Type::type_id:
        {
            uint32_t a;
            byteSize = sizeof(a);
            return typeid(a).name();
            break;
        }
        case arrow::UInt16Type::type_id:
        {
            uint16_t a;
            byteSize = sizeof(a);
            return typeid(a).name();
            break;
        }
        case arrow::Int16Type::type_id:
        {
            int16_t a;
            byteSize = sizeof(a);
            return typeid(a).name();
            break;
        }
        case arrow::UInt8Type::type_id:
        {
            uint8_t a;
            byteSize = sizeof(a);
            return typeid(a).name();
            break;
        }
        case arrow::Int8Type::type_id:
        {
            int8_t a;
            byteSize = sizeof(a);
            return typeid(a).name();
            break;
        }
        // Note that boolean types need to come in
        // as uint8_t
        case arrow::BooleanType::type_id:
        {
            uint8_t a;
            byteSize = sizeof(a);
            return typeid(a).name();
            break;
        }
        case arrow::DoubleType::type_id:
        {
            double a;
            byteSize = sizeof(a);
            return typeid(a).name();
            break;
        }
        case arrow::FloatType::type_id:
        {
            float a;
            byteSize = sizeof(a);
            return typeid(a).name();
            break;
        }
        case arrow::StringType::type_id:
        {
            std::string a;
            byteSize = sizeof(a);
            return typeid(a).name();
            break;
        }

        default:
            SPDLOG_ERROR("Data type not included in add field");
            return "NA";
            break;
    }
}

void ParquetContext::FillStringVec(std::vector<std::string>* str_data_vec_ptr,
                                   const int& count,
                                   const int offset)
{
    if (temp_string_vec_.size() != count)
        temp_string_vec_.resize(count);

    std::copy(str_data_vec_ptr->data() + offset,
              str_data_vec_ptr->data() + offset + count,
              temp_string_vec_.data());
}

bool ParquetContext::IncrementAndWrite(const uint16_t& thread_id)
{
    // Increment appended row counter.
    appended_row_count_++;

    // If the buffer is full, write the data to disk.
    if (appended_row_count_ == max_temp_element_count_)
    {
        if (print_activity_)
        {
            SPDLOG_INFO("({:02d}) {:s}: Writing {:d} rows", thread_id,
                        print_msg_, appended_row_count_);
        }

        // Indicate that some data have been written.
        did_write_columns_ = true;

        // Write each of the row groups.
        bool result = false;
        for (int i = 0; i < row_group_count_multiplier_; i++)
        {
            //printf("writecolumns(%zu, %zu)\n", ROW_GROUP_COUNT_, i * ROW_GROUP_COUNT_);
            result = WriteColumns(ROW_GROUP_COUNT_, i * ROW_GROUP_COUNT_);
            if (!result)
                SPDLOG_ERROR("WriteColumns() failure");
        }

        // Reset
        appended_row_count_ = 0;

        // Return true to indicate that buffers were written.
        return true;
    }
    return false;
}

bool ParquetContext::SetupRowCountTracking(size_t row_group_count,
                                           size_t row_group_count_multiplier,
                                           bool print_activity, std::string print_msg)
{
    // Do not allow unreasonable values.
    if (row_group_count < 1 || row_group_count_multiplier < 1)
    {
        SPDLOG_ERROR("row_group_count ({:d}) < 1 || row_group_count_multiplier ({:d}) < 1",
                     row_group_count, row_group_count_multiplier);
        ready_for_automatic_tracking_ = false;
        parquet_stop_ = true;
        return ready_for_automatic_tracking_;
    }

    ROW_GROUP_COUNT_ = row_group_count;
    row_group_count_multiplier_ = row_group_count_multiplier;
    max_temp_element_count_ = row_group_count_multiplier_ * ROW_GROUP_COUNT_;
    print_activity_ = print_activity;
    print_msg_ = print_msg;

    // Ensure that max_temp_element_count does not exceed the allocated
    // size of the buffers, i.e., the allocated size of the vector that
    // represents each column. Loop over the map of ColumnData objects
    // and check that this is true for each.
    for (std::map<std::string, ColumnData>::iterator it = column_data_map_.begin();
         it != column_data_map_.end(); ++it)
    {
        if (max_temp_element_count_ > it->second.initial_max_row_size_)
        {
            parquet_stop_ = true;
            ready_for_automatic_tracking_ = false;
            return ready_for_automatic_tracking_;
        }
    }

    // Reset appended row count if parameters are valid.
    appended_row_count_ = 0;
    ready_for_automatic_tracking_ = true;
    return ready_for_automatic_tracking_;
}

void ParquetContext::Finalize(const uint16_t& thread_id)
{
    if (appended_row_count_ > 0)
    {
        if (print_activity_)
        {
            SPDLOG_INFO("({:02d}) {:s}, Writing {:d} rows", thread_id,
                        print_msg_, appended_row_count_);
        }

        int n_calls = static_cast<int>(std::ceil(static_cast<double>(appended_row_count_) / static_cast<double>(ROW_GROUP_COUNT_)));
        SPDLOG_DEBUG("({:02d}) {:s}, {:d} row groups", thread_id,
                     print_msg_, n_calls);
        for (int i = 0; i < n_calls; i++)
        {
            if (i == n_calls - 1)
            {
                SPDLOG_DEBUG("({:02d}) {:s}, WriteColumns(count = {:d}, offset = {:d})",
                             thread_id, print_msg_,
                             appended_row_count_ - (n_calls - 1) * ROW_GROUP_COUNT_,
                             i * ROW_GROUP_COUNT_);

                if (!WriteColumns(appended_row_count_ - (n_calls - 1) * ROW_GROUP_COUNT_, i * ROW_GROUP_COUNT_))
                {
                    SPDLOG_ERROR("({:02d}) {:s}, WriteColumns() failure",
                                 thread_id, print_msg_);
                }
            }
            else
            {
                SPDLOG_DEBUG("({:02d}) {:s}, WriteColumns(count = {:d}, offset = {:d})",
                             thread_id, print_msg_, ROW_GROUP_COUNT_, i * ROW_GROUP_COUNT_);

                if (!WriteColumns(ROW_GROUP_COUNT_, i * ROW_GROUP_COUNT_))
                {
                    SPDLOG_ERROR("({:02d}) {:s}, WriteColumns() failure", thread_id,
                                 print_msg_);
                }
            }
        }

        appended_row_count_ = 0;

        // Indicate that data were written.
        did_write_columns_ = true;
    }
}

void ParquetContext::EnableEmptyFileDeletion(const std::string& path)
{
    path_ = path;
    empty_file_deletion_enabled_ = true;
}

bool ParquetContext::GetColumnDataByField(const std::string& field,
    std::map<std::string, ColumnData>& col_data_map, ColumnData*& col_data)
{
    for (std::map<std::string, ColumnData>::iterator it = col_data_map.begin();
         it != col_data_map.end(); ++it)
    {
        if (it->first == field)
        {
            col_data = &(it->second);
            return true;
        }
    }
    SPDLOG_CRITICAL("Field name doesn't exist: {:s}", field);
    parquet_stop_ = true;
    return false;
}

bool ParquetContext::SetMemLocI64(std::vector<int64_t>& data, const std::string& fieldName,
                        std::vector<uint8_t>* boolField)
{
    ColumnData* col_data = nullptr;
    if(!GetColumnDataByField(fieldName, column_data_map_, col_data))
        return false;

    return SetColumnMemoryLocation(data, col_data, boolField);
}

bool ParquetContext::SetMemLocI32(std::vector<int32_t>& data, const std::string& fieldName,
                        std::vector<uint8_t>* boolField)
{
    ColumnData* col_data = nullptr;
    if(!GetColumnDataByField(fieldName, column_data_map_, col_data))
        return false;

    return SetColumnMemoryLocation(data, col_data, boolField);
}

bool ParquetContext::SetMemLocI16(std::vector<int16_t>& data, const std::string& fieldName,
                        std::vector<uint8_t>* boolField)
{
    ColumnData* col_data = nullptr;
    if(!GetColumnDataByField(fieldName, column_data_map_, col_data))
        return false;

    return SetColumnMemoryLocation(data, col_data, boolField);
}

bool ParquetContext::SetMemLocI8(std::vector<int8_t>& data, const std::string& fieldName,
                        std::vector<uint8_t>* boolField)
{
    ColumnData* col_data = nullptr;
    if(!GetColumnDataByField(fieldName, column_data_map_, col_data))
        return false;

    return SetColumnMemoryLocation(data, col_data, boolField);
}

bool ParquetContext::SetMemLocString(std::vector<std::string>& data, const std::string& fieldName,
                        std::vector<uint8_t>* boolField)
{
    ColumnData* col_data = nullptr;
    if(!GetColumnDataByField(fieldName, column_data_map_, col_data))
        return false;

    return SetColumnMemoryLocation(data, col_data, boolField);
}

bool ParquetContext::SetMemLocUI8(std::vector<uint8_t>& data, const std::string& fieldName,
                        std::vector<uint8_t>* boolField)
{
    ColumnData* col_data = nullptr;
    if(!GetColumnDataByField(fieldName, column_data_map_, col_data))
        return false;

    return SetColumnMemoryLocation(data, col_data, boolField);
}
