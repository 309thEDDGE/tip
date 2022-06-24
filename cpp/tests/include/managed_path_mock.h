#ifndef MANAGED_PATH_MOCK_H_
#define MANAGED_PATH_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "managed_path.h"

class MockManagedPath : public ManagedPath
{
   public:
    MockManagedPath() : ManagedPath() {}
    MOCK_CONST_METHOD0(is_regular_file, bool());
    MOCK_CONST_METHOD2(GetFileSize, void(bool& success, uint64_t& result));
};

#endif  // MANAGED_PATH_MOCK_H_
