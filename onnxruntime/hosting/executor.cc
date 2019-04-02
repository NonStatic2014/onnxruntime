// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <fstream>
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

// TODO: make all logging has request id

static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  char char_array_3[3];
  char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; (i < 4); i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while ((i++ < 3))
      ret += '=';
  }

  return ret;
}

protobufutil::Status Executor::Predict(const std::string& model_name, const std::string& model_version, const std::string& request_id,
                                       onnxruntime::hosting::PredictRequest& request,
                                       /* out */ onnxruntime::hosting::PredictResponse& response) {
  bool using_raw_data = true;
  auto logger = env_->GetLogger(request_id);

  // Create the input NameMLValMap
  onnxruntime::NameMLValMap nameMlValMap{};
  common::Status status{};
  for (const auto& input : request.inputs()) {
    std::string input_name = input.first;

    //    onnx::TensorProto input_tensor = input.second;
    onnx::TensorProto input_tensor;
    std::fstream input_file_stream("/home/klein/code/temp/3/input_0.pb", std::ios::in | std::ios::binary);
    bool result = input_tensor.ParseFromIstream(&input_file_stream);
    std::cout << "Parse input: " << result << std::endl;

    using_raw_data = using_raw_data && input_tensor.has_raw_data();

    // Prepare the MLValue object
    //    AllocatorPtr cpuAllocatorInfo = std::make_shared<CPUAllocator>();
    OrtAllocatorInfo* cpuAllocatorInfo = nullptr;
    auto ort_status = OrtCreateAllocatorInfo("Cpu", OrtArenaAllocator, 0, OrtMemTypeDefault, &cpuAllocatorInfo);
    if (ort_status != nullptr || cpuAllocatorInfo == nullptr) {
      LOGS(*logger, ERROR) << "OrtCreateAllocatorInfo FAILED! Input name: " << input_name;
      return protobufutil::Status(protobufutil::error::Code::RESOURCE_EXHAUSTED, "OrtCreateAllocatorInfo() FAILED!");
    }

    size_t cpu_tensor_length = 0;
    status = onnxruntime::utils::GetSizeInBytesFromTensorProto<0>(input_tensor, &cpu_tensor_length);
    if (!status.IsOK()) {
      LOGS(*logger, ERROR) << "GetSizeInBytesFromTensorProto() FAILED! Input name: " << input_name
                           << " Error code: " << status.Code()
                           << ". Error Message: " << status.ErrorMessage();
      return protobufutil::Status(static_cast<protobufutil::error::Code>(status.Code()),
                                  "GetSizeInBytesFromTensorProto() FAILED: " + status.ErrorMessage());
    }

    std::unique_ptr<char[]> data(new char[cpu_tensor_length]);
    memset(data.get(), 0, cpu_tensor_length);
    if (nullptr == data) {
      LOGS(*logger, ERROR) << "Run out memory. Input name: " << input_name;
      return protobufutil::Status(protobufutil::error::Code::RESOURCE_EXHAUSTED, "Run out of memory");
    }

    // "rawData": "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"
    //             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
    std::cout << "*********" << std::endl;
    std::cout << input_tensor.raw_data() << std::endl;
    std::string str = base64_encode(input_tensor.raw_data().c_str(), input_tensor.raw_data().length());
    std::cout << str << std::endl;
    std::cout << "*********" << std::endl;

    // TensorProto -> MLValue
    MLValue ml_value;
    OrtCallback deleter;
    status = onnxruntime::utils::TensorProtoToMLValue(onnxruntime::Env::Default(), nullptr, input_tensor,
                                                      onnxruntime::MemBuffer(data.get(), cpu_tensor_length, *cpuAllocatorInfo),
                                                      ml_value, deleter);
    if (!status.IsOK()) {
      LOGS(*logger, ERROR) << "TensorProtoToMLValue() FAILED! Input name: " << input_name
                           << " Error code: " << status.Code()
                           << ". Error Message: " << status.ErrorMessage();
      return protobufutil::Status(static_cast<protobufutil::error::Code>(status.Code()),
                                  "TensorProtoToMLValue() FAILED: " + status.ErrorMessage());
    }

    std::cout << "@@@@@@@@@@@@@@@@@" << std::endl;
    const auto& tensor = ml_value.Get<onnxruntime::Tensor>();
    const auto* datac = tensor.Data<float>();
    for (size_t i = 0; i < tensor.Size() / sizeof(float); i++) {
      std::cout << datac[i] << "  ";
    }
    std::cout << std::endl;
    std::cout << "@@@@@@@@@@@@@@@@@" << std::endl;

    nameMlValMap[input_name] = ml_value;
  }  // for(const auto& input : request.inputs())

  // Prepare the output names and vector
  std::vector<std::string> output_names;
  for (const auto& name : request.output_filter()) {
    output_names.push_back(name);
  }
  std::vector<onnxruntime::MLValue> outputs(output_names.size());

  // Run()!
  OrtRunOptions runOptions{};
  runOptions.run_log_verbosity_level = 4;  // TODO: respect user selected log level
  runOptions.run_tag = request_id;

  std::cout << "#####################" << std::endl;
  const auto& tensor = nameMlValMap["Input3"].Get<onnxruntime::Tensor>();
  const auto* datai = tensor.Data<float>();
  for (size_t i = 0; i < tensor.Size() / sizeof(float); i++) {
    std::cout << datai[i] << "  ";
  }
  std::cout << std::endl;
  std::cout << "#####################" << std::endl;

  // status = env_->GetSession()->Run(runOptions, nameMlValMap, output_names, &outputs);
  status = env_->GetSession()->Run(nameMlValMap, output_names, &outputs);
  if (!status.IsOK()) {
    LOGS(*logger, ERROR) << "Run() FAILED!"
                         << " Error code: " << status.Code()
                         << ". Error Message: " << status.ErrorMessage();
    return protobufutil::Status(static_cast<protobufutil::error::Code>(status.Code()),
                                "Run() FAILED!" + status.ErrorMessage());
  }

  // -131074  4.58281e-41  -131074  4.58281e-41  0  0  0  0  0  1  3  0  0  4  2  0  11  0  0  14  1  0  19  0  0  0  0  0  0  0  0  0  0  0  0  0  0  12  0  0  7  0  1  10  0  2  2  16  0  3  3  0  0  0  0  0  0  0  0  0  0  0  0  0  7  8  0  8  0  0  8  0  0  19  0  0  1  21  0  4  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1  0  0  1  0  0  0  0  0  11  0  0  10  3  0  0  0  0  0  0  0  0  0  0  0  0  13  0  15  10  26  34  17  77  181  178  35  4  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  150  254  250  251  243  252  252  255  45  6  0  5  0  9  0  0  0  0  0  0  0  0  0  0  0  0  7  72  205  255  238  243  255  254  251  248  201  198  57  0  19  0  0  0  0  0  0  0  0  0  0  0  0  0  0  218  255  241  255  249  250  251  250  255  255  242  224  49  0  12  0  0  0  0  0  0  1  2  3  2  2  1  0  65  228  255  254  244  119  34  41  110  250  255  248  124  20  0  0  0  0  0  1  1  0  0  0  0  0  0  12  0  62  103  113  117  34  0  0  0  200  244  255  255  0  12  0  0  0  0  2  1  0  0  0  0  1  2  0  0  2  4  0  11  0  7  6  0  75  244  255  255  4  0  0  0  0  0  0  0  0  1  2  3  4  4  0  14  0  0  0  9  0  2  0  0  34  255  255  253  10  10  0  0  0  0  0  0  1  2  3  2  0  0  3  2  0  13  11  0  0  0  6  12  99  255  254  248  15  12  0  0  0  0  0  1  1  1  0  0  0  0  1  1  0  0  5  6  11  0  0  17  184  247  255  243  13  0  0  0  0  0  3  2  0  0  0  0  2  4  4  0  11  0  51  94  85  5  5  25  246  246  255  208  0  9  0  0  0  0  4  1  0  0  1  7  15  19  99  103  182  189  237  253  252  191  190  227  243  252  210  18  7  0  0  0  0  0  0  4  0  0  32  109  185  247  255  242  255  244  255  255  242  251  255  240  255  255  218  124  9  0  0  0  0  0  2  0  0  0  127  255  235  255  255  247  229  212  242  250  255  255  248  255  253  249  255  243  170  12  0  0  0  0  0  11  0  9  253  255  255  233  202  85  0  53  196  238  255  227  238  142  109  193  255  240  255  180  0  0  0  0  6  0  22  1  245  243  254  255  217  235  226  213  244  251  255  239  77  0  0  20  182  247  239  243  0  0  0  0  0  0  0  4  165  251  255  245  255  242  253  250  255  197  107  59  0  18  2  6  0  54  255  158  0  0  0  0  0  24  0  0  6  34  167  194  176  183  164  44  2  10  6  6  0  0  5  0  1  0  14  2  0  0  0  0  10  0  14  0  12  0  5  0  1  0  6  0  7  0  0  0  8  0  10  0  5  0  0  10  0  0  0  0  0  14  0  4  0  0  25  0  0  9  0  0  9  0  11  0  1  0  0  2  0  0  7  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
  // 0        0            0        0            0  0  0  0  0  1  3  0  0  4  2  0  11  0  0  14  1  0  19  0  0  0  0  0  0  0  0  0  0  0  0  0  0  12  0  0  7  0  1  10  0  2  2  16  0  3  3  0  0  0  0  0  0  0  0  0  0  0  0  0  7  8  0  8  0  0  8  0  0  19  0  0  1  21  0  4  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1  0  0  1  0  0  0  0  0  11  0  0  10  3  0  0  0  0  0  0  0  0  0  0  0  0  13  0  15  10  26  34  17  77  181  178  35  4  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  150  254  250  251  243  252  252  255  45  6  0  5  0  9  0  0  0  0  0  0  0  0  0  0  0  0  7  72  205  255  238  243  255  254  251  248  201  198  57  0  19  0  0  0  0  0  0  0  0  0  0  0  0  0  0  218  255  241  255  249  250  251  250  255  255  242  224  49  0  12  0  0  0  0  0  0  1  2  3  2  2  1  0  65  228  255  254  244  119  34  41  110  250  255  248  124  20  0  0  0  0  0  1  1  0  0  0  0  0  0  12  0  62  103  113  117  34  0  0  0  200  244  255  255  0  12  0  0  0  0  2  1  0  0  0  0  1  2  0  0  2  4  0  11  0  7  6  0  75  244  255  255  4  0  0  0  0  0  0  0  0  1  2  3  4  4  0  14  0  0  0  9  0  2  0  0  34  255  255  253  10  10  0  0  0  0  0  0  1  2  3  2  0  0  3  2  0  13  11  0  0  0  6  12  99  255  254  248  15  12  0  0  0  0  0  1  1  1  0  0  0  0  1  1  0  0  5  6  11  0  0  17  184  247  255  243  13  0  0  0  0  0  3  2  0  0  0  0  2  4  4  0  11  0  51  94  85  5  5  25  246  246  255  208  0  9  0  0  0  0  4  1  0  0  1  7  15  19  99  103  182  189  237  253  252  191  190  227  243  252  210  18  7  0  0  0  0  0  0  4  0  0  32  109  185  247  255  242  255  244  255  255  242  251  255  240  255  255  218  124  9  0  0  0  0  0  2  0  0  0  127  255  235  255  255  247  229  212  242  250  255  255  248  255  253  249  255  243  170  12  0  0  0  0  0  11  0  9  253  255  255  233  202  85  0  53  196  238  255  227  238  142  109  193  255  240  255  180  0  0  0  0  6  0  22  1  245  243  254  255  217  235  226  213  244  251  255  239  77  0  0  20  182  247  239  243  0  0  0  0  0  0  0  4  165  251  255  245  255  242  253  250  255  197  107  59  0  18  2  6  0  54  255  158  0  0  0  0  0  24  0  0  6  34  167  194  176  183  164  44  2  10  6  6  0  0  5  0  1  0  14  2  0  0  0  0  10  0  14  0  12  0  5  0  1  0  6  0  7  0  0  0  8  0  10  0  5  0  0  10  0  0  0  0  0  14  0  4  0  0  25  0  0  9  0  0  9  0  11  0  1  0  0  2  0  0  7  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0

  // (975.6701049804688, -618.7239379882812, 6574.568359375, 668.0289306640625, -917.2709350585938, -1671.6358642578125, -1952.7598876953125, -61.54987335205078, -777.1766357421875, -1439.5316162109375)
  std::cout << "+++++++++nnnnn++++++++" << std::endl;
  const auto& otensor = outputs[0].Get<onnxruntime::Tensor>();
  const auto* datao = otensor.Data<float>();
  for (size_t i = 0; i < otensor.Size() / sizeof(float); i++) {
    std::cout << datao[i] << "  ";
  }
  std::cout << std::endl;
  std::cout << "+++++++++nnnn++++++++" << std::endl;

  // Build the response
  for (size_t i = 0; i < outputs.size(); ++i) {
    onnx::TensorProto output_tensor{};
    status = MLValue2TensorProto(outputs[i], using_raw_data, std::move(logger), output_tensor);
    if (!status.IsOK()) {
      LOGS(*logger, ERROR) << "MLValue2TensorProto() FAILED! Output name: " << output_names[i]
                           << " Error code: " << status.Code()
                           << ". Error Message: " << status.ErrorMessage();
      return protobufutil::Status(static_cast<protobufutil::error::Code>(status.Code()), "MLValue2TensorProto() FAILED!");
    }

    response.mutable_outputs()->insert({output_names[i], output_tensor});
  }

  return protobufutil::Status::OK;
}

}  // namespace hosting
}  // namespace onnxruntime