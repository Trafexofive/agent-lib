#include "../inc/Agent.hpp"      // Make sure this path is correct
#include "../inc/MiniGemini.hpp" // Make sure this path is correct
#include "../inc/Tool.hpp"       // Make sure this path is correct
#include "../inc/modelApi.hpp"   // For ApiError (ensure this exists)

#include <algorithm> // For std::find_if, std::remove_if
#include <chrono>    // For timestamp
#include <cstdlib>   // For system, getenv
#include <ctime>
#include <fstream> // For file operations
#include <iomanip> // For formatting time
#include <iostream>
#include <json/json.h> // Includes reader, writer, value
#include <map>
#include <memory> // For unique_ptr if used elsewhere
#include <sstream>
#include <stdexcept>
#include <vector>

// Simple colored logging function (assuming it's defined correctly, e.g., like
// previous versions) void logMessage(LogLevel level, const std::string
// &message, const std::string &details = "");
// --- Logging Function Placeholder ---
// Replace with your actual logging implementation
void logMessage(LogLevel level, const std::string &message,
                const std::string &details) {
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::localtime(&now_c);
  char time_buffer[20];
  std::strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", &now_tm);
  std::string prefix;
  std::string color_start = "";
  std::string color_end = "\033[0m";
  switch (level) {
  case LogLevel::DEBUG:
    prefix = "[DEBUG] ";
    color_start = "\033[36m";
    break;
  case LogLevel::INFO:
    prefix = "[INFO]  ";
    color_start = "\033[32m";
    break;
  case LogLevel::WARN:
    prefix = "[WARN]  ";
    color_start = "\033[33m";
    break;
  case LogLevel::ERROR:
    prefix = "[ERROR] ";
    color_start = "\033[1;31m";
    break;
  case LogLevel::TOOL_CALL:
    prefix = "[TOOL CALL] ";
    color_start = "\033[1;35m";
    break;
  case LogLevel::TOOL_RESULT:
    prefix = "[TOOL RESULT] ";
    color_start = "\033[35m";
    break;
  case LogLevel::PROMPT:
    prefix = "[PROMPT] ";
    color_start = "\033[34m";
    break;
  }
  std::ostream &out = (level == LogLevel::ERROR || level == LogLevel::WARN)
                          ? std::cerr
                          : std::cout;
  out << color_start << std::string(time_buffer) << " " << prefix << message
      << color_end << std::endl;
  if (!details.empty()) {
    const size_t max_detail_len = 500;
    std::string truncated_details = details.substr(0, max_detail_len);
    if (details.length() > max_detail_len)
      truncated_details += "... (truncated)";
    out << color_start << "  " << truncated_details << color_end << std::endl;
  }
}
// --- End Logging Function ---

// --- Agent Implementation ---

Agent::Agent(MiniGemini &apiRef)
    : api(apiRef), systemPrompt(R"(SYSTEM PROMPT: Base Agent

**Role:** Process user requests, utilize tools via actions, and provide responses.

**Interaction Model:**
You MUST respond with a single JSON object containing the following fields:
1.  `status`: (String - REQUIRED) Outcome hint: 'SUCCESS_FINAL', 'REQUIRES_ACTION', 'REQUIRES_CLARIFICATION', 'ERROR_INTERNAL'.
2.  `thoughts`: (Array of Objects - REQUIRED) Your structured reasoning, each object as `{"type": "TYPE", "content": "..."}`. Can be `[]`.
3.  `actions`: (Array of Objects - REQUIRED) Actions to execute now, each as `{"action": "...", "type": "...", "params": { ... }}`. MUST be `[]` if status is 'SUCCESS_FINAL'.
4.  `final_response`: (String | null - DEPRECATED?) Primarily use `send_response` action. Use this field ONLY if status is 'SUCCESS_FINAL' and 'actions' is `[]`. Set to `null` or `""` otherwise.

Adhere strictly to this JSON format (LLM Output Schema v0.3).
)"),
      iteration(0), iterationCap(10), skipFlowIteration(false),
      name("default_agent") {
  // Initialize internal tool descriptions
  internalToolDescriptions["help"] =
      "Provides descriptions of available tools/actions. Parameters: "
      "{\"action_name\": \"string\" (optional)}";
  internalToolDescriptions["skip"] =
      "Skips the final response generation for the current turn. Parameters: "
      "null (action implies skipping)";
  internalToolDescriptions["promptAgent"] =
      "Sends a prompt to another registered agent. Parameters: "
      "{\"agent_name\": \"string\", \"prompt\": \"string\"}";
  internalToolDescriptions["summarizeTool"] =
      "Summarizes provided text content. Parameters: {\"text\": \"string\"}";
  internalToolDescriptions["summarizeHistory"] =
      "Summarizes the current conversation history. Parameters: {}";
  internalToolDescriptions["getWeather"] =
      "Fetches current weather. Parameters: {\"location\": \"string\"}";
  // Add others as needed...
  logMessage(LogLevel::DEBUG, "Agent instance created", "Name: " + name);
}

