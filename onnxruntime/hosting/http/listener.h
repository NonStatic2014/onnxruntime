// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef ONNXRUNTIME_HOSTING_HTTP_LISTENER_H
#define ONNXRUNTIME_HOSTING_HTTP_LISTENER_H

#include <memory>

#include <boost/asio/ip/tcp.hpp>

#include "routes.h"
#include "util.h"

namespace onnxruntime {
namespace hosting {

namespace net = boost::asio;       // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

// Listens on a socket and creates an HTTP session
class Listener : public std::enable_shared_from_this<Listener> {
  const std::shared_ptr<Routes> routes_;
  const std::shared_ptr<HostingEnvironment> env_;
  tcp::acceptor acceptor_;
  tcp::socket socket_;

 public:
  Listener(std::shared_ptr<Routes> routes, std::shared_ptr<HostingEnvironment> env, net::io_context& ioc, const tcp::endpoint& endpoint);

  // Start accepting incoming connections
  void Run();

  // Asynchronously accepts the socket
  void DoAccept();

  // Creates the HTTP session and runs it
  void OnAccept(beast::error_code ec);
};

}  // namespace hosting
}  // namespace onnxruntime

#endif  // ONNXRUNTIME_HOSTING_HTTP_LISTENER_H
