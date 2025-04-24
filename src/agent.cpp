
#include "../inc/Agent.hpp"
#include "../inc/MiniGemini.hpp"
#include "../inc/Tool.hpp"
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
#include "../inc/Utils.hpp"   // For fileList, fileExists

#include <string>
#include <sys/stat.h> // For file operations
#include <unistd.h>   // For getcwd

#include "../inc/Agent.hpp"
#include "../inc/MiniGemini.hpp"
#include "../inc/Tool.hpp"
#include "../inc/modelApi.hpp" // For ApiError

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <json/json.h>
#include <memory> // For unique_ptr in parsing
#include <sstream>
#include <stdexcept>

// --- Logging Function ---
// (Assuming definition exists as provided previously)
// void logMessage(LogLevel level, const std::string &message, const std::string
// &details = "");

// Simple colored logging function
void logMessage(LogLevel level, const std::string &message,
                const std::string &details) {
  // Optimization: Skip DEBUG messages unless explicitly enabled (e.g., via
  // flag) if (level == LogLevel::DEBUG && !debug_logging_enabled) return;
  // Optimization: Skip PROMPT details unless explicitly enabled
  // if (level == LogLevel::PROMPT && details.empty() &&
  // !prompt_logging_enabled) return;

  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::localtime(&now_c); // Use localtime for local timezone

  char time_buffer[20]; // HH:MM:SS
  std::strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", &now_tm);

  std::string prefix;
  std::string color_start = "";
  std::string color_end = "\033[0m"; // ANSI Reset color

  switch (level) {
  case LogLevel::DEBUG:
    prefix = "[DEBUG] ";
    color_start = "\033[36m";
    break; // Cyan
  case LogLevel::INFO:
    prefix = "[INFO]  ";
    color_start = "\033[32m";
    break; // Green
  case LogLevel::WARN:
    prefix = "[WARN]  ";
    color_start = "\033[33m";
    break; // Yellow
  case LogLevel::ERROR:
    prefix = "[ERROR] ";
    color_start = "\033[1;31m";
    break; // Bold Red
  case LogLevel::TOOL_CALL:
    prefix = "[TOOL CALL] ";
    color_start = "\033[1;35m";
    break; // Bold Magenta
  case LogLevel::TOOL_RESULT:
    prefix = "[TOOL RESULT] ";
    color_start = "\033[35m";
    break; // Magenta
  case LogLevel::PROMPT:
    prefix = "[PROMPT] ";
    color_start = "\033[34m";
    break; // Blue
  }

  // Use cerr for errors and warnings, cout for others
  std::ostream &out = (level == LogLevel::ERROR || level == LogLevel::WARN)
                          ? std::cerr
                          : std::cout;

  out << color_start << std::string(time_buffer) << " " << prefix << message
      << color_end << std::endl;
  if (!details.empty()) {
    // Indent details slightly for readability
    // Limit detail length in logs to prevent flooding
    const size_t max_detail_len = 500;
    std::string truncated_details = details.substr(0, max_detail_len);
    if (details.length() > max_detail_len) {
      truncated_details += "... (truncated)";
    }
    out << color_start << "  " << truncated_details << color_end << std::endl;
  }
}

// --- Agent Implementation ---

