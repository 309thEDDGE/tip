#ifndef MD_CATEGORY_SCALAR_H_
#define MD_CATEGORY_SCALAR_H_

#include <string>
#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"
#include "md_category.h"

class MDCategoryScalar: public MDCategory
{
private:

public:

    MDCategoryScalar(std::string category_label);
    virtual ~MDCategoryScalar() {}

    /*
    Set the value that is mapped to the label. This defines 
    the category as a simple key-value pair.

    Args:
        value       --> Any value of basic data type 
                        (string, int, float)
    */
   template<typename T>
   void SetScalarValue(T value);

};

template<typename T>
void MDCategoryScalar::SetScalarValue(T value)
{
    node_ = value;
}

#endif  // MD_CATEGORY_SCALAR_H_