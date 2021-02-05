#pragma once

#ifndef ICD_TRANSLATE_H
#define ICD_TRANSLATE_H

#include <cstdint>
#include <cstdio>
#include <cmath>
#include "icd_element.h"

class ICDTranslate
{
private:
	// Stolen from:
	// https://stackoverflow.com/questions/1055452/c-get-name-of-type-in-template
	template<typename T>
	struct TypeTranslateTraits
	{
		static const char* name;
	};

	bool should_resize_output_vector_;
	static const uint64_t wide_one_ = 1;

	// Count of EU values at output of translation routine.
	size_t n_translated_values_;

	// Count of input words per output translated EU.
	uint8_t elem_word_count_;

	// Count of bits by which word is downshifted prior to scaling,
	// for first and second/final input words.
	uint16_t downshift1_;
	uint16_t downshift2_;

	// Upshift to apply to input word.
	//uint16_t upshift_;

	// Mask to apply to input word for unsigned and signed, respectively, 
	// for first and second/final input words.
	uint16_t mask1_;
	uint16_t mask2_;
	uint16_t mask1_twos_;
	uint16_t mask2_twos_;
	uint16_t sign_bit_mask_;

	// Scale to apply to raw translated value for unsigned and signed, respectively.
	double scale_;
	double scale_twos_;



public:
	ICDTranslate();

	template<typename OutType>
	bool TranslateArrayOfElement(const std::vector<uint16_t>& input_words, std::vector<OutType>& output_eu,
		const ICDElement& icd_elem);

	template<typename OutType>
	void ConfigureBitLevel(size_t input_word_count, std::vector<OutType>& output_eu, const ICDElement& icd_elem);

	template<typename OutType>
	bool ConfigureWordLevel(size_t input_word_count, std::vector<OutType>& output_eu, const ICDElement& icd_elem);

	template<typename OutType>
	void TranslateUnsignedBits(const std::vector<uint16_t>& input_words,
		std::vector<OutType>& output_eu);
	template<typename OutType>
	void TranslateSignedBits(const std::vector<uint16_t>& input_words,
		std::vector<OutType>& output_eu);
	template<typename OutType>
	void TranslateUnsigned16(const std::vector<uint16_t>& input_words,
		std::vector<OutType>& output_eu);
	template<typename OutType>
	void TranslateSigned16(const std::vector<uint16_t>& input_words,
		std::vector<OutType>& output_eu);
	template<typename OutType>
	void TranslateUnsigned32(const std::vector<uint16_t>& input_words,
		std::vector<OutType>& output_eu);
	template<typename OutType>
	void TranslateSigned32(const std::vector<uint16_t>& input_words,
		std::vector<OutType>& output_eu);
	template<typename OutType>
	void TranslateFloat32GPS(const std::vector<uint16_t>& input_words,
		std::vector<OutType>& output_eu);
	template<typename OutType>
	void TranslateFloat64GPS(const std::vector<uint16_t>& input_words,
		std::vector<OutType>& output_eu);
	template<typename OutType>
	void TranslateCAPS(const std::vector<uint16_t>& input_words,
		std::vector<OutType>& output_eu);
	template<typename OutType>
	void TranslateFloat16(const std::vector<uint16_t>& input_words,
		std::vector<OutType>& output_eu);
	template<typename OutType>
	void TranslateFloat64IEEE(const std::vector<uint16_t>& input_words,
		std::vector<OutType>& output_eu);
	template<typename OutType>
	void TranslateFloat321750(const std::vector<uint16_t>& input_words,
		std::vector<OutType>& output_eu);
	template<typename OutType>
	void TranslateFloat32IEEE(const std::vector<uint16_t>& input_words,
		std::vector<OutType>& output_eu);

	template<typename OutType>
	bool CheckType(const std::vector<std::string>& type_strings, const std::string& elem_name);

	// Testing
	void GetConfigParams(size_t& n_vals, uint16_t& dshift1, uint16_t& dshift2,
		uint16_t& mask1, uint16_t& mask2, uint16_t& mask1_twos, uint16_t& mask2_twos, double& scale,
		double& scale_twos, uint16_t& sign_bit_mask);

};

