// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <iostream>

#include "gtest/gtest.h"
#include "hosting/include/routes.h"

namespace onnxruntime {
namespace test {

using handler_fn = std::function<void(std::string, std::string, std::string, Http_Context&)>;
using test_data = std::tuple<http::verb, std::string, std::string, std::string, std::string, http::status>;

void do_something(const std::string& name, const std::string& version,
                  const std::string& action, Http_Context& context) {
  auto noop = name + version + action + context.request.body();
}

void run_route(const std::regex& pattern, http::verb method, const std::vector<test_data>& data, bool does_validate_data);

TEST(PositiveTests, RegisterTest) {
  auto predict_regex = std::regex(
      R"(/v1/models(?:/([^/:]+))(?:/versions/(\d+))?:(classify|regress|predict))");
  Routes routes;
  EXPECT_TRUE(routes.register_controller(http::verb::post, predict_regex, do_something));

  auto status_regex = std::regex(
      R"(/v1/models(?:/([^/:]+))?(?:/versions/(\d+))?(?:\/(metadata))?)");
  EXPECT_TRUE(routes.register_controller(http::verb::get, status_regex, do_something));
}

TEST(PositiveTests, PostRouteTest) {
  auto predict_regex = std::regex(
      R"(/v1/models(?:/([^/:]+))(?:/versions/(\d+))?:(classify|regress|predict))");

  std::vector<test_data> actions{
      std::make_tuple(http::verb::post, "/v1/models/abc/versions/23:predict", "abc", "23", "predict", http::status::ok),
      std::make_tuple(http::verb::post, "/v1/models/abc:predict", "abc", "", "predict", http::status::ok),
      std::make_tuple(http::verb::post, "/v1/models/models/versions/45:predict", "models", "45", "predict", http::status::ok),
      std::make_tuple(http::verb::post, "/v1/models/versions/versions/45:predict", "versions", "45", "predict", http::status::ok)};

  run_route(predict_regex, http::verb::post, actions, true);
}

TEST(NegativeTests, PostRouteInvalidURLTest) {
  auto predict_regex = std::regex(
      R"(/v1/models(?:/([^/:]+))(?:/versions/(\d+))?:(classify|regress|predict))");

  std::vector<test_data> actions{
      std::make_tuple(http::verb::post, "/v1/models", "", "", "", http::status::not_found),
      std::make_tuple(http::verb::post, "/v1/models:predict", "", "", "", http::status::not_found),
      std::make_tuple(http::verb::post, "/v1/models:bar", "", "", "", http::status::not_found),
      std::make_tuple(http::verb::post, "/v1/models/abc/versions", "", "", "", http::status::not_found),
      std::make_tuple(http::verb::post, "/v1/models/abc/versions:predict", "", "", "", http::status::not_found),
      std::make_tuple(http::verb::post, "/v1/models/abc/versions/23:foo", "", "", "", http::status::not_found)};

  run_route(predict_regex, http::verb::post, actions, false);
}

TEST(NegativeTests, PostRouteInvalidMethodTest) {
  auto predict_regex = std::regex(
      R"(/v1/models(?:/([^/:]+))(?:/versions/(\d+))?:(classify|regress|predict))");

  std::vector<test_data> actions{
      std::make_tuple(http::verb::get, "/v1/models/abc/versions/23:predict", "abc", "23", "predict", http::status::method_not_allowed)};

  run_route(predict_regex, http::verb::post, actions, false);
}

void run_route(const std::regex& pattern, http::verb method, const std::vector<test_data>& data, bool does_validate_data) {
  Routes routes;
  EXPECT_TRUE(routes.register_controller(method, pattern, do_something));

  for (const auto& i : data) {
    http::verb test_method;
    std::string url_string;
    std::string name;
    std::string version;
    std::string action;
    handler_fn fn;

    std::string expected_name;
    std::string expected_version;
    std::string expected_action;
    http::status expected_status;

    std::tie(test_method, url_string, expected_name, expected_version, expected_action, expected_status) = i;
    EXPECT_EQ(expected_status, routes.parse_url(test_method, url_string, name, version, action, fn));
    if (does_validate_data) {
      EXPECT_EQ(name, expected_name);
      EXPECT_EQ(version, expected_version);
      EXPECT_EQ(action, expected_action);
    }
  }
}

} // namespace test
} // namespace onnxruntime