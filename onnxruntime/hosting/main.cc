// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "core/session/inference_session.h"

#include "http_server.h"
#include "server_configuration.h"
#include "core/session/inference_session.h"

namespace beast = boost::beast;
namespace http = beast::http;

void test_request(const std::string& name, const std::string& version,
                  const std::string& action, onnxruntime::hosting::HttpContext& context) {
  std::stringstream ss;

  ss << "\tModel Name: " << name << std::endl;
  ss << "\tModel Version: " << version << std::endl;
  ss << "\tAction: " << action << std::endl;
  ss << "\tHTTP method: " << context.request.method() << std::endl;

  http::response<http::string_body>
      res{std::piecewise_construct, std::make_tuple(ss.str()), std::make_tuple(http::status::ok, context.request.version())};

  res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  res.set(http::field::content_type, "plain/text");
  res.keep_alive(context.request.keep_alive());
  context.response = res;
}

int main(int argc, char* argv[]) {
  onnxruntime::hosting::ServerConfiguration config{};
  config.ParseInput(argc, argv);

  // onnxruntime::SessionOptions options {};
  // onnxruntime::InferenceSession session(options);

  auto const boost_address = boost::asio::ip::make_address(config.address);

  onnxruntime::hosting::App app{};
  app.Post(R"(/v1/models/([^/:]+)(?:/versions/(\d+))?:(classify|regress|predict))", test_request)
     .Bind(boost_address, config.port)
     .NumThreads(config.threads)
     .Run();

  return EXIT_SUCCESS;
}
