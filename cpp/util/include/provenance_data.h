#ifndef PROVENANCE_DATA_H_
#define PROVENANCE_DATA_H_

#include <string>
#include <ctime>
#include "version_info.h"
#include "sha256_tools.h"
#include "managed_path.h"
#include "spdlog/spdlog.h"

class ProvenanceData
{
public:
    std::string tip_version;
    std::string hash;
    std::string time;

    ProvenanceData() : tip_version(""), hash(""), time("")
    {}

    bool operator==(const ProvenanceData& rhs) const;
};


/*
Get the GMT time as a string formatted according to
strftime function in the C standard library.

Args:
    strftime_fmt    --> Format string following the 
                        strftime format spec
                    
Return:
    A string representing the GMT time in the format 
    indicated.
*/
std::string GetGMTString(const std::string& strftime_fmt);



/*
Define provenance data object at the time this function is called
and compute the hash of the input file path.

Args:
    hash_file_path  --> Path to file for which sha256 shall be 
                        calculated
    hash_byte_count --> Count of bytes to hash. Use zero to calculate
                        the hash of the entire file.
    data            --> ProvenanceData object which shall be defined

Return:
    True if no problems occur; false otherwise
*/
bool GetProvenanceData(const ManagedPath& hash_file_path, size_t hash_byte_count, 
    ProvenanceData& data);


#endif  // PROVENANCE_DATA_H_