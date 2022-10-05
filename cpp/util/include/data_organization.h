#ifndef MILSTD1553_TRANSLATION_DATA_ORG_H
#define MILSTD1553_TRANSLATION_DATA_ORG_H

#include <cstdint>
#include <cstring>
#include <vector>
#include <set>
#include <iterator>
#include <string>
#include <map>
#include <algorithm>
#include <numeric>
#include <utility>

class DataOrg
{
   private:
   public:
    DataOrg();
    ~DataOrg();

    // Search an array of class/struct C pointed to by data (with data_count elements) for search_key
    // in the c-string field "field" which contains N elements.
    template <class C, int N>
    uint32_t binary_search_struct_cstr(C* data, uint32_t& data_count, const char* search_key, char (C::*field)[N]);

    // Search an array of class/struct C pointed to by data (with data_count elements) for search_key
    // in the c-string field "field" which contains N elements. Only compare the first n characters,
    // where n is the number of characters in search_key.
    template <class C, int N>
    uint32_t binary_search_struct_cstr_part(C* data, uint32_t& data_count, const char* search_key, char (C::*field)[N]);

    // Binary search vector.
    uint32_t binary_search_string_vector(std::vector<std::string> data, std::string search_key);
    bool binary_search_string_vector_exists(std::vector<std::string> data, std::string search_key);

    // Binary search set.
    uint32_t binary_search_string_set(std::set<std::string> data, std::string search_key);

    template <typename T>
    bool binary_search_vector_exists(std::vector<T>& data, T& search_key);

    // Get truth of query: key "test_key" exists in map "test_map"
    template <typename KEY, typename VAL>
    bool key_in_map(std::map<KEY, VAL>& test_map, KEY& test_key);

    // Get vector of unique elements in cstring field "field" of struct/class C pointed to by
    // "data" with "count" number of elements. Vector of strings is returned instead of cstrings.
    template <class C, int N>
    std::vector<std::string> unique_struct_cstr_elements_by_field(C* data, uint32_t count, char (C::*field)[N]);

    // Get vector of unique elements in cstring field "field" of struct/class C pointed to by
    // "data" with "count" number of elements. Vector of strings is returned instead of cstrings.
    // This function is different from above in that only a subset of elements are compared for
    // uniqueness by selecting with a vector of pre-selected indices.
    template <class C, int N>
    std::vector<std::string> unique_struct_cstr_elements_by_field_and_indices(C* data,
                                                                              char (C::*field)[N], std::vector<uint32_t>& inds);

    //
    template <class C, int N>
    std::map<std::string, std::vector<uint32_t>> map_string_to_struct_indices_by_cstr_field(std::vector<std::string>& map_keys,
                                                                                            C* data, uint32_t count, char (C::*field)[N]);

    template <class C, int N>
    std::map<std::string, std::vector<uint32_t>> map_string_to_struct_indices_by_cstr_field_and_indices(std::vector<std::string>& map_keys,
                                                                                                        C* data, char (C::*field)[N], std::vector<uint32_t>& inds);

    template <class C, int N>
    std::map<std::string, std::vector<C>> map_string_to_struct_data_by_cstr_field_and_indices(std::vector<std::string>& map_keys,
                                                                                              C* data, char (C::*field)[N], std::vector<uint32_t>& inds);

    template <typename T>
    std::vector<size_t> arg_sort_ascending(const std::vector<T>& v);

    template <typename T>
    std::vector<size_t> arg_sort_descending(const std::vector<T>& v);

    //// Create a map from field type T (and member of C) to vector of C from
    //// original vector of C.
    //template<typename T, class C>
    //std::map<T, std::vector<C>> map_by_member(std::vector<C> data, T C::* field);

    //// Create a map from field type T (and member of C) to vector of C from
    //// original pointer to C (with the assumption of data_count elements).
    //template<typename T, class C>
    //std::map<T, std::vector<C>> map_by_member(C* data, uint32_t& data_count, T C::* field);

    //// Create a map from type T to vector of C using input of vector of T to
    //// organize the map. Data originate from vector of C. key_values
    //// are paired with data via index.
    //template<typename T, class C>
    //std::map<T, std::vector<C>> map_by_member(std::vector<C> data, std::vector<T> key_values);

    //// Create a map from type T to vector of C using input of vector of T to
    //// organize the map. Data originate from array of C. key_values
    //// are paired with data via index. key_values.size() must be equal to number of
    //// elements in array pointed to by data.
    //template<typename T, class C>
    //std::map<T, std::vector<C>> map_by_member(C* data, std::vector<T> key_values);
};

template <class C, int N>
std::map<std::string, std::vector<C>> DataOrg::map_string_to_struct_data_by_cstr_field_and_indices(std::vector<std::string>& map_keys,
                                                                                                   C* data, char (C::*field)[N], std::vector<uint32_t>& inds)
{
    std::map<std::string, std::vector<C>> the_map;
    std::vector<std::string>::iterator it_beg = map_keys.begin();
    std::vector<std::string>::iterator it_end = map_keys.end();

    // Insert empty mapped vectors for each of the keys.
    for (std::vector<std::string>::iterator it = it_beg; it != it_end; ++it)
    {
        std::vector<C> temp_vec;
        the_map.insert(std::pair<std::string, std::vector<C>>(*it, temp_vec));
    }

    // Fill the indices vectors.
    std::string field_val = "";
    for (std::vector<uint32_t>::iterator it = inds.begin(); it != inds.end(); ++it)
    {
        field_val = data[*it].*field;
        the_map[field_val].push_back(data[*it]);
    }

    return the_map;
}

