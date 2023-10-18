#pragma once

#include "coordinator.h"
#include "utils.h"
#include <memory>
#include <ylt/coro_rpc/coro_rpc_client.hpp>

class Client {
public:
  Client(std::string ip, int port, std::string coordinator_ip,
         int coordinator_port);

  std::unique_ptr<coro_rpc::coro_rpc_client> &get_rpc_client();
  void connect_to_coordinator();
  void set_ec_parameter(EC_schema ec_schema);

  void set(std::string key, std::string value);
  std::string get(std::string key);

private:
  std::unique_ptr<coro_rpc::coro_rpc_client> rpc_client_{nullptr};
  asio::io_context io_context_;
  int port_for_transfer_data_;
  std::string ip_;
  std::string coordinator_ip_;
  int coordinator_port_;
  asio::ip::tcp::acceptor acceptor_;
};