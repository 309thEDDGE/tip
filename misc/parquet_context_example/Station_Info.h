#pragma once

#include<string>
#include<iostream>
#include <fstream>
#include <iomanip>


class Station_Info
{
	private: 
		void parse(std::string);

	public:

		// default constructor
		Station_Info() {};

		// parse string
		Station_Info(std::string line) { parse(line); };

		void fileOut(std::ofstream&);
		void pysparkOut(std::ofstream&);
		char id[12];
		double latitude;
		double longitude;
		float elevation;
		char state[3];
		char name[31];
		int16_t firstYear;
		int16_t lastYear;
		int32_t NOS;
};

