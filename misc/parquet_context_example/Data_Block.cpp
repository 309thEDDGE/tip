#include "Data_Block.h"

void Data_Block::coutData_Block()
{
	header.coutHeader();
	cout << "\n";
	for (int i = 0; i < dr.size(); i++) {
		dr[i].coutData_Record();
		cout << "\n";
	}
}

void Data_Block::fileOut(ofstream& os)
{
	header.fileOut(os);
	os << endl;
	for (int i = 0; i < dr.size(); i++) {
		dr[i].fileOut(os);
		os << endl;
	}
}


void Data_Block::pysparkOut(ofstream& osPyspark)
{
	for (int i = 0; i < dr.size(); i++) {
		dr[i].pysparkOut(osPyspark, header.output_ts);
		osPyspark << endl;
	}
}
