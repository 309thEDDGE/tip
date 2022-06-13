#ifndef CONTAINER_ARG_H_
#define CONTAINER_ARG_H_

#include <string>
#include <vector>
#include <utility>
#include <map>
#include <iomanip>
#include <iostream>
#include <memory>
#include <type_traits>
#include <charconv>
#include <system_error>
#include "parse_text.h"

class ContainerArg
{
    private:
        // Split on colon
        ParseText pt_;

    public:
        ContainerArg() : pt_() {}
        virtual ~ContainerArg() {}
        static const std::string whitespace_code;

        /*
        Parse user input and place in container.

        Args:
            user_input      --> User input string which must be formatted
                                in some specific way in order to be parsed.
                                See derived classes for details.

        Return:
            True if user_input is formatted as expected and values
            can be parsed and casted. False otherwise.
        */
        virtual bool Parse(const std::string& user_input) { return true; }


        /*
        Check if the from_chars_result indicates that all chars contributed
        to the result value and no errors occurred.

        Args:
            status      --> from_chars_result
            input_str   --> User input string on which from_chars is called

        Return:
            True if status.ptr is last of input_str and status.errc is value
            initialized; false otherwise.
        */
        virtual bool CheckFromCharsResultExactMatch(std::from_chars_result& status,
            const std::string& input_str) 
        {
            if(status.ec == std::errc::invalid_argument) 
            {
                std::cout << "CheckFromCharsResultExactMatch: invalid_argument" << std::endl;
                std::cout << "pointer: " << std::quoted(status.ptr) << std::endl;
                return false;
            }
            if(status.ec == std::errc::result_out_of_range)
            {
                std::cout << "CheckFromCharsResultExactMatch: result_out_of_range" << std::endl;
                std::cout << "pointer: " << std::quoted(status.ptr) << std::endl;
                return false;
            }

            if (status.ptr != (input_str.data() + input_str.size()))
            {
                std::cout << "pointer: " << std::quoted(status.ptr) << std::endl;
                return false;
            }

            return true;
        }


        /*
        Split input string on colon.

        Args:
            user_input      --> User input string which must be formatted
                                in some specific way in order to be parsed.
            key             --> Output key component (string before colon)
            val             --> Output value component (string after colon)

        Return:
            True if key and val can be retrieved from input string. False
            if some error occurs.
        */
        virtual bool SplitOnColon(const std::string& user_input, std::string& key,
            std::string& val)
        {
            if(user_input.find(':') == std::string::npos)
                return false;

            std::vector<std::string> comp = pt_.Split(user_input, ':');
            if(comp.size() != 2)
                return false;

            key = comp.at(0);
            val = comp.at(1);

            if (key == "" || val == "")
                return false;

            return true;
        }

        template<typename T>
        bool ParseComponent(const std::string& input, T& parsed);
       
};

inline const std::string ContainerArg::whitespace_code = "--..--";

template<typename T>
bool ContainerArg::ParseComponent(const std::string& input, T& parsed)
{
    std::from_chars_result status = std::from_chars(input.data(), 
        input.data() + input.size(), parsed);
    if(!CheckFromCharsResultExactMatch(status, input))
        return false;
    return true;
}

template<>
inline bool ContainerArg::ParseComponent(const std::string& input, double& parsed)
{
    if(!pt_.ConvertDouble(input, parsed))
        return false;
    return true;
}

template<>
inline bool ContainerArg::ParseComponent(const std::string& input, float& parsed)
{
    double temp_double = 0.0;
    if(!pt_.ConvertDouble(input, temp_double))
        return false;
    parsed = static_cast<float>(temp_double);
    return true;
}

template<>
inline bool ContainerArg::ParseComponent(const std::string& input, std::string& parsed)
{
    if(input.find(whitespace_code) != std::string::npos)
    {
        parsed = pt_.Replace(input, whitespace_code, " ");
    }
    else
        parsed = input;
    return true;
}

template<typename ValType>
class ContainerArgVector : public ContainerArg
{
    private:
        // Pointer to user-submitted container
        std::vector<ValType>* vec_ptr_;

        // Pointer to temporary parsed/casted component
        ValType* parsed_val_ptr_;

