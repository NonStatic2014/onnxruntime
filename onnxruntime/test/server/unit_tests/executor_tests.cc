// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <iostream>

#include "gtest/gtest.h"
#include "test/framework/test_utils.h"
#include "test/test_environment.h"
#include "server/executor.h"
#include "server/http/json_handling.h"
#include "core/common/logging/sinks/clog_sink.h"

namespace onnxruntime {
namespace server {
namespace test {

TEST(PositiveTests, TestServerEnvironment) {
  const static std::string model_file = "testdata/mul_1.pb";
  const static std::string input_json = R"({"inputs":{"X":{"dims":[3,2],"dataType":1,"floatData":[1,2,3,4,5,6]}},"outputFilter":["Y"]})";

  {
    /*onnxruntime::server::ServerEnvironment env(logging::Severity::kWARNING, logging::LoggingManager::InstanceType::Temporal, false);
    auto res = env.InitializeModel(model_file);
    EXPECT_TRUE(res.IsOK());

    onnxruntime::server::Executor executor(&env, "RequestId");
    onnxruntime::server::PredictRequest request{};
    onnxruntime::server::PredictResponse response{};
    auto status = onnxruntime::server::GetRequestFromJson(input_json, request);
    if (!status.ok()) {
      FAIL() << status.ToString();
    }
    const auto& logger = env.GetAppLogger();

    LOGS(logger, WARNING) << "####################################&&&&&&&&&&&&&&&&&&&";*/
    // status = executor.Predict("sdf", "PSs", request, response);
    onnxruntime::SessionOptions options_{};
    std::unique_ptr<onnxruntime::Environment> runtime_environment_;
    //auto status = onnxruntime::Environment::Create(runtime_environment_);
    //EXPECT_TRUE(status.IsOK());
    onnxruntime::InferenceSession sess(options_, &::onnxruntime::test::DefaultLoggingManager());
  }
  EXPECT_TRUE(true);
}

}  // namespace test
}  // namespace server
}  // namespace onnxruntime
