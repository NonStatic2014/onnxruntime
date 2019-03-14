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
      const_cast<char*>("--model_path"), const_cast<char*>("/another/path"),
      const_cast<char*>("--address"), const_cast<char*>("4.4.4.4"),
      const_cast<char*>("--port"), const_cast<char*>("80"),
      const_cast<char*>("--threads"), const_cast<char*>("1")};

  onnxruntime::hosting::ServerConfiguration config{};
  config.ParseInput(9, test_argv);
  EXPECT_EQ(config.model_path, "/another/path");
  EXPECT_EQ(config.address, "4.4.4.4");
  EXPECT_EQ(config.port, 80);
  EXPECT_EQ(config.threads, 1);
}

TEST(PositiveTests, ConfigParsingShortArgs) {
  char* test_argv[] = {
      const_cast<char*>("/path/to/binary"),
      const_cast<char*>("-m"), const_cast<char*>("/model/path"),
      const_cast<char*>("-a"), const_cast<char*>("4.4.4.4"),
      const_cast<char*>("-p"), const_cast<char*>("5000"),
      const_cast<char*>("-t"), const_cast<char*>("2")};

  onnxruntime::hosting::ServerConfiguration config{};
  config.ParseInput(9, test_argv);
  EXPECT_EQ(config.model_path, "/model/path");
  EXPECT_EQ(config.address, "4.4.4.4");
  EXPECT_EQ(config.port, 5000);
  EXPECT_EQ(config.threads, 2);
}

TEST(PositiveTests, ConfigParsingDefaults) {
  char* test_argv[] = {
      const_cast<char*>("/path/to/binary"),
      const_cast<char*>("-m"), const_cast<char*>("/model/path"),
      const_cast<char*>("-t"), const_cast<char*>("3")};

  onnxruntime::hosting::ServerConfiguration config{};
  config.ParseInput(5, test_argv);
  EXPECT_EQ(config.model_path, "/model/path");
  EXPECT_EQ(config.address, "0.0.0.0");
  EXPECT_EQ(config.port, 8080);
  EXPECT_EQ(config.threads, 3);
}

}  // namespace test
}  // namespace hosting
}  // namespace onnxruntime