// --- Configuration Setters ---
void Agent::setSystemPrompt(const std::string &prompt) {
  systemPrompt = prompt;
  // Optional: Add reminder check
  //   if (systemPrompt.find("\"status\"") == std::string::npos ||
  //       systemPrompt.find("\"thoughts\"") == std::string::npos ||
  //       systemPrompt.find("\"actions\"") == std::string::npos) {
  //     systemPrompt += R"(
  //
  // // Reminder: ALWAYS respond with a single JSON object matching the required
  // schema (status, thoughts, actions, final_response).
  // )";
  //   }
}
void Agent::setName(const std::string &newName) { name = newName; }
void Agent::setDescription(const std::string &newDescription) {
  description = newDescription;
}
void Agent::setIterationCap(int cap) { iterationCap = (cap > 0) ? cap : 1; }
void Agent::setDirective(const DIRECTIVE &dir) { directive = dir; }
void Agent::addTask(const std::string &task) { tasks.push_back(task); }
void Agent::addInitialCommand(const std::string &command) {
  initialCommands.push_back(command);
}

// --- Public Getters ---
const std::string Agent::getName() const { return name; }
const std::string Agent::getDescription() const { return description; }
MiniGemini &Agent::getApi() { return api; }
fileList Agent::getFiles() const { return files; }
const std::vector<std::pair<std::string, std::string>> &
Agent::getHistory() const {
  return history;
}
const std::string &Agent::getSystemPrompt() const { return systemPrompt; }
int Agent::getIterationCap() const { return iterationCap; }
const std::vector<std::pair<std::string, std::string>> &Agent::getEnv() const {
  return env;
}
const std::vector<std::string> &Agent::getExtraPrompts() const {
  return extraPrompts;
}
const std::vector<std::string> &Agent::getTasks() const { return tasks; }
const Agent::DIRECTIVE &Agent::getDirective() const { return directive; }

// --- Tool Management ---
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
  if (tools.count(toolName) || internalToolDescriptions.count(toolName)) {
    logMessage(LogLevel::WARN,
               "Attempted to add tool/action with duplicate name: '" +
                   toolName + "'. Ignoring.");
  } else {
    tools[toolName] = tool; // Use map assignment
    internalToolDescriptions[toolName] =
        tool->getDescription(); // Store description
    logMessage(LogLevel::INFO,
               "Agent '" + name + "' added tool: '" + toolName + "'");
  }
}

void Agent::removeTool(const std::string &toolName) {
  if (tools.erase(toolName)) {
    internalToolDescriptions.erase(toolName);
    logMessage(LogLevel::INFO,
               "Agent '" + name + "' removed tool: '" + toolName + "'");
  } else {
    logMessage(LogLevel::WARN,
               "Agent '" + name + "' attempted to remove non-existent tool: '" +
                   toolName + "'");
  }
}

Tool *Agent::getTool(const std::string &toolName) const {
  auto it = tools.find(toolName);
  return (it != tools.end()) ? it->second : nullptr;
}

Tool *Agent::getRegisteredTool(const std::string &toolName) const {
  auto it = tools.find(toolName);
  return (it != tools.end()) ? it->second : nullptr;
}

