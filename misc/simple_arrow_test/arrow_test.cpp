#include "arrow_test.h"

ArrowTest::~ArrowTest()
{
	printf("(destructor)\n");
	if (have_created_writer)
		writer->Close();
}

ArrowTest::ArrowTest(std::string& out_path) : TEMP_ELEMENT_COUNT(1000), have_created_table(false),
	path(out_path), have_created_writer(false), pool(nullptr), schema(nullptr)
{
	// build vector of widgets.
	/*Widget w1;
	w1.time = 135633662246;
	w1.id = 200;
	w1.cost = 32.30;
	std::vector<double> comp_cost = { 0.32, 5.20, 5.23, 6.18 };
	w1.cost_components = comp_cost;

	Widget w2;
	w2.time = 135883662246;
	w2.id = 201;
	w2.cost = 33.80;
	w2.cost_components = comp_cost;

	widget_vec.push_back(w1);
	widget_vec.push_back(w2);*/
}

// This is the function that will be made pure virtual in the base class.
void ArrowTest::create_schema()
{
	// Fields
	std::vector<std::shared_ptr<arrow::Field>> fields;
	std::shared_ptr<arrow::Field> time_field = arrow::field("time", arrow::uint64());
	fields.push_back(time_field);

	std::shared_ptr<arrow::Field> id_field = arrow::field("ID", arrow::int32());
	fields.push_back(id_field);

	std::shared_ptr<arrow::Field> cost_comp_field = arrow::field("cost", arrow::list(arrow::uint16()));
	//std::shared_ptr<arrow::Field> cost_comp_field = arrow::field("cost", );
	fields.push_back(cost_comp_field);

	// Make the Schema.
	schema = arrow::schema(fields);

	// Can add more fields to schema later. Schema::AddField. For parquet archives which are generated
	// on the fly, the custom schema could add only a time column and the other fields can be added
	// later as comet is mined for relevant columns. 
}

arrow::Status ArrowTest::build_table(int64_t& n, uint64_t* time, int32_t* id, uint16_t* cost, int64_t& cost_n)
{
	create_schema();
	std::string cost_name = schema->GetFieldByName("cost")->type()->name();
	std::string cost_str = schema->GetFieldByName("cost")->type()->ToString();
	printf("cost name/str = %s / %s\n", cost_name.c_str(), cost_str.c_str());

	// Arrays
	printf("before vector of arrays\n");
	arrow::ArrayVector arr_vec;
	
	// Memory pool.
	pool = arrow::default_memory_pool();
	printf("after mem pool\n");

	// Create time field and vector. (add metadata later.)
	builders.push_back(std::make_unique<arrow::UInt64Builder>(pool));
	std::unique_ptr<arrow::UInt64Builder> time_bldr = std::dynamic_pointer_cast<arrow::UInt64Builder>(builders[0]);
	time_bldr->Resize(n);
	time_bldr->AppendValues(time, n);
	/*arrow::UInt64Builder time_bldr;
	time_bldr.Resize(n);
	time_bldr.AppendValues(time, n);*/
	std::shared_ptr<arrow::Array> time_array_ptr;
	ARROW_RETURN_NOT_OK(time_bldr->Finish(&time_array_ptr));
	//arrays.push_back(time_array_ptr);
	arr_vec.push_back(time_array_ptr);
	printf("after time builder\n");

	// ID field and vector.
	builders.push_back(std::make_unique<arrow::Int32Builder>(pool));
	std::shared_ptr<arrow::Int32Builder> id_bldr = std::dynamic_pointer_cast<arrow::Int32Builder>(builders[1]);
	id_bldr->Resize(n);
	id_bldr->AppendValues(id, n);
	/*arrow::Int32Builder id_bldr;
	id_bldr.Resize(n);
	id_bldr.AppendValues(id, n);*/
	std::shared_ptr<arrow::Array> id_array_ptr;
	ARROW_RETURN_NOT_OK(id_bldr->Finish(&id_array_ptr));
	//arrays.push_back(id_array_ptr);
	arr_vec.push_back(id_array_ptr);
	printf("after id builder\n");

	// Cost components
	builders.push_back(std::make_unique<arrow::ListBuilder>(pool, std::make_shared<arrow::UInt16Builder>(pool)));
	std::shared_ptr<arrow::ListBuilder> lbldr = std::dynamic_pointer_cast<arrow::ListBuilder>(builders[2]);
	printf("list builder type: %s\n", lbldr->type()->name().c_str()); 
	printf("list builder value builder type: %s\n", lbldr->value_builder()->type()->name().c_str());
	arrow::UInt16Builder* cost_bldr = static_cast<arrow::UInt16Builder*>(lbldr->value_builder());
	lbldr->Resize(n);
	const int32_t offsets[] = { 0, 3, 6, 9, 12 };
	lbldr->AppendValues(offsets, n);
	cost_bldr->AppendValues(cost, cost_n * n);
	std::shared_ptr<arrow::Array> cost_array_ptr;
	ARROW_RETURN_NOT_OK(lbldr->Finish(&cost_array_ptr));
	arr_vec.push_back(cost_array_ptr);
	
	// Make the Table
	std::shared_ptr<arrow::Table> table = arrow::Table::Make(schema, arr_vec);
	printf("after table\n");

	// Output stream and FileWriter.
	std::shared_ptr<arrow::io::FileOutputStream> ostream;
	ARROW_RETURN_NOT_OK(arrow::io::FileOutputStream::Open(path, &ostream));
	printf("after open out stream\n");
	//std::shared_ptr<arrow::io::OutputStream> ostream_base = (std::shared_ptr<arrow::io::OutputStream>)ostream;
	std::shared_ptr<parquet::WriterProperties> props = parquet::default_writer_properties();


	//std::unique_ptr<parquet::arrow::FileWriter> writer;
	ARROW_RETURN_NOT_OK(parquet::arrow::FileWriter::Open(*schema, pool, ostream, props, &writer));
	have_created_writer = true;

	//// Write table.
	ARROW_RETURN_NOT_OK(writer->WriteTable(*table, n));

	return arrow::Status::OK();
}

