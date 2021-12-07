#ifndef MD_CATEGORY_SEQUENCE_H_
#define MD_CATEGORY_SEQUENCE_H_

#include <string>
#include <vector>
#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"
#include "md_category.h"

class MDCategorySequence: public MDCategory
{
private:


public:
    MDCategorySequence(std::string category_label);
    virtual ~MDCategorySequence() {}

    /*
    Insert a value into the sequence.

    Args:
        val     --> Value to be inserted into the sequence
    */
    template<class T>
    void Insert(T val);

};

template<class T>
void MDCategorySequence::Insert(T val)
{
    node_.push_back(val);
}

#endif  // MD_CATEGORY_SEQUENCE_H_