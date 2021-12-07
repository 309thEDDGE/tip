#include "provenance_data.h"

bool GetProvenanceData(const ManagedPath& hash_file_path, size_t hash_byte_count, 
    ProvenanceData& data)
{
    data.time = GetGMTString("%F %T");
    data.tip_version = GetVersionString();

    std::string hash;
    if(!ComputeFileSHA256(hash_file_path, hash, hash_byte_count))
    {
        SPDLOG_WARN("Failed to compute sha256 for file: {:s}",
            hash_file_path.RawString());
        return false;
    }
    data.hash = hash;

    return true;
}

std::string GetGMTString(const std::string& strftime_fmt)
{
    std::string time_str;

    time_t rawtime;
    struct tm* tm_ptr;
    time(&rawtime);
    tm_ptr = gmtime(&rawtime);
    const size_t bufflen = 100;
    char time_buff[bufflen];
    time_buff[bufflen-1] = '\0';
    size_t copied_len = strftime(time_buff, bufflen, strftime_fmt.c_str(), tm_ptr);

    time_str = std::string(time_buff, copied_len);
    return time_str;
}
