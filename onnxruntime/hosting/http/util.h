// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef BEAST_SERVER_UTIL_H
#define BEAST_SERVER_UTIL_H

#include <boost/beast/core.hpp>

namespace onnxruntime {
namespace hosting {

namespace beast = boost::beast;  // from <boost/beast.hpp>

// Report a failure
void ErrorHandling(beast::error_code ec, char const* what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

}  // namespace hosting
}  // namespace onnxruntime
#endif  //BEAST_SERVER_UTIL_H
