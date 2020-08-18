#pragma once
#include<vector>
#include"Data_Block.h"

using namespace std;

class Columnizer
{
	private:
		void size(int numRecords);

	public:
		Columnizer() {};

		vector<int64_t> time;
		vector<uint16_t> lvltyp1;
		vector<uint16_t> lvltyp2;
		vector<uint32_t> etime;
		vector<uint32_t> press;
		vector<uint8_t> pflag;
		vector<uint8_t> pflag_null;
		vector<uint32_t> gph;
		vector<uint8_t> zflag;
		vector<uint8_t> zflag_null;
		vector<uint16_t> temp;
		vector<uint8_t> tflag;
		vector<uint8_t> tflag_null;
		vector<uint16_t> rh;
		vector<uint16_t> dpdp;
		vector<uint16_t> wdir;
		vector<uint16_t> wspd;

		void setColumns(int numRecords, vector<Data_Block>& db);
};

