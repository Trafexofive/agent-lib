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

// --- JSON Parsing (NEW Structure) ---
bool Agent::parseStructuredLLMResponse(const std::string &jsonString,
                                       std::string &status,
                                       std::vector<StructuredThought> &thoughts,
                                       std::vector<ActionInfo> &actions,
                                       std::string &finalResponse) {
  status = "ERROR_INTERNAL";
  thoughts.clear();
  actions.clear();
  finalResponse = "";
  Json::Value root;
  Json::Reader reader;
  std::stringstream ss(jsonString);

  if (!reader.parse(ss, root, false)) {
    logMessage(LogLevel::ERROR,
               "Failed to parse LLM response as JSON for agent '" + name + "'",
               reader.getFormattedErrorMessages());
    finalResponse = jsonString;
    return false;
  }

  if (root.isMember("status") && root["status"].isString()) {
    status = root["status"].asString();
  } else {
    logMessage(LogLevel::ERROR,
               "LLM JSON missing required 'status' field for agent '" + name +
                   "'.",
               jsonString);
  }

  if (root.isMember("thoughts") && root["thoughts"].isArray()) {
    for (const auto &tv : root["thoughts"]) {
      if (tv.isObject() && tv.isMember("type") && tv["type"].isString() &&
          tv.isMember("content") && tv["content"].isString()) {
        thoughts.push_back({tv["type"].asString(), tv["content"].asString()});
      } else {
        logMessage(LogLevel::WARN,
                   "Malformed thought object in LLM JSON for agent '" + name +
                       "'. Skipping.",
                   tv.toStyledString());
      }
    }
  } else {
    logMessage(LogLevel::ERROR,
               "LLM JSON missing required 'thoughts' array for agent '" + name +
                   "'.",
               jsonString);
  }

  if (root.isMember("actions") && root["actions"].isArray()) {
    for (const auto &av : root["actions"]) {
      if (av.isObject() && av.isMember("action") && av["action"].isString() &&
          av.isMember("type") && av["type"].isString() &&
          av.isMember("params") && av["params"].isObject()) {
        ActionInfo ai;
        ai.action = av["action"].asString();
        ai.type = av["type"].asString();
        ai.params = av["params"];
        if (av.isMember("confidence") && av["confidence"].isNumeric()) {
          ai.confidence = av["confidence"].asDouble();
        }
        if (av.isMember("warnings") && av["warnings"].isArray()) {
          for (const auto &w : av["warnings"]) {
            if (w.isString())
              ai.warnings.push_back(w.asString());
          }
        }
        actions.push_back(ai);
      } else {
        logMessage(LogLevel::WARN,
                   "Malformed action object in LLM JSON for agent '" + name +
                       "'. Skipping.",
                   av.toStyledString());
      }
    }
  } else {
    logMessage(LogLevel::ERROR,
               "LLM JSON missing required 'actions' array for agent '" + name +
                   "'.",
               jsonString);
  }

  if (root.isMember("final_response") && root["final_response"].isString()) {
    finalResponse = root["final_response"].asString();
  } else if (root.isMember("final_response") &&
             root["final_response"].isNull()) {
    finalResponse = "";
  } else {
    logMessage(LogLevel::DEBUG, "LLM JSON missing 'final_response' field or "
                                "not string/null for agent '" +
                                    name + "'.");
  }

  if (status == "SUCCESS_FINAL" && !actions.empty()) {
    logMessage(LogLevel::WARN,
               "LLM JSON: Status SUCCESS_FINAL but actions array not empty for "
               "agent '" +
                   name + "'.",
               jsonString);
  }
  if ((status == "REQUIRES_ACTION" || status == "REQUIRES_CLARIFICATION") &&
      actions.empty()) {
    logMessage(LogLevel::WARN,
               "LLM JSON: Status requires action/clarification but actions "
               "array empty for agent '" +
                   name + "'.",
               jsonString);
  }

  return true;
}