Agent::Agent(MiniGemini &api)
    : m_api(api),
      // Default prompt emphasizes structured JSON output
      m_systemPrompt(R"(SYSTEM PROMPT: Base Agent

**Role:** Process user requests, utilize tools, and provide responses.

**Core Interaction Model:**
You MUST respond with a single JSON object containing the following fields:
1.  `thought`: (String) Your reasoning, analysis, and plan.
2.  `tool_calls`: (Array of Objects | null) Tools to execute now, each as `{"tool_name": "...", "params": { ... }}`. Use `null` or `[]` if none.
3.  `final_response`: (String | null) The final response to the user. Provide ONLY when the task is complete. Set to `null` otherwise.

Adhere strictly to this JSON format.
)"),
      iteration(0), iterationCap(10), skipFlowIteration(false),
      _name("default_agent") {
  std::srand(static_cast<unsigned int>(std::time(nullptr)));

  // Initialize internal tool descriptions
  m_internalToolDescriptions["help"] =
      "Provides descriptions of available tools. Parameters: {\"tool_name\": "
      "\"string\" (optional)}";
  m_internalToolDescriptions["skip"] =
      "Skips the final response generation for the current turn. Parameters: "
      "{\"skip\": boolean (required, usually true)}";
  m_internalToolDescriptions["promptAgent"] =
      "Sends a prompt to another registered agent. Parameters: {\"name\": "
      "\"string\" (agent name), \"prompt\": \"string\"}";
  m_internalToolDescriptions["summarizeTool"] =
      "Summarizes provided text content. Parameters: {\"content\": \"string\"}";
  m_internalToolDescriptions["summarizeHistory"] =
      "Summarizes the current conversation history. Parameters: {}";
  m_internalToolDescriptions["getWeather"] =
      "Fetches current weather. Parameters: {\"location\": \"string\"}";
}

void Agent::setSystemPrompt(const std::string &prompt) {
  m_systemPrompt = prompt;
  if (m_systemPrompt.find("\"thought\"") == std::string::npos ||
      m_systemPrompt.find("\"tool_calls\"") == std::string::npos) {
    m_systemPrompt += R"(

Reminder: ALWAYS respond with a single JSON object containing 'thought' (string), 'tool_calls' (array of tool objects or null), and 'final_response' (string or null).
)";
  }
}

void Agent::setName(const std::string &name) { _name = name; }
void Agent::setDescription(const std::string &description) {
  _description = description;
}
void Agent::setIterationCap(int cap) { iterationCap = (cap > 0) ? cap : 1; }
const std::string Agent::getName() const { return _name; }
const std::string Agent::getDescription() const { return _description; }
MiniGemini &Agent::getApi() { return m_api; }
fileList Agent::getFiles() const { return _files; }
const std::vector<std::pair<std::string, std::string>> &
Agent::getHistory() const {
  return m_history;
}

void Agent::addTool(Tool *tool) {
  if (!tool) {
    logMessage(LogLevel::WARN, "Attempted to add a null tool.");
    return;
  }
  const std::string &toolName = tool->getName();
  if (toolName.empty()) {
    logMessage(LogLevel::WARN, "Attempted to add a tool with an empty name.");
    return;
  }
  if (m_tools.count(toolName) || m_internalToolDescriptions.count(toolName)) {
    logMessage(LogLevel::WARN, "Attempted to add tool with duplicate name: '" +
                                   toolName + "'. Ignoring.");
  } else {
    m_tools.insert(std::make_pair(toolName, tool));
    logMessage(LogLevel::INFO,
               "Agent '" + _name + "' added tool: '" + toolName + "'");
  }
}

void Agent::removeTool(const std::string &toolName) {
  if (m_tools.erase(toolName)) {
    logMessage(LogLevel::INFO,
               "Agent '" + _name + "' removed tool: '" + toolName + "'");
  } else {
    logMessage(LogLevel::WARN,
               "Agent '" + _name +
                   "' attempted to remove non-existent tool: '" + toolName +
                   "'");
  }
}

Tool *Agent::getTool(const std::string &toolName) const {
  auto it = m_tools.find(toolName);
  return (it != m_tools.end()) ? it->second : nullptr;
}

Tool *Agent::getRegisteredTool(const std::string &toolName) const {
  auto it = m_tools.find(toolName);
  if (it != m_tools.end()) {
    return it->second;
  }
  return nullptr;
}

std::string
Agent::getInternalToolDescription(const std::string &toolName) const {
  auto it = m_internalToolDescriptions.find(toolName);
  return (it != m_internalToolDescriptions.end()) ? it->second : "";
}

void Agent::reset() {
  m_history.clear();
  Scratchpad.clear();
  iteration = 0;
  skipFlowIteration = false;
  logMessage(LogLevel::INFO, "Agent '" + _name + "' reset.");
}

