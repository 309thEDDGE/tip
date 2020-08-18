#include "Station_Info.h"

void Station_Info::parse(std::string line)
{
	// parsed according to https://www1.ncdc.noaa.gov/pub/data/igra/igra2-list-format.txt

	if (line.length() < 88)
		std::cout << "ERROR -> line isn't long enough to parse data record!\n";

	// ID
	strcpy_s(id, line.substr(0, 11).c_str());

	// Latitude
	latitude = atof(line.substr(12, 8).c_str());

	// Longitude
	longitude = atof(line.substr(21, 9).c_str());

	// Elevation
	elevation = atof(line.substr(31, 6).c_str());
	
	// State
	/*std::string temp = line.substr(38, 2);
	if (temp == "  ") {
		temp = "NA";
		strcpy_s(state, temp.c_str());
	}else
		strcpy_s(state, temp.c_str());*/
	strcpy_s(state, line.substr(38, 2).c_str());

	// Name
	strcpy_s(name,line.substr(41, 30).c_str());

	// firstYear
	firstYear = stoi(line.substr(72, 4));

	// lastYear
	lastYear = stoi(line.substr(77, 4));

	// NOS
	NOS = stoi(line.substr(82, 6));
}

void Station_Info::fileOut(std::ofstream& os)
{
	os << std::fixed << std::setw(11) << id << " " << std::setw(8) << std::setprecision(4) << latitude << " " << std::setw(9) << std::setprecision(4) << longitude << " " << std::setw(6) << std::setprecision(1) << elevation << " " << std::setw(2)
		<< state << " " << std::setw(30) << name << " " << std::setw(4) << firstYear << " " << std::setw(4) << lastYear << " " << std::setw(6) << NOS << std::endl;
}


void Station_Info::pysparkOut(std::ofstream& osPyspark)
{
	osPyspark << std::fixed << std::setw(11) << id << "\t" << std::setw(8) << std::setprecision(4) << latitude << "\t" << std::setw(9) << std::setprecision(4) << longitude << "\t" << std::setw(6) << std::setprecision(1) << elevation << "\t";
	if (strcmp(state, "  ") == 0)
		osPyspark << std::setw(2) << "NA" << "\t";
	else
		osPyspark << std::setw(2) << state << "\t";
	osPyspark << std::setw(4) << firstYear << "\t" << std::setw(4) << lastYear << "\t" << std::setw(6) << NOS << std::endl;
}