template<typename OutType>
bool ICDTranslate::TranslateArrayOfElement(const std::vector<uint16_t>& input_words,
	std::vector<OutType>& output_eu, const ICDElement& icd_elem)
{
	// Check if ICDElement fields are not defaults:
	// msg_name_, elem_name_, elem_word_count_, schema_.
	if (icd_elem.msg_name_ == "" || icd_elem.elem_name_ == ""
		|| icd_elem.elem_word_count_ == UINT8_MAX || icd_elem.schema_ == ICDElementSchema::BAD)
	{
#ifdef DEBUG
#if DEBUG > 0
		printf("ICDTranslate::TranslateArrayOfElement(): Required ICDElement fields must not be default values!\n");
#endif
#endif
		return false;
	}

	// Call the correct translation function.
	if (icd_elem.is_bitlevel_)
	{
		switch (icd_elem.schema_)
		{
		case ICDElementSchema::ASCII:
			//if (strcmp(TypeTranslateTraits<OutType>::name, "int8_t") != 0) return false;
			if (!CheckType<OutType>(std::vector<std::string>(
				{ "int8_t" }), icd_elem.elem_name_)) return false;
			ConfigureBitLevel(input_words.size(), output_eu, icd_elem);
			TranslateUnsignedBits(input_words, output_eu);
			break;
		case ICDElementSchema::SIGNEDBITS:
			if (!CheckType<OutType>(std::vector<std::string>(
				{ "float", "double" }), icd_elem.elem_name_)) return false;
			/*if (!(strcmp(TypeTranslateTraits<OutType>::name, "float") == 0  
				|| strcmp(TypeTranslateTraits<OutType>::name, "double") == 0))
				return false;*/
			ConfigureBitLevel(input_words.size(), output_eu, icd_elem);
			TranslateSignedBits(input_words, output_eu);
			break;
		case ICDElementSchema::UNSIGNEDBITS:
			if (!CheckType<OutType>(std::vector<std::string>(
				{ "uint8_t", "float", "double" }), icd_elem.elem_name_)) return false;
			/*if (!(strcmp(TypeTranslateTraits<OutType>::name, "float") == 0
				|| strcmp(TypeTranslateTraits<OutType>::name, "double") == 0
				|| strcmp(TypeTranslateTraits<OutType>::name, "uint8_t") == 0))
				return false;*/
			ConfigureBitLevel(input_words.size(), output_eu, icd_elem);
			TranslateUnsignedBits(input_words, output_eu);
			break;
		default:
			return false;
		}
	}
	else
	{
		switch (icd_elem.schema_)
		{
		case ICDElementSchema::MODE_CODE:
			// In most common case, there is nothing to translate. 
			// There is a special case in which the mode code has a single data payload--not implemented.
			return false;
		case ICDElementSchema::SIGNED16:
			if (!CheckType<OutType>(std::vector<std::string>(
				{ "float" }), icd_elem.elem_name_)) return false;
			/*if (strcmp(TypeTranslateTraits<OutType>::name, "float") != 0) return false;*/
			if(!ConfigureWordLevel(input_words.size(), output_eu, icd_elem)) return false;
			TranslateSigned16(input_words, output_eu);
			//TranslateUnsigned16(input_words, output_eu);
			break;
		case ICDElementSchema::SIGNED32:
			if (!CheckType<OutType>(std::vector<std::string>(
				{ "double" }), icd_elem.elem_name_)) return false;
			//if (strcmp(TypeTranslateTraits<OutType>::name, "double") != 0) return false;
			if (!ConfigureWordLevel(input_words.size(), output_eu, icd_elem)) return false;
			TranslateSigned32(input_words, output_eu);
			break;
		case ICDElementSchema::UNSIGNED16:
			if (!CheckType<OutType>(std::vector<std::string>(
				{ "float" }), icd_elem.elem_name_)) return false;
			//if (strcmp(TypeTranslateTraits<OutType>::name, "float") != 0) return false;
			if (!ConfigureWordLevel(input_words.size(), output_eu, icd_elem)) return false;
			TranslateUnsigned16(input_words, output_eu);
			break;
		case ICDElementSchema::UNSIGNED32:
			if (!CheckType<OutType>(std::vector<std::string>(
				{ "double" }), icd_elem.elem_name_)) return false;
			//if (strcmp(TypeTranslateTraits<OutType>::name, "double") != 0) return false;
			if (!ConfigureWordLevel(input_words.size(), output_eu, icd_elem)) return false;
			TranslateUnsigned32(input_words, output_eu);
			break;
		case ICDElementSchema::FLOAT32_1750:
			if (!CheckType<OutType>(std::vector<std::string>(
				{ "float" }), icd_elem.elem_name_)) return false;
			//if (strcmp(TypeTranslateTraits<OutType>::name, "float") != 0) return false;
			if (!ConfigureWordLevel(input_words.size(), output_eu, icd_elem)) return false;
			TranslateFloat321750(input_words, output_eu);
			break;
		case ICDElementSchema::FLOAT32_IEEE:
			if (!CheckType<OutType>(std::vector<std::string>(
				{ "float" }), icd_elem.elem_name_)) return false;
			//if (strcmp(TypeTranslateTraits<OutType>::name, "float") != 0) return false;
			if (!ConfigureWordLevel(input_words.size(), output_eu, icd_elem)) return false;
			TranslateFloat32IEEE(input_words, output_eu);
			break;
		case ICDElementSchema::FLOAT64_IEEE:
			if (!CheckType<OutType>(std::vector<std::string>(
				{ "double" }), icd_elem.elem_name_)) return false;
			//if (strcmp(TypeTranslateTraits<OutType>::name, "double") != 0) return false;
			if(!ConfigureWordLevel(input_words.size(), output_eu, icd_elem)) return false;
			TranslateFloat64IEEE(input_words, output_eu);
			break;
		case ICDElementSchema::FLOAT16:
			if (!CheckType<OutType>(std::vector<std::string>(
				{ "float" }), icd_elem.elem_name_)) return false;
			//if (strcmp(TypeTranslateTraits<OutType>::name, "float") != 0) return false;
			if (!ConfigureWordLevel(input_words.size(), output_eu, icd_elem)) return false;
			TranslateFloat16(input_words, output_eu);
			break;
		case ICDElementSchema::CAPS:
			if (!CheckType<OutType>(std::vector<std::string>(
				{ "double" }), icd_elem.elem_name_)) return false;
			//if (strcmp(TypeTranslateTraits<OutType>::name, "double") != 0) return false;
			if (!ConfigureWordLevel(input_words.size(), output_eu, icd_elem)) return false;
			TranslateCAPS(input_words, output_eu);
			break;
		case ICDElementSchema::FLOAT32_GPS:
			if (!CheckType<OutType>(std::vector<std::string>(
				{ "float" }), icd_elem.elem_name_)) return false;
			//if (strcmp(TypeTranslateTraits<OutType>::name, "float") != 0) return false;
			if (!ConfigureWordLevel(input_words.size(), output_eu, icd_elem)) return false;
			TranslateFloat32GPS(input_words, output_eu);
			break;
		case ICDElementSchema::FLOAT64_GPS:
			if (!CheckType<OutType>(std::vector<std::string>(
				{ "double" }), icd_elem.elem_name_)) return false;
			//if (strcmp(TypeTranslateTraits<OutType>::name, "double") != 0) return false;
			if (!ConfigureWordLevel(input_words.size(), output_eu, icd_elem)) return false;
			TranslateFloat64GPS(input_words, output_eu);
			break;
		default:
			return false;
		}
	}

	return true;
}

