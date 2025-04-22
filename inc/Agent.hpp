#pragma once

#include "File.hpp"
#include "MiniGemini.hpp" // Include MiniGemini header
#include "Tool.hpp"       // Include Tool header
#include <algorithm>
#include <chrono> // For timestamp
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <curl/curl.h>
#include <fstream>
#include <iomanip> // For timestamp formatting
#include <iostream>
#include <json/json.h> // For Json::Value
#include <json/value.h>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// inc/Agent.hpp (Regenerated with updates)

#include <string>
#include <vector>
#include <map>
#include <stdexcept> // For std::runtime_error
#include <chrono>    // For timestamp generation
#include <iomanip>   // For timestamp formatting

// Forward declarations to reduce header dependencies
namespace Json { class Value; }
class MiniGemini;
class Tool;
class File; // Assuming File.hpp is included where needed, not necessarily here

// Typedefs for clarity
typedef std::vector<File> fileList; // If File class is used
typedef std::vector<std::pair<std::string, std::string>> StrkeyValuePair;

// Logging Enum (Definition remains the same)
enum class LogLevel {
    DEBUG, INFO, WARN, ERROR, TOOL_CALL, TOOL_RESULT, PROMPT
};

// Logging Function Prototype (Implementation in agent.cpp)
void logMessage(LogLevel level, const std::string &message, const std::string &details = "");


class Agent {
public:
    // Structure for parsed tool calls (remains the same)
    struct ToolCallInfo {
        std::string toolName;
        Json::Value params; // Always JSON parameters
    };

    // Constructor takes API client by reference
    Agent(MiniGemini &api);

    // --- Configuration ---
    void setSystemPrompt(const std::string &prompt);
    void setName(const std::string &name);
    void setDescription(const std::string &description);
    void setIterationCap(int cap); // Declaration added

    // --- Tool Management ---
    void addTool(Tool *tool); // Manages Tool pointers (caller owns the Tool)
    void removeTool(const std::string &toolName);
    Tool *getTool(const std::string &toolName) const; // Gets external tool
    Tool *getRegisteredTool(const std::string& toolName) const; // ** NEW ** Gets tool registered to *this* agent
    std::string getInternalToolDescription(const std::string &toolName) const; // Gets description only

    // --- Core Agent Loop ---
    void reset(); // Clears history and resets state
    std::string prompt(const std::string &userInput); // Processes a single user turn
    void run(); // Starts the interactive console loop

    // --- Agent Interaction (Orchestration) ---
    void addAgent(Agent *agent); // Adds a sub-agent
    std::string agentInstance(const std::string &name); // Gets sub-agent name (simple check)

    // --- Manual Operations ---
    std::string manualToolCall(const std::string &toolName, const Json::Value &params);

    // --- Getters ---
    const std::string getName() const;
    const std::string getDescription() const;
    MiniGemini &getApi(); // Access the API client
    fileList getFiles() const; // Get associated files (if File class is used)
    const std::vector<std::pair<std::string, std::string>>& getHistory() const; // Access conversation history

    // --- Memory & State ---
    void addMemory(const std::string &role, const std::string &content); // Adds to LongTermMemory
    void removeMemory(const std::string &role, const std::string &content); // Removes from LongTermMemory
    void addEnvVar(const std::string &key, const std::string &value);
    void importEnvFile(const std::string &filePath); // Imports from .env style file
    void addPrompt(const std::string &prompt); // Adds to extraPrompts

    // --- Persistence (Conceptual - No implementation details here) ---
    // void loadState(const std::string& path);
    // void saveState(const std::string& path);

