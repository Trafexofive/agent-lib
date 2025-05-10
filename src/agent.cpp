#include "../inc/Agent.hpp" // Adjust path as necessary
#include <algorithm>        // For std::remove_if, std::find_if
#include <cstdlib>          // For std::getenv (importEnvironmentFile)
#include <fstream>          // For file operations (importEnvironmentFile)
#include <iostream>         // For std::cout, std::cin, std::cerr
#include <sstream>          // For std::stringstream

// --- Logging Function Implementation (Example) ---
void logMessage(LogLevel level, const std::string &message,
                const std::string &details) {
  auto nowChrono = std::chrono::system_clock::now();
  auto nowTimeT = std::chrono::system_clock::to_time_t(nowChrono);
  std::tm nowTm = *std::localtime(&nowTimeT);
  char timeBuffer[20];
  std::strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &nowTm);

  std::string prefix;
  std::string colorStart = "";
  std::string colorEnd = "\033[0m"; // ANSI Reset

  switch (level) {
  case LogLevel::DEBUG:
    prefix = "[DEBUG] ";
    colorStart = "\033[36m";
    break; // Cyan
  case LogLevel::INFO:
    prefix = "[INFO]  ";
    colorStart = "\033[32m";
    break; // Green
  case LogLevel::WARN:
    prefix = "[WARN]  ";
    colorStart = "\033[33m";
    break; // Yellow
  case LogLevel::ERROR:
    prefix = "[ERROR] ";
    colorStart = "\033[1;31m";
    break; // Bold Red
  case LogLevel::TOOL_CALL:
    prefix = "[TOOL CALL] ";
    colorStart = "\033[1;35m";
    break; // Bold Magenta
  case LogLevel::TOOL_RESULT:
    prefix = "[TOOL RESULT] ";
    colorStart = "\033[35m";
    break; // Magenta
  case LogLevel::PROMPT:
    prefix = "[PROMPT] ";
    colorStart = "\033[34m";
    break; // Blue
  }
  std::ostream &outStream =
      (level == LogLevel::ERROR || level == LogLevel::WARN) ? std::cerr
                                                            : std::cout;
  outStream << colorStart << std::string(timeBuffer) << " " << prefix << message
            << colorEnd << std::endl;
  if (!details.empty()) {
    const size_t MAX_DETAIL_LEN = 500;
    std::string truncatedDetails = details.substr(0, MAX_DETAIL_LEN);
    if (details.length() > MAX_DETAIL_LEN)
      truncatedDetails += "... (truncated)";
    outStream << colorStart << "  " << truncatedDetails << colorEnd
              << std::endl;
  }
}
// --- End Logging ---

Agent::Agent(MiniGemini &apiRef, const std::string &agentNameVal)
    : api(apiRef), agentName(agentNameVal), currentIteration(0),
      iterationLimit(10), skipNextFlowIteration(false) {
  logMessage(LogLevel::DEBUG, "Agent instance created", "Name: " + agentName);
  // Initialize descriptions for internal functions
  internalFunctionDescriptions["help"] =
      "Provides descriptions of available tools/actions. Parameters: "
      "{\"action_name\": \"string\" (optional)}";
  internalFunctionDescriptions["skip"] = "Skips the final response generation "
                                         "for the current turn. No parameters.";
  internalFunctionDescriptions["promptAgent"] =
      "Sends a prompt to another registered agent. Parameters: "
      "{\"agent_name\": \"string\", \"prompt\": \"string\"}";
  internalFunctionDescriptions["summarizeText"] =
      "Summarizes provided text content. Parameters: {\"text\": \"string\"}";
  internalFunctionDescriptions["summarizeHistory"] =
      "Summarizes the current conversation history. No parameters.";
  internalFunctionDescriptions["getWeather"] =
      "Fetches current weather. Parameters: {\"location\": \"string\"}";
}

Agent::~Agent() {
  logMessage(LogLevel::DEBUG, "Agent instance destroyed, cleaning up tools.",
             "Name: " + agentName);
  for (auto &pair : registeredTools) {
    delete pair.second; // Delete tools agent owns
  }
  registeredTools.clear();
}

