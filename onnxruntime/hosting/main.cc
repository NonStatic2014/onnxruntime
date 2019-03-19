// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <vector>

#include "http_server.h"
#include "server_configuration.h"
#include "environment.h"
#include "json_handling.h"

#include "core/platform/env.h"
#include "core/framework/framework_common.h"
#include "core/framework/mem_buffer.h"
#include "core/framework/ml_value.h"
#include "core/framework/tensorprotoutils.h"

#include "predict.pb.h"

namespace beast = boost::beast;
namespace http = beast::http;

onnxruntime::hosting::HostingEnvironment env;

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

  onnxruntime::hosting::PredictResponse predict_response;
  auto oTensorProto = outputs[0].GetMutable<onnxruntime::Tensor>();
  ::onnx::TensorProto* my_tensor = reinterpret_cast<::onnx::TensorProto*>(oTensorProto);
  (*(predict_response.mutable_outputs()))[output_names[0]] = *my_tensor;

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