// Helper to execute external commands
int executeCommand(const std::string &command, std::string &output) {
  output.clear();
  std::string cmd_with_stderr = command + " 2>&1";
  FILE *pipe = popen(cmd_with_stderr.c_str(), "r");
  if (!pipe) {
    logMessage(LogLevel::ERROR, "popen() failed for command:", command);
    return -1;
  }
  char buffer[256];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    output += buffer;
  }
  int status = pclose(pipe);
  return WEXITSTATUS(status);
}

std::string Agent::buildFullPrompt() const {
  std::ostringstream oss;
  // --- Context Management ---
  // TODO (Phase 2+): Implement History Summarization & Memory Retrieval

  oss << m_systemPrompt << "\n\n";

  if (!extraPrompts.empty()) {
    oss << "Additional Instructions:\n";
    for (const auto &extraPrompt : extraPrompts)
      oss << "- " << extraPrompt << "\n";
    oss << "\n";
  }

  oss << "Agent Information:\n- Name: " << _name << "\n";
  if (!_description.empty())
    oss << "- Description: " << _description << "\n";
  oss << "\n";

  if (!_env.empty()) {
    oss << "Environment Variables:\n";
    for (const auto &pair : _env)
      oss << "- " << pair.first << "=" << pair.second << "\n";
    oss << "\n";
  }

  if (!agents.empty()) {
    oss << "Available Sub-Agents (Interact using 'promptAgent' tool):\n";
    for (const auto &pair : agents)
      oss << "- Name: " << pair.first
          << " (Description: " << pair.second->getDescription() << ")\n";
    oss << "\n";
  }

  bool hasTools = !m_tools.empty() || !m_internalToolDescriptions.empty();
  if (hasTools) {
    oss << "Available Tools (Use STRICT JSON format: {\"tool_name\": \"...\", "
           "\"params\": {...}} within 'tool_calls'):\n";
    for (const auto &pair : m_internalToolDescriptions)
      oss << "- " << pair.first << ": " << pair.second << "\n";
    for (const auto &pair : m_tools)
      oss << "- " << pair.second->getName() << ": "
          << pair.second->getDescription() << "\n";
    oss << "\n";
  }

  if (!m_history.empty()) {
    oss << "Conversation History (Recent relevant context):\n";
    // TODO (Phase 2+): Only include relevant/summarized history
    for (const auto &entry : m_history) {
      std::string role = entry.first;
      if (!role.empty())
        role[0] = std::toupper(static_cast<unsigned char>(role[0]));
      oss << role << ": " << entry.second << "\n";
    }
    oss << "\n";
  }

  oss << "Assistant Response (Provide JSON with 'thought', 'tool_calls', "
         "'final_response'):";

  logMessage(LogLevel::PROMPT, "Full prompt built for Agent '" + _name + "'");
  return oss.str();
}

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

std::string Agent::handleToolExecution(const ToolCallInfo &call) {
  logMessage(LogLevel::TOOL_CALL, "Executing tool '" + call.toolName + "'",
             "Params: " + call.params.toStyledString());
  std::string result;
  try {
    // Internal Tools
    if (call.toolName == "help")
      result = this->getHelp(call.params);
    else if (call.toolName == "skip")
      result = skip(call.params);
    else if (call.toolName == "promptAgent")
      result = promptAgentTool(call.params);
    else if (call.toolName == "summarizeTool")
      result = summerizeTool(call.params);
    else if (call.toolName == "summarizeHistory")
      result = summarizeHistory(call.params);
    else if (call.toolName == "getWeather")
      result = getWeather(call.params);
    // External Tools
    else {
      Tool *tool = getTool(call.toolName);
      if (tool) {
        result = tool->execute(call.params);
      } else {
        logMessage(LogLevel::WARN, "Attempted to execute unknown tool: '" +
                                       call.toolName + "'");
        result = "Error: Unknown tool '" + call.toolName + "'.";
      }
    }
    logMessage(LogLevel::TOOL_RESULT, "Tool '" + call.toolName + "' executed.",
               "Result: " + result);
  } catch (const std::exception &e) {
    result = "Error executing tool '" + call.toolName + "': " + e.what();
    logMessage(LogLevel::ERROR,
               "Exception during tool execution for '" + call.toolName + "'.",
               "Error: " + std::string(e.what()));
  } catch (...) {
    result = "Error: Unknown exception executing tool '" + call.toolName + "'";
    logMessage(LogLevel::ERROR,
               "Unknown exception during tool execution for '" + call.toolName +
                   "'.");
  }
  return result;
}