// --- Configuration Setters ---
void Agent::setName(const std::string &newName) { agentName = newName; }
void Agent::setDescription(const std::string &newDescription) {
  agentDescription = newDescription;
}
void Agent::setSystemPrompt(const std::string &prompt) {
  systemPrompt = prompt;
}
void Agent::setSchema(const std::string &schema) { llmResponseSchema = schema; }
void Agent::setExample(const std::string &example) {
  llmResponseExample = example;
}
void Agent::setIterationCap(int cap) { iterationLimit = (cap > 0) ? cap : 1; }
void Agent::setDirective(const AgentDirective &directive) {
  currentDirective = directive;
}
void Agent::addTask(const std::string &task) { tasks.push_back(task); }
void Agent::addInitialCommand(const std::string &command) {
  initialCommands.push_back(command);
}

// --- Tool Management ---
void Agent::addTool(Tool *tool) {
  if (!tool) {
    logMessage(LogLevel::WARN, "Attempted to add a null tool.");
    return;
  }
  const std::string &toolName = tool->getName();
  if (toolName.empty()) {
    logMessage(LogLevel::WARN, "Attempted to add a tool with an empty name.");
    delete tool; // Clean up if not added
    return;
  }
  if (registeredTools.count(toolName) ||
      internalFunctionDescriptions.count(toolName)) {
    logMessage(LogLevel::WARN,
               "Agent '" + agentName +
                   "': Tool/internal function name conflict for '" + toolName +
                   "'. Ignoring new tool.");
    delete tool;
  } else {
    registeredTools[toolName] = tool;
    logMessage(LogLevel::INFO,
               "Agent '" + agentName + "' registered tool: '" + toolName + "'");
  }
}

void Agent::removeTool(const std::string &toolName) {
  auto it = registeredTools.find(toolName);
  if (it != registeredTools.end()) {
    delete it->second;
    registeredTools.erase(it);
    logMessage(LogLevel::INFO,
               "Agent '" + agentName + "' removed tool: '" + toolName + "'");
  } else {
    logMessage(LogLevel::WARN,
               "Agent '" + agentName +
                   "' attempted to remove non-existent tool: '" + toolName +
                   "'");
  }
}

Tool *Agent::getTool(const std::string &toolName) const {
  auto it = registeredTools.find(toolName);
  return (it != registeredTools.end()) ? it->second : nullptr;
}

// --- Core Agent Loop ---
void Agent::reset() {
  conversationHistory.clear();
  scratchpad.clear();
  shortTermMemory.clear();
  // longTermMemory typically persists unless explicitly cleared
  currentIteration = 0;
  skipNextFlowIteration = false;
  logMessage(LogLevel::INFO, "Agent '" + agentName + "' state reset.");
}

void Agent::run() {
  logMessage(LogLevel::INFO,
             "Agent '" + agentName + "' starting interactive loop.");
  logMessage(LogLevel::INFO,
             "Type 'exit' or 'quit' to stop, 'reset' to clear history.");

  // Execute initial commands if any
  if (!initialCommands.empty()) {
    logMessage(LogLevel::INFO,
               "Executing " + std::to_string(initialCommands.size()) +
                   " initial commands for agent '" + agentName + "'...");
    std::vector<std::string> commandsToRun =
        initialCommands; // Copy to allow modification if needed
    initialCommands
        .clear(); // Clear so they don't run again on 'reset' then 'run'
    for (const auto &command : commandsToRun) {
      Json::Value bashParams;
      bashParams["command"] =
          command; // Assuming your bash tool takes {"command": "..."}
      logMessage(LogLevel::INFO, "Running initial command:", command);
      try {
        // We need a way to call the bash tool. The 'bash' tool should be
        // registered.
        Tool *bashTool =
            getTool("bash"); // Assumes a tool named "bash" is registered
        if (bashTool) {
          std::string result = bashTool->execute(bashParams);
          logMessage(LogLevel::INFO, "Initial command result:", result);
        } else {
          logMessage(LogLevel::ERROR,
                     "Initial command '" + command +
                         "' requires 'bash' tool, but it's not registered.");
        }
      } catch (const std::exception &e) {
        logMessage(
            LogLevel::ERROR,
            "Error running initial command '" + command + "':", e.what());
      }
    }
  }

  std::string userInputText;
  while (true) {
    std::cout << "\nUser (" << agentName << ") > ";
    if (!std::getline(std::cin, userInputText)) {
      logMessage(LogLevel::INFO, "Input stream closed (EOF). Exiting agent '" +
                                     agentName + "'.");
      break;
    }
    userInputText.erase(
        0, userInputText.find_first_not_of(" \t\r\n")); // Trim leading
    userInputText.erase(userInputText.find_last_not_of(" \t\r\n") +
                        1); // Trim trailing

    if (userInputText == "exit" || userInputText == "quit") {
      logMessage(LogLevel::INFO, "Exit command received. Goodbye from agent '" +
                                     agentName + "'!");
      break;
    } else if (userInputText == "reset") {
      reset();
      logMessage(LogLevel::INFO, "Agent '" + agentName + "' has been reset.");
      continue;
    } else if (userInputText.empty()) {
      continue;
    }

    try {
      std::string agentResponseText = prompt(userInputText);
      std::cout << "\n" << agentName << ":\n" << agentResponseText << std::endl;
      std::cout << "-----------------------------------------" << std::endl;
    } catch (const std::runtime_error
                 &e) { // Catch specific ApiError if you have one
      logMessage(LogLevel::ERROR,
                 "Agent runtime error for '" + agentName + "':", e.what());
      std::cout << "\n[Agent Error - Runtime]: " << e.what() << std::endl;
    } catch (const std::exception &e) {
      logMessage(LogLevel::ERROR,
                 "General exception for agent '" + agentName + "':", e.what());
      std::cout << "\n[Agent Error - General]: " << e.what() << std::endl;
    }
  }
  logMessage(LogLevel::INFO,
             "Agent '" + agentName + "' interactive loop finished.");
}

