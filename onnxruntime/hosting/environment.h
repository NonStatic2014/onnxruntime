// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef ONNXRUNTIME_HOSTING_ENVIRONMENT_H
#define ONNXRUNTIME_HOSTING_ENVIRONMENT_H

#include <memory>

#include "core/framework/environment.h"
#include "core/common/logging/logging.h"
#include "core/session/inference_session.h"
#include "log_sink.h"

namespace onnxruntime {
namespace hosting {

class HostingEnvironment {
 public:
  explicit HostingEnvironment(onnxruntime::logging::Severity severity);
  ~HostingEnvironment() = default;
  HostingEnvironment(const HostingEnvironment&) = delete;

  const onnxruntime::logging::Logger& GetAppLogger();
  std::shared_ptr<onnxruntime::logging::Logger> GetLogger(const std::string& id);
  std::shared_ptr<onnxruntime::InferenceSession> GetSession() const;
  onnxruntime::logging::Severity GetSeverity() const;

 private:
  const onnxruntime::logging::Severity severity_;
  bool default_filter_user_data_;
  const std::string logger_id_;
  onnxruntime::hosting::LogSink sink_;
  onnxruntime::logging::LoggingManager default_logging_manager_;

  std::unique_ptr<onnxruntime::Environment> runtime_environment_;
  onnxruntime::SessionOptions options_;
  std::shared_ptr<onnxruntime::InferenceSession> session_;
};

}  // namespace hosting
}  // namespace onnxruntime

#endif  //ONNXRUNTIME_HOSTING_ENVIRONMENT_H
