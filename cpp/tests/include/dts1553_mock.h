#ifndef DTS_1553_MOCK_H_
#define DTS_1553_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "dts1553.h"

class MockDTS1553 : public DTS1553
{
   public:
    MockDTS1553() : DTS1553() {}
    MOCK_METHOD4(IngestLines, bool(const ManagedPath& dts_path, const std::vector<std::string>& lines,
                     std::map<std::string, std::string>& msg_name_substitutions,
                     std::map<std::string, std::string>& elem_name_substitutions));
};

#endif  // DTS_1553_MOCK_H_