// --- Memory & State ---
void Agent::addToHistory(const std::string &role, const std::string &content) {
  const size_t MAX_HISTORY_CONTENT_LEN =
      2500; // Prevent excessively long history entries
  std::string processedContent = content.substr(0, MAX_HISTORY_CONTENT_LEN);
  bool truncated = (content.length() > MAX_HISTORY_CONTENT_LEN);
  if (truncated)
    processedContent += "... (truncated)";

  conversationHistory.push_back({role, processedContent});
  logMessage(LogLevel::DEBUG, "Added to history for agent '" + agentName + "'",
             "Role: " + role + (truncated ? " (Content Truncated)" : ""));
}
void Agent::addScratchpadItem(const std::string &key,
                              const std::string &value) {
  scratchpad.push_back({key, value});
} // Simple append, could be map
void Agent::addShortTermMemory(const std::string &role,
                               const std::string &content) {
  shortTermMemory.push_back({role, content});
}
void Agent::addLongTermMemory(const std::string &role,
                              const std::string &content) {
  longTermMemory.push_back({role, content});
}

void Agent::addEnvironmentVariable(const std::string &key,
                                   const std::string &value) {
  // Update if exists, else add
  auto it =
      std::find_if(environmentVariables.begin(), environmentVariables.end(),
                   [&key](const auto &pair) { return pair.first == key; });
  if (it != environmentVariables.end()) {
    it->second = value;
    logMessage(LogLevel::DEBUG,
               "Updated environment variable for agent '" + agentName + "'",
               key + "=" + value);
  } else {
    environmentVariables.push_back({key, value});
    logMessage(LogLevel::DEBUG,
               "Added environment variable for agent '" + agentName + "'",
               key + "=" + value);
  }
}

void Agent::importEnvironmentFile(const std::string &filePath) {
  std::ifstream envFile(filePath);
  if (!envFile.is_open()) {
    logMessage(LogLevel::ERROR,
               "Agent '" + agentName + "' could not open env file:", filePath);
    return;
  }
  std::string line;
  int count = 0;
  while (std::getline(envFile, line)) {
    line.erase(0, line.find_first_not_of(" \t")); // Trim leading whitespace
    if (line.empty() || line[0] == '#')
      continue; // Skip empty lines and comments

    size_t eqPos = line.find('=');
    if (eqPos != std::string::npos) {
      std::string key = line.substr(0, eqPos);
      key.erase(key.find_last_not_of(" \t") +
                1); // Trim trailing space from key

      std::string value = line.substr(eqPos + 1);
      value.erase(
          0, value.find_first_not_of(" \t")); // Trim leading space from value
      value.erase(value.find_last_not_of(" \t") + 1); // Trim trailing space

      // Optional: remove quotes if value is quoted
      if (value.length() >= 2 &&
          ((value.front() == '"' && value.back() == '"') ||
           (value.front() == '\'' && value.back() == '\''))) {
        value = value.substr(1, value.length() - 2);
      }
      if (!key.empty()) {
        addEnvironmentVariable(key, value);
        count++;
      }
    }
  }
  envFile.close();
  logMessage(LogLevel::INFO,
             "Agent '" + agentName + "' imported " + std::to_string(count) +
                 " environment variables from:",
             filePath);
}

