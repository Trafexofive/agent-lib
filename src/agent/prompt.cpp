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


// helper to trim markdown code blocks if present
void trimCodeBlock(std::string &str)
{
    std::string startMarker = "```json";
    std::string endMarker = "```";
    size_t startPos = str.find(startMarker);
    if (startPos != std::string::npos)
    {
        // Found start marker, look for actual JSON start '{'
        size_t jsonStart = str.find('{', startPos + startMarker.length());
        if (jsonStart != std::string::npos)
        {
            // Found '{', now find end marker '```' after it
            size_t endPos = str.find(endMarker, jsonStart);
            if (endPos != std::string::npos)
            {
                // Found end marker, find last '}' before it
                size_t jsonEnd = str.rfind('}', endPos);
                if (jsonEnd != std::string::npos && jsonEnd >= jsonStart)
                {
                    // Extract the likely JSON object content
                    str = str.substr(jsonStart, jsonEnd - jsonStart + 1);
                }
            }
        }
    }
    else
    {
        // No code block found, just trim whitespace
        str.erase(str.find_last_not_of(" \t\r\n") + 1);
    }
    // Trim whitespace from the string we intend to parse
    str.erase(0, str.find_first_not_of(" \t\r\n"));
    str.erase(str.find_last_not_of(" \t\r\n") + 1);
    
    // Check if the string is empty after trimming
    if (str.empty())
    {
        throw std::runtime_error("Parsed JSON string is empty after trimming.");
    }

}
    



