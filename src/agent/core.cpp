// src/agent/core.cpp
#include "../../inc/Agent.hpp"
#include "../../inc/Utils.hpp" // For executeScriptTool
#include <json/json.h>
#include <sstream> // For std::stringstream in processActions

// processActions: Aggregates results from multiple action calls
std::string Agent::processActions(const std::vector<ActionInfo> &actions) {
  if (actions.empty()) {
    return "<action_results status=\"no_actions_requested\"/>\n"; // XML-like is
                                                                  // fine for
                                                                  // now
  }
  std::stringstream resultsSs;
  resultsSs << "<action_results>\n";
  for (const auto &action : actions) {
    std::string result =
        processSingleAction(action); // Call the refined single action processor
    resultsSs << "  <action_result action_name=\"" << action.action
              << "\" type=\"" << action.type << "\">\n";
    // Basic XML escaping for the result text to avoid breaking the structure
    std::string escapedResult = result;
    size_t pos = 0;
    while ((pos = escapedResult.find("&", pos)) != std::string::npos) {
      escapedResult.replace(pos, 1, "&");
      pos += 5;
    }
    pos = 0;
    while ((pos = escapedResult.find("<", pos)) != std::string::npos) {
      escapedResult.replace(pos, 1, "<");
      pos += 4;
    }
    pos = 0;
    while ((pos = escapedResult.find(">", pos)) != std::string::npos) {
      escapedResult.replace(pos, 1, ">");
      pos += 4;
    }

    // Using CDATA for safety if results can be complex or contain problematic
    // characters
    resultsSs << "    <output><![CDATA[" << result << "]]></output>\n";
    resultsSs << "  </action_result>\n";
  }
  resultsSs << "</action_results>\n";
  return resultsSs.str();
}

