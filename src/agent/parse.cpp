#include "../../inc/Agent.hpp"
#include "../../inc/MiniGemini.hpp"
#include "../../inc/Tool.hpp"

bool Agent::parseStructuredLLMResponse(const std::string &jsonString,
                                       std::string &status,
                                       std::vector<Thought> &thoughts,
                                       std::vector<Action> &actions,
                                       std::string &finalResponseField) {
  status = "ERROR_INTERNAL_PARSE"; // Default status if parsing fails badly
  thoughts.clear();
  actions.clear();
  finalResponseField = "";

  Json::Value root;
  Json::CharReaderBuilder readerBuilder;
  std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());

  std::string parseErrors;

  if (!reader->parse(jsonString.c_str(),
                     jsonString.c_str() + jsonString.length(), &root,
                     &parseErrors)) {
    logMessage(LogLevel::ERROR,
               "Failed to parse LLM response JSON for agent '" + agentName +
                   "'.",
               "Errors: " + parseErrors + "\nInput: " + jsonString);
    finalResponseField = jsonString; // Fallback to raw response
    return false;
  }

  if (!root.isObject()) {
    logMessage(LogLevel::ERROR,
               "LLM response root is not a JSON object for agent '" +
                   agentName + "'.",
               jsonString);
    finalResponseField = jsonString;
    return false;
  }

  // Status (Required)
  if (root.isMember("status") && root["status"].isString()) {
    status = root["status"].asString();
  } else {
    logMessage(LogLevel::ERROR,
               "LLM JSON missing required 'status' (string) field for agent '" +
                   agentName + "'.",
               jsonString);
    // status remains ERROR_INTERNAL_PARSE or could be set to a more specific
    // missing field error
    return false; // Critical field missing
  }

  // Thoughts (Required, can be empty array)
  if (root.isMember("thoughts") && root["thoughts"].isArray()) {
    for (const auto &thoughtVal : root["thoughts"]) {
      if (thoughtVal.isObject() && thoughtVal.isMember("type") &&
          thoughtVal["type"].isString() && thoughtVal.isMember("content") &&
          thoughtVal["content"].isString()) {
        thoughts.push_back(
            {thoughtVal["type"].asString(), thoughtVal["content"].asString()});
      } else {
        logMessage(LogLevel::WARN,
                   "Malformed thought object in LLM JSON for agent '" +
                       agentName + "'. Skipping.",
                   thoughtVal.toStyledString());
      }
    }
  } else {
    logMessage(
        LogLevel::ERROR,
        "LLM JSON missing required 'thoughts' (array) field for agent '" +
            agentName + "'.",
        jsonString);
    return false; // Critical field missing
  }

  // Actions (Required, can be empty array)
  if (root.isMember("actions") && root["actions"].isArray()) {
    for (const auto &actionVal : root["actions"]) {
      if (actionVal.isObject() && actionVal.isMember("action") &&
          actionVal["action"].isString() && actionVal.isMember("type") &&
          actionVal["type"].isString() && actionVal.isMember("params") &&
          actionVal["params"].isObject()) {
        Action ai;
        ai.action = actionVal["action"].asString();
        ai.type = actionVal["type"].asString();
        ai.params = actionVal["params"];
        if (actionVal.isMember("confidence") &&
            actionVal["confidence"].isNumeric()) {
          ai.confidence = actionVal["confidence"].asDouble();
        }
        if (actionVal.isMember("warnings") && actionVal["warnings"].isArray()) {
          for (const auto &warnVal : actionVal["warnings"]) {
            if (warnVal.isString())
              ai.warnings.push_back(warnVal.asString());
          }
        }
        actions.push_back(ai);
      } else {
        logMessage(LogLevel::WARN,
                   "Malformed action object in LLM JSON for agent '" +
                       agentName + "'. Skipping.",
                   actionVal.toStyledString());
      }
    }
  } else {
    logMessage(LogLevel::ERROR,
               "LLM JSON missing required 'actions' (array) field for agent '" +
                   agentName + "'.",
               jsonString);
    return false; // Critical field missing
  }

  // Final Response (Optional: string or null)
  if (root.isMember("final_response")) {
    if (root["final_response"].isString()) {
      finalResponseField = root["final_response"].asString();
    } else if (root["final_response"].isNull()) {
      finalResponseField = ""; // Represent null as empty string
    } else {
      logMessage(LogLevel::WARN,
                 "LLM JSON 'final_response' field is present but not string or "
                 "null for agent '" +
                     agentName + "'.",
                 root["final_response"].toStyledString());
    }
  } else {
    logMessage(LogLevel::DEBUG,
               "LLM JSON does not have a 'final_response' field for agent '" +
                   agentName +
                   "'. This is often normal if actions are present.");
  }

  // Consistency checks (optional, but good for debugging LLM behavior)
  if (status == "SUCCESS_FINAL" && !actions.empty()) {
    logMessage(LogLevel::WARN,
               "LLM JSON: Status is SUCCESS_FINAL but 'actions' array is not "
               "empty for agent '" +
                   agentName + "'.",
               jsonString);
  }
  if ((status == "REQUIRES_ACTION" || status == "REQUIRES_CLARIFICATION") &&
      actions.empty() && finalResponseField.empty()) {
    // If status needs action/clarification but there are no actions AND no
    // final_response to ask the user, it's a problematic state.
    logMessage(LogLevel::WARN,
               "LLM JSON: Status '" + status +
                   "' but 'actions' array is empty and 'final_response' is "
                   "also empty for agent '" +
                   agentName + "'.",
               jsonString);
  }

  return true;
}
