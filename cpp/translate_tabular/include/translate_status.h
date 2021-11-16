#ifndef TRANSLATE_STATUS_H_
#define TRANSLATE_STATUS_H_

#include <cstdint>

enum class TranslateStatus : uint8_t
{
    NONE,
    OK,
    CONTINUE,
    FAIL,
};

#endif
