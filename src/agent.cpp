// src/agent.cpp
#include "../inc/Agent.hpp" // Adjust path as necessary
#include <algorithm>        // For std::remove_if, std::find_if
#include <chrono>           // For time functions in generateTimestamp
#include <cstdlib>          // For std::getenv
#include <fstream>          // For file operations
#include <iomanip>          // For std::put_time in generateTimestamp
#include <iostream>         // For std::cout, std::cin, std::cerr
#include <sstream>          // For std::stringstream

// --- End Logging ---

Agent::Agent(MiniGemini &apiRef, const std::string &agentNameVal)
    : api(apiRef), agentName(agentNameVal), currentIteration(0),
      iterationLimit(10), skipNextFlowIteration(false) {
  logMessage(LogLevel::DEBUG, "Agent instance created", "Name: " + agentName);
  // internalFunctionDescriptions["help"] =
  //     "Provides descriptions of available tools/actions. Parameters: "
  //     "{\"action_name\": \"string\" (optional)}";
  // internalFunctionDescriptions["skip"] = "Skips the final response generation "
  //                                        "for the current turn. No parameters.";
  internalFunctionDescriptions["promptAgent"] =
      "Used To communicate with available types of agents (e.g sub-agents). "
      "Functions like talking to another person, all agents have pretty much "
      "the same runtime logic. Parameters: "
      "{\"agent_name\": \"string\", \"prompt\": \"string\"}";
  internalFunctionDescriptions["hotReload"] =
      "Reloads the agent's configuration and tools based off the agent-profile.yml conf. No parameters.";
  // internalFunctionDescriptions["summarizeText"] =
  //     "Summarizes provided text content. Parameters: {\"text\": "
  //     "\"string\"}"; // Corrected typo from summerizeTool
  // internalFunctionDescriptions["summarizeHistory"] =
  //     "Summarizes the current conversation history. No parameters.";
  // internalFunctionDescriptions["getWeather"] =
  //     "Fetches current weather. Parameters: {\"location\": \"string\"}";
}

Agent::~Agent() {
  logMessage(LogLevel::DEBUG, "Agent instance destroyed, cleaning up tools.",
             "Name: " + agentName);
  for (auto &pair : registeredTools) {
    delete pair.second;
  }
  registeredTools.clear();
}

// --- Configuration Setters (Implementations) ---
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
void Agent::setIterationCap(int cap) {
  iterationLimit = (cap > 0) ? cap : 10;
} // Ensure a positive cap, default 10
void Agent::setDirective(const AgentDirective &dir) { currentDirective = dir; }
void Agent::addTask(const std::string &task) { tasks.push_back(task); }
void Agent::addInitialCommand(const std::string &command) {
  initialCommands.push_back(command);
}

// --- Tool Management (Implementations) ---
void Agent::addTool(Tool *tool) {
  if (!tool) {
    logMessage(LogLevel::WARN, "Attempted to add a null tool.");
    return;
  }
  const std::string &toolNameStr = tool->getName(); // Use local var for clarity
  if (toolNameStr.empty()) {
    logMessage(LogLevel::WARN, "Attempted to add a tool with an empty name.");
    delete tool;
    return;
  }
  if (registeredTools.count(toolNameStr) ||
      internalFunctionDescriptions.count(toolNameStr)) {
    logMessage(LogLevel::WARN,
               "Agent '" + agentName +
                   "': Tool/internal function name conflict for '" +
                   toolNameStr + "'. Ignoring new tool.");
    delete tool;
  } else {
    registeredTools[toolNameStr] = tool;
    logMessage(LogLevel::INFO, "Agent '" + agentName + "' registered tool: '" +
                                   toolNameStr + "'");
  }
}