template<typename OutType>
void ICDTranslate::ConfigureBitLevel(size_t input_word_count, std::vector<OutType>& output_eu, const ICDElement& icd_elem)
{
	elem_word_count_ = icd_elem.elem_word_count_;
	n_translated_values_ = size_t(floor(float(input_word_count) / elem_word_count_));

	// Resize the output array to contain the translated values.
	if (should_resize_output_vector_ && output_eu.size() != n_translated_values_)
		output_eu.resize(n_translated_values_);

	downshift1_ = 16 - icd_elem.bitlsb_;
	mask1_ = (1 << (icd_elem.bitlsb_ - icd_elem.bitmsb_ + 1)) - 1;
	mask1_twos_ = (1 << (icd_elem.bitlsb_ - icd_elem.bitmsb_)) - 1;
	scale_ = icd_elem.msb_val_ / double(ICDTranslate::wide_one_ << (icd_elem.bitlsb_ - icd_elem.bitmsb_));
	scale_twos_ = icd_elem.msb_val_ / double(ICDTranslate::wide_one_ << (icd_elem.bitlsb_ - icd_elem.bitmsb_ - 1));
	sign_bit_mask_ = 1 << (16 - icd_elem.bitmsb_);

	// Handle case with more than one word.
	if (icd_elem.elem_word_count_ > 1)
	{
		downshift1_ = 0;
		mask1_ = (1 << (16 - icd_elem.bitmsb_ + 1)) - 1;
		mask1_twos_ = (1 << (16 - icd_elem.bitmsb_)) - 1;
		downshift2_ = 16 - icd_elem.bitlsb_;
		mask2_ = (1 << (icd_elem.bitlsb_ - 1 + 1)) - 1;
		mask2_twos_ = (1 << icd_elem.bitlsb_ ) - 1;
		uint8_t bits_1st_word = 16 - icd_elem.bitmsb_ + 1;
		uint8_t bits_last_word = icd_elem.bitlsb_;
		uint8_t bits_other_words = 16 * (icd_elem.elem_word_count_ - 2);
		scale_ = icd_elem.msb_val_ / double(ICDTranslate::wide_one_ <<
			(bits_1st_word + bits_last_word + bits_other_words - 1));
		scale_twos_ = icd_elem.msb_val_ / double(ICDTranslate::wide_one_ <<
			(bits_1st_word + bits_last_word + bits_other_words - 2));
	}

	if (scale_ == 0.0)
		scale_ = 1.0;
	if (scale_twos_ == 0.0)
		scale_twos_ = 1.0;
}

