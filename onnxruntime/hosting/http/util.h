// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef ONNXRUNTIME_HOSTING_HTTP_UTIL_H
#define ONNXRUNTIME_HOSTING_HTTP_UTIL_H

#include <boost/beast/core.hpp>
#include <boost/beast/http/status.hpp>
#include <google/protobuf/stubs/status.h>

namespace onnxruntime {
namespace hosting {

namespace beast = boost::beast;  // from <boost/beast.hpp>

// Report a failure
void ErrorHandling(beast::error_code ec, char const* what);

// Mapping protobuf status to http status
boost::beast::http::status GetHttpStatusCode(google::protobuf::util::Status status);
}  // namespace hosting
}  // namespace onnxruntime

#endif  // ONNXRUNTIME_HOSTING_HTTP_UTIL_H
