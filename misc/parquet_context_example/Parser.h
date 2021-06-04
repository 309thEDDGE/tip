#pragma once
#include<iostream>
#include<fstream>
#include<string>
#include <experimental/filesystem>

#include "Data_Block.h"
#include "Data_Record.h"
#include "Header.h"
#include "Columnizer.h"
#include "Station_Columnizer.h"
#include "Station_Info.h"


using namespace std;

class Parser
{
	private:
		vector<Data_Block> data;
		vector<Station_Info> metadata;
		bool printFile;
		bool printMeta;
		int numRecords;
		int numHeaders;
		Columnizer columnizer;
		Station_Columnizer stationColumnizer;
		string outputStem;
		ofstream os;
		ofstream osPyspark;
		ofstream osMeta;
		ofstream osMeta_pyspark;

	public:
		Parser() {
			printFile = false;
			printMeta = false;
			numRecords = 0;
			numHeaders = 0;
		};

		~Parser() {
			os.close();
			osPyspark.close();
			osMeta.close();
			osMeta_pyspark.close();
		};

		void outputDump(string outStem);
		void metaDump(string outStem);
		void parseFile(ifstream& is);
		void parseMeta(ifstream& is);
		string trim(string& line, int& whitespaceCount);
		Columnizer& getColumns();
		Station_Columnizer& getMetadata_columns();
};