template<typename OutType>
bool ICDTranslate::ConfigureWordLevel(size_t input_word_count, std::vector<OutType>& output_eu, 
	const ICDElement& icd_elem)
{
	elem_word_count_ = icd_elem.elem_word_count_;
	n_translated_values_ = size_t(floor(float(input_word_count) / elem_word_count_));

	// Resize the output array to contain the translated values.
	if (should_resize_output_vector_ && output_eu.size() != n_translated_values_)
		output_eu.resize(n_translated_values_);

	bool count_is_correct = true;
	switch (icd_elem.schema_)
	{
	case ICDElementSchema::UNSIGNED16:
		if (elem_word_count_ != 1)
			count_is_correct = false;
		scale_ = icd_elem.msb_val_ / (wide_one_ << 15);
		break;
	case ICDElementSchema::UNSIGNED32:
		if (elem_word_count_ != 2)
			count_is_correct = false;
		scale_ = icd_elem.msb_val_ / (wide_one_ << 31);
		break;
	case ICDElementSchema::SIGNED16:
		if (elem_word_count_ != 1)
			count_is_correct = false;
		scale_twos_ = icd_elem.msb_val_ / (wide_one_ << 14);
		break;
	case ICDElementSchema::SIGNED32:
		if (elem_word_count_ != 2)
			count_is_correct = false;
		scale_twos_ = icd_elem.msb_val_ / (wide_one_ << 30);
		break;
	case ICDElementSchema::ASCII:
		if (elem_word_count_ != 1)
			count_is_correct = false;
		scale_ = 1.0;
		break;
	case ICDElementSchema::FLOAT32_1750:
		if (elem_word_count_ != 2)
			count_is_correct = false;
		scale_ = 1.0;
		break;
	case ICDElementSchema::FLOAT32_IEEE:
		if (elem_word_count_ != 2)
			count_is_correct = false;
		scale_ = 1.0;
		break;
	case ICDElementSchema::FLOAT64_IEEE:
		if (elem_word_count_ != 4)
			count_is_correct = false;
		scale_ = 1.0;
		break;
	case ICDElementSchema::FLOAT16:
		if (elem_word_count_ != 1)
			count_is_correct = false;
		scale_ = 1.0;
		break;
	case ICDElementSchema::CAPS:
		if (elem_word_count_ != 3)
			count_is_correct = false;
		scale_ = 1.0;
		break;
	case ICDElementSchema::FLOAT32_GPS:
		if (elem_word_count_ != 2)
			count_is_correct = false;
		scale_ = 1.0;
		break;
	case ICDElementSchema::FLOAT64_GPS:
		if (elem_word_count_ != 4)
			count_is_correct = false;
		scale_ = 1.0;
		break;
	default:
		return false;
	}

	if (!count_is_correct)
	{
#ifdef DEBUG
#if DEBUG > 0
		printf("ICDTranslate::ConfigureWordLevel(): elem %s incorrect word count %hhu!\n",
			icd_elem.elem_name_.c_str(), elem_word_count_);
#endif
#endif
		return false;
	}

	// Scale factor of 0 does not give meaningful data. Handle
	// case when MSB val implies scale factor of zero.
	if (scale_ == 0.0)
		scale_ = 1.0;
	if (scale_twos_ == 0.0)
		scale_twos_ = 1.0;

	return true;
}

