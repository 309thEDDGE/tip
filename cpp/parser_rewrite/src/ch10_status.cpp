#include "ch10_status.h"

std::string Ch10StatusString(const Ch10Status& status)
{
    return ch10status_to_string_map.count(status)
               ? ch10status_to_string_map.at(status)
               : "Ch10StatusString(): Not a valid Ch10Status!";
}