void ArrowTest::append(int64_t& n, uint64_t* time, int32_t* id, uint16_t* cost, int64_t& cost_n)
{
	if (!have_created_table)
	{
		st = build_table(n, time, id, cost, cost_n);
		if (!st.ok())
		{
			printf("build_table() error (ID %s): %s", st.CodeAsString().c_str(), st.message().c_str());
		}
		have_created_table = true;
	}
	else
	{
		// Create new row group.
		writer->NewRowGroup(n);

		// Write Cols iteratively. Builders are reset when finished, so only need to resize.
		std::shared_ptr<arrow::UInt64Builder> time_bldr = std::dynamic_pointer_cast<arrow::UInt64Builder>(builders[0]);
		time_bldr->Resize(n);
		time_bldr->AppendValues(time, n);
		/*arrow::UInt64Builder time_bldr;
		time_bldr.Resize(n);
		time_bldr.AppendValues(time, n);*/
		std::shared_ptr<arrow::Array> time_array_ptr;
		st = time_bldr->Finish(&time_array_ptr);
		if (!st.ok())
		{
			printf("append() error (ID %s): %s", st.CodeAsString().c_str(), st.message().c_str());
		}
		writer->WriteColumnChunk(*time_array_ptr);

		std::shared_ptr<arrow::Int32Builder> id_bldr = std::dynamic_pointer_cast<arrow::Int32Builder>(builders[1]);
		id_bldr->Resize(n);
		id_bldr->AppendValues(id, n);
		/*arrow::UInt32Builder id_bldr;
		id_bldr.Resize(n);
		id_bldr.AppendValues(id, n);*/
		std::shared_ptr<arrow::Array> id_array_ptr;
		st = id_bldr->Finish(&id_array_ptr);
		if (!st.ok())
		{
			printf("append() error (ID %s): %s", st.CodeAsString().c_str(), st.message().c_str());
		}
		writer->WriteColumnChunk(*id_array_ptr);

		std::shared_ptr<arrow::ListBuilder> lbldr = std::dynamic_pointer_cast<arrow::ListBuilder>(builders[2]);
		arrow::UInt16Builder* cost_bldr = static_cast<arrow::UInt16Builder*>(lbldr->value_builder());
		lbldr->Resize(n);
		const int32_t offsets[] = { 0, 3, 6, 9, 12 };;
		lbldr->AppendValues(offsets, n);
		cost_bldr->AppendValues(cost, cost_n * n);
		std::shared_ptr<arrow::Array> cost_array_ptr;
		st = lbldr->Finish(&cost_array_ptr);
		if (!st.ok())
		{
			printf("append() error (cost components %c): %s", st.code(), st.message().c_str());
		}
		writer->WriteColumnChunk(*cost_array_ptr);
	}
}