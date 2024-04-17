#pragma once

#include "asio.hpp"
#include "utils.h"
#include <string>
#include <sw/redis++/redis++.h>

class Datanode {
public:
  Datanode(std::string ip, int port);
  ~Datanode();
  void keep_working();
  bool write_key_value_to_disk(const std::string& key, const std::string& value);
  std::string find_value_by_key_from_disk(const std::string& key);
  std::string filename;

private:
  std::string ip_;
  int port_;
  asio::io_context io_context_{};
  asio::ip::tcp::acceptor acceptor_;
  std::unique_ptr<sw::redis::Redis> redis_{nullptr};
  std::mutex file_write_mutex;
  std::mutex file_read_mutex;
  
};