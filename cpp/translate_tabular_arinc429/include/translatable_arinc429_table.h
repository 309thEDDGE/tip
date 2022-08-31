#ifndef TRANSLATABLE_ARINC429_TABLE_
#define TRANSLATABLE_ARINC429_TABLE_

#include <string>
#include <vector>
#include <memory>
#include "translatable_table_base.h"
#include "translatable_explicit_sign_column_template.h"

class TranslatableARINC429Table : public TranslatableTableBase
{
    private:
    
    public:
        TranslatableARINC429Table() : TranslatableTableBase() {}
        ~TranslatableARINC429Table() {}

        template <typename RawType, typename TranslatedType>
        bool AppendExplicitSignTranslatableColumn(const std::string& col_name, bool ridealong,
            std::shared_ptr<arrow::DataType> arrow_type, const ICDElement& icd_elem,
            std::vector<std::shared_ptr<TranslatableColumnBase>>& columns_vec);

        template <typename RawType, typename TranslatedType>
        bool AppendExplicitSignTranslatableColumn(const std::string& col_name, bool ridealong,
                                  std::shared_ptr<arrow::DataType> arrow_type, const ICDElement& icd_elem);

        virtual bool AppendRawData(const uint8_t* data, const size_t& count, int8_t sign);
        virtual void _AppendToColumns(const uint8_t* data, const size_t& count, int8_t sign); 
};

template <typename RawType, typename TranslatedType>
bool TranslatableARINC429Table::AppendExplicitSignTranslatableColumn(
    const std::string& col_name, bool ridealong,
    std::shared_ptr<arrow::DataType> arrow_type, const ICDElement& icd_elem,
    std::vector<std::shared_ptr<TranslatableColumnBase>>& columns_vec)
{
    if (!is_valid_)
    {
        SPDLOG_WARN(
            "{:02d} Table has not been configured or is not "
            "valid/has experience some other error",
            thread_index_);
        return false;
    }

    std::shared_ptr<TranslatableExplicitSignColumnTemplate<RawType, TranslatedType>> col =
        std::make_shared<TranslatableExplicitSignColumnTemplate<RawType, TranslatedType>>();
    if (!col->Configure(col_name, ridealong, arrow_type, row_group_size_,
                        icd_elem, thread_index_))
    {
        SPDLOG_WARN("{:02d} Failed to configure new column", thread_index_);
        is_valid_ = false;
        return false;
    }

    std::shared_ptr<TranslatableColumnBase> base_col = col;
    columns_vec.push_back(base_col);

    // If it's a ridealong column. Add it's index in columns_ to the map
    if (ridealong)
    {
        ridealong_col_name_to_index_map_[col_name] = columns_vec.size() - 1;
    }

    return true;
}

template <typename RawType, typename TranslatedType>
bool TranslatableARINC429Table::AppendExplicitSignTranslatableColumn(
    const std::string& col_name, bool ridealong,
    std::shared_ptr<arrow::DataType> arrow_type, const ICDElement& icd_elem)
{
    is_valid_ = AppendExplicitSignTranslatableColumn<RawType, TranslatedType>(
        col_name, ridealong, arrow_type, icd_elem, columns_);
    return is_valid_;
}

#endif  // TRANSLATABLE_ARINC429_TABLE_
