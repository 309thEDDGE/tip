#ifndef DEFAULT_SPECIAL_CONFIG_H_
#define DEFAULT_SPECIAL_CONFIG_H_

#include <string>
#include "managed_path.h"
#include "arg_special_config.h"

class ArgParentPathConfig : public ArgSpecialConfig<std::string>
{
    public:
        ArgParentPathConfig() : ArgSpecialConfig<std::string>()
        {}
        virtual ~ArgParentPathConfig() {} 
        virtual void Compute()
        {
            ManagedPath temp(*input_);
            *output_ = temp.parent_path().string();
        }
};

template <typename T>
class ArgUseValueConfig : public ArgSpecialConfig<T>
{
    public:
        ArgUseValueConfig() : ArgSpecialConfig<T>()
        {}
        virtual ~ArgUseValueConfig() {}
        virtual void Compute();
};

template<typename T>
void ArgUseValueConfig<T>::Compute()
{
    *(this->output_) = *(this->input_);
}

#endif  // DEFAULT_SPECIAL_CONFIG_H_ 