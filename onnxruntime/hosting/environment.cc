// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <memory>
#include "core/common/logging/logging.h"

#include "environment.h"
#include "log_sink.h"

namespace onnxruntime {
namespace hosting {

HostingEnvironment::HostingEnvironment(logging::Severity severity, logging::LoggingManager::InstanceType instance_type) : severity_(severity),
                                                                                                                          default_filter_user_data_(false),
                                                                                                                          logger_id_("HostingApp"),
                                                                                                                          default_logging_manager_(
                                                                                                                              std::unique_ptr<logging::ISink>{new LogSink{}},
                                                                                                                              severity,
                                                                                                                              false,
                                                                                                                              instance_type,
                                                                                                                              &logger_id_) {
  auto status = onnxruntime::Environment::Create(runtime_environment_);

  // The session initialization MUST BE AFTER environment creation
  session_ = std::make_shared<onnxruntime::InferenceSession>(options_, &default_logging_manager_);
}

const logging::Logger& HostingEnvironment::GetAppLogger() {
  return default_logging_manager_.DefaultLogger();
}

std::shared_ptr<logging::Logger> HostingEnvironment::GetLogger(const std::string& id) {
  if (id.empty()) {
    LOGS(GetAppLogger(), WARNING) << "Request id is null or empty string";
  }

  return default_logging_manager_.CreateLogger(id, severity_, default_filter_user_data_);
}

std::shared_ptr<onnxruntime::InferenceSession> HostingEnvironment::GetSession() const {
  return session_;
}

}  // namespace hosting
}  // namespace onnxruntime
