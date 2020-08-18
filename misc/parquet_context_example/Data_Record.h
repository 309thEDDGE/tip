#pragma once
#include<stdint.h>
#include<string>
#include<fstream>
#include<iostream>
#include<iomanip>

using namespace std;

class Data_Record
{
	private:
		void parse(string);
		void flag_to_bool(char flag, bool& output);
		void etime_to_sec(int32_t* output);
		
	public:
		uint16_t lvltyp1;
		uint16_t lvltyp2;
		int16_t etime;
		int32_t press;
		int32_t gph;
		int16_t temp;
		bool pflag_b;
		bool zflag_b;
		bool tflag_b;
		char zflag;
		char pflag;
		char tflag;
		int16_t rh;
		int16_t dpdp;
		int16_t wdir;
		int16_t wspd;
		int32_t output_etime = 0;
		// Default constructor
		Data_Record() {};

		// initialize with a line
		Data_Record(string line) {
			parse(line);
		};

		void coutData_Record();

		void fileOut(ofstream& os);
		void pysparkOut(ofstream& osPyspark, uint64_t& headerTime);
		
};