void Agent::removeTool(const std::string &toolNameKey) { // Renamed for clarity
  auto it = registeredTools.find(toolNameKey);
  if (it != registeredTools.end()) {
    delete it->second;
    registeredTools.erase(it);
    logMessage(LogLevel::INFO,
               "Agent '" + agentName + "' removed tool: '" + toolNameKey + "'");
  } else {
    logMessage(LogLevel::WARN,
               "Agent '" + agentName +
                   "' attempted to remove non-existent tool: '" + toolNameKey +
                   "'");
  }
}

Tool *Agent::getTool(const std::string &toolNameKey) const {
  auto it = registeredTools.find(toolNameKey);
  return (it != registeredTools.end()) ? it->second : nullptr;
}

// --- Core Agent Loop (Reset, Run - Implementations) ---
void Agent::reset() {
  conversationHistory.clear();
  scratchpad.clear();
  shortTermMemory.clear();
  // LongTermMemory might persist or be cleared based on deeper design choices
  currentIteration = 0;
  skipNextFlowIteration = false;
  logMessage(LogLevel::INFO, "Agent '" + agentName + "' state reset.");
}

void Agent::run() {
  logMessage(LogLevel::INFO,
             "Agent '" + agentName + "' starting interactive loop.");
  logMessage(LogLevel::INFO,
             "Type 'exit' or 'quit' to stop, 'reset' to clear history.");

  if (!initialCommands.empty()) {
    logMessage(LogLevel::INFO,
               "Executing " + std::to_string(initialCommands.size()) +
                   " initial commands for agent '" + agentName + "'...");
    std::vector<std::string> commandsToExecute = initialCommands;
    initialCommands.clear();

    for (const auto &cmd : commandsToExecute) {
      Json::Value bashParams;
      bashParams["command"] = cmd;
      logMessage(LogLevel::INFO,
                 "Running initial command for " + agentName + ":", cmd);
      std::string result = manualToolCall(
          "bash",
          bashParams); // Assuming "bash" tool is available for initial commands
      logMessage(LogLevel::INFO,
                 "Initial command result for " + agentName + ":",
                 result.substr(0, 200) + (result.length() > 200 ? "..." : ""));
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
    userInputText.erase(0, userInputText.find_first_not_of(" \t\r\n"));
    userInputText.erase(userInputText.find_last_not_of(" \t\r\n") + 1);

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
      std::string agentResponseText = prompt(userInputText); // Core call
      std::cout << "\n" << agentName << ":\n" << agentResponseText << std::endl;
      std::cout << "-----------------------------------------" << std::endl;
    } catch (const ApiError &e) { // Specific API error
      logMessage(LogLevel::ERROR,
                 "Agent API error for '" + agentName + "':", e.what());
      std::cout << "\n[Agent Error - API]: " << e.what() << std::endl;
    } catch (const std::runtime_error &e) {
      logMessage(LogLevel::ERROR,
                 "Agent runtime error for '" + agentName + "':", e.what());
      std::cout << "\n[Agent Error - Runtime]: " << e.what() << std::endl;
    } catch (const std::exception &e) {
      logMessage(LogLevel::ERROR,
                 "General exception for agent '" + agentName + "':", e.what());
      std::cout << "\n[Agent Error - General]: " << e.what() << std::endl;
    } catch (...) {
      logMessage(LogLevel::ERROR,
                 "Unknown error in agent '" + agentName + "' run loop.");
      std::cout << "\n[Agent Error - Unknown]: An unexpected error occurred."
                << std::endl;
    }
  }
  logMessage(LogLevel::INFO,
             "Agent '" + agentName + "' interactive loop finished.");
}