std::string
Agent::getInternalToolDescription(const std::string &toolName) const {
  auto it_internal = internalToolDescriptions.find(toolName);
  if (it_internal != internalToolDescriptions.end()) {
    return it_internal->second;
  }
  auto it_external = tools.find(toolName);
  if (it_external != tools.end() && it_external->second) {
    return it_external->second->getDescription();
  }
  return "";
}

// --- Core Agent Logic ---
void Agent::reset() {
  history.clear();
  scratchpad.clear();
  iteration = 0;
  skipFlowIteration = false;
  logMessage(LogLevel::INFO, "Agent '" + name + "' reset.");
}

void Agent::setSchema(const std::string &schema) { this->schema = schema; }

std::string Agent::executeApiCall(const std::string &fullPrompt) {
  logMessage(LogLevel::PROMPT,
             "Sending prompt to API for agent '" + name + "'");
  // logMessage(LogLevel::DEBUG, "Full Prompt:", fullPrompt); // Uncomment for
  // debugging
  try {
    std::string response = api.generate(fullPrompt);
    logMessage(LogLevel::DEBUG,
               "Received API response for agent '" + name + "'");
    return response;
  } catch (const ApiError &e) {
    logMessage(LogLevel::ERROR,
               "API Error occurred for agent '" + name + "':", e.what());
    throw;
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR,
               "Standard exception during API call for agent '" + name + "':",
               e.what());
    throw std::runtime_error("Error during API call: " + std::string(e.what()));
  } catch (...) {
    logMessage(LogLevel::ERROR,
               "Unknown exception during API call for agent '" + name + "'.");
    throw std::runtime_error("Unknown error during API call");
  }
}

// --- Deprecated / Fallback Methods ---
std::vector<Agent::ToolCallInfo>
Agent::extractToolCalls(const std::string &response) {
  logMessage(LogLevel::WARN,
             "Using fallback simple tool call extraction for agent '" + name +
                 "'.");
  std::vector<ToolCallInfo> calls;
  std::string startDelimiter = "```tool:";
  std::string endDelimiter = "```";
  size_t startPos = response.find(startDelimiter);
  while (startPos != std::string::npos) {
    size_t endPos =
        response.find(endDelimiter, startPos + startDelimiter.length());
    if (endPos == std::string::npos)
      break;
    std::string toolBlock =
        response.substr(startPos + startDelimiter.length(),
                        endPos - (startPos + startDelimiter.length()));
    // Basic parsing - assumes toolName\n{JSON_PARAMS}
    size_t firstNewline = toolBlock.find('\n');
    if (firstNewline == std::string::npos) {
      startPos = response.find(startDelimiter, endPos);
      continue;
    }
    ToolCallInfo callInfo;
    callInfo.toolName = toolBlock.substr(0, firstNewline);
    std::string jsonParamsStr = toolBlock.substr(firstNewline + 1);
    Json::Value paramsJson;
    Json::Reader reader;
    if (reader.parse(jsonParamsStr, paramsJson)) {
      callInfo.params = paramsJson;
      calls.push_back(callInfo);
    } // Fixed placeholder
    else {
      logMessage(LogLevel::ERROR,
                 "Failed to parse JSON params in tool block (fallback)",
                 reader.getFormattedErrorMessages());
    }
    startPos = response.find(startDelimiter, endPos);
  }
  return calls;
}

std::string
Agent::processToolCalls(const std::vector<ToolCallInfo> &toolCalls) {
  logMessage(LogLevel::WARN,
             "Using deprecated processToolCalls for agent '" + name + "'.");
  std::stringstream resultsStream;
  resultsStream << "<tool_results>\n";
  for (const auto &call : toolCalls) {
    logMessage(LogLevel::TOOL_CALL, "Processing OLD tool call format",
               call.toolName);
    std::string result = handleToolExecution(call); // Delegate to old handler
    logMessage(LogLevel::TOOL_RESULT, "OLD Tool result",
               call.toolName + ": " + result);
    resultsStream << "\t<tool_result tool_name=\"" << call.toolName << "\">\n";
    std::stringstream ss_result(result);
    std::string line;
    while (std::getline(ss_result, line)) {
      resultsStream << "\t\t" << line << "\n";
    }
    resultsStream << "\t</tool_result>\n";
  }
  resultsStream << "</tool_results>";
  return resultsStream.str();
}