    public:
        ContainerArgVector(std::vector<ValType>& input_vec, ValType& parsed_val) : ContainerArg(), 
            vec_ptr_(&input_vec), parsed_val_ptr_(&parsed_val) {}
        virtual ~ContainerArgVector() {}

        /*
        Parse single user input value by casting to ValType
        and insert into *vec_ptr_ if no errors occur.
        */
        virtual bool Parse(const std::string& user_input);
};

template<typename ValType>
bool ContainerArgVector<ValType>::Parse(const std::string& user_input)
{
    if(!ParseComponent(user_input, *parsed_val_ptr_))
        return false;

    vec_ptr_->push_back(*parsed_val_ptr_);
    return true;
}

template<typename KeyType, typename ValType>
class ContainerArgMap : public ContainerArg
{
    private:
        // Pointer to user-submitted container
        std::map<KeyType, ValType>* map_ptr_;

        KeyType* parsed_key_ptr_;
        ValType* parsed_val_ptr_;


    public:
        ContainerArgMap(std::map<KeyType, ValType>& input_map, KeyType& parsed_key, 
            ValType& parsed_val) : ContainerArg(), map_ptr_(&input_map), 
            parsed_key_ptr_(&parsed_key), parsed_val_ptr_(&parsed_val) 
        {}
        virtual ~ContainerArgMap() {}

        /*
        Parse single user input value which is a string formatted as 
        <key>:<value>. Successful if parsed and key can be cast to
        KeyType, value can be cast to ValType, key and value can be
        inserted into *map_ptr_.
        */
        virtual bool Parse(const std::string& user_input);

};

template<typename KeyType, typename ValType>
bool ContainerArgMap<KeyType, ValType>::Parse(const std::string& user_input)
{
    std::string key("");
    std::string val("");

    if(!SplitOnColon(user_input, key, val))
    {
        std::cout << "failed to parse: " << user_input << std::endl;
        return false;
    }

    if(!ParseComponent(key, *parsed_key_ptr_))
    {
        std::cout << "failed to parse key: " << key << std::endl;
        return false;
    }

    if(!ParseComponent(val, *parsed_val_ptr_))
    {
        std::cout << "failed to parse val: " << val << std::endl;
        return false;
    }

    map_ptr_->insert(std::pair<KeyType, ValType>(*parsed_key_ptr_, *parsed_val_ptr_));

    return true;
}

// template<typename ValType>
// class ContainerArgScalar : public ContainerArg
// {
//     private:
//         ValType* scalar_ptr_;

//     public:
//         ContainerArgScalar(ValType& input_scalar) : ContainerArg(), 
//             scalar_ptr_(&input_scalar) {}
//         virtual ~ContainerArgScalar() {}

//         /*
//         Parse single user input value by casting to ValType
//         and insert into *vec_ptr_ if no errors occur.
//         */
//         virtual bool Parse(const std::string& user_input);
// };

// template<typename ValType>
// bool ContainerArgScalar<ValType>::Parse(const std::string& user_input)
// {
//     ValType result{};
//     if(!ParseComponent(user_input, result))
//         return false;

//     *(this->scalar_ptr_) = result;
//     return true;
// }

template<typename ValType>
inline std::shared_ptr<ContainerArg> MakeContainerArg(std::vector<ValType>& input_vec,
    ValType& parsed_val)
{
    std::shared_ptr<ContainerArg> carg = std::make_shared<ContainerArgVector<ValType>>(
        input_vec, parsed_val);
    return carg;
}

template<typename KeyType, typename ValType>
inline std::shared_ptr<ContainerArg> MakeContainerArg(std::map<KeyType, ValType>& input_map,
    KeyType& parsed_key, ValType& parsed_val)
{
    std::shared_ptr<ContainerArg> carg = std::make_shared<ContainerArgMap<KeyType, ValType>>(
        input_map, parsed_key, parsed_val);
    return carg;
}

// template<typename ValType>
// inline std::shared_ptr<ContainerArg> MakeContainerArg(ValType& input_val)
// {
//     std::shared_ptr<ContainerArg> carg = std::make_shared<ContainerArgScalar<ValType>>(
//         input_val);
//     return carg;
// }

#endif  // CONTAINER_ARG_H_