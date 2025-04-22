
#include "json/json.h" // For parsing the JSON response
#include <cstdlib>     // For std::getenv
#include <curl/curl.h> // For making HTTP requests
#include <iostream>
#include <memory> // For unique_ptr
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Gets current time (params ignored)
std::string getTime(const Json::Value &params) {
  (void)params; // Unused
  time_t now = time(0);
  char buf[80];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
  return buf;
}

// Rolls a die (uses params)
std::string rollDice(const Json::Value &params) {
  int sides = 6; // Default
  int rolls = 1; // Default
  if (params.isMember("sides") && params["sides"].isInt()) {
    int requestedSides = params["sides"].asInt();
    if (requestedSides > 0) {
      sides = requestedSides;
    }
  }
  std::ostringstream oss;
  if (params.isMember("rolls") && params["rolls"].isInt()) {
    rolls = params["rolls"].asInt();
    while (rolls > 0) {
      int roll = (rand() % sides) + 1;
      oss << "Rolled a " << roll << " (d" << sides << ")";
      rolls--;
    }
  }
  return oss.str();
}

std::string calculate(const Json::Value &params) {
  if (params == Json::nullValue) {
    return "No parameters provided.";
  } else if (params.isMember("expression")) {
    std::string expression = params["expression"].asString();
    // Simple parsing and evaluation logic (for demonstration)
    // In a real scenario, you would use a proper expression parser
    std::istringstream iss(expression);
    double a, b;
    char op;
    iss >> a >> op >> b;
    double result = 0.0;
    switch (op) {
    case '+':
      result = a + b;
      break;
    case '-':
      result = a - b;
      break;
    case '*':
      result = a * b;
      break;
    case '/':
      result = (b != 0) ? a / b : 0;
      break;
    default:
      return "Invalid operator.";
    }
    return "Result: " + std::to_string(result);
  }
  return "Invalid parameters.";
}