void Agent::addExtraSystemPrompt(const std::string &promptFragment) {
  extraSystemPrompts.push_back(promptFragment);
}

// --- Getters ---
const std::string &Agent::getName() const { return agentName; }
const std::string &Agent::getDescription() const { return agentDescription; }
const std::string &Agent::getSystemPrompt() const { return systemPrompt; }
const std::string &Agent::getSchema() const { return llmResponseSchema; }
const std::string &Agent::getExample() const { return llmResponseExample; }
int Agent::getIterationCap() const { return iterationLimit; }
const Agent::AgentDirective &Agent::getDirective() const {
  return currentDirective;
}
const std::vector<std::string> &Agent::getTasks() const { return tasks; }
const StringKeyValuePair &Agent::getEnvironmentVariables() const {
  return environmentVariables;
}
const std::vector<std::string> &Agent::getExtraSystemPrompts() const {
  return extraSystemPrompts;
}
const std::vector<std::pair<std::string, std::string>> &
Agent::getHistory() const {
  return conversationHistory;
}

// --- Sub-Agent Management ---
void Agent::addSubAgent(Agent *subAgentInstance) {
  if (!subAgentInstance || subAgentInstance == this) {
    logMessage(
        LogLevel::WARN,
        "Invalid sub-agent provided or self-addition attempt for agent '" +
            agentName + "'.");
    return;
  }
  // Check for duplicate name
  if (std::find_if(subAgents.begin(), subAgents.end(), [&](const auto &pair) {
        return pair.first == subAgentInstance->getName();
      }) != subAgents.end()) {
    logMessage(LogLevel::WARN, "Agent '" + agentName +
                                   "' already has a sub-agent named '" +
                                   subAgentInstance->getName() + "'.");
    return;
  }
  subAgents.push_back({subAgentInstance->getName(), subAgentInstance});
  logMessage(LogLevel::INFO, "Agent '" + agentName +
                                 "' registered sub-agent: '" +
                                 subAgentInstance->getName() + "'");
}

Agent *Agent::getSubAgent(const std::string &subAgentNameKey) const {
  auto it =
      std::find_if(subAgents.begin(), subAgents.end(), [&](const auto &pair) {
        return pair.first == subAgentNameKey;
      });
  return (it != subAgents.end()) ? it->second : nullptr;
}

// --- Manual Tool Call ---
std::string Agent::manualToolCall(const std::string &toolName,
                                  const Json::Value &params) {
  logMessage(LogLevel::INFO, "Manually calling tool '" + toolName +
                                 "' for agent '" + agentName + "'");
  Action ai;
  ai.action = toolName;
  ai.type = "tool"; // Assume it's a registered tool
  ai.params = params;
  return processSingleAction(ai);
}

// --- Private Helper Methods ---

std::string Agent::processActions(const std::vector<Action> &actions) {
  std::stringstream resultsSs;
  resultsSs << "<action_results>\n"; // XML-like wrapper for multiple results

  for (const auto &actionInfo : actions) {
    std::string singleResult = processSingleAction(actionInfo);
    resultsSs << "\t<action_result action_name=\"" << actionInfo.action
              << "\" type=\"" << actionInfo.type << "\">\n";
    // Indent the result for readability
    std::stringstream resultLines(singleResult);
    std::string line;

    while (std::getline(resultLines, line)) {
      resultsSs << "\t\t" << line << "\n";
    }

    resultsSs << "\t</action_result>\n";
  }

  resultsSs << "</action_results>";
  return resultsSs.str();
}

