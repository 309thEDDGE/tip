#include <vector>
#include <iostream>
#include <cstdint>
#include <map>
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/writer.h>
#include <parquet/arrow/schema.h>
//#include <arrow/>

int main(int argc, char* argv[])
{
	/*********************************
			Create Parquet File
	**********************************/
	arrow::Status st;
	arrow::MemoryPool* pool = arrow::default_memory_pool();

	// Create Schema and fields with metadata
	std::vector<std::shared_ptr<arrow::Field>> fields;

	std::unordered_map<std::string, std::string> a_keyval;
	a_keyval["unit"] = "sec";
	a_keyval["note"] = "not the standard millisecond unit";
	arrow::KeyValueMetadata a_md(a_keyval);
	std::shared_ptr<arrow::Field> a_field = arrow::field("a", arrow::int16(), false, a_md.Copy());
	fields.push_back(a_field);

	std::unordered_map<std::string, std::string> b_keyval;
	b_keyval["unit"] = "ft";
	arrow::KeyValueMetadata b_md(b_keyval);
	std::shared_ptr<arrow::Field> b_field = arrow::field("b", arrow::int16(), false, b_md.Copy());
	fields.push_back(b_field);

	std::shared_ptr<arrow::Schema> schema = arrow::schema(fields);

	// Add metadata to schema.
	std::unordered_map<std::string, std::string> schema_keyval;
	schema_keyval["classification"] = "Type 0";
	arrow::KeyValueMetadata schema_md(schema_keyval);
	schema = schema->AddMetadata(schema_md.Copy());

	// Build arrays of data and add to Table.
	const int64_t rowgroup_size = 100;
	std::vector<int16_t> a_data(rowgroup_size, 0);
	std::vector<int16_t> b_data(rowgroup_size, 0);

	for (int16_t i = 0; i < rowgroup_size; i++)
	{
		a_data[i] = i;
		b_data[i] = rowgroup_size - i;
	}
	
	arrow::Int16Builder a_bldr(pool);
	arrow::Int16Builder b_bldr(pool);
	st = a_bldr.Resize(rowgroup_size);
	if (!st.ok()) return 1;

	st = b_bldr.Resize(rowgroup_size);
	if (!st.ok()) return 1;

	st = a_bldr.AppendValues(a_data);
	if (!st.ok()) return 1;

	st = b_bldr.AppendValues(b_data);
	if (!st.ok()) return 1;

	std::shared_ptr<arrow::Array> a_arr_ptr;
	std::shared_ptr<arrow::Array> b_arr_ptr;

	arrow::ArrayVector arr_vec;
	st = a_bldr.Finish(&a_arr_ptr);
	if (!st.ok()) return 1;
	arr_vec.push_back(a_arr_ptr);
	st = b_bldr.Finish(&b_arr_ptr);
	if (!st.ok()) return 1;
	arr_vec.push_back(b_arr_ptr);

	std::shared_ptr<arrow::Table> table = arrow::Table::Make(schema, arr_vec);

	// Test metadata
	printf("\nMetadata from original schema:\n");
	printf("%s\n", schema->metadata()->ToString().c_str());
	printf("%s\n", schema->field(0)->metadata()->ToString().c_str());
	printf("%s\n", schema->field(1)->metadata()->ToString().c_str());

	std::shared_ptr<arrow::Schema> table_schema = table->schema();
	printf("\nMetadata from schema retrieved from table (should be the same):\n");
	printf("%s\n", table_schema->metadata()->ToString().c_str());
	printf("%s\n", table_schema->field(0)->metadata()->ToString().c_str());
	printf("%s\n", table_schema->field(1)->metadata()->ToString().c_str());

	// Open file and write table.
	std::string file_name = "test.parquet";
	std::shared_ptr<arrow::io::FileOutputStream> ostream;
	st = arrow::io::FileOutputStream::Open(file_name, &ostream);
	if (!st.ok()) return 1;

	std::unique_ptr<parquet::arrow::FileWriter> writer;
	std::shared_ptr<parquet::WriterProperties> props = parquet::default_writer_properties();
	st = parquet::arrow::FileWriter::Open(*schema, pool, ostream, props, &writer);
	if (!st.ok()) return 1;
	st = writer->WriteTable(*table, rowgroup_size);
	if (!st.ok()) return 1;

	// Close file and stream.
	st = writer->Close();
	if (!st.ok()) return 1;
	st = ostream->Close();
	if (!st.ok()) return 1;

	/*********************************
			Read Parquet File
	**********************************/

	

	// Create new memory pool. Not sure if this is necessary.
	//arrow::MemoryPool* pool2 = arrow::default_memory_pool();

	// Open file reader.
	std::shared_ptr<arrow::io::ReadableFile> input_file;
	st = arrow::io::ReadableFile::Open(file_name, pool, &input_file);
	if (!st.ok()) return 1;
	std::unique_ptr<parquet::arrow::FileReader> reader;
	st = parquet::arrow::OpenFile(input_file, pool, &reader);
	if (!st.ok()) return 1;

	// Get schema and read metadata.
	std::shared_ptr<arrow::Schema> new_schema;
	st = reader->GetSchema(&new_schema);
	if (!st.ok()) return 1;
	printf("\nMetadata from schema read from file:\n");
	printf("%s\n", new_schema->metadata()->ToString().c_str());

	// Crashes because there are no metadata.
	/*printf("%s\n", new_schema->field(0)->metadata()->ToString().c_str());
	printf("%s\n", new_schema->field(1)->metadata()->ToString().c_str());*/

	
	printf("field name %s metadata exists: %d\n", new_schema->field(0)->name().c_str(),
		new_schema->field(0)->HasMetadata());
	printf("field name %s metadata exists: %d\n", new_schema->field(1)->name().c_str(),
		new_schema->field(1)->HasMetadata());

	// What if I read the whole table and get the schema from it.
	std::shared_ptr<arrow::Table> new_table;
	st = reader->ReadTable(&new_table);
	if (!st.ok()) return 1;
	std::shared_ptr<arrow::Schema> schema_from_table = new_table->schema();
	printf("\nMetadata from schema that is retrieved through table that is read from file:\n");
	printf("%s\n", schema_from_table->metadata()->ToString().c_str());

	// Crashes because there are no metadata.
	/*printf("%s\n", schema_from_table->field(0)->metadata()->ToString().c_str());
	printf("%s\n", schema_from_table->field(1)->metadata()->ToString().c_str());*/
	
	printf("field name %s metadata exists: %d\n", schema_from_table->field(0)->name().c_str(),
		schema_from_table->field(0)->HasMetadata());
	printf("field name %s metadata exists: %d\n", schema_from_table->field(1)->name().c_str(),
		schema_from_table->field(1)->HasMetadata());
	st = input_file->Close();
	if (!st.ok()) return 1;

	return 0;
}