std::string Agent::handleToolExecution(const ToolCallInfo &call) {
  logMessage(LogLevel::WARN,
             "Using deprecated handleToolExecution for agent '" + name + "'.",
             call.toolName);
  ActionInfo actionInfo;
  actionInfo.action = call.toolName;
  actionInfo.params = call.params;
  if (tools.count(call.toolName)) {
    actionInfo.type = "tool";
  } else if (internalToolDescriptions.count(call.toolName)) {
    actionInfo.type = "internal_function";
  } else {
    actionInfo.type = "unknown";
    logMessage(LogLevel::WARN, "Cannot determine type for fallback tool call",
               call.toolName);
  }
  return processAction(actionInfo); // Delegate to new processor
}

// --- Internal Tool Implementations (Fixing signatures) ---

std::string Agent::getHelp(const Json::Value &params) { // Added &
  std::string targetAction = "";
  if (params.isMember("action_name") && params["action_name"].isString()) {
    targetAction = params["action_name"].asString();
  }
  std::stringstream help;
  if (!targetAction.empty()) {
    std::string desc = getInternalToolDescription(targetAction);
    if (!desc.empty()) {
      help << "Help for action '" << targetAction << "':\n" << desc;
    } else {
      help << "Error: No help found for action '" << targetAction << "'.";
    }
  } else {
    help << "Available actions/tools:\n";
    std::map<std::string, std::string> availableActions =
        internalToolDescriptions;
    for (const auto &pair : tools) {
      if (pair.second) {
        availableActions[pair.first] = pair.second->getDescription();
      }
    }
    if (availableActions.empty()) {
      help << "- No actions/tools currently registered.\n";
    } else {
      for (const auto &pair : availableActions) {
        help << "- " << pair.first << ": " << pair.second << "\n";
      }
    }
  }
  return help.str();
}

std::string Agent::skip(const Json::Value &params) { // Added &
  (void)params;
  logMessage(LogLevel::INFO,
             "Skip action called for agent '" + name + "'. Ending turn.");
  setSkipFlowIteration(true);
  return "Skipping remainder of turn.";
}

std::string Agent::promptAgentTool(const Json::Value &params) { // Added &
  if (!params.isMember("agent_name") || !params["agent_name"].isString() ||
      !params.isMember("prompt") || !params["prompt"].isString()) {
    return "Error: promptAgent action requires 'agent_name' (string) and "
           "'prompt' (string) parameters.";
  }
  std::string agentName = params["agent_name"].asString();
  std::string userPrompt = params["prompt"].asString();
  Agent *subAgent = nullptr;
  for (const auto &pair : subAgents) {
    if (pair.first == agentName) {
      subAgent = pair.second;
      break;
    }
  }
  if (subAgent) {
    logMessage(LogLevel::INFO,
               "Agent '" + name + "' prompting sub-agent '" + agentName + "'");
    return subAgent->prompt(userPrompt);
  } else {
    return "Error: Sub-agent '" + agentName + "' not found.";
  }
}

std::string
Agent::summerizeTool(const Json::Value &params) { // Added & - Typo preserved
  if (!params.isMember("text") || !params["text"].isString()) {
    return "Error: summarizeTool action requires 'text' (string) parameter.";
  }
  std::string textToSummarize = params["text"].asString();
  logMessage(LogLevel::INFO, "Summarize tool called by agent '" + name + "'.");
  std::string summarizationPrompt =
      "Please summarize the following text concisely:\n\n" + textToSummarize;
  try {
    logMessage(LogLevel::WARN,
               "Summarization uses placeholder implementation.");
    return "Summary of: " + textToSummarize.substr(0, 30) +
           "... (Placeholder implementation)";
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR, "Summarization API call failed", e.what());
    return "Error during summarization API call: " + std::string(e.what());
  }
}

