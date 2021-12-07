#ifndef TIP_MD_DOCUMENT_H_
#define TIP_MD_DOCUMENT_H_

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <cstdio>
#include "yaml_md_document.h"
#include "md_category_scalar.h"
#include "md_category_map.h"
#include "md_category_sequence.h"

class TIPMDDocument : public YamlMDDocument
{
private:
    const std::vector<std::string> provenance_keys_;
    const std::vector<std::string> resource_entry_keys_;

public:

    // Explicit categories, specific to tip metadata
    std::shared_ptr<MDCategoryScalar> type_category_;
    std::shared_ptr<MDCategoryScalar> uid_category_;
    std::shared_ptr<MDCategoryMap> prov_category_;
    std::shared_ptr<MDCategorySequence> resource_category_;
    std::shared_ptr<MDCategoryMap> config_category_;
    std::shared_ptr<MDCategoryMap> runtime_category_;

    TIPMDDocument();
    virtual ~TIPMDDocument() {}


    /*
    Insert resource category in provenance.resource sequence.

    Args:
        type        --> The type of metadata resource
        path        --> path to resource
        uid         --> uid of resource
    */
    void AddResource(std::string type, std::string path, std::string uid);



    /*
    Insert resource category in provenance.resource sequence.

    Args:
        md_document     --> TIPMDDocument from which provenance
                            resource entries shall be added
                        
    Return:
        True if no errors occur; false otherwise.
    */
    bool AddResources(const TIPMDDocument& md_document);

    //////////////////////////////////////////////////////////////////////////
    //      Internal functions
    //////////////////////////////////////////////////////////////////////////

    /*
    Initialize categories with default values and keys, etc.
    */
    void InitCategories();
};

#endif  // TIP_MD_DOCUMENT_H_