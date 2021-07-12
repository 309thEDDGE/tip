#include "icd_translate.h"

#define REGISTER_TRANSLATE_TYPE(X) \
    template <>                    \
    const char* ICDTranslate::TypeTranslateTraits<X>::name = #X

REGISTER_TRANSLATE_TYPE(uint8_t);
REGISTER_TRANSLATE_TYPE(int8_t);
REGISTER_TRANSLATE_TYPE(uint16_t);
REGISTER_TRANSLATE_TYPE(int16_t);
REGISTER_TRANSLATE_TYPE(uint32_t);
REGISTER_TRANSLATE_TYPE(int32_t);
REGISTER_TRANSLATE_TYPE(uint64_t);
REGISTER_TRANSLATE_TYPE(int64_t);
REGISTER_TRANSLATE_TYPE(float);
REGISTER_TRANSLATE_TYPE(double);

ICDTranslate::ICDTranslate() : n_translated_values_(0), downshift1_(0), downshift2_(0), mask1_(0), mask1_twos_(0), mask2_(0), mask2_twos_(0), scale_(0.0), scale_twos_(0.0), elem_word_count_(0), should_resize_output_vector_(true), sign_bit_mask_(0)
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
