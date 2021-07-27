
#include "tmats_parser.h"

CodeName::CodeName(std::string raw, bool show_debug)
{
    debug = show_debug;
    plaintext = raw;
    std::smatch match;
    std::string prefix;
    std::string match_substr;
    if (debug)
        printf("\n\nCodeName::CodeName(): raw = %s\n\n", raw.c_str());
    while (regex_search(raw, match, (std::regex) "([0-9A-Z\\\\-]*)([a-z]+)"))
    {
        prefix = match.prefix();
        match_substr = match.str(1);
        regex_string += prefix + match_substr;
        regex_string += "([0-9]+)";
        if (debug)
        {
            printf(
                "raw %s, prefix %s, match_substr %s, regex_string %s, "
                "subattr %s\n",
                raw.c_str(), prefix.c_str(), match_substr.c_str(),
                regex_string.c_str(), match.str(2).c_str());
        }
        subattrs.push_back(match.str(2));
        raw = match.suffix().str();
    }

    // Find and escape slashes before building the regex.
    std::string escaped;
    for (int i = 0; i < regex_string.length(); i++)
    {
        if (regex_string[i] == '\\')
            escaped += "\\\\";
        else
            escaped += regex_string[i];
    }

    regex_string = escaped;
    re = (std::regex)escaped;
    if (debug)
        printf("\nescaped regex_string %s\n", regex_string.c_str());
}

// Take a regex match (from CodeName->re) and parse out the subattrs (x, n, etc.).
std::map<std::string, int> CodeName::groups(std::smatch match)
{
    std::map<std::string, int> result;
    for (int i = 0; i < subattrs.size(); i++)
        result[subattrs[i]] = stoi(match.str(i + 1));
    return result;
}

TMATSParser::TMATSParser(std::string tmats, bool show_debug)
{
    raw = tmats;
    debug = show_debug;
}

// Takes two TMATS attribute codenames (as seen in the Chapter 9 standard) and
// returns matching values as key:value pairs. ie: map_attrs("R-n", "G-n")
// returns the values of all found pairs of R-n:G-n where n is some number.
//std::map<std::string, std::string> TMATSParser::MapAttrs(std::string key_attr, std::string value_attr)
//{
//    CodeName key(key_attr, debug);
//    CodeName value(value_attr, debug);
//
//    // Find matching keys and values;
//    std::map<std::string, std::map<std::string, int>> keys;
//    std::map<std::string, std::map<std::string, int>> values;
//    std::smatch matches;
//    std::string s = raw.substr(0, raw.length());
//    std::regex r = (std::regex)"([0-9A-Za-z\\\\-]+):(.*);";
//    if (debug)
//        printf("\n\nTMATSParser::MapAttrs():\n");
//    for (std::sregex_iterator i = std::sregex_iterator(s.begin(), s.end(), r);
//        i != std::sregex_iterator(); i++)
//    {
//        std::smatch m = *i;
//        std::string codename = m.str(1);
//        std::smatch code_matches;
//
//        if (debug)
//            printf("\nbasic match codename: %s\n", codename);
//
//        // Get the value of the key attribute and its subattrs (x, n, etc.).
//        if (regex_search(codename, code_matches, key.re))
//        {
//            keys[m.str(2)] = key.groups(code_matches);
//            if (debug)
//                printf("KEY match (%s): %s\n", key_attr.c_str(), m.str(2).c_str());
//        }
//        else if (regex_search(codename, code_matches, value.re))
//        {
//            values[m.str(2)] = key.groups(code_matches);
//            if (debug)
//                printf("VALUE match (%s): %s\n", value_attr.c_str(), m.str(2).c_str());
//        }
//    }
//
//    if (debug)
//    {
//        IterableTools iter;
//        std::vector<std::string> cols({ "label", "value" });
//        std::string map_name;
//        char buff[100];
//        std::map<std::string, int> temp_map;
//        printf("\nKeys map (%s):\n", key_attr.c_str());
//        for (std::map<std::string, std::map<std::string, int>>::const_iterator it =
//            keys.begin(); it != keys.end(); ++it)
//        {
//            //map_name = it->first;
//            sprintf(buff, "mapped-to value: %s", it->first.c_str());
//            map_name = std::string(buff);
//            temp_map = it->second;
//            iter.PrintMapWithHeader_KeyToValue(temp_map, cols, map_name);
//        }
//
//        printf("\nValues map (%s):\n", value_attr.c_str());
//        for (std::map<std::string, std::map<std::string, int>>::const_iterator it =
//            values.begin(); it != values.end(); ++it)
//        {
//            //map_name = it->first;
//            sprintf(buff, "mapped-to value: %s", it->first.c_str());
//            map_name = std::string(buff);
//            temp_map = it->second;
//            iter.PrintMapWithHeader_KeyToValue(temp_map, cols, map_name);
//        }
//    }
//
//    // Connect keys and values
//    std::map<std::string, std::string> result;
//    for (auto i : keys)
//    {
//        for (auto j : values)
//        {
//            if (i.second.size() != j.second.size())
//                continue;
//            int match = 1;
//            for (auto subattr : i.second)
//            {
//                if (subattr.second != j.second[subattr.first])
//                {
//                    match = 0;
//                    break;
//                }
//            }
//            if (match)
//            {
//                result[i.first] = j.first;
//                break;
//            }
//        }
//    }
//
//    return result;
//}

