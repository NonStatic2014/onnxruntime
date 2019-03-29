// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef ONNXRUNTIME_HOSTING_HTTP_JSON_HANDLING_H
#define ONNXRUNTIME_HOSTING_HTTP_JSON_HANDLING_H

#include <google/protobuf/util/json_util.h>
#include <boost/beast/http.hpp>

#include "predict.pb.h"

namespace onnxruntime {
namespace hosting {

namespace http = boost::beast::http;

// Deserialize Json input to PredictRequest.
// Unknown field in the json file will be ignored.
google::protobuf::util::Status GetRequestFromJson(const std::string& json_string, /* out */ onnxruntime::hosting::PredictRequest& request);

// Serialize PredictResponse to json string
// 1. To save the space, no whitespace will be added to prettify the string;
// 2. Proto3 primitive fields with default values will be omitted in JSON output. Eg. int32 field with value 0 will be omitted
// 3. Enums will be printed as string, not int, to improve the readability
// 4. Not preserve proto field names
google::protobuf::util::Status GenerateResponseInJson(const onnxruntime::hosting::PredictResponse& response, /* out */ std::string& json_string);

// Constructs JSON error message from error code object and error message
std::string CreateJsonError(http::status error_code, const std::string& error_message);

}  // namespace hosting
}  // namespace onnxruntime

#endif  // ONNXRUNTIME_HOSTING_HTTP_JSON_HANDLING_H
