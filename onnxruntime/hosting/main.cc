// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <vector>

#include "http_server.h"
#include "server_configuration.h"
#include "environment.h"
#include "json_handling.h"

#include "core/platform/env.h"
#include "core/framework/data_types.h"
#include "core/framework/framework_common.h"
#include "core/framework/mem_buffer.h"
#include "core/framework/ml_value.h"
#include "core/framework/tensorprotoutils.h"

#include "onnx-ml.pb.h"
#include "predict.pb.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace protobufutil = google::protobuf::util;

onnxruntime::hosting::HostingEnvironment env;

onnx::TensorProto_DataType MLDataTypeToTensorProtoDataType(
    const onnxruntime::DataTypeImpl* cpp_type) {
  onnx::TensorProto_DataType type;
  if (cpp_type == onnxruntime::DataTypeImpl::GetType<float>()) {
    type = onnx::TensorProto_DataType_FLOAT;
  } else if (cpp_type == onnxruntime::DataTypeImpl::GetType<uint8_t>()) {
    type = onnx::TensorProto_DataType_UINT8;
  } else if (cpp_type == onnxruntime::DataTypeImpl::GetType<int8_t>()) {
    type = onnx::TensorProto_DataType_INT8;
  } else if (cpp_type == onnxruntime::DataTypeImpl::GetType<uint16_t>()) {
    type = onnx::TensorProto_DataType_UINT16;
  } else if (cpp_type == onnxruntime::DataTypeImpl::GetType<int16_t>()) {
    type = onnx::TensorProto_DataType_INT16;
  } else if (cpp_type == onnxruntime::DataTypeImpl::GetType<int32_t>()) {
    type = onnx::TensorProto_DataType_INT32;
  } else if (cpp_type == onnxruntime::DataTypeImpl::GetType<int64_t>()) {
    type = onnx::TensorProto_DataType_INT64;
  } else if (cpp_type == onnxruntime::DataTypeImpl::GetType<std::string>()) {
    type = onnx::TensorProto_DataType_STRING;
  } else if (cpp_type == onnxruntime::DataTypeImpl::GetType<bool>()) {
    type = onnx::TensorProto_DataType_BOOL;
  } else if (cpp_type == onnxruntime::DataTypeImpl::GetType<onnxruntime::MLFloat16>()) {
    type = onnx::TensorProto_DataType_FLOAT16;
  } else if (cpp_type == onnxruntime::DataTypeImpl::GetType<onnxruntime::BFloat16>()) {
    type = onnx::TensorProto_DataType_BFLOAT16;
  } else if (cpp_type == onnxruntime::DataTypeImpl::GetType<double>()) {
    type = onnx::TensorProto_DataType_DOUBLE;
  } else if (cpp_type == onnxruntime::DataTypeImpl::GetType<uint32_t>()) {
    type = onnx::TensorProto_DataType_UINT32;
  } else if (cpp_type == onnxruntime::DataTypeImpl::GetType<uint64_t>()) {
    type = onnx::TensorProto_DataType_UINT64;
  } else {
    type = onnx::TensorProto_DataType_UNDEFINED;
  }
  return type;
}

protobufutil::Status MLValue2TensorProto(onnxruntime::MLValue& ml_value, /* out */ onnx::TensorProto& tensor_proto) {
  // Tensor in MLValue
  onnxruntime::Tensor* tensor = ml_value.GetMutable<onnxruntime::Tensor>();

  // dims
  const onnxruntime::TensorShape& tensor_shape = tensor->Shape();
  for (auto dim : tensor_shape.GetDims()) {
    tensor_proto.add_dims(dim);
  }

  // data_type
  auto data_type = MLDataTypeToTensorProtoDataType(tensor->DataType());
  tensor_proto.set_data_type(data_type);

  // segment: ignored for now. We do not expect very large tensors in the output

  // data
  tensor_proto.set_raw_data(tensor->Data<float>(), tensor->Size());
  //  switch (data_type) {
  //    case onnx::TensorProto_DataType_FLOAT: {
  //      auto data = tensor->Data<float>();
  //      size_t data_length = tensor->Size() / sizeof(float);
  //      for (int i = 0; i < data_length; ++i) {
  //        tensor_proto.add_float_data(data[i]);
  //      }
  //      break;
  //    }
  //    default:
  //      std::cout << "error: " << data_type << std::endl;
  //  }

  return protobufutil::Status(protobufutil::Status::OK);
}

