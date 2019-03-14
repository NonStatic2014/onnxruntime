// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef ONNXRUNTIME_HOSTING_SERVER_CONFIGURATION_H
#define ONNXRUNTIME_HOSTING_SERVER_CONFIGURATION_H

#include <thread>
#include <fstream>

#include "boost/program_options.hpp"

namespace onnxruntime {
namespace hosting {

namespace po = boost::program_options;

// Wrapper around Boost program_options and should provide all the functionality for options parsing
// Provides sane default values
class ServerConfiguration {
 public:
  ServerConfiguration() {
    desc.add_options()("help,h", "Shows a help message and exits");
    desc.add_options()("model_path,m", po::value(&model_path)->required(), "Path to ONNX model");
    desc.add_options()("address,a", po::value(&address)->default_value(address), "The base HTTP address");
    desc.add_options()("port,p", po::value(&port)->default_value(port), "HTTP port to listen to requests");
    desc.add_options()("threads,t", po::value(&threads)->default_value(threads), "Number of http threads");
  }

  void ParseInput(int ac, char** av) {
    try {
      po::store(po::command_line_parser(ac, av).options(desc).run(), vm);  // can throw

      if (ContainsHelp()) {
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

  bool ContainsHelp() const {
    return vm.count("help") || vm.count("h");
  }

  void PrintHelp(std::ostream& out, const std::string& what) const {
    out << what << std::endl
        << desc << std::endl;
  }

  const std::string full_desc = "ONNX Hosting: host an ONNX model for inferencing with ONNXRuntime";
  std::string model_path;
  std::string address = "0.0.0.0";
  int port = 8080;
  int threads = std::thread::hardware_concurrency();

 private:
  po::options_description desc{"Allowed options"};
  po::variables_map vm{};
};

}  // namespace hosting
}  // namespace onnxruntime

#endif  // ONNXRUNTIME_HOSTING_SERVER_CONFIGURATION_H
