// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef ONNXRUNTIME_HOSTING_SERVER_CONFIGURATION_H
#define ONNXRUNTIME_HOSTING_SERVER_CONFIGURATION_H

namespace onnxruntime {
namespace hosting {

class ServerConfiguration {
 public:
  ServerConfiguration() = default;

  std::string model_path;
  std::string address;
  int port;
  int threads;
};

}  // namespace hosting
}  // namespace onnxruntime

#endif  // ONNXRUNTIME_HOSTING_SERVER_CONFIGURATION_H
