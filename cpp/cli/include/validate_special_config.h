#ifndef VALIDATE_SPECIAL_CONFIG_H_
#define VALIDATE_SPECIAL_CONFIG_H_

#include <string>
#include <set>
#include <iostream>
#include "arg_special_config.h"

template<typename T>
class ArgPermittedValuesConfig : public ArgSpecialConfig<T>
{
    private:
        std::set<T> permitted_vals_;

    public:
        ArgPermittedValuesConfig(const std::set<T>& permitted_vals) : ArgSpecialConfig<T>(),
            permitted_vals_(permitted_vals)
        {}
        virtual ~ArgPermittedValuesConfig() {}
        virtual bool Validate(const std::string& fail_msg);
};

template<typename T>
bool ArgPermittedValuesConfig<T>::Validate(const std::string& fail_msg)
{
    T user_input = *(this->output_);
    T default_val = *(this->input_);

    if(permitted_vals_.count(user_input) == 1)
        return true;
    else
    {
        std::cout << std::endl << fail_msg << ": input \"" << user_input << 
            "\" not one of the allowed values:" << std::endl << std::endl;

        for(typename std::set<T>::const_iterator it = permitted_vals_.cbegin(); 
            it != permitted_vals_.cend(); ++it)
            std::cout << *it << std::endl;

        std::cout << std::endl;
        // std::cout << std::endl << "Using default value \"" << default_val << "\"" << 
        //     std::endl << std::endl;

        *(this->output_) = default_val;
        return false;
    }
}

template<typename T>
class ArgPermittedInclusiveRangeConfig : public ArgSpecialConfig<T>
{
    private:
        T range_low_;
        T range_high_;

    public:
        ArgPermittedInclusiveRangeConfig(T range_low, T range_high) : ArgSpecialConfig<T>(),
            range_low_(range_low), range_high_(range_high)
        {}
        virtual ~ArgPermittedInclusiveRangeConfig() {}
        virtual bool Validate(const std::string& fail_msg);
};

template<typename T>
bool ArgPermittedInclusiveRangeConfig<T>::Validate(const std::string& fail_msg)
{
    T user_input = *(this->output_);
    T default_val = *(this->input_);

    if(user_input >= range_low_ && user_input <= range_high_)
        return true;
    else
    {
        std::cout << std::endl << fail_msg << ": input \"" << user_input << 
            "\" not in [" << range_low_ << ", " << range_high_ << "]" << std::endl << std::endl;

        *(this->output_) = default_val;
        return false;
    }
}

#endif  // VALIDATE_SPECIAL_CONFIG_H_