#ifndef YAMLREADER_H
#define YAMLREADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "yaml-cpp/yaml.h"
#include "yaml-cpp/exceptions.h"

class YamlReader
{
   private:
    YAML::Node node;
    std::stringstream ss;
    template <typename T>
    bool Read(std::string parameter, T& output, bool print = false);

   public:
    YamlReader() {}
    ~YamlReader() {}

    bool LinkFile(std::string file_name);

    /*
	Ingest yaml from an input string as an alternative to reading from a
	file. After YamlReader is instantiated, exactly one of LinkFile or
	IngestYamlAsString must called in order to use Read or GetParams.

	Args:
		yaml_matter	--> Input string with yaml content

	Return:
		True if ingest succeeded and input matter can be interpreted
		as yaml. False otherwise.
	*/
    bool IngestYamlAsString(const std::string& yaml_matter);

    // GetParams reads the parameter specified from the yaml file
    // The value is returned through the output parameter passed by reference
    // If print is true, it will print to console if the read failed
    //
    // returns: true if the value was successfully read into the output
    //		    datatype & the parameter exists in the file, false otherwise
    template <typename T>
    bool GetParams(std::string parameter, T& output, bool print);

    // GetParams reads the parameter specified from the yaml file
    // The value is returned through the output parameter passed by reference
    // If print is true, it will print to console if the read failed
    // This overloaded function also checks if boundary conditions are met
    // Note: Only use this function with values that can be compared using < and >
    //
    // returns: true if the value was successfully read into the output
    //		    datatype & the parameter exists in the file, & the value read
    //			was within the inclusive boundary conditions, false otherwise
    template <typename T>
    bool GetParams(std::string parameter, T& output, T lower_bound, T upper_bound, bool print = false);


    /*
    Get map node parameter casted to output type.

    Args:
        node        --> YAML::Node which must be a map
        parameter   --> Name of parameter in map to cast
        output      --> Variable in which to store the output

    Return:
        True if node is a map and the mapped parameter can be obtained
        as a the data type of the output variable. False otherwise.
    */
   template <typename T>
   static bool GetMapNodeParameter(const YAML::Node& node, std::string parameter,
                            T& output);

    /*
    Get sequence node casted to vector of output type.

    Args:
        node        --> YAML::Node which must be a sequence
        output      --> Vector in which to store output

    Return:
        True if node is a sequence and the data in the sequence
        can be converted to vector of the type specified by the
        output vector type. False otherwise.
    */
   template <typename T>
   static bool GetSequenceNodeVector(const YAML::Node& node, std::vector<T>& output);
};

template <typename T>
inline bool YamlReader::Read(std::string parameter, T& output, bool print)
{
    // If the parameter does not exist return false
    if (!node[parameter])
    {
        if (print)
            printf("\nNonexistent config parameter %s\n", parameter.c_str());
        return false;
    }
    else
    {
        try
        {
            output = node[parameter].as<T>();
        }
        catch (...)
        {
            if (print)
                printf("\nInvalid read for config parameter %s\n", parameter.c_str());
            return false;
        }
        return true;
    }
}

template <typename T>
bool YamlReader::GetParams(std::string parameter, T& output, bool print)
{
    return Read(parameter, output, print);
}

template <typename T>
inline bool YamlReader::GetParams(std::string parameter, T& output, T lower_bound, T upper_bound, bool print)
{
    bool success = Read(parameter, output, print);

    if (success)
    {
        if (output < lower_bound || output > upper_bound)
        {
            if (print)
            {
                ss << "\nParameter " << parameter << " (" << output << ") is out of range [" << lower_bound << " , " << upper_bound << "]\n";
                printf("%s", ss.str().c_str());
            }
            return false;
        }
        return true;
    }
    else
    {
        return false;
    }
}


template <typename T>
bool YamlReader::GetMapNodeParameter(const YAML::Node& node, std::string parameter,
                        T& output)
{
    if(!node.IsMap())
        return false;

    if(!node[parameter])
        return false;

    try
    {
        output = node[parameter].as<T>();
    }
    catch(YAML::RepresentationException& e)
    {
        return false;
    }
    return true;                        
}



template <typename T>
bool YamlReader::GetSequenceNodeVector(const YAML::Node& node, std::vector<T>& output)
{
    if(!node.IsSequence())
        return false;

    try
    {
        for(YAML::const_iterator it = node.begin(); it != node.end(); ++it)
        {
            output.push_back(it->as<T>());
        }
    }
    catch(YAML::RepresentationException& e)
    {
        return false;
    }
    
    return true;
}

#endif

/*	Examples on how to use the yaml-cpp libraries
	This example uses the example.yaml file provided in
	the "misc" folder

YAML::Node config =
YAML::LoadFile("misc\\example.yaml");

if (config["platform"]) {
	std::string platform = config["platform"].as<std::string>();
}

// Boolean
if (config["booleanTest"]) {
	bool booleanTest = config["booleanTest"].as<bool>();
	printf("boolean test\n");
}

// Int
if (config["integerTest"]) {
	int integerTest = config["integerTest"].as<int>();
	printf("integer test\n");
}

// Integer List
if (config["integerList"]) {
	std::vector<uint64_t> integerList =
	config["integerList"].as<std::vector<uint64_t>>();
}

// Key to list of lists
if (config["corrections"]) {
	std::vector<std::vector<std::string>> corrections =
	config["corrections"].as<std::vector<std::vector<std::string>>>();
}

// Key to Key/Value pairs
if (config["corrections2"]) {
	std::map<std::string, std::string> temp =
	config["corrections2"].as<std::map<std::string, std::string>>();
}
*/