// --- Memory & State (Implementations) ---
void Agent::addToHistory(const std::string &role, const std::string &content) {
  const size_t MAX_HISTORY_CONTENT_LEN = 2500;
  std::string processedContent = content.substr(0, MAX_HISTORY_CONTENT_LEN);
  bool truncated = (content.length() > MAX_HISTORY_CONTENT_LEN);
  if (truncated)
    processedContent += "... (truncated)";
  conversationHistory.push_back({role, processedContent});
  logMessage(LogLevel::DEBUG, "Agent '" + agentName + "': Added to history.",
             "Role: " + role + (truncated ? " (Content Truncated)" : ""));
}
void Agent::addScratchpadItem(const std::string &key,
                              const std::string &value) {
  scratchpad.push_back({key, value});
}
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
  auto it =
      std::find_if(environmentVariables.begin(), environmentVariables.end(),
                   [&key](const auto &pair) { return pair.first == key; });
  if (it != environmentVariables.end()) {
    it->second = value;
    logMessage(LogLevel::DEBUG, "Agent '" + agentName + "': Updated env var.",
               key + "=" + value);
  } else {
    environmentVariables.push_back({key, value});
    logMessage(LogLevel::DEBUG, "Agent '" + agentName + "': Added env var.",
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
    line.erase(0, line.find_first_not_of(" \t"));
    if (line.empty() || line[0] == '#')
      continue;
    size_t eqPos = line.find('=');
    if (eqPos != std::string::npos) {
      std::string key = line.substr(0, eqPos);
      key.erase(key.find_last_not_of(" \t") + 1);
      std::string value = line.substr(eqPos + 1);
      value.erase(0, value.find_first_not_of(" \t"));
      value.erase(value.find_last_not_of(" \t") + 1);
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
                 " env vars from:",
             filePath);
}

void Agent::addExtraSystemPrompt(const std::string &promptFragment) {
  extraSystemPrompts.push_back(promptFragment);
}

// --- Getters (Implementations) ---
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

// --- Sub-Agent Management (Implementations) ---
void Agent::addSubAgent(Agent *subAgentInstance) {
  if (!subAgentInstance || subAgentInstance == this) {
    logMessage(LogLevel::WARN,
               "Agent '" + agentName +
                   "': Invalid sub-agent or self-addition attempt.");
    return;
  }
  if (std::find_if(subAgents.begin(), subAgents.end(), [&](const auto &p) {
        return p.first == subAgentInstance->getName();
      }) != subAgents.end()) {
    logMessage(LogLevel::WARN, "Agent '" + agentName +
                                   "' already has sub-agent '" +
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
      std::find_if(subAgents.begin(), subAgents.end(),
                   [&](const auto &p) { return p.first == subAgentNameKey; });
  return (it != subAgents.end()) ? it->second : nullptr;
}

// --- Manual Operations (Implementations) ---
std::string Agent::manualToolCall(const std::string &toolName,
                                  const Json::Value &params) {
  logMessage(LogLevel::INFO, "Agent '" + agentName +
                                 "': Manually calling tool '" + toolName + "'");
  ActionInfo ai;
  ai.action = toolName;
  ai.type = "tool"; // Assume registered tool for manual call
  ai.params = params;
  return processSingleAction(ai);
}

std::string Agent::executeApiCall(const std::string &fullPromptText) {
  logMessage(LogLevel::PROMPT,
             "Agent '" + agentName + "': Sending prompt to API.",
             "Length: " + std::to_string(fullPromptText.length()));
  // For extreme debugging: logMessage(LogLevel::DEBUG, "Full prompt text:",
  // fullPromptText);
  try {
    std::string response = api.generate(fullPromptText);
    logMessage(LogLevel::DEBUG,
               "Agent '" + agentName + "': Received API response.",
               "Length: " + std::to_string(response.length()));
    // For extreme debugging: logMessage(LogLevel::DEBUG, "Raw API Response:",
    // response.substr(0, 500));
    return response;
  } catch (const ApiError &e) { // Catch specific ApiError
    logMessage(LogLevel::ERROR,
               "Agent '" + agentName + "': API Error occurred.", e.what());
    Json::Value errorJson;
    errorJson["status"] = "ERROR_INTERNAL_API_CALL_FAILED";
    Json::Value thoughtError;
    thoughtError["type"] = "ERROR_OBSERVATION";
    thoughtError["content"] =
        "The call to the language model API failed: " + std::string(e.what());
    errorJson["thoughts"].append(thoughtError);
    errorJson["actions"] = Json::arrayValue; // Empty array for actions
    errorJson["final_response"] = "I encountered an issue communicating with "
                                  "the language model. The error was: " +
                                  std::string(e.what());
    Json::StreamWriterBuilder writer;
    writer["indentation"] = ""; // Compact error JSON
    return Json::writeString(writer, errorJson);
  } catch (const std::exception &e) { // Catch other standard exceptions
    logMessage(LogLevel::ERROR,
               "Agent '" + agentName + "': Standard exception during API call.",
               e.what());
    Json::Value errorJson; // Construct a similar error JSON
    errorJson["status"] = "ERROR_INTERNAL_STD_EXCEPTION_IN_API_CALL";
    Json::Value thoughtError;
    thoughtError["type"] = "ERROR_OBSERVATION";
    thoughtError["content"] =
        "A standard C++ exception occurred during the API call: " +
        std::string(e.what());
    errorJson["thoughts"].append(thoughtError);
    errorJson["actions"] = Json::arrayValue;
    errorJson["final_response"] =
        "A system error occurred while trying to reach the language model: " +
        std::string(e.what());
    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";
    return Json::writeString(writer, errorJson);
  }
}

void Agent::setSkipNextFlowIteration(bool skip) {
  skipNextFlowIteration = skip;
  if (skip) {
    logMessage(LogLevel::DEBUG, "Agent '" + agentName +
                                    "': Next flow iteration will be skipped.");
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
    return "UNKNOWN_DIRECTIVE";
  }
}

// --- Internal "Tool-Like" Function Implementations ---
std::string Agent::internalGetHelp(const Json::Value &params) {
  std::string targetActionName;
  if (params.isMember("action_name") && params["action_name"].isString()) {
    targetActionName = params["action_name"].asString();
  }
  std::stringstream helpSs;
  if (!targetActionName.empty()) {
    helpSs << "Help for action '" << targetActionName
           << "' requested by agent '" << agentName << "':\n";
    bool found = false;
    auto internalIt = internalFunctionDescriptions.find(targetActionName);
    if (internalIt != internalFunctionDescriptions.end()) {
      helpSs << "- Type: Internal Function\n- Description & Params: "
             << internalIt->second << "\n";
      found = true;
    }
    Tool *extTool = getTool(targetActionName);
    if (extTool) {
      if (found)
        helpSs << "---\n";
      helpSs << "- Type: Registered Tool\n- Description: "
             << extTool->getDescription() << "\n";
      // Add tool-specific examples if your Tool class supports it
      found = true;
    }
    if (!found)
      helpSs << "Action '" << targetActionName
             << "' not found or no help available.";
  } else {
    helpSs << "Available Actions for agent '" << agentName
           << "':\n--- Internal Functions ---\n";
    if (internalFunctionDescriptions.empty())
      helpSs << "(None)\n";
    for (const auto &pair : internalFunctionDescriptions)
      helpSs << "- " << pair.first << ": " << pair.second << "\n";
    helpSs << "\n--- Registered Tools ---\n";
    if (registeredTools.empty())
      helpSs << "(None)\n";
    for (const auto &pair : registeredTools) {
      if (pair.second)
        helpSs << "- " << pair.second->getName() << ": "
               << pair.second->getDescription() << "\n";
    }
    helpSs
        << "\nUse help with {\"action_name\": \"<action_name>\"} for details.";
  }
  return helpSs.str();
}

std::string Agent::internalSkipIteration(const Json::Value &params) {
  (void)params; // Params usually not needed for skip
  logMessage(LogLevel::INFO, "Agent '" + agentName +
                                 "': 'skip' action invoked. Next LLM call will "
                                 "be skipped for this turn.");
  setSkipNextFlowIteration(true);
  return "Success: Current agent turn processing will halt after this action; "
         "no final LLM response will be generated for this iteration.";
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
      std::string contextualPrompt =
          "CONTEXT: This prompt is from Agent '" + agentName +
          "'. Please process the following request:\n---\n" + subPromptText;
      std::string response = targetAgent->prompt(contextualPrompt);
      logMessage(LogLevel::INFO, "Agent '" + agentName +
                                     "' received response from sub-agent '" +
                                     targetAgentName + "'.");
      return "Response from Agent '" + targetAgentName + "':\n" + response;
    } catch (const std::exception &e) {
      logMessage(LogLevel::ERROR,
                 "Agent '" + agentName + "': Error prompting sub-agent '" +
                     targetAgentName + "'.",
                 e.what());
      return "Error [promptAgent]: Exception while prompting '" +
             targetAgentName + "': " + std::string(e.what());
    }
  }
  logMessage(LogLevel::WARN, "Agent '" + agentName + "': Sub-agent '" +
                                 targetAgentName +
                                 "' not found for prompting.");
  return "Error [promptAgent]: Sub-agent '" + targetAgentName + "' not found.";
}

std::string Agent::internalSummarizeText(const Json::Value &params) {
  if (!params.isMember("text") || !params["text"].isString()) {
    return "Error [summarizeText]: Missing or invalid string parameter 'text'.";
  }
  std::string textToSummarize = params["text"].asString();
  if (textToSummarize.length() < 30) {
    return "Content is too short to summarize meaningfully.";
  }
  logMessage(LogLevel::DEBUG, "Agent '" + agentName + "' summarizing text.",
             "Length: " + std::to_string(textToSummarize.length()));
  try {
    // Simplified: Create a one-off prompt for summarization
    std::string summarizationTaskPrompt =
        "You are a summarization expert. Provide a concise summary (2-3 "
        "sentences) of the following text:\n\n\"\"\"\n" +
        textToSummarize + "\n\"\"\"\n\nSummary:";
    // This call bypasses the agent's main loop for a direct LLM utility call
    std::string summary = api.generate(summarizationTaskPrompt);
    // Basic cleaning of the summary might be needed
    summary.erase(0, summary.find_first_not_of(" \t\r\n"));
    summary.erase(summary.find_last_not_of(" \t\r\n") + 1);
    return summary;
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR,
               "Agent '" + agentName + "': Error during summarization task.",
               e.what());
    return "Error [summarizeText]: Exception during summarization: " +
           std::string(e.what());
  }
}

