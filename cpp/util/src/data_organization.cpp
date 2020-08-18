#include "data_organization.h"

DataOrg::DataOrg()
{

}

DataOrg::~DataOrg()
{

}

uint32_t DataOrg::binary_search_string_vector(std::vector<std::string> data, std::string search_key)
{
	auto it = std::lower_bound(data.begin(), data.end(), search_key);
	if (it != data.end() && (*it == search_key))
	{
		return uint32_t(it - data.begin());
	}
	else
		return UINT32_MAX;
}

bool DataOrg::binary_search_string_vector_exists(std::vector<std::string> data, std::string search_key)
{
	auto it = std::lower_bound(data.begin(), data.end(), search_key);
	if (it != data.end() && (*it == search_key))
	{
		return true;
	}
	else
		return false;
}

uint32_t DataOrg::binary_search_string_set(std::set<std::string> data, std::string search_key)
{
	auto it = std::lower_bound(data.begin(), data.end(), search_key);
	if (it != data.end() && (*it == search_key))
	{
		return uint32_t(std::distance(data.begin(), it));
	}
	else
		return UINT32_MAX;
}