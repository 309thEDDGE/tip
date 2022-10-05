#ifndef WORKER_CONFIG_MOCK_H_
#define WORKER_CONFIG_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "worker_config.h"

class MockWorkerConfig : public WorkerConfig
{
   public:
      MockWorkerConfig() : WorkerConfig() {}

      MOCK_METHOD0(CheckConfiguration, bool());
};


#endif  // WORKER_CONFIG_MOCK_H_
