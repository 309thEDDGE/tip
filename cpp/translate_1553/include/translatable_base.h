#ifndef TRANSLATABLE_BASE_H
#define TRANSLATABLE_BASE_H

#include <vector>
#include <cstdint>
#include <arrow/api.h>
#include <arrow/io/api.h>
#include "icd_element.h"

class TranslatableBase
{
private:

protected:
	bool is_ride_along_;
	std::vector<uint16_t> raw_data_;
	uint16_t* raw_data_ptr_;
	//arrow::Type::type arrow_type_id_;
	std::shared_ptr<arrow::DataType> arrow_type_;
	uint32_t translated_data_append_count_;

	// Count of raw values appended/placed in raw_data_ vector.
	uint32_t append_count_;

	// Name of the column/field.
	std::string col_name_;


	// True if setMemoryLocation() of ParquetContext has been called
	// for this column. 
	bool have_set_memory_location_;

public:
	// TODO: make this reference to const and change CometTranslationUnit as necessary to
	// accept this change. 
	std::vector<uint16_t>& raw_data_ref_;

	//TranslatableBase(std::string& name, bool is_ride_along, arrow::Type::type& arrow_type);
	
	void append_raw_values(const int32_t* data_arr);
	virtual ~TranslatableBase();
	const std::string& name();
	uint8_t offset();
	const uint32_t& append_count();
	uint32_t effective_append_count();
	TranslatableBase(std::string& col_name, bool is_ride_along,
		std::shared_ptr<arrow::DataType> arrow_type);
	const ICDElement* icd_elem_ptr_;
	TranslatableBase(const ICDElement* icd_elem_ptr, bool is_ride_along,
		std::shared_ptr<arrow::DataType> arrow_type);
	//arrow::Type::type arrow_type();
	std::shared_ptr<arrow::DataType> arrow_type();
	void memory_location_configured();
	bool is_bitlevel();
	void reset_append_count();
	void reset_translated_data_append_count();
};

#endif