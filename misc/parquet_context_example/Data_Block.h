#pragma once
#include "Data_Record.h"
#include "Header.h"
#include<vector>


class Data_Block
{
	private: 
		

	public:
		vector<Data_Record> dr;
		Header header;
		// default constructor
		Data_Block() {};

		// initialization constructor
		Data_Block(Header headerIn, vector<Data_Record> drIN) {
			dr = drIN;
			header = headerIn;
		};

		void coutData_Block();

		void fileOut(ofstream& os);
		void pysparkOut(ofstream& osPyspark_readable);

};

