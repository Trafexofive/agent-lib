#pragma once

#include "File.hpp"       // If fileList is used
#include "MiniGemini.hpp" // Assuming your LLM client
#include "Tool.hpp"       // The simplified Tool class
#include "Utils.hpp"      // For utility functions if Agent calls them directly

#include <chrono>  // For timestamp generation
#include <iomanip> // For timestamp formatting
#include <json/json.h>
#include <map>
#include <memory> // For std::unique_ptr if you decide to use it for tools later
#include <stdexcept> // For std::runtime_error
#include <string>
#include <vector>

// Forward declarations
namespace Json {
class Value;
}
class MiniGemini; // Already included but good practice if only pointers/refs
                  // used in some contexts
class Tool;       // Already included

// Logging Enum & Function Prototype (must be implemented, e.g., in agent.cpp)
enum class LogLevel {
  DEBUG,
  INFO,
  WARN,
  ERROR,
  TOOL_CALL,
  TOOL_RESULT,
  PROMPT
};

void logMessage(LogLevel level, const std::string &message,
                const std::string &details = "");

// Typedefs
using FileList = std::vector<File>; // Assuming File class from File.hpp
using StringKeyValuePair = std::vector<std::pair<std::string, std::string>>;

// ActionInfo struct for parsing LLM responses and directing actions
struct Action {
  std::string action;
  std::string
      type; // "tool", "internal_function", "output", "workflow_control", etc.

  Json::Value params;

  double confidence = 1.0;

  std::vector<std::string> warnings;
};

// StructuredThought for LLM's reasoning process
struct Thought {
  std::string type; // e.g., PLAN, OBSERVATION, QUESTION, NORM, etc.
  std::string content;
};

class Agent {
public:
  // Agent Directive (publicly accessible for configuration)
  struct AgentDirective {
    enum class Type { BRAINSTORMING, AUTONOMOUS, NORMAL, EXECUTE, REPORT };
    Type type = Type::NORMAL;
    std::string description;
    std::string format; // Expected format for directive-specific output
  };

  // --- Constructor & Destructor ---
  Agent(MiniGemini &apiRef, const std::string &agentName = "defaultAgent");
  ~Agent();

  // --- Configuration Setters (for YAML loading and programmatic setup) ---
  void setName(const std::string &newName);
  void setDescription(const std::string &newDescription);
  void setSystemPrompt(const std::string &prompt);
  void setSchema(const std::string &schema);   // LLM Response Schema
  void setExample(const std::string &example); // Example LLM Response
  void setIterationCap(int cap);
  void setDirective(const AgentDirective &directive);
  void addTask(const std::string &task);
  void addInitialCommand(
      const std::string &command); // For commands to run on agent startup

  // --- Tool Management (using simplified Tool class) ---
  void addTool(Tool *tool); // Agent takes ownership of the raw pointer
  void removeTool(const std::string &toolName); // Deletes the tool
  Tool *getTool(const std::string &toolName) const;

  // --- Core Agent Loop ---
  void reset(); // Clears history, iteration count, etc.
  std::string prompt(const std::string &userInput); // Main interaction point
  void run(); // Interactive command-line loop

  // --- Memory & State ---
  void addToHistory(const std::string &role, const std::string &content);
  void addScratchpadItem(const std::string &key, const std::string &value);
  void addShortTermMemory(const std::string &role, const std::string &content);
  void addLongTermMemory(const std::string &role, const std::string &content);
  void addEnvironmentVariable(const std::string &key, const std::string &value);
  void importEnvironmentFile(
      const std::string &filePath); // Loads from a .env style file
  void addExtraSystemPrompt(
      const std::string &promptFragment); // Appends to extra prompts

  // --- Getters (for saving profile or introspection) ---
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
  const std::vector<std::pair<std::string, std::string>> &
  getHistory() const; // For introspection/debugging

  // --- Sub-Agent Management (Basic) ---
  void
  addSubAgent(Agent *subAgentInstance); // Agent doesn't own sub-agent pointers
  Agent *getSubAgent(const std::string &subAgentName) const;

  // --- Manual Operations ---
  std::string manualToolCall(const std::string &toolName,
                             const Json::Value &params);

  class IContext {
  public:
    virtual ~IContext() = default;
    virtual std::string getContext() const = 0;
    virtual void setContext(const std::string &context) = 0;
    virtual void clearContext() = 0;

  };

private:
  // --- Core Members ---
  MiniGemini &api; // Reference to the LLM API client

  std::string agentName;
  std::string agentDescription;
  std::string systemPrompt;

  std::string llmResponseSchema;  // Stores the "schema" field from YAML
  std::string llmResponseExample; // Stores the "example" field from YAML

  std::vector<std::pair<std::string, std::string>> conversationHistory;
  int currentIteration = 0;
  int iterationLimit = 10;
  bool skipNextFlowIteration = false;

  // --- Configuration & Context ---
  StringKeyValuePair environmentVariables;
  FileList agentFiles; // List of files associated with the agent (if any)
  std::vector<std::string> extraSystemPrompts;
  std::vector<std::pair<std::string, Agent *>> subAgents; // Name -> Agent*

  // --- Memory Stores ---
  StringKeyValuePair scratchpad;
  StringKeyValuePair shortTermMemory;
  StringKeyValuePair longTermMemory;

  std::vector<std::string>
      tasks; // Tasks defined in YAML or added programmatically
  std::vector<std::string> initialCommands; // Shell commands to run at start

  AgentDirective currentDirective;

  // --- Tools (using simplified Tool class) ---
  std::map<std::string, Tool *> registeredTools; // Agent owns these Tool*
  std::map<std::string, std::string>
      internalFunctionDescriptions; // For agent's built-in functions

  // --- Private Helper Methods ---
  std::string buildFullPrompt() const;
  bool parseStructuredLLMResponse(const std::string &jsonString,
                                  std::string &status,
                                  std::vector<Thought> &thoughts,
                                  std::vector<Action> &actions,
                                  std::string &finalResponseField);
  std::string processActions(const std::vector<Action> &actions);
  std::string
  processSingleAction(const Action &actionInfo); // Renamed for clarity
  std::string executeApiCall(const std::string &fullPrompt);
  void setSkipNextFlowIteration(bool skip);
  std::string directiveTypeToString(AgentDirective::Type type) const;

  // --- Internal "Tool-Like" Functions (not exposed as separate Tool objects)
  // ---
  std::string internalGetHelp(const Json::Value &params);
  std::string internalSkipIteration(const Json::Value &params);
  std::string internalPromptAgent(const Json::Value &params);
  std::string internalSummarizeText(
      const Json::Value &params); // New name for summerizeTool
  std::string internalSummarizeHistory(const Json::Value &params);
  std::string internalGetWeather(const Json::Value &params);
  // Add more internal functions as needed (e.g., reportTool, generateCode)
  // std::string internalReportTool(const Json::Value& params);
  // std::string internalGenerateCode(const Json::Value& params);

  // Utility
  std::string generateTimestamp() const;
  void trimLLMResponse(std::string &responseText); // Helper for ```json ... ```
};
