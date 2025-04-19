#include "json/json.h" // For parsing/generating JSON
#include <cstdlib>     // For std::rand, std::srand
#include <ctime>       // For std::time, std::strftime, std::localtime
#include <iostream>    // For std::cout (optional debugging)
#include <sstream>     // For std::ostringstream, std::istringstream
#include <stdexcept>   // For potential exceptions (though not used here)
#include <string>

// Gets current time (params ignored). Returns JSON string.
std::string getTime(const Json::Value &params) {
  (void)params; // Unused
  Json::Value response;
  try {
    time_t now = time(0);
    char buf[80];
    if (strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now))) {
      response["status"] = "success";
      response["result"] = buf;
    } else {
      response["status"] = "error";
      response["result"] = "Error formatting time.";
    }
  } catch (...) {
      response["status"] = "error";
      response["result"] = "Unknown error getting time.";
  }
  return response.toStyledString();
}

// Rolls dice (uses params). Returns JSON string.
std::string rollDice(const Json::Value &params) {
  Json::Value response;
  int sides = 6; // Default
  int rolls = 1; // Default

  try {
      if (params.isMember("sides") && params["sides"].isInt()) {
        int requestedSides = params["sides"].asInt();
        if (requestedSides > 0) {
          sides = requestedSides;
        } else {
            response["status"] = "error";
            response["result"] = "Error: 'sides' parameter must be positive.";
            return response.toStyledString();
        }
      }
      if (params.isMember("rolls") && params["rolls"].isInt()) {
          int requestedRolls = params["rolls"].asInt();
          if (requestedRolls > 0 && requestedRolls <= 100) { // Add a reasonable limit
              rolls = requestedRolls;
          } else {
              response["status"] = "error";
              response["result"] = "Error: 'rolls' parameter must be between 1 and 100.";
              return response.toStyledString();
          }
      }

      std::ostringstream oss;
      oss << "Rolled " << rolls << " d" << sides << ":";
      for (int i = 0; i < rolls; ++i) {
          int roll = (std::rand() % sides) + 1;
          oss << " " << roll;
      }

      response["status"] = "success";
      response["result"] = oss.str();

  } catch (const Json::Exception& e) {
      response["status"] = "error";
      response["result"] = "Error processing parameters: " + std::string(e.what());
  } catch (...) {
      response["status"] = "error";
      response["result"] = "Unknown error rolling dice.";
  }
  return response.toStyledString();
}

// Calculates a simple expression (uses params). Returns JSON string.
std::string calculate(const Json::Value &params) {
  Json::Value response;

  if (!params.isObject() || !params.isMember("expression") || !params["expression"].isString()) {
    response["status"] = "error";
    response["result"] = "Error: Missing or invalid 'expression' string parameter.";
    return response.toStyledString();
  }

  std::string expression = params["expression"].asString();
  if (expression.empty()) {
      response["status"] = "error";
      response["result"] = "Error: 'expression' cannot be empty.";
      return response.toStyledString();
  }

  // Extremely basic parser: only handles 'number op number'
  std::istringstream iss(expression);
  double a = 0.0, b = 0.0, result = 0.0;
  char op = 0;

  // Attempt to parse
  // Note: This doesn't handle whitespace robustly or operator precedence.
  // A real calculator needs a proper parser (e.g., shunting-yard algorithm).
  iss >> a >> op >> b;

  // Check if parsing failed or if there's leftover input
  if (iss.fail() || !iss.eof()) {
       // Try a simpler version: just check for single number
       iss.clear();
       iss.seekg(0);
       if (iss >> a && iss.eof()) { // If it's just a number, return it
            op = '='; // Treat as a single number expression
       } else {
          response["status"] = "error";
          response["result"] = "Error: Could not parse expression. Use format: 'number operator number' (e.g., '5 + 3'). Simple evaluation only.";
          return response.toStyledString();
       }
  }

  try {
      bool error = false;
      switch (op) {
      case '+': result = a + b; break;
      case '-': result = a - b; break;
      case '*': result = a * b; break;
      case '/':
        if (b != 0.0) {
          result = a / b;
        } else {
          response["status"] = "error";
          response["result"] = "Error: Division by zero.";
          error = true;
        }
        break;
      case '=': // Case for single number
           result = a;
           break;
      default:
        response["status"] = "error";
        response["result"] = "Error: Invalid or unsupported operator. Supported: +, -, *, /.";
        error = true;
        break;
      }

      if (!error) {
        response["status"] = "success";
        // Format result nicely
        std::ostringstream res_oss;
        res_oss << result;
        response["result"] = res_oss.str();
      }
  } catch (...) {
      response["status"] = "error";
      response["result"] = "Unknown error during calculation.";
  }

  return response.toStyledString();
}