// Main prompt processing loop (using new JSON structure)
std::string Agent::prompt(const std::string &userInput) {
  if (!userInput.empty()) {
    addToHistory("user", userInput);
  }
  iteration = 0;
  skipFlowIteration = false;
  std::string finalAgentResponse = "";

  while (iteration < iterationCap && !skipFlowIteration) {
    iteration++;
    logMessage(LogLevel::INFO, "Agent '" + name + "' Iteration " +
                                   std::to_string(iteration) + "/" +
                                   std::to_string(iterationCap));

    std::string fullPrompt = buildFullPrompt();
    std::string llmResponseJson = executeApiCall(fullPrompt);
    try {
      trimCodeBlock(llmResponseJson); // Trim code block if present
    } catch (const std::exception &e) {
      logMessage(LogLevel::ERROR,
                 "Error trimming code block from LLM response for agent '" +
                     name + "': " + e.what());
      finalAgentResponse = llmResponseJson;
      setSkipFlowIteration(true);
      continue;
    }
    addToHistory("Agent: " + this->getName(), llmResponseJson);

    std::string status;
    std::vector<StructuredThought> thoughts;
    std::vector<ActionInfo> actions;
    std::string llmFinalResponseField;

    // Call the *newly declared* parser
    bool parsedSuccessfully = parseStructuredLLMResponse(
        llmResponseJson, status, thoughts, actions, llmFinalResponseField);

    if (!parsedSuccessfully) {
      logMessage(LogLevel::ERROR,
                 "Failed to parse structured LLM response for agent '" + name +
                     "'. Treating raw response as final.",
                 llmResponseJson);
      finalAgentResponse = llmResponseJson;
      setSkipFlowIteration(true);
      continue;
    }

    for (const auto &thought : thoughts) {
      logMessage(LogLevel::DEBUG,
                 "LLM Thought (" + thought.type + "):", thought.content);
    }

    if (status == "REQUIRES_CLARIFICATION") {
      logMessage(LogLevel::INFO,
                 "LLM requires clarification for agent '" + name + "'");
      finalAgentResponse = "Agent needs clarification."; // Default
      for (const auto &action : actions) {
        if (action.action == "request_user_input" &&
            action.params.isMember("query_to_user") &&
            action.params["query_to_user"].isString()) {
          finalAgentResponse = action.params["query_to_user"].asString();
          break;
        }
      }
      setSkipFlowIteration(true);
    } else if (status == "REQUIRES_ACTION" && !actions.empty()) {
      logMessage(LogLevel::INFO,
                 "LLM requested actions for agent '" + name + "'");
      std::string actionResults =
          processActions(actions);         // Use the new function
      addToHistory("user", actionResults); // Feed results back
    } else if (status == "SUCCESS_FINAL") {
      logMessage(LogLevel::INFO,
                 "LLM provided final response for agent '" + name + "'");
      finalAgentResponse =
          llmFinalResponseField; // Default to final_response field
      // Allow first 'send_response' action to override if present (and
      // final_response was empty/null)
      for (const auto &action : actions) {
        if (action.action == "send_response") {
          if (finalAgentResponse.empty()) { // Only override if final_response
                                            // wasn't explicitly set
            finalAgentResponse =
                processAction(action); // Process and use this as final
          } else {
            logMessage(LogLevel::WARN,
                       "LLM has status SUCCESS_FINAL and non-empty "
                       "final_response, ignoring send_response action.",
                       action.params.toStyledString());
          }
          break; // Only handle first send_response as final here
        }
      }
      setSkipFlowIteration(true);
    } else if (status == "ERROR_INTERNAL") {
      logMessage(LogLevel::ERROR,
                 "LLM indicated an internal error for agent '" + name + "'");
      finalAgentResponse = llmFinalResponseField.empty()
                               ? "LLM reported an internal error."
                               : llmFinalResponseField;
      setSkipFlowIteration(true);
    } else {
      logMessage(LogLevel::WARN,
                 "LLM response has unclear status/action state for agent '" +
                     name + "'. Treating as final.",
                 "Status: " + status);
      finalAgentResponse = llmFinalResponseField.empty()
                               ? llmResponseJson
                               : llmFinalResponseField; // Use raw if empty
      setSkipFlowIteration(true);
    }
  } // End while loop

  if (iteration >= iterationCap &&
      !skipFlowIteration) { // Check skipFlowIteration flag
    logMessage(LogLevel::WARN,
               "Iteration limit reached for agent '" + name + "'",
               std::to_string(iterationCap));
    if (finalAgentResponse.empty()) {
      // Try to get last sensible model response from history
      finalAgentResponse = "Agent reached iteration limit (" +
                           std::to_string(iterationCap) +
                           ") without a conclusive response.";
      for (auto it = history.rbegin(); it != history.rend(); ++it) {
        if (it->first == "model") {
          if (it->second.find("<action_results>") == std::string::npos &&
              it->second.find("\"actions\": [") == std::string::npos) {
            finalAgentResponse = it->second; // Use last model message not
                                             // containing results/actions
            break;
          }
        }
      }
    }
  }

  setSkipFlowIteration(false);
  logMessage(LogLevel::INFO,
             "Final response for agent '" + name + "':", finalAgentResponse);
  return finalAgentResponse;
}