std::string Agent::processSingleAction(const Action &actionInfo) {
  logMessage(LogLevel::TOOL_CALL,
             "Agent '" + agentName +
                 "' processing action: " + actionInfo.action,
             "Type: " + actionInfo.type + ", Params: " +
                 actionInfo.params.toStyledString().substr(0, 100) + "...");

  try {
    if (actionInfo.type == "tool") {
      Tool *toolToRun = getTool(actionInfo.action);
      if (toolToRun) {
        return toolToRun->execute(actionInfo.params);
      } else {
        logMessage(LogLevel::ERROR, "Tool '" + actionInfo.action +
                                        "' not found for agent '" + agentName +
                                        "'.");
        return "Error: Tool '" + actionInfo.action +
               "' not registered or available.";
      }
    } else if (actionInfo.type == "internal_function") {
      if (actionInfo.action == "help")
        return internalGetHelp(actionInfo.params);
      if (actionInfo.action == "skip")
        return internalSkipIteration(actionInfo.params);
      if (actionInfo.action == "promptAgent")
        return internalPromptAgent(actionInfo.params);
      if (actionInfo.action == "summarizeText")
        return internalSummarizeText(actionInfo.params);
      if (actionInfo.action == "summarizeHistory")
        return internalSummarizeHistory(actionInfo.params);
      if (actionInfo.action == "getWeather")
        return internalGetWeather(actionInfo.params);
      // Add other internal functions here
      logMessage(LogLevel::ERROR, "Unknown internal function '" +
                                      actionInfo.action + "' for agent '" +
                                      agentName + "'.");
      return "Error: Unknown internal function '" + actionInfo.action + "'.";
    } else if (actionInfo.type == "output") {
      if (actionInfo.action == "send_response") {
        if (actionInfo.params.isMember("text") &&
            actionInfo.params["text"].isString()) {
          logMessage(LogLevel::INFO,
                     "Agent '" + agentName +
                         "' received 'send_response' output action.",
                     actionInfo.params["text"].asString());
          // This action type's "result" is effectively what it outputs.
          // The main loop handles setting this as the finalAgentResponseToUser
          // if appropriate.
          return actionInfo.params["text"].asString();
        } else {
          logMessage(
              LogLevel::ERROR,
              "Invalid params for 'send_response' output action in agent '" +
                  agentName + "'.",
              actionInfo.params.toStyledString());
          return "Error: Invalid parameters for 'send_response' action.";
        }
      }
      logMessage(LogLevel::ERROR, "Unknown output action '" +
                                      actionInfo.action + "' for agent '" +
                                      agentName + "'.");
      return "Error: Unknown output action '" + actionInfo.action + "'.";
    } else if (actionInfo.type == "workflow_control") {
      logMessage(LogLevel::DEBUG,
                 "Workflow control action '" + actionInfo.action +
                     "' acknowledged by agent '" + agentName +
                     "'. Handled by main loop.",
                 actionInfo.params.toStyledString());
      return "Workflow action '" + actionInfo.action +
             "' noted."; // Typically doesn't produce direct output for history
    }
    // Add script, http_request later if needed
    logMessage(LogLevel::WARN, "Unsupported action type '" + actionInfo.type +
                                   "' for action '" + actionInfo.action +
                                   "' in agent '" + agentName + "'.");
    return "Error: Unsupported action type '" + actionInfo.type + "'.";
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR,
               "Exception during action '" + actionInfo.action +
                   "' execution for agent '" + agentName + "'.",
               e.what());
    return "Error executing action '" + actionInfo.action +
           "': " + std::string(e.what());
  } catch (...) {
    logMessage(LogLevel::ERROR,
               "Unknown exception during action '" + actionInfo.action +
                   "' execution for agent '" + agentName + "'.");
    return "Error: Unknown exception executing action '" + actionInfo.action +
           "'.";
  }
}

std::string Agent::executeApiCall(const std::string &fullPromptText) {
  logMessage(LogLevel::PROMPT,
             "Sending prompt to API for agent '" + agentName +
                 "' (length: " + std::to_string(fullPromptText.length()) + ")");
  // logMessage(LogLevel::DEBUG, "Full Prompt Text:", fullPromptText); //
  // Uncomment for detailed debugging
  try {
    std::string response = api.generate(fullPromptText);
    logMessage(LogLevel::DEBUG,
               "Received API response for agent '" + agentName +
                   "' (length: " + std::to_string(response.length()) + ")");
    // logMessage(LogLevel::DEBUG, "API Raw Response:", response); // For
    // debugging LLM output directly
    return response;
  } catch (const std::runtime_error &e) { // Catch ApiError if you have one
    logMessage(LogLevel::ERROR,
               "API Error occurred for agent '" + agentName + "':", e.what());
    // Construct a JSON error response that the agent's parsing logic can handle
    Json::Value errorJson;
    errorJson["status"] = "ERROR_INTERNAL_API";
    errorJson["thoughts"][0u]["type"] = "ERROR";
    errorJson["thoughts"][0u]["content"] =
        "API call failed: " + std::string(e.what());
    errorJson["actions"] = Json::arrayValue;
    errorJson["final_response"] = "I encountered an error communicating with "
                                  "the language model. Please try again later.";
    Json::StreamWriterBuilder writer;
    return Json::writeString(writer, errorJson);
  }
  // Add catch for std::exception and (...) if needed, though MiniGemini should
  // throw specific error or std::runtime_error
}

