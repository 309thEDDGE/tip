#include "Data_Record.h"

void Data_Record::parse(string line)
{
	// parsed according to https://www1.ncdc.noaa.gov/pub/data/igra/data/igra2-data-format.txt

	output_etime = 0;

	if (line.length() < 51)
		cout << "ERROR -> line isn't long enough to parse data record!\n";

	string inVal;

	// LVLTYPE1
	lvltyp1 = stoi(line.substr(0, 1));

	// LVLTYPE2
	lvltyp2 = stoi(line.substr(1, 1));

	// etime
	etime = stoi(line.substr(3, 5));

	// press
	press = stoi(line.substr(9, 6));
	
	// pflag
	pflag = line[15];
	flag_to_bool(pflag, pflag_b);

	// gpg
	gph = stoi(line.substr(16, 5));

	// zflag
	zflag = line[21];
	flag_to_bool(zflag, zflag_b);

	// temp
	temp = stoi(line.substr(22, 5));

	// tflag
	tflag = line[27];
	flag_to_bool(tflag, tflag_b);

	// rh
	rh = stoi(line.substr(28, 5));

	// dpdp
	dpdp = stoi(line.substr(34, 5));

	// wdir
	wdir = stoi(line.substr(40, 5));

	// wspd
	wspd = stoi(line.substr(46, 5));

	etime_to_sec(&output_etime);

}

void Data_Record::coutData_Record()
{
	cout << fixed << setw(1) << lvltyp1 << setw(1) << lvltyp2 << " " << setw(5) << etime << " "
		 << setw(6) << press << setw(1) << pflag << setw(5) << gph << setw(1) << zflag << setw(5) << temp <<
		 setw(1) << tflag << setw(5) << rh << " " << setw(5) << dpdp << " " << setw(5) << wdir << " "
		 << setw(5) << wspd << " ";
}

void Data_Record::fileOut(ofstream& os)
{
	os << fixed << setw(1) << lvltyp1 << setw(1) << lvltyp2 << " " << setw(5) << etime << " "
		<< setw(6) << press << setw(1) << pflag << setw(5) << gph << setw(1) << zflag << setw(5) << temp <<
		setw(1) << tflag << setw(5) << rh << " " << setw(5) << dpdp << " " << setw(5) << wdir << " "
		<< setw(5) << wspd << " ";
}


void Data_Record::pysparkOut(ofstream& osPyspark, uint64_t& headerTime)
{
	osPyspark << fixed << headerTime << "\t" << lvltyp1 << "\t" << lvltyp2 << "\t" << output_etime << "\t"
	 << press << "\t";
	
	if (pflag == ' ')
		osPyspark  << '*';
	else
		osPyspark << pflag;
	
	osPyspark << "\t" << gph << "\t";
	
	if (zflag == ' ')
		osPyspark << '*';
	else
		osPyspark << zflag;
	
	osPyspark << "\t" << temp << "\t";
	
	if (tflag == ' ')
		osPyspark << '*';
	else
		osPyspark << tflag;
	
	osPyspark  << "\t" << rh << "\t" << dpdp << "\t" << wdir << "\t" << wspd;
}

void Data_Record::etime_to_sec(int32_t* output)
{
	if (etime == -8888)
	{
		// valid data, value remove by IGRA QA
		*output = -1;
		return;
	}
	else if (etime == -9999)
	{
		// value originally missing
		*output = -2;
		return;
	}

	int32_t temp = floor(float(etime) / 100.); // 
	*output = (etime - temp * 100) + temp * 60;
}

void Data_Record::flag_to_bool(char flag, bool& output)
{
	if (flag == ' ')
		output = NULL;
	else if (flag == 'A')
	{
		//printf("found A\n");
		output = true;
	}
	else if (flag == 'B')
	{
		//printf("found B\n");
		output = false;
	}
	else
	{
		output = NULL;
		printf("flag_to_bool(): none of the above\n");
	}
}