std::string
Agent::processToolCalls(const std::vector<ToolCallInfo> &toolCalls) {
  if (toolCalls.empty())
    return "";
  std::ostringstream toolResultsOss;
  toolResultsOss << "Tool Execution Results:\n";
  for (const auto &call : toolCalls) {
    std::string result = handleToolExecution(call);
    toolResultsOss << "--- Tool: " << call.toolName << " ---\n"
                   << "Params: " << call.params.toStyledString()
                   << "Result: " << result << "\n";
  }
  toolResultsOss << "--- End Tool Results ---";
  return toolResultsOss.str();
}

std::string Agent::executeApiCall(const std::string &fullPrompt) {
  logMessage(LogLevel::PROMPT,
             "Sending prompt to API for agent '" + _name + "'");
  try {
    std::string response = m_api.generate(fullPrompt);
    logMessage(LogLevel::DEBUG,
               "Received API response for agent '" + _name + "'");
    return response;
  } catch (const ApiError &e) {
    logMessage(LogLevel::ERROR,
               "API Error occurred for agent '" + _name + "':", e.what());
    throw;
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR,
               "Standard exception during API call for agent '" + _name + "':",
               e.what());
    throw std::runtime_error("Error during API call: " + std::string(e.what()));
  } catch (...) {
    logMessage(LogLevel::ERROR,
               "Unknown exception during API call for agent '" + _name + "'.");
    throw std::runtime_error("Unknown error during API call");
  }
}

