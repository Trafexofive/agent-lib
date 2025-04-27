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

std::string Agent::prompt(const std::string &userInput) {
  addToHistory("Master", userInput);
  logMessage(LogLevel::INFO, "User Input received by Agent '" + name + "'",
             userInput);

  std::string rawLLMResponse = "";
  std::string finalResponseToUser = "";
  std::string thought = "";

  std::vector<std::string> extraPrompts;
  std::vector<std::string> thoughts;

  std::vector<ToolCallInfo> toolCalls;
  bool taskComplete = false;

  iteration = 0; // Reset iteration count for new prompt

  // --- Enhanced Agent Loop ---
  for (; iteration < iterationCap && !taskComplete; ++iteration) {
    logMessage(LogLevel::DEBUG, "Agent '" + name + "' - Iteration " +
                                    std::to_string(iteration + 1) + "/" +
                                    std::to_string(iterationCap));

    // --- Context Management Phase ---
    // TODO (Phase 2+): Summarize history, retrieve memories, build working
    // memory

    // --- Prompt Building Phase ---
    std::string currentPrompt = buildFullPrompt();

    // --- LLM Call Phase ---
    try {
      rawLLMResponse = executeApiCall(currentPrompt);
    } catch (const std::exception &e) {
      logMessage(LogLevel::ERROR,
                 "Failed to execute API call in iteration " +
                     std::to_string(iteration + 1),
                 e.what());
      finalResponseToUser =
          "[SYSTEM] Error: Failed to communicate with the language model.";
      taskComplete = true;
      break;
    }
    // Add raw response BEFORE attempting to parse/extract
    addToHistory(this->name, rawLLMResponse);

    // --- Extraction Phase (Handles ```json ... ```) ---
    std::string jsonToParse = rawLLMResponse; // Start with raw
    std::string startMarker = "```json";
    std::string endMarker = "```";

    size_t startPos = rawLLMResponse.find(startMarker);
    if (startPos != std::string::npos) {
      // Found start marker, look for actual JSON start '{'
      size_t jsonStart =
          rawLLMResponse.find('{', startPos + startMarker.length());
      if (jsonStart != std::string::npos) {
        // Found '{', now find end marker '```' after it
        size_t endPos = rawLLMResponse.find(endMarker, jsonStart);
        if (endPos != std::string::npos) {
          // Found end marker, find last '}' before it
          size_t jsonEnd = rawLLMResponse.rfind('}', endPos);
          if (jsonEnd != std::string::npos && jsonEnd >= jsonStart) {
            // Extract the likely JSON object content
            jsonToParse =
                rawLLMResponse.substr(jsonStart, jsonEnd - jsonStart + 1);
            logMessage(LogLevel::DEBUG,
                       "Extracted JSON block for parsing:", jsonToParse);
          } else {
            logMessage(LogLevel::WARN,
                       "Could not find matching '}' within JSON code block. "
                       "Attempting parse on block content.",
                       rawLLMResponse.substr(jsonStart, endPos - jsonStart));
            jsonToParse = rawLLMResponse.substr(jsonStart,
                                                endPos - jsonStart); // Fallback
          }
        } else {
          logMessage(LogLevel::WARN,
                     "Found ```json and '{' but no closing ``` marker. "
                     "Attempting parse from '{'.",
                     rawLLMResponse);
          jsonToParse =
              rawLLMResponse.substr(jsonStart); // Fallback: parse from '{'
        }
      } else {
        logMessage(LogLevel::WARN,
                   "Found ```json but no opening '{' found after it. "
                   "Attempting parse on raw response.",
                   rawLLMResponse);
        jsonToParse = rawLLMResponse; // Fallback: parse raw (likely fail)
      }
    } else {
      logMessage(LogLevel::WARN,
                 "LLM response did not contain ```json marker. Attempting to "
                 "parse raw response.",
                 rawLLMResponse);
      // Fallback: attempt to parse the raw response directly
      jsonToParse = rawLLMResponse;
    }

    // Trim whitespace from the string we intend to parse
    jsonToParse.erase(0, jsonToParse.find_first_not_of(" \t\r\n"));
    jsonToParse.erase(jsonToParse.find_last_not_of(" \t\r\n") + 1);

    // --- Response Parsing Phase ---
    toolCalls.clear();
    finalResponseToUser = ""; // Reset final response for this iteration
    thought = "";             // Reset thought
    bool parseSuccess = parseStructuredLLMResponse(
        jsonToParse, thought, toolCalls, finalResponseToUser);

    if (!parseSuccess) {
      logMessage(LogLevel::ERROR,
                 "Failed to parse extracted JSON. Aborting task.",
                 "Attempted to parse: " + jsonToParse);
      // Provide a more informative error, including the raw response if parsing
      // failed completely
      finalResponseToUser =
          "[SYSTEM] Error: Internal issue processing language model response "
          "format. Raw response was:\n" +
          rawLLMResponse;
      taskComplete = true;
      break;
    }

    if (!thought.empty()) {
      logMessage(LogLevel::DEBUG, "LLM Thought:", thought);
    }

    // --- Task Completion Check ---
    if (!finalResponseToUser.empty()) {
      logMessage(LogLevel::INFO, "LLM provided final response. Task complete.");
      taskComplete = true;
      break;
    }

    // --- Tool Execution Phase ---
    if (toolCalls.empty()) {
      logMessage(LogLevel::WARN, "LLM provided no tool calls and no final "
                                 "response. Agent may be stuck.");
      finalResponseToUser =
          "[SYSTEM] Agent process inconclusive. The language model did not "
          "provide a final answer or request further actions. Last thought: " +
          (thought.empty() ? "(None)" : thought);
      taskComplete = true;
      break;
    } else {
      logMessage(LogLevel::INFO, std::to_string(toolCalls.size()) +
                                     " tool call(s) requested. Executing...");
      std::string toolResultsString = processToolCalls(toolCalls);
      addToHistory("tool", toolResultsString);

      if (skipFlowIteration) {
        std::string notice =
            "[SYSTEM] Tool execution finished, skip requested. Review results.";
        logMessage(LogLevel::INFO,
                   "Skipping next LLM call as requested by a tool.");
        finalResponseToUser = notice;
        taskComplete = true;
        skipFlowIteration = false;
        break;
      }
    }
  } // End of iteration loop

  // --- Post-Loop Finalization ---
  if (!taskComplete && iteration >= iterationCap) {
    logMessage(LogLevel::WARN, "Agent '" + name + "' reached iteration cap (" +
                                   std::to_string(iterationCap) + ").");
    finalResponseToUser = "[SYSTEM] Reached maximum interaction depth (" +
                          std::to_string(iterationCap) +
                          "). Task may be incomplete. Last thought: " +
                          (thought.empty() ? "(None)" : thought);
  } else if (!taskComplete && finalResponseToUser.empty()) {
    logMessage(LogLevel::ERROR, "Loop finished unexpectedly without task "
                                "completion or final response.");
    finalResponseToUser =
        "[SYSTEM] An unexpected error occurred in the agent processing loop.";
  }

  logMessage(LogLevel::INFO, "Final response generated by Agent '" + name +
                                 "' after " + std::to_string(iteration) +
                                 " iteration(s).");
  return finalResponseToUser;
}

