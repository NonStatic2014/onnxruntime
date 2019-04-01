// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "environment.h"
#include "executor.h"

#include <google/protobuf/stubs/status.h>


namespace hosting = onnxruntime::hosting;

int main(int argc, char* argv[]) {
  auto env = std::make_shared<hosting::HostingEnvironment>(onnxruntime::logging::Severity::kVERBOSE);
  auto logger = env->GetAppLogger();
  LOGS(logger, VERBOSE) << "Logging manager initialized.";
  auto status = env->GetSession()->Load("onnxruntime/test/testdata/mul_1.pb");
  LOGS(logger, VERBOSE) << "Load Model Status: " << status.Code() << " ---- Error: [" << status.ErrorMessage() << "]";
  LOGS(logger, VERBOSE) << "Session Initialized: " << env->GetSession()->Initialize();

  auto body = "afdsd";

  PredictRequest predictRequest{};
  auto status = GetRequestFromJson(body, predictRequest);
  if (!status.ok()) {
    GenerateErrorResponse(logger, GetHttpStatusCode((status)), status, context);
    return 1;
  }

  Executor executor(env);
  PredictResponse predictResponse{};
  status = executor.Predict(name, version, "request_id", predictRequest, predictResponse);
  if (!status.ok()) {
    GenerateErrorResponse(logger, GetHttpStatusCode((status)), status, context);
    return 1;
  }

  std::string response_body{};
  status = GenerateResponseInJson(predictResponse, response_body);
  if (!status.ok()) {
    GenerateErrorResponse(logger, http::status::internal_server_error, status, context);
    return 1;
  }

  return EXIT_SUCCESS;
}