// ** UPDATED Agent::prompt using structured JSON approach **
std::string Agent::prompt(const std::string &userInput) {
  addToHistory("Master", userInput);
  logMessage(LogLevel::INFO, "User Input received by Agent '" + _name + "'",
             userInput);

  std::string rawLLMResponse = "";
  std::string finalResponseToUser = "";
  std::string thought = "";
  std::vector<ToolCallInfo> toolCalls;
  bool taskComplete = false;

  iteration = 0; // Reset iteration count for new prompt

  // --- Enhanced Agent Loop ---
  for (; iteration < iterationCap && !taskComplete; ++iteration) {
    logMessage(LogLevel::DEBUG, "Agent '" + _name + "' - Iteration " +
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
    addToHistory(this->_name, rawLLMResponse);

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
    logMessage(LogLevel::WARN, "Agent '" + _name + "' reached iteration cap (" +
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

  logMessage(LogLevel::INFO, "Final response generated by Agent '" + _name +
                                 "' after " + std::to_string(iteration) +
                                 " iteration(s).");
  return finalResponseToUser;
}

// --- Other Methods (getHelp, skip, promptAgentTool, etc.) ---
// (Implementations remain the same as previously generated,
//  assuming they handle JSON parameters correctly)

// Provides help/descriptions for available tools.
std::string Agent::getHelp(const Json::Value &params) {
  std::ostringstream helpOss;
  std::string specificTool;

  if (params.isMember("tool_name") && params["tool_name"].isString()) {
    specificTool = params["tool_name"].asString();
  }

  if (!specificTool.empty()) {
    helpOss << "Help for tool '" << specificTool << "':\n";
    bool found = false;
    auto internalIt = m_internalToolDescriptions.find(specificTool);
    if (internalIt != m_internalToolDescriptions.end()) {
      helpOss << "- Type: Internal\n";
      helpOss << "- Description & Params: " << internalIt->second;
      found = true;
    }
    Tool *tool = getTool(specificTool);
    if (tool) {
      if (found)
        helpOss << "\n---\n";
      helpOss << "- Type: External\n";
      helpOss << "- Description: " << tool->getDescription();
      helpOss << "\n" << tool->getAllUseCaseCap(2); // Show examples
      found = true;
    }
    if (!found) {
      helpOss
          << "Tool '" << specificTool
          << "' not found. Use 'help' with no parameters to list all tools.";
    }
  } else {
    helpOss << "Available Tools:\n";
    helpOss << "--- Internal Tools ---\n";
    for (const auto &pair : m_internalToolDescriptions) {
      helpOss << "- " << pair.first << ": " << pair.second << "\n";
    }
    helpOss << "\n--- External Tools ---\n";
    if (m_tools.empty()) {
      helpOss << "(No external tools registered)\n";
    } else {
      for (const auto &pair : m_tools) {
        helpOss << "- " << pair.second->getName() << ": "
                << pair.second->getDescription() << "\n";
      }
    }
    helpOss << "\nUse help with {\"tool_name\": \"<tool_name>\"} for details.";
  }
  return helpOss.str();
}

std::string Agent::skip(const Json::Value &params) {
  bool doSkip = false;
  if (params.isMember("skip") && params["skip"].isBool()) {
    doSkip = params["skip"].asBool();
  } else {
    return "Error [skip]: Missing or invalid boolean parameter 'skip'. "
           "Example: {\"skip\": true}";
  }

  if (doSkip) {
    this->setSkipFlowIteration(true);
    return "Success [skip]: Final response generation for this turn will be "
           "skipped.";
  } else {
    this->setSkipFlowIteration(false);
    return "Success [skip]: Final response generation will proceed normally.";
  }
}

std::string Agent::promptAgentTool(const Json::Value &params) {
  if (!params.isMember("name") || !params["name"].isString() ||
      !params.isMember("prompt") || !params["prompt"].isString()) {
    return "Error [promptAgent]: Requires string parameters 'name' (target "
           "agent) and 'prompt'.";
  }
  std::string agentName = params["name"].asString();
  std::string userInput = params["prompt"].asString();

  logMessage(LogLevel::INFO,
             "Agent '" + _name + "' is prompting agent '" + agentName + "'");

  Agent *targetAgent = nullptr;
  for (auto &agentPair : agents) {
    if (agentPair.first == agentName) {
      targetAgent = agentPair.second;
      break;
    }
  }

  if (targetAgent) {
    try {
      std::string contextualPrompt =
          "Received prompt from Agent '" + this->_name + "':\n" + userInput;
      std::string response = targetAgent->prompt(contextualPrompt);
      logMessage(LogLevel::INFO,
                 "Received response from agent '" + agentName + "'");
      return "Response from Agent '" + agentName + "':\n---\n" + response +
             "\n---";
    } catch (const std::exception &e) {
      logMessage(LogLevel::ERROR, "Error prompting agent '" + agentName + "'",
                 e.what());
      return "Error [promptAgent]: Exception occurred while prompting agent '" +
             agentName + "': " + e.what();
    }
  } else {
    logMessage(LogLevel::WARN,
               "Agent '" + agentName + "' not found for prompting.");
    return "Error [promptAgent]: Agent '" + agentName + "' not found.";
  }
}

std::string Agent::summerizeTool(const Json::Value &params) {
  if (!params.isMember("content") || !params["content"].isString()) {
    return "Error [summarizeTool]: Missing or invalid string parameter "
           "'content'.";
  }
  std::string content = params["content"].asString();
  if (content.length() < 50) {
    return "Content is too short to summarize effectively.";
  }
  logMessage(LogLevel::DEBUG, "Summarizing content (length: " +
                                  std::to_string(content.length()) + ")");
  try {
    std::string task =
        "Provide a concise summary of the following text:\n\n" + content;
    std::string format = "{\"summary\": \"string (concise summary)\"}";
    std::string llmResponse = executeTask(task, format);

    Json::Value summaryJson;
    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
    std::string errs;
    if (reader->parse(llmResponse.c_str(),
                      llmResponse.c_str() + llmResponse.length(), &summaryJson,
                      &errs) &&
        summaryJson.isObject() && summaryJson.isMember("summary") &&
        summaryJson["summary"].isString()) {
      return summaryJson["summary"].asString();
    } else {
      logMessage(
          LogLevel::WARN,
          "Failed to parse summary JSON from LLM, returning raw response.",
          "LLM Response: " + llmResponse);
      return llmResponse;
    }
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR, "Error during summarization task execution",
               e.what());
    return "Error [summarizeTool]: Exception during summarization: " +
           std::string(e.what());
  }
}

std::string Agent::summarizeHistory(const Json::Value &params) {
  (void)params;
  if (m_history.empty())
    return "Conversation history is empty.";
  std::string historyText = "Conversation History:\n";
  for (const auto &entry : m_history)
    historyText += entry.first + ": " + entry.second + "\n";
  Json::Value summarizeParams;
  summarizeParams["content"] = historyText;
  return summerizeTool(summarizeParams);
}

std::string Agent::getWeather(const Json::Value &params) {
  if (!params.isMember("location") || !params["location"].isString()) {
    return "Error [getWeather]: Missing or invalid string parameter "
           "'location'.";
  }
  std::string location = params["location"].asString();
  std::string originalLocation = location;
  std::replace(location.begin(), location.end(), ' ', '+');
  std::string command =
      "curl -s -L \"https://wttr.in/" + location + "?format=3\"";
  std::string weatherResult;
  int status = executeCommand(command, weatherResult);
  weatherResult.erase(0, weatherResult.find_first_not_of(" \t\r\n"));
  weatherResult.erase(weatherResult.find_last_not_of(" \t\r\n") + 1);
  if (status != 0 || weatherResult.empty() ||
      weatherResult.find("Unknown location") != std::string::npos ||
      weatherResult.find("ERROR") != std::string::npos ||
      weatherResult.find("Sorry") != std::string::npos) {
    logMessage(LogLevel::WARN, "Failed to get weather using wttr.in",
               "Command: " + command + ", Status: " + std::to_string(status) +
                   ", Output: " + weatherResult);
    return "Error [getWeather]: Could not retrieve weather for '" +
           originalLocation + "'.";
  }
  return "Current weather for " + originalLocation + ": " + weatherResult;
}

// --- Utility Methods ---

std::string Agent::generateStamp(void) {
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::localtime(&now_c);
  // *** FIX: Declare buffer as a char array ***
  char buffer[80]; // Increased size to safely hold the timestamp
  // Use strftime correctly with the buffer array
  if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S%Z", &now_tm)) {
    // *** FIX: Construct string from the null-terminated buffer ***
    return std::string(buffer);
  } else {
    // Handle error if strftime fails
    return "[Timestamp Error]";
  }
}

void Agent::addToHistory(const std::string &role, const std::string &content) {
  const size_t max_history_content_len = 2500;
  std::string processed_content = content.substr(0, max_history_content_len);
  bool truncated = (content.length() > max_history_content_len);
  if (truncated)
    processed_content += "... (truncated)";
  m_history.push_back(std::make_pair(role, processed_content));
  logMessage(LogLevel::DEBUG, "Added to history",
             "Role: " + role + (truncated ? " (Content Truncated)" : ""));
}

void Agent::addMemory(const std::string &role, const std::string &content) {
  LongTermMemory.push_back(std::make_pair(role, content));
  logMessage(LogLevel::DEBUG, "Added to LongTermMemory", "Role: " + role);
}

void Agent::removeMemory(const std::string &role, const std::string &content) {
  auto it = std::remove_if(LongTermMemory.begin(), LongTermMemory.end(),
                           [&role, &content](const auto &p) {
                             return p.first == role && p.second == content;
                           });
  if (it != LongTermMemory.end()) {
    LongTermMemory.erase(it, LongTermMemory.end());
    logMessage(LogLevel::DEBUG, "Removed from LongTermMemory", "Role: " + role);
  }
}

void Agent::addEnvVar(const std::string &key, const std::string &value) {
  bool found = false;
  for (auto &pair : _env) {
    if (pair.first == key) {
      pair.second = value;
      found = true;
      break;
    }
  }
  if (!found)
    _env.push_back(std::make_pair(key, value));
  logMessage(LogLevel::DEBUG, "Set environment variable", key + "=" + value);
}
void Agent::importEnvFile(const std::string &filePath) {
  std::ifstream file(filePath);
  if (!file.is_open()) {
    logMessage(LogLevel::ERROR, "Could not open environment file:", filePath);
    return;
  }
  std::string line;
  int count = 0;
  while (std::getline(file, line)) {
    line.erase(0, line.find_first_not_of(" \t"));
    // *** FIX: Check the first character of the string for '#' ***
    if (line.empty() || line[0] == '#')
      continue; // Corrected check
    size_t pos = line.find('=');
    if (pos != std::string::npos) {
      std::string key = line.substr(0, pos);
      key.erase(key.find_last_not_of(" \t") + 1);
      std::string value = line.substr(pos + 1);
      value.erase(0, value.find_first_not_of(" \t"));
      value.erase(value.find_last_not_of(" \t") + 1);
      if (value.length() >= 2 &&
          ((value.front() == '"' && value.back() == '"') ||
           (value.front() == '\'' && value.back() == '\''))) {
        value = value.substr(1, value.length() - 2);
      }
      if (!key.empty()) {
        addEnvVar(key, value);
        count++;
      }
    }
  }
  file.close();
  logMessage(LogLevel::INFO,
             "Imported " + std::to_string(count) +
                 " environment variables from:",
             filePath);
}

void Agent::addPrompt(const std::string &prompt) {
  extraPrompts.push_back(prompt);
  logMessage(LogLevel::DEBUG, "Added extra prompt instruction.");
}

void Agent::addAgent(Agent *agent) {
  if (!agent) {
    logMessage(LogLevel::WARN, "Attempted to add a null agent.");
    return;
  }
  for (const auto &pair : agents) {
    if (pair.first == agent->getName()) {
      logMessage(LogLevel::WARN,
                 "Attempted to add agent with duplicate name: '" +
                     agent->getName() + "'. Ignoring.");
      return;
    }
  }
  agents.push_back(std::make_pair(agent->getName(), agent));
  logMessage(LogLevel::INFO, "Agent '" + _name + "' registered sub-agent: '" +
                                 agent->getName() + "'");
}

std::string Agent::agentInstance(const std::string &name) {
  for (const auto &pair : agents)
    if (pair.first == name)
      return pair.second->getName();
  return "";
}

void Agent::setSkipFlowIteration(bool skip) { skipFlowIteration = skip; }

// --- Interactive Loop ---
void Agent::run() {
  logMessage(LogLevel::INFO,
             "Agent '" + _name + "' starting interactive loop.");
  logMessage(LogLevel::INFO,
             "Type 'exit' or 'quit' to stop, 'reset' to clear history.");
  std::string userInput;

  if (!initialCommands.empty()) {
    logMessage(LogLevel::INFO,
               "Executing initial commands for agent '" + _name + "'...");
    Json::Value bashParams;
    for (const auto &command : initialCommands) {
      bashParams["command"] = command;
      logMessage(LogLevel::INFO, "Running initial command:", command);
      std::string result = manualToolCall("bash", bashParams);
      logMessage(LogLevel::INFO, "Initial command result:", result);
    }
    initialCommands.clear();
  }

  while (true) {
    std::cout << "\nMaster (" << _name << ") > ";
    if (!std::getline(std::cin, userInput)) {
      logMessage(LogLevel::INFO,
                 "Input stream closed (EOF). Exiting agent '" + _name + "'.");
      break;
    }
    userInput.erase(0, userInput.find_first_not_of(" \t\r\n"));
    userInput.erase(userInput.find_last_not_of(" \t\r\n") + 1);
    if (userInput == "exit" || userInput == "quit") {
      logMessage(LogLevel::INFO,
                 "Exit command received for agent '" + _name + "'. Goodbye!");
      break;
    } else if (userInput == "reset") {
      reset();
      continue;
    } else if (userInput.empty()) {
      continue;
    }

    try {
      std::string response = prompt(userInput);
      std::cout << "\n" << _name << ":\n" << response << std::endl;
      std::cout << "-----------------------------------------" << std::endl;
    } catch (const ApiError &e) {
      logMessage(LogLevel::ERROR,
                 "API Error occurred during processing:", e.what());
      std::cout << "\n[Agent Error - API]: " << e.what() << std::endl;
      std::cout << "-----------------------------------------" << std::endl;
    } catch (const std::exception &e) {
      logMessage(LogLevel::ERROR,
                 "An error occurred during processing:", e.what());
      std::cout << "\n[Agent Error - General]: " << e.what() << std::endl;
      std::cout << "-----------------------------------------" << std::endl;
    } catch (...) {
      logMessage(LogLevel::ERROR,
                 "An unknown error occurred during processing.");
      std::cout << "\n[Agent Error - Unknown]: An unexpected error occurred."
                << std::endl;
      std::cout << "-----------------------------------------" << std::endl;
    }
  }
  logMessage(LogLevel::INFO,
             "Agent '" + _name + "' interactive loop finished.");
}

// --- Other method implementations ---
std::string Agent::executeTask(const std::string &task,
                               const std::string &format) {
  logMessage(LogLevel::DEBUG, "Executing task with specific format",
             "Task: " + task);
  std::string taskPrompt = "SYSTEM: Execute the following task and respond "
                           "ONLY in the specified format.\n";
  taskPrompt += "TASK:\n" + task + "\n\n";
  taskPrompt += "FORMAT:\n" + format + "\n";
  taskPrompt += "RESPONSE (JSON ONLY):";
  try {
    return executeApiCall(taskPrompt);
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR, "Failed to execute internal task via API",
               e.what());
    // Return error as JSON string
    Json::Value errorJson;
    errorJson["error"] =
        "Failed to execute internal task: " + std::string(e.what());
    Json::StreamWriterBuilder writer;
    writer["indentation"] = ""; // Compact
    return Json::writeString(writer, errorJson);
  }
}