void Agent::setSkipNextFlowIteration(bool skip) {
  skipNextFlowIteration = skip;
  if (skip) {
    logMessage(LogLevel::DEBUG,
               "Agent '" + agentName +
                   "' flow iteration will be skipped for the current turn.");
  }
}

std::string Agent::directiveTypeToString(AgentDirective::Type type) const {
  switch (type) {
  case AgentDirective::Type::BRAINSTORMING:
    return "BRAINSTORMING";
  case AgentDirective::Type::AUTONOMOUS:
    return "AUTONOMOUS";
  case AgentDirective::Type::NORMAL:
    return "NORMAL";
  case AgentDirective::Type::EXECUTE:
    return "EXECUTE";
  case AgentDirective::Type::REPORT:
    return "REPORT";
  default:
    return "UNKNOWN_DIRECTIVE_TYPE";
  }
}

// --- Internal "Tool-Like" Functions ---
std::string Agent::internalGetHelp(const Json::Value &params) {
  std::string targetActionName;
  if (params.isMember("action_name") && params["action_name"].isString()) {
    targetActionName = params["action_name"].asString();
  }
  std::stringstream helpSs;
  if (!targetActionName.empty()) {
    helpSs << "Help for action '" << targetActionName << "':\n";
    bool found = false;
    if (internalFunctionDescriptions.count(targetActionName)) {
      helpSs << "- Type: Internal Function\n";
      helpSs << "- Description & Params: "
             << internalFunctionDescriptions.at(targetActionName);
      found = true;
    }
    Tool *extTool = getTool(targetActionName);
    if (extTool) {
      if (found)
        helpSs << "\n---\n"; // Separator if also internal
      helpSs << "- Type: Registered Tool\n";
      helpSs << "- Description: " << extTool->getDescription();
      // Could add example params if Tool class stores them
      found = true;
    }
    if (!found) {
      helpSs << "Action '" << targetActionName
             << "' not found or no help available.";
    }
  } else {
    helpSs << "Available Actions for agent '" << agentName << "':\n";
    helpSs << "--- Internal Functions ---\n";
    if (internalFunctionDescriptions.empty())
      helpSs << "(None)\n";
    for (const auto &pair : internalFunctionDescriptions) {
      helpSs << "- " << pair.first << ": " << pair.second << "\n";
    }
    helpSs << "\n--- Registered Tools ---\n";
    if (registeredTools.empty())
      helpSs << "(None)\n";
    for (const auto &pair : registeredTools) {
      if (pair.second)
        helpSs << "- " << pair.second->getName() << ": "
               << pair.second->getDescription() << "\n";
    }
    helpSs << "\nUse help with {\"action_name\": \"<action_name>\"} for "
              "details on a specific action.";
  }
  return helpSs.str();
}

std::string Agent::internalSkipIteration(const Json::Value &params) {
  (void)params; // No params expected
  logMessage(LogLevel::INFO,
             "Agent '" + agentName +
                 "' is skipping the rest of this turn due to 'skip' action.");
  setSkipNextFlowIteration(true);
  return "Success: Final response generation for this turn will be skipped.";
}

