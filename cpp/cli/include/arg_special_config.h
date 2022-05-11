#ifndef ARG_SPECIAL_CONFIG_H_
#define ARG_SPECIAL_CONFIG_H_

#include <string>
#include <typeinfo>
#include <set>
#include <iostream>
#include "managed_path.h"

class ArgConfig
{
    private:

    public:
        ArgConfig() {}
        virtual ~ArgConfig() {}

        /*
        Primary function that carries out the purpose of configuration
        for default arguments. Only relevant if the user input has been
        determined to be not present or can't be cast to the intended data
        type, etc.
        */
        virtual void Compute() {}


        /*
        Apply the logic to carry out special validation of the user-input
        argument.
        Args:
            fail_msg    --> String which identifies the argument in a fail
                            message

        Return:
            True if validated; false otherwise.
        */
        virtual bool Validate(const std::string& fail_msg) { printf("in here :<(\n"); return true; }
};

template<typename T>
class ArgSpecialConfig : public ArgConfig
{
    protected:
        const T* input_;
        T* output_;

    public:
        ArgSpecialConfig() : ArgConfig(), input_(nullptr), output_(nullptr)
        {}
        virtual ~ArgSpecialConfig() {} 
        void Set(const T* input, T* output);
};

template<typename T>
void ArgSpecialConfig<T>::Set(const T* input, T* output)
{ 
    input_ = input; 
    output_ = output;
}

#endif  // ARG_SPECIAL_CONFIG_H_