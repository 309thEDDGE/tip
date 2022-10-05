#ifndef CH10_TMATS_COMPONENT_MOCK_H_
#define CH10_TMATS_COMPONENT_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_tmats_component.h"

class MockCh10TMATSComponent: public Ch10TMATSComponent
{
   public:
    MockCh10TMATSComponent(Ch10Context* const ch10ctx) : Ch10TMATSComponent(ch10ctx) {}
    MOCK_METHOD1(Parse, Ch10Status(const uint8_t*& data));
};


#endif  // CH10_TMATS_COMPONENT_MOCK_H_