template <class C, int N>
std::map<std::string, std::vector<uint32_t>> DataOrg::map_string_to_struct_indices_by_cstr_field_and_indices(std::vector<std::string>& map_keys,
                                                                                                             C* data, char (C::*field)[N], std::vector<uint32_t>& inds)
{
    std::map<std::string, std::vector<uint32_t>> the_map;
    std::vector<std::string>::iterator it_beg = map_keys.begin();
    std::vector<std::string>::iterator it_end = map_keys.end();

    // Insert empty mapped vectors for each of the keys.
    for (std::vector<std::string>::iterator it = it_beg; it != it_end; ++it)
    {
        std::vector<uint32_t> temp_vec;
        the_map.insert(std::pair<std::string, std::vector<uint32_t>>(*it, temp_vec));
    }

    // Fill the indices vectors.
    std::string field_val = "";
    for (std::vector<uint32_t>::iterator it = inds.begin(); it != inds.end(); ++it)
    {
        field_val = data[*it].*field;
        the_map[field_val].push_back(*it);
    }

    return the_map;
}

template <class C, int N>
std::map<std::string, std::vector<uint32_t>> DataOrg::map_string_to_struct_indices_by_cstr_field(
    std::vector<std::string>& map_keys, C* data, uint32_t count, char (C::*field)[N])
{
    std::map<std::string, std::vector<uint32_t>> the_map;
    std::vector<std::string>::iterator it_beg = map_keys.begin();
    std::vector<std::string>::iterator it_end = map_keys.end();

    // Insert empty mapped vectors for each of the keys.
    for (std::vector<std::string>::iterator it = it_beg; it != it_end; ++it)
    {
        std::vector<uint32_t> temp_vec;
        the_map.insert(std::pair<std::string, std::vector<uint32_t>>(*it, temp_vec));
    }

    // Fill the indices vectors.
    std::string field_val = "";
    for (uint32_t i = 0; i < count; i++)
    {
        field_val = data[i].*field;
        the_map[field_val].push_back(i);
    }

    return the_map;
}

template <class C, int N>
std::vector<std::string> DataOrg::unique_struct_cstr_elements_by_field_and_indices(C* data,
                                                                                   char (C::*field)[N], std::vector<uint32_t>& inds)
{
    // Fill a vector with the "field" value from each of the items in the array.
    std::vector<std::string> unique_elems;
    for (size_t i = 0; i < inds.size(); i++)
    {
        unique_elems.push_back(data[inds[i]].*field);
    }

    // Sort the items.
    std::sort(unique_elems.begin(), unique_elems.end());

    // Erase redundant values.
    unique_elems.erase(std::unique(unique_elems.begin(), unique_elems.end()), unique_elems.end());

    return unique_elems;
}

template <class C, int N>
std::vector<std::string> DataOrg::unique_struct_cstr_elements_by_field(C* data, uint32_t count, char (C::*field)[N])
{
    // Fill a vector with the "field" value from each of the items in the array.
    std::vector<std::string> unique_elems;
    for (uint32_t i = 0; i < count; i++)
    {
        unique_elems.push_back(data[i].*field);
    }

    // Sort the items.
    std::sort(unique_elems.begin(), unique_elems.end());

    // Erase redundant values.
    unique_elems.erase(std::unique(unique_elems.begin(), unique_elems.end()), unique_elems.end());

    return unique_elems;
}

template <class C, int N>
uint32_t DataOrg::binary_search_struct_cstr(C* data, uint32_t& data_count, const char* search_key, char (C::*field)[N])
{
    auto it = std::lower_bound(data, data + data_count, search_key,
                               [&](const C& lhs, const char* rhs) {
                                   if (std::strcmp(lhs.*field, rhs) < 0)
                                   {
                                       return true;
                                   }
                                   else
                                   {
                                       return false;
                                   }
                               });
    uint32_t idx = uint32_t(std::distance(data, it));
    if (!(std::strcmp(data[idx].*field, search_key)))
        return idx;
    return UINT32_MAX;
}

template <class C, int N>
uint32_t DataOrg::binary_search_struct_cstr_part(C* data, uint32_t& data_count, const char* search_key, char (C::*field)[N])
{
    size_t num = std::strlen(search_key);
    auto it = std::lower_bound(data, data + data_count, search_key,
                               [&](const C& lhs, const char* rhs) {
                                   if (std::strcmp(lhs.*field, rhs) < 0)
                                   {
                                       return true;
                                   }
                                   else
                                   {
                                       return false;
                                   }
                               });
    uint32_t idx = uint32_t(std::distance(data, it));
    if (!(std::strncmp(data[idx].*field, search_key, num)))
        return idx;
    return UINT32_MAX;
}

