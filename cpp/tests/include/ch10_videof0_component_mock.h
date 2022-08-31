#ifndef CH10_VIDEOF0_COMPONENT_MOCK_H_
#define CH10_VIDEOF0_COMPONENT_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_videof0_component.h"

class MockCh10VideoF0Component: public Ch10VideoF0Component
{
   public:
    MockCh10VideoF0Component(Ch10Context* const ch10ctx) : Ch10VideoF0Component(ch10ctx) {}
    MOCK_METHOD1(Parse, Ch10Status(const uint8_t*& data));
};


#endif  // CH10_VIDEOF0_COMPONENT_MOCK_H_