// --- Prompt Building ---
std::string Agent::buildFullPrompt() const {
  std::stringstream promptStream;
  // 1. System Prompt
  if (!systemPrompt.empty()) {
    promptStream << "<system_prompt>\n";
    promptStream << systemPrompt << "\n";
    promptStream << "</system_prompt>\n\n";
  }

  // agent schema
  if (!schema.empty()) {
    promptStream << "<agent_reply_schema>\n";
    promptStream << "\t" << schema << "\n";
    promptStream << "</agent_reply_schema>\n\n";
  }
  if (!example.empty()) {
    promptStream << "<agent_reply_example>\n";
    promptStream << "\t" << example << "\n";
    promptStream << "</agent_reply_example>\n\n";
  }
  // 2. Agent Info
  promptStream << "<agent_context>\n";
  if (!name.empty()) {
    promptStream << "\t<name>" << name << "</name>\n";
  }
  if (!description.empty()) {
    promptStream << "\t<description>" << description << "</description>\n";
  }
  promptStream << "</agent_context>\n\n";
  // 3. Environment Variables
  if (!env.empty()) {
    promptStream << "<environment_variables>\n";
    for (const auto &pair : env) {
      promptStream << "\t<variable name=\"" << pair.first << "\">"
                   << pair.second << "</variable>\n";
    }
    promptStream << "</environment_variables>\n\n";
  }
  // 4. Available Tools/Actions
  std::map<std::string, std::string> availableActions =
      internalToolDescriptions;
  for (const auto &pair : tools) {
    if (pair.second) {
      availableActions[pair.first] = pair.second->getDescription();
    }
  }
  if (!availableActions.empty()) {
    promptStream << "<available_actions>\n";
    for (const auto &pair : availableActions) {
      promptStream << "\t<action name=\"" << pair.first << "\">\n";
      promptStream << "\t\t<description>" << pair.second << "</description>\n";
      promptStream << "\t</action>\n";
    }
    promptStream << "</available_actions>\n\n";
  }
  // 5. Directive
  if (directive.type != DIRECTIVE::NORMAL || !directive.description.empty()) {
    promptStream << "<current_directive>\n";
    promptStream << "\t<type>" << directiveTypeToString(directive.type)
                 << "</type>\n";
    if (!directive.description.empty())
      promptStream << "\t<description>" << directive.description
                   << "</description>\n";
    if (!directive.format.empty())
      promptStream << "\t<expected_format>" << directive.format
                   << "</expected_format>\n";
    promptStream << "</current_directive>\n\n";
  }
  // 6. Extra Prompts
  if (!extraPrompts.empty()) {
    promptStream << "<additional_instructions>\n";
    for (const auto &p : extraPrompts) {
      promptStream << "\t" << p << "\n";
    }
    promptStream << "</additional_instructions>\n\n";
  }
  // 7. Memory
  std::string memoryContent;
  if (!shortTermMemory.empty()) {
    memoryContent += "\t<short_term>\n";
    for (const auto &pair : shortTermMemory)
      memoryContent +=
          "\t\t<item key=\"" + pair.first + "\">" + pair.second + "</item>\n";
    memoryContent += "\t</short_term>\n";
  }
  if (!longTermMemory.empty()) {
    memoryContent += "\t<long_term>\n";
    for (const auto &pair : longTermMemory)
      memoryContent +=
          "\t\t<item role=\"" + pair.first + "\">" + pair.second + "</item>\n";
    memoryContent += "\t</long_term>\n";
  }
  if (!scratchpad.empty()) {
    memoryContent += "\t<scratchpad>\n";
    for (const auto &pair : scratchpad)
      memoryContent +=
          "\t\t<item key=\"" + pair.first + "\">" + pair.second + "</item>\n";
    memoryContent += "\t</scratchpad>\n";
  }
  if (!memoryContent.empty()) {
    promptStream << "<memory_context>\n"
                 << memoryContent << "</memory_context>\n\n";
  }
  // 8. Conversation History
  if (!history.empty()) {
    promptStream << "<conversation_history>\n";
    for (const auto &entry : history) {
      promptStream << "\t<turn role=\"" << entry.first << "\">\n";
      std::stringstream ss_content(entry.second);
      std::string line;
      while (std::getline(ss_content, line)) {
        promptStream << "\t\t" << line << "\n";
      }
      promptStream << "\t</turn>\n";
    }
    promptStream << "</conversation_history>\n\n";
  }
  promptStream << "CONTEXT END\n\n";
  // 9. Final Instruction
  promptStream
      << "RESPONSE INSTRUCTIONS: Generate your response strictly as a single "
         "JSON object containing 'status', 'thoughts' (structured array), "
         "'actions' (array, use '[]' if none), and potentially "
         "'final_response' (usually null/empty if actions are present). Base "
         "your response on the entire context provided above.";
  std::cout << "DEBUG: ++++++++++++++++++++++++++++ \n" + promptStream.str()
            << std::endl;
  return promptStream.str();
}
