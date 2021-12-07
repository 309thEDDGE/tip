#include "tip_md_document.h"

TIPMDDocument::TIPMDDocument() : YamlMDDocument(), 
    provenance_keys_({"time", "version", "resource"}),
    resource_entry_keys_({"type", "path", "uid"}),
    type_category_(std::make_shared<MDCategoryScalar>("type")),
    uid_category_(std::make_shared<MDCategoryScalar>("uid")),
    prov_category_(std::make_shared<MDCategoryMap>("provenance")),
    resource_category_(std::make_shared<MDCategorySequence>("resource")),
    config_category_(std::make_shared<MDCategoryMap>("config")),
    runtime_category_(std::make_shared<MDCategoryMap>("runtime"))
{
    InitCategories();
}


void TIPMDDocument::InitCategories()
{
    prov_category_->SetMapKeys(provenance_keys_);
    AddCategory(type_category_);
    AddCategory(uid_category_);
    AddCategory(prov_category_);
    AddCategory(config_category_);
    AddCategory(runtime_category_);
}


void TIPMDDocument::AddResource(std::string type, std::string path, std::string uid)
{
    MDCategoryMap resource_entry("resource_entry");
    resource_entry.SetMapKeys(resource_entry_keys_);
    resource_entry.SetMappedValue("type", type);
    resource_entry.SetMappedValue("path", path);
    resource_entry.SetMappedValue("uid", uid);

    resource_category_->Insert(resource_entry.node);
    prov_category_->SetMappedValue("resource", resource_category_->node);
}


bool TIPMDDocument::AddResources(const TIPMDDocument& md_document)
{
    YAML::Node input_resource_node = md_document.prov_category_->node["resource"];
    if(!input_resource_node.IsSequence())
    {
        SPDLOG_WARN("Input document \"resource\" value in \"provenance\" "
            "category is not a sequence");
        return false;     
    }

    if(input_resource_node.size() == 0)
    {
        SPDLOG_WARN("Input document \"resource\" sequence in \"provenance\" "
            "category has size zero");
        return false;     
    }

    for(size_t i = 0; i < input_resource_node.size(); i++)
    {
        YAML::Node resource_entry_node = input_resource_node[i];
        resource_category_->Insert(resource_entry_node);
    }

    prov_category_->SetMappedValue("resource", resource_category_->node);
    return true;
}