template<typename OutType>
void ICDTranslate::TranslateUnsignedBits(const std::vector<uint16_t>& input_words,
	std::vector<OutType>& output_eu)
{
	if (elem_word_count_ > 1)
	{
		uint64_t part1 = 0;
		uint64_t part2 = 0;
		uint64_t middle_part = 0;
		int part1_upshift = 0; 
		if (elem_word_count_ == 2)
		{
			part1_upshift = 16 - downshift2_;
			for (size_t i = 0; i < n_translated_values_; i++)
			{
				part1 = (input_words[elem_word_count_ * i] >> downshift1_) & mask1_;
				part2 = (input_words[elem_word_count_ * i + 1] >> downshift2_) & mask2_;
				output_eu[i] = double((part1 << part1_upshift) + part2) * scale_;
			}
		}
		else
		{
			part1_upshift = (elem_word_count_ - 2) * 16 + (16 - downshift2_);
			size_t midi = 0;
			int part2_ind_adjust = elem_word_count_ - 1;
			for (size_t i = 0; i < n_translated_values_; i++)
			{
				part1 = (input_words[elem_word_count_ * i] >> downshift1_) & mask1_;
				part2 = (input_words[elem_word_count_ * i + part2_ind_adjust] >> downshift2_) & mask2_;
				middle_part = 0;
				for (midi = 1; midi < elem_word_count_-1; midi++)
				{
					middle_part += (input_words[elem_word_count_ * i + midi] << 
						((elem_word_count_ - midi - 1) * 16));
				}
				output_eu[i] = double((part1 << part1_upshift) + part2 + middle_part) * scale_;
			}
		}
	}
	else 
	{
		for (size_t i = 0; i < n_translated_values_; i++)
		{
			output_eu[i] = double((input_words[elem_word_count_ * i] >> downshift1_) & mask1_) * scale_;
		}
	}
}

template<typename OutType>
void ICDTranslate::TranslateSignedBits(const std::vector<uint16_t>& input_words,
	std::vector<OutType>& output_eu)
{
	uint64_t part1 = 0;
	if (elem_word_count_ > 1)
	{
		uint64_t part2 = 0;
		uint64_t middle_part = 0;
		int part1_upshift = 0;
		if (elem_word_count_ == 2)
		{
			part1_upshift = 16 - downshift2_;
			for (size_t i = 0; i < n_translated_values_; i++)
			{
				
				if (input_words[elem_word_count_ * i] & sign_bit_mask_)
				{
					part1 = ~((input_words[elem_word_count_ * i] >> downshift1_) & mask1_twos_);
					part2 = ~((input_words[elem_word_count_ * i + 1] >> downshift2_) & mask2_twos_) + 1;
					output_eu[i] = double(((part1 & mask1_twos_) << part1_upshift) + (part2 & mask2_twos_)) * -scale_twos_;
				}
				else
				{
					part1 = (input_words[elem_word_count_ * i] >> downshift1_) & mask1_twos_;
					part2 = (input_words[elem_word_count_ * i + 1] >> downshift2_) & mask2_twos_;
					output_eu[i] = double((part1 << part1_upshift) + part2) * scale_twos_;
				}
			}
		}
		else
		{
			part1_upshift = (elem_word_count_ - 2) * 16 + (16 - downshift2_);
			size_t midi = 0;
			int part2_ind_adjust = elem_word_count_ - 1;
			for (size_t i = 0; i < n_translated_values_; i++)
			{
				if (input_words[elem_word_count_ * i] & sign_bit_mask_)
				{
					part1 = ~((input_words[elem_word_count_ * i] >> downshift1_) & mask1_twos_);
					part2 = ~((input_words[elem_word_count_ * i + part2_ind_adjust] >> downshift2_) & mask2_twos_) + 1;
					middle_part = 0;
					for (midi = 1; midi < elem_word_count_ - 1; midi++)
					{
						middle_part += (~(input_words[elem_word_count_ * i + midi]) <<
							((elem_word_count_ - midi - 1) * 16));
					}
					output_eu[i] = double(((part1 & mask1_twos_) << part1_upshift) +
						(part2 & mask2_twos_) + middle_part) * -scale_twos_;
				}
				else
				{
					part1 = (input_words[elem_word_count_ * i] >> downshift1_) & mask1_twos_;
					part2 = (input_words[elem_word_count_ * i + part2_ind_adjust] >> downshift2_) & mask2_twos_;
					middle_part = 0;
					for (midi = 1; midi < elem_word_count_ - 1; midi++)
					{
						middle_part += (input_words[elem_word_count_ * i + midi] <<
							((elem_word_count_ - midi - 1) * 16));
					}
					output_eu[i] = double((part1 << part1_upshift) + part2 + middle_part) * scale_twos_;
				}
			}
		}
	}
	else
	{
		for (size_t i = 0; i < n_translated_values_; i++)
		{
			if (input_words[elem_word_count_ * i] & sign_bit_mask_)
			{
				part1 = ~((input_words[elem_word_count_ * i] >> downshift1_) & mask1_twos_) + 1;
				output_eu[i] = double(part1 & mask1_twos_) * -scale_twos_;
			}
			else
				output_eu[i] = double((input_words[elem_word_count_ * i] >> downshift1_) & mask1_twos_) * scale_twos_;
		}
	}
}