// processSingleAction: Handles execution of one action
std::string Agent::processSingleAction(const ActionInfo &actionInfo) {
  logMessage(LogLevel::TOOL_CALL,
             "Agent '" + agentName +
                 "' preparing to execute action: " + actionInfo.action,
             "Type: " + actionInfo.type + ", Confidence: " +
                 std::to_string(actionInfo.confidence) + ", Params: " +
                 actionInfo.params.toStyledString().substr(0, 200) + "...");

  for (const auto &warning : actionInfo.warnings) {
    logMessage(LogLevel::WARN,
               "Agent '" + agentName + "': LLM Warning for action '" +
                   actionInfo.action + "'",
               warning);
  }

  try {
    if (actionInfo.type == "tool") {
      Tool *toolToRun = getTool(actionInfo.action);
      if (toolToRun) {
        std::string result = toolToRun->execute(actionInfo.params);
        logMessage(LogLevel::TOOL_RESULT,
                   "Agent '" + agentName + "' tool '" + actionInfo.action +
                       "' result:",
                   result.substr(0, 500) +
                       (result.length() > 500 ? "..." : ""));
        return result;
      } else {
        logMessage(LogLevel::ERROR, "Agent '" + agentName + "': Tool '" +
                                        actionInfo.action + "' not found.");
        return "Error: Tool '" + actionInfo.action +
               "' not registered or available.";
      }
    } else if (actionInfo.type == "internal_function") {
      // Dispatch to internal private methods
      // This mapping makes it cleaner and more extensible than many if-else ifs
      using InternalFuncPtr = std::string (Agent::*)(const Json::Value &);
      std::map<std::string, InternalFuncPtr> internalFuncMap = {
          {"help", &Agent::internalGetHelp},
          {"skip", &Agent::internalSkipIteration},
          {"promptAgent", &Agent::internalPromptAgent},
          {"summarizeText", &Agent::internalSummarizeText},
          {"summarizeHistory", &Agent::internalSummarizeHistory},
          {"getWeather", &Agent::internalGetWeather}
          // Add new internal functions here
      };

      auto it = internalFuncMap.find(actionInfo.action);
      if (it != internalFuncMap.end()) {
        std::string result = (this->*(it->second))(actionInfo.params);
        logMessage(LogLevel::TOOL_RESULT,
                   "Agent '" + agentName + "' internal_function '" +
                       actionInfo.action + "' result:",
                   result.substr(0, 500) +
                       (result.length() > 500 ? "..." : ""));
        return result;
      }
      logMessage(LogLevel::ERROR, "Agent '" + agentName +
                                      "': Unknown internal_function '" +
                                      actionInfo.action + "'.");
      return "Error: Unknown internal_function '" + actionInfo.action + "'.";

    } else if (actionInfo.type == "script") {
      if (!actionInfo.params.isMember("runtime") ||
          !actionInfo.params["runtime"].isString()) {
        return "Error [script action]: Missing 'runtime' string parameter.";
      }
      std::string runtime = actionInfo.params["runtime"].asString();
      std::string scriptSource;
      bool isInline = false;

      if (actionInfo.params.isMember("code") &&
          actionInfo.params["code"].isString()) {
        scriptSource = actionInfo.params["code"].asString();
        isInline = true;
      } else if (actionInfo.params.isMember("path") &&
                 actionInfo.params["path"].isString()) {
        scriptSource = actionInfo.params["path"].asString();
      } else {
        return "Error [script action]: Must provide either 'code' (string) or "
               "'path' (string) parameter.";
      }
      // Assuming executeScriptTool is globally available or via Utils.hpp
      std::string result =
          executeScriptTool(scriptSource, runtime, actionInfo.params, isInline);
      logMessage(LogLevel::TOOL_RESULT,
                 "Agent '" + agentName + "' script '" + actionInfo.action +
                     "' result:",
                 result.substr(0, 500) + (result.length() > 500 ? "..." : ""));
      return result;
    } else if (actionInfo.type == "http_request") {
      logMessage(LogLevel::WARN,
                 "Agent '" + agentName + "': Action type 'http_request' for '" +
                     actionInfo.action +
                     "' is a placeholder and not yet fully implemented.");
      // Placeholder for actual HTTP client call
      // Example params: actionInfo.params["url"], actionInfo.params["method"],
      // actionInfo.params["headers"], actionInfo.params["body"]
      return "Error: Action type 'http_request' not implemented in "
             "Agent::processSingleAction.";
    } else if (actionInfo.type == "output") {
      if (actionInfo.action == "send_response") {
        if (actionInfo.params.isMember("text") &&
            actionInfo.params["text"].isString()) {
          std::string text = actionInfo.params["text"].asString();
          logMessage(LogLevel::INFO,
                     "Agent '" + agentName +
                         "' 'send_response' action content:",
                     text.substr(0, 200) + (text.length() > 200 ? "..." : ""));
          return "Output action 'send_response' noted. Content: " +
                 text.substr(0, 100) +
                 (text.length() > 100 ? "..." : ""); // Main loop uses this
        }
        return "Error: 'send_response' action missing 'text' parameter.";
      }
      return "Error: Unknown 'output' action '" + actionInfo.action + "'.";
    } else if (actionInfo.type == "workflow_control") {
      if (actionInfo.action == "request_user_input") {
        if (actionInfo.params.isMember("query_to_user") &&
            actionInfo.params["query_to_user"].isString()) {
          logMessage(LogLevel::INFO,
                     "Agent '" + agentName + "' 'request_user_input' query:",
                     actionInfo.params["query_to_user"].asString());
          return "Workflow control: '" + actionInfo.action +
                 "' noted. Query: " +
                 actionInfo.params["query_to_user"].asString().substr(0, 100) +
                 (actionInfo.params["query_to_user"].asString().length() > 100
                      ? "..."
                      : "");
        }
        return "Error: 'request_user_input' action missing 'query_to_user' "
               "parameter.";
      }
      return "Error: Unknown 'workflow_control' action '" + actionInfo.action +
             "'.";
    }

    logMessage(LogLevel::ERROR, "Agent '" + agentName +
                                    "': Unsupported action type '" +
                                    actionInfo.type + "' for action '" +
                                    actionInfo.action + "'.");
    return "Error: Unsupported action type '" + actionInfo.type + "'.";

  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR,
               "Agent '" + agentName + "': Exception during action '" +
                   actionInfo.action + "' execution.",
               e.what());
    return "Error executing action '" + actionInfo.action +
           "': " + std::string(e.what());
  } catch (...) {
    logMessage(LogLevel::ERROR, "Agent '" + agentName +
                                    "': Unknown exception during action '" +
                                    actionInfo.action + "' execution.");
    return "Error: Unknown exception executing action '" + actionInfo.action +
           "'.";
  }
}

