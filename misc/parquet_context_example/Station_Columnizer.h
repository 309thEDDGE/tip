#pragma once
#include<string>
#include<vector>
#include"Station_Info.h"

class Station_Columnizer
{
private:
	void size(int numRecords);

public:
	Station_Columnizer() {};

	std::vector<std::string> id;
	std::vector<double> latitude;
	std::vector<double> longitude;
	std::vector<float> elevation;
	std::vector<std::string> state;
	std::vector<uint8_t> stateNull;
	std::vector<std::string> name;
	std::vector<int16_t> firstYear;
	std::vector<int16_t> lastYear;
	std::vector<int32_t> NOS;
	std::vector<uint8_t> NOSnull;

	std::vector<int32_t> int32List;
	std::vector<std::string> stringList;
	std::vector<uint8_t> boolData;
	std::vector<uint8_t> boolList;

	void setColumns(std::vector<Station_Info>& si);

	
};

