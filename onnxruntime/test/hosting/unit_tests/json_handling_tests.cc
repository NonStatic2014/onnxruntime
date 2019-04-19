// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <fstream>
#include <google/protobuf/stubs/status.h>

#include "gtest/gtest.h"

#include "predict.pb.h"
#include "hosting/http/json_handling.h"

namespace onnxruntime {
namespace hosting {
namespace test {
namespace protobufutil = google::protobuf::util;

TEST(JsonDeserializationTests, HappyPath) {
  std::string input_json = "{\"inputs\":{\"Input3\":{\"dims\":[\"1\",\"1\",\"28\",\"28\"],\"dataType\":1,\"rawData\":\"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\"}},\"outputFilter\":[\"Plus214_Output_0\"]}";
  onnxruntime::hosting::PredictRequest request;
  protobufutil::Status status = onnxruntime::hosting::GetRequestFromJson(input_json, request);

  EXPECT_EQ(protobufutil::error::OK, status.error_code());
}

TEST(JsonDeserializationTests, WithUnknownField) {
  std::string input_json = "{\"foo\": \"bar\",\"inputs\":{\"Input3\":{\"dims\":[\"1\",\"1\",\"28\",\"28\"],\"dataType\":1,\"rawData\":\"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\"}},\"outputFilter\":[\"Plus214_Output_0\"]}";
  onnxruntime::hosting::PredictRequest request;
  protobufutil::Status status = onnxruntime::hosting::GetRequestFromJson(input_json, request);

  EXPECT_EQ(protobufutil::error::OK, status.error_code());
}

TEST(JsonDeserializationTests, InvalidData) {
  std::string input_json = "{\"inputs\":{\"Input3\":{\"dims\":[\"1\",\"1\",\"28\",\"28\"],\"dataType\":1,\"rawData\":\"hello\"}},\"outputFilter\":[\"Plus214_Output_0\"]}";
  onnxruntime::hosting::PredictRequest request;
  protobufutil::Status status = onnxruntime::hosting::GetRequestFromJson(input_json, request);

  EXPECT_EQ(protobufutil::error::INVALID_ARGUMENT, status.error_code());
  EXPECT_EQ("inputs[0].value.raw_data: invalid value \"hello\" for type TYPE_BYTES", status.error_message());
}

TEST(JsonDeserializationTests, InvalidJson) {
  std::string input_json = "{inputs\":{\"Input3\":{\"dims\":[\"1\",\"1\",\"28\",\"28\"],\"dataType\":1,\"rawData\":\"hello\"}},\"outputFilter\":[\"Plus214_Output_0\"]}";
  onnxruntime::hosting::PredictRequest request;
  protobufutil::Status status = onnxruntime::hosting::GetRequestFromJson(input_json, request);

  EXPECT_EQ(protobufutil::error::INVALID_ARGUMENT, status.error_code());
  std::string errmsg = status.error_message();
  EXPECT_EQ("Expected : between key:value pair.\n{inputs\":{\"Input3\":{\"dims\":\n       ^", status.error_message());
}

TEST(JsonSerializationTests, HappyPath) {
  std::string test_data = "testdata/hosting/response_0.pb";
  std::string expected_json_string = "{\"outputs\":{\"Plus214_Output_0\":{\"dims\":[\"1\",\"10\"],\"dataType\":1,\"rawData\":\"4+pzRFWuGsSMdM1F2gEnRFdRZcRZ9NDEURj0xBIzdsJOS0LEA/GzxA==\"}}}";
  onnxruntime::hosting::PredictResponse response;
  std::string json_string;

  std::ifstream ifs(test_data, std::ios_base::in | std::ios_base::binary);
  ASSERT_TRUE(ifs) << test_data << " Not Found" << std::endl;

  bool succeeded = response.ParseFromIstream(&ifs);
  ifs.close();
  EXPECT_TRUE(succeeded) << test_data << " is invalid" << std::endl;

  protobufutil::Status status = onnxruntime::hosting::GenerateResponseInJson(response, json_string);

  EXPECT_EQ(protobufutil::error::OK, status.error_code());
  EXPECT_EQ(expected_json_string, json_string);
}

TEST(StringEscapingTests, SimpleString) {
  std::string unescaped = "This is an error message \" \n ";
  EXPECT_EQ("This is an error message \\\" \\n ", escape_string(unescaped));
}

TEST(StringEscapingTests, SimpleStringWithControlCharacter) {
  std::string unescaped = "This is an \x1f error message";
  EXPECT_EQ("This is an \\u001f error message", escape_string(unescaped));
}

TEST(StringEscapingTests, SimpleStringWithNullCharacter) {
  std::string unescaped = "This is an error message \x00 end";
  EXPECT_EQ("This is an error message ", escape_string(unescaped));
}

TEST(JsonErrorMessageTests, SimpleMessage) {
  auto status = http::status::bad_request;
  std::string error_message = "Incorrect headers";
  std::string expected = "{\"error_code\": 400, \"error_message\": \"Incorrect headers\"}\n";
  std::string res = CreateJsonError(status, error_message);
  EXPECT_EQ(expected, res);
}

TEST(JsonErrorMessageTests, MessageWithNewLine) {
  auto status = http::status::internal_server_error;
  std::string error_message = "Contains newline \n here";
  std::string expected = "{\"error_code\": 500, \"error_message\": \"Contains newline \\n here\"}\n";
  std::string res = CreateJsonError(status, error_message);
  EXPECT_EQ(expected, res);
}

TEST(JsonErrorMessageTests, MessageWithRealError) {
  auto status = http::status::bad_request;
  std::string error_message = "Expected , or ] after array value.\n0, 0.0, 0.0, 0.0    }  },  \"outputFilter\n                    ^";
  std::string expected = "{\"error_code\": 400, \"error_message\": \"Expected , or ] after array value.\\n0, 0.0, 0.0, 0.0    }  },  \\\"outputFilter\\n                    ^\"}\n";
  std::string res = CreateJsonError(status, error_message);
  EXPECT_EQ(expected, res);
}

TEST(JsonErrorMessageTests, MessageWithQuotations) {
  auto status = http::status::bad_request;
  std::string error_message = R"(Error with "{"bleh": [1,2,3]|")";
  std::string expected = "{\"error_code\": 400, \"error_message\": \"Error with \\\"{\\\"bleh\\\": [1,2,3]|\\\"\"}\n";
  std::string result_t = CreateJsonError(status, error_message);
  EXPECT_EQ(expected, result_t);
}

}  // namespace test
}  // namespace hosting
}  // namespace onnxruntime