// --- Action Processing (NEW Structure) ---
std::string Agent::processActions(const std::vector<ActionInfo> &actions) {
  std::stringstream resultsStream;
  resultsStream << "<action_results>\n";
  for (const auto &actionInfo : actions) {
    logMessage(LogLevel::TOOL_CALL, "Processing action '" + actionInfo.action +
                                        "' (type: " + actionInfo.type + ")");
    if (!actionInfo.warnings.empty()) {
      for (const auto &warn : actionInfo.warnings)
        logMessage(LogLevel::WARN, "Action Warning:", warn);
    }
    if (actionInfo.confidence < 0.95)
      logMessage(LogLevel::DEBUG,
                 "Action Confidence:", std::to_string(actionInfo.confidence));
    std::string result = processAction(actionInfo);
    logMessage(LogLevel::TOOL_RESULT,
               "Action result for '" + actionInfo.action + "'", result);
    resultsStream << "\t<action_result action=\"" << actionInfo.action
                  << "\" type=\"" << actionInfo.type << "\">\n";
    std::stringstream ss_result(result);
    std::string line;
    while (std::getline(ss_result, line)) {
      resultsStream << "\t\t" << line << "\n";
    }
    resultsStream << "\t</action_result>\n";
  }
  resultsStream << "</action_results>";
  return resultsStream.str();
}

std::string Agent::processAction(const ActionInfo &actionInfo) {
  if (actionInfo.type == "tool") {
    Tool *tool = getRegisteredTool(actionInfo.action);
    if (tool) {
      try {
        return tool->execute(actionInfo.params);
      } catch (const std::exception &e) {
        logMessage(LogLevel::ERROR, "Tool Exception: " + actionInfo.action,
                   e.what());
        return "Error: Tool Exception: " + std::string(e.what());
      } catch (...) {
        logMessage(LogLevel::ERROR, "Unknown Tool Exception",
                   actionInfo.action);
        return "Error: Unknown Tool Exception.";
      }
    } else {
      logMessage(LogLevel::ERROR, "Tool not found", actionInfo.action);
      return "Error: Tool '" + actionInfo.action + "' not registered.";
    }
  } else if (actionInfo.type == "internal_function") {
    if (actionInfo.action == "help")
      return getHelp(actionInfo.params);
    if (actionInfo.action == "skip")
      return skip(actionInfo.params);
    if (actionInfo.action == "promptAgent")
      return promptAgentTool(actionInfo.params);
    if (actionInfo.action == "summarizeTool")
      return summerizeTool(actionInfo.params); // Typo kept
    if (actionInfo.action == "summarizeHistory")
      return summarizeHistory(actionInfo.params);
    if (actionInfo.action == "getWeather")
      return getWeather(actionInfo.params);
    logMessage(LogLevel::ERROR, "Unknown internal function", actionInfo.action);
    return "Error: Unknown internal function '" + actionInfo.action + "'.";
  } else if (actionInfo.type == "script") {
    logMessage(LogLevel::WARN, "Script execution not implemented",
               actionInfo.action);
    return "Error: Script execution not supported.";
  } else if (actionInfo.type == "http_request") {
    logMessage(LogLevel::WARN, "HTTP request execution not implemented",
               actionInfo.action);
    return "Error: Direct HTTP requests not supported.";
  } else if (actionInfo.type == "output") {
    if (actionInfo.action == "send_response") {
      if (actionInfo.params.isMember("text") &&
          actionInfo.params["text"].isString()) {
        logMessage(LogLevel::INFO, "Executing send_response (text)",
                   actionInfo.params["text"].asString());
        return actionInfo.params["text"].asString();
      } // Add file_path, variable handling here (FUTURE)
      else {
        logMessage(LogLevel::ERROR, "Invalid params for send_response",
                   actionInfo.params.toStyledString());
        return "Error: Invalid params for send_response.";
      }
    } else {
      logMessage(LogLevel::ERROR, "Unknown output action", actionInfo.action);
      return "Error: Unknown output action '" + actionInfo.action + "'.";
    }
  } else if (actionInfo.type == "workflow_control") {
    logMessage(LogLevel::DEBUG, "Workflow control action handled by main loop",
               actionInfo.action);
    return "Action '" + actionInfo.action + "' acknowledged.";
  } else {
    logMessage(LogLevel::ERROR, "Unsupported action type",
               actionInfo.type + " for " + actionInfo.action);
    return "Error: Unsupported action type '" + actionInfo.type + "'.";
  }
}
