#ifndef UTILITYCLASSES_H
#define UTILITYCLASSES_H

#include<string>
#include <set>
#include <vector>

class ChannelIDBusName 
{
public:
	std::set<size_t> channel_ids_;
	std::set<std::string> bus_names_;
}



#endif