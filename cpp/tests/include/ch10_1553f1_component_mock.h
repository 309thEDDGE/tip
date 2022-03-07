#ifndef CH10_MILSTD1553F1_COMPONENT_MOCK_H_
#define CH10_MILSTD1553F1_COMPONENT_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_1553f1_component.h"

class MockCh101553F1Component: public Ch101553F1Component
{
   public:
    MockCh101553F1Component(Ch10Context* const ch10ctx) : Ch101553F1Component(ch10ctx) {}
    MOCK_METHOD1(Parse, Ch10Status(const uint8_t*& data));
};


#endif  // CH10_MILSTD1553F1_COMPONENT_MOCK_H_