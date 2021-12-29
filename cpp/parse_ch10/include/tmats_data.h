#ifndef TMATS_DATA_H_
#define TMATS_DATA_H_


#define RETFAIL(x, y) if(!(x)) {printf("TMATSData::Parse: Failed to MapAttrs: " y); return false; }

#include <map>
#include <vector>
#include <string>
// #include "spdlog/spdlog.h"
#include "tmats_parser.h"

using cmap = std::map<std::string, std::string>;
using cmapvec = std::map<std::string, std::vector<std::string>>;
using cmapmap = std::map<std::string, std::map<std::string, std::string>>;

class TMATSData
{

private:
	cmap chanid_to_source_map_;
    cmap chanid_to_type_map_;
	cmap chanid_to_429_format_;
    cmapvec	chanid_to_429_subchans_;
	cmapmap chanid_to_429_subchan_and_name_;

public:
	const cmap& chanid_to_source_map;
    const cmap& chanid_to_type_map;
	const cmap& chanid_to_429_format;
	const cmapvec& chanid_to_429_subchans;
	const cmapmap& chanid_to_429_subchan_and_name;

    TMATSData();
    virtual ~TMATSData() {}



    /*
    Parse TMATS matter passed in the form of a string. 

    Args:
        tmats_data      --> String representation of entire
                            TMATs blob. Includes formatting
                            chars such as newlines.
    
    Return:
        False if one of the attributes fails to map; true
        otherwise.
    */
   bool Parse(const std::string& tmats_data);


    //////////////////////////////////////////////////////////////////////////
    //                  internal functions
    //////////////////////////////////////////////////////////////////////////

    /*
    Combine two cmapmaps under the assumption that the key of the submaps
    ought to be matched.
    */
    void CombineMaps(const cmapmap& map1, const cmapmap& map2, cmapmap& outmap);

};


#endif  // TMATS_DATA_H_