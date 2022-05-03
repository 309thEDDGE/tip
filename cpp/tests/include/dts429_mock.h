#ifndef DTS_429_MOCK_H_
#define DTS_1553_MOCK_H_

#include <vector>
#include <string>
#include <unordered_map>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "dts429.h"
#include "icd_element.h"


class MockDTS429 : public DTS429
{
   public:
    MockDTS429() : DTS429() {}

    MOCK_METHOD2(IngestLines, bool(const std::vector<std::string>& lines,
                  std::unordered_map<std::string, std::vector<ICDElement>>& word_elements));

};


#endif  //DTS_429_MOCK_H_