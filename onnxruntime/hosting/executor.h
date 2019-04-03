#include <utility>

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
  Executor(std::shared_ptr<HostingEnvironment> hosting_env, std::string request_id) : env_(std::move(hosting_env)), request_id_(std::move(request_id)) {}

  // Prediction method
  google::protobuf::util::Status Predict(const std::string& model_name,
                                         const std::string& model_version,
                                         onnxruntime::hosting::PredictRequest& request,
                                         /* out */ onnxruntime::hosting::PredictResponse& response);

 private:
  const std::shared_ptr<HostingEnvironment> env_;
  const std::string request_id_;

  google::protobuf::util::Status SetMLValue(const onnx::TensorProto& input_tensor,
                                            OrtAllocatorInfo* cpu_allocator_info,
                                            /* out */ MLValue& ml_value);
};
}  // namespace hosting
}  // namespace onnxruntime

#endif  //ONNXRUNTIME_HOSTING_EXECUTOR_H
