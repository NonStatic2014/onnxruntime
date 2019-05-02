// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <iostream>

#include "gtest/gtest.h"

#include "server/executor.h"
#include "server/http/json_handling.h"
#include "test/test_environment.h"

namespace onnxruntime {
namespace server {
namespace test {

namespace {
void CheckStringInFile(const std::string& filename, const std::string& look_for) {
  std::ifstream ifs{filename};
  std::string content(std::istreambuf_iterator<char>{ifs},
                      std::istreambuf_iterator<char>{});

  EXPECT_NE(content.find(look_for), std::string::npos);
}

void DeleteFile(const std::string& filename) {
  int result = std::remove(filename.c_str());
  EXPECT_EQ(result, 0);
}
}  // namespace

TEST(ExecutorTests, TestLogSink) {
  const std::string filename{"TestLogSink.out"};
  const std::string logid{"LogSink"};
  const std::string message{"Test log message"};

  // redirect cout to a file so we can check the output
  std::ofstream ofs(filename);

  auto old_rdbuf = std::cout.rdbuf();
  std::cout.rdbuf(ofs.rdbuf());

  // create scoped manager so sink gets destroyed once done
  {
    onnxruntime::server::ServerEnvironment env(logging::Severity::kWARNING, logging::LoggingManager::InstanceType::Temporal, false);

    auto logger = env.GetLogger(logid);

    LOGS(*logger, WARNING) << message;
  }

  // check message was flushed to file before we close ofs.
  CheckStringInFile(filename, message);

  // revert redirection
  std::cout.rdbuf(old_rdbuf);
  ofs.close();

  DeleteFile(filename);
}

TEST(ExecutorTests, TestMul_1) {
  const static std::string model_file = "testdata/mul_1.pb";
  const static std::string input_json = R"({"inputs":{"X":{"dims":[3,2],"dataType":1,"floatData":[1,2,3,4,5,6]}},"outputFilter":["Y"]})";

  onnxruntime::server::ServerEnvironment env(logging::Severity::kWARNING, logging::LoggingManager::InstanceType::Temporal, false);

  auto res = env.InitializeModel(model_file);
  EXPECT_TRUE(res.IsOK());

  onnxruntime::server::Executor executor(&env, "RequestId");
  onnxruntime::server::PredictRequest request{};
  onnxruntime::server::PredictResponse response{};

  auto status = onnxruntime::server::GetRequestFromJson(input_json, request);
  if (!status.ok()) {
    FAIL() << status.ToString();
  }
  executor.Predict()

}

}  // namespace test
}  // namespace server
}  // namespace onnxruntime
