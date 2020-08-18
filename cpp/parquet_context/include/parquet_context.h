#ifndef PARQUET_CONTEXT_H
#define PARQUET_CONTEXT_H

#include <vector>
#include <cstdint>
#include <stdio.h>
#include <map>
#include <arrow/api.h>
#include <arrow/io/api.h>
//#include <parquet/api/reader.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/writer.h>
#include <parquet/arrow/schema.h>
#include <typeinfo>

#include "Field_Info.h"

/*

!!! NOTICE !!!

The preprocessor macros ARROW_STATIC and PARQUET_STATIC must be defined
if linking to the library statically.

!!! NOTICE !!!

*/
//const int64_t DEFAULT_ROW_GROUP_COUNT = 10000;


class ParquetContext
{
private:
	std::string host_;
	std::string user_;
	std::string path_;
	std::vector<std::string> temp_string_vec_;
	int port_;
	bool truncate_;
	bool readwrite_;
	bool writeonly_;
	arrow::Status st_;
	bool have_created_writer_;
	bool have_created_reader_;
	bool have_created_schema_;
	bool is_hdfs_output_;
	arrow::MemoryPool* pool_;
	//std::vector<std::shared_ptr<arrow::ArrayBuilder>> builders_;
	std::unique_ptr<parquet::arrow::FileReader> reader_;
	std::unique_ptr<parquet::arrow::FileWriter> writer_;
	std::unique_ptr<arrow::ArrayBuilder> get_builder_from_data_type(std::shared_ptr<arrow::DataType> dtype,
		bool is_list_builder);
	void create_reader();
	bool parquetStop;
	void append_column(Field_Info& dataInfo, int rows, bool explicit_row_count, int offset);
	//std::vector<Data_Info> dataInfo;

protected:
	virtual void create_schema();
	int ROW_GROUP_COUNT_;
	uint8_t ret_;
	int64_t append_row_count_;
	bool have_created_table_;
	//std::map<std::string, uint8_t*> NULL_selections_map_;
	//std::map<std::string, std::shared_ptr<arrow::ArrayBuilder>> builders_map_;
	//std::map<std::string, bool> have_appended_col_map_;
	//std::map<std::string, std::shared_ptr<arrow::DataType>> list_fields_dtype_map_;
	//std::map<std::string, int64_t> list_fields_count_per_row_map_;
	std::map<std::string, Field_Info> field_info_map_;
	//std::map<std::string, std::vector<int32_t>> list_fields_offsets_map_;
	std::shared_ptr<arrow::Schema> schema_;
	std::shared_ptr<arrow::io::FileOutputStream> ostream_;
	std::shared_ptr<parquet::WriterProperties> props_;
	std::shared_ptr<arrow::io::ReadableFile> input_file_;
	//uint8_t check_col_name(std::string&);
	//uint8_t check_field_name(std::string& check_name);
	void create_builders();
	std::vector<int32_t> get_offsets_vector(int64_t& n_rows, int64_t& elements_per_row, int offset);
	//void prepare_append();
	void write_cols_if_ready();
	void fill_string_vec(std::vector<std::string>* str_data_vec_ptr, int64_t count, int offset);
	std::vector<std::shared_ptr<arrow::Field>> fields;
	

public:
	ParquetContext();
	ParquetContext(int rgSize);
	~ParquetContext();
	void Close();
	uint8_t open_for_write(std::string& path, bool truncate);
	uint8_t open_for_hdfs_write(std::string& hdfs_path, std::string& host, int& port, std::string& user);
	uint8_t open_for_read(std::string& path, bool readwrite);
	

	/*template<typename NativeType, typename ArrowType>
	uint8_t append_to_column(std::vector<NativeType>& new_data, std::string col_name);

	template<typename NativeType, typename ArrowType>
	uint8_t append_to_column(NativeType* new_data, int64_t size, std::string col_name, uint8_t* valid);

	uint8_t append_to_bool_column(uint8_t* new_data, int64_t size, std::string col_name, uint8_t* valid);

	template<typename NativeType, class ArrowType>
	uint8_t append_lists_to_column(std::vector<NativeType>& new_data, std::string col_name);

	template<typename NativeType, class ArrowType>
	uint8_t append_lists_to_column(NativeType* new_data, int64_t size, std::string col_name);
	//arrow::Status build_table(int64_t& n, uint64_t* time, int32_t* id, uint16_t* cost, int64_t& cost_n);
	//uint8_t append(arrow::ArrayVector& arr_vec);*/

	uint8_t set_metadata(std::unordered_map<std::string, std::string>& keyval_map);
	std::unordered_map<std::string, std::string> get_metadata();
	uint8_t set_field_metadata(std::unordered_map<std::string, std::string>& keyval_map, std::string& field_name);
	
	void print_schema();
	void print_metadata();
	void addField(std::shared_ptr<arrow::DataType> type, std::string fieldName, int listSize=NULL);
	void writeColumns(int rows, int offset=0);
	void writeColumns();
	bool isUnsigned(std::shared_ptr<arrow::DataType> type);
	std::string getTypeID_fromArrowType(std::shared_ptr<arrow::DataType> type, int& byteSize);
	template<typename NativeType> bool setMemoryLocation(std::vector<NativeType>& data, std::string, uint8_t* boolField=nullptr);
	template<typename NativeType> bool setMemoryLocation(std::vector<std::string>& strVec, std::string fieldName, uint8_t* boolField=nullptr);
	template<typename castType> void castTo(void* data, std::vector<castType>& buildVec, std::string castFrom, 
		int size, int offset);
};

#endif