#include "icd_translate.h"

// #define REGISTER_TRANSLATE_TYPE(X) \
//     template <>                    \
//     const char* ICDTranslate::TypeTranslateTraits<X>::name = #X

// REGISTER_TRANSLATE_TYPE(uint8_t);
// REGISTER_TRANSLATE_TYPE(int8_t);
// REGISTER_TRANSLATE_TYPE(uint16_t);
// REGISTER_TRANSLATE_TYPE(int16_t);
// REGISTER_TRANSLATE_TYPE(uint32_t);
// REGISTER_TRANSLATE_TYPE(int32_t);
// REGISTER_TRANSLATE_TYPE(uint64_t);
// REGISTER_TRANSLATE_TYPE(int64_t);
// REGISTER_TRANSLATE_TYPE(float);
// REGISTER_TRANSLATE_TYPE(double);
// REGISTER_TRANSLATE_TYPE(char);

ICDTranslate::ICDTranslate() : n_translated_values_(0), downshift1_(0), downshift2_(0), mask1_(0), mask1_twos_(0), mask2_(0), mask2_twos_(0), scale_(0.0), scale_twos_(0.0), elem_word_count_(0), should_resize_output_vector_(true), sign_bit_mask_(0), bit_count_(0), bitmsb_(0), bitlsb_(0)
{
}

void ICDTranslate::GetConfigParams(size_t& n_vals, uint16_t& dshift1, uint16_t& dshift2,
                                   uint16_t& mask1, uint16_t& mask2, uint16_t& mask1_twos, uint16_t& mask2_twos, double& scale,
                                   double& scale_twos, uint16_t& sign_bit_mask)
{
    n_vals = n_translated_values_;
    dshift1 = downshift1_;
    dshift2 = downshift2_;
    mask1 = mask1_;
    mask2 = mask2_;
    mask1_twos = mask1_twos_;
    mask2_twos = mask2_twos_;
    scale = scale_;
    scale_twos = scale_twos_;
    sign_bit_mask = sign_bit_mask_;
}

void ICDTranslate::ConfigureU32BitElemForU16(ICDElement& icd_elem, 
    const std::vector<uint32_t>& input_words, std::vector<uint16_t>& output_words)

{
    if(icd_elem.bitlsb_ < 17)
    {
        icd_elem.elem_word_count_ = 1;
        KeepMS16(input_words, output_words); 
    }
    else if(icd_elem.bitmsb_ > 16)
    {
        icd_elem.elem_word_count_ = 1;
        icd_elem.bitmsb_ = icd_elem.bitmsb_ - 16;
        icd_elem.bitlsb_ = icd_elem.bitlsb_ - 16;
        KeepLS16(input_words, output_words);
    }
    else
    {
        icd_elem.elem_word_count_ = 2;
        icd_elem.bitlsb_ = icd_elem.bitlsb_ - 16;
        Swap16(input_words, output_words);
    }
}

void ICDTranslate::Swap16(const std::vector<uint32_t>& input_words, 
    std::vector<uint16_t>& output_words)
{
    size_t index_mult = sizeof(uint32_t) / 2;
    if(output_words.size() != (input_words.size() * index_mult))
        output_words.resize(input_words.size() * index_mult);

    const uint16_t* ui16ptr = reinterpret_cast<const uint16_t*>(input_words.data());
    for(size_t i = 0; i < input_words.size(); i++)
    {
        output_words[i * index_mult] = ui16ptr[i * index_mult + 1];
        output_words[i * index_mult + 1] = ui16ptr[i * index_mult];
    }
}

void ICDTranslate::KeepMS16(const std::vector<uint32_t>& input_words,
    std::vector<uint16_t>& output_words)
{
    size_t index_mult = sizeof(uint32_t) / 2;
    if(output_words.size() != input_words.size())
        output_words.resize(input_words.size());

    const uint16_t* ui16ptr = reinterpret_cast<const uint16_t*>(input_words.data());
    for(size_t i = 0; i < input_words.size(); i++)
    {
        output_words[i] = ui16ptr[i * index_mult + 1];
    }
}

void ICDTranslate::KeepLS16(const std::vector<uint32_t>& input_words,
    std::vector<uint16_t>& output_words)
{
    size_t index_mult = sizeof(uint32_t) / 2;
    if(output_words.size() != input_words.size())
        output_words.resize(input_words.size());

    const uint16_t* ui16ptr = reinterpret_cast<const uint16_t*>(input_words.data());
    for(size_t i = 0; i < input_words.size(); i++)
    {
        output_words[i] = ui16ptr[i * index_mult];
    }
}


