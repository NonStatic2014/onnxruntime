#include <iostream>

#include "gtest/gtest.h"

#include "server/executor.h"
#include "server/grpc/prediction_service_impl.h"
#include "test/test_environment.h"
#include "server_context_test_spouse.h"
#include <grpcpp/impl/grpc_library.h>

namespace onnxruntime {
namespace server {
namespace grpc {
namespace test {
static ::grpc::internal::GrpcLibraryInitializer g_initializer;

PredictRequest GetRequest() {
  PredictRequest req{};
  req.add_output_filter("Y");
  onnx::TensorProto proto{};
  proto.add_dims(3);
  proto.add_dims(2);
  proto.set_data_type(1);
  std::vector<float> floats = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f};
  std::for_each(floats.begin(), floats.end(), [&](float& f) { proto.add_float_data(f); });
  (*req.mutable_inputs())["X"] = proto;
  return req;
}

std::shared_ptr<onnxruntime::server::ServerEnvironment> GetEnvironment() {
  const static auto model_file = "testdata/mul_1.pb";

  auto env = std::make_shared<onnxruntime::server::ServerEnvironment>(logging::Severity::kWARNING, logging::LoggingManager::InstanceType::Temporal, false);

  auto status = env->InitializeModel(model_file);
  EXPECT_TRUE(status.IsOK());

  status = env->GetSession()->Initialize();
  EXPECT_TRUE(status.IsOK());
  return env;
}

TEST(PredictionServiceImplTests, Test_OK) {
  auto env = GetEnvironment();
  PredictionServiceImpl test{env};
  auto request = GetRequest();
  PredictResponse resp{};
  ::grpc::ServerContext context;
  auto status = test.Predict(&context, &request, &resp);
  EXPECT_TRUE(status.ok());
}

TEST(PredictionServiceImplTests, Test_fail_invalid_input) {
  auto env = GetEnvironment();
  PredictionServiceImpl test{env};
  auto request = GetRequest();
  (*request.mutable_inputs())["X"].add_dims(1);
  PredictResponse resp{};
  ::grpc::ServerContext context{};
  auto status = test.Predict(&context, &request, &resp);
  EXPECT_EQ(status.error_code(), ::grpc::INVALID_ARGUMENT);
}

TEST(PredictionServiceImplTests, Test_Set_request_ID_Success) {
  auto env = GetEnvironment();
  PredictionServiceImpl test{env};
  auto request = GetRequest();
  PredictResponse resp{};
  ::grpc::ServerContext context;
  ::grpc::testing::ServerContextTestSpouse spouse(&context);
  auto status = test.Predict(&context, &request, &resp);
  auto metadata = spouse.GetInitialMetadata();
  EXPECT_NE(metadata.find("x-ms-request-id"), metadata.end());
  EXPECT_TRUE(status.ok());
}

TEST(PredictionServiceImplTests, Test_Set_request_ID_invalid_input) {
  auto env = GetEnvironment();
  PredictionServiceImpl test{env};
  auto request = GetRequest();
  request.clear_inputs();

  PredictResponse resp{};
  ::grpc::ServerContext context;
  ::grpc::testing::ServerContextTestSpouse spouse(&context);
  auto status = test.Predict(&context, &request, &resp);
  auto metadata = spouse.GetInitialMetadata();
  EXPECT_NE(metadata.find("x-ms-request-id"), metadata.end());
  EXPECT_FALSE(status.ok());
}

TEST(PredictionServiceImplTests, Test_Set_client_request_ID_Success) {
  auto env = GetEnvironment();
  PredictionServiceImpl test{env};
  auto request = GetRequest();
  PredictResponse resp{};
  ::grpc::ServerContext context;
  ::grpc::testing::ServerContextTestSpouse spouse(&context);
  spouse.AddClientMetadata("x-ms-client-request-id", "client-id");
  auto status = test.Predict(&context, &request, &resp);
  auto metadata = spouse.GetInitialMetadata();
  EXPECT_NE(metadata.find("x-ms-client-request-id"), metadata.end());
  EXPECT_EQ(metadata.find("x-ms-client-request-id")->second, "client-id");
  EXPECT_TRUE(status.ok());
}

TEST(PredictionServiceImplTests, Test_Set_client_request_ID_invalid_input) {
  auto env = GetEnvironment();
  PredictionServiceImpl test{env};
  auto request = GetRequest();
  request.clear_inputs();

  PredictResponse resp{};
  ::grpc::ServerContext context;
  ::grpc::testing::ServerContextTestSpouse spouse(&context);
  spouse.AddClientMetadata("x-ms-client-request-id", "client-id");
  auto status = test.Predict(&context, &request, &resp);
  auto metadata = spouse.GetInitialMetadata();
  EXPECT_NE(metadata.find("x-ms-client-request-id"), metadata.end());
  EXPECT_EQ(metadata.find("x-ms-client-request-id")->second, "client-id");
  EXPECT_FALSE(status.ok());
}

}  // namespace test
}  // namespace grpc
}  // namespace server
}  // namespace onnxruntime