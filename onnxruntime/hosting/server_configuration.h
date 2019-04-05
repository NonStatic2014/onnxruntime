// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef ONNXRUNTIME_HOSTING_SERVER_CONFIGURATION_H
#define ONNXRUNTIME_HOSTING_SERVER_CONFIGURATION_H

#include <thread>
#include <fstream>

#include "boost/program_options.hpp"

#include "core/common/logging/logging.h"

namespace onnxruntime {
namespace hosting {

enum class Result { ExitSuccess = 1,
                    ExitFailure,
                    ContinueSuccess };

namespace po = boost::program_options;

// Wrapper around Boost program_options and should provide all the functionality for options parsing
// Provides sane default values
class ServerConfiguration {
 public:
  const std::string full_desc = "ONNX Hosting: host an ONNX model for inferencing with ONNXRuntime";
  std::string model_path;
  std::string address = "0.0.0.0";
  int http_port = 8001;
  int num_http_threads = std::thread::hardware_concurrency();
  onnxruntime::logging::Severity logging_level{};

  ServerConfiguration() {
    desc.add_options()("help,h", "Shows a help message and exits");
    desc.add_options()("logging_level", po::value(&logging_level_str)->default_value(logging_level_str), "Logging level. Allowed options (case sensitive): verbose, info, warning, error, fatal");
    desc.add_options()("model_path,m", po::value(&model_path)->required(), "Path to ONNX model");
    desc.add_options()("address,a", po::value(&address)->default_value(address), "The base HTTP address");
    desc.add_options()("http_port", po::value(&http_port)->default_value(http_port), "HTTP port to listen to requests");
    desc.add_options()("num_http_threads", po::value(&num_http_threads)->default_value(num_http_threads), "Number of http threads");
  }

  // Parses argc and argv and sets the values for the class
  // Returns an enum with three options: ExitSuccess, ExitFailure, ContinueSuccess
  // ExitSuccess and ExitFailure means the program should exit but is left to the caller
  Result ParseInput(int ac, char** av) {
    try {
      po::store(po::command_line_parser(ac, av).options(desc).run(), vm);  // can throw

      if (ContainsHelp()) {
        PrintHelp(std::cout, full_desc);
        return Result::ExitSuccess;
      }

      po::notify(vm);  // throws on error, so do after help
    } catch (const po::error& e) {
      PrintHelp(std::cerr, e.what());
      return Result::ExitFailure;
    } catch (const std::exception& e) {
      PrintHelp(std::cerr, e.what());
      return Result::ExitFailure;
    }

    logging_level = GetSeverity(logging_level_str);
    return ValidateOptions();
  }

 private:
  po::options_description desc{"Allowed options"};
  po::variables_map vm{};
  std::string logging_level_str = "verbose";

  // Print help and return if there is a bad value
  Result ValidateOptions() {
    if (vm.count("logging_level") &&
        (!(logging_level_str == "verbose" || logging_level_str == "info") || logging_level_str == "warning" || logging_level_str == "error" || logging_level_str == "fatal")) {
      PrintHelp(std::cerr, "logging_level must be one of verbose, info, warning, error, or fatal");
      return Result::ExitFailure;
    } else if (num_http_threads <= 0) {
      PrintHelp(std::cerr, "num_http_threads must be greater than 0");
      return Result::ExitFailure;
    } else if (http_port < 0 || http_port > 65535) {
      PrintHelp(std::cerr, "http_port input invalid");
      return Result::ExitFailure;
    } else if (!file_exists(model_path)) {
      PrintHelp(std::cerr, "model_path must be the location of a valid file");
      return Result::ExitFailure;
    } else {
      return Result::ContinueSuccess;
    }
  }

  // Checks if program options contains help
  bool ContainsHelp() const {
    return vm.count("help") || vm.count("h");
  }

  onnxruntime::logging::Severity GetSeverity(const std::string& level) const {
    if (level == "verbose") return onnxruntime::logging::Severity::kVERBOSE;
    if (level == "info") return onnxruntime::logging::Severity::kINFO;
    if (level == "warning") return onnxruntime::logging::Severity::kWARNING;
    if (level == "error") return onnxruntime::logging::Severity::kERROR;
    if (level == "fatal") return onnxruntime::logging::Severity::kFATAL;
    return onnxruntime::logging::Severity::kVERBOSE;
  }

  // Prints a helpful message (param: what) to the user and then the program options
  // Example: config.PrintHelp(std::cout, "Non-negative values not allowed")
  // Which will print that message and then all publicly available options
  void PrintHelp(std::ostream& out, const std::string& what) const {
    out << what << std::endl
        << desc << std::endl;
  }

  inline bool file_exists(const std::string& fileName) {
    std::ifstream infile(fileName.c_str());
    return infile.good();
  }
};

}  // namespace hosting
}  // namespace onnxruntime

#endif  // ONNXRUNTIME_HOSTING_SERVER_CONFIGURATION_H