// Takes two TMATS attribute codenames (as seen in the Chapter 9 standard) and
// returns matching values as key:value pairs. ie: map_attrs("R-n", "G-n")
// returns the values of all found pairs of R-n:G-n where n is some number.
std::map<std::string, std::string> TMATSParser::MapAttrs(std::string key_attr, std::string value_attr)
{
    CodeName key(key_attr, debug);
    CodeName value(value_attr, debug);

    // Find matching keys and values;
    using subattr_index_map = std::map<std::string, int>;
    using subattr_data_tuple = std::tuple<subattr_index_map, std::string>;
    std::vector<subattr_data_tuple> keys;
    std::vector<subattr_data_tuple> values;
    std::smatch matches;
    std::string s = raw.substr(0, raw.length());
    std::regex r = (std::regex) "([0-9A-Za-z\\\\-]+):(.*);";
    if (debug)
        printf("\n\nTMATSParser::MapAttrs():\n");
    for (std::sregex_iterator i = std::sregex_iterator(s.begin(), s.end(), r);
         i != std::sregex_iterator(); i++)
    {
        std::smatch m = *i;
        std::string codename = m.str(1);
        std::smatch code_matches;

        if (debug)
            printf("\nbasic match codename: %s\n", codename.c_str());

        // Get the value of the key attribute and its subattrs (x, n, etc.).
        if (regex_search(codename, code_matches, key.re))
        {
            subattr_data_tuple temp_tuple(key.groups(code_matches), m.str(2));
            keys.push_back(temp_tuple);
            if (debug)
                printf("KEY match (%s): %s\n", key_attr.c_str(), m.str(2).c_str());
        }
        else if (regex_search(codename, code_matches, value.re))
        {
            subattr_data_tuple temp_tuple(value.groups(code_matches), m.str(2));
            values.push_back(temp_tuple);
            if (debug)
                printf("VALUE match (%s): %s\n", value_attr.c_str(), m.str(2).c_str());
        }
    }

    if (debug)
    {
        IterableTools iter;
        std::vector<std::string> cols({"label", "value"});
        std::string map_name;
        char buff[100];
        std::map<std::string, int> temp_map;
        printf("\nKeys map (%s):\n", key_attr.c_str());
        for (std::vector<subattr_data_tuple>::const_iterator it =
                 keys.begin();
             it != keys.end(); ++it)
        {
            snprintf(buff, std::get<1>(*it).size() + 17, "mapped-to value: %s", std::get<1>(*it).c_str());
            map_name = std::string(buff);
            temp_map = std::get<0>(*it);
            iter.PrintMapWithHeader_KeyToValue(temp_map, cols, map_name);
        }

        printf("\nValues map (%s):\n", value_attr.c_str());
        for (std::vector<subattr_data_tuple>::const_iterator it =
                 values.begin();
             it != values.end(); ++it)
        {
            snprintf(buff, std::get<1>(*it).size() + 17, "mapped-to value: %s", std::get<1>(*it).c_str());
            map_name = std::string(buff);
            temp_map = std::get<0>(*it);
            iter.PrintMapWithHeader_KeyToValue(temp_map, cols, map_name);
        }
    }

    // Connect keys and values
    std::map<std::string, std::string> result;
    for (auto i : keys)
    {
        for (auto j : values)
        {
            subattr_index_map keys_map = std::get<0>(i);
            subattr_index_map vals_map = std::get<0>(j);
            if (keys_map.size() != vals_map.size())
                continue;
            int match = 1;
            for (auto subattr : keys_map)
            {
                if (subattr.second != vals_map[subattr.first])
                {
                    match = 0;
                    break;
                }
            }
            if (match)
            {
                result[std::get<1>(i)] = std::get<1>(j);
                break;
            }
        }
    }

    return result;
}

/* Sample Usage */

/* int main(){ */
/*     map<string, string> result ; */
/*     ifstream t("tmats.txt"); */
/*     string tmats((istreambuf_iterator<char>(t)), istreambuf_iterator<char>()); */

/*     TMATSParser *parser = new TMATSParser(tmats); */
/*     cout << "Channel Names (R-x\\DSI-n)" << endl; */
/*     result = parser->map_attrs(R"(R-x\TK1-n)", R"(R-x\DSI-n)"); */
/*     for (auto const& i : result) */
/*         cout << i.first << ": " << i.second << endl; */

/*     cout << endl << "Channel Types (R-x\\CDT-n)" << endl; */
/*     result = parser->map_attrs(R"(R-x\TK1-n)", R"(R-x\CDT-n)"); */
/*     for (auto const& i : result) */
/*         cout << i.first << ": " << i.second << endl; */

/*     return 0; */
/* } */
