// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <fstream>

#include "gtest/gtest.h"
#include "hosting/server_configuration.h"

namespace onnxruntime {
namespace hosting {
namespace test {

TEST(PositiveTests, ConfigParsingFullArgs) {
  char* test_argv[] = {
      const_cast<char*>("/path/to/binary"),
      const_cast<char*>("--address"), const_cast<char*>("0.0.0.0"),
      const_cast<char*>("--port"), const_cast<char*>("80"),
      const_cast<char*>("--threads"), const_cast<char*>("1")};

  onnxruntime::hosting::ServerConfiguration config{};
  config.ParseInput(7, test_argv);
  EXPECT_EQ(config.address, "0.0.0.0");
  EXPECT_EQ(config.port, 80);
  EXPECT_EQ(config.threads, 1);
}

TEST(PositiveTests, ConfigParsingShortArgs) {
  char* test_argv[] = {
      const_cast<char*>("/path/to/binary"),
      const_cast<char*>("-a"), const_cast<char*>("0.0.0.0"),
      const_cast<char*>("-p"), const_cast<char*>("5000"),
      const_cast<char*>("-t"), const_cast<char*>("2")};

  onnxruntime::hosting::ServerConfiguration config{};
  config.ParseInput(7, test_argv);
  EXPECT_EQ(config.address, "0.0.0.0");
  EXPECT_EQ(config.port, 5000);
  EXPECT_EQ(config.threads, 2);
}

TEST(PositiveTests, ConfigParsingHelp) {
  std::ostringstream out;
  std::streambuf* coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(out.rdbuf());

  char* test_argv[] = {
      const_cast<char*>("/path/to/binary"),
      const_cast<char*>("-h")};

  onnxruntime::hosting::ServerConfiguration config{};
  config.ParseInput(2, test_argv);
  EXPECT_EQ(config.address, "");

  std::string help_output = "ONNX Hosting: host an ONNX model for inferencing with ONNXRuntime\n"
                            "Allowed options:\n"
                            "  -h [ --help ]           Shows a help message and exits\n"
                            "  -a [ --address ] arg    The base HTTP address\n"
                            "  -p [ --port ] arg       HTTP port to listen to requests\n"
                            "  -t [ --threads ] arg    Number of http threads\n"
                            "  -m [ --model_path ] arg Path of the model file";
  EXPECT_EQ(out.str(), help_output);

  std::cout.rdbuf(coutbuf);

  std::cout << "@@@@@@ " << std::endl;
}
}  // namespace test
}  // namespace hosting
}  // namespace onnxruntime