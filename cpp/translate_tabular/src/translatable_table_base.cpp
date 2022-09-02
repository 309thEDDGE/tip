#include "translatable_table_base.h"

TranslatableTableBase::TranslatableTableBase() : is_valid_(false), is_valid(is_valid_), name_(""), row_group_size_(0), name(name_), row_group_size(row_group_size_), append_count_(0), ridealong_col_name_to_index_map(ridealong_col_name_to_index_map_), output_path_(""), output_path(output_path_), pq_ctx_(nullptr), is_pqctx_configured_(false), is_pqctx_configured(is_pqctx_configured_), icd_translate_(), thread_index_(0), thread_index(thread_index_)
{
    icd_translate_.SetAutomaticResizeOutputVector(false);
}

TranslatableTableBase::~TranslatableTableBase()
{
    if (pq_ctx_ != nullptr)
        pq_ctx_->Close();
}

bool TranslatableTableBase::Configure(const std::string& name, size_t row_group_size,
                                      const ManagedPath& output_path, const size_t& thread_index)
{
    if (name == "")
    {
        SPDLOG_WARN("{:02d} name is an empty string", thread_index);
        return false;
    }

    if (row_group_size <= 0)
    {
        SPDLOG_WARN("{:02d} row_group_size <= 0", thread_index);
        return false;
    }

    name_ = name;
    row_group_size_ = row_group_size;
    is_valid_ = true;
    output_path_ = output_path;
    thread_index_ = thread_index;

    return true;
}

const std::shared_ptr<TranslatableColumnBase> TranslatableTableBase::GetColumnByIndex(
    size_t index)
{
    if (index < columns_.size())
        return columns_.at(index);

    return nullptr;
}

void TranslatableTableBase::LogSchema()
{
    SPDLOG_DEBUG("{:02d} Table \"{:s}\": rowgroupsize({:d}):", thread_index_,
                 name_, row_group_size_);
    for (size_t i = 0; i < columns_.size(); i++)
    {
        columns_[i]->LogSchema();
    }
}

bool TranslatableTableBase::AppendRawData(const uint8_t* data, const size_t& count)
{
    _AppendToColumns(data, count);
    return _IncrementAndTranslate();
}

void TranslatableTableBase::_AppendToColumns(const uint8_t* data, const size_t& count)
{
    for (std::vector<std::shared_ptr<TranslatableColumnBase>>::iterator
             it = columns_.begin();
         it != columns_.end(); ++it)
    {
        // Use the built-in checks to handle correct column types (not ridealong),
        // sufficient count in data vector, etc. Therefore, do not check return type
        (*it)->AppendRawData(data, count);
    }
}

bool TranslatableTableBase::_IncrementAndTranslate()
{
    // If the append count is equal to row group size, translate and
    // write the data.
    // Note that this assumes all ridealong columns have already been copied.
    append_count_++;
    if (append_count_ == row_group_size_)
    {
        SPDLOG_DEBUG("{:02d} Translating columns of table: {:s}", thread_index_, name_);

        for (std::vector<std::shared_ptr<TranslatableColumnBase>>::iterator
                 it = columns_.begin();
             it != columns_.end(); ++it)
        {
            (*it)->Translate(icd_translate_);
        }
        append_count_ = 0;
    }

    if (is_pqctx_configured_)
    {
        if (pq_ctx_->IncrementAndWrite(0))
        {
        }
    }
    else
    {
        SPDLOG_WARN("{:02d} ParquetContext not configured. Use ConfigurePqContext()",
                    thread_index_);
        is_valid_ = false;
        return false;
    }
    return true;
}

bool TranslatableTableBase::AppendRidealongColumnData(const uint8_t* data, const size_t& count,
                                                      const std::string& col_name)
{
    if (ridealong_col_name_to_index_map_.count(col_name) == 0)
    {
        SPDLOG_WARN("{:02d} Column with name \"{:s}\" does not exist in map",
                    thread_index_, col_name);
        is_valid_ = false;
        return false;
    }

    if (!columns_.at(ridealong_col_name_to_index_map_.at(col_name))->AppendRidealongData(data, count))
    {
        SPDLOG_WARN("{:02d} Failed to AppendRidealongData for column \"{:s}\"",
                    thread_index_, col_name);
        is_valid_ = false;
        return false;
    }

    return true;
}

bool TranslatableTableBase::ConfigurePqContext(std::shared_ptr<ParquetContext>& pq_ctx,
                                               const std::vector<std::shared_ptr<TranslatableColumnBase>>& columns,
                                               bool is_valid, size_t rg_count, const ManagedPath& output_path)
{
    if (!is_valid)
    {
        SPDLOG_WARN(
            "{:02d} Flag is_valid_ not true. Table may need Configure() or some "
            "other error has occurred",
            thread_index_);
        return false;
    }

    if (columns.size() == 0)
    {
        SPDLOG_WARN(
            "{:02d} Input columns vector has size zero. Use AppendTranslatableColumn "
            "to add columns to table.",
            thread_index_);
        return false;
    }

    if (pq_ctx != nullptr)
    {
        SPDLOG_WARN(
            "{:02d} ParquetContext pointer pq_ctx is not nullptr. It may have "
            "already been configured.",
            thread_index_);
        return false;
    }

    if (rg_count == 0)
    {
        SPDLOG_WARN("{:02d} Row group count must be greater than zero.", thread_index_);
        return false;
    }

    if (output_path.RawString().compare("") == 0)
    {
        SPDLOG_WARN("{:02d} The output file path is an empty string.", thread_index_);
        return false;
    }

    pq_ctx = std::make_shared<ParquetContext>(static_cast<int>(rg_count));
    for (std::vector<std::shared_ptr<TranslatableColumnBase>>::const_iterator it =
             columns.cbegin();
         it != columns.cend(); ++it)
    {
        (*it)->ConfigureParquetContext(pq_ctx);
    }

    if (!pq_ctx->OpenForWrite(output_path.string(), true))
    {
        SPDLOG_ERROR("{:02d} Failed to open Parquet file: {:s}",
                     thread_index_, output_path.RawString());
        return false;
    }

    if (!pq_ctx->SetupRowCountTracking(rg_count, 1, false, name_))
    {
        SPDLOG_ERROR("{:02d} SetupRowCountTracking failed for file: {:s}",
                     thread_index_, output_path.RawString());
        return false;
    }
    pq_ctx->EnableEmptyFileDeletion(output_path.string());

    return true;
}

bool TranslatableTableBase::ConfigurePqContext()
{
    is_valid_ = ConfigurePqContext(pq_ctx_, columns_, is_valid_,
                                   row_group_size_, output_path_);
    if (is_valid_)
        is_pqctx_configured_ = true;
    return is_valid_;
}

void TranslatableTableBase::CloseOutputFile()
{
    if (append_count_ > 0)
    {
        SPDLOG_DEBUG("{:02d} Translating columns of table: {:s}", thread_index_, name_);
        for (std::vector<std::shared_ptr<TranslatableColumnBase>>::iterator
                 it = columns_.begin();
             it != columns_.end(); ++it)
        {
            (*it)->Translate(icd_translate_);
        }
        append_count_ = 0;
    }

    if (pq_ctx_ != nullptr)
        pq_ctx_->Close(static_cast<uint16_t>(thread_index_));
}