std::string Agent::internalPromptAgent(const Json::Value &params) {
  if (!params.isMember("agent_name") || !params["agent_name"].isString() ||
      !params.isMember("prompt") || !params["prompt"].isString()) {
    return "Error [promptAgent]: Requires string parameters 'agent_name' and "
           "'prompt'.";
  }
  std::string targetAgentName = params["agent_name"].asString();
  std::string subPromptText = params["prompt"].asString();

  Agent *targetAgent = getSubAgent(targetAgentName);
  if (targetAgent) {
    logMessage(LogLevel::INFO, "Agent '" + agentName +
                                   "' is prompting sub-agent '" +
                                   targetAgentName + "'.");
    try {
      // Potentially prepend context that this is a sub-prompt
      std::string contextualPrompt =
          "Received from Agent '" + agentName + "':\n" + subPromptText;
      std::string response =
          targetAgent->prompt(contextualPrompt); // Recursive call
      logMessage(LogLevel::INFO,
                 "Received response from sub-agent '" + targetAgentName + "'.");
      return "Response from Agent '" + targetAgentName + "':\n---\n" +
             response + "\n---";
    } catch (const std::exception &e) {
      logMessage(LogLevel::ERROR,
                 "Error prompting sub-agent '" + targetAgentName +
                     "' from agent '" + agentName + "'.",
                 e.what());
      return "Error [promptAgent]: Exception while prompting '" +
             targetAgentName + "': " + e.what();
    }
  } else {
    logMessage(LogLevel::WARN, "Sub-agent '" + targetAgentName +
                                   "' not found for prompting by agent '" +
                                   agentName + "'.");
    return "Error [promptAgent]: Sub-agent '" + targetAgentName +
           "' not found.";
  }
}

std::string Agent::internalSummarizeText(const Json::Value &params) {
  if (!params.isMember("text") || !params["text"].isString()) {
    return "Error [summarizeText]: Missing or invalid string parameter 'text'.";
  }
  std::string textToSummarize = params["text"].asString();
  if (textToSummarize.length() < 50) { // Arbitrary short content check
    return "Content is too short to summarize effectively.";
  }
  logMessage(LogLevel::DEBUG,
             "Agent '" + agentName + "' summarizing text (length: " +
                 std::to_string(textToSummarize.length()) + ")");
  try {
    // This is a simplified way to use the LLM for a sub-task.
    // Could be more elaborate, creating a temporary directive.
    std::string taskDescription =
        "Provide a concise summary of the following text:\n\n" +
        textToSummarize;
    std::string format =
        "{\"summary\": \"string (the concise summary of the text)\"}";

    std::string llmTaskPrompt = systemPrompt; // Start with base system prompt
    llmTaskPrompt += "\n\n<current_task_for_summarization>\n";
    llmTaskPrompt += "\t<description>" + taskDescription + "</description>\n";
    llmTaskPrompt += "\t<required_format>" + format + "</required_format>\n";
    llmTaskPrompt += "</current_task_for_summarization>\n\n";
    llmTaskPrompt += "RESPONSE INSTRUCTIONS: Execute the summarization task "
                     "and respond ONLY with "
                     "the JSON containing the 'summary' field as specified in "
                     "'required_format'. "
                     "Set status to 'SUCCESS_FINAL' and actions to '[]'.";

    std::string llmSummaryJson = executeApiCall(llmTaskPrompt);

    Json::Value summaryRoot;
    Json::Reader reader;
    if (reader.parse(llmSummaryJson, summaryRoot) && summaryRoot.isObject() &&
        summaryRoot.isMember("final_response") &&
        summaryRoot["final_response"].isString()) {
      // Assuming the LLM correctly put the JSON summary *inside* the
      // final_response string.
      std::string finalResponseContent =
          summaryRoot["final_response"].asString();
      Json::Value actualSummaryJson;
      if (reader.parse(finalResponseContent, actualSummaryJson) &&
          actualSummaryJson.isMember("summary") &&
          actualSummaryJson["summary"].isString()) {
        return actualSummaryJson["summary"].asString();
      } else {
        logMessage(LogLevel::WARN,
                   "Failed to parse JSON *from final_response* for "
                   "summarizeText by agent '" +
                       agentName + "'.",
                   "Final Response: " + finalResponseContent);
      }
    }
    logMessage(LogLevel::WARN,
               "Failed to parse outer JSON or extract summary for "
               "summarizeText by agent '" +
                   agentName + "', returning raw-ish LLM response.",
               "LLM Response: " + llmSummaryJson);
    return llmSummaryJson; // Fallback
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR,
               "Error during summarization task execution for agent '" +
                   agentName + "'.",
               e.what());
    return "Error [summarizeText]: Exception during summarization: " +
           std::string(e.what());
  }
}

std::string Agent::internalSummarizeHistory(const Json::Value &params) {
  (void)params; // No params expected
  if (conversationHistory.empty())
    return "Conversation history is empty.";

  std::stringstream historyTextSs;
  historyTextSs << "Current Conversation History (to be summarized):\n";
  for (const auto &entry : conversationHistory) {
    historyTextSs << entry.first << ": " << entry.second.substr(0, 150)
                  << (entry.second.length() > 150 ? "..." : "") << "\n";
  }
  Json::Value summarizeParams;
  summarizeParams["text"] = historyTextSs.str();
  return internalSummarizeText(summarizeParams); // Reuse the text summarizer
}

