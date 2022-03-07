#ifndef CH10_TIME_MOCK_H_
#define CH10_TIME_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_time.h"

class MockCh10Time: public Ch10Time
{
   public:
    MockCh10Time() : Ch10Time() {}
    MOCK_METHOD2(CalculateRTCTimeFromComponents, uint64_t&(const uint32_t& rtc1, 
        const uint32_t& rtc2));
};

#endif  // CH10_TIME_MOCK_H_