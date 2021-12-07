#include "md_document.h"

MDDocument::MDDocument()
{

}

void MDDocument::AddCategory(const std::shared_ptr<MDCategory>& category)
{
    categories_.push_back(category);
}

bool MDDocument::CountMatchedLabels(const std::vector<std::shared_ptr<MDCategory>>& categories,
    const std::vector<std::string>& labels, size_t& nonmatched_count)
{
    if(categories.size() == 0)
    {
        SPDLOG_WARN("categories vector has size zero");
        return false;
    }
    if(labels.size() == 0)
    {
        SPDLOG_WARN("labels vector has size zero");
        return false;
    }

    size_t count = 0;
    size_t nonmatched = 0;
    for(size_t i = 0; i < categories.size(); i++)
    {
        count = std::count(labels.cbegin(), labels.cend(), categories.at(i)->label);
        if(count == 0)
            nonmatched++;
    }

    nonmatched_count = nonmatched;
    return true;
}
