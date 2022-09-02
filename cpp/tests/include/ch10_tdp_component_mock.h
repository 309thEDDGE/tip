#ifndef CH10_TDP_COMPONENT_MOCK_H_
#define CH10_TDP_COMPONENT_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_tdp_component.h"

class MockCh10TDPComponent: public Ch10TDPComponent
{
   public:
    MockCh10TDPComponent(Ch10Context* const ch10ctx) : Ch10TDPComponent(ch10ctx) {}
    MOCK_METHOD1(Parse, Ch10Status(const uint8_t*& data));
};


#endif  // CH10_TMATS_COMPONENT_MOCK_H_