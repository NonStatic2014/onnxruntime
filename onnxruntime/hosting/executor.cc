// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <stdio.h>
#include <onnx/onnx_pb.h>
#include "core/common/logging/logging.h"
#include "core/framework/data_types.h"
#include "core/framework/environment.h"
#include "core/framework/framework_common.h"
#include "core/framework/mem_buffer.h"
#include "core/framework/ml_value.h"
#include "core/framework/tensor.h"
#include "core/framework/tensorprotoutils.h"

#include "onnx-ml.pb.h"
#include "predict.pb.h"

#include "converter.h"
#include "executor.h"

namespace onnxruntime {
namespace hosting {

namespace protobufutil = google::protobuf::util;


protobufutil::Status helper(const std::string& input_name,
                            const onnx::TensorProto& input_tensor,
                            std::shared_ptr<onnxruntime::logging::Logger> logger) {
  // Prepare the MLValue object
  OrtAllocatorInfo* cpu_allocator_info = nullptr;
  auto ort_status = OrtCreateAllocatorInfo("Cpu", OrtDeviceAllocator, 0, OrtMemTypeDefault, &cpu_allocator_info);
  if (ort_status != nullptr || cpu_allocator_info == nullptr) {
    LOGS(*logger, ERROR) << "OrtCreateAllocatorInfo FAILED! Input name: " << input_name;
    return protobufutil::Status(protobufutil::error::Code::RESOURCE_EXHAUSTED, "OrtCreateAllocatorInfo() FAILED!");
  }

  size_t cpu_tensor_length = 0;
  auto status = onnxruntime::utils::GetSizeInBytesFromTensorProto<0>(input_tensor, &cpu_tensor_length);
    if (!status.IsOK()) {
      LOGS(*logger, ERROR) << "GetSizeInBytesFromTensorProto() failed! Input name: " << input_name
                           << " Error code: " << status.Code()
                           << ". Error Message: " << status.ErrorMessage();
      return protobufutil::Status(static_cast<protobufutil::error::Code>(status.Code()),
                                  "GetSizeInBytesFromTensorProto() failed: " + status.ErrorMessage());
    }

    std::unique_ptr<char[]> data(new char[cpu_tensor_length]);
    if (nullptr == data) {
      LOGS(*logger, ERROR) << "Run out memory. Input name: " << input_name;
      return protobufutil::Status(protobufutil::error::Code::RESOURCE_EXHAUSTED, "Run out of memory");
    }
    memset(data.get(), 0, cpu_tensor_length);

    // TensorProto -> MLValue
    MLValue ml_value;
    OrtCallback deleter;
    status = onnxruntime::utils::TensorProtoToMLValue(onnxruntime::Env::Default(), nullptr, input_tensor,
                                                      onnxruntime::MemBuffer(data.release(), cpu_tensor_length, *cpu_allocator_info),
                                                      ml_value, deleter);
    if (!status.IsOK()) {
      LOGS(*logger, ERROR) << "TensorProtoToMLValue() failed! Input name: " << input_name
                           << " Error code: " << status.Code()
                           << ". Error Message: " << status.ErrorMessage();
      return protobufutil::Status(static_cast<protobufutil::error::Code>(status.Code()),
                                  "TensorProtoToMLValue() failed: " + status.ErrorMessage());
    }

    return protobufutil::Status::OK;
}

protobufutil::Status Executor::Predict(const std::string& model_name,
                                       const std::string& model_version,
                                       const std::string& request_id,
                                       onnxruntime::hosting::PredictRequest& request,
                                       /* out */ onnxruntime::hosting::PredictResponse& response) {
  bool using_raw_data = true;
  auto logger = env_->GetLogger(request_id);

  // Create the input NameMLValMap
  onnxruntime::NameMLValMap name_ml_value_map{};
  common::Status status{};

  OrtAllocatorInfo* cpu_allocator_info = nullptr;
  auto ort_status = OrtCreateAllocatorInfo("Cpu", OrtDeviceAllocator, 0, OrtMemTypeDefault, &cpu_allocator_info);
  if (ort_status != nullptr || cpu_allocator_info == nullptr) {
    LOGS(*logger, ERROR) << "OrtCreateAllocatorInfo failed";
    return protobufutil::Status(protobufutil::error::Code::RESOURCE_EXHAUSTED, "OrtCreateAllocatorInfo() failed!");
  }

  // Prepare the MLValue object
  for (const auto& input : request.inputs()) {
    std::string input_name = input.first;
    onnx::TensorProto input_tensor = input.second;
    using_raw_data = using_raw_data && input_tensor.has_raw_data();

    auto status = helper()

    auto insertion_result = name_ml_value_map.insert(std::make_pair(input_name, ml_value));
    if (!insertion_result.second) {
      LOGS(*logger, ERROR) << "Predict() failed! Input name: " << input_name
                           << " Trying to overwrite existing input value";
      return protobufutil::Status(protobufutil::error::Code::ALREADY_EXISTS,
                                  "Predict() failed: Trying to overwrite existing input value");
    }
  }  // for(const auto& input : request.inputs())

  // Prepare the output names and vector
  std::vector<std::string> output_names;
  for (const auto& name : request.output_filter()) {
    output_names.push_back(name);
  }
  std::vector<onnxruntime::MLValue> outputs(output_names.size());

  // Run()!
  OrtRunOptions run_options{};
  run_options.run_log_verbosity_level = 4;  // TODO: respect user selected log level
  run_options.run_tag = request_id;

  status = env_->GetSession()->Run(run_options, name_ml_value_map, output_names, &outputs);
  if (!status.IsOK()) {
    LOGS(*logger, ERROR) << "Run() failed!"
                         << " Error code: " << status.Code()
                         << ". Error Message: " << status.ErrorMessage();
    return protobufutil::Status(static_cast<protobufutil::error::Code>(status.Code()),
                                "Run() failed!" + status.ErrorMessage());
  }

  // Build the response
  for (size_t i = 0; i < outputs.size(); ++i) {
    onnx::TensorProto output_tensor{};
    status = MLValueToTensorProto(outputs[i], using_raw_data, logger, output_tensor);
    if (!status.IsOK()) {
      LOGS(*logger, ERROR) << "MLValue2TensorProto() failed! Output name: " << output_names[i]
                           << " Error code: " << status.Code()
                           << ". Error Message: " << status.ErrorMessage();
      return protobufutil::Status(static_cast<protobufutil::error::Code>(status.Code()), "MLValue2TensorProto() failed!");
    }

    response.mutable_outputs()->insert({output_names[i], output_tensor});
  }

  return protobufutil::Status::OK;
}

}  // namespace hosting
}  // namespace onnxruntime