std::string
Agent::summarizeHistory(const Json::Value &params) { // Added & and fixed name
  (void)params;
  logMessage(LogLevel::INFO,
             "Summarize history tool called by agent '" + name + "'.");
  std::stringstream historyStream;
  for (const auto &entry : history) {
    historyStream << entry.first << ": " << entry.second << "\n---\n";
  }
  std::string historyStr = historyStream.str();
  if (historyStr.length() < 100) {
    return "History is too short to summarize meaningfully.";
  }
  std::string summarizationPrompt = "Please summarize the key points of the "
                                    "following conversation history:\n\n" +
                                    historyStr;
  try {
    logMessage(LogLevel::WARN,
               "History Summarization uses placeholder implementation.");
    return "Summary of conversation history... (Placeholder implementation)";
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR, "History summarization API call failed",
               e.what());
    return "Error during history summarization API call: " +
           std::string(e.what());
  }
}

std::string
Agent::getWeather(const Json::Value &params) { // Added & and fixed name
  if (!params.isMember("location") || !params["location"].isString()) {
    return "Error: getWeather action requires 'location' (string) parameter.";
  }
  std::string location = params["location"].asString();
  logMessage(LogLevel::INFO,
             "Get weather tool called by agent '" + name + "' for", location);
  logMessage(LogLevel::WARN, "Get weather uses placeholder implementation.");
  return "Weather in " + location +
         " is sunny, 25°C (Placeholder implementation).";
}

// --- Utility Methods (add*, remove*, import*, wrap*, run) remain largely the
// same, using clean names ---
// ... (Implementations for addToHistory, addMemory, removeMemory, addEnvVar,
// importEnvFile, addPrompt, addAgent, agentInstance, setSkipFlowIteration, run)
// ... (Ensure they use clean names like history, longTermMemory, env,
// extraPrompts, subAgents internally) Example addToHistory already shown above
// uses clean names.