template<typename OutType>
void ICDTranslate::TranslateUnsigned16(const std::vector<uint16_t>& input_words,
	std::vector<OutType>& output_eu)
{
	for (size_t i = 0; i < n_translated_values_; i++)
	{
		output_eu[i] = float(input_words[i]) * scale_;
	}
}

template<typename OutType>
void ICDTranslate::TranslateSigned16(const std::vector<uint16_t>& input_words,
	std::vector<OutType>& output_eu)
{
	const int16_t* vdata = (int16_t*)input_words.data();
	for (size_t i = 0; i < n_translated_values_; i++)
	{
		output_eu[i] = float(vdata[i]) * scale_twos_;
	}
}

template<typename OutType>
void ICDTranslate::TranslateUnsigned32(const std::vector<uint16_t>& input_words,
	std::vector<OutType>& output_eu)
{
	uint32_t temp = 0;
	for (size_t i = 0; i < n_translated_values_; i++)
	{
		temp = input_words[2 * i];
		output_eu[i] = double((temp << 16) + input_words[2 * i + 1]) * scale_;
	}
}

template<typename OutType>
void ICDTranslate::TranslateSigned32(const std::vector<uint16_t>& input_words,
	std::vector<OutType>& output_eu)
{
	int32_t data_val = 0;
	uint16_t* ui16ptr = (uint16_t*)&data_val;
	for (size_t i = 0; i < n_translated_values_; i++)
	{
		ui16ptr[0] = input_words[2 * i + 1];
		ui16ptr[1] = input_words[2 * i];
		output_eu[i] = double(data_val) * scale_twos_;
	}
}

template<typename OutType>
void ICDTranslate::TranslateFloat32GPS(const std::vector<uint16_t>& input_words,
	std::vector<OutType>& output_eu)
{
	float fsign = 1.;
	float expon = 0.;
	float mantissa = 0.;
	float denom = wide_one_ << 24;

	for (size_t i = 0; i < n_translated_values_; i++)
	{
		// Handle identical zero.
		if (input_words[2 * i] == 0 && input_words[2 * i + 1] == 0)
			output_eu[i] = 0.;
		// Handle case for values in [0, 0.5),
		else if (input_words[2 * i] < (1 << 7))
		{
			output_eu[i] = ((input_words[2 * i] & ((wide_one_ << 7) - wide_one_)) << 16) / denom +
				input_words[2 * i + 1] / denom;
		}
		// Sign bit is high, all other exponent bits are zero ==> undefined.
		else if ((input_words[2 * i] >> 7) == (1 << 8))
		{
			output_eu[i] = NAN;
		}
		else
		{
			fsign = pow(-1.0, input_words[2 * i] >> 15);
			expon = pow(2.0, double((input_words[2 * i] >> 7) & ((wide_one_ << 8) - wide_one_)) - 128);
			mantissa = 0.5 + ((input_words[2 * i] & ((wide_one_ << 7) - wide_one_)) << 16) / denom +
				input_words[2 * i + 1] / denom;
			output_eu[i] = fsign * expon * mantissa;
		}
	}
}

