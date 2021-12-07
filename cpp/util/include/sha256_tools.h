
#ifndef SHA256_TOOLS_H_
#define SHA256_TOOLS_H_

#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>
#include "managed_path.h"
#include "sha256.h"

/*
Generate the SHA256 hash from a stream.

Args:
    input_stream    --> input stream forwhich the sha256 shall
                        be computed
    sha256_value    --> The computed sha256 value
    byte_count      --> The sha256 shall be computed for the first
                        byte_count bytes of the file. If byte_count = 0,
                        the sha256 shall be computed for the entire file.
                    
Return:
    True if the sufficient chars in the stream and the hash can be 
    computed as expected; false otherwise.
*/
bool ComputeSHA256(std::istream& input_stream, std::string& sha256_value,
                   size_t byte_count = 0);

/*
Compute total or partial file hash.

Args:
    input_file      --> input file from which the sha256 shall
                        be computed
    sha256_value    --> The computed sha256 value
    byte_count      --> The sha256 shall be computed for the first
                        byte_count bytes of the file. If byte_count = 0,
                        the sha256 shall be computed for the entire file.
                    
Return:
    True if the file exists, can be read, and the hash can be 
    computed as expected; false otherwise.
*/
bool ComputeFileSHA256(const ManagedPath& input_file, std::string& sha256_value,
                       size_t byte_count = 0);



/*
Compute sha256 from string.

Args:
    input   --> String for which sha shall be computed

Return:
    sha256 value of input
*/
std::string Sha256(std::string input);

#endif  // SHA256_TOOLS_H_