std::string Agent::internalSummarizeHistory(const Json::Value &params) {
  (void)params;
  if (conversationHistory.empty())
    return "Conversation history is empty.";
  std::stringstream historySs;
  historySs
      << "Summary of the current conversation history (most recent last):\n";
  // Limit history length for summarization to avoid overly long prompts
  const int MAX_HISTORY_TURNS_FOR_SUMMARY = 10;
  int startIdx = std::max(0, (int)conversationHistory.size() -
                                 MAX_HISTORY_TURNS_FOR_SUMMARY);
  for (size_t i = startIdx; i < conversationHistory.size(); ++i) {
    const auto &entry = conversationHistory[i];
    historySs << entry.first << ": " << entry.second.substr(0, 150)
              << (entry.second.length() > 150 ? "..." : "") << "\n";
  }
  Json::Value summarizeParams;
  summarizeParams["text"] = historySs.str();
  return internalSummarizeText(summarizeParams);
}

std::string Agent::internalGetWeather(const Json::Value &params) {
  if (!params.isMember("location") || !params["location"].isString()) {
    return "Error [getWeather]: Missing or invalid string parameter "
           "'location'.";
  }
  std::string location = params["location"].asString();
  std::string originalLocation = location;
  std::replace(location.begin(), location.end(), ' ', '+'); // For wttr.in URL
  std::string command =
      "curl -s -L \"https://wttr.in/" + location +
      "?format=%l:+%t+(%f),+%C+%w\""; // More structured format

  logMessage(LogLevel::DEBUG,
             "Agent '" + agentName +
                 "' getting weather for: " + originalLocation,
             "Command: " + command);

  Tool *bashTool = getTool("bash");
  if (!bashTool) {
    logMessage(LogLevel::ERROR,
               "Agent '" + agentName +
                   "': 'bash' tool required for getWeather but not found.");
    return "Error [getWeather]: 'bash' tool is unavailable.";
  }
  Json::Value bashParams;
  bashParams["command"] = command;
  std::string weatherResultOutput = bashTool->execute(bashParams);
  weatherResultOutput.erase(0,
                            weatherResultOutput.find_first_not_of(" \t\r\n"));
  weatherResultOutput.erase(weatherResultOutput.find_last_not_of(" \t\r\n") +
                            1);

  if (weatherResultOutput.empty() ||
      weatherResultOutput.find("Unknown location") != std::string::npos ||
      weatherResultOutput.find("ERROR") != std::string::npos ||
      weatherResultOutput.find("Sorry, we are running into an issue") !=
          std::string::npos ||
      weatherResultOutput.find("wttr.in") !=
          std::string::npos) { // wttr.in often includes its name in error/info
                               // messages
    logMessage(LogLevel::WARN,
               "Agent '" + agentName + "': Failed to get weather via wttr.in.",
               "Location: " + originalLocation +
                   ", Output: " + weatherResultOutput);
    return "Error [getWeather]: Could not retrieve valid weather information "
           "for '" +
           originalLocation + "'.";
  }
  return "Weather for " + originalLocation + ": " + weatherResultOutput;
}

