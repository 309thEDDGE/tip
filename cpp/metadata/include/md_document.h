#ifndef MD_DOCUMENT_H_
#define MD_DOCUMENT_H_

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include "md_category.h"

class MDDocument
{
private:
    

protected:

    // MDCategories which will be written to or read from to
    // create the total metadata document.
    // std::map<std::string, std::shared_ptr<MDCategory>> categories_;
    std::vector<std::shared_ptr<MDCategory>> categories_;

public:

    MDDocument();
    virtual ~MDDocument() {}
    

    /*
    Add a category to be written or read into

    Args:
        category        --> Category object for which data will be
                            recorded to the yaml emitter.
    */
    virtual void AddCategory(const std::shared_ptr<MDCategory>& category);


    /*
    Create the document from categories that have been submitted
    via AddCategory. To be called prior to GetMetadataString.

    Return:
        True if no errors occur; false otherwise.
    */
    virtual bool CreateDocument() { return true;}
    


    /*
    Read a document by parsing the input string, presumably
    matter which has been read from a file. Fields in categories 
    which have been added via AddCategory will be populated.

    Args:
        doc     --> String representation of a document, including 
                    non-visible formatting characters  

    Return:
        True if no errors occur; false otherwise.
    */
    virtual bool ReadDocument(const std::string& doc) { return true; }



    /*
    Get a string representing all of the metadata in a document.

    Return:
        String object which represents the metadata
    */
    virtual std::string GetMetadataString() 
    {
        std::string null("");
        return null;
    }


    ///////////////////////////////////////////////////////////////////////
    //              Internal functions made public for testing
    ///////////////////////////////////////////////////////////////////////

    /*
    Read data from the input string (ReadDocument) and populate 
    the categories in the categories_ vector.
    */
    // bool PopulateCategories


    /*
    Check if categories are present based on vector of category labels.
    
    Args:
            categories      --> Vector of pre-defined categories which
                                are to be matched with input document
                                labels/sections and eventually populated
                                with input document data
            labels          --> Vector of labels to matched with categories
            nonmatched_count--> Count of categories which were not matched
                                to a label

    Return:
        True if no errors occurred; false otherwise. If false, do not use
        value of nonmatched_count.
    */
    bool CountMatchedLabels(const std::vector<std::shared_ptr<MDCategory>>& categories,
        const std::vector<std::string>& labels, size_t& nonmatched_count);
};

#endif  // MD_DOCUMENT_H_
