#include "../../inc/Agent.hpp"
#include "../../inc/MiniGemini.hpp"
#include "../../inc/Tool.hpp"
#include <algorithm> // For std::find_if, std::remove_if
#include <cctype>    // For std::toupper
#include <chrono>    // For timestamp
#include <cstdio>    // For FILE, fgets
#include <cstdlib>   // For popen, pclose, system
#include <ctime>
#include <fstream> // For file operations
#include <iomanip> // For formatting time
#include <iostream>
#include <json/json.h>
#include <sstream>
#include <stdexcept>

#include <string>
#include <sys/stat.h> // For file operations
#include <unistd.h>   // For getcwd
                      
// Helper function to parse the structured LLM response
bool Agent::parseStructuredLLMResponse(const std::string &jsonString,
                                       std::string &thought,
                                       std::vector<ToolCallInfo> &toolCalls,
                                       std::string &finalResponse) {
  Json::Value root;
  Json::CharReaderBuilder readerBuilder;
  std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
  std::string errs;

  toolCalls.clear();
  finalResponse = "";
  thought = "";

  if (!reader->parse(jsonString.c_str(),
                     jsonString.c_str() + jsonString.length(), &root, &errs)) {
    // Log the specific parsing error and the problematic string
    logMessage(LogLevel::ERROR, "JSON Parsing failed for LLM Response",
               "Errors: " + errs + "\nInput: " + jsonString);
    return false;
  }

  if (!root.isObject()) {
    logMessage(LogLevel::ERROR, "LLM Response root is not a JSON object",
               jsonString);
    return false;
  }

  // Extract fields safely
  if (root.isMember("thought") && root["thought"].isString()) {
    thought = root["thought"].asString();
  } else {
    logMessage(LogLevel::WARN,
               "LLM Response missing or invalid 'thought' field.");
  }

  if (root.isMember("tool_calls")) {
    if (root["tool_calls"].isNull()) { /* Explicit null is fine */
    } else if (root["tool_calls"].isArray()) {
      for (const auto &callJson : root["tool_calls"]) {
        if (callJson.isObject() && callJson.isMember("tool_name") &&
            callJson["tool_name"].isString() && callJson.isMember("params") &&
            callJson["params"].isObject()) {
          ToolCallInfo info;
          info.toolName = callJson["tool_name"].asString();
          if (info.toolName.empty()) {
            logMessage(LogLevel::WARN,
                       "Empty 'tool_name' found in tool_calls object.",
                       callJson.toStyledString());
            continue;
          }
          info.params = callJson["params"];
          toolCalls.push_back(info);
        } else {
          logMessage(LogLevel::WARN,
                     "Malformed tool_call object in LLM response",
                     callJson.toStyledString());
        }
      }
    } else {
      logMessage(LogLevel::WARN,
                 "'tool_calls' field exists but is not an array or null.",
                 root["tool_calls"].toStyledString());
    }
  } else {
    logMessage(LogLevel::WARN, "LLM Response missing 'tool_calls' field.");
  }

  if (root.isMember("final_response")) {
    if (root["final_response"].isNull()) { /* Explicit null is fine */
    } else if (root["final_response"].isString()) {
      finalResponse = root["final_response"].asString();
    } else {
      logMessage(LogLevel::WARN,
                 "'final_response' field exists but is not a string or null.",
                 root["final_response"].toStyledString());
    }
  } else {
    logMessage(LogLevel::WARN, "LLM Response missing 'final_response' field.");
  }

  // Enforce: Cannot have both final_response and tool_calls
  if (!finalResponse.empty() && !toolCalls.empty()) {
    logMessage(LogLevel::WARN, "LLM provided both final_response and "
                               "tool_calls. Ignoring tool_calls.");
    toolCalls.clear();
  }

  return true;
}
