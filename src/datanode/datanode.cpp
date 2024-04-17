#include "../../include/datanode.h"
#include "../../include/nlohmann/json.hpp"
#include <fstream>
#include <string>
#include <unordered_map>
using json = nlohmann::json;


/*# "../jsoncpp.cpp"*/
Datanode::Datanode(std::string ip, int port)
    : ip_(ip), port_(port),
      acceptor_(io_context_,
                asio::ip::tcp::endpoint(
                    asio::ip::address::from_string(ip.c_str()), port)) {
  // port是datanode的地址,port + 1000是redis的地址
  std::string url = "tcp://" + ip_ + ":" + std::to_string(port_ + 1000);
  redis_ = std::make_unique<sw::redis::Redis>(url);
  filename="/home/kvgroup/cxm/cache/happylrc/jsonout/"+ ip_ + std::to_string(port_) +"data.json";
}

Datanode::~Datanode() { acceptor_.close(); }

void Datanode::keep_working() {
  for (;;) {
    asio::ip::tcp::socket peer(io_context_);
    acceptor_.accept(peer);

    std::vector<unsigned char> flag_buf(sizeof(int));
    asio::read(peer, asio::buffer(flag_buf, flag_buf.size()));
    int flag = bytes_to_int(flag_buf);

    if (flag == 0) {
      std::vector<unsigned char> value_or_key_size_buf(sizeof(int));

      asio::read(peer, asio::buffer(value_or_key_size_buf,
                                    value_or_key_size_buf.size()));
      int key_size = bytes_to_int(value_or_key_size_buf);

      asio::read(peer, asio::buffer(value_or_key_size_buf,
                                    value_or_key_size_buf.size()));
      int value_size = bytes_to_int(value_or_key_size_buf);

      std::string key_buf(key_size, 0);
      std::string value_buf(value_size, 0);
      asio::read(peer, asio::buffer(key_buf.data(), key_buf.size()));
      asio::read(peer, asio::buffer(value_buf.data(), value_buf.size()));

      redis_->set(key_buf, value_buf);

      std::vector<char> finish(1);
      asio::write(peer, asio::buffer(finish, finish.size()));

      asio::error_code ignore_ec;
      peer.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      peer.close(ignore_ec);
    } else if(flag==2){
      std::vector<unsigned char> value_or_key_size_buf(sizeof(int));

      asio::read(peer, asio::buffer(value_or_key_size_buf,
                                    value_or_key_size_buf.size()));
      int key_size = bytes_to_int(value_or_key_size_buf);

      asio::read(peer, asio::buffer(value_or_key_size_buf,
                                    value_or_key_size_buf.size()));
      int value_size = bytes_to_int(value_or_key_size_buf);

      std::string key_buf(key_size, 0);
      std::string value_buf(value_size, 0);
      asio::read(peer, asio::buffer(key_buf.data(), key_buf.size()));
      asio::read(peer, asio::buffer(value_buf.data(), value_buf.size()));

      write_key_value_to_disk(key_buf, value_buf);

      std::vector<char> finish(1);
      asio::write(peer, asio::buffer(finish, finish.size()));

      asio::error_code ignore_ec;
      peer.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      peer.close(ignore_ec);

    }else if (flag==3){
      std::vector<unsigned char> key_size_buf(sizeof(int));
      asio::read(peer, asio::buffer(key_size_buf, key_size_buf.size()));
      int key_size = bytes_to_int(key_size_buf);

      std::string key_buf(key_size, 0);
      asio::read(peer, asio::buffer(key_buf.data(), key_buf.size()));

      /*auto value_returned = redis_->get(key_buf);
      my_assert(value_returned.has_value());
      std::string value = value_returned.value();*/
      std::string value = find_value_by_key_from_disk(key_buf);




      asio::write(peer, asio::buffer(value.data(), value.length()));

      asio::error_code ignore_ec;
      peer.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      peer.close(ignore_ec);
    } else {
      std::vector<unsigned char> key_size_buf(sizeof(int));
      asio::read(peer, asio::buffer(key_size_buf, key_size_buf.size()));
      int key_size = bytes_to_int(key_size_buf);

      std::string key_buf(key_size, 0);
      asio::read(peer, asio::buffer(key_buf.data(), key_buf.size()));

      auto value_returned = redis_->get(key_buf);
      my_assert(value_returned.has_value());
      std::string value = value_returned.value();

      asio::write(peer, asio::buffer(value.data(), value.length()));

      asio::error_code ignore_ec;
      peer.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      peer.close(ignore_ec);
    }
  }
}