template<typename OutType>
void ICDTranslate::TranslateFloat64GPS(const std::vector<uint16_t>& input_words,
	std::vector<OutType>& output_eu)
{
	double dsign = 1.;
	double expon = 0.;
	double mantissa = 0.;
	double denom = wide_one_ << 56;

	// No scale for GPS float.
	for (size_t i = 0; i < n_translated_values_; i++)
	{
		// Handle identical zero.
		if (input_words[4 * i] == 0 && input_words[4 * i + 1] == 0 && input_words[4 * i + 2] == 0
			&& input_words[4 * i + 3] == 0)
		{
			output_eu[i] = 0.;
		}
		// Handle case for values in [0, 0.5).
		else if (input_words[4 * i] < (1 << 7))
		{
			output_eu[i] = (uint64_t(input_words[4 * i] & ((wide_one_ << 7) - wide_one_)) << 48) / denom +
				(uint64_t(input_words[4 * i + 1]) << 32) / denom +
				(uint64_t(input_words[4 * i + 2]) << 16) / denom +
				input_words[4 * i + 3] / denom;
		}
		// Sign bit is high, all exponent bits are zero ==> undefined.
		else if ((input_words[4 * i] >> 7) == (1 << 8))
		{
			output_eu[i] = NAN;
		}
		else
		{
			dsign = pow(double(-1.0), input_words[4 * i] >> 15);
			expon = pow(double(2.0), double((input_words[4 * i] >> 7) & ((wide_one_ << 8) - wide_one_)) - 128);
			mantissa = 0.5 + (uint64_t(input_words[4 * i] & ((wide_one_ << 7) - wide_one_)) << 48) / denom +
				(uint64_t(input_words[4 * i + 1]) << 32) / denom +
				(uint64_t(input_words[4 * i + 2]) << 16) / denom +
				input_words[4 * i + 3] / denom;
			output_eu[i] = dsign * expon * mantissa;
		}
	}
}

template<typename OutType>
void ICDTranslate::TranslateCAPS(const std::vector<uint16_t>& input_words,
	std::vector<OutType>& output_eu)
{
	double dsign = 1.;
	double expon = 0.;
	double mantissa = 0.;
	double denom = wide_one_ << 40;

	// No scale for CAPS.
	for (size_t i = 0; i < n_translated_values_; i++)
	{
		// Following "05.01...IS-GPS-059 Revision D...pdf" (bit 0 = MSB in word)

		// Handle identical zero.
		if (input_words[3 * i] == 0 && input_words[3 * i + 1] == 0 && input_words[3 * i + 2] == 0)
			output_eu[i] = 0.;
		else if ((input_words[3 * i] & ((1 << 8) - 1)) == 0)
		{
			// Sign bit is high, all exponent bits are zero ==> undefined.
			if ((input_words[3 * i + 2] >> 15) == 1)
			{
				output_eu[i] = NAN;
			}
			// Handle case for values in [0, 0.5).
			else
			{
				output_eu[i] = ((input_words[3 * i + 2] & ((wide_one_ << 15) - wide_one_)) << 24) / denom +
					(uint64_t(input_words[3 * i + 1]) << 8) / denom +
					(input_words[3 * i] >> 8) / denom;
			}
		}
		else
		{
			dsign = pow(-1.0, input_words[3 * i + 2] >> 15);
			expon = pow(2.0, double(input_words[3 * i] & ((wide_one_ << 8) - wide_one_)) - 128);
			mantissa = 0.5 + ((input_words[3 * i + 2] & ((wide_one_ << 15) - wide_one_)) << 24) / denom +
				(uint64_t(input_words[3 * i + 1]) << 8) / denom +
				(input_words[3 * i] >> 8) / denom;
			output_eu[i] = dsign * expon * mantissa;
		}
	}
}

template<typename OutType>
void ICDTranslate::TranslateFloat16(const std::vector<uint16_t>& input_words,
	std::vector<OutType>& output_eu)
{
	double dsign = 1.;
	double expon = 0.;
	double mantissa = 0.;
	double neg_one = -1.0;
	double sixteen = 16.0;

	for (size_t i = 0; i < n_translated_values_; i++)
	{
		if (input_words[i] == 0)
			output_eu[i] = 0.0;
		else
		{
			dsign = pow(neg_one, input_words[i] >> 15);
			expon = pow(sixteen, (input_words[i] & ((wide_one_ << 4) - wide_one_)));
			mantissa = (input_words[i] >> 4 & ((wide_one_ << 11) - wide_one_));
			output_eu[i] = dsign * expon * mantissa;
		}
	}
}

