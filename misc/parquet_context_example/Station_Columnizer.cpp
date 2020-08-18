#include "Station_Columnizer.h"

void Station_Columnizer::size(int numRecords)
{
	id.resize(numRecords);
	latitude.resize(numRecords);
	longitude.resize(numRecords);
	elevation.resize(numRecords);
	state.resize(numRecords);
	stateNull.resize(numRecords, 1);
	name.resize(numRecords);
	firstYear.resize(numRecords);
	lastYear.resize(numRecords);
	NOS.resize(numRecords);
	NOSnull.resize(numRecords, 1);
	boolData.resize(numRecords);
}

void Station_Columnizer::setColumns(std::vector<Station_Info>& si)
{
	size(si.size());
	int counter = 0;


	for (int i = 0; i < si.size(); i++) {
		id[i] = std::string(si[i].id);
		latitude[i] = si[i].latitude;
		longitude[i] = si[i].longitude;
		elevation[i] = si[i].elevation;
		if (strcmp(si[i].state, "  ") == 0) {
			stateNull[i] = 0;
		}
		state[i] = std::string(si[i].state);
		
		name[i] = std::string(si[i].name); 
		firstYear[i] = si[i].firstYear;
		lastYear[i] = si[i].lastYear;
		NOS[i] = si[i].NOS;

		for (int j = 0; j < 2; j++) {
			int32List.push_back(j);
			stringList.push_back("Hello");
			if (i % 2 == 0)
				boolList.push_back(true);
			else
				boolList.push_back(false);
		}
			
		if (i % 2 == 0)
			boolData[i] = true;
		else
			boolData[i] = false;
		counter++;
	}

	NOSnull[0] = 0;
	NOSnull[5] = 0;
	NOSnull[6] = 0;
	NOSnull[10] = 0;

	if (counter != si.size()) {
		std::cout << "Warning -> numRecords != vector size \n";
	}
}
