// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "gtest/gtest.h"
#include "test/providers/provider_test_utils.h"

namespace onnxruntime {
namespace test {

const int Scatter_ver = 9;

TEST(ScatterOpTest, WithoutAxis) {
  OpTester test("Scatter", Scatter_ver);

  std::vector<float> input;
  input.resize(3 * 3);
  std::fill(input.begin(), input.end(), .0f);
  test.AddInput<float>("data", {3, 3}, input);

  test.AddInput<int64_t>("indices", {2, 3},
                         {1, 0, 2,
                          0, 2, 1});

  test.AddInput<float>("updates", {2, 3},
                       {1.0f, 1.1f, 1.2f,
                        2.0f, 2.1f, 2.2f});

  test.AddOutput<float>("y", {3, 3},
                        {2.0f, 1.1f, 0.0f,
                         1.0f, 0.0f, 2.2f,
                         0.0f, 2.1f, 1.2f});
  test.Run();
}

TEST(ScatterOpTest, WithAxis) {
  OpTester test("Scatter", Scatter_ver);
  test.AddAttribute<int64_t>("axis", 1);

  test.AddInput<float>("data", {1, 5}, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f});
  test.AddInput<int64_t>("indices", {1, 2}, {1, 3});
  test.AddInput<float>("updates", {1, 2}, {1.1f, 2.1f});
  test.AddOutput<float>("y", {1, 5}, {1.0f, 1.1f, 3.0f, 2.1f, 5.0f});
  test.Run();
}

TEST(ScatterOpTest, WithAxisStrings) {
  OpTester test("Scatter", Scatter_ver);
  test.AddAttribute<int64_t>("axis", 1);

  test.AddInput<std::string>("data", {1, 5}, {"1.0f", "2.0f", "3.0f", "4.0f", "5.0f"});
  test.AddInput<int64_t>("indices", {1, 2}, {1, 3});
  test.AddInput<std::string>("updates", {1, 2}, {"1.1f", "2.1f"});
  test.AddOutput<std::string>("y", {1, 5}, {"1.0f", "1.1f", "3.0f", "2.1f", "5.0f"});
  test.Run();
}

TEST(ScatterOpTest, NegativeAxis) {
  OpTester test("Scatter", Scatter_ver);
  test.AddAttribute<int64_t>("axis", -1);

  test.AddInput<float>("data", {1, 5}, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f});
  test.AddInput<int64_t>("indices", {1, 2}, {1, 3});
  test.AddInput<float>("updates", {1, 2}, {1.1f, 2.1f});
  test.AddOutput<float>("y", {1, 5}, {1.0f, 1.1f, 3.0f, 2.1f, 5.0f});
  test.Run();
}

TEST(ScatterOpTest, InvalidAxis) {
  OpTester test("Scatter", Scatter_ver);
  test.AddAttribute<int64_t>("axis", 4);

  test.AddInput<float>("data", {1, 5}, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f});
  test.AddInput<int64_t>("indices", {1, 2}, {1, 3});
  test.AddInput<float>("updates", {1, 2}, {1.1f, 2.1f});
  test.AddOutput<float>("y", {1, 5}, {1.0f, 1.1f, 3.0f, 2.1f, 5.0f});
  test.Run(OpTester::ExpectResult::kExpectFailure, "onnxruntime::HandleNegativeAxis axis >= -tensor_rank && axis <= tensor_rank - 1 was false. axis 4 is not in valid range [-2,1]");
}

TEST(ScatterOpTest, IndicesUpdatesDimsDonotMatch) {
  OpTester test("Scatter", Scatter_ver);
  test.AddAttribute<int64_t>("axis", 1);

  test.AddInput<float>("data", {1, 5}, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f});
  test.AddInput<int64_t>("indices", {1, 3}, {1, 3, 3});
  test.AddInput<float>("updates", {1, 2}, {1.1f, 2.1f});
  test.AddOutput<float>("y", {1, 5}, {1.0f, 1.1f, 3.0f, 2.1f, 5.0f});
  test.Run(OpTester::ExpectResult::kExpectFailure, "Indicies vs updates dimensions differs at position=1 3 vs 2");
}
}  // namespace test
}  // namespace onnxruntime