std::string Agent::internalGetWeather(const Json::Value &params) {
  if (!params.isMember("location") || !params["location"].isString()) {
    return "Error [getWeather]: Missing or invalid string parameter "
           "'location'.";
  }
  std::string location = params["location"].asString();
  std::string originalLocation = location; // For user-facing messages
  // Sanitize location for URL? For wttr.in, spaces become '+'
  std::replace(location.begin(), location.end(), ' ', '+');

  // Use the global executeCommand utility
  std::string command =
      "curl -s -L \"https://wttr.in/" + location + "?format=3\"";
  std::string weatherResultOutput;

  logMessage(LogLevel::DEBUG,
             "Agent '" + agentName +
                 "' attempting to get weather for: " + originalLocation,
             "Command: " + command);

  // We need a 'bash' tool registered to use executeCommand through the agent's
  // tool mechanism, OR call a global executeCommand directly if this internal
  // function has such privilege. For now, let's assume it uses a registered
  // 'bash' tool for consistency.
  Tool *bashTool = getTool("bash");
  if (!bashTool) {
    logMessage(
        LogLevel::ERROR,
        "Agent '" + agentName +
            "': 'bash' tool is required for getWeather but not registered.");
    return "Error [getWeather]: The 'bash' tool is unavailable to fetch "
           "weather data.";
  }
  Json::Value bashParams;
  bashParams["command"] = command;
  weatherResultOutput = bashTool->execute(bashParams);

  // Basic result cleaning and validation
  weatherResultOutput.erase(0,
                            weatherResultOutput.find_first_not_of(" \t\r\n"));
  weatherResultOutput.erase(weatherResultOutput.find_last_not_of(" \t\r\n") +
                            1);

  if (weatherResultOutput.empty() ||
      weatherResultOutput.find("Unknown location") != std::string::npos ||
      weatherResultOutput.find("ERROR") != std::string::npos ||
      weatherResultOutput.find("Sorry, we are runnning into an issue") !=
          std::string::npos) {
    logMessage(
        LogLevel::WARN,
        "Failed to get weather using wttr.in for agent '" + agentName + "'.",
        "Location: " + originalLocation + ", Output: " + weatherResultOutput);
    return "Error [getWeather]: Could not retrieve weather information for '" +
           originalLocation +
           "'. The location might be invalid or the service unavailable.";
  }
  return "Current weather for " + originalLocation + ": " + weatherResultOutput;
}

// Utility
std::string Agent::generateTimestamp() const {
  auto nowChrono = std::chrono::system_clock::now();
  auto nowTimeT = std::chrono::system_clock::to_time_t(nowChrono);
  std::tm nowTm =
      *std::localtime(&nowTimeT); // Use localtime for local timezone
  std::stringstream ss;
  ss << std::put_time(&nowTm, "%Y-%m-%dT%H:%M:%S%Z"); // ISO 8601 like format
  return ss.str();
}

void Agent::trimLLMResponse(std::string &responseText) {
  // Finds ```json ... ``` or ``` ... ``` and extracts the content.
  size_t startPos = responseText.find("```");
  if (startPos == std::string::npos)
    return; // No code block found

  size_t contentStart = responseText.find_first_not_of(" \t\r\n", startPos + 3);
  if (contentStart == std::string::npos)
    return; // Empty after ```

  // If it's ```json, skip 'json' part
  if (responseText.substr(startPos + 3, 4) == "json") {
    contentStart = responseText.find_first_not_of(" \t\r\n", startPos + 3 + 4);
    if (contentStart == std::string::npos)
      return;
  }

  size_t endPos = responseText.rfind("```");
  if (endPos == std::string::npos || endPos <= contentStart)
    return; // No closing ``` or it's before content

  size_t contentEnd = responseText.find_last_not_of(" \t\r\n", endPos - 1);
  if (contentEnd == std::string::npos ||
      contentEnd < contentStart) { // Empty before ```
    responseText = "";             // Or handle as error
    return;
  }

  responseText =
      responseText.substr(contentStart, contentEnd - contentStart + 1);
  logMessage(LogLevel::DEBUG,
             "Trimmed LLM response block for agent '" + agentName + "'.");
}
