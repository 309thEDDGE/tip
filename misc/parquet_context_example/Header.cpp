#include "Header.h"

void Header::parse(string line)
{	
	// parsed according to https://www1.ncdc.noaa.gov/pub/data/igra/data/igra2-data-format.txt

	output_ts = 0;

	if (line.length() < 71)
		cout << "ERROR -> line isn't long enough to parse header!\n";

	string inVal;

	// ID
	inVal = line.substr(0, 12);
	if (inVal[0] != '#')
		cout << "invalid header, missing #" << endl;
	inVal.erase(0, 1);
	strcpy_s(id, inVal.c_str());

	// Year
	year = stoi(line.substr(13, 4));

	// Month
	month = stoi(line.substr(18, 2));

	// Day
	day = stoi(line.substr(21, 2));

	// Hour
	hour = stoi(line.substr(24, 2));

	// reltime
	reltime = stoi(line.substr(27, 4));

	// numLevels
	numLevels = stoi(line.substr(32, 4));

	// P_SRC
	inVal = line.substr(37, 8);
	strcpy_s(p_src, inVal.c_str());

	// NP_SRC
	inVal = line.substr(46, 8);
	strcpy_s(np_src, inVal.c_str());

	// Latitude 
	latitude = stoi(line.substr(55, 7));

	// longitude
	longitude = stoi(line.substr(63, 8));

	time_stamp(&output_ts);

}



void Header::coutHeader()
{
	cout << fixed << "#" << setw(11) << id << " " << setfill('0') << setw(4) << year << " " << setw(2) << month << " " << setw(2) << day << " " << setw(2) << hour << " " << setw(4) << reltime << " " <<
		setfill(' ') << setw(4) << numLevels << " " << setw(8) << p_src << " " << setw(8) << np_src << " " <<
		setw(7) << latitude << " " << setw(8) << longitude;
}


void Header::fileOut(ofstream& os)
{
	os << fixed << "#" << setw(11) << id << " " << setfill('0') << setw(4) << year << " "  << setw(2) << month << " " << setw(2) << day << " " << setw(2) << hour << " " << setw(4) << reltime << " " <<
		setfill(' ') << setw(4) << numLevels << " " << setw(8) << p_src << " " << setw(8) << np_src << " " <<
		setw(7) << latitude << " " << setw(8) << longitude;
}

void Header::time_stamp(uint64_t* output_ts)
{
	tm struct_time = { 0 };
	struct_time.tm_year = year - 1900;
	struct_time.tm_mon = month - 1;
	struct_time.tm_mday = day;
	if (hour != 99)
		struct_time.tm_hour = hour;
	if (reltime != 9999)
	{
		uint8_t temp = uint8_t(floor(float(reltime) / 100.)); // hour
		if (temp != 0)
			struct_time.tm_hour = temp;

		temp = reltime - temp * 100; // minute
		if (temp != 0)
			struct_time.tm_min = temp;
	}
	__time64_t timet = _mkgmtime64(&struct_time);
	if (timet < 0)
		*output_ts = 0;
	else
		*output_ts = uint64_t(timet);
}