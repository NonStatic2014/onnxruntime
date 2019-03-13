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
    desc.add_options()("help,h", "Shows a help message and exits");
    desc.add_options()("address,a", po::value(&address), "The base HTTP address");
    desc.add_options()("port,p", po::value(&port), "HTTP port to listen to requests");
    desc.add_options()("threads,t", po::value(&threads), "Number of http threads");
    desc.add_options()("model_path,m", po::value(&model_path), "Path of the model file");
  }

  void ParseInput(int argc_, char** argv_) {
    try {
      po::store(po::command_line_parser(argc_, argv_).options(desc).run(), vm);  // can throw

      if (vm.count("help") || vm.count("h")) {
        PrintHelp(std::cout, full_desc);
        return exit(EXIT_SUCCESS);
      }

      po::notify(vm);  // throws on error, so do after help
    } catch (const po::error& e) {
      PrintHelp(std::cerr, e.what());
      return exit(EXIT_FAILURE);
    } catch (const std::exception& e) {
      PrintHelp(std::cerr, e.what());
      return exit(EXIT_FAILURE);
    }
  }

  std::string full_desc;
  std::string model_path;
  std::string address;
  int port;
  int threads;

 private:
  void PrintHelp(std::ostream& out, const std::string& what) {
    out << what << std::endl
        << desc << std::endl;
  }

  po::options_description desc{"Allowed options"};
  po::variables_map vm{};
};

}  // namespace hosting
}  // namespace onnxruntime

#endif  // ONNXRUNTIME_HOSTING_SERVER_CONFIGURATION_H