void Agent::addToHistory(const std::string &role, const std::string &content) {
  const size_t max_history_content_len = 2500;
  std::string processed_content = content.substr(0, max_history_content_len);
  bool truncated = (content.length() > max_history_content_len);
  if (truncated)
    processed_content += "... (truncated)";
  history.push_back({role, processed_content});
  logMessage(LogLevel::DEBUG, "Added to history for agent '" + name + "'",
             "Role: " + role + (truncated ? " (Content Truncated)" : ""));
}
void Agent::addMemory(const std::string &role, const std::string &content) {
  longTermMemory.push_back({role, content});
  logMessage(LogLevel::DEBUG,
             "Added to LongTermMemory for agent '" + name + "'",
             "Role: " + role);
}
void Agent::removeMemory(const std::string &role, const std::string &content) {
  auto it = std::remove_if(longTermMemory.begin(), longTermMemory.end(),
                           [&role, &content](const auto &p) {
                             return p.first == role && p.second == content;
                           });
  if (it != longTermMemory.end()) {
    longTermMemory.erase(it, longTermMemory.end());
    logMessage(LogLevel::DEBUG,
               "Removed from LongTermMemory for agent '" + name + "'",
               "Role: " + role);
  }
}
void Agent::addEnvVar(const std::string &key, const std::string &value) {
  bool found = false;
  for (auto &pair : env) {
    if (pair.first == key) {
      pair.second = value;
      found = true;
      logMessage(LogLevel::DEBUG, "Updated env var for agent '" + name + "'",
                 key);
      break;
    }
  }
  if (!found) {
    env.push_back({key, value});
    logMessage(LogLevel::DEBUG, "Set env var for agent '" + name + "'", key);
  }
}
void Agent::importEnvFile(const std::string &filePath) {
  std::ifstream file(filePath);
  if (!file.is_open()) {
    logMessage(LogLevel::ERROR,
               "Agent '" + name + "' could not open env file:", filePath);
    return;
  }
  std::string line;
  int count = 0;
  while (std::getline(file, line)) {
    line.erase(0, line.find_first_not_of(" \t"));
    if (line.empty() || line[0] == '#')
      continue;
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
             "Agent '" + name + "' imported " + std::to_string(count) +
                 " env vars from:",
             filePath);
}
void Agent::addPrompt(const std::string &prompt) {
  extraPrompts.push_back(prompt);
  logMessage(LogLevel::DEBUG, "Added extra prompt for agent '" + name + "'.");
}
void Agent::addAgent(Agent *agent) {
  if (!agent) {
    logMessage(LogLevel::WARN, "Attempted to add null agent.");
    return;
  }
  if (agent == this) {
    logMessage(LogLevel::WARN, "Agent '" + name + "' cannot add self.");
    return;
  }
  for (const auto &pair : subAgents) {
    if (pair.first == agent->getName()) {
      logMessage(LogLevel::WARN, "Agent '" + name +
                                     "' duplicate sub-agent name: '" +
                                     agent->getName() + "'. Ignoring.");
      return;
    }
  }
  subAgents.push_back({agent->getName(), agent});
  logMessage(LogLevel::INFO, "Agent '" + name + "' registered sub-agent: '" +
                                 agent->getName() + "'");
}
std::string Agent::agentInstance(const std::string &agentName) {
  for (const auto &pair : subAgents) {
    if (pair.first == agentName)
      return pair.second->getName();
  }
  return "";
}
void Agent::setSkipFlowIteration(bool skip) { skipFlowIteration = skip; }
void Agent::run() {
  logMessage(LogLevel::INFO, "Agent '" + name + "' starting interactive loop.");
  logMessage(LogLevel::INFO,
             "Type 'exit' or 'quit' to stop, 'reset' to clear history.");
  std::string userInput;
  if (!initialCommands.empty()) {
    logMessage(LogLevel::INFO,
               "Executing initial commands for agent '" + name + "'...");
    Json::Value bashParams;
    std::vector<std::string> commandsToRun = initialCommands;
    initialCommands.clear();
    for (const auto &command : commandsToRun) {
      bashParams["command"] = command;
      logMessage(LogLevel::INFO, "Running initial command:", command);
      try {
        std::string result = manualToolCall("bash", bashParams);
        logMessage(LogLevel::INFO, "Initial command result:", result);
      } catch (const std::exception &e) {
        logMessage(
            LogLevel::ERROR,
            "Error running initial command '" + command + "':", e.what());
      } catch (...) {
        logMessage(LogLevel::ERROR,
                   "Unknown error running initial command '" + command + "'");
      }
    }
  }
  while (true) {
    std::cout << "\nUser (" << name << ") > ";
    if (!std::getline(std::cin, userInput)) {
      logMessage(LogLevel::INFO,
                 "Input stream closed. Exiting agent '" + name + "'.");
      break;
    }
    userInput.erase(0, userInput.find_first_not_of(" \t\r\n"));
    userInput.erase(userInput.find_last_not_of(" \t\r\n") + 1);
    if (userInput == "exit" || userInput == "quit") {
      logMessage(LogLevel::INFO, "Exit command received. Goodbye!");
      break;
    } else if (userInput == "reset") {
      reset();
      continue;
    } else if (userInput.empty()) {
      continue;
    }
    try {
      std::string response = prompt(userInput);
      std::cout << "\n" << name << ":\n" << response << std::endl;
      std::cout << "-----------------------------------------" << std::endl;
    } catch (const ApiError &e) {
      logMessage(LogLevel::ERROR, "API Error:", e.what());
      std::cout << "\n[Agent Error - API]: " << e.what() << std::endl;
      std::cout << "-----------------------------------------" << std::endl;
    } catch (const std::exception &e) {
      logMessage(LogLevel::ERROR, "General Error:", e.what());
      std::cout << "\n[Agent Error - General]: " << e.what() << std::endl;
      std::cout << "-----------------------------------------" << std::endl;
    } catch (...) {
      logMessage(LogLevel::ERROR, "Unknown Error.");
      std::cout << "\n[Agent Error - Unknown]" << std::endl;
      std::cout << "-----------------------------------------" << std::endl;
    }
  }
  logMessage(LogLevel::INFO, "Agent '" + name + "' interactive loop finished.");
}

