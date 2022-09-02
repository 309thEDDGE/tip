#ifndef CH10_ARINC429F0_COMPONENT_MOCK_H_
#define CH10_ARINC429F0_COMPONENT_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_arinc429f0_component.h"

class MockCh10429F0Component: public Ch10429F0Component
{
   public:
    MockCh10429F0Component(Ch10Context* const ch10ctx) : Ch10429F0Component(ch10ctx) {}
    MOCK_METHOD1(Parse, Ch10Status(const uint8_t*& data));
};


#endif  // CH10_ARINC429F0_COMPONENT_MOCK_H_