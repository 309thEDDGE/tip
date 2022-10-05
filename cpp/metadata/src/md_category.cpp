#include "md_category.h"

MDCategory::MDCategory(std::string category_label) : label_(category_label), 
        label(label_), node_(), node(node_), is_block_style_(true)
{
    node_.SetStyle(YAML::EmitterStyle::Block);
}


void MDCategory::SetValue(const YAML::Node& val)
{
    node_ = val;
}


void MDCategory::SetFlowStyle()
{
    node_.SetStyle(YAML::EmitterStyle::Flow);
    is_block_style_ = false;
}