#pragma once

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <source_location>
#include <string>
#include <unordered_set>

#define COORDINATOR_RPC_PORT 11111
#define CLIENT_TRANSFER_DATA_PORT 22222

#define my_assert(condition)                                                   \
  exit_when((condition), std::source_location::current())

enum class Encode_Type { RS, OPPO_LRC, Azure_LRC, Azure_LRC_1 };

enum class Placement_Type { random, flat, strategy1 };

typedef struct {
  bool partial_decoding;
  Encode_Type encode_type;
  Placement_Type placement_type;
  int k;      // num_of_data_block
  int real_l; // num_of_local_parity_block, 不包括Azure_LRC_1中的“1”
  int g;      // num_of_global_parity_block
  int b;      // num_of_data_block_per_group
  size_t strip_size_upper;
} EC_schema;

typedef struct {
  size_t value_len;
  std::vector<unsigned int> stripes;
} meta_info_of_object;

typedef struct {
  unsigned int stripe_id;
  Encode_Type encode_type;
  Placement_Type placement_type;
  int k;
  int real_l;
  int g;
  int b;
  size_t block_size;
  std::vector<unsigned int> nodes;
} stripe_item;

typedef struct {
  unsigned int node_id;
  std::string ip;
  int port;
} node_item;

typedef struct {
  unsigned int cluster_id;
  std::string proxy_ip;
  int proxy_port;
  std::vector<unsigned int> nodes;
} cluster_item;

typedef struct {
  Encode_Type encode_type;
  std::vector<unsigned int> stripe_ids;
  std::string key;
  int k;
  int real_l;
  int g;
  int b;
  size_t value_len;
  size_t block_size;
  size_t tail_block_size;
  std::vector<std::pair<std::string, int>> datanode_ip_port;

  std::string client_ip;
  int client_port;
} placement_info;

// 生成随机字符串
std::string generate_random_string(int length);

// 生成不重复的随机字符串对
std::pair<std::string, std::string>
generate_unique_random_strings(int key_length, int value_length);

// 生成某个数组或容器的随机索引
int random_index(size_t len);

void exit_when(bool condition, const std::source_location &location);