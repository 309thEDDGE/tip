#ifndef MD_CATEGORY_MAP_H_
#define MD_CATEGORY_MAP_H_

#include <string>
#include <map>
#include <algorithm>
#include <vector>
#include <utility>
#include <set>
#include "md_category.h"
#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"

class MDCategoryMap: public MDCategory
{
private:

    // Map keys
    std::vector<std::string> keys_;

    // Set true if SetMapKeys has been called. Use
    // to prevent recording of data in a way other
    // than SetMappedValue
    bool keys_defined_;

public:
    const std::vector<std::string>& keys;
    MDCategoryMap(std::string category_label);
    virtual ~MDCategoryMap() {}


    /*
    Define the map keys.

    Args:
        keys        --> Vector of strings which define
                        the map keys

    Return:
        False if the map keys have already been defined.
    */
    bool SetMapKeys(const std::vector<std::string>& keys);



    /*
    Set the value mapped by a key.

    args:
        key     --> Key name which must be one which was
                    configured by SetMapKeys
        val     --> Value to be assigned to the key

    Return:
        False if key is not one of the elements of
        vector passed to SetMapKeys or if that function
        has not been called; otherwise true.
    */
    template<class T>
    bool SetMappedValue(std::string key, const T& val);
    // template<class T>
    //     bool SetMappedValueImpl(std::string key, const T& val);
    // template<class T>
    //     bool SetMappedValueImpl(std::string key, const T&& val);



    /*
    Assign a new 'key' with the associated value the accompanying
    `type`, which can be simple and compound types such as map<k, v>,
    map<k, vector<v>>, etc. 

    This function is to be used to when keys can't be known
    beforehand and SetMapKeys hasn't been called and 
    therefore SetMappedValue will not work. Example use case is
    runtime data.

    args:
        key             --> Key name 
        type            --> data to be assigned to the value
                            associated with 'key' 

    Return:
    */
    template<class T>
    void SetArbitraryMappedValue(std::string key, const T& type);
    // template<class T>
        // void SetArbitraryMappedValue(std::string key, const T&& type);


    ///////////////////////////////////////////////////////////////////////
    //              Internal functions made public for testing
    ///////////////////////////////////////////////////////////////////////

    /*

    */
    template<class T>
    void PopulateNode(YAML::Node& node, const std::set<T>& data);

    template<class T>
    void PopulateNode(YAML::Node& node, const std::vector<T>& data);

    template<class T>
    void PopulateNode(YAML::Node& node, const T& data);
    template<class T>
    void PopulateNode(YAML::Node& node, const T&& data);

    template<class K, class V>
    void PopulateNode(YAML::Node& node, const std::map<K, V>& data);

    template<class K, class V>
    void PopulateNode(YAML::Node& node, const std::map<K, std::vector<V>>& data);

    template<class K, class V>
    void PopulateNode(YAML::Node& node, const std::map<K, std::set<V>>& data);

    template<class K, class V>
    void PopulateNode(YAML::Node& node, const std::map<K, std::vector<std::vector<V>>>& data);

};

// template<class T>
//     bool MDCategoryMap::SetMappedValue(std::string key, const T&& val)
// {
//     SetMappedValueImpl(key, std::forward<T>(val));
// }


template<class T>
bool MDCategoryMap::SetMappedValue(std::string key, const T& val)
{
    if(keys_.size() == 0)
        return false;

    if(std::count(keys_.begin(), keys_.end(), key) == 0)
    {
        return false;
    }

    YAML::Node temp_node = node_[key];
    PopulateNode(temp_node, val);

    return true;
}

// template<class T>
// bool MDCategoryMap::SetMappedValueImpl(std::string key, const T&& val)
// {
//     if(keys_.size() == 0)
//         return false;

//     if(std::count(keys_.begin(), keys_.end(), key) == 0)
//     {
//         return false;
//     }

//     PopulateNode(node_[key], val);

//     return true;
// }

template<class T>
void MDCategoryMap::SetArbitraryMappedValue(std::string key, const T& type)
{
    YAML::Node temp_node = node_[key];
    PopulateNode(temp_node, type);
}

// template<class T>
// void MDCategoryMap::SetArbitraryMappedValue(std::string key, const T&& type)
// {
//     PopulateNode(node_[key], type);
// }

template<class T>
void MDCategoryMap::PopulateNode(YAML::Node& node, const std::set<T>& data)
{
    node = YAML::Node(YAML::NodeType::Sequence);
    typename std::set<T>::const_iterator it;
    for(it = data.cbegin(); it != data.cend(); ++it)
    {
        node.push_back(*it);
    }
    node.SetStyle(YAML::EmitterStyle::Flow);
}

template<class T>
void MDCategoryMap::PopulateNode(YAML::Node& node, const std::vector<T>& data)
{

    node = YAML::Node(YAML::NodeType::Sequence);
    typename std::vector<T>::const_iterator it;
    for(it = data.cbegin(); it != data.cend(); ++it)
    {
        node.push_back(*it);
    }
    node.SetStyle(YAML::EmitterStyle::Flow);
}

template<class T>
void MDCategoryMap::PopulateNode(YAML::Node& node, const T& data)
{
    node = data;
}

template<class T>
void MDCategoryMap::PopulateNode(YAML::Node& node, const T&& data)
{
    node = data;
}

template<class K, class V>
void MDCategoryMap::PopulateNode(YAML::Node& node, const std::map<K, V>& data)
{
    node = YAML::Node(YAML::NodeType::Map);
    typename std::map<K, V>::const_iterator it;
    for(it = data.cbegin(); it != data.cend();
        ++it)
    {
        node[it->first] = it->second;
    }
}

template<class K, class V>
void MDCategoryMap::PopulateNode(YAML::Node& node, const std::map<K, std::vector<V>>& data)
{
    node = YAML::Node(YAML::NodeType::Map);
    typename std::map<K, std::vector<V>>::const_iterator it;
    for(it = data.cbegin(); it != data.cend(); ++it)
    {
        YAML::Node temp_node = node[it->first];
        PopulateNode(temp_node, it->second);
    }
}

template<class K, class V>
void MDCategoryMap::PopulateNode(YAML::Node& node, const std::map<K, std::set<V>>& data)
{
    node = YAML::Node(YAML::NodeType::Map);
    typename std::map<K, std::set<V>>::const_iterator it;
    for(it = data.cbegin(); it != data.cend(); ++it)
    {
        YAML::Node temp_node = node[it->first];
        PopulateNode(temp_node, it->second);
    }
}

template<class K, class V>
void MDCategoryMap::PopulateNode(YAML::Node& node, 
    const std::map<K, std::vector<std::vector<V>>>& data)
{
    node = YAML::Node(YAML::NodeType::Map);
    typename std::map<K, std::vector<std::vector<V>>>::const_iterator it;
    typename std::vector<std::vector<V>>::const_iterator it2;
    for(it = data.cbegin(); it != data.cend(); ++it)
    {
        YAML::Node key_node = node[it->first];
        for(it2 = it->second.cbegin(); it2 != it->second.cend(); ++it2)
        {
            YAML::Node vec_node;
            PopulateNode(vec_node, *it2);
            vec_node.SetStyle(YAML::EmitterStyle::Flow);
            key_node.push_back(vec_node);
        }
        key_node.SetStyle(YAML::EmitterStyle::Block);
    }
}

#endif  // MD_CATEGORY_MAP_H_