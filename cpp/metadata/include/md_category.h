#ifndef MD_CATEGORY_H_
#define MD_CATEGORY_H_

#include <string>
#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"

/*
A MDCategory is a fundamental key:value map as in a yaml document.
It's purpose is to force the use of a key, and through mapped 
children, to force the use of sub-level keys and corresponding 
value types. 
*/
class MDCategory
{
private:

    // Category label is equivalent to the key in a 
    // dictionary or yaml map
    std::string label_;

protected:

    // Node which represents the data mapped by the label.
    YAML::Node node_; 

    // Set to false if SetFlowStyle() is called
    bool is_block_style_;

public:

    const std::string& label;
    const YAML::Node& node;
    MDCategory(std::string category_label);
    virtual ~MDCategory() {}

    /*
    Set the value of the node associated with the key (here called the label)

    Args:
        val     --> YAML::Node value to which the internal Node will
                    be assigned    
    */
    void SetValue(const YAML::Node& val);



    /*
    Set the style of the node to flow. Default is block.
    */
    void SetFlowStyle();
    

    ///////////////////////////////////////////////////////////////////////
    //              Internal functions made public for testing
    ///////////////////////////////////////////////////////////////////////

};



#endif  // MD_CATEGORY_H_