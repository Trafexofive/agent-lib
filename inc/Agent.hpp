#pragma once

#include "File.hpp"
#include "MiniGemini.hpp"
#include "Tool.hpp"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <ctime>
// #include <curl/curl.h> // Include if needed
#include <fstream>
#include <iomanip>
#include <iostream>
#include <json/json.h>
// #include <json/value.h> // Included by json/json.h
#include <map>
#include <memory> // For std::unique_ptr if used elsewhere
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Forward declarations
namespace Json { class Value; }
// namespace YAML { class Node; } // Only needed if Agent methods take YAML nodes directly
class MiniGemini;
class Tool;
class File;

typedef std::vector<File> fileList;
typedef std::vector<std::pair<std::string, std::string>> StrkeyValuePair;

enum class LogLevel {
    DEBUG, INFO, WARN, ERROR, TOOL_CALL, TOOL_RESULT, PROMPT
};

void logMessage(LogLevel level, const std::string &message, const std::string &details = "");

// --- MOVED TO PUBLIC ---
// Needs to be accessible outside the Agent class for configuration/loading.
struct AgentDirective { // Renamed slightly to avoid ambiguity if Agent has a member named directive
    enum Type { BRAINSTORMING, AUTONOMOUS, NORMAL, EXECUTE, REPORT };
    Type type = NORMAL;
    std::string description;
    std::string format;
};


class Agent {
public: // Make sure this is the public section
    // --- MOVED/ADDED TO PUBLIC for profile loading/saving ---
    using DIRECTIVE = AgentDirective; // Add using alias for convenience if preferred


    struct ToolCallInfo {
        std::string toolName;
        Json::Value params;
    };

    // --- Constructor ---
    Agent(MiniGemini &api);

    // --- Static Factory for YAML Loading (if you have one) ---
    // static std::unique_ptr<Agent> loadFromYaml(const std::string& yamlPath, MiniGemini& api);

    // --- Configuration (Public Setters) ---
    void setSystemPrompt(const std::string &prompt);
    void setName(const std::string &name);
    void setDescription(const std::string &description);
    void setIterationCap(int cap);
    // --- MADE PUBLIC ---
    void setDirective(const DIRECTIVE &dir);

    // --- Tool Management ---
    void addTool(Tool *tool);
    void removeTool(const std::string &toolName);
    Tool *getTool(const std::string &toolName) const;
    Tool *getRegisteredTool(const std::string& toolName) const;
    std::string getInternalToolDescription(const std::string &toolName) const;

    // --- Core Agent Loop ---
    void reset();
    std::string prompt(const std::string &userInput);
    void run();

    // --- Agent Interaction (Orchestration) ---
    void addAgent(Agent *agent);
    std::string agentInstance(const std::string &name);

    // --- Manual Operations ---
    std::string manualToolCall(const std::string &toolName, const Json::Value &params);

    // --- Public Getters (Ensure these exist and are const) ---
    const std::string getName() const;
    const std::string getDescription() const;
    MiniGemini &getApi(); // Cannot be const if MiniGemini methods aren't const
    fileList getFiles() const; // Assuming File is copyable or returned appropriately
    const std::vector<std::pair<std::string, std::string>>& getHistory() const;

    // --- ADDED/MODIFIED Getters needed for saveAgentProfile ---
    const std::string& getSystemPrompt() const; // Return const ref
    int getIterationCap() const;
    const std::vector<std::pair<std::string, std::string>>& getEnv() const; // Return const ref
    const std::vector<std::string>& getExtraPrompts() const; // Return const ref
    const std::vector<std::string>& getTasks() const; // Return const ref
    // --- MADE PUBLIC and CONST ---
    const DIRECTIVE& getDirective() const; // Return const ref and mark method const

    // --- Memory & State (Public Modifiers) ---
    void addMemory(const std::string &role, const std::string &content);
    void removeMemory(const std::string &role, const std::string &content);
    void addEnvVar(const std::string &key, const std::string &value);
    void importEnvFile(const std::string &filePath);
    void addPrompt(const std::string &prompt);
    // --- MADE PUBLIC ---
    void addTask(const std::string &task);
    void addInitialCommand(const std::string &command); // MADE PUBLIC - Assuming needed


    // --- Utilities ---
    std::string generateStamp(void); // Should this be static or member? Assuming member.
    void addToHistory(const std::string &role, const std::string &content);
    std::string wrapText(const std::string &text);
    std::string wrapXmlLine(const std::string &content, const std::string &tag);
    std::string wrapXmlBlock(const std::string &content, const std::string &tag);
    void addBlockToSystemPrompt(const std::string &content, const std::string &tag);


private:
    // --- Private Members ---
    MiniGemini &api;
    std::map<std::string, Tool *> tools;
    std::map<std::string, std::string> internalToolDescriptions;

    std::string systemPrompt;
    std::vector<std::pair<std::string, std::string>> history;
    int iteration = 0;
    int iterationCap = 10;
    bool skipFlowIteration = false;
    std::vector<std::pair<std::string, std::string>> env;
    fileList files;
    std::string name;
    std::string description;
    std::vector<std::string> extraPrompts;

    std::vector<std::pair<std::string, Agent *>> subAgents;

    StrkeyValuePair scratchpad;
    StrkeyValuePair shortTermMemory;
    StrkeyValuePair longTermMemory;

    std::vector<std::string> tasks; // Made private data member
    std::vector<std::string> initialCommands; // Made private data member

    // --- Private DIRECTIVE member ---
    DIRECTIVE directive; // The member variable remains private

    // --- Private Methods ---
    std::string buildFullPrompt() const;
    std::vector<ToolCallInfo> extractToolCalls(const std::string &response);
    std::string processToolCalls(const std::vector<ToolCallInfo> &toolCalls);
    std::string handleToolExecution(const ToolCallInfo &call);
    std::string executeApiCall(const std::string &fullPrompt);
    void setSkipFlowIteration(bool skip);

    bool parseStructuredLLMResponse(const std::string& jsonString, std::string& thought, std::vector<ToolCallInfo>& toolCalls, std::string& finalResponse);

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
    std::string executeTask(const std::string &task, const Json::Value &format);
    std::string executeTask(const std::string &task);
    std::string executeTask(const std::string &task, const std::string &format);
    std::string auditResponse(const std::string &response);

    // --- Getters/Setters for Private Members (Remove public versions if they were just examples) ---
    // The public versions added above handle access now.
    // DIRECTIVE &getDirective(); // Keep private if needed internally
    // void setDirective(const DIRECTIVE &dir); // Keep private if needed internally
    // void addTask(const std::string &task); // Keep private if needed internally
    // void addInitialCommand(const std::string &command); // Keep private if needed internally
};

// --- Implementation for new/modified getters (in Agent.cpp) ---
/*
inline const std::string& Agent::getSystemPrompt() const { return systemPrompt; }
inline int Agent::getIterationCap() const { return iterationCap; }
inline const std::vector<std::pair<std::string, std::string>>& Agent::getEnv() const { return env; }
inline const std::vector<std::string>& Agent::getExtraPrompts() const { return extraPrompts; }
inline const std::vector<std::string>& Agent::getTasks() const { return tasks; }
inline const Agent::DIRECTIVE& Agent::getDirective() const { return directive; }
*/
