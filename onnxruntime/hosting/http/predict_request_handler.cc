// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "environment.h"
#include "http_server.h"
#include "json_handling.h"

namespace onnxruntime {
namespace hosting {

std::string BadRequest(const http::status error_code, const std::string& error_message, HttpContext& context) {
  return "{\"error_code\": "  + std::to_string(int(error_code)) + ", \"error_message\": " + error_message + " }";
}

// TODO: decide whether this should be a class
void Predict(const std::string& name,
             const std::string& version,
             const std::string& action,
             HttpContext& context,
             std::shared_ptr<HostingEnvironment> env) {
  PredictRequest predictRequest{};
  auto logger = env->GetLogger();

  LOGS(logger, VERBOSE) << "Name: " << name;
  LOGS(logger, VERBOSE) << "Version: " << version;
  LOGS(logger, VERBOSE) << "Action: " << action;

  auto body = context.request.body();
  auto status = GetRequestFromJson(body, predictRequest);

  if (!status.ok()) {
    context.response.result(400);
    context.response.body() = BadRequest(http::status::bad_request, status.error_message(), context);
    return;
  }

  context.response.result(200);
  context.response.body() = body;
};

}  // namespace hosting
}  // namespace onnxruntime
