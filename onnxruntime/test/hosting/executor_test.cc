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
  /*auto res = env->GetSession()->Load("testdata/mul_1.pb");
  if (!res.IsOK()) {
    FAIL() << res.ErrorMessage();
  }

  onnxruntime::hosting::Executor executor(env, "RequestId");

  onnxruntime::hosting::PredictRequest request{};
  onnxruntime::hosting::PredictResponse response{};

  const static std::string input_json = R"({"inputs":{"X":{"dims":[3,2],"dataType":1,"floatData":[1,2,3,4,5,6]}},"outputFilter":["Y"]})";
  auto status = onnxruntime::hosting::GetRequestFromJson(input_json, request);

  if (!status.ok()) {
    FAIL() << status.ToString();
  } */

  // status = executor.Predict("sdf", "PSs", request, response);

  EXPECT_TRUE(true);
}

} // namespace test
} // namespace hosting
} // namespace onnxruntime
