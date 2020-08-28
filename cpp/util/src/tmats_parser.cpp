
#include <fstream>
#include <map>
#include <string>
#include <regex>
#include <iostream>
#include "tmats_parser.h"


CodeName::CodeName(std::string raw)
{
    plaintext = raw;
    std::smatch match;
    while (regex_search(raw, match, (std::regex)"([0-9A-Z\\\\-]*)([a-z]+)"))
    {
        regex_string += (std::string)match.prefix() + match.str(1);
        regex_string += "([0-9]+)";
        subattrs.push_back(match[2]);
        raw = match.suffix().str();
    }

    // Find and escape slashes before building the regex.
    std::string escaped;
    for (int i=0; i<regex_string.length();i++)
    {
        if (regex_string[i] == '\\')
            escaped += "\\\\";
        else
            escaped += regex_string[i];
    }

    regex_string = escaped;
    re = (std::regex)escaped;
}


// Take a regex match (from CodeName->re) and parse out the subattrs (x, n, etc.).
std::map<std::string, int> CodeName::groups(std::smatch match)
{
    std::map<std::string, int> result;
    for (int i=0; i<subattrs.size(); i++)
        result[subattrs[i]] = stoi(match.str(i+1));
    return result;
}


TMATSParser::TMATSParser(std::string tmats){
    raw = tmats;
}


// Takes two TMATS attribute codenames (as seen in the Chapter 9 standard) and
// returns matching values as key:value pairs. ie: map_attrs("R-n", "G-n")
// returns the values of all found pairs of R-n:G-n where n is some number.
std::map<std::string, std::string> TMATSParser::MapAttrs(std::string key_attr, std::string value_attr)
{
    CodeName *key = new CodeName(key_attr);
    CodeName *value = new CodeName(value_attr);

    // Find matching keys and values;
    std::map<std::string, std::map<std::string, int>> keys;
    std::map<std::string, std::map<std::string, int>> values;
    std::smatch matches;
    std::string s = raw.substr(0, raw.length());
    std::regex r = (std::regex)"([0-9A-Za-z\\\\-]+):(.*);";
    for (std::sregex_iterator i = std::sregex_iterator(s.begin(), s.end(), r);
        i != std::sregex_iterator(); i++)
    {
        std::smatch m = *i;
        std::string codename = m.str(1);
        std::smatch code_matches;

        // Get the value of the key attribute and its subattrs (x, n, etc.).
        if (regex_search(codename, code_matches, key->re))
            keys[m.str(2)] = key->groups(code_matches);
        else if (regex_search(codename, code_matches, value->re))
            values[m.str(2)] = key->groups(code_matches);
    }

    // Connect keys and values
    std::map<std::string, std::string> result;
    for (auto i : keys)
    {
        for (auto j : values)
        {
            if (i.second.size() != j.second.size())
                continue;
            int match = 1;
            for (auto subattr : i.second)
            {
                if (subattr.second != j.second[subattr.first])
                {
                    match = 0;
                    break;
                }
            }
            if (match)
            {
                result[i.first] = j.first;
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
