// tmats_item.cpp

#include "tmats_item.h"

TMATSItem::~TMATSItem()
{

}

TMATSItem::TMATSItem(std::string code, TMATSDataType data_type) : index_markers("xnmdwyeipsr"),
raw_code(code), dtype(data_type), match_regex(), nint_regex("[0-9]+")
{
	#ifdef DEBUG
	if (DEBUG > 2)
		printf("TMATS input code: %s, data type: %s\n", raw_code.c_str(), item_type_desc().c_str());
	#endif
	create_regex();
}

TMATSItem::TMATSItem(const TMATSItem& c) : index_markers("xnmdwyeipsr"),
	nint_regex("[0-9]+")
{
	
	ids.reserve(c.ids.size());
	ids.assign(c.ids.begin(), c.ids.end());
	indices.reserve(c.indices.size());
	indices.assign(c.indices.begin(), c.indices.end());
	raw_data.reserve(c.raw_data.size());
	raw_data.assign(c.raw_data.begin(), c.raw_data.end());
	dtype = c.dtype;
	raw_code = c.raw_code;
	raw_regex = c.raw_regex;
	match_regex = std::regex(raw_regex);
	/*printf("in copy constructor, raw code %s, raw_regex %s\n", raw_code.c_str(),
		raw_regex.c_str());*/

}

void TMATSItem::operator = (const TMATSItem& c)
{
	ids.assign(c.ids.begin(), c.ids.end());
	indices.assign(c.indices.begin(), c.indices.end());
	raw_data.assign(c.raw_data.begin(), c.raw_data.end());
	dtype = c.dtype;
	raw_code = c.raw_code;
	match_regex = c.match_regex;
}

void TMATSItem::create_regex()
{
	size_t pos = 0;
	size_t len = nint_regex.length();
	raw_regex = raw_code;
	#ifdef DEBUG
	if (DEBUG > 2)
		printf("raw_regex: %s\n", raw_regex.c_str());
	#endif
	for (std::string::const_iterator it = index_markers.cbegin(); it != index_markers.cend(); ++it)
	{
		pos = raw_regex.find(*it);
		if (pos != std::string::npos)
		{
			raw_regex.erase(pos, 1);
			#ifdef DEBUG
			if (DEBUG > 4)
				printf("After erase: %s\n", raw_regex.c_str());
			#endif

			raw_regex.insert(pos, nint_regex);
			#ifdef DEBUG
			if (DEBUG > 4)
				printf("After insert: %s\n", raw_regex.c_str());
			#endif
		}
	}
	
	#ifdef DEBUG
	if (DEBUG > 2)
		printf("Regex string is %s\n", raw_regex.c_str());
	#endif
	
	/*match_regex = std::regex(raw_regex.c_str());
	std::string code = "R-1\\DSI-1";
	printf("my code is %s\n", code.c_str());
	if (std::regex_match(code, match_regex))
		printf("matched 1\n");*/
}


std::string TMATSItem::item_type_desc()
{
	switch (dtype)
	{
	case TMATSDataType::INT:
		return "INT";
	case TMATSDataType::FLT:
		return "FLT";
	case TMATSDataType::STR:
		return "STR";
	default:
		return "";
	}
}

void TMATSItem::data(uint16_t index, int32_t& val)
{
	if (dtype != TMATSDataType::INT)
	{
		printf("TMATSItem::data(): Item type configured as %s\n", item_type_desc().c_str());
		val = INT32_MAX;
	}
	val = int32_t(std::stoi(raw_data[index]));
}

void TMATSItem::data(uint16_t index, float& val)
{
	if (dtype != TMATSDataType::FLT)
	{
		printf("TMATSItem::data(): Item type configured as %s\n", item_type_desc().c_str());
		val = std::numeric_limits<float>::max();
	}
	val = std::stof(raw_data[index]);
}

void TMATSItem::data(uint16_t index, std::string& val)
{
	if (dtype != TMATSDataType::STR)
	{
		printf("TMATSItem::data(): Item type configured as %s\n", item_type_desc().c_str());
		val = "NONE";
	}
	val = raw_data[index];
}


void TMATSItem::match_item(std::string test_code, std::string rawdata)
{
	// Check if the regex matches the input code string.
	if (std::regex_match(test_code, match_regex))
	{
		raw_data.push_back(rawdata);
		std::vector<uint16_t> temp_indices;
		#ifdef DEBUG
		if (DEBUG > 3)
		{
			printf("\nMatched code %s, rawdata is %s, rawdata size %zu\n", test_code.c_str(), 
				raw_data[raw_data.size() - 1].c_str(), raw_data.size());
		}
		#endif
		
		// Extract the id and index.
		//size_t len = code.length();
		size_t pos = 0;
		size_t next_pos = 0;
		bool first = true;
		while ((pos = test_code.find('-', pos)) != std::string::npos)
		{
			#ifdef DEBUG
			if (DEBUG > 4)
				printf("pos is %zu\n", pos);
			#endif
			if (first)
			{
				// If it's the first hyphen, the value following it is the id.
				//size_t backslash_pos = code.find('\\', pos+1);
				next_pos = test_code.find('\\', pos + 1);
				if (pos != std::string::npos)
				{
					#ifdef DEBUG
					if (DEBUG > 3)
						printf("ID substring is %s\n", test_code.substr(pos + 1, next_pos-(pos+1)).c_str());
					#endif
					ids.push_back(uint16_t(std::stoi(test_code.substr(pos + 1, next_pos-(pos+1)))));
					pos = next_pos + 2;
				}
				else
					return;
				first = false;
			}
			else
			{
				// All other hyphens are followed by various integer indices, 
				// which shall all be stored, in descending order, in 'index'.
				// The position of the next hyphen or the end of the string marks
				// the end of the index.
				if ((next_pos = test_code.find('-', pos + 1)) == std::string::npos)
				{
					// Extract from the current hyphen to the end of the string.
					#ifdef DEBUG
					if (DEBUG > 3)
						printf("index substring is %s\n", test_code.substr(pos + 1, test_code.length()-(pos+1)).c_str());
					#endif
					temp_indices.push_back(uint16_t(std::stoi(test_code.substr(pos+1, test_code.length()))));
					//printf("size id: %zu\n", ids.size());
					indices.push_back(temp_indices);
					return;
				}
				else
				{
					// Extract to the next hyphen.
					#ifdef DEBUG
					if (DEBUG > 3)
						printf("index substring is %s\n", test_code.substr(pos + 1, next_pos - (pos+1)).c_str());
					#endif
					temp_indices.push_back(uint16_t(std::stoi(test_code.substr(pos + 1, next_pos - (pos+1)))));
				}
				pos++;
			}
		}
		//printf("size id: %zu\n", ids.size());
		indices.push_back(temp_indices);
	}
}

std::string TMATSItem::code()
{
	return raw_code;
}

std::vector<uint16_t> TMATSItem::match_ids(uint16_t id_to_match)
{
	std::vector<uint16_t> matched_ids;
	//printf("id to match: %hu\n", id_to_match);
	for (size_t i = 0; i < ids.size(); i++)
	{
		//printf("id[%zu]: %hu\n", i, ids[i]);
		if (ids[i] == id_to_match)
			matched_ids.push_back(uint16_t(i));
	}
	return matched_ids;
}

std::vector<uint16_t> TMATSItem::subids(uint16_t index)
{
	return indices[index];
}