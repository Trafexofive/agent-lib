// src/agent/core.cpp
#include "../../inc/Agent.hpp"
#include "../../inc/Utils.hpp" // For logMessage, executeScriptTool
#include <ctime>               // For internalGetCurrentTime
#include <iomanip>             // For std::put_time
#include <json/json.h>
#include <sstream> // For std::stringstream in processActions

// processActions: Aggregates results from multiple action calls
std::string Agent::processActions(const std::vector<ActionInfo> &actions) {
  if (actions.empty()) {
    return "<action_results status=\"no_actions_requested\"/>\n";
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

    resultsSs << "    <output><![CDATA[" << result
              << "]]></output>\n"; // Use original result for CDATA
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
    // For "tool" and "script" types that are loaded from YAML, they are
    // registered as Tools. The Tool's execute method will call the appropriate
    // lambda (which in turn calls executeScriptTool or a C++ function).
    if (actionInfo.type == "tool" || actionInfo.type == "script") {
      Tool *toolToRun = getTool(actionInfo.action);
      if (toolToRun) {
        // The params for Tool::execute are the script-specific params from the
        // LLM.
        std::string result = toolToRun->execute(actionInfo.params);
        logMessage(LogLevel::TOOL_RESULT,
                   "Agent '" + agentName + "' " + actionInfo.type + " '" +
                       actionInfo.action + "' result:",
                   result.substr(0, 500) +
                       (result.length() > 500 ? "..." : ""));
        return result;
      } else {
        logMessage(LogLevel::ERROR,
                   "Agent '" + agentName + "': " + actionInfo.type + " '" +
                       actionInfo.action +
                       "' not found or not registered correctly.");
        return "Error: " + actionInfo.type + " '" + actionInfo.action +
               "' not registered or available.";
      }
    } else if (actionInfo.type == "internal_function") {
      using InternalFuncPtr = std::string (Agent::*)(const Json::Value &);
      std::map<std::string, InternalFuncPtr> internalFuncMap = {
          {"help", &Agent::internalGetHelp},
          {"skip", &Agent::internalSkipIteration},
          {"promptAgent", &Agent::internalPromptAgent},
          // {"summarizeText", &Agent::internalSummarizeText},
          // {"summarizeHistory", &Agent::internalSummarizeHistory},
          {"getWeather", &Agent::internalGetWeather},
          {"get_current_time", &Agent::internalGetCurrentTime}};

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

    } else if (actionInfo.type == "http_request") {
      logMessage(LogLevel::WARN,
                 "Agent '" + agentName + "': Action type 'http_request' for '" +
                     actionInfo.action +
                     "' is a placeholder and not yet fully implemented.");
      return "Error: Action type 'http_request' not implemented in "
             "Agent::processSingleAction.";
    } else if (actionInfo.type == "output") {
      logMessage(LogLevel::WARN,
                 "Agent '" + agentName +
                     "': LLM attempted to use 'output' action type for '" +
                     actionInfo.action +
                     "'. This may indicate a deviation from the expected JSON "
                     "schema usage. "
                     "Final responses should typically use status: "
                     "SUCCESS_FINAL and the 'final_response' field.");
      if (actionInfo.action == "send_response") {
        if (actionInfo.params.isMember("text") &&
            actionInfo.params["text"].isString()) {
          std::string text = actionInfo.params["text"].asString();
          logMessage(
              LogLevel::INFO,
              "Agent '" + agentName +
                  "' 'send_response' action (via 'output' type) content:",
              text.substr(0, 200) + (text.length() > 200 ? "..." : ""));
          return "Output action 'send_response' noted by agent. Content: " +
                 text.substr(0, 100) + (text.length() > 100 ? "..." : "");
        }
        return "Error: 'send_response' action (via 'output' type) missing "
               "'text' parameter.";
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

// Agent::prompt method (modified for clarity based on prior discussion, no
// major logic change here)
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
    addToHistory("model", parsedData.rawTrimmedJson);

    if (!parsedData.success) {
      logMessage(LogLevel::ERROR,
                 "Agent '" + agentName +
                     "': Critical failure parsing LLM response. Internal "
                     "parser status: " +
                     parsedData.status,
                 "Raw trimmed JSON: " +
                     parsedData.rawTrimmedJson.substr(0, 500));

      Json::Value rawJsonCheck;
      Json::CharReaderBuilder rBuilder;
      std::unique_ptr<Json::CharReader> r(rBuilder.newCharReader());
      std::string err_parse_raw;

      if (r->parse(parsedData.rawTrimmedJson.c_str(),
                   parsedData.rawTrimmedJson.c_str() +
                       parsedData.rawTrimmedJson.length(),
                   &rawJsonCheck, &err_parse_raw) &&
          rawJsonCheck.isObject() && rawJsonCheck.isMember("error")) {
        finalAgentResponseToUser = parsedData.rawTrimmedJson;
      } else {
        finalAgentResponseToUser =
            "Agent '" + agentName +
            "' encountered an issue processing the response from the language "
            "model. Parser status: " +
            parsedData.status +
            ". Raw: " + parsedData.rawTrimmedJson.substr(0, 200);
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
                           "final output (SUCCESS_FINAL).",
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
      bool queryFoundInAction = false;
      for (const auto &action : parsedData.actions) {
        if (action.type == "workflow_control" &&
            action.action == "request_user_input" &&
            action.params.isMember("query_to_user") &&
            action.params["query_to_user"].isString()) {
          finalAgentResponseToUser = action.params["query_to_user"].asString();
          queryFoundInAction = true;
          logMessage(LogLevel::DEBUG,
                     "Agent '" + agentName +
                         "': Using query from 'request_user_input' action.");
          break;
        }
      }
      if (!queryFoundInAction && !parsedData.finalResponseField.empty()) {
        finalAgentResponseToUser = parsedData.finalResponseField;
        logMessage(
            LogLevel::DEBUG,
            "Agent '" + agentName +
                "': Using 'final_response' field for clarification query.",
            finalAgentResponseToUser.substr(0, 200));
      } else if (!queryFoundInAction && parsedData.finalResponseField.empty()) {
        logMessage(
            LogLevel::WARN,
            "Agent '" + agentName +
                "': REQUIRES_CLARIFICATION status but no 'request_user_input' "
                "action with query found, and 'final_response' is empty.");
      }
      setSkipNextFlowIteration(true);
    } else if (parsedData.status.rfind("ERROR_", 0) == 0) {
      logMessage(
          LogLevel::ERROR, "Agent '" + agentName + "': LLM reported an error.",
          "Status: " + parsedData.status + ". Details in final_response: " +
              parsedData.finalResponseField.substr(0, 200));
      finalAgentResponseToUser =
          parsedData.finalResponseField.empty()
              ? ("Agent '" + agentName +
                 "' encountered an error: " + parsedData.status)
              : parsedData.finalResponseField;
      setSkipNextFlowIteration(true);
    } else {
      logMessage(
          LogLevel::WARN,
          "Agent '" + agentName + "': LLM response has unknown status ('" +
              parsedData.status + "'). Using raw JSON as final response.",
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

  logMessage(LogLevel::INFO,
             "Agent '" + agentName + "' completed prompt cycle.",
             "Current iteration: " + std::to_string(currentIteration) +
                 ", Iteration limit: " + std::to_string(iterationLimit) +
                 ", Skip next iteration: " +
                 (skipNextFlowIteration ? "true" : "false"));

  // logMessage(LogLevel::INFO,
  //            "Agent '" + agentName + "' final response to user:",
  //            finalAgentResponseToUser.substr(0, 200) +
  //                (finalAgentResponseToUser.length() > 200 ? "..." : ""));

  // logMessage(LogLevel::INFO,
  //            "Agent '" + agentName + "' final response to user (full):",
  //            finalAgentResponseToUser);

#define RESET "\033[0m"
#define RED "\033[31m"

  std::cout << "\n"
            << RED << agentName << ": " << RESET << finalAgentResponseToUser
            << std::endl;

  return finalAgentResponseToUser;
}

// Implementation for internalGetCurrentTime
std::string Agent::internalGetCurrentTime(const Json::Value &params) {
  (void)params;
  logMessage(LogLevel::DEBUG,
             "Agent '" + agentName + "' executing internal: get_current_time");
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm_buf;

  std::tm *now_tm = localtime_r(&now_c, &now_tm_buf);

  if (now_tm) {
    std::stringstream ss;
    // ISO 8601 like format, common for APIs
    ss << std::put_time(now_tm, "%Y-%m-%dT%H:%M:%S%Z");
    return ss.str();
  } else {
    return "Error: Could not retrieve current time.";
  }
}

std::string
staticAgentCall(std::vector<std::string> tasks,
                Agent &TargetAgent) { // primitive relic Implementation
  std::string result;

  return result;
}

/**
 * @brief Makes a "static" call to a target Agent instance with a list of tasks.
 * This function simulates a higher-level command or a primitive "Relic"
 * interaction that directs another agent. It's intended to be called by an
 * orchestrating agent or a system component to delegate a bundle of tasks to a
 * target agent.
 *
 * @param tasks A vector of strings, where each string is a task or instruction.
 * @param targetAgent A reference to the Agent instance to be prompted.
 * @param callingAgentName Optional: Name of the entity making the call, for
 * logging.
 * @return std::string The consolidated final response from the targetAgent
 * after processing tasks.
 */
std::string
staticAgentCall(const std::vector<std::string> &tasks, Agent &targetAgent,
                const std::string &callingAgentName = "StaticCaller") {
  if (tasks.empty()) {
    logMessage(LogLevel::WARN, // Using global LogLevel
               "[" + callingAgentName + " -> staticAgentCall]",
               "No tasks provided to target agent: " + targetAgent.getName());
    return "Error: No tasks provided for agent '" + targetAgent.getName() +
           "'.";
  }

  std::stringstream combinedPromptSs;
  combinedPromptSs
      << "ATTENTION " << targetAgent.getName()
      << ": You have received a direct command assignment from '"
      << callingAgentName
      << "'. Please process the following tasks sequentially and provide a "
         "consolidated final report or outcome. Your standard iterative "
         "thinking process is expected.\n\nTasks:\n";
  for (size_t i = 0; i < tasks.size(); ++i) {
    combinedPromptSs << (i + 1) << ". " << tasks[i] << "\n";
  }
  combinedPromptSs << "\nReport on all outcomes.";
  std::string combinedPrompt = combinedPromptSs.str();

  logMessage(LogLevel::INFO, "[" + callingAgentName + " -> staticAgentCall]",
             "Prompting target agent '" + targetAgent.getName() + "' with " +
                 std::to_string(tasks.size()) + " task(s).");
  logMessage(LogLevel::DEBUG, "[" + callingAgentName + " -> staticAgentCall]",
             "Combined prompt for '" + targetAgent.getName() +
                 "': " + combinedPrompt.substr(0, 250) +
                 (combinedPrompt.length() > 250 ? "..." : ""));

  std::string result;
  try {
    result = targetAgent.prompt(combinedPrompt);

    logMessage(LogLevel::INFO, "[" + callingAgentName + " -> staticAgentCall]",
               "Received response from target agent '" + targetAgent.getName() +
                   "'.");
    logMessage(LogLevel::DEBUG, "[" + callingAgentName + " -> staticAgentCall]",
               "Response from '" + targetAgent.getName() +
                   "': " + result.substr(0, 250) +
                   (result.length() > 250 ? "..." : ""));

  } catch (const ApiError &e) {
    logMessage(LogLevel::ERROR, "[" + callingAgentName + " -> staticAgentCall]",
               "API Error while prompting target agent '" +
                   targetAgent.getName() + "': " + e.what());
    result = "Error: API interaction with agent '" + targetAgent.getName() +
             "' failed: " + e.what();
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR, "[" + callingAgentName + " -> staticAgentCall]",
               "Standard Exception while prompting target agent '" +
                   targetAgent.getName() + "': " + e.what());
    result = "Error: Exception while interacting with agent '" +
             targetAgent.getName() + "': " + e.what();
  } catch (...) {
    logMessage(LogLevel::ERROR, "[" + callingAgentName + " -> staticAgentCall]",
               "Unknown error while prompting target agent '" +
                   targetAgent.getName() + "'.");
    result = "Error: Unknown error occurred while interacting with agent '" +
             targetAgent.getName() + "'.";
  }

  return result;
}