    // --- Utilities ---
    std::string generateStamp(void); // Generates timestamp string
    void addToHistory(const std::string &role, const std::string &content); // Adds formatted entry to history
    std::string wrapText(const std::string &text); // Wraps in ```
    std::string wrapXmlLine(const std::string &content, const std::string &tag); // Wraps in <tag>content</tag>\n
    std::string wrapXmlBlock(const std::string &content, const std::string &tag); // Wraps in <tag>\n\tcontent\n</tag>\n
    void addBlockToSystemPrompt(const std::string &content, const std::string &tag); // Appends XML block to system prompt


private: // Private members should come after public interface
    // --- Core Components ---
    MiniGemini &m_api;
    std::map<std::string, Tool *> m_tools; // External tools registered with this agent
    std::map<std::string, std::string> m_internalToolDescriptions; // Descriptions only

    // --- State ---
    std::string m_systemPrompt;
    std::vector<std::pair<std::string, std::string>> m_history; // Role, Content pairs
    int iteration = 0;
    int iterationCap = 10;
    bool skipFlowIteration = false;
    std::vector<std::pair<std::string, std::string>> _env;
    fileList _files; // If File class is used
    std::string _name;
    std::string _description;
    std::vector<std::string> extraPrompts;

    // --- Orchestration ---
    std::vector<std::pair<std::string, Agent *>> agents; // Sub-agents managed by this agent

    // --- Memory Tiers (Conceptual - represented simply for now) ---
    StrkeyValuePair Scratchpad;      // Transient per-prompt data
    StrkeyValuePair ShortTermMemory; // Could hold structured context beyond history
    StrkeyValuePair LongTermMemory;  // Persistent key-value store (implementation uses SQLite via MemoryManager eventually)

    // --- Task Management (Conceptual) ---
    std::vector<std::string> tasks;
    std::vector<std::string> initialCommands;

    // --- Directives (Conceptual) ---
    struct DIRECTIVE {
        enum Type { BRAINSTORMING, AUTONOMOUS, NORMAL, EXECUTE, REPORT };
        Type type = NORMAL;
        std::string description;
        std::string format;
    };
    DIRECTIVE directive;

    // --- Helper Methods ---
    std::string buildFullPrompt() const;
    std::vector<ToolCallInfo> extractToolCalls(const std::string &response); // Parses ```tool:...``` blocks (OLD - might be replaced or unused if parseStructuredLLMResponse is primary)
    std::string processToolCalls(const std::vector<ToolCallInfo> &toolCalls);
    std::string handleToolExecution(const ToolCallInfo &call);
    std::string executeApiCall(const std::string &fullPrompt);
    void setSkipFlowIteration(bool skip);

    // ** NEW ** Helper for parsing structured LLM JSON output
    bool parseStructuredLLMResponse(const std::string& jsonString, std::string& thought, std::vector<ToolCallInfo>& toolCalls, std::string& finalResponse);

    // --- Internal Tool Implementations (Prototypes) ---
    // These are called by handleToolExecution, no need to be public unless called externally
    std::string getHelp(const Json::Value &params);
    std::string skip(const Json::Value &params);
    std::string promptAgentTool(const Json::Value &params);
    std::string summerizeTool(const Json::Value &params); // Intentional typo? Should be summarizeTool?
    std::string summarizeHistory(const Json::Value &params);
    std::string getWeather(const Json::Value &params);
    std::string reportTool(const Json::Value &params); // Placeholder
    std::string generateCode(const Json::Value &params); // Placeholder
    // std::string createAgentFromYamlTool(const Json::Value& params, Agent& callingAgent); // Needs Agent& param, handled via Tool constructor

    // --- Task Execution Helpers (Conceptual - Prototypes) ---
    std::string executeTask(const std::string &task, const Json::Value &format);
    std::string executeTask(const std::string &task); // Overload without format
    std::string executeTask(const std::string &task, const std::string &format); // Overload with string format
    std::string auditResponse(const std::string &response); // Placeholder

    // --- Getters/Setters for Private Members (if needed) ---
    DIRECTIVE &getDirective(); // Example
    void setDirective(const DIRECTIVE &dir); // Example
    void addTask(const std::string &task); // Example
    void addInitialCommand(const std::string &command); // Example
};
