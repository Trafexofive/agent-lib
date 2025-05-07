#pragma once

#include "File.hpp" // Forward declare if only pointers/refs used, include if values/size needed
#include "MiniGemini.hpp" // Forward declare if only pointers/refs used
#include "Tool.hpp"       // Forward declare if only pointers/refs used

#include <cerrno>
#include <chrono>  // For timestamp generation
#include <iomanip> // For timestamp formatting
#include <map>
#include <stdexcept> // For std::runtime_error
#include <string>
#include <vector>

// Needed for ActionInfo struct and some method parameters
#include <json/json.h> // Includes json/value.h
                       //

// Forward declarations (if full definitions aren't needed in this header)
namespace Json {
class Value;
}
class MiniGemini;
class Tool;
class File;

// Typedefs
typedef std::vector<File> fileList;
typedef std::vector<std::pair<std::string, std::string>> StrkeyValuePair;

// Logging Enum
enum class LogLevel {
  DEBUG,
  INFO,
  WARN,
  ERROR,
  TOOL_CALL,
  TOOL_RESULT,
  PROMPT
};

// Logging Function Prototype (Implementation likely in agent.cpp or logging
// util)
void logMessage(LogLevel level, const std::string &message,
                const std::string &details = "");

// --- Struct Definitions for LLM Interaction (NEW/UPDATED) ---

struct StructuredThought {
  std::string type;    // e.g., PLAN, OBSERVATION, QUESTION, NORM, etc.
  std::string content; // The text of the thought
};

struct ActionInfo {
  std::string action; // Identifier (tool name, function name, script name)
  std::string type;   // Category (tool, script, internal_function, output,
                      // workflow_control)
  Json::Value
      params; // Parameters for the action (ensure json/json.h is included)
  double confidence = 1.0;           // Optional confidence score
  std::vector<std::string> warnings; // Optional warnings

  // Default constructor might be needed depending on usage
  ActionInfo() = default;
};

// --- Struct Definition for Directive (PUBLIC) ---
struct AgentDirective {
  enum Type { BRAINSTORMING, AUTONOMOUS, NORMAL, EXECUTE, REPORT };
  Type type = NORMAL;
  std::string description;
  std::string format;
};

namespace types {

// dealing with runtime data context
typedef enum {
  FILE,
  FOLDER,
  EXECUTABLE, // could be  script
  STRING,
} VarType;

} // namespace types

#define FILE_EXT_ALLOWED                                                       \
  {".txt", ".csv", ".json", ".xml", ".yaml", ".yml", ".md", ".log"}

class CONTEXT_ITEM {
private:
  unsigned long id; // will just index them for now, imma look into how to
                    // porperly implement uuid  later.

  std::string name; //  in this case it is refering to the varname inside the
                    //  yaml file
  types::VarType type;
  std::string path;
  std::string content;

  std::string runtime; // for executable type only.

  bool loaded;
};

// --- Agent Class Definition ---
class Agent {
public:
  // Public alias for the directive struct
  using DIRECTIVE = AgentDirective;

  // Struct for OLD tool call format (potentially deprecated)
  struct ToolCallInfo {
    std::string toolName;
    Json::Value params;
  };

  // --- Constructor ---
  Agent(MiniGemini &api);

  // --- Static Factory (Optional) ---
  // static std::unique_ptr<Agent> loadFromYaml(const std::string& yamlPath,
  // MiniGemini& api);

  // --- Configuration (Public Setters - Now including Directive/Task/Command)
  // ---
  void setSystemPrompt(const std::string &prompt);
  void setName(const std::string &name);
  void setDescription(const std::string &description);
  void setIterationCap(int cap);
  void setDirective(const DIRECTIVE &dir);            // MADE PUBLIC
  void addTask(const std::string &task);              // MADE PUBLIC
  void addInitialCommand(const std::string &command); // MADE PUBLIC
  void setSchema(const std::string &schema);
  void setExample(const std::string &example) { this->example = example; }

  // --- Tool Management ---
  void addTool(Tool *tool);
  void removeTool(const std::string &toolName);
  Tool *getTool(const std::string &toolName) const;
  Tool *getRegisteredTool(const std::string &toolName) const;
  std::string getInternalToolDescription(const std::string &toolName) const;

  // finds a tool by name then add it as a built-in
  void addBuiltin(const std::string &toolName);

  // --- Core Agent Loop ---
  void reset();
  std::string prompt(const std::string &userInput);
  void run();

  // --- Agent Interaction (Orchestration) ---
  void addAgent(Agent *agent);
  std::string
  agentInstance(const std::string &name); // Gets sub-agent name (simple check)

  // --- Manual Operations ---
  std::string manualToolCall(const std::string &toolName,
                             const Json::Value &params);