template <typename KEY, typename VAL>
bool DataOrg::key_in_map(std::map<KEY, VAL>& test_map, KEY& test_key)
{
    typename std::map<KEY, VAL>::iterator it;
    for (it = test_map.begin(); it != test_map.end(); ++it)
    {
        if (test_key == it->first)
            return true;
    }
    return false;
}

template <typename T>
bool DataOrg::binary_search_vector_exists(std::vector<T>& data, T& search_key)
{
    auto it = std::lower_bound(data.begin(), data.end(), search_key);
    if (it != data.end() && !(*it > search_key))
    {
        return true;
    }
    else
        return false;
}

template <typename T>
std::vector<size_t> DataOrg::arg_sort_ascending(const std::vector<T>& v)
{
    std::vector<size_t> idx(v.size());
    std::iota(idx.begin(), idx.end(), 0);

    std::sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) { return v[i1] < v[i2]; });
    return idx;
}

template <typename T>
std::vector<size_t> DataOrg::arg_sort_descending(const std::vector<T>& v)
{
    std::vector<size_t> idx(v.size());
    std::iota(idx.begin(), idx.end(), 0);

    std::sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) { return v[i1] > v[i2]; });
    return idx;
}

//template<typename T, class C>
//std::map<T, std::vector<C>> DataOrg::map_by_member(std::vector<C> data, T C::* field)
//{
//	size_t data_count = data.size();
//
//	// Get a vector of all "field" entries.
//	std::vector<T> field_entries(data_count);
//	uint32_t i = 0;
//	for (std::vector<C>::iterator it = data.begin(); it < data.end(); ++it)
//	{
//		field_entries[i] = it->*field;
//		i++;
//	}
//
//	// Get a set of the possible field values.
//	std::set<T> map_keys(field_entries.begin(), field_entries.end());
//
//	// Prepare the map by inserting an empty vector for each of the unique key values.
//	std::map<T, std::vector<C>> the_map;
//	for (auto key : map_keys)
//	{
//		std::vector<C> temp_vec;
//		the_map.insert(std::pair<T, std::vector<C>>(key, temp_vec));
//	}
//
//	// For each data entry, map it to the correct vector.
//	for (std::vector<C>::iterator it = data.begin(); it < data.end(); ++it)
//	{
//		the_map[it->*field].push_back(*it);
//	}
//
//	return the_map;
//}
//
//template<typename T, class C>
//std::map<T, std::vector<C>> DataOrg::map_by_member(C* data, uint32_t& data_count, T C::* field)
//{
//	// Get a vector of all "field" entries.
//	std::vector<T> field_entries(data_count);
//	for (uint32_t i = 0; i < data_count; i++)
//	{
//		field_entries[i] = data[i].*field;
//	}
//
//	// Get a set of the possible field values.
//	std::set<T> map_keys(field_entries.begin(), field_entries.end());
//
//	// Prepare the map by inserting an empty vector for each of the unique key values.
//	std::map<T, std::vector<C>> the_map;
//	for (auto key : map_keys)
//	{
//		std::vector<C> temp_vec;
//		the_map.insert(std::pair<T, std::vector<C>>(key, temp_vec));
//	}
//
//	// For each data entry, map it to the correct vector.
//	for (uint32_t i = 0; i < data_count; i++)
//	{
//		the_map[data[i].*field].push_back(data[i]);
//	}
//
//	return the_map;
//}
//
//template<typename T, class C>
//std::map<T, std::vector<C>> DataOrg::map_by_member(std::vector<C> data, std::vector<T> key_values)
//{
//	// Get a set of the possible field values.
//	std::set<T> map_keys(key_values.begin(), key_values.end());
//
//	// Prepare the map by inserting an empty vector for each of the unique key values.
//	std::map<T, std::vector<C>> the_map;
//	for (auto key : map_keys)
//	{
//		std::vector<C> temp_vec;
//		the_map.insert(std::pair<T, std::vector<C>>(key, temp_vec));
//	}
//
//	// For each data entry, map it to the correct vector.
//	for (uint32_t i = 0; i < key_values.size(); i++)
//	{
//		the_map[key_values[i]].push_back(data[i]);
//	}
//
//	return the_map;
//}
//
//template<typename T, class C>
//std::map<T, std::vector<C>> DataOrg::map_by_member(C* data, std::vector<T> key_values)
//{
//	// Get a set of the possible field values.
//	std::set<T> map_keys(key_values.begin(), key_values.end());
//
//	// Prepare the map by inserting an empty vector for each of the unique key values.
//	std::map<T, std::vector<C>> the_map;
//	for (auto key : map_keys)
//	{
//		std::vector<C> temp_vec;
//		the_map.insert(std::pair<T, std::vector<C>>(key, temp_vec));
//	}
//
//	// For each data entry, map it to the correct vector.
//	for (uint32_t i = 0; i < key_values.size(); i++)
//	{
//		the_map[key_values[i]].push_back(data[i]);
//	}
//
//	return the_map;
//}

#endif