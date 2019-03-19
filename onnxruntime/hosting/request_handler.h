// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "http_server.h"

namespace onnxruntime {
namespace hosting {

namespace beast = boost::beast;
namespace http = beast::http;

auto test_request = [](const std::string& name,
                       const std::string& version,
                       const std::string& action,
                       HttpContext& context,
                       HostingEnvironment& env) {
  std::stringstream ss;
  auto logger = env.GetLogger();

  std::string body = context.request.body();

  ss << "\tModel Name: " << name << std::endl;
  ss << "\tModel Version: " << version << std::endl;
  ss << "\tAction: " << action << std::endl;
  ss << "\tHTTP method: " << context.request.method() << std::endl;
  ss << "\tJSON body: " << body << std::endl;

  http::response<http::string_body> res{std::piecewise_construct,
                                        std::make_tuple(ss.str()), std::make_tuple(http::status::ok, context.request.version())};
  res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  res.set(http::field::content_type, "plain/text");
  res.keep_alive(context.request.keep_alive());
  context.response = res;
};

}  // namespace hosting
}  // namespace onnxruntime
