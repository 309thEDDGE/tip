#include "parquet_noaa_igra.h"

IGRAData::IGRAData() : ParquetContext(), normal_dist_count_(10)
{
	//tm tt = { 0 };
	//tt.tm_year = 71;
	//tt.tm_mon = 3;
	//printf("before print\n");
	//__time64_t t64 = _mkgmtime64(&tt);
	////printf("assigned t64 = %llzd\n", t64);
	//uint64_t t = t64;
	//printf("after assign t\n");
	//printf("gm time is %lld\n", t);
}

void IGRAData::create_schema()
{
	//printf("creating schema!\n");

	// Fields -- this vector is required in all specializations.
	//std::vector<std::shared_ptr<arrow::Field>> fields;

	// Time field, relative to epoch (1970) [sec]
	std::shared_ptr<arrow::Field> time_field = arrow::field("time", arrow::int64());

	// Add metadata to time field in custom create_schema().
	/*std::vector<std::string> time_md_keys = { "unit", "note" };
	std::vector<std::string> time_md_vals = { "sec", "seconds elapsed since epoch, not the standard milliseconds" };
	arrow::KeyValueMetadata time_md(time_md_keys, time_md_vals);*/
	/*std::unordered_map<std::string, std::string> time_md_map;
	time_md_map["unit"] = "sec";
	time_md_map["note"] = "seconds elapsed since epoch, not the standard milliseconds";*/
	////std::shared_ptr<arrow::KeyValueMetadata> time_md_ptr = time_md.Copy();
	//arrow::KeyValueMetadata time_md(time_md_map);
	//time_field = time_field->AddMetadata(time_md.Copy());
	fields.push_back(time_field);

	// LVLTYP1
	std::shared_ptr<arrow::Field> lvltyp1_field = arrow::field("lvltyp1", arrow::int16());
	fields.push_back(lvltyp1_field);

	// LVLTYP2
	std::shared_ptr<arrow::Field> lvltyp2_field = arrow::field("lvltyp2", arrow::int16());
	fields.push_back(lvltyp2_field);

	// etime, elapsed time since launch [sec]
	// Add metadata at field creation time.
	/*std::vector<std::string> etime_md_keys = { "unit" };
	std::vector<std::string> etime_md_vals = { "sec" };
	arrow::KeyValueMetadata etime_md(etime_md_keys, etime_md_vals);
	bool is_nullable = false;*/
	std::shared_ptr<arrow::Field> etime_field = arrow::field("etime", arrow::int32());
	//std::shared_ptr<arrow::Field> etime_field = arrow::field("etime", arrow::int32(), is_nullable, etime_md.Copy());
	fields.push_back(etime_field);

	// press, pressure [Pa]
	std::shared_ptr<arrow::Field> press_field = arrow::field("press", arrow::int32());
	fields.push_back(press_field);

	// pflag, pressure flag [char]

	//std::shared_ptr<arrow::Field> pflag_field = arrow::field("pflag", arrow::int8());
	std::shared_ptr<arrow::Field> pflag_field = arrow::field("pflag", arrow::boolean());
	fields.push_back(pflag_field);

	// geopotential height, mean sea level [m]
	std::shared_ptr<arrow::Field> gph_field = arrow::field("gph", arrow::int32());
	fields.push_back(gph_field);

	// zflag, gph processing field [char]
	//std::shared_ptr<arrow::Field> zflag_field = arrow::field("zflag", arrow::int8());
	std::shared_ptr<arrow::Field> zflag_field = arrow::field("zflag", arrow::boolean());
	fields.push_back(zflag_field);

	// temp, temperature to tenths, multiplied by 10 e.g., 11 = 1.1 C [C]
	std::shared_ptr<arrow::Field> temp_field = arrow::field("temp", arrow::int16());
	fields.push_back(temp_field);

	// tflag, temperature processing flag [char]
	//std::shared_ptr<arrow::Field> tflag_field = arrow::field("tflag", arrow::int8());
	std::shared_ptr<arrow::Field> tflag_field = arrow::field("tflag", arrow::boolean());
	fields.push_back(tflag_field);

	// rh, relative humidity to tenths, multiplied by 10, e.g., 11 =1.1% rh [%]
	std::shared_ptr<arrow::Field> rh_field = arrow::field("rh", arrow::int16());
	fields.push_back(rh_field);

	// dpdp, dewpoint depression to tenths, multiplied by 10 [C]
	std::shared_ptr<arrow::Field> dpdp_field = arrow::field("dpdp", arrow::int16());
	fields.push_back(dpdp_field);

	// wdir, reported wind direction from north (90 = east) [deg]
	std::shared_ptr<arrow::Field> wdir_field = arrow::field("wdir", arrow::int16());
	fields.push_back(wdir_field);

	// wspd, wind speed to tenths, multiplied by 10 [m/s]
	std::shared_ptr<arrow::Field> wspd_field = arrow::field("wspd", arrow::int16());
	fields.push_back(wspd_field);

	// Normal distribution field (example for creating a row entry that is a list values of same data type.
	std::shared_ptr<arrow::Field> norm_list_field = arrow::field("norm", arrow::list(arrow::int16()));
	fields.push_back(norm_list_field);
	//list_fields_dtype_map_["norm"] = arrow::int16();
	//list_fields_count_per_row_map_["norm"] = normal_dist_count_;

	// Make the Schema.
	//schema_ = arrow::schema(fields);

	/*if (schema_->GetFieldByName("etime")->HasMetadata())
		printf("etime has md\n");
	else
		printf("etime DOES NOT HAVE md\n");*/

	/*time_field = schema_->field(0)->AddMetadata(time_md.Copy());
	arrow::Status st = schema_->SetField(0, time_field, &schema_);*/

	// Note:
	// Can add more fields to schema later. Schema::AddField. For parquet archives which are generated
	// on the fly, the custom schema could add only a time column and the other fields can be added
	// later as comet is mined for relevant columns. 
}


