// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "boost/program_options.hpp"
#include "server_configuration.h"

namespace onnxruntime {
namespace hosting {

int ServerConfiguration::square(int x) {
    return x * x;
}

}  // namespace hosting
}  // namespace onnxruntime
