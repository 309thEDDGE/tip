#include "translate_tabular_context_base.h"

TranslateTabularContextBase::~TranslateTabularContextBase()
{
}

std::shared_ptr<TranslateTabularContextBase> TranslateTabularContextBase::Clone()
{
    std::shared_ptr<TranslateTabularContextBase> temp =
        std::make_shared<TranslateTabularContextBase>();
    temp->SetColumnNames(this->ridealong_col_names_, this->data_col_names_);
    return temp;
}

bool TranslateTabularContextBase::IsConfigured()
{
    if (data_col_names_.size() == 0)
    {
        SPDLOG_WARN("data_col_names_ has size zero. Use SetColumnNames() method.");
        return false;
    }
    return true;
}

void TranslateTabularContextBase::SetColumnNames(const std::vector<std::string>& ridealong_col_names,
                                                 const std::vector<std::string>& data_col_names)
{
    ridealong_col_names_ = ridealong_col_names;
    data_col_names_ = data_col_names;
}

bool TranslateTabularContextBase::CreateTranslatableTable(const std::string& name,
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

    std::shared_ptr<TranslatableTableBase> table = std::make_shared<TranslatableTableBase>();
    if (!table->Configure(name, row_group_size, output_path, thread_index))
    {
        SPDLOG_WARN("{:02d} Table is not configured", thread_index);
        return false;
    }

    table_map[index] = table;

    return true;
}

bool TranslateTabularContextBase::AppendColumnPreCheck(
    std::shared_ptr<TranslatableTableBase>& table_ptr)
{
    // Check for null pointer
    if (table_ptr == nullptr)
    {
        SPDLOG_WARN("table_ptr is nullptr");
        return false;
    }

    if (!IsConfigured())
    {
        SPDLOG_WARN("Not configured: Use Configure()");
        return false;
    }

    if (!table_ptr->is_valid)
    {
        SPDLOG_WARN("Table is not valid");
        return false;
    }
    return true;
}
