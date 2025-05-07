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
  return history;
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
  history.clear();
  Scratchpad.clear();
  iteration = 0;
  skipFlowIteration = false;
  logMessage(LogLevel::INFO, "Agent '" + _name + "' reset.");
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

// --- Other Methods (getHelp, skip, promptAgentTool, etc.) ---
// (Implementations remain the same as previously generated,
//  assuming they handle JSON parameters correctly)





// --- Utility Methods ---


void Agent::addToHistory(const std::string &role, const std::string &content) {
  const size_t max_history_content_len = 2500;
  std::string processed_content = content.substr(0, max_history_content_len);
  bool truncated = (content.length() > max_history_content_len);
  if (truncated)
    processed_content += "... (truncated)";
  history.push_back(std::make_pair(role, processed_content));
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

