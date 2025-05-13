#pragma once

#include "File.hpp"
#include "MiniGemini.hpp"
#include "Tool.hpp"
#include "Utils.hpp"

#include <chrono>
#include <iomanip>
#include <json/json.h>
#include <map>
#include <memory>
#include <stack> // For potential future use, not strictly needed by current regeneration
#include <stdexcept>
#include <string>
#include <vector>

// Forward declarations
namespace Json {
class Value;
}
class MiniGemini;
class Tool;
class File; // Assuming File.hpp defines this

void logMessage(LogLevel level, const std::string &message,
                const std::string &details = "");

// Typedefs
using FileList = std::vector<File>;
using StringKeyValuePair = std::vector<std::pair<std::string, std::string>>;

// --- Structs for LLM Interaction ---

struct StructuredThought {
  std::string type;
  std::string content;
};

// ActionInfo is designed for extensibility.
// To add a new core field to all actions (e.g., 'priority'):
// 1. Add the member here (e.g., `int priority = 0;`).
// 2. Update system prompts to instruct the LLM to include this field.
// 3. Update `Agent::parseStructuredLLMResponse` to parse it from the JSON.
// Tool-specific parameters remain flexible within `Json::Value params`.
struct ActionInfo {
  std::string action;
  std::string type;
  Json::Value params;
  double confidence = 1.0; // Default confidence
  std::vector<std::string> warnings;
};

struct ParsedLLMResponse {
  bool success = false;
  std::string status;
  std::vector<StructuredThought> thoughts;
  std::vector<ActionInfo> actions;
  std::string finalResponseField;
  std::string rawTrimmedJson;
};

class Agent {
public:
  struct AgentDirective {
    enum class Type { BRAINSTORMING, AUTONOMOUS, NORMAL, EXECUTE, REPORT };
    Type type = Type::NORMAL;
    std::string description;
    std::string format;
  };
  using DIRECTIVE = AgentDirective;

  // --- Constructor & Destructor ---
  Agent(MiniGemini &apiRef, const std::string &agentName = "defaultAgent");
  ~Agent();

  // --- Configuration Setters ---
  void setName(const std::string &newName);
  void setDescription(const std::string &newDescription);
  void setSystemPrompt(const std::string &prompt);
  void
  setSchema(const std::string &schema); // For LLM Response Schema (as guide)
  void
  setExample(const std::string &example); // For Example LLM Response (as guide)
  void setIterationCap(int cap);
  void setDirective(const AgentDirective &directive);
  void addTask(const std::string &task); // Conceptual task for prompting
  void addInitialCommand(const std::string &command);

  // --- Tool Management ---
  void addTool(Tool *tool); // Agent takes ownership of this raw pointer
  void removeTool(const std::string &toolName); // Deletes the tool
  Tool *getTool(const std::string &toolName) const;

  // --- Core Agent Loop ---
  void reset();
  std::string prompt(const std::string &userInput);
  void run(); // Interactive CLI loop

  // --- Memory & State ---
  void addToHistory(const std::string &role, const std::string &content);
  void addScratchpadItem(const std::string &key, const std::string &value);
  void addShortTermMemory(const std::string &role, const std::string &content);
  void addLongTermMemory(const std::string &role, const std::string &content);
  void addEnvironmentVariable(const std::string &key, const std::string &value);
  void importEnvironmentFile(const std::string &filePath);
  void addExtraSystemPrompt(const std::string &promptFragment);

  // --- Getters ---
  const std::string &getName() const;
  const std::string &getDescription() const;
  const std::string &getSystemPrompt() const;
  const std::string &getSchema() const;
  const std::string &getExample() const;
  int getIterationCap() const;
  const AgentDirective &getDirective() const;
  const std::vector<std::string> &getTasks() const;
  const StringKeyValuePair &getEnvironmentVariables() const;
  const std::vector<std::string> &getExtraSystemPrompts() const;
  const std::vector<std::pair<std::string, std::string>> &getHistory() const;

  // --- Sub-Agent Management ---
  void
  addSubAgent(Agent *subAgentInstance); // Agent does NOT own sub-agent pointers
  Agent *getSubAgent(const std::string &subAgentName) const;

  // --- Manual Operations ---
  std::string manualToolCall(const std::string &toolName,
                             const Json::Value &params);

private:
  // --- Core Members ---
  MiniGemini &api;
  std::string agentName;
  std::string agentDescription;
  std::string systemPrompt;
  std::string llmResponseSchema;  // For guiding LLM, not strict validation here
  std::string llmResponseExample; // For guiding LLM

  std::vector<std::pair<std::string, std::string>> conversationHistory;
  int currentIteration;
  int iterationLimit;
  bool skipNextFlowIteration;

  StringKeyValuePair environmentVariables;
  FileList agentFiles;
  std::vector<std::string> extraSystemPrompts;
  std::vector<std::pair<std::string, Agent *>> subAgents; // name -> Agent*

  StringKeyValuePair scratchpad;
  StringKeyValuePair shortTermMemory;
  StringKeyValuePair longTermMemory;

  std::vector<std::string> tasks;
  std::vector<std::string> initialCommands;

  AgentDirective currentDirective;

  std::map<std::string, Tool *> registeredTools; // Agent owns these Tool*
  std::map<std::string, std::string> internalFunctionDescriptions;

  // --- Private Helper Methods ---
  std::string buildFullPrompt() const;
  ParsedLLMResponse
  parseStructuredLLMResponse(const std::string &trimmedJsonString);
  std::string processActions(const std::vector<ActionInfo> &actions);
  std::string processSingleAction(const ActionInfo &actionInfo);
  std::string executeApiCall(const std::string &fullPrompt);
  void setSkipNextFlowIteration(bool skip);
  std::string directiveTypeToString(AgentDirective::Type type) const;

  // Internal "Tool-Like" Functions
  std::string internalGetHelp(const Json::Value &params);
  std::string internalSkipIteration(const Json::Value &params);
  std::string internalPromptAgent(const Json::Value &params);
  std::string internalSummarizeText(const Json::Value &params);
  std::string internalSummarizeHistory(const Json::Value &params);
  std::string internalGetWeather(const Json::Value &params);

  // Utility
  std::string generateTimestamp() const;
  void trimLLMResponse(std::string &responseText);
};
