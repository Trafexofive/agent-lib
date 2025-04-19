/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Agent.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlamkadm <mlamkadm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/05 15:09:32 by mlamkadm          #+#    #+#             */
/*   Updated: 2025/04/12 18:07:46 by mlamkadm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "File.hpp"
#include "MiniGemini.hpp" // Include MiniGemini header
#include "Tool.hpp"       // Include Tool header
#include <json/json.h>    // For Json::Value
#include <json/value.h>
#include <map>
#include <string>
#include <vector>
// main.cpp includes
#include <cstdlib>
#include <ctime>
#include <curl/curl.h>
#include <iostream>
#include <json/json.h>
#include <sstream> // Required for rollDice
#include <stdexcept>
#include <string>
// remove_if
#include "Tool.hpp"
#include <algorithm> // For std::remove_if
#include <fstream>   // For file operations

#include <algorithm> // For std::remove, std::find
#include <cstddef>   // For size_t
#include <fstream>   // For std::ifstream, std::ofstream
#include <iostream>  // For std::ostream
#include <sstream>   // For std::ostringstream
#include <stdexcept> // For std::runtime_error
#include <string>
#include <vector>

typedef std::vector<File> fileList;
typedef std::vector<std::pair<std::string, std::string>> StrkeyValuePair;
class Tool;        // Forward declaration of Tool class
class Agent;       // Forward declaration of Agent class
class BuiltInTool; // Forward declaration of BuiltInTool class
struct ToolParams {
  const Json::Value &params; // Parameters for the tool
  Agent &agent;              // Reference to the agent
};

// Basic Logger (can be expanded into a class later if needed)
enum class LogLevel {
  DEBUG,
  INFO,
  WARN,
  ERROR,
  TOOL_CALL,
  TOOL_RESULT,
  PROMPT
};

class Agent {
public:
  // Structure to hold parsed tool call information (Unified JSON)
  struct ToolCallInfo {
    std::string toolName;
    Json::Value params; // Holds parameters for all tools
  };

  Agent(MiniGemini &api); // Takes API client by reference

  void setSystemPrompt(const std::string &prompt);
  void addTool(Tool *tool); // Manages Tool pointers (caller owns the Tool)
  // Removed addTextTool method
  void reset();
  std::string prompt(const std::string &userInput);

  void run(); // Starts the interactive loop

  void addAgent(Agent *agent) {
    agents.push_back(std::make_pair(agent->getName(), agent));
  } // Adds an agent to the list
  std::string agentInstance(const std::string &name) {
    auto it = std::find_if(agents.begin(), agents.end(),
                           [&name](const std::pair<std::string, Agent *> &p) {
                             return p.first == name;
                           });
    if (it != agents.end()) {
      return it->second->getName();
    }
    return "Agent name not found.";
  } // Returns the agent instance by name
    // Potentially add methods to manage iteration cap, skip flag etc. if needed
    // externally
    // =========================================================
    // ================================[ Basic Memory Management

  std::string manualToolCall(const std::string &toolName,
                             const Json::Value &params) {
    ToolCallInfo call;
    call.toolName = toolName;
    call.params = params;
    return handleToolExecution(call);
  } // Manually calls a tool with the given parameters
  const std::string getName() const {
    return this->_name;
  } // Returns the name of the agent
  void addMemory(const std::string &role, const std::string &content) {
    this->LongTermMemory.push_back(std::make_pair(role, content));
  }
  void removeMemory(const std::string &role, const std::string &content) {
    auto it = std::remove_if(
        this->LongTermMemory.begin(), this->LongTermMemory.end(),
        [&role, &content](const std::pair<std::string, std::string> &p) {
          return p.first == role && p.second == content;
        });
    this->LongTermMemory.erase(it, this->LongTermMemory.end());
  }

  MiniGemini &getApi() { return m_api; } // Returns the API client reference

  fileList getFiles() { return _files; } // Returns the list of files
  void setName(const std::string &name) { _name = name; }\

  void addPrompt(const std::string &prompt) { extraPrompts.push_back(prompt); }
  void addEnvVar(const std::string &key, const std::string &value) {
    this->_env.push_back(std::make_pair(key, value));
  } // Adds to the environment variables

  void importEnvFile(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
      throw std::runtime_error("Could not open file: " + filePath);
    }
    std::string line;
    while (std::getline(file, line)) {
      size_t pos = line.find('=');
      if (pos != std::string::npos) {
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        addEnvVar(key, value);
      }
    }
    file.close();
  } // Imports environment variables from a file

  void setDescription(const std::string &description) {
    _description = description;
  } // Sets the description
  const std::string getDescription() const {
    return _description;
  } // Returns the description
  void loadFromYaml(const std::string &filePath);
  void saveToYaml(const std::string &filePath);

  std::string generateStamp(void) {
    std::time_t now = std::time(nullptr);
    std::tm *tm = std::localtime(&now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);

    std::string result = "[" + std::string(buffer) + "]";

    return result;
  } // Generates a timestamp

  void addToHistory(const std::string &role, const std::string &content) {
    std::string prefixStamps = generateStamp();
    std::string result = prefixStamps + " :
" + content;
    m_history.push_back(std::make_pair(role, result));
  }

  std::string wrapText(const std::string &text) {
    std::string wrappedText = "```" + text + "```";
    return wrappedText;
  } // Wraps text in code block
  std::string wrapXmlLine(const std::string &content, const std::string &tag) {
    std::string wrappedText = "<" + tag + ">" + content + "</" + tag + ">
";
    return wrappedText;
  } // Wraps text in XML tags
    //
  std::string wrapXmlBlock(const std::string &content, const std::string &tag) {
    std::string wrappedText =
        "<" + tag + ">
	" + content + "
</" + tag + ">
";
    return wrappedText;
  } // Wraps text in XML tags

  void removeTool(const std::string &toolName);\
  Tool *getTool(const std::string &toolName) const;
  // --- Built-in Tools ---\
  void addBuiltInTool(Tool *tool);
  Tool *getBuiltInTool(const std::string &toolName) const;

  void addBlockToSystemPrompt(const std::string &content,
                              const std::string &tag) {
    std::string wrappedText = wrapXmlBlock(content, tag);
    m_systemPrompt += wrappedText;
  } // Adds a block to the system prompt

private:
  // --- Core Components ---\
  MiniGemini &m_api;                     // Reference to the API client\
  std::map<std::string, Tool *> m_tools; // Map of available external tools\
  std::map<std::string, std::string>\
      m_internalToolDescriptions; // Descriptions for internal tools like 'help'

  // --- State ---\
  std::string m_systemPrompt;
  std::vector<std::pair<std::string, std::string>>\
      m_history; // Conversation history (role, content)
  std::vector<std::pair<std::string, std::string>>\
      conversationHistory; // Persistent conversation history
  int iteration;
  int iterationCap;
  bool skipFlowIteration; // Flag to skip the final LLM call after tool execution
  std::vector<std::pair<std::string, std::string>>\
      _env;          // Store results of tool calls for later use\
  fileList _files;   // Store files for later use\
  std::string _name; // Store name for later use\
  std::vector<std::string> extraPrompts;

  std::vector<std::pair<std::string, Agent *>>\
      agents; // Orchestrator mode/cooperation mode\
  StrkeyValuePair Scratchpad;
  StrkeyValuePair ShortTermMemory;
  StrkeyValuePair LongTermMemory;

  std::map<std::string, Tool *> m_builtInTools; // Map of built-in tools
  // Removed m_textTools map

  std::vector<std::string> tasks;
  void addTask(const std::string &task) { tasks.push_back(task); }\

  std::vector<std::string> initialCommands;
  void addInitialCommand(const std::string &command) {\
    initialCommands.push_back(command);
  } // Adds to the initial commands\

  struct DIRECTIVE { // Referring to the conversation flow or the underlying intonation/stance
    enum Type { BRAINSTORMING, AUTONOMOUS, NORMAL, EXECUTE, REPORT };
    Type type;
    std::string description;
    std::string format;
  };\

  DIRECTIVE directive;
  DIRECTIVE &getDirective() { return directive; } // Returns the directive\
  void setDirective(const DIRECTIVE &dir) {\
    directive = dir;
  } // Sets the directive\
  std::string _description;

  // --- Helper Methods ---\
  std::string buildFullPrompt() const;
  std::vector<ToolCallInfo> extractToolCalls(const std::string &response);
  std::string processToolCalls(const std::vector<ToolCallInfo> &toolCalls);
  std::string handleToolExecution(const ToolCallInfo &call);
  std::string executeApiCall(const std::string &fullPrompt);
  std::string\
  getHelp(const Json::Value &params); // Internal help tool implementation

  void setSkipFlowIteration(bool skip) {\
    this->skipFlowIteration = skip;
  } // Set the skip flag\

  // --- Built-in tool implementations (declarations) ---\
  std::string skip(const Json::Value &params);
  std::string promptAgentTool(const Json::Value &params);
  std::string summarizeHistory(const Json::Value &params);
  std::string getWeather(const Json::Value &params);
  std::string summerizeTool(const Json::Value &params); // Keep if used internally
  std::string reportTool(const Json::Value &params); // Keep if used internally
  std::string generateCode(const Json::Value &params); // Keep if used internally
  std::string generateTool(const Json::Value &params); // Keep if used internally
  std::string auditResponse(const std::string &response); // Keep if used internally
  std::string executeTask(const std::string &task, const Json::Value &format); // Keep if used internally
  std::string executeTask(const std::string &task, const std::string &format); // Keep if used internally
  std::string executeTask(const std::string &task); // Keep if used internally

  // --- Helper promptAgent (used by promptAgentTool) ---\
  std::string promptAgent(Agent &agentToCall, const std::string &userInput);
  std::string promptAgent(const std::string &name,\
                          const std::string &userInput);
};
