#include "Parser.h"

void Parser::outputDump(string outStem)
{
	printFile = true;
	outputStem = outStem;
	os.open(outStem + "_output_exact.txt");
	osPyspark.open(outStem + "_output_pyspark.txt");
	osPyspark << "time\tlvltyp1\tlvltyp2\tetime\tpress\tpflag\tgph\tzflag\ttemp\ttflag\trh\tdpdp\twdir\twspd\n";
}

void Parser::metaDump(string outStem)
{
	printMeta = true;
	outputStem = outStem;
	osMeta.open(outStem + "_meta_exact.txt");
	osMeta_pyspark.open(outStem + "_meta_pyspark.txt");
	osMeta_pyspark << "ID\tlatitude\tlongitude\televation\tstate\tfirstYear\tlastYear\tNOS\n";
}

void Parser::parseFile(ifstream& is)
{
	string line;
	
	getline(is, line);

	// parse each block (Header and Data Records)
	while (line.length() >= 71 && line[0] == '#') {
		Header header(line);
		getline(is, line);

		vector<Data_Record> drVec(header.getNum_levels());

		for (int i = 0; i < header.getNum_levels(); i++) {
			drVec[i] = Data_Record(line);
			getline(is, line);
		}

		Data_Block db(header, drVec);

		if (header.getYear() >= 1970) {
			data.push_back(db);
			numRecords = numRecords + drVec.size();
			numHeaders++;

			if (printFile) {
				db.pysparkOut(osPyspark);
				//db.coutData_Block();
			}
		}

		if (printFile) {
			db.fileOut(os);
		}
		
		
	}	
}

void Parser::parseMeta(ifstream& is)
{
	string line;

	getline(is, line);

	// parse each block (Header and Data Records)
	while (line.length() > 10) {
		Station_Info station_info(line);
		metadata.push_back(station_info);
		getline(is, line);
		
	
		if (printMeta) {
			station_info.fileOut(osMeta);
			station_info.pysparkOut(osMeta_pyspark);
		}

	}
}

string Parser::trim(string& line, int& whitespaceCount)
{
	whitespaceCount = 0;
	
	int index = 0;
	char* charTmp = new char[line.length() + 1]; // Note: this 10000 is too arbitrary
	int charCount = 0;

	// skip white space
	while (isspace(line[index])) {
		index++;
		whitespaceCount++;
	}

	while (!isspace(line[index]) && index < line.length()) {
		charTmp[charCount] = line[index];
		index++;
		charCount++;
	}

	// Terminate char array
	charTmp[charCount] = '\0';

	line.erase(0,index);
	
	string output(charTmp);

	delete[] charTmp;
	
	return output;
}

Columnizer& Parser::getColumns()
{
	columnizer.setColumns(numRecords, data);
	return columnizer;
}

Station_Columnizer& Parser::getMetadata_columns()
{
	stationColumnizer.setColumns(metadata);
	return stationColumnizer;
}