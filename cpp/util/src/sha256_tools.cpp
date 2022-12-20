#include "sha256_tools.h"

int ComputeSHA256(std::istream& input_stream, std::string& sha256_value,
                   size_t byte_count)
{
    // fail bits
    if (!input_stream.good())
    {
        printf("ComputeSHA256(): Fail bit(s) are set\n");
        sha256_value = "null";
        return EX_IOERR;
    }

    input_stream.seekg(0, input_stream.end);
    size_t stream_size = input_stream.tellg();
    input_stream.seekg(0, input_stream.beg);

    std::istreambuf_iterator<char> it(input_stream);
    std::string input;
    if (byte_count > 0)
    {
        if (stream_size < byte_count)
        {
            printf("ComputeSHA256(): Stream size %zu less than requested size %zu\n",
                   stream_size, byte_count);
            sha256_value = "null";
            return EX_IOERR;
        }

        input.reserve(byte_count);
        std::copy_n(it, byte_count, std::back_inserter(input));
    }
    else
    {
        input.reserve(stream_size);
        std::copy_n(it, stream_size, std::back_inserter(input));
    }

    sha256_value = CalcSHA256(input);

    return EX_OK;
}

int ComputeFileSHA256(const ManagedPath& input_file, std::string& sha256_value,
                       size_t byte_count)
{
    if (!input_file.is_regular_file())
    {
        printf("ComputeFileSHA256(): Input file (%s) does not exist\n",
               input_file.RawString().c_str());
        return EX_NOINPUT;
    }

    std::ifstream input_stream(input_file.string(), std::ifstream::binary);
    if (!input_stream.is_open())
    {
        printf("ComputeFileSHA256(): Input file (%s) could not be opened\n",
               input_file.RawString().c_str());
        return EX_IOERR;
    }

    if(byte_count > 0)
    {
        input_stream.seekg(0, input_stream.end);
        size_t stream_size = input_stream.tellg();
        input_stream.seekg(0, input_stream.beg);

        if(byte_count > stream_size)
            byte_count = 0;
    }

    int retcode = 0;
    if ((retcode = ComputeSHA256(input_stream, sha256_value, byte_count)) != 0)
    {
        printf("ComputeFileSHA256(): Failed to compute SHA256 for file (%s)\n",
               input_file.RawString().c_str());
        return retcode;
    }

    return EX_OK;
}


// std::string Sha256(std::string input)
// {
//     return sha256(input);
// }
