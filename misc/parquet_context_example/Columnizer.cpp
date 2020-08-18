#include "Columnizer.h"

void Columnizer::size(int numRecords)
{
	time.resize(numRecords);
	lvltyp1.resize(numRecords);
	lvltyp2.resize(numRecords);
	etime.resize(numRecords);
	press.resize(numRecords);
	pflag.resize(numRecords, 0);
	pflag_null.resize(numRecords, 0);
	gph.resize(numRecords);
	zflag.resize(numRecords, 0);
	zflag_null.resize(numRecords, 0);
	temp.resize(numRecords);
	tflag.resize(numRecords, 0);
	tflag_null.resize(numRecords, 0);
	rh.resize(numRecords);
	dpdp.resize(numRecords);
	wdir.resize(numRecords);
	wspd.resize(numRecords);
}

void Columnizer::setColumns(int numRecords, vector<Data_Block>& db)
{
	size(numRecords);
	int counter = 0;


	for (int i = 0; i < db.size(); i++) {
		for (int j = 0; j < db[i].dr.size(); j++) {
			time[counter] = db[i].header.output_ts;
			lvltyp1[counter] = db[i].dr[j].lvltyp1;
			lvltyp2[counter] = db[i].dr[j].lvltyp2;
			etime[counter] = db[i].dr[j].output_etime;
			press[counter] = db[i].dr[j].press;
			if (db[i].dr[j].pflag != ' ')
			{
				pflag_null[counter] = 1;
				pflag[counter] = db[i].dr[j].pflag_b;
			}
	
			gph[counter] = db[i].dr[j].gph;
			if (db[i].dr[j].zflag != ' ')
			{
				zflag_null[counter] = 1;
				zflag[counter] = db[i].dr[j].zflag_b;
			}
			
			temp[counter] = db[i].dr[j].temp;
			if (db[i].dr[j].tflag != ' ')
			{
				tflag_null[counter] = 1;
				tflag[counter] = db[i].dr[j].tflag_b;
			}
			
			rh[counter] = db[i].dr[j].rh;
			dpdp[counter] = db[i].dr[j].dpdp;
			wdir[counter] = db[i].dr[j].wdir;
			wspd[counter] = db[i].dr[j].wspd;
			counter++;
		}
	}
	if (counter != numRecords) {
		cout << "Warning -> numRecords != vector size \n";
	}
}


