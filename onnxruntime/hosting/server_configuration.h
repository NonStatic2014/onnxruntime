// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef ONNXRUNTIME_HOSTING_SERVER_CONFIGURATION_H
#define ONNXRUNTIME_HOSTING_SERVER_CONFIGURATION_H

#include <thread>

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
    desc.add_options()("http_port", po::value(&http_port)->default_value(http_port), "HTTP port to listen to requests");
    desc.add_options()("num_http_threads", po::value(&num_http_threads)->default_value(num_http_threads), "Number of http threads");
  }

  // Parses argc and argv and sets the values for the class
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

    ValidateOptions();
  }

  const std::string full_desc = "ONNX Hosting: host an ONNX model for inferencing with ONNXRuntime";
  std::string model_path;
  std::string address = "0.0.0.0";
  int http_port = 5000;
  int num_http_threads = std::thread::hardware_concurrency();

 private:
  // Print help and exit if there is a bad value
  void ValidateOptions() {
    if (num_http_threads <= 0) {
      PrintHelp(std::cerr, "num_http_threads must be greater than 0");
      return exit(EXIT_FAILURE);
    } else if (http_port < 0 || http_port > 65535) {
      PrintHelp(std::cerr, "http_port input invalid");
      return exit(EXIT_FAILURE);
    }
  }

  bool ContainsHelp() const {
    return vm.count("help") || vm.count("h");
  }

  // Prints a helpful message (param: what) to the user and then the program options
  // Example: config.PrintHelp(std::cout, "Non-negative values not allowed")
  // Which will print that message and then all publicly available options
  void PrintHelp(std::ostream& out, const std::string& what) const {
    out << what << std::endl
        << desc << std::endl;
  }

  po::options_description desc{"Allowed options"};
  po::variables_map vm{};
};

}  // namespace hosting
}  // namespace onnxruntime

#endif  // ONNXRUNTIME_HOSTING_SERVER_CONFIGURATION_H
