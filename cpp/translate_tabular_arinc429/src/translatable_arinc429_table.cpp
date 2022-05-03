#include "translatable_arinc429_table.h"


bool TranslatableARINC429Table::AppendRawData(const uint8_t* data, const size_t& count, int8_t sign)
{
    _AppendToColumns(data, count, sign);
    return _IncrementAndTranslate();
}

void TranslatableARINC429Table::_AppendToColumns(const uint8_t* data, const size_t& count, int8_t sign)
{
    for (std::vector<std::shared_ptr<TranslatableColumnBase>>::iterator
             it = columns_.begin();
         it != columns_.end(); ++it)
    {
        // Use the built-in checks to handle correct column types (not ridealong),
        // sufficient count in data vector, etc. Therefore, do not check return type
        if((*it)->GetICDElement().schema_ == ICDElementSchema::BCD)
            (*it)->AppendRawData(data, count, sign);
        else
            (*it)->AppendRawData(data, count);
    }
}