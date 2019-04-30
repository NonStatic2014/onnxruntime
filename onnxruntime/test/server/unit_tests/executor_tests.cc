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
  onnxruntime::SessionOptions options_{};
  std::unique_ptr<onnxruntime::Environment> runtime_environment_;
  #ifdef HAVE_FRAMEWORK_LIB
    std::cout << "HAVE_FRAMEWORK_LIB enabled" << std::endl;
  #else
    auto status = onnxruntime::Environment::Create(runtime_environment_);
  #endif
  onnxruntime::InferenceSession sess(options_, &::onnxruntime::test::DefaultLoggingManager());
}

}  // namespace test
}  // namespace server
}  // namespace onnxruntime