// --- executeTask method ---
std::string Agent::executeTask(const std::string &task,
                               const std::string &format) {
  logMessage(LogLevel::DEBUG,
             "Executing task with specific format for agent '" + name + "'",
             "Task: " + task);
  std::string taskPrompt = systemPrompt;
  taskPrompt += "\n\n<current_task>\n";
  taskPrompt += "\t<description>" + task + "</description>\n";
  taskPrompt += "\t<required_format>" + format + "</required_format>\n";
  taskPrompt += "</current_task>\n\n";
  taskPrompt += "RESPONSE INSTRUCTIONS: Execute the task and respond ONLY with "
                "the result in the specified format within 'final_response', "
                "status='SUCCESS_FINAL', actions='[]'.";
  try {
    std::string llmResponseJson = executeApiCall(taskPrompt);
    std::string status, finalResponse;
    std::vector<StructuredThought> thoughts;
    std::vector<ActionInfo> actions;
    if (parseStructuredLLMResponse(llmResponseJson, status, thoughts, actions,
                                   finalResponse) &&
        (status == "SUCCESS_FINAL" || !finalResponse.empty())) {
      return finalResponse;
    } else {
      logMessage(LogLevel::ERROR,
                 "Task execution failed or yielded invalid response",
                 llmResponseJson);
      return "Error: Task execution failed to produce expected result.";
    }
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR, "Failed to execute task via API", e.what());
    return "Error: Failed to execute task: " + std::string(e.what());
  }
}
// Overloads for executeTask (assuming Json::Value version might be needed by
// tools)
// std::string Agent::executeTask(const std::string &task) {
//   return executeTask(task, "Provide the result directly as plain text.");
// } // Default format
std::string Agent::executeTask(const std::string &task,
                               const Json::Value &format) {
  return executeTask(task, format.toStyledString());
} // Convert JSON format to string

// --- manualToolCall (signature corrected) ---
std::string
Agent::manualToolCall(const std::string &toolName,
                      const Json::Value &params) { // Added & and fixed name
  logMessage(LogLevel::INFO, "Manually calling tool '" + toolName +
                                 "' for agent '" + name + "'");
  ActionInfo actionInfo;
  actionInfo.action = toolName;
  if (!params.isObject()) {
    logMessage(LogLevel::ERROR,
               "Manual tool call params not JSON object for agent '" + name +
                   "'.",
               params.toStyledString());
    return "Error: Manual tool call params must be JSON object.";
  }
  actionInfo.params = params;
  if (tools.count(toolName)) {
    actionInfo.type = "tool";
  } else if (internalToolDescriptions.count(toolName)) {
    actionInfo.type = "internal_function";
  } else {
    logMessage(LogLevel::WARN,
               "Cannot determine type for manual tool call. Assuming 'tool'.",
               toolName);
    actionInfo.type = "tool";
  }
  return processAction(actionInfo);
}

// --- Other Wrappers / Placeholders ---
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
  systemPrompt += "\n" + wrapXmlBlock(content, tag);
}
std::string Agent::reportTool(const Json::Value &params) {
  (void)params;
  logMessage(LogLevel::WARN,
             "'reportTool' not implemented for agent '" + name + "'.");
  return "Error: reportTool not implemented.";
}
std::string Agent::generateCode(const Json::Value &params) {
  (void)params;
  logMessage(LogLevel::WARN,
             "'generateCode' not implemented for agent '" + name + "'.");
  return "Error: generateCode not implemented.";
}
std::string Agent::auditResponse(const std::string &response) {
  (void)response;
  logMessage(LogLevel::WARN,
             "'auditResponse' not implemented for agent '" + name + "'.");
  return response;
}

// --- Directive Type to String Helper ---
std::string Agent::directiveTypeToString(Agent::DIRECTIVE::Type type) const {
  switch (type) {
  case DIRECTIVE::Type::BRAINSTORMING:
    return "BRAINSTORMING";
  case DIRECTIVE::Type::AUTONOMOUS:
    return "AUTONOMOUS";
  case DIRECTIVE::Type::NORMAL:
    return "NORMAL";
  case DIRECTIVE::Type::EXECUTE:
    return "EXECUTE";
  case DIRECTIVE::Type::REPORT:
    return "REPORT";
  default:
    logMessage(LogLevel::WARN, "Unknown directive type encountered");
    return "UNKNOWN";
  }
}

// --- Timestamp Generation Utility ---
std::string Agent::generateStamp(void) {
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  // Use std::localtime for local time formatting
  ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
  return ss.str();
}
