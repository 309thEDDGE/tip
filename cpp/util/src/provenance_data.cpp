#include "provenance_data.h"

int GetProvenanceData(const ManagedPath& hash_file_path, size_t hash_byte_count, 
    ProvenanceData& data)
{
    data.time = GetGMTString("%F %T");
    data.tip_version = GetVersionString();

    std::string hash;
    int retcode = 0;
    if((retcode = ComputeFileSHA256(hash_file_path, hash, hash_byte_count)) != 0)
    {
        SPDLOG_WARN("Failed to compute sha256 for file: {:s}",
            hash_file_path.RawString());
        return retcode;
    }
    data.hash = hash;

    return EX_OK;
}

std::string GetGMTString(const std::string& strftime_fmt)
{
    std::string time_str;
    time_t rawtime;
    struct tm* tm_ptr = nullptr;
    time(&rawtime);
#if defined __WIN64
    struct tm tm_data;
    tm_ptr = &tm_data;
    gmtime_s(tm_ptr, &rawtime);
#else
    tm_ptr = gmtime(&rawtime);
#endif
    const size_t bufflen = 100;
    char time_buff[bufflen];
    time_buff[bufflen-1] = '\0';
    size_t copied_len = strftime(time_buff, bufflen, strftime_fmt.c_str(), tm_ptr);

    time_str = std::string(time_buff, copied_len);
    return time_str;
}

bool ProvenanceData::operator==(const ProvenanceData& rhs) const
{
    return ((this->hash == rhs.hash) && (this->time == rhs.time)
        && (this->tip_version == rhs.tip_version));
}