#include "md_category_map.h"

MDCategoryMap::MDCategoryMap(std::string category_label) : MDCategory(category_label), 
   keys(keys_), keys_defined_(false)
{ 
   node_ = YAML::Node(YAML::NodeType::Map);
}

bool MDCategoryMap::SetMapKeys(const std::vector<std::string>& keys)
{
   if(keys_.size() > 0)
      return false;

   for(std::vector<std::string>::const_iterator it = keys.cbegin(); it != keys.cend(); 
      ++it)
   {
      keys_.push_back(*it);
      node_[*it] = "";
   }      

   if(is_block_style_)
      node_.SetStyle(YAML::EmitterStyle::Block);
   else
      node_.SetStyle(YAML::EmitterStyle::Flow);

   keys_defined_ = true;

   return true;
}