bool Datanode::write_key_value_to_disk(const std::string& key, const std::string& value){
  /*
  //方法一：写普通文件，逗号分隔
  std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
      std::cerr << "Error opening file!" << std::endl;
      return false;
    }

    file << key << "," << value << std::endl;
    file.close();

    return true;
  */ 

  
  //方法二：写普通文件，key.size()分隔
  std::ofstream file(filename, std::ios::app);
  if (file.is_open()) {
    file.write(key.c_str(), key.size());
    file.write(value.c_str(), value.size());
    file.write("\n", 1 );
    file.close();
    return true;
  } else {
    return false;
  }
  
  
  /*
  //方法三：使用nlohmann的json库[头文件修改"#include ../../include/nlohmann/json.hpp" & using json = nlohmann::json;]
  //方法三可以避免字符被转义，但是读取卡住
  try {
    // 构造 JSON 对象
    json json_obj;
    // 将包含特殊字符的字符串设置为未转义的 JSON 字符串
    //json_obj[key] = json::parse("\"" + value + "\"");
    json_obj[key] = json::parse("\"" + value + "\"");

    // 序列化 JSON 对象为字符串
    std::string json_str = json_obj.dump() + "\n";

    // 尝试以追加写入模式打开文件
    std::ofstream outfile(filename, std::ios::app);
    if (outfile.is_open()) {
        outfile << json_str;
        outfile.close();
        return true; // 文件写入成功
    }
    else {
        std::cerr << "Error: Unable to open file " << filename << " for writing." << std::endl;
        return false; // 文件打开失败
    }
  }catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return false; // 文件写入过程中出现异常
  }
  */

  /*
  //方法四：使用传统json库[头文件修改#include "../../include/json/json.h" & #include "../jsoncpp.cpp"]
  //方法四读写顺利，但是字符会被转义导致读取和原始数据不一致
  Json::Value root;

  root[key] = value;

  // 尝试以追加写入模式打开文件
  Json::StreamWriterBuilder builder;
  builder["indentation"] = ""; // 禁用缩进，以确保每次写入一行
  std::string json_str = Json::writeString(builder, root) + "\n"; // 添加换行符
  try {
    std::ofstream outfile(filename, std::ios::app);
    if (outfile.is_open()) {
      outfile << json_str;
      outfile.close();
      return true; // 文件写入成功
    } else {
      std::cerr << "Error: Unable to open file " << filename << " for writing." << std::endl;
      return false; // 文件打开失败
    }
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return false; // 文件写入过程中出现异常
  }
  */
  
  

}


std::string Datanode::find_value_by_key_from_disk(const std::string& key) {
  /*
  //方法一：写普通文件，逗号分隔
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error opening file!" << std::endl;
    return "";
  }

  std::string line;
  while (std::getline(file, line)) {
    std::stringstream ss(line);
    std::string stored_key, value;
    if (std::getline(ss, stored_key, ',') && std::getline(ss, value)) {
      if (stored_key == key) {
        file.close();
        return value;
      }
    }
  }
  file.close();
  return ""; // Key not found
  */

  
  //方法二：写普通文件，key.size()分隔
  std::ifstream file(filename);
  if (file.is_open()) {
    std::string line, k, value;
    while (std::getline(file, line)) {
      k = line.substr(0, key.size());
      if (k == key) {
      value = line.substr(key.size());
      break;
      }
    }
    return value;
  } else {
    return "";
  }
  
  
  /*
  //方法三：使用使用nlohmann的json库[头文件修改"../../include/nlohmann/json.hpp" & using json = nlohmann::json;]
  std::ifstream infile(filename);
  if (!infile.is_open()) {
    std::cerr << "Error: Unable to open file " << filename << " for reading." << std::endl;
    return ""; // 文件打开失败，返回空字符串
  }
  
  std::string line;
  while (std::getline(infile, line)) {
     try {
            json root = json::parse(line);
            if (root.contains(key)) {
                infile.close();
                return root[key].get<std::string>();
            }
        } catch (const std::exception& e) {
            // 解析失败
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            continue;
        }
  }
  infile.close();
  return ""; // 指定的键不存在，返回空字符串
  */
  

  /*
  //方法四：使用传统json库[头文件修改#include "../../include/json/json.h" & #include "../jsoncpp.cpp"]
  //std::lock_guard<std::mutex> lock(file_read_mutex);
  std::ifstream infile(filename);
  
  if (!infile.is_open()) {
    std::cerr << "Error: Unable to open file " << filename << " for reading." << std::endl;
    return ""; // 文件打开失败，返回空字符串
  }

  std::string line;
  while (std::getline(infile, line)) {
    Json::Value root;
    Json::Reader reader;
    if (reader.parse(line, root)) {
      if (root.isMember(key)) {
        infile.close();
        return root[key].asString();
      }
    } else {
      std::cerr << "Error: Failed to parse JSON from line: " << line << std::endl;
    }
  }

  infile.close();
  std::cerr << "Error: Key '" << key << "' not found." << std::endl;
  return ""; // 指定的键不存在，返回空字符串
  */
  
  
}









//写入单个键值对数据到磁盘//
/*bool Datanode::write_key_value_to_disk(const std::string& key, const std::string& value){
  // 构造JSON对象
  // 使用互斥量锁定文件写入操作 应该不需要？一个块写一个node 不冲突
  std::lock_guard<std::mutex> lock(file_write_mutex);

  json data;
  data[key] = value;

  // 打开JSON文件
  std::ofstream file(filename, std::ios_base::app); // 使用追加模式 

  // 将JSON对象写入文件
  file << data<< std::endl;

  // 关闭文件
  file.close();
  return true;

  //std::cout << "Data written to data.json" << std::endl;

}*/