std::string Agent::manualToolCall(const std::string &toolName,
                                  const Json::Value &params) {
  logMessage(LogLevel::INFO, "Manually calling tool '" + toolName +
                                 "' for agent '" + _name + "'");
  ToolCallInfo call;
  call.toolName = toolName;
  if (!params.isObject()) { // Ensure params is an object
    logMessage(LogLevel::ERROR,
               "Manual tool call parameters are not a JSON object.",
               params.toStyledString());
    return "Error: Manual tool call parameters must be a JSON object.";
  }
  call.params = params;
  return handleToolExecution(call);
}

Agent::DIRECTIVE &Agent::getDirective() { return directive; }
void Agent::setDirective(const DIRECTIVE &dir) { directive = dir; }
void Agent::addTask(const std::string &task) { tasks.push_back(task); }
void Agent::addInitialCommand(const std::string &command) {
  initialCommands.push_back(command);
}
std::string Agent::wrapText(const std::string &text) {
  return "```\n" + text + "\n```";
}
std::string Agent::wrapXmlLine(const std::string &content,
                               const std::string &tag) {
  return "<" + tag + ">" + content + "</" + tag + ">\n";
}
std::string Agent::wrapXmlBlock(const std::string &content,
                                const std::string &tag) {
  return "<" + tag + ">\n\t" + content + "\n</" + tag + ">\n";
}
void Agent::addBlockToSystemPrompt(const std::string &content,
                                   const std::string &tag) {
  m_systemPrompt += "\n" + wrapXmlBlock(content, tag);
}

// Placeholders
std::string Agent::reportTool(const Json::Value &params) {
  (void)params;
  logMessage(LogLevel::WARN, "'reportTool' not implemented.");
  return "Error: reportTool not implemented.";
}
std::string Agent::generateCode(const Json::Value &params) {
  (void)params;
  logMessage(LogLevel::WARN, "'generateCode' not implemented.");
  return "Error: generateCode not implemented.";
}
std::string Agent::auditResponse(const std::string &response) {
  (void)response;
  logMessage(LogLevel::WARN, "'auditResponse' not implemented.");
  return "Audit skipped.";
}
