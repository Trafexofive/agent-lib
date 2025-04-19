#include "inc/Tool.hpp"
#include <fstream>
#include <string>

std::string readFile(const Json::Value& params) {
  if (!params.isMember("filepath") || !params["filepath"].isString()) {
    return "Error: 'filepath' parameter is missing or not a string.";
  }
  std::string filepath = params["filepath"].asString();
  std::ifstream file(filepath);
  if (file.is_open()) {
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    return content;
  } else {
    return "Error: Could not open file.";
  }
}

int main(){
    return 0;
}
EOF 2>&1