template<typename OutType>
void ICDTranslate::TranslateFloat64IEEE(const std::vector<uint16_t>& input_words,
	std::vector<OutType>& output_eu)
{
	/*
	Note about NaNs: During comparison of linux- and windows-generated 
	translated pq files from the same commit it was discovered that the
	NaNs are not of the same variety and thus are represented by different
	bits in the pq file. It appears that linux is better at encoding the type
	of NaN, neg or pos, etc. 

	The pq comparator code uses std::equal on arrays of data casted
	to uint8_t as a general solution for comparing columns of different 
	data types. In the case of different NaNs, which are by definition not
	comparable and therefore shouldn't be compared, the comparison fails 
	because the underlying bits of the NaN
	representations are different. We can solve this in two ways:

	1) catch the presence of NaN in the for loop below, set the value to some portable and
	generic version of NaN such that the bits are the same and the comparator 
	does not fail. Note again that NaNs can't be compared, so the presence of 
	NaNs in a truth and test data set in the same row ought not to be compared
	as unequal in the context of checking if the contents of two pq files are 
	dissimilar, in which case we wish NaNs to compare as equal and not indicate
	that the two data sets are different. This option was not chosen because
	we want to avoid another logical clause in the for loop to maximize execution
	efficiency.

	2) Do not rely on the generic comparison of uint8_t/bytes in the
	pq comparator code and specialize the function for double type. In the 
	specialization, loop over the columns of data being compared and skip
	comparison if std::isnan is true for the data in both columns. We have
	currently chosen this option and implemented the change in comparator.cpp.

	The same consideration may come up for float type. It is not implemented 
	for float currently. 
	*/
	double data_val = 0.;
	uint16_t* ui16ptr = (uint16_t*)&data_val;
	// No scale for double.
	for (size_t i = 0; i < n_translated_values_; i++)
	{
		if (input_words[4 * i] == 0 && input_words[4 * i + 1] == 0 && input_words[4 * i + 2] == 0
			&& input_words[4 * i + 3] == 0)
			output_eu[i] = 0.;
		else
		{
			ui16ptr[0] = input_words[4 * i + 3];
			ui16ptr[1] = input_words[4 * i + 2];
			ui16ptr[2] = input_words[4 * i + 1];
			ui16ptr[3] = input_words[4 * i];
			output_eu[i] = data_val;
		}
	}
}

template<typename OutType>
void ICDTranslate::TranslateFloat321750(const std::vector<uint16_t>& input_words,
	std::vector<OutType>& output_eu)
{
	int dsign = 0;
	double expon = 0.;
	int exp_sign = 0;
	double mantissa = 0.;
	double denom = double(wide_one_ << 23);
	double two = 2.0;

	for (size_t i = 0; i < n_translated_values_; i++)
	{
		if (input_words[2 * i] == 0 && input_words[2 * i + 1] == 0)
			output_eu[i] = 0.;
		else
		{
			dsign = input_words[2 * i] >> 15;
			expon = (input_words[2 * i + 1] & ((wide_one_ << 7) - wide_one_));
			exp_sign = (input_words[2 * i + 1] >> 7) & ((wide_one_ << 1) - wide_one_);
			mantissa = (uint64_t(input_words[2 * i] & ((wide_one_ << 15) - wide_one_)) << 8) / denom
				+ (uint64_t(input_words[2 * i + 1]) >> 8) / denom;

			// 2s complement mantissa
			if (dsign == 1)
				mantissa = mantissa - 1.0;
			// 2s complement exponent
			if (exp_sign == 1)
				expon = expon - int(wide_one_ << 7);

			output_eu[i] = pow(two, expon) * mantissa;
		}
	}
}

template<typename OutType>
void ICDTranslate::TranslateFloat32IEEE(const std::vector<uint16_t>& input_words,
	std::vector<OutType>& output_eu)
{
	float data_val = 0.;
	uint16_t* ui16ptr = (uint16_t*)&data_val;

	// No scale for float.
	for (size_t i = 0; i < n_translated_values_; i++)
	{
		ui16ptr[0] = input_words[2 * i + 1];
		ui16ptr[1] = input_words[2 * i];
		output_eu[i] = data_val;
	}
}
template<typename OutType>
bool ICDTranslate::CheckType(const std::vector<std::string>& type_strings, const std::string& elem_name)
{
	std::string type_names;
	for (size_t i = 0; i < type_strings.size(); i++)
	{
		if (type_strings[i].compare(TypeTranslateTraits<OutType>::name) == 0)
			return true;
		if (i < type_strings.size() - 1)
			type_names += type_strings[i] + ", ";
		else
			type_names += type_strings[i];
	}
#ifdef DEBUG
#if DEBUG > 0
	printf("ICDTranslate::CheckType(): Elem %s failed with types %s\n", elem_name.c_str(),
		type_names.c_str());
#endif
#endif
	return false;
}


#endif