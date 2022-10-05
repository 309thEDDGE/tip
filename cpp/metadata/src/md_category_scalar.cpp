#include "md_category_scalar.h"

MDCategoryScalar::MDCategoryScalar(std::string category_label) : MDCategory(category_label)
{
    node_ = YAML::Node(YAML::NodeType::Scalar);
}