void test_request(const std::string& name, const std::string& version,
                  const std::string& action, onnxruntime::hosting::HttpContext& context) {
  std::stringstream ss;

  ss << "\tModel Name: " << name << std::endl;
  ss << "\tModel Version: " << version << std::endl;
  ss << "\tAction: " << action << std::endl;
  ss << "\tHTTP method: " << context.request.method() << std::endl;

  std::string body_string = context.request.body();
  onnxruntime::hosting::PredictRequest predict_request;
  auto json2pd_result = GetRequestFromJson(body_string, predict_request);
  //  Status TensorProtoToMLValue(const Env& env, const ORTCHAR_T* tensor_proto_path,
  //                              const ONNX_NAMESPACE::TensorProto& tensor_proto, const MemBuffer& m, MLValue& value,
  //                              OrtCallback& deleter)

  std::unique_ptr<onnxruntime::MLValue> ml_value = std::make_unique<onnxruntime::MLValue>();
  std::unique_ptr<OrtCallback> del = std::make_unique<OrtCallback>();
  OrtAllocatorInfo* cpuAllocatorInfo;
  auto st = OrtCreateAllocatorInfo("Cpu", OrtDeviceAllocator, 0, OrtMemTypeDefault, &cpuAllocatorInfo);
  if (st != nullptr) {
    ss << "OrtCreateAllocatorInfo FAILED!" << std::endl;
  }
  auto one_tensorproto = predict_request.inputs().begin()->second;
  size_t cpu_tensor_length;
  auto getsize_status = onnxruntime::utils::GetSizeInBytesFromTensorProto<0>(one_tensorproto, &cpu_tensor_length);
  std::unique_ptr<char[]> data(new char[cpu_tensor_length]);
  auto status =
      onnxruntime::utils::TensorProtoToMLValue(onnxruntime::Env::Default(), nullptr, one_tensorproto,
                                               onnxruntime::MemBuffer(data.get(), cpu_tensor_length, *cpuAllocatorInfo),
                                               *ml_value, *del);

  ss << "\tIs Tensor: " << ml_value->IsTensor() << std::endl;
  ss << "\tType: " << ml_value->Type() << std::endl;

  ss << "\tCurrent Num Runs: " << env.GetSession()->GetCurrentNumRuns() << std::endl;

  OrtRunOptions runOptions{};
  runOptions.run_log_verbosity_level = 4;
  runOptions.run_tag = "my-tag";
  onnxruntime::NameMLValMap nameMlValMap;
  nameMlValMap[predict_request.inputs().begin()->first] = *ml_value;
  std::vector<std::string> output_names{predict_request.output_filter(0)};
  std::vector<onnxruntime::MLValue> outputs;

  auto run_status = env.GetSession()->Run(runOptions, nameMlValMap, output_names, &outputs);

  ss << "\tRun Status: " << run_status.Code() << ". Error Message: [" << run_status.ErrorMessage() << "]" << std::endl;
  ss << "\tCurrent Num Runs: " << env.GetSession()->GetCurrentNumRuns() << std::endl;

  auto oTensor = outputs[0].GetMutable<onnxruntime::Tensor>();
  auto t = MLDataTypeToTensorProtoDataType(oTensor->DataType());
  ss << "\tOutput Data Type: " << t << std::endl;
  ss << "\tOutput Shape: " << oTensor->Shape().ToString() << std::endl;
  ss << "\tOutput Size: " << oTensor->Size() << std::endl;

  //  onnxruntime::hosting::PredictResponse predict_response;
  //  auto oTensorProto = outputs[0].GetMutable<onnxruntime::Tensor>();
  //  ::onnx::TensorProto* my_tensor = reinterpret_cast<::onnx::TensorProto*>(oTensorProto);
  //  (*(predict_response.mutable_outputs()))[output_names[0]] = *my_tensor;
  //
  //  std::string json_response;
  //  auto pd2json_result = GenerateResponseInJson(predict_response, json_response);
  //
  //  ss << "\tJson Response: " << json_response << std::endl;

  onnx::TensorProto output_tensor;
  auto mlvalue2tensorproto_status = MLValue2TensorProto(outputs[0], output_tensor);

  onnxruntime::hosting::PredictResponse predict_response;
  predict_response.mutable_outputs()->insert({output_names[0], output_tensor});
  std::string json_response;
  auto pd2json_result = GenerateResponseInJson(predict_response, json_response);
  ss << "\tJson Response: " << json_response << std::endl;

  // Build response
  http::response<http::string_body>
      res{std::piecewise_construct, std::make_tuple(ss.str()), std::make_tuple(http::status::ok, context.request.version())};

  res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  res.set(http::field::content_type, "plain/text");
  res.keep_alive(context.request.keep_alive());
  context.response = res;
}

int main(int argc, char* argv[]) {
  onnxruntime::hosting::ServerConfiguration config{};
  auto res = config.ParseInput(argc, argv);

  if (res == onnxruntime::hosting::Result::ExitSuccess) {
    exit(EXIT_SUCCESS);
  } else if (res == onnxruntime::hosting::Result::ExitFailure) {
    exit(EXIT_FAILURE);
  }

  // onnxruntime::hosting::HostingEnvironment env;
  auto logger = env.GetLogger();

  // TODO: below code snippet just trying to show case how to use the "env".
  //       Will be moved to proper place.
  LOGS(logger, VERBOSE) << "Logging manager initialized.";
  LOGS(logger, VERBOSE) << "Model path: " << config.model_path;
  auto status = env.GetSession()->Load(config.model_path);
  LOGS(logger, VERBOSE) << "Load Model Status: " << status.Code() << " ---- Error: [" << status.ErrorMessage() << "]";
  LOGS(logger, VERBOSE) << "Session Initialized: " << env.GetSession()->Initialize();
  auto const boost_address = boost::asio::ip::make_address(config.address);

  onnxruntime::hosting::App app{};
  app.Post(R"(/v1/models/([^/:]+)(?:/versions/(\d+))?:(classify|regress|predict))", test_request)
      .Bind(boost_address, config.http_port)
      .NumThreads(config.num_http_threads)
      .Run();

  return EXIT_SUCCESS;
}
