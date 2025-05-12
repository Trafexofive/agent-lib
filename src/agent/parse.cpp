#include "../../inc/Agent.hpp" // Includes ActionInfo, StructuredThought, ParsedLLMResponse
#include <json/json.h>         // Your JSON library
#include <memory>              // For std::unique_ptr

// Ensure LogLevel and logMessage are accessible

ParsedLLMResponse Agent::parseStructuredLLMResponse(const std::string &trimmedJsonString) {
  ParsedLLMResponse result;
  result.rawTrimmedJson = trimmedJsonString; // Store the input for fallback/debugging

  Json::Value root;
  Json::CharReaderBuilder readerBuilder;
  std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
  std::string parseErrors;

  if (!reader->parse(trimmedJsonString.c_str(),
                     trimmedJsonString.c_str() + trimmedJsonString.length(),
                     &root, &parseErrors)) {
    logMessage(LogLevel::ERROR,
               "Agent '" + agentName + "': Failed to parse LLM JSON.",
               "Errors: " + parseErrors + "\nInput: " + trimmedJsonString.substr(0, 500));
    result.success = false;
    result.status = "ERROR_INTERNAL_PARSE_FAILURE";
    result.finalResponseField = trimmedJsonString; // Fallback to raw (trimmed) response
    return result;
  }

  if (!root.isObject()) {
    logMessage(LogLevel::ERROR,
               "Agent '" + agentName + "': LLM response root is not a JSON object.",
               trimmedJsonString.substr(0, 500));
    result.success = false;
    result.status = "ERROR_INVALID_JSON_STRUCTURE_ROOT_NOT_OBJECT";
    result.finalResponseField = trimmedJsonString;
    return result;
  }

  // 1. Parse 'status' (REQUIRED)
  if (root.isMember("status") && root["status"].isString()) {
    result.status = root["status"].asString();
  } else {
    logMessage(LogLevel::ERROR,
               "Agent '" + agentName + "': LLM JSON missing/invalid 'status' (string) field.",
               trimmedJsonString.substr(0, 500));
    result.success = false;
    result.status = "ERROR_MISSING_OR_INVALID_STATUS_FIELD"; // More specific internal error status
    result.finalResponseField = trimmedJsonString; // Fallback
    return result;
  }

  // 2. Parse 'thoughts' (REQUIRED array of objects: {type: string, content: string})
  if (root.isMember("thoughts") && root["thoughts"].isArray()) {
    for (const auto &thoughtNode : root["thoughts"]) {
      if (thoughtNode.isObject() &&
          thoughtNode.isMember("type") && thoughtNode["type"].isString() &&
          thoughtNode.isMember("content") && thoughtNode["content"].isString()) {
        result.thoughts.push_back({thoughtNode["type"].asString(), thoughtNode["content"].asString()});
      } else {
        logMessage(LogLevel::WARN,
                   "Agent '" + agentName + "': Malformed 'thought' object in LLM JSON. Skipping.",
                   thoughtNode.toStyledString().substr(0, 200));
      }
    }
  } else {
    logMessage(LogLevel::ERROR,
               "Agent '" + agentName + "': LLM JSON missing/invalid 'thoughts' (array) field.",
               trimmedJsonString.substr(0, 500));
    result.success = false;
    result.status = "ERROR_MISSING_OR_INVALID_THOUGHTS_FIELD";
    result.finalResponseField = trimmedJsonString;
    return result;
  }

  // 3. Parse 'actions' (REQUIRED array of objects or null)
  if (root.isMember("actions") && (root["actions"].isArray() || root["actions"].isNull())) {
    if (root["actions"].isArray()) {
        for (const auto &actionNode : root["actions"]) {
            if (actionNode.isObject() &&
                actionNode.isMember("action") && actionNode["action"].isString() &&
                actionNode.isMember("type") && actionNode["type"].isString() &&
                actionNode.isMember("params") && actionNode["params"].isObject()) { // Ensure params is an object
            ActionInfo currentAction;
            currentAction.action = actionNode["action"].asString();
            currentAction.type = actionNode["type"].asString();
            currentAction.params = actionNode["params"];

            if (actionNode.isMember("confidence") && actionNode["confidence"].isNumeric()) {
                currentAction.confidence = actionNode["confidence"].asDouble();
            }
            if (actionNode.isMember("warnings") && actionNode["warnings"].isArray()) {
                for (const auto &warnNode : actionNode["warnings"]) {
                if (warnNode.isString()) {
                    currentAction.warnings.push_back(warnNode.asString());
                }
                }
            }
            result.actions.push_back(currentAction);
            } else {
            logMessage(LogLevel::WARN,
                       "Agent '" + agentName + "': Malformed 'action' object in LLM JSON. Skipping.",
                       actionNode.toStyledString().substr(0, 200));
            // Depending on strictness, you might set result.success = false here too.
            // For now, we'll be lenient and skip malformed actions but continue.
            }
        }
    } // else: actions is null, result.actions remains empty, which is valid.
  } else {
    logMessage(LogLevel::ERROR,
               "Agent '" + agentName + "': LLM JSON missing/invalid 'actions' (array or null) field.",
               trimmedJsonString.substr(0, 500));
    result.success = false;
    result.status = "ERROR_MISSING_OR_INVALID_ACTIONS_FIELD";
    result.finalResponseField = trimmedJsonString;
    return result;
  }

  // 4. Parse 'final_response' (string or null)
  if (root.isMember("final_response")) {
    if (root["final_response"].isString()) {
      result.finalResponseField = root["final_response"].asString();
    } else if (root["final_response"].isNull()) {
      result.finalResponseField = ""; // Represent JSON null as empty string for simplicity
    } else {
      logMessage(LogLevel::WARN,
                 "Agent '" + agentName + "': LLM JSON 'final_response' is present but not string or null.",
                 root["final_response"].toStyledString().substr(0, 200));
      // Not a critical failure for success=true, but good to note.
    }
  } // else: field is optional, result.finalResponseField remains empty.

  // If we've reached here without returning false, parsing of required fields was structurally okay.
  result.success = true;
  return result;
}
