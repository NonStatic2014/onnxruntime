// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <iostream>

#include "gtest/gtest.h"
#include "test/test_environment.h"
#include "hosting/executor.h"
#include "hosting/http/json_handling.h"


namespace onnxruntime {
namespace hosting {
namespace test {

TEST(PositiveTests, PredictInputRequestWorks) {
  auto model_file = "testdata/mul_1.pb";

  {
    onnxruntime::hosting::HostingEnvironment env(logging::Severity::kWARNING, logging::LoggingManager::InstanceType::Temporal);
    auto res = env.GetSession()->Load(model_file);
    EXPECT_TRUE(res.IsOK());
  }

  EXPECT_TRUE(true);
}

} // namespace test
} // namespace hosting
} // namespace onnxruntime
