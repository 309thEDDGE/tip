#ifndef YAML_MD_DOCUMENT_H_
#define YAML_MD_DOCUMENT_H_

#include <map>
#include <memory>
#include <ctime>
#include <string>
#include <vector>
#include "yaml-cpp/yaml.h"
#include "md_document.h"
#include "md_category.h"

class YamlMDDocument : public MDDocument
{
private:
    
    // Emitter with which yaml will be recorded.
    YAML::Emitter emitter_;    

protected:

public:

    YamlMDDocument();
    virtual ~YamlMDDocument() {}

    virtual std::string GetMetadataString();
    virtual bool CreateDocument(); 
    virtual bool ReadDocument(const std::string& doc);


    ///////////////////////////////////////////////////////////////////////
    //              Internal functions made public for testing
    ///////////////////////////////////////////////////////////////////////

    /*
    Emit a category object. The data represented by the category
    will be recorded to the emitter. Handles the creation of the
    base map. 

    Args:
        category        --> Category object for which data will be
                            recorded to the yaml emitter.

    */
    void EmitCategory(const std::shared_ptr<MDCategory>& category);



    /*
    Read top-level keys from Yaml::Node.

    Args:
            node        --> Yaml::Node which is a map and 
                            from which top-level keys are
                            extracted
            keys        --> Output vector of top-level keys
        
    Return:
        False if node is not a map or keys cannot be extracted;
        true otherwise.
    */
    bool ExtractTopLevelKeys(const YAML::Node& node, 
        std::vector<std::string>& keys);
    


    /*
    Set the category node value to the value read from the
    corresponding node in the input document.

    Args:
        category        --> Category object for which data will be
                            recorded to the yaml emitter.
        input_node      --> YAML::Node which has been matched
                            by label to the input category.

    Return:
        True if no errors; false otherwise. 
    */
    bool SetCategoryValueFromInputNode(std::shared_ptr<MDCategory> category,
        const YAML::Node& input_node);
    
};

#endif  // YAML_MD_DOCUMENT_H_
