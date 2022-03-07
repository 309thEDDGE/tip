#ifndef CH10_ETHERNETF0_COMPONENT_MOCK_H_
#define CH10_ETHERNETF0_COMPONENT_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_ethernetf0_component.h"

class MockCh10EthernetF0Component: public Ch10EthernetF0Component
{
   public:
    MockCh10EthernetF0Component(Ch10Context* const ch10ctx) : Ch10EthernetF0Component(ch10ctx) {}
    MOCK_METHOD1(Parse, Ch10Status(const uint8_t*& data));
};


#endif  // CH10_ETHERNETF0_COMPONENT_MOCK_H_