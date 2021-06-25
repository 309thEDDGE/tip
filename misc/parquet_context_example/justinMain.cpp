/*
	NOAA Data Parser
	Author: Justin Roberts
	Data taken from https://www1.ncdc.noaa.gov/pub/data/igra/data/
*/

#include<iostream>
#include <stdio.h>
#include<fstream>
#include <string>
#include <filesystem>
#include <direct.h>

#include"Parser.h"
#include"Header.h"
#include "Data_Record.h"
#include "parquet_noaa_igra.h"
#include "parquet_station_info.h"

using namespace std;

int main(int argc, char* argv[]) {


	bool parseNOAA = false;
	bool parseMeta = true;


	Parser parser;

	// Parse NOAA data
	if (parseNOAA) {

		string filename;
		bool outputTest_files = true;
		bool writeParquet = true;

		if (argc < 2)
		{
			//printf("Need input argument: path to IGRA data *.txt file\n");
			printf("No command line argument, using default IGRA data *.txt file \n");
			// other test files
			//filename = "C:/Users/A10/Documents/Justins/noaaData/PKM00041780-data.txt"; // all years for station PKM00041780
			//filename = "C:/Users/A10/Documents/Justins/noaaData/USW00013859-data.txt"; // all years for station USW00013859
			//filename = "C:/Users/A10/Documents/Justins/noaaData/ZZV00ASEU05-data.txt"; // all years for station ZZV00ASEU05
			//filename = "C:/Users/A10/Documents/Justins/noaaData/ACM00078861-data.txt"; // all years for station ACM00078861
			//filename = "C:/Users/A10/Documents/Justins/noaaData/AEM00041217-data.txt"; // ytd for station AEM00041217
			//filename = "C:/Users/A10/Documents/Justins/noaaData/ACM00078861-data_header.txt"; // only one data block for station ACM0078861
			//filename = "C:/Users/A10/Documents/Justins/noaaData/ACM00078861-data_header_corner.txt"; // corner case three header
			//filename = "C:\\Users\\A10\\Documents\\Justins\\repos\\fdw_proto\\test/inputfile.txt";
			//filename = "C:/Users/A10/Documents/Justins/noaaData/tester/threeHeader.txt";
			//filename = "C:/Users/A10/Documents/Justins/noaaData/testFiles/ACM00078861-data_header.txt";
			//filename = "C:\\Users\\A10\\Documents\\Justins\\noaaData\\DataSubset/ACM00078861-data.txt";
			//filename = "C:\\Users\\A10\\Documents\\Justins\\noaaData\\DataSubset/AEM00041217-data.txt";
			//filename = "C:\\Users\\A10\\Documents\\Justins\\noaaData\\cmpParquet\\ACM00078861-data_header_corner.txt";
			//filename = "C:\\Users\\A10\\Documents\\test_data\\igra\\AEM00041217-data.txt\\AEM00041217-data_truncated.txt";
			filename = "C:\\Users\\A10\\Documents\\Justins\\noaaData\\BadFiles\\ACM00078861-data_header.txt";

		}
		else {
			filename = argv[1];
			outputTest_files = (bool)atoi(argv[2]);
		}

		ifstream is(filename);

		// check if the file exists
		if (!is.good()) {
			cout << "Error -> invalid filename " << filename << endl;
		}
		else {

			// Use std::filesystem to create the output path
			filesystem::path input_path = filename;

			// output test files for debugging
			if (outputTest_files) {
				filesystem::path testDir = input_path.parent_path() / "testing";
				filesystem::create_directory(testDir);
				filesystem::path testing_output_path_stem = testDir / input_path.stem();
				parser.outputDump(testing_output_path_stem.string());  // dump parsed NOAA files for testing and validation
			}



			// Parse NOAA data file
			printf("\nParsing %s\n", filename.c_str());
			parser.parseFile(is);
			printf("Parsing complete \n");




			// write parquet
			if (writeParquet) {
				// Write out Parquet
				// pass output path to IGRAData::open_for_write() then call write_columns, passing c
				// and the station name
				filesystem::path parquetDir = input_path.parent_path() / "parquet_out";
				filesystem::create_directory(parquetDir);
				filesystem::path output_path_stem = parquetDir / input_path.stem();
				string parquet_output_path = output_path_stem.string() + ".parquet";
				printf("Getting Columns \n");
				Columnizer c = parser.getColumns();

				string hdfs_base_path = "/data/igra";
				string hdfs_output_path_stem = hdfs_base_path + "/" + input_path.stem().string();
				string hdfs_output_path = hdfs_output_path_stem + ".parquet";

				printf("output path: %s\n", parquet_output_path.c_str());
				printf("hdfs output path: %s\n", hdfs_output_path.c_str());





				/*
				Add fields (columns) to be written to parquet
				Parameters:

				first parameter: target data type of the field (if int, or uint, it may be cast from whatever type given in setMemoryLocation to the type specified here)
				int8()
				int16()
				int32()
				int64()
				uint8()
				uint16()
				uint32()
				uint64()
				float32()
				float64()
				boolean()

				second parameter: Field name

				third parameter: if it is a list provide the number values in the list

				*/
				int rowgroupSize = 10000;
				ParquetContext id(rowgroupSize);

				id.addField(arrow::int64(), "time");
				id.addField(arrow::int32(), "lvltyp1");
				id.addField(arrow::int32(), "lvltyp2");
				id.addField(arrow::int64(), "etime");
				id.addField(arrow::int64(), "press");
				id.addField(arrow::boolean(), "pflag");
				id.addField(arrow::int64(), "gph");
				id.addField(arrow::boolean(), "zflag");
				id.addField(arrow::int32(), "temp");
				id.addField(arrow::boolean(), "tflag");
				id.addField(arrow::int32(), "rh");
				id.addField(arrow::int32(), "dpdp");
				id.addField(arrow::int32(), "wdir");
				id.addField(arrow::int32(), "wspd");

				vector<int64_t> time(rowgroupSize);
				vector<int16_t> lvltyp1(rowgroupSize);
				vector<int16_t> lvltyp2(rowgroupSize);
				vector<int32_t> etime(rowgroupSize);
				vector<int32_t> press(rowgroupSize);
				vector<uint8_t> pflag(rowgroupSize);
				vector<uint8_t> pflag_null(rowgroupSize);
				vector<int32_t> gph(rowgroupSize);
				vector<uint8_t> zflag(rowgroupSize);
				vector<uint8_t> zflag_null(rowgroupSize);
				vector<int16_t> temp(rowgroupSize);
				vector<uint8_t> tflag(rowgroupSize);
				vector<uint8_t> tflag_null(rowgroupSize);
				vector<int16_t> rh(rowgroupSize);
				vector<int16_t> dpdp(rowgroupSize);
				vector<int16_t> wdir(rowgroupSize);
				vector<int16_t> wspd(rowgroupSize);


				/*
				After creating memory in the parser, point parquet_context
				to the vector being used to store data for parquet write out (data types for int and uint can be different from the target data type specied in addField, as long as the target data type is larger than the type given here. Strings, Bool, Float, and Double are not set up to be cast)

				Parameters:
				Template datatypes:
				string -> StringType (utf8())
				double -> DoubleType (float64())
				float -> FloatType (float32())
				int8_t -> Int8Type (int8())
				int16_t -> Int16Type (int16())
				int32_t -> Int32Type (int32())
				int64_t -> Int64Type (int64())
				uint8_t -> UInt8Type (uint8())
				uint16_t -> UInt16Type (uint16())
				uint32_t -> UInt32Type (uint32())
				uint64_t -> UInt64Type (uint64())
				float -> FloatType (float32())
				double -> DoubleType (float64())
				uint8_t -> BooleanType (uint8())

				First parameter: A vector to the data being appended in the
				parquet file

				Second parameter: Field name

				Third parameter: optional and can be passed to specify
				null values. It needs to be a pointer to a vector of uint8_t with
				the same size as the data vector. The null vector should have 1 for
				valid values and 0 for null values. Null is not currently available
				for lists (ie: when adding a third argument to addField)
				*/
				id.setMemoryLocation<int64_t>(time, "time");
				id.setMemoryLocation<int16_t>(lvltyp1, "lvltyp1");
				id.setMemoryLocation<int16_t>(lvltyp2, "lvltyp2");
				id.setMemoryLocation<int32_t>(etime, "etime");
				id.setMemoryLocation<int32_t>(press, "press");
				id.setMemoryLocation<uint8_t>(pflag, "pflag",pflag_null.data());
				id.setMemoryLocation<int32_t>(gph, "gph");
				id.setMemoryLocation<uint8_t>(zflag, "zflag",zflag_null.data());
				id.setMemoryLocation<int16_t>(temp, "temp");
				id.setMemoryLocation<uint8_t>(tflag, "tflag",tflag_null.data());
				id.setMemoryLocation<int16_t>(rh, "rh");
				id.setMemoryLocation<int16_t>(dpdp, "dpdp");
				id.setMemoryLocation<int16_t>(wdir, "wdir");
				id.setMemoryLocation<int16_t>(wspd, "wspd");


				// Write locally.
				id.open_for_write(parquet_output_path, true);

				/*
				HDFS write. Currently broken due to clunky Arrow build process.
				some work will be required on build side, and possibly environment
				 vars added before this functions correctly.
				*/

				/*
				string host = "192.168.1.54";
				int port = 9000;
				string user = "isaac";
				id.open_for_hdfs_write(hdfs_output_path, host, port, user);
				*/

				string file_name = input_path.filename().string();
				string station_name = file_name.substr(0, file_name.find("-data.txt"));
				printf("station name: %s\n", station_name.c_str());

				int n_rows = c.wdir.size();
				int start_at = 0;
				while ((n_rows - start_at) >= rowgroupSize)
				{
					for (int i = 0; i < rowgroupSize; i++) {
						time[i] = c.time[i + start_at];
						lvltyp1[i] = c.lvltyp1[i + start_at];
						lvltyp2[i] = c.lvltyp2[i + start_at];
						etime[i] = c.etime[i + start_at];
						press[i] = c.press[i + start_at];
						pflag[i] = c.pflag[i + start_at];
						pflag_null[i] = c.pflag_null[i + start_at];
						gph[i] = c.gph[i + start_at];
						zflag[i] = c.zflag[i + start_at];
						zflag_null[i] = c.zflag_null[i + start_at];
						temp[i] = c.temp[i + start_at];
						tflag[i] = c.tflag[i + start_at];
						tflag_null[i] = c.tflag_null[i + start_at];
						rh[i] = c.rh[i + start_at];
						dpdp[i] = c.dpdp[i + start_at];
						wdir[i] = c.wdir[i + start_at];
						wspd[i] = c.wspd[i + start_at];
					}
					start_at += rowgroupSize;

					id.writeColumns();
				}

				if (start_at < n_rows) {
					int diff = n_rows - start_at;

					for (int i = 0; i < diff; i++) {
						time[i] = c.time[i + start_at];
						lvltyp1[i] = c.lvltyp1[i + start_at];
						lvltyp2[i] = c.lvltyp2[i + start_at];
						etime[i] = c.etime[i + start_at];
						press[i] = c.press[i + start_at];
						pflag[i] = c.pflag[i + start_at];
						pflag_null[i] = c.pflag_null[i + start_at];
						gph[i] = c.gph[i + start_at];
						zflag[i] = c.zflag[i + start_at];
						zflag_null[i] = c.zflag_null[i + start_at];
						temp[i] = c.temp[i + start_at];
						tflag[i] = c.tflag[i + start_at];
						tflag_null[i] = c.tflag_null[i + start_at];
						rh[i] = c.rh[i + start_at];
						dpdp[i] = c.dpdp[i + start_at];
						wdir[i] = c.wdir[i + start_at];
						wspd[i] = c.wspd[i + start_at];
					}
					start_at += rowgroupSize;

					id.writeColumns(diff);
				}
			}
		}

		is.close();
	}

	if (parseMeta) {
		bool dumpMetaFiles = true;
		bool writeMetaParquet = true;
		string metaFile = "C:\\Users\\A10\\Documents\\Justins\\noaaData\\parquet_metadata\\igra2-station-list_adjusted.txt";

		filesystem::path meta_path = metaFile;

		if (dumpMetaFiles) {
			filesystem::path testDir = meta_path.parent_path() / "testing";
			filesystem::create_directory(testDir);
			filesystem::path testing_output_path_stem = testDir / "station_info";
			parser.metaDump(testing_output_path_stem.string());  // dump parsed meta files for testing and validation
		}

		// Parse NOAA data file
		printf("\nParsing Metadata Filename: %s\n", metaFile.c_str());
		ifstream metaStream(metaFile);
		parser.parseMeta(metaStream);
		printf("Parsing complete \n");

		// write parquet
		if (writeMetaParquet) {
			// Write out Parquet
			// pass output path to IGRAData::open_for_write() then call write_columns, passing c
			// and the station name
			filesystem::path parquetDir = meta_path.parent_path() / "parquet_out";
			filesystem::create_directory(parquetDir);
			filesystem::path output_path_stem = parquetDir / meta_path.stem();
			string parquet_output_path = output_path_stem.string() + ".parquet";
			printf("Getting Columns \n");
			Station_Columnizer c = parser.getMetadata_columns();

			printf("output path: %s\n", parquet_output_path.c_str());

			// construct with row group size
			ParquetContext pc(2788);

			/*
				Add fields (columns) to be written to parquet
				Parameters:

				first parameter: target data type of the field (if int, or uint, it may be cast from whatever type given in setMemoryLocation to the type specified here)
				int8()
				int16()
				int32()
				int64()
				uint8()
				uint16()
				uint32()
				uint64()
				float32()
				float64()
				boolean()

				second parameter: Field name

				third parameter: if it is a list provide the number values in the list

			*/
			pc.addField(arrow::utf8(), "ID");
			pc.addField(arrow::float64(), "Latitude");
			pc.addField(arrow::float64(), "Longitude");
			pc.addField(arrow::float32(), "Elevation");
			pc.addField(arrow::utf8(), "State");
			pc.addField(arrow::utf8(), "Name");
			pc.addField(arrow::int16(), "firstYear");
			pc.addField(arrow::int16(), "lastYear");
			pc.addField(arrow::int32(), "NOS");

			pc.addField(arrow::int32(), "INT32LIST", 2);
			pc.addField(arrow::utf8(), "STRINGLIST", 2);
			pc.addField(arrow::boolean(), "BOOL");
			pc.addField(arrow::boolean(), "boolLIST", 2);


			/*
				After creating memory in the parser, point parquet_context
				to the vector being used to store data for parquet write out (data types for int and uint can be different from the target data type specied in addField, as long as the target data type is larger than the type given here. Strings, Bool, Float, and Double are not set up to be cast)

				Parameters:
				Template datatypes:
				string -> StringType (utf8())
				double -> DoubleType (float64())
				float -> FloatType (float32())
				int8_t -> Int8Type (int8())
				int16_t -> Int16Type (int16())
				int32_t -> Int32Type (int32())
				int64_t -> Int64Type (int64())
				uint8_t -> UInt8Type (uint8())
				uint16_t -> UInt16Type (uint16())
				uint32_t -> UInt32Type (uint32())
				uint64_t -> UInt64Type (uint64())
				float -> FloatType (float32())
				double -> DoubleType (float64())
				uint8_t -> BooleanType (uint8())

				First parameter: A vector to the data being appended in the
				parquet file

				Second parameter: Field name

				Third parameter: optional and can be passed to specify
				null values. It needs to be a pointer to a vector of uint8_t with
				the same size as the data vector. The null vector should have 1 for
				valid values and 0 for null values. Null is not currently available
				for lists (ie: when adding a third argument to addField)
			*/
			//pc.setMemoryLocation<int16_t>(c.firstYear, "ID");
			pc.setMemoryLocation<string>(c.id, "ID");  // If passing a vector of string, must pass the vector, not a pointer to the vector
			//pc.setMemoryLocation<float>(c.elevation, "Latitude");
			pc.setMemoryLocation<double>(c.latitude, "Latitude");
			pc.setMemoryLocation<double>(c.longitude, "Longitude");
			pc.setMemoryLocation<float>(c.elevation, "Elevation");
			//pc.setMemoryLocation<int32_t>(c.NOS, "Elevation");
			pc.setMemoryLocation<string>(c.state, "State", c.stateNull.data());
			pc.setMemoryLocation<string>(c.name, "Name");
			pc.setMemoryLocation<int16_t>(c.firstYear, "firstYear");
			pc.setMemoryLocation<int16_t>(c.lastYear, "lastYear");
			//pc.setMemoryLocation<uint16_t>(c.lastYear, "lastYear");
			pc.setMemoryLocation<int32_t>(c.NOS, "NOS", c.NOSnull.data());
			pc.setMemoryLocation<int32_t>(c.int32List, "INT32LIST");
			pc.setMemoryLocation<string>(c.stringList, "STRINGLIST");
			pc.setMemoryLocation<uint8_t>(c.boolData, "BOOL");
			pc.setMemoryLocation<uint8_t>(c.boolList, "boolLIST");

			// Write locally.
			pc.open_for_write(parquet_output_path, true);

			// Write columns pointed to by setMemoryLocation passing in the
			// number of rows in the vectors
			pc.writeColumns();
		}
	}

	return 0;
}
