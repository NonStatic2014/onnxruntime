// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <iostream>

#include "gtest/gtest.h"
#include "hosting/executor.h"

namespace onnxruntime {
namespace hosting {
namespace test {


TEST(PositiveTests, PredictInputRequestWorks) {
  auto env = std::make_shared<onnxruntime::hosting::HostingEnvironment>(onnxruntime::logging::Severity::kWARNING);
  onnxruntime::hosting::Executor executor(env, "RequestId");

  onnxruntime::hosting::PredictRequest request{};
  onnxruntime::hosting::PredictResponse response{};

  auto status = onnxruntime::hosting::GetRequestFromJson("stub", request);

  EXPECT_EQ(3,2);
}

} // namespace test
} // namespace hosting
} // namespace onnxruntime
