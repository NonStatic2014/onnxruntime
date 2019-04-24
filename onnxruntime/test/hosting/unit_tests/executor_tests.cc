// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <iostream>

#include "gtest/gtest.h"
#include "test/test_environment.h"
#include "hosting/executor.h"
#include "hosting/http/json_handling.h"
#include "core/common/logging/sinks/clog_sink.h"

namespace onnxruntime {
namespace hosting {
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

TEST(PositiveTests, StolenFromSinksTest) {
  const std::string filename{"TestCLogSink.out"};
  const std::string logid{"CLogSink"};
  const std::string message{"Test clog message"};
  const logging::Severity min_log_level = logging::Severity::kWARNING;

  // redirect clog to a file so we can check the output
  std::ofstream ofs(filename);

  auto old_rdbuf = std::clog.rdbuf();
  std::clog.rdbuf(ofs.rdbuf());

  // create scoped manager so sink gets destroyed once done
  {
    logging::LoggingManager manager{std::unique_ptr<logging::ISink>{new logging::CLogSink{}}, min_log_level, false,
                                    logging::LoggingManager::InstanceType::Temporal};

    auto logger = manager.CreateLogger(logid);

    LOGS(*logger, WARNING) << message;
  }

  // check message was flushed to file before we close ofs.
  CheckStringInFile(filename, message);

  // revert redirection
  std::clog.rdbuf(old_rdbuf);
  ofs.close();

  DeleteFile(filename);
}

TEST(PositiveTests, TestHostingEnvironment) {
  const static std::string model_file = "testdata/mul_1.pb";
  const static std::string input_json = R"({"inputs":{"X":{"dims":[3,2],"dataType":1,"floatData":[1,2,3,4,5,6]}},"outputFilter":["Y"]})";

  #ifdef HAVE_FRAMEWORK_LIB
  std::cout << "@@@@@@@@@@@@@@@@@@@@@@" << std::endl;
  #endif
  // create scoped manager so sink gets destroyed once done
  {
    onnxruntime::hosting::HostingEnvironment env(logging::Severity::kWARNING, logging::LoggingManager::InstanceType::Temporal, true);
    auto res = env.InitializeModel(model_file);
    EXPECT_TRUE(res.IsOK());

    onnxruntime::hosting::Executor executor(&env, "RequestId");
    onnxruntime::hosting::PredictRequest request{};
    onnxruntime::hosting::PredictResponse response{};
    auto status = onnxruntime::hosting::GetRequestFromJson(input_json, request);
    if (!status.ok()) {
      FAIL() << status.ToString();
    }
    const auto& logger = env.GetAppLogger();

    LOGS(logger, WARNING) << "####################################&&&&&&&&&&&&&&&&&&&";
    // status = executor.Predict("sdf", "PSs", request, response);
  }
  int * q = new int;
}

}  // namespace test
}  // namespace hosting
}  // namespace onnxruntime