void IGRAData::write_columns(Columnizer& c, std::string& station_name)
{
	// Record station name as metadata. Lookup coordinates of station
	// and write as metadata.
	std::unordered_map<std::string, std::string> metadata;
	metadata["station_name"] = station_name;
	//metadata["test"] = "ha hahaha, is this working?";
	set_metadata(metadata);

	/*std::unordered_map<std::string, std::string> time_md_map;
	time_md_map["unit"] = "sec";
	time_md_map["note"] = "seconds elapsed since epoch, not the standard milliseconds";
	std::string field_name = "time";
	set_field_metadata(time_md_map, field_name);*/

	/*if (schema_->GetFieldByName("time")->HasMetadata())
		printf("time has md\n");
	else
		printf("time DOES NOT HAVE md\n");*/

	// Change the default row group count.
	ROW_GROUP_COUNT_ = 10000;

	/* normal distribution */
	std::default_random_engine generator;
	std::normal_distribution<float> distribution(500.0, 10.0);
	std::vector<int16_t> norm_vec(ROW_GROUP_COUNT_ * normal_dist_count_);
	/* normal distribution */

	int64_t n_rows = int64_t(c.time.size());
	printf("Total rows in file: %lld\n", n_rows);
	int64_t start_at = 0;
	int64_t current_count = 0;
	//uint8_t* nullval = nullptr;
	while(start_at < n_rows)
	{
		if (start_at + ROW_GROUP_COUNT_ < n_rows + 1)
			current_count = ROW_GROUP_COUNT_;
		else
			current_count = n_rows - start_at;

		//printf("IGRAData: appending row group with size %lld\n", current_count);

		uint8_t retval = 0;/*
		retval = append_to_column <int64_t, arrow::Int64Type>(c.time.data() + start_at, 
			current_count, "time", nullptr);
		if (retval != 0) break;
		retval = append_to_column <int16_t, arrow::Int16Type>(c.lvltyp1.data() + start_at, 
			current_count, "lvltyp1", nullptr);
		retval = append_to_column <int16_t, arrow::Int16Type>(c.lvltyp2.data() + start_at, 
			current_count, "lvltyp2", nullptr);
		retval = append_to_column <int32_t, arrow::Int32Type>(c.etime.data() + start_at, 
			current_count, "etime", nullptr);
		retval = append_to_column <int32_t, arrow::Int32Type>(c.press.data() + start_at, 
			current_count, "press", nullptr);
		retval = append_to_bool_column(c.pflag.data() + start_at, 
			current_count, "pflag", c.pflag_null.data() + start_at);
		retval = append_to_column <int32_t, arrow::Int32Type>(c.gph.data() + start_at, 
			current_count, "gph", nullptr);
		retval = append_to_bool_column(c.zflag.data() + start_at, 
			current_count, "zflag", c.zflag_null.data() + start_at);
		retval = append_to_column <int16_t, arrow::Int16Type>(c.temp.data() + start_at, 
			current_count, "temp", nullptr);
		retval = append_to_bool_column(c.tflag.data() + start_at, 
			current_count, "tflag", c.tflag_null.data() + start_at);
		retval = append_to_column <int16_t, arrow::Int16Type>(c.rh.data() + start_at, 
			current_count, "rh", nullptr);
		retval = append_to_column <int16_t, arrow::Int16Type>(c.dpdp.data() + start_at, 
			current_count, "dpdp", nullptr);
		retval = append_to_column <int16_t, arrow::Int16Type>(c.wdir.data() + start_at, 
			current_count, "wdir", nullptr);
		retval = append_to_column <int16_t, arrow::Int16Type>(c.wspd.data() + start_at, 
			current_count, "wspd", nullptr);
		if (retval != 0) break;

		
		for (int i = 0; i < current_count * normal_dist_count_; i++)
			norm_vec[i] = int16_t(distribution(generator));
		//retval = append_lists_to_column<int16_t, arrow::Int16Type>(norm_vec, "norm");
		retval = append_lists_to_column<int16_t, arrow::Int16Type>(norm_vec.data(), 
			current_count, "norm");

		start_at += ROW_GROUP_COUNT_;*/

	}

}

//void IGRAData::append(arrow::ArrayVector& arr_vec)
//{
//	if (!have_created_table_)
//		create_builders();
//
//
//}