// Agent::prompt method (complete, using ParsedLLMResponse)
std::string Agent::prompt(const std::string &userInput) {
  if (!userInput.empty()) {
    addToHistory("user", userInput);
  }

  currentIteration = 0;
  skipNextFlowIteration = false;
  std::string finalAgentResponseToUser = "";

  while (currentIteration < iterationLimit && !skipNextFlowIteration) {
    currentIteration++;
    logMessage(LogLevel::INFO, "Agent '" + agentName + "' Iteration " +
                                   std::to_string(currentIteration) + "/" +
                                   std::to_string(iterationLimit));

    std::string fullPromptText = buildFullPrompt();
    std::string llmRawResponse = executeApiCall(fullPromptText);

    std::string trimmedLlmResponse = llmRawResponse;
    trimLLMResponse(trimmedLlmResponse);

    ParsedLLMResponse parsedData =
        parseStructuredLLMResponse(trimmedLlmResponse);
    addToHistory(
        "model",
        parsedData.rawTrimmedJson); // Add the (trimmed) raw JSON to history

    if (!parsedData.success) {
      logMessage(LogLevel::ERROR,
                 "Agent '" + agentName +
                     "': Critical failure parsing LLM response. Internal "
                     "parser status: " +
                     parsedData.status,
                 "Raw trimmed JSON: " +
                     parsedData.rawTrimmedJson.substr(0, 500));
      // Check if the raw response was an API error JSON from our client
      Json::Value rawJsonCheck;
      Json::CharReaderBuilder rBuilder;
      std::unique_ptr<Json::CharReader> r(rBuilder.newCharReader());
      std::string err_parse_raw;
      if (r->parse(parsedData.rawTrimmedJson.c_str(),
                   parsedData.rawTrimmedJson.c_str() +
                       parsedData.rawTrimmedJson.length(),
                   &rawJsonCheck, &err_parse_raw) &&
          rawJsonCheck.isObject() &&
          rawJsonCheck.isMember("error")) { // A common error pattern
        finalAgentResponseToUser = parsedData.rawTrimmedJson;
      } else {
        finalAgentResponseToUser =
            "Agent '" + agentName +
            "' encountered an issue processing the response from the language "
            "model. Parser status: " +
            parsedData.status;
      }
      setSkipNextFlowIteration(true);
      continue;
    }

    for (const auto &thought : parsedData.thoughts) {
      logMessage(LogLevel::DEBUG,
                 "Agent '" + agentName + "': LLM Thought (" + thought.type +
                     ")",
                 thought.content);
    }

    if (parsedData.status == "SUCCESS_FINAL") {
      logMessage(LogLevel::INFO,
                 "Agent '" + agentName + "': LLM indicates SUCCESS_FINAL.");
      finalAgentResponseToUser = parsedData.finalResponseField;
      if (finalAgentResponseToUser.empty()) {
        for (const auto &action : parsedData.actions) {
          if (action.type == "output" && action.action == "send_response" &&
              action.params.isMember("text") &&
              action.params["text"].isString()) {
            finalAgentResponseToUser = action.params["text"].asString();
            logMessage(LogLevel::DEBUG,
                       "Agent '" + agentName +
                           "': Using text from 'send_response' action for "
                           "final output.",
                       finalAgentResponseToUser.substr(0, 200));
            break;
          }
        }
      }
      if (finalAgentResponseToUser.empty()) {
        logMessage(LogLevel::WARN,
                   "Agent '" + agentName +
                       "': SUCCESS_FINAL status but 'final_response' field and "
                       "'send_response' action text are empty. Sending generic "
                       "success.");
        finalAgentResponseToUser =
            "Task completed successfully by " + agentName + ".";
      }
      setSkipNextFlowIteration(true);
    } else if (parsedData.status == "REQUIRES_ACTION") {
      if (!parsedData.actions.empty()) {
        logMessage(
            LogLevel::INFO,
            "Agent '" + agentName + "': LLM requires action(s). Processing " +
                std::to_string(parsedData.actions.size()) + " action(s).");
        std::string actionResultsText = processActions(parsedData.actions);
        addToHistory("action_results", actionResultsText);
      } else {
        logMessage(LogLevel::WARN,
                   "Agent '" + agentName +
                       "': LLM status REQUIRES_ACTION but no actions provided.",
                   parsedData.rawTrimmedJson);
        finalAgentResponseToUser = "Agent '" + agentName +
                                   "' is unable to proceed: LLM indicated "
                                   "action needed but provided no actions.";
        setSkipNextFlowIteration(true);
      }
    } else if (parsedData.status == "REQUIRES_CLARIFICATION") {
      logMessage(LogLevel::INFO,
                 "Agent '" + agentName + "': LLM requires clarification.");
      finalAgentResponseToUser = "I need more information to proceed.";
      bool queryFound = false;
      for (const auto &action : parsedData.actions) {
        if (action.type == "workflow_control" &&
            action.action == "request_user_input" &&
            action.params.isMember("query_to_user") &&
            action.params["query_to_user"].isString()) {
          finalAgentResponseToUser = action.params["query_to_user"].asString();
          queryFound = true;
          break;
        }
      }
      if (!queryFound &&
          !parsedData.finalResponseField
               .empty()) { // Check finalResponseField if no specific action
        finalAgentResponseToUser = parsedData.finalResponseField;
        logMessage(
            LogLevel::DEBUG,
            "Agent '" + agentName +
                "': Using 'final_response' field for clarification query.",
            finalAgentResponseToUser.substr(0, 200));
      } else if (!queryFound) {
        logMessage(
            LogLevel::WARN,
            "Agent '" + agentName +
                "': REQUIRES_CLARIFICATION status but no 'request_user_input' "
                "action with query found, and 'final_response' is empty.");
      }
      setSkipNextFlowIteration(true);
    } else if (parsedData.status.rfind("ERROR_", 0) == 0) {
      logMessage(LogLevel::ERROR,
                 "Agent '" + agentName + "': LLM or internal error.",
                 "Status: " + parsedData.status +
                     ". Details: " + parsedData.finalResponseField);
      finalAgentResponseToUser =
          parsedData.finalResponseField.empty()
              ? ("Agent '" + agentName +
                 "' encountered an error: " + parsedData.status)
              : parsedData.finalResponseField;
      setSkipNextFlowIteration(true);
    } else {
      logMessage(LogLevel::WARN,
                 "Agent '" + agentName +
                     "': LLM response has unknown status ('" +
                     parsedData.status + "'). Using raw response as final.",
                 parsedData.rawTrimmedJson.substr(0, 500));
      finalAgentResponseToUser = parsedData.rawTrimmedJson;
      setSkipNextFlowIteration(true);
    }
  }

  if (currentIteration >= iterationLimit && !skipNextFlowIteration) {
    logMessage(LogLevel::WARN, "Agent '" + agentName +
                                   "' reached iteration limit (" +
                                   std::to_string(iterationLimit) + ").");
    if (finalAgentResponseToUser.empty()) {
      finalAgentResponseToUser = "Agent '" + agentName +
                                 "' has processed the maximum iterations (" +
                                 std::to_string(iterationLimit) +
                                 ") for this request. Please try rephrasing or "
                                 "breaking down the request.";
    }
  }

  std::cout << "Final response from Agent '" << agentName << "':\n"
            << finalAgentResponseToUser.substr(0, 500) +
                   (finalAgentResponseToUser.length() > 500 ? "..." : "")
            << std::endl;

  // logMessage(LogLevel::INFO,
  //            "Agent '" + agentName + "' final response for prompt cycle:",
  //
  //            finalAgentResponseToUser.substr(0, 500) +
  //                (finalAgentResponseToUser.length() > 500 ? "..." : ""));
  return finalAgentResponseToUser;
}
