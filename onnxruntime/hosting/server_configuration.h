// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef ONNXRUNTIME_HOSTING_SERVER_CONFIGURATION_H
#define ONNXRUNTIME_HOSTING_SERVER_CONFIGURATION_H

#include "boost/program_options.hpp"

namespace onnxruntime {
namespace hosting {

namespace po = boost::program_options;

class ServerConfiguration {
 public:
  ServerConfiguration() {
    full_desc = "ONNX Hosting: host an ONNX model for inferencing with ONNXRuntime";
    desc.add_options()("help,h", "Shows a help message and exits")("address,a", po::value(&address), "The base HTTP address")("port,p", po::value(&port), "HTTP port to listen to requests")("threads,t", po::value(&threads), "Number of http threads")("model_path,m", po::value(&model_path), "Path of the model file");
  }

  void ParseInput(int argc_, char** argv);

  std::string full_desc;
  std::string model_path;
  std::string address;
  int port;
  int threads;

 private:
  void PrintHelp(std::ostream& output, const std::string& what);

  po::options_description desc{"Allowed options"};
  po::variables_map vm;
};

}  // namespace hosting
}  // namespace onnxruntime

#endif  // ONNXRUNTIME_HOSTING_SERVER_CONFIGURATION_H
