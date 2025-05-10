#include "../../inc/Agent.hpp"
#include "../../inc/MiniGemini.hpp"
#include "../../inc/Tool.hpp"
#include <json/json.h>

std::string Agent::prompt(const std::string &userInput) {
  if (!userInput.empty()) {
    addToHistory("user", userInput);
  }
  currentIteration = 0;
  skipNextFlowIteration =
      false; // Reset skip flag at the beginning of a new prompt
  std::string finalAgentResponseToUser = "";

  // Agent Runtime loop.
  while (currentIteration < iterationLimit && !skipNextFlowIteration) {
    currentIteration++;
    logMessage(LogLevel::INFO, "Agent '" + agentName + "' Iteration " +
                                   std::to_string(currentIteration) + "/" +
                                   std::to_string(iterationLimit));

    std::string fullPromptText = buildFullPrompt();
    std::string llmRawResponse = executeApiCall(fullPromptText);

    trimLLMResponse(llmRawResponse); // Trim ```json ... ``` if present
    addToHistory("model",
                 llmRawResponse); // Add possibly trimmed response to history

    std::string llmStatus;
    std::vector<Thought> llmThoughts;
    std::vector<Action> llmActions;
    std::string llmFinalResponseField;

    bool parsedOk =
        parseStructuredLLMResponse(llmRawResponse, llmStatus, llmThoughts,
                                   llmActions, llmFinalResponseField);

    if (!parsedOk) {
      logMessage(LogLevel::ERROR,
                 "Failed to parse structured LLM response. Agent '" +
                     agentName + "' treating raw response as final.",
                 llmRawResponse);
      finalAgentResponseToUser =
          llmRawResponse;             // Use the (trimmed) raw response
      setSkipNextFlowIteration(true); // End loop
      continue;
    }

    for (const auto &thought : llmThoughts) {
      logMessage(LogLevel::DEBUG,
                 "LLM Thought (" + thought.type + ") for agent '" + agentName +
                     "':",
                 thought.content);
    }

    if (llmStatus == "REQUIRES_CLARIFICATION") {
      logMessage(LogLevel::INFO,
                 "LLM requires clarification for agent '" + agentName + "'.");
      finalAgentResponseToUser = "Agent needs clarification."; // Default
      for (const auto &action :
           llmActions) { // Check if there's a specific query in actions
        if (action.action == "request_user_input" &&
            action.params.isMember("query_to_user") &&
            action.params["query_to_user"].isString()) {
          finalAgentResponseToUser = action.params["query_to_user"].asString();
          break;
        }
      }
      setSkipNextFlowIteration(true);
    } else if (llmStatus == "REQUIRES_ACTION" && !llmActions.empty()) {
      logMessage(LogLevel::INFO,
                 "LLM requested actions for agent '" + agentName + "'.");
      std::string actionResultsText = processActions(llmActions);
      addToHistory("action_results",
                   actionResultsText); // Feed results back for next iteration
    } else if (llmStatus == "SUCCESS_FINAL") {
      logMessage(LogLevel::INFO,
                 "LLM provided final response for agent '" + agentName + "'.");
      finalAgentResponseToUser = llmFinalResponseField;
      // Check for "output" type action "send_response" to potentially override
      // if final_response field was empty
      for (const auto &action : llmActions) {
        if (action.type == "output" && action.action == "send_response") {
          if (finalAgentResponseToUser.empty() &&
              action.params.isMember("text") &&
              action.params["text"].isString()) {
            finalAgentResponseToUser = action.params["text"].asString();
            logMessage(
                LogLevel::DEBUG,
                "Overriding final response with 'send_response' action text.",
                finalAgentResponseToUser);
          }
          break; // Only one send_response action should dictate the final
                 // output here.
        }
      }
      setSkipNextFlowIteration(true);
    } else if (llmStatus == "ERROR_INTERNAL") {
      logMessage(LogLevel::ERROR,
                 "LLM indicated an internal error for agent '" + agentName +
                     "'.",
                 llmFinalResponseField);
      finalAgentResponseToUser = llmFinalResponseField.empty()
                                     ? "LLM reported an internal error."
                                     : llmFinalResponseField;
      setSkipNextFlowIteration(true);
    } else { // Unknown status or valid status but logic error (e.g.
             // REQUIRES_ACTION with no actions)
      logMessage(LogLevel::WARN,
                 "LLM response has unclear status/action state for agent '" +
                     agentName + "'. Treating as final.",
                 "Status: " + llmStatus + " | Raw: " + llmRawResponse);
      finalAgentResponseToUser = llmFinalResponseField.empty()
                                     ? llmRawResponse
                                     : llmFinalResponseField;
      setSkipNextFlowIteration(true);
    }
  } // End while loop

  if (currentIteration >= iterationLimit && !skipNextFlowIteration) {
    logMessage(LogLevel::WARN,
               "Iteration limit reached for agent '" + agentName +
                   "' without a conclusive response.",
               std::to_string(iterationLimit));
    if (finalAgentResponseToUser.empty()) {
      finalAgentResponseToUser =
          "Agent '" + agentName + "' reached iteration limit (" +
          std::to_string(iterationLimit) + ") without a final answer.";
      // Try to find a sensible last "model" message from history that wasn't an
      // action request
      for (auto it = conversationHistory.rbegin();
           it != conversationHistory.rend(); ++it) {
        if (it->first == "model") {
          Json::Reader reader;
          Json::Value parsedJson;
          if (reader.parse(it->second, parsedJson) &&
              parsedJson.isMember("status") &&
              parsedJson["status"].asString() == "SUCCESS_FINAL" &&
              parsedJson.isMember("final_response")) {
            finalAgentResponseToUser = parsedJson["final_response"].asString();
            break;
          } else if (it->second.find("\"actions\": []") != std::string::npos &&
                     it->second.find("REQUIRES_ACTION") == std::string::npos) {
            // A bit heuristic: if it looks like a final JSON without actions.
            finalAgentResponseToUser = it->second;
            break;
          }
        }
      }
    }
  }
  // Ensure skip flag is reset for the next independent call to prompt()
  // skipNextFlowIteration = false; // This is now reset at the start of
  // prompt()

  logMessage(LogLevel::INFO,
             "Final response for agent '" + agentName + "' after prompt cycle:",
             finalAgentResponseToUser.substr(0, 200) +
                 (finalAgentResponseToUser.length() > 200 ? "..." : ""));
  return finalAgentResponseToUser;
}