  // --- Public Getters (Ensure const correctness and existence) ---
  const std::string getName() const;
  const std::string getDescription() const;
  MiniGemini &getApi(); // Cannot be const if MiniGemini methods aren't const
  fileList getFiles() const;
  const std::vector<std::pair<std::string, std::string>> &getHistory() const;

  // --- Getters needed for saveAgentProfile (Added/Made Public/Const) ---
  const std::string &getSystemPrompt() const;
  int getIterationCap() const;
  const std::vector<std::pair<std::string, std::string>> &getEnv() const;
  const std::vector<std::string> &getExtraPrompts() const;
  const std::vector<std::string> &getTasks() const;
  const DIRECTIVE &getDirective() const; // MADE PUBLIC and CONST

  // --- Memory & State (Public Modifiers) ---
  void addMemory(const std::string &role, const std::string &content);
  void removeMemory(const std::string &role, const std::string &content);
  void addEnvVar(const std::string &key, const std::string &value);
  void importEnvFile(const std::string &filePath);
  void addPrompt(const std::string &prompt); // Adds to extraPrompts

  // --- Utilities ---
  std::string generateStamp(void);
  void addToHistory(const std::string &role, const std::string &content);
  std::string wrapText(const std::string &text);
  std::string wrapXmlLine(const std::string &content, const std::string &tag);
  std::string wrapXmlBlock(const std::string &content, const std::string &tag);
  void addBlockToSystemPrompt(const std::string &content,
                              const std::string &tag);

  void listAllAgentInfo() {
    std::cout << "Agent Name: " << name << "\n"
              << "Description: " << description << "\n"
              << "System Prompt: " << systemPrompt << "\n"
              << "Iteration Cap: " << iterationCap << "\n"
              << "Current Iteration: " << iteration << "\n"
              << "Schema: " << schema << "\n"
              << "Tasks: ";
    for (const auto &task : tasks) {
      std::cout << task << ", ";
    }
    std::cout << "\n";
  }

private:
  // --- Private Members (Clean Naming Scheme) ---
  MiniGemini &api;
  std::map<std::string, Tool *> tools;
  std::map<std::string, std::string> internalToolDescriptions;

  std::string systemPrompt;
  std::vector<std::pair<std::string, std::string>> history;

  int iteration = 0;
  int iterationCap = 10;
  bool skipFlowIteration = false;

  std::vector<std::pair<std::string, std::string>> env;

  fileList files; // Assuming File class definition exists
  std::string name;
  std::string description;

  std::vector<std::string> extraPrompts;

  std::vector<std::pair<std::string, Agent *>> subAgents;

  StrkeyValuePair scratchpad;
  StrkeyValuePair shortTermMemory;
  StrkeyValuePair longTermMemory;

  std::vector<std::string> tasks;
  std::vector<std::string> initialCommands;

  DIRECTIVE directive; // Private member instance192.168.1.101

  std::string schema;
  std::string example;

  // std::vector<std::string, CONTEXT_ITEM> context;
  std::map<std::string, CONTEXT_ITEM> contextMap;
  // --- Private Helper Methods ---
  std::string buildFullPrompt() const;
  // --- UPDATED parseStructuredLLMResponse declaration ---
  bool parseStructuredLLMResponse(
      const std::string &jsonString,
      std::string &status,                      // Changed
      std::vector<StructuredThought> &thoughts, // Changed
      std::vector<ActionInfo> &actions,         // Changed
      std::string &finalResponse);

  // --- ADDED declarations for new private helpers ---
  std::string processActions(const std::vector<ActionInfo> &actions);
  std::string processAction(const ActionInfo &actionInfo);
  std::string directiveTypeToString(
      Agent::DIRECTIVE::Type type) const; // Helper for buildFullPrompt

  // --- Potentially Deprecated Fallback/Old Methods ---
  std::vector<ToolCallInfo>
  extractToolCalls(const std::string &response); // Deprecated?
  std::string
  processToolCalls(const std::vector<ToolCallInfo> &toolCalls); // Deprecated?
  std::string handleToolExecution(const ToolCallInfo &call);    // Deprecated?

  std::string executeApiCall(const std::string &fullPrompt);
  void setSkipFlowIteration(bool skip);

  // --- Internal Tool Implementations (Remain Private) ---
  std::string getHelp(const Json::Value &params);
  std::string skip(const Json::Value &params);
  std::string promptAgentTool(const Json::Value &params);
  std::string summerizeTool(const Json::Value &params); // Typo preserved
  std::string summarizeHistory(const Json::Value &params);
  std::string getWeather(const Json::Value &params);
  std::string reportTool(const Json::Value &params);
  std::string generateCode(const Json::Value &params);

  // --- Task Execution Helpers (Remain Private) ---
  std::string
  executeTask(const std::string &task,
              const Json::Value &format); // Needs Json::Value variant if used
  std::string executeTask(const std::string &task);
  std::string executeTask(const std::string &task, const std::string &format);
  std::string auditResponse(const std::string &response);
};
