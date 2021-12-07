#include "yaml_md_document.h"

YamlMDDocument::YamlMDDocument() : MDDocument()
{
    emitter_ << YAML::BeginDoc;

    // The yaml MD document is a single map.
    emitter_ << YAML::BeginMap;
}

void YamlMDDocument::EmitCategory(const std::shared_ptr<MDCategory>& category)
{
    emitter_ << YAML::Key << category->label;
    emitter_ << YAML::Value << category->node;
}


std::string YamlMDDocument::GetMetadataString()
{
    emitter_ << YAML::EndMap;
    emitter_ << YAML::EndDoc;

    std::string metadata(emitter_.c_str());
    return metadata;
}


bool YamlMDDocument::CreateDocument()
{
    for(std::vector<std::shared_ptr<MDCategory>>::const_iterator it = categories_.cbegin();
        it != categories_.cend(); ++it)
    {
        EmitCategory(*it);
    }   
    return true;
}


bool YamlMDDocument::ReadDocument(const std::string& doc)
{
    YAML::Node doc_node = YAML::Load(doc);
    std::vector<std::string> keys;
    if(!ExtractTopLevelKeys(doc_node, keys))
        return false;
        
    size_t nonmatched_count = 0;
    if(!CountMatchedLabels(categories_, keys, nonmatched_count))
        return false;

    if(nonmatched_count > 0)
    {
        SPDLOG_WARN("{:d} non-matched labels given the following top-level keys:",
            nonmatched_count);
        for(size_t i = 0; i < keys.size(); i++)
            SPDLOG_WARN("{:s}", keys.at(i));
        return false;
    }

    std::vector<std::shared_ptr<MDCategory>>::iterator it;
    std::string category_label("");
    for(it = categories_.begin(); it != categories_.end(); ++it)
    {
        category_label = (*it)->label;
        if(!SetCategoryValueFromInputNode(*it, doc_node[category_label]))
            return false;
    }    
    return true;
}


bool YamlMDDocument::ExtractTopLevelKeys(const YAML::Node& node, 
    std::vector<std::string>& keys)
{
    if(!node.IsMap())
    {
        SPDLOG_WARN("Top-level document node is not a map");
        return false;
    }

    YAML::const_iterator it;
    std::string curr_key;
    for(it = node.begin(); it != node.end(); ++it)
    {
        curr_key = it->first.as<std::string>();
        keys.push_back(curr_key);
    } 

    return true;
}

bool YamlMDDocument::SetCategoryValueFromInputNode(std::shared_ptr<MDCategory> category,
    const YAML::Node& input_node)
{
    if(category->node.Type() != input_node.Type())
    {
        SPDLOG_WARN("Category node type ({:d}) does not match input node type ({:d})",
            static_cast<int>(category->node.Type()), static_cast<int>(input_node.Type()));
        return false;
    } 

    category->SetValue(input_node);

    return true;
}