// --- Utility Implementations ---
std::string Agent::generateTimestamp() const {
  auto nowChrono = std::chrono::system_clock::now();
  auto nowTimeT = std::chrono::system_clock::to_time_t(nowChrono);
  std::tm nowTmLocalBuf;
#ifdef _WIN32
  localtime_s(&nowTmLocalBuf, &nowTimeT);
  std::tm *nowTm = &nowTmLocalBuf;
#else
  std::tm *nowTm = localtime_r(&nowTimeT, &nowTmLocalBuf);
#endif

  if (nowTm) {
    std::stringstream ss;
    ss << std::put_time(nowTm, "%Y-%m-%dT%H:%M:%S%Z"); // ISO 8601 like
    return ss.str();
  }
  return "[TIMESTAMP_ERROR]";
}

void Agent::trimLLMResponse(std::string &responseText) {
  // Finds ```json ... ``` or ``` ... ``` and extracts the content.
  size_t startPos = responseText.find("```");
  if (startPos == std::string::npos)
    return;

  // Look for "json" immediately after the first ```
  size_t contentActualStart = startPos + 3;
  if (responseText.length() > startPos + 7 &&
      responseText.substr(startPos + 3, 4) == "json") {
    // Skip "json" and any immediate newline/whitespace
    contentActualStart =
        responseText.find_first_not_of(" \t\r\n", startPos + 7);
    if (contentActualStart ==
        std::string::npos) { // Only ```json and whitespace after
      responseText = "";
      return;
    }
  } else {
    // Skip just ``` and any immediate newline/whitespace
    contentActualStart =
        responseText.find_first_not_of(" \t\r\n", startPos + 3);
    if (contentActualStart ==
        std::string::npos) { // Only ``` and whitespace after
      responseText = "";
      return;
    }
  }

  size_t endPos = responseText.rfind("```");
  if (endPos == std::string::npos || endPos <= contentActualStart)
    return;

  // Content is between contentActualStart and just before endPos
  responseText =
      responseText.substr(contentActualStart, endPos - contentActualStart);
  // Trim any leading/trailing whitespace from the extracted content itself
  responseText.erase(0, responseText.find_first_not_of(" \t\r\n"));
  responseText.erase(responseText.find_last_not_of(" \t\r\n") + 1);

  logMessage(LogLevel::DEBUG,
             "Agent '" + agentName + "': Trimmed LLM response code block.");
}

