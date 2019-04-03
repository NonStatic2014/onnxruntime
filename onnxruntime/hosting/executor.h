// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef ONNXRUNTIME_HOSTING_EXECUTOR_H
#define ONNXRUNTIME_HOSTING_EXECUTOR_H

#include <google/protobuf/stubs/status.h>

#include "environment.h"
#include "predict.pb.h"

namespace onnxruntime {
namespace hosting {

class Executor {
 public:
  explicit Executor(std::shared_ptr<HostingEnvironment> hosting_env) : env_(std::move(hosting_env)) {}

  google::protobuf::util::Status SetMLValue(const onnx::TensorProto& input_tensor,
                                            std::shared_ptr<onnxruntime::logging::Logger> logger,
                                            OrtAllocatorInfo* cpu_allocator_info,
                                            /* out */ MLValue& ml_value);

  // Prediction method
  google::protobuf::util::Status Predict(const std::string& model_name,
                                         const std::string& model_version,
                                         const std::string& request_id,
                                         onnxruntime::hosting::PredictRequest& request,
                                         /* out */ onnxruntime::hosting::PredictResponse& response);

 private:
  const std::shared_ptr<HostingEnvironment> env_;
};
}  // namespace hosting
}  // namespace onnxruntime

#endif  //ONNXRUNTIME_HOSTING_EXECUTOR_H
