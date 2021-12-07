#include "md_category_sequence.h"

MDCategorySequence::MDCategorySequence(std::string category_label) : MDCategory(category_label)
{ 
   node_ = YAML::Node(YAML::NodeType::Sequence);
   node_.SetStyle(YAML::EmitterStyle::Block);
}

