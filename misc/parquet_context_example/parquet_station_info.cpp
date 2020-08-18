#include "parquet_station_info.h"

void Parquet_Station_Info::create_schema()
{
	/*
	// Fields -- this vector is required in all specializations.
	std::vector<std::shared_ptr<arrow::Field>> fields;

	// ID field
	//std::shared_ptr<arrow::Field> ID_field = arrow::field("ID", arrow::utf8());
	//fields.push_back(ID_field);

	// Latitude
	std::shared_ptr<arrow::Field> latitude_field = arrow::field("Latitude", arrow::float64());
	fields.push_back(latitude_field);

	// Longitude
	std::shared_ptr<arrow::Field> longitude_field = arrow::field("Longitude", arrow::float64());
	fields.push_back(longitude_field);

	// Elevation
	std::shared_ptr<arrow::Field> elevation_field = arrow::field("Elevation", arrow::float32());
	fields.push_back(elevation_field);

	// State
	//std::shared_ptr<arrow::Field> state_field = arrow::field("State", arrow::utf8());
	//fields.push_back(state_field);

	// Name
	//std::shared_ptr<arrow::Field> name_field = arrow::field("Name", arrow::utf8());
	//fields.push_back(name_field);

	// firstYear
	std::shared_ptr<arrow::Field> firstYear_field = arrow::field("firstYear", arrow::int16());
	fields.push_back(firstYear_field);

	// lastYear
	std::shared_ptr<arrow::Field> lastYear_field = arrow::field("lastYear", arrow::int16());
	fields.push_back(lastYear_field);

	// NOS
	std::shared_ptr<arrow::Field> NOS_field = arrow::field("NOS", arrow::int32());
	fields.push_back(NOS_field);
	*/


	/*
		Add needed column fields with the name of the column
		Options for arrow::Type
		  NA, BOOL, UINT8, INT8,
		  UINT16, INT16, UINT32, INT32,
		  UINT64, INT64, HALF_FLOAT, FLOAT,
		  DOUBLE, STRING, BINARY, FIXED_SIZE_BINARY,
		  DATE32, DATE64, TIMESTAMP, TIME32,
		  TIME64, INTERVAL, DECIMAL, LIST,
		  STRUCT, UNION, DICTIONARY, MAP
		  https://arrow.apache.org/docs/cpp/structarrow_1_1_type.html
	*/
	addField(arrow::float64(), "Latitude");
	addField(arrow::float64(), "Longitude");
	addField(arrow::float32(), "Elevation");
	addField(arrow::int16(), "firstYear");
	addField(arrow::int16(), "lastYear");
	addField(arrow::int32(), "NOS");
	// Make the Schema. (needed for all specilizations)
	
}

void Parquet_Station_Info::write_columns(Station_Columnizer& c)
{
	// Potentially pass in a vector<void*>, and a row group count 
	std::vector<void*> fieldData;
	fieldData.push_back(c.latitude.data());
	fieldData.push_back(c.longitude.data());
	fieldData.push_back(c.elevation.data());
	fieldData.push_back(c.firstYear.data());
	fieldData.push_back(c.lastYear.data());
	fieldData.push_back(c.NOS.data());


	// Change the default row group count.
	ROW_GROUP_COUNT_ = 10000;


	
	int64_t n_rows = int64_t(c.id.size());  ///This would need changed to the vector size


	printf("Total rows in file: %lld\n", n_rows);
	int64_t start_at = 0;
	int64_t current_count = 0;

	while (start_at < n_rows)
	{
		if (start_at + ROW_GROUP_COUNT_ < n_rows + 1)
			current_count = ROW_GROUP_COUNT_;
		else
			current_count = n_rows - start_at;

		//printf("IGRAData: appending row group with size %lld\n", current_count);

		
		
		
		/*
		uint8_t retval = 0;
		//retval = append_to_column <string, arrow::StringType>(c.id.data() + start_at,
			//current_count, "id", nullptr);
		if (retval != 0) break;
		retval = append_to_column <double, arrow::DoubleType>(c.latitude.data() + start_at,
			current_count, "Latitude", nullptr);
		retval = append_to_column <double, arrow::DoubleType>(c.longitude.data() + start_at,
			current_count, "Longitude", nullptr);
		retval = append_to_column <float, arrow::FloatType>(c.elevation.data() + start_at,
			current_count, "Elevation", nullptr);
		//retval = append_to_column <string, arrow::StringType>(c.state.data() + start_at,
			//current_count, "state", nullptr);
		//retval = append_to_column <string, arrow::StringType>(c.name.data() + start_at,
			//current_count, "name", nullptr);
		retval = append_to_column <int16_t, arrow::Int16Type>(c.firstYear.data() + start_at,
			current_count, "firstYear", nullptr);
		retval = append_to_column <int16_t, arrow::Int16Type>(c.lastYear.data() + start_at,
			current_count, "lastYear", nullptr);
		retval = append_to_column <int32_t, arrow::Int32Type>(c.NOS.data() + start_at,
			current_count, "NOS", nullptr);
		if (retval != 0) break;*/

		//retval = append_to_column <string, arrow::StringType>(c.id.data() + start_at,
			//current_count, "id", nullptr);*/

		
			
			
			
			/*
		append_column(c.latitude.data() + start_at,	current_count, "Latitude");
		append_column(c.longitude.data() + start_at, current_count, "Longitude");
		append_column(c.elevation.data() + start_at, current_count, "Elevation");
		//append_column(c.state.data() + start_at, current_count, "state");
		//append_column(c.name.data() + start_at, current_count, "name");
		append_column(c.firstYear.data() + start_at, current_count, "firstYear");
		append_column(c.lastYear.data() + start_at,	current_count, "lastYear");
		append_column(c.NOS.data() + start_at, current_count, "NOS");*/



		for (int i = 0; i < fieldData.size(); i++)
			//append_column(fieldData[i], current_count, i);

		start_at += ROW_GROUP_COUNT_;
	}
}
