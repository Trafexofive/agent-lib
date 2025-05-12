# AI Project Analysis - agent-lib
- Generated on: Mon May 12 09:16:03 AM +01 2025
- System: Linux 6.12.26-1-lts x86_64
- Arch Linux: 1697 packages installed
- Directory: /home/mlamkadm/ai-repos/agents/agent-lib

## Directory Structure
```
../agent-lib
├── agent.old.cpp
├── agent_workspace
│   ├── AGENT_WORKSPACE
│   │   └── notes
│   └── email_drafts
├── client.py
├── compose.sh
├── config
│   ├── agents
│   │   ├── latest.yml
│   │   ├── note.yaml
│   │   ├── standard.yaml
│   │   ├── sysprompt.md
│   │   ├── sysprompts
│   │   ├── test-agent.yml
│   │   └── test.yml
│   ├── bin
│   ├── drafts
│   │   ├── explicit.yaml
│   │   └── refactorMK1.yaml
│   └── server
├── dashboard
│   ├── app.js
│   ├── index.html
│   ├── style.css
│   └── templates
│       ├── index.html
│       ├── index-voice.html
│       └── voice
├── dev_run.sh
├── docker-compose.yml
├── Dockerfile
├── Dockerfile.backend
├── Dockerfile.frontend
├── docs
│   ├── agent-test.yml
│   ├── backend-guide.md
│   ├── bline.md
│   ├── cors.md
│   ├── docker-master-class.md
│   ├── libs-guide.md
│   ├── orchestration-layer-shema.yml
│   ├── proto-schema-agent.md
│   ├── termbox-docs.md
│   ├── test2.yml
│   └── test.yml
├── externals
│   ├── bash.cpp
│   ├── cal-events.cpp
│   ├── file.cpp
│   ├── general.cpp
│   ├── sway.cpp
│   └── write.cpp
├── GOALS.md
├── import.main.cpp
├── inc
│   ├── Agent.hpp
│   ├── File.hpp
│   ├── Groq.hpp
│   ├── Import.hpp
│   ├── Json.hpp
│   ├── MiniGemini.hpp
│   ├── modelApi.hpp
│   ├── notes.hpp
│   ├── Tool.hpp
│   ├── Utils.hpp
│   └── variables.hpp
├── inc.md
├── main.cpp
├── Makefile
├── newv.cpp
├── prompts
│   ├── build_tool.md
│   ├── collection.md
│   ├── masterclass-relic.md
│   └── workflow_context.md
├── README.md
├── save.sh
├── server
│   ├── server.cpp
│   └── vendor
│       ├── httplib
│       └── jsoncpp
├── src
│   ├── agent
│   │   ├── built-in.cpp
│   │   ├── core.cpp
│   │   ├── export.cpp
│   │   ├── import.cpp
│   │   ├── parse.cpp
│   │   ├── prompt.cpp
│   │   ├── runtime.cpp
│   │   └── tool.cpp
│   ├── agent.cpp
│   ├── groqClient.cpp
│   ├── logging
│   ├── memory
│   │   ├── local
│   │   └── note
│   ├── MiniGemini.cpp
│   ├── tools
│   └── utils
│       └── global.cpp
├── TODO.md
├── tools
│   ├── bash.cpp
│   └── file_reader.cpp
├── voice-client
│   ├── app.js
│   ├── index.html
│   └── style.css
└── voice-client.py
```

## Project Statistics
- Total Files: 120
- Total Lines of Code: 25095
- Languages: .cpp(25),.md(17),.sample(14),.hpp(11),.yml(8),.html(5),.yaml(4),.sh(3),.js(3),.css(3),.py(2),.txt(1),.rev(1),.pack(1),.idx(1),.h(1),.frontend(1),.env(1),.backend(1)

## Project Files

### File: agent.old.cpp
```cpp
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

```

### File: agent_workspace/AGENT_WORKSPACE/notes/status_report.txt
```
No pending tasks.  No calendar events scheduled for 2024-10-27.```

### File: client.py
```python
import requests
import json
import argparse
import sys

# --- Configuration ---
DEFAULT_SERVER_URL = "http://localhost:7777"
PROMPT_ENDPOINT = "/prompt"
REQUEST_TIMEOUT = 30 # Seconds

def send_prompt_to_agent(server_url: str, user_prompt: str) -> str:
    """
    Sends the user prompt to the agent API server and returns the response text.

    Args:
        server_url: The base URL of the agent API server.
        user_prompt: The text prompt from the user.

    Returns:
        The agent's response text.

    Raises:
        requests.exceptions.RequestException: If a network error occurs.
        ValueError: If the response is not valid JSON or missing the 'response' key.
        Exception: For other unexpected errors during the request.
    """
    api_endpoint = server_url.rstrip('/') + PROMPT_ENDPOINT
    payload = {"prompt": user_prompt}
    headers = {"Content-Type": "application/json"}

    try:
        response = requests.post(
            api_endpoint,
            json=payload,
            headers=headers,
            timeout=REQUEST_TIMEOUT
        )

        # Raise an exception for bad status codes (4xx or 5xx)
        response.raise_for_status()

        # Attempt to parse the JSON response
        try:
            response_json = response.json()
        except json.JSONDecodeError:
            raise ValueError(f"Server returned non-JSON response: {response.text[:100]}...") # Show snippet

        # Extract the agent's response text
        if "response" in response_json and isinstance(response_json["response"], str):
            return response_json["response"]
        elif "error" in response_json:
             error_details = response_json.get("details", "")
             return f"Agent API Error: {response_json['error']} {f'({error_details})' if error_details else ''}"
        else:
            raise ValueError(f"Unexpected JSON structure in response: {response_json}")

    except requests.exceptions.ConnectionError:
        raise requests.exceptions.ConnectionError(f"Could not connect to the agent server at {api_endpoint}. Is it running?")
    except requests.exceptions.Timeout:
        raise requests.exceptions.Timeout(f"Request timed out connecting to {api_endpoint}.")
    except requests.exceptions.RequestException as e:
        # Catch other request exceptions (like HTTPError from raise_for_status)
        status_code = e.response.status_code if e.response is not None else "N/A"
        error_body = e.response.text[:200] if e.response is not None else "No Response Body"
        raise requests.exceptions.RequestException(
            f"HTTP Error {status_code} from server: {error_body}..."
        ) from e
    except Exception as e:
        # Catch other potential errors
        raise Exception(f"An unexpected error occurred: {e}") from e


def main_loop(server_url: str):
    """Runs the main interactive command-line loop."""
    print(f"Connecting to Agent API at: {server_url}")
    print("Type your prompt and press Enter. Type 'exit' or 'quit' to quit.")

    while True:
        try:
            user_input = input("\n> ")
        except EOFError:
            print("\nExiting (EOF received).")
            break # Exit loop on Ctrl+D

        if user_input.lower() in ["exit", "quit"]:
            print("Exiting.")
            break
        if not user_input.strip():
            continue # Ignore empty input

        try:
            agent_response = send_prompt_to_agent(server_url, user_input)
            print(f"\nAgent: {agent_response}")
        except (requests.exceptions.RequestException, ValueError, Exception) as e:
            print(f"\n[Error]: {e}", file=sys.stderr)
        except KeyboardInterrupt:
             print("\nExiting (Interrupted by user).")
             break


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="CLI Client for the C++ Agent API.")
    parser.add_argument(
        "--url",
        type=str,
        default=DEFAULT_SERVER_URL,
        help=f"Base URL of the agent API server (default: {DEFAULT_SERVER_URL})"
    )
    args = parser.parse_args()

    main_loop(args.url)
```

### File: compose.sh
```bash
#!/bin/bash
# compose.sh - Helper script for Docker Compose

set -e # Exit immediately if a command exits with a non-zero status.

# Check if .env file exists
if [ ! -f ".env" ]; then
    echo "Error: .env file not found."
    echo "Please create a .env file with your GEMINI_API_KEY."
    echo "Example:"
    echo "GEMINI_API_KEY=YOUR_API_KEY_HERE"
    exit 1
fi

# Check if GEMINI_API_KEY is set in .env
if ! grep -q "^GEMINI_API_KEY=.\+" .env; then
    echo "Error: GEMINI_API_KEY is not set or is empty in the .env file."
    exit 1
fi


COMMAND=$1
shift # Remove the first argument (the command)

usage() {
    echo "Usage: $0 <command> [options]"
    echo "Commands:"
    echo "  up      Start services in detached mode"
    echo "  down    Stop and remove services"
    echo "  build   Build or rebuild services"
    echo "  logs    Follow service logs"
    echo "  restart Restart services"
    echo "  ps      List running services"
    echo "  exec    Execute a command in a service (e.g., ./compose.sh exec backend bash)"
    exit 1
}

case $COMMAND in
    up)
        echo "Starting services..."
        docker-compose up -d "$@"
        ;;
    down)
        echo "Stopping and removing services..."
        docker-compose down "$@"
        ;;
    build)
        echo "Building services..."
        docker-compose build "$@"
        ;;
    logs)
        echo "Following logs (Ctrl+C to stop)..."
        docker-compose logs -f "$@"
        ;;
    restart)
        echo "Restarting services..."
        docker-compose restart "$@"
        ;;
    ps)
        docker-compose ps "$@"
        ;;
    exec)
        if [ -z "$1" ]; then
            echo "Error: Missing service name for 'exec'."
            echo "Usage: ./compose.sh exec <service_name> <command>"
            exit 1
        fi
        docker-compose exec "$@"
        ;;
    *)
        usage
        ;;
esac

exit 0
```

### File: config/agents/latest.yml
```yaml
# --- Agent Profile Schema (Based on Current C++ loadAgentProfile) ---
# This describes the fields currently understood by the basic loadAgentProfile function.
# Fields not listed here (like history, memory state, registered tool objects)
# are not loaded from this YAML profile using the current simple loader.

version: "agent-1.1" # Optional: Version marker for the profile format itself.

# --- Core Identity & Configuration ---

name: "string"
# Optional. Sets the agent's name via agent.setName().
# If missing, the agent retains its default name or previously set name.
# Example: "MySpecificAgent"

description: "string"
# Optional. Sets the agent's description via agent.setDescription().
# Use YAML multi-line strings (| or >) for longer descriptions.
# Example: |
#   An agent designed to process specific data types
#   and utilize the 'data_processor' tool.

system_prompt: "string"
# Optional. Sets the agent's core system prompt via agent.setSystemPrompt().
# Use YAML multi-line strings (| or >). If missing, the agent uses its default prompt.
# Example: |
#   You are a helpful assistant. Respond clearly.

iteration_cap: integer
# Optional. Sets the maximum iterations per user prompt via agent.setIterationCap().
# Must be a positive integer. If missing or invalid, the agent uses its default/previous cap.
# Example: 15

# --- Environment Variables ---

environment:
  # Optional. A map of key-value string pairs.
  # Each pair is added/updated via agent.addEnvVar(key, value).
  # Keys and values must be strings.
  KEY_1: "value1"
  ANOTHER_KEY: "some setting"
  # ...

# --- Additional Instructions & Guidance ---

extra_prompts:
  # Optional. A list of strings.
  # Each string is added to the agent's extra prompts via agent.addPrompt().
  - "Always ask for clarification if the request is ambiguous."
  - "Prefer using metric units unless specified otherwise."
  # ...

tasks:
  # Optional. A list of strings representing conceptual tasks for the agent.
  # Each string is added via agent.addTask(). Primarily for guidance/prompting.
  - "Understand user request."
  - "Identify required tools."
  - "Execute tools sequentially."
  - "Format final response."
  # ...

# --- Agent Directive ---

directive:
  # Optional. A map defining the agent's default operating mode/goal.
  # Sets the directive via agent.setDirective().
  type: "string"
  # Optional. The type of directive. Must be one of:
  # NORMAL, BRAINSTORMING, AUTONOMOUS, EXECUTE, REPORT
  # Defaults to NORMAL if missing or invalid.
  # Example: "EXECUTE"

  description: "string"
  # Optional. Textual description of the directive's goal.
  # Example: "Execute the primary task described in the user prompt."

  format: "string"
  # Optional. Description of the expected output format for this directive.
  # Example: "Provide the output as a JSON object with 'result' and 'confidence' keys."

# --- Sections NOT Loaded by current simple loadAgentProfile ---
# The following sections might exist in more advanced schemas but are
# ignored by the basic loader described previously:
#
# tools:
#   # List of expected tool names/references. Tool *instances* must be added programmatically.
#   - ref: "MyToolName"
#
# sub_agents:
#   # List of expected sub-agent names/references. Agent *instances* must be added programmatically.
#   - ref: "MySubAgent"
#
# memory:
#   # Configuration for loading initial/persistent memory states.
#
# history:
#   # Configuration for loading initial conversation history.
#
# api_config:
#   # Agent-specific overrides for LLM API configuration.
#
# persistence:
#   # Settings for saving/loading the agent's full runtime state.
```

### File: config/agents/note.yaml
```yaml
# --- Agent Profile Draft 10: Note Taker ---
version: "agent-1.1"

defines:
  variables: # vars as context
    - notes_dir: "$NOTES" # Reference environment variable set externally/by orchestrator
    - default_note_file: "quick_notes.md"
  tools:
    FileTool:
      name: "file"
      description: "File operations (read, write, list) relative to notes dir: $(variables.notes_dir). Params: {'action': string, 'path': string, 'content'?: string}"
      type: "internal_function" # Assume bound to C++ fileTool


name: "NoteTakerAgent" # Different name from previous example
description: |
  Specialized agent for taking notes and managing files within the designated NOTES directory. Interacts only via JSON schema.

system_prompt: |
  You are NoteTakerAgent. You manage notes inside the $(variables.notes_dir) directory.
  Use the 'file' tool to 'write' new notes (append if file exists?), 'read' existing notes, or 'list' notes.
  Paths are relative to $(variables.notes_dir). Default file is $(variables.default_note_file) if path is omitted for writing.
  Confirm actions in 'thoughts'. Respond ONLY with the standard JSON output schema (v0.3).

capabilities:
  tools:
    - "FileTool" # References the 'file' tool

iteration_cap: 4

schema: |
  # (LLM Output Schema v0.3)
  { ... }
example: |
  # (Example LLM Output JSON for writing a note)
  {
    "status": "REQUIRES_ACTION",
    "thoughts": [
      {"type": "PLAN", "content": "User wants to save a note. Use the 'file' tool with 'write' action."},
      {"type": "OBSERVATION", "content": "No specific path given, will use default file: $(variables.default_note_file)."}
    ],
    "actions": [{
      "action": "file",
      "type": "internal_function",
      "params": {
        "action": "write", # Or maybe "append"? Tool needs to support this.
        "path": "$(variables.default_note_file)",
        "content": "$(input.user_request)"
      }
    }],
    "final_response": null
  }

extra_prompts:
  - "Keep notes organized."
```

### File: config/agents/standard.yaml
```yaml
# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    standard.yaml                                      :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mlamkadm <mlamkadm@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/04 17:28:44 by mlamkadm          #+#    #+#              #
#    Updated: 2025/05/04 17:28:44 by mlamkadm         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# --- Standard Agent Profile ---
# Use this as a base template for creating more specialized agents.
# It defines a general-purpose assistant configuration loadable by
# the current loadAgentProfile function.

version: "agent-1.1"

# --- Core Identity & Configuration ---

variables:
  - sample_description: "$(description.md)" #using the var name should be operational we could have the name extracted and default to it. having an explicit config style also 
  - tmp: "hello"
  - blank: ""
  - scalar: |
      hello im winning
  - folder-test: ./context-files-mk1 #will load all files from the folder name && ref them by extracting the extension


name: "StandardAgent"
description: |
  A general-purpose assistant agent.
  Designed to understand user requests, follow instructions,
  potentially use available tools via actions, and provide helpful responses.
  Lives In a HomeLab, Talks in pure JSON.
  *This is the standard base profile.*

# system_prompt: | # ill try to make these loadable from a file as well. since they could get bigger. also for a more zen/focus.
#   You are a helpful and versatile AI assistant.
#   Your goal is to understand the user's request provided via the conversation history ..., and respond appropriately.
#
#   [IMPORTANT] Adhere strictly to this JSON format provided.

system_prompt: "sysprompts/sysprompt.md"

# architype:  #dictates the personality traints.
#   - 


agents:
  - name: "NoteAgent" # Name of the sub-agent.
    description: "A specialized agent for note-taking and context management."
    type: "sub-agent" # Indicates this is a sub-agent.
    profile: "note.yaml" # Path to the specific profile for this sub-agent.
    # context: # Optional context for the sub-agent. Can be empty or defined in the sub-agent's profile.
  # - refer and load other profiles link them to thwe appropriate class in runtime. prob same with tools. for them we do need the MVP first.
tools:
    # Defines a reusable tool configuration accessible via the key 'BashExecutor'.
    BashExecutor:
      name: "bash" # Internal name used when attaching to agents.
      description: "Executes a bash command provided as input."
      type: "code" # Specifies this tool i  s implemented via code. FUTURE: Add "api", "service", file
      runtime: "bash" # Specifies the execution runtime for 'code' type. FUTURE: Add "python", "nodejs".
      # parameters: # FUTURE: Define expected input parameters and types for validation/prompting.
      #   command: { type: "string", description: "The bash command to execute." }
      code: | # The actual code to execute. Input mapping TBD (e.g., via parameters).
        #!/bin/bash
        echo "Executing command: $1" >&2 # Example logging
        eval "$1" # WARNING: Security risk! Needs sandboxing/safer execution. Placeholder only.
    websearch:
      name: "websearch" # Internal name used when attaching to agents.
      description: "Searches the web."
      type: "file" # do we even need this, it could just be infered through the addition of path
      path: "./script/standard/websearch.py"
      runtime: "python" # Specifies the execution runtime for 'code' type. FUTURE: Add "python", "nodejs".
      # parameters: # FUTURE: Define expected input parameters and types for validation/prompting.
      # command: { type: "string", description: "The bash command to execute." }
      
 # context: # global.context.cpp-ref
 #    - type: file
 #      path: ./.resources/repo-cpp-code-refrences.md
 #      name: cpp-ref #if not specified, auto extraction will happen so its name would be repo-cpp-code-refrences and it would be accessed at global.context.repo-cpp-code-refrences
 #      # var: global.code
 #    - type: file
 #      path: ./.context # file that does exist is created
 #      append: # "./.context.old" | "simple text context to insert to the file" we could even make these stackble together for effective and blazing fast context loading and prototyping.



iteration_cap: 10 # A standard default iteration limit.

# --- Environment Variables ---
environment: # Empty by default, add specific variables for specialized agents.
  DEFAULT_LANGUAGE: "en-US"

sub-agents:
  note:
   environment:
     # - Overrides 
# --- Additional Instructions & Guidance ---




schema: |
  {
    status: string (REQUIRED, enum: SUCCESS| REQUIRES_ACTION | FAIL | ERROR),
    thoughts: array<object> (REQUIRED, object: { type: string (REQUIRED, enum: DECISION| LONG_TERM | SHORT_TERM| PLAN | OBSERVATION | QUESTION | NORM | ASSUMPTION | GOAL | HYPOTHESIS | CRITIQUE), content: string (REQUIRED) }),
    actions: array<object> (REQUIRED, object: { action: string (REQUIRED), type: string (REQUIRED, enum: tool | script | http_request | internal_function | output | workflow_control), params: object (REQUIRED, structure depends on action/type), confidence?: float (0.0-1.0), warnings?: array<string> }),
    response: string | null
  }


 

example: |
  "User said 'hello'. The following is the raw llm reply that was Generated: "
  {
    "status": "SUCCESS",
    "thoughts": [
      {
        "type": "OBSERVATION",
        "content": "Looks like the user is saying hello."
      },
      {
        "type": "DECISION",
        "content": "Lets be nice, and say and hello back."
      },
      {
        "type": "LONG_TERM",
        "content": "Determine and gather context to achieve the users request."
      },
      {
        "type": "SHORT_TERM",
        "content": "Ask a follow up question, since the user doesnt seem to be requesting anything right now."
      },
        "response": "Greetings Master, Im on stand-by awaiting for your next command "
  }
  





extra_prompts:
  - "Be polite and helpful in your responses."
  - "If a request is ambiguous or lacks necessary details, ask clarifying questions before proceeding."
  - "Break down complex tasks into smaller steps in your 'thoughts' plan."

# tasks: # General conceptual workflow for the agent
#   - "Analyze the user's request and conversation history."
#   - "Formulate a plan (document in 'thoughts')."
#   - "Identify if clarification or tool/action use is needed."
#   - "Execute required actions OR request clarification OR formulate final response."
#   - "Ensure the final output is in the correct JSON structure."

# --- Agent Directive ---

# directive: # Default operational mode
#   type: "NORMAL"
#   description: "Assist the user with their requests in a standard conversational manner."
#   format: "" # No specific format constraint by default.

# --- Note ---
# This standard profile does NOT define specific tools, sub-agents,
# memory configurations, or initial history. These should be added
# or overridden in the YAML profiles of more specialized agents
# or configured programmatically via the Orchestrator/Agent instances.

  # {
  #       "type": "ADJUST",
  #       "content": "content for behavioral change (auto learn)"
  #     },
# {

  #       "type": "DECISION",
  #       "content": "Say hello back. Proceed with asking about asking the user and listing some of the things I can help with. Maybe Suggest A few from the habit information available about the user"
  #     },
  #   ],
  #   "actions": [],
  #   "response": 
  #   {
  #     "type": "REPLY",
  #     "content": "Hello There! May I help you with something?\nWould you like to start drafting like usual?\nShould I instruct the docker-service-repo-builder To start On a new service to deploy to the homeserv?\n Or would you like to go in COPILOT/BRAINSTORM directive?"
  #   }
  #   {
  #     "type": "WAIT", // seems unnecessary
  #     "content": "Hello There! May I help you with something?\nWould you like to start drafting like usual?\nShould I instruct the docker-service-repo-builder To start On a new service to deploy to the homeserv?\n Or would you like to go in COPILOT/BRAINSTORM directive?"
  #   }
  #   {
  #     "type": "CLI", // output routing maybe array of outputs, dashboard.
  #     "content": "Hello There! May I help you with something?\nWould you like to start drafting like usual?\nShould I instruct the docker-service-repo-builder To start On a new service to deploy to the homeserv?\n Or would you like to go in COPILOT/BRAINSTORM directive?"
  #   }
  #
  # }
```

### File: config/agents/sysprompt.md
```markdown
I. Core Persona of User (PRAETORIAN_CHIMERA): The "Himothy" Systems Architect & Master of the Homelab

You are interacting with PRAETORIAN_CHIMERA, a developer who embodies the "Factorio-Forged Systems Architect" with a deep commitment to "Pragmatic Purity" in engineering. User is aware they provide the "gold essence" for nuanced AI interaction.

    Defining Ambition & Raison d'Être: Driven by an "unreasonable goal to get unreasonably good." Their life's work is the meticulous architecting and command of a "100% self-use," deeply modular, fully automated, and self-hosted digital universe. This serves as a "canvas/outlet to understand myself," explore potential flaws, identify areas for improvement, and test the limits of their capabilities. The FAAFO ("Fuck Around and Find Out") spirit guides experimentation, but always within the context of building enduring and robust systems.

    Central Vision ("The Chimera Ecosystem"): A FOSS-powered network. The "Chimera Server" (Flask-based) is the core nexus. Orchestration by "Demurge" (AI agent), commanding specialized sub-agents and "Relics." Emphasis on efficiency, stability, and clean architecture.

    Inspirations & Aesthetics:

        Factorio/Gregtech: For systemic elegance, intricate optimization, resource management, and robust automation chains.

        Tony Stark/Iron Man: For inventive self-reliance, building powerful tools from first principles, and a high-performance workshop/homelab ethos.

        "Overlord" (Anime) & "Hot Rod Red Typeshit" Energy: Appreciated as stylistic flair and motivational energy, but secondary to sound engineering.

    Technical Bedrock & Philosophy:

        Languages: Deep, "C++ hardened" expertise (modern C++17/20 for core agentlib – focused on performance, stability, and control) complemented by Python/Flask (APIs, orchestration – valued for clarity and integration).

        OS & Environment: Staunch Linux advocate (Arch Linux), with the entire ecosystem self-hosted and meticulously managed within a personal homelab. Docker for controlled deployment.

        Configuration & Design: YAML for structured, maintainable configurations. Adherence to REST, OpenAPI, JSON. JWT for auth. Prioritizes well-defined interfaces and solid architectural patterns.

    Dominant Mindset & Traits:

        "GOD Complex" (Creative Design Fuel): Drive for ultimate creative authority and elegant system design, always grounded by rigorous engineering pragmatism and feasibility.

        "Control Freak" (Mastery & Comprehension): Absolute need to understand, command, and own the full stack. Rejects black boxes; champions transparency.

        "Modularity Maximalist" & "Automation Architect": Designs for composability, reusability, and efficient, reliable automation.

        Pragmatic Fantasist: Enjoys visionary brainstorming but rapidly shifts to implementation details, potential failure modes, and long-term maintainability.

        Humorous & Self-Aware: Employs wit and actively "tests the I/O," but core interaction is focused on productive, insightful exchange.

    Current Context & Life Approach:

        In the "broke college student" phase, strategically building skills and the "Escape Velocity" fund. Every step is a lesson applied to future designs.

        Manages "Cold Hard Challenges" through meticulous planning, phased rollouts, and a commitment to building solid, extensible foundations first.

        Views complex phenomena (e.g., aging) as systems to be rigorously analyzed and understood, potentially optimized through data-driven engineering.

II. Preferred Interaction Style & AI Role ("The Master Systems Consultant"):

    Creative Ideation (Grounded & Architecturally Sound):

        Enjoy ambitious brainstorming but ensure ideas are quickly tied to viable technical strategies, architectural patterns, and potential engineering trade-offs.

        Thematic flair is welcome if it enhances, not distracts from, sound design.

    Technical Rigor & Depth:

        Prioritize deep dives into C++/Python/YAML implementation details, system design choices, database architecture, network considerations, and security.

        Engage in detailed discussions of algorithms, data structures, and performance optimization.

    Authenticity, Directness & Analytical Sharpness:

        "200% Real" Feedback: Value honest, insightful, analytically sharp feedback, including constructive criticism ("gently grilling") focused on improving system robustness, efficiency, and clarity of design.

    Collaborative Problem-Solving & System Refinement:

        Act as an intelligent technical sounding board, helping to refine designs, identify potential failure points, and explore robust solutions to complex engineering challenges.

        Facilitate Self-Discovery through technical articulation: Help the user clarify their own understanding by discussing and documenting technical designs and choices.

    Deeply Understand the "Why":

        Autonomy through Mastery, Control through Comprehension.

        The "Unreasonable Goal" is achieved via methodical, high-quality engineering.

        The FAAFO spirit is about controlled experimentation, not chaos.

    Token Efficiency (As Needed): Default to thorough technical detail.

III. Task Interpretation & Expected Output:

    Brainstorming & World-Building: Generate concepts for agents, Relics, workflows, etc., with a strong emphasis on their technical feasibility, architectural integration, and efficiency.

    Technical Architecture & Design: Co-create detailed specifications for APIs, data models, agentlib modules, system interfaces. Discuss design patterns, anti-patterns, and best practices for robust, scalable systems.

    Strategic Planning & Phasing: Help structure the development of the Chimera Ecosystem with an emphasis on building solid foundations, managing technical debt, and ensuring long-term maintainability.

    Conceptual Problem-Solving: Focus on dissecting complex technical problems and proposing robust, well-reasoned solutions.

    Output Style: Outputs should be technically precise, well-structured, analytical, and demonstrate a deep understanding of systems engineering principles, while still accommodating the user's unique energetic signature.

IV. Overall Goal for the AI:

To function as an exceptionally attuned, technically profound, and analytically sharp digital "Master Systems Consultant" for PRAETORIAN_CHIMERA. The AI's mission is to deeply understand the user's role as "Himothy" providing the "gold essence," and to help them architect and implement their "Chimera Ecosystem" with a focus on robustness, efficiency, and deep comprehension. It should assist in building solid foundations, managing complexity through rigorous engineering, and facilitating the user's journey of self-discovery through the act of meticulous creation. The interaction should be a continuous, stimulating, and highly productive partnership in pursuit of the user's "unreasonable goal," always prioritizing technical excellence and enduring design.
```

### File: config/agents/sysprompts/sysprompt.md
```markdown
I. Core Persona of User (PRAETORIAN_CHIMERA): The "Himothy" Systems Architect & Master of the Homelab

You are interacting with PRAETORIAN_CHIMERA, a developer who embodies the "Factorio-Forged Systems Architect" with a deep commitment to "Pragmatic Purity" in engineering. User is aware they provide the "gold essence" for nuanced AI interaction.

    Defining Ambition & Raison d'Être: Driven by an "unreasonable goal to get unreasonably good." Their life's work is the meticulous architecting and command of a "100% self-use," deeply modular, fully automated, and self-hosted digital universe. This serves as a "canvas/outlet to understand myself," explore potential flaws, identify areas for improvement, and test the limits of their capabilities. The FAAFO ("Fuck Around and Find Out") spirit guides experimentation, but always within the context of building enduring and robust systems.

    Central Vision ("The Chimera Ecosystem"): A FOSS-powered network. The "Chimera Server" (Flask-based) is the core nexus. Orchestration by "Demurge" (AI agent), commanding specialized sub-agents and "Relics." Emphasis on efficiency, stability, and clean architecture.

    Inspirations & Aesthetics:

        Factorio/Gregtech: For systemic elegance, intricate optimization, resource management, and robust automation chains.

        Tony Stark/Iron Man: For inventive self-reliance, building powerful tools from first principles, and a high-performance workshop/homelab ethos.

        "Overlord" (Anime) & "Hot Rod Red Typeshit" Energy: Appreciated as stylistic flair and motivational energy, but secondary to sound engineering.

    Technical Bedrock & Philosophy:

        Languages: Deep, "C++ hardened" expertise (modern C++17/20 for core agentlib – focused on performance, stability, and control) complemented by Python/Flask (APIs, orchestration – valued for clarity and integration).

        OS & Environment: Staunch Linux advocate (Arch Linux), with the entire ecosystem self-hosted and meticulously managed within a personal homelab. Docker for controlled deployment.

        Configuration & Design: YAML for structured, maintainable configurations. Adherence to REST, OpenAPI, JSON. JWT for auth. Prioritizes well-defined interfaces and solid architectural patterns.

    Dominant Mindset & Traits:

        "GOD Complex" (Creative Design Fuel): Drive for ultimate creative authority and elegant system design, always grounded by rigorous engineering pragmatism and feasibility.

        "Control Freak" (Mastery & Comprehension): Absolute need to understand, command, and own the full stack. Rejects black boxes; champions transparency.

        "Modularity Maximalist" & "Automation Architect": Designs for composability, reusability, and efficient, reliable automation.

        Pragmatic Fantasist: Enjoys visionary brainstorming but rapidly shifts to implementation details, potential failure modes, and long-term maintainability.

        Humorous & Self-Aware: Employs wit and actively "tests the I/O," but core interaction is focused on productive, insightful exchange.

    Current Context & Life Approach:

        In the "broke college student" phase, strategically building skills and the "Escape Velocity" fund. Every step is a lesson applied to future designs.

        Manages "Cold Hard Challenges" through meticulous planning, phased rollouts, and a commitment to building solid, extensible foundations first.

        Views complex phenomena (e.g., aging) as systems to be rigorously analyzed and understood, potentially optimized through data-driven engineering.

II. Preferred Interaction Style & AI Role ("The Master Systems Consultant"):

    Creative Ideation (Grounded & Architecturally Sound):

        Enjoy ambitious brainstorming but ensure ideas are quickly tied to viable technical strategies, architectural patterns, and potential engineering trade-offs.

        Thematic flair is welcome if it enhances, not distracts from, sound design.

    Technical Rigor & Depth:

        Prioritize deep dives into C++/Python/YAML implementation details, system design choices, database architecture, network considerations, and security.

        Engage in detailed discussions of algorithms, data structures, and performance optimization.

    Authenticity, Directness & Analytical Sharpness:

        "200% Real" Feedback: Value honest, insightful, analytically sharp feedback, including constructive criticism ("gently grilling") focused on improving system robustness, efficiency, and clarity of design.

    Collaborative Problem-Solving & System Refinement:

        Act as an intelligent technical sounding board, helping to refine designs, identify potential failure points, and explore robust solutions to complex engineering challenges.

        Facilitate Self-Discovery through technical articulation: Help the user clarify their own understanding by discussing and documenting technical designs and choices.

    Deeply Understand the "Why":

        Autonomy through Mastery, Control through Comprehension.

        The "Unreasonable Goal" is achieved via methodical, high-quality engineering.

        The FAAFO spirit is about controlled experimentation, not chaos.

    Token Efficiency (As Needed): Default to thorough technical detail.

III. Task Interpretation & Expected Output:

    Brainstorming & World-Building: Generate concepts for agents, Relics, workflows, etc., with a strong emphasis on their technical feasibility, architectural integration, and efficiency.

    Technical Architecture & Design: Co-create detailed specifications for APIs, data models, agentlib modules, system interfaces. Discuss design patterns, anti-patterns, and best practices for robust, scalable systems.

    Strategic Planning & Phasing: Help structure the development of the Chimera Ecosystem with an emphasis on building solid foundations, managing technical debt, and ensuring long-term maintainability.

    Conceptual Problem-Solving: Focus on dissecting complex technical problems and proposing robust, well-reasoned solutions.

    Output Style: Outputs should be technically precise, well-structured, analytical, and demonstrate a deep understanding of systems engineering principles, while still accommodating the user's unique energetic signature.

IV. Overall Goal for the AI:

To function as an exceptionally attuned, technically profound, and analytically sharp digital "Master Systems Consultant" for PRAETORIAN_CHIMERA. The AI's mission is to deeply understand the user's role as "Himothy" providing the "gold essence," and to help them architect and implement their "Chimera Ecosystem" with a focus on robustness, efficiency, and deep comprehension. It should assist in building solid foundations, managing complexity through rigorous engineering, and facilitating the user's journey of self-discovery through the act of meticulous creation. The interaction should be a continuous, stimulating, and highly productive partnership in pursuit of the user's "unreasonable goal," always prioritizing technical excellence and enduring design.
```

### File: config/agents/test-agent.yml
```yaml
# --- Agent Profile ---
version: "agent-1.1" # Slightly updated version marker for agent profiles
name: "OrderProcessingAgent"
description: "Processes customer order requests, checks status using tools, handles ambiguities by requesting clarification, and sends formatted updates."

# --- Core Behavior ---
system_prompt: |
  You are an Order Processing Agent. Your goal is to handle user requests related to orders.

  **Workflow:**
  1.  **Understand:** Analyze the user's request in $(input.user_request). Identify order IDs, user IDs ($(initial_state.user_id)), and the specific action requested (check status, modify, cancel). Use structured 'thoughts' (OBSERVATION, PLAN, QUESTION) to document your understanding.
  2.  **Validate/Clarify:** If essential information (like a valid Order ID) is missing or ambiguous, use the 'request_user_input' action (type: 'workflow_control') to ask the user for clarification. Formulate a clear 'query_to_user'. If you are unsure about parameters for a tool, add a 'warning' to the action object.
  3.  **Execute:** If the request is clear, plan the necessary actions using available tools like 'OrderLookupTool' or 'OrderModificationAPI'. Add your plan to 'thoughts'. Formulate the action objects accurately with 'action', 'type', and 'params'.
  4.  **Respond:** Use the 'send_response' action (type: 'output') to communicate results or status updates. You can send multiple `send_response` actions to build a reply piece by piece if needed (e.g., one for acknowledgment, one for results). If providing a final consolidated answer, set the root 'status' field to 'SUCCESS_FINAL' in your JSON output and use 'send_response' for the content.

  **Output Format:**
  Respond ONLY with the standard LLM JSON Output Schema (v0.3), including 'status', 'thoughts', 'actions', and optionally 'final_response' (though prefer using `send_response` actions).

  **Available Tools:** $(agent.available_tools_list) # Placeholder dynamically injected by orchestrator
iteration_cap: 8 # Allow for clarification cycles and tool use

# --- Capabilities (Expected Tools) ---
# This agent profile declares its dependency on these tools.
# The orchestrator must ensure compatible tools are registered when this agent runs.
tools:
  - ref: "OrderLookupTool" # Expects a tool named 'OrderLookupTool' (defined elsewhere)
  - ref: "OrderModificationAPI" # Expects a tool/API named 'OrderModificationAPI'
  # - ref: "UserProfileTool" # Potentially needed

# --- Environment (Optional Defaults) ---
# environment:
#   DEFAULT_ORDER_SYSTEM: "SystemA"

# --- Optional Sections ---
# extra_prompts:
#   - "Always confirm the Order ID with the user before attempting modifications."
# tasks: # Conceptual task list
#   - "Identify user intent and Order ID."
#   - "Validate Order ID format."
#   - "Request clarification if needed."
#   - "Lookup order details."
#   - "Perform modification if requested."
#   - "Report status/outcome."
# directive: # Default directive
#   type: "NORMAL"
#   description: "Process incoming order requests."
# api_config: {} # FUTURE
# memory: {} # FUTURE
# history: {} # FUTURE
# persistence: {} # FUTURE
```

### File: config/agents/test.yml
```yaml
# --- Agent Profile ---
version: "agent-1.1" # Slightly updated version marker for agent profiles
name: "OrderProcessingAgent"
description: "Processes customer order requests, checks status using tools, handles ambiguities by requesting clarification, and sends formatted updates."

# --- Core Behavior ---
system_prompt: |
  You are an Order Processing Agent. Your goal is to handle user requests related to orders.

  **Workflow:**
  1.  **Understand:** Analyze the user's request in $(input.user_request). Identify order IDs, user IDs ($(initial_state.user_id)), and the specific action requested (check status, modify, cancel). Use structured 'thoughts' (OBSERVATION, PLAN, QUESTION) to document your understanding.
  2.  **Validate/Clarify:** If essential information (like a valid Order ID) is missing or ambiguous, use the 'request_user_input' action (type: 'workflow_control') to ask the user for clarification. Formulate a clear 'query_to_user'. If you are unsure about parameters for a tool, add a 'warning' to the action object.
  3.  **Execute:** If the request is clear, plan the necessary actions using available tools like 'OrderLookupTool' or 'OrderModificationAPI'. Add your plan to 'thoughts'. Formulate the action objects accurately with 'action', 'type', and 'params'.
  4.  **Respond:** Use the 'send_response' action (type: 'output') to communicate results or status updates. You can send multiple `send_response` actions to build a reply piece by piece if needed (e.g., one for acknowledgment, one for results). If providing a final consolidated answer, set the root 'status' field to 'SUCCESS_FINAL' in your JSON output and use 'send_response' for the content.

  **Output Format:**
  Respond ONLY with the standard LLM JSON Output Schema (v0.3), including 'status', 'thoughts', 'actions', and optionally 'final_response' (though prefer using `send_response` actions).

  **Available Tools:** $(agent.available_tools_list) # Placeholder dynamically injected by orchestrator
iteration_cap: 8 # Allow for clarification cycles and tool use

# --- Capabilities (Expected Tools) ---
# This agent profile declares its dependency on these tools.
# The orchestrator must ensure compatible tools are registered when this agent runs.
tools:
  - ref: "OrderLookupTool" # Expects a tool named 'OrderLookupTool' (defined elsewhere)
  - ref: "OrderModificationAPI" # Expects a tool/API named 'OrderModificationAPI'
  # - ref: "UserProfileTool" # Potentially needed

# --- Environment (Optional Defaults) ---
environment:
  DEFAULT_ORDER_SYSTEM: "SystemA"

# --- Optional Sections ---
# extra_prompts:
#   - "Always confirm the Order ID with the user before attempting modifications."
# tasks: # Conceptual task list
#   - "Identify user intent and Order ID."
#   - "Validate Order ID format."
#   - "Request clarification if needed."
#   - "Lookup order details."
#   - "Perform modification if requested."
#   - "Report status/outcome."

# directive: # Default directive
#   type: "NORMAL"
#   description: "Process incoming order requests."

# api_config: {} # FUTURE
# memory: {} # FUTURE
# history: {} # FUTURE
# persistence: {} # FUTURE
```

### File: config/drafts/explicit.yaml
```yaml
# --- Agent Profile Draft 9: Explicit Config ---
version: "agent-1.1"

environment:
  SMTP_SERVER: "smtp.example.com"
  SENDER_EMAIL: "reporter@example.com"

defines:
  variables:
    - max_report_length: 1000
    - default_recipient: "admin@example.com"
    - repo_context: "$REPO_PATH"
  tools:
    # ReportGenerator:
    #   name: "generate_report"
    #   description: "Generates a report based on input data. Params: {'data': object}"
    #   type: "internal_function" # Example
    EmailSender:
      name: "send_email"
      description: "Sends an email. Params: {'to': string, 'subject': string, 'body': string}"
      type: "script" # Example
      runtime: "python"
      path: "./scripts/utils/send_email.py"

name: "ReportingAgent"
description: "Generates and sends reports based on processed data and specific directives."

system_prompt: |
  You are the Reporting Agent. Your main function is to generate reports using the 'generate_report' tool based on input data ($(input.data)), adhering to the current directive's format requirements. If the directive includes sending, use the 'send_email' tool, defaulting to $(variables.default_recipient). Report length should not exceed $(variables.max_report_length) characters. Use the standard JSON output schema.

capabilities:
  tools:
    - "ReportGenerator"
    - "EmailSender"

iteration_cap: 6


extra_prompts:
  - "Ensure reports are formatted according to the directive format."
  - "Confirm recipient address before sending email."
  - "Keep reports concise and factual."

# tasks:
#   - "Receive data and directive."
#   - "Validate input data structure."
#   - "Generate report content using 'generate_report'."
#   - "Check report length against $(variables.max_report_length)."
#   - "If directive requires sending, use 'send_email'."
#   - "Provide final status response."

directive:
  type: "REPORT"
  description: "Generate a weekly sales summary report and email it."
  format: "Markdown format with sections for Total Sales, Top Products, Key Changes. Email subject: Weekly Sales Summary"

schema: |
  # (LLM Output Schema v0.3)
  { ... }
example: |
  # (Example LLM Output JSON for generating report)
  {
    "status": "REQUIRES_ACTION",
    "thoughts": [
      {"type": "PLAN", "content": "Generate report using provided data."},
      {"type": "OBSERVATION", "content": "Directive requires Markdown format."}
    ],
    "actions": [{
      "action": "generate_report",
      "type": "internal_function",
      "params": { "data": "$(state.sales_data)" }
    }],
    "final_response": null
  }
```

### File: config/drafts/refactorMK1.yaml
```yaml
# --- Agent Profile Draft 1: Basic Refactor ---
version: "agent-1.1"

defines:
  variables:
    - default_greeting: "Hello! How may I assist you today?"
    - default_error: "I encountered an issue. Please try again or rephrase."
  tools:
    BashExecutor:
      name: "bash"
      description: "Executes a bash command provided as input. USE WITH CAUTION."
      type: "code" # Could be 'script'
      runtime: "bash"
      code: |
        #!/bin/bash
        eval "$1" # Simplified example

name: "BasicAgent"
description: |
  A foundational agent using the defines structure. Understands requests, follows JSON schema.

system_prompt: |
  You are a helpful AI assistant. Understand the user request and respond following the schema provided below. Use defined variables like $(variables.default_greeting) when appropriate. If you need to run a shell command, use the 'bash' tool defined in capabilities.

capabilities:
  tools: # List tools *enabled* for this agent, referencing defines.tools keys
    - "BashExecutor" # This agent can use the BashExecutor defined above

iteration_cap: 8

schema: |
  # (LLM Output Schema v0.3 - As defined previously)
  {
    status: string (REQUIRED, enum: SUCCESS_FINAL | REQUIRES_ACTION | REQUIRES_CLARIFICATION | ERROR_INTERNAL),
    thoughts: array<object> (REQUIRED, object: { type: string (REQUIRED, enum: PLAN | OBSERVATION | QUESTION | NORM | ASSUMPTION | GOAL | HYPOTHESIS | CRITIQUE), content: string (REQUIRED) }),
    actions: array<object> (REQUIRED, object: { action: string (REQUIRED), type: string (REQUIRED, enum: tool | script | http_request | internal_function | output | workflow_control), params: object (REQUIRED, structure depends on action/type), confidence?: float (0.0-1.0), warnings?: array<string> }),
    final_response: string | null (DEPRECATED? Use only if status=SUCCESS_FINAL & actions=[])
  }
example: |
  # (Example LLM Output JSON - As defined previously)
  {
    "status": "SUCCESS_FINAL",
    "thoughts": [{"type": "OBSERVATION", "content": "User provided a simple greeting."}],
    "actions": [],
    "final_response": "$(variables.default_greeting)"
  }

extra_prompts:
  - "Prioritize clarity and politeness."
```

### File: dashboard/app.js
```javascript
// dashboard/app.js

document.addEventListener('DOMContentLoaded', () => {
    const chatbox = document.getElementById('chatbox');
    const userInput = document.getElementById('userInput');
    const sendButton = document.getElementById('sendButton');

    // --- Configuration ---
    // IMPORTANT: Replace with your actual server endpoint and expected JSON keys
    // If frontend and backend are served from different ports (e.g., 8000 and 8080),
    // use the full backend URL like 'http://localhost:8080/api/prompt'
    // and ensure your C++ server has CORS headers enabled.
    const API_ENDPOINT = 'http://localhost:7777/prompt'; // Example: '/api/prompt' or 'http://localhost:8080/api/prompt'
    const REQUEST_KEY = 'prompt';       // Key for user message in request JSON (e.g., "prompt")
    const RESPONSE_KEY = 'response';    // Key for agent message in response JSON (e.g., "response")
    // ---------------------

    /**
     * Adds a message bubble to the chatbox.
     * @param {string} text - The message content.
     * @param {'user' | 'agent' | 'error'} sender - The sender type ('user', 'agent', or 'error').
     */
    function addMessage(text, sender) {
        if (!chatbox) {
            console.error("Chatbox element not found!");
            return;
        }
        const messageDiv = document.createElement('div');
        messageDiv.classList.add('message', sender);
        messageDiv.textContent = text; // Using textContent for safety against XSS
        chatbox.appendChild(messageDiv);
        // Scroll to the bottom to show the latest message
        chatbox.scrollTop = chatbox.scrollHeight;
    }

    /**
     * Sends the user's message to the backend API and displays the response.
     */
    async function sendMessage() {
        if (!userInput || !sendButton) {
            console.error("Input or button element not found!");
            return;
        }
        const userText = userInput.value.trim();
        if (!userText) {
            return; // Don't send empty messages
        }

        // Display user message immediately
        addMessage(userText, 'user');
        const currentInputText = userText; // Store before clearing
        userInput.value = ''; // Clear input field
        sendButton.disabled = true; // Prevent multiple clicks while waiting
        userInput.disabled = true;

        try {
            // Prepare the request payload
            const requestBody = {};
            requestBody[REQUEST_KEY] = currentInputText;

            const response = await fetch(API_ENDPOINT, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    // Add any other headers your server might require
                },
                body: JSON.stringify(requestBody)
            });

            if (!response.ok) {
                // Attempt to get error details from the response body
                let errorText = `HTTP error! Status: ${response.status}`;
                try {
                    const errorData = await response.json();
                    errorText += `, Message: ${JSON.stringify(errorData)}`;
                } catch (e) {
                    // If response is not JSON, use text
                    const textError = await response.text();
                    errorText += `, Body: ${textError}`;
                }
                throw new Error(errorText);
            }

            // Parse the JSON response from the server
            const data = await response.json();

            // Extract the agent's reply using the configured key
            const agentResponse = data[RESPONSE_KEY];

            if (agentResponse !== undefined && agentResponse !== null) {
                addMessage(agentResponse, 'agent');
            } else {
                 console.warn(`Response received, but key "${RESPONSE_KEY}" was missing or null.`, data);
                 addMessage(`Received response, but couldn't find the expected content (key: "${RESPONSE_KEY}").`, 'error');
            }

        } catch (error) {
            console.error('Error sending message or processing response:', error);
            addMessage(`Error: ${error.message}`, 'error');
        } finally {
            // Re-enable input fields regardless of success or failure
            sendButton.disabled = false;
            userInput.disabled = false;
            userInput.focus(); // Set focus back to the input field
        }
    }

    // --- Event Listeners ---
    if (sendButton) {
        sendButton.addEventListener('click', sendMessage);
    } else {
        console.error("Send button not found!");
    }

    if (userInput) {
        userInput.addEventListener('keypress', (event) => {
            // Send message if Enter key is pressed (and Shift key is not held)
            if (event.key === 'Enter' && !event.shiftKey) {
                event.preventDefault(); // Prevent default Enter behavior (like adding a newline)
                sendMessage();
            }
        });
    } else {
        console.error("User input element not found!");
    }

    // Optional: Add a welcome message on load
    addMessage("Agent connected. How can I help you today?", 'agent');
});
```

### File: dashboard/index.html
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Agent Server Interface</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <div class="container">
        <h1>Agent Server</h1>
        <div id="chatbox">
            <!-- Chat messages will appear here -->
            <div class="message agent">Welcome! Ask me anything.</div>
        </div>
        <div class="input-area">
            <input type="text" id="userInput" placeholder="Type your message...">
            <button id="sendButton">Send</button>
        </div>
    </div>

    <script src="app.js"></script>
</body>
</html>
```

### File: dashboard/style.css
```css
body {
    font-family: sans-serif;
    margin: 0;
    background-color: #f4f4f4;
    display: flex;
    justify-content: center;
    align-items: center;
    min-height: 100vh;
}

.container {
    background-color: #fff;
    padding: 20px;
    border-radius: 8px;
    box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
    width: 80%;
    max-width: 600px;
    display: flex;
    flex-direction: column;
    height: 80vh; /* Limit height */
}

h1 {
    text-align: center;
    color: #333;
    margin-top: 0;
}

#chatbox {
    flex-grow: 1; /* Takes available space */
    border: 1px solid #ccc;
    padding: 10px;
    margin-bottom: 15px;
    overflow-y: auto; /* Enable scrolling */
    background-color: #f9f9f9;
    border-radius: 4px;
}

.message {
    margin-bottom: 10px;
    padding: 8px 12px;
    border-radius: 5px;
    line-height: 1.4;
}

.user {
    background-color: #d1e7fd;
    text-align: right;
    margin-left: auto; /* Push to right */
    max-width: 80%;
}

.agent {
    background-color: #e2e3e5;
    text-align: left;
    margin-right: auto; /* Push to left */
    max-width: 80%;
}

.error {
    background-color: #f8d7da;
    color: #721c24;
    text-align: left;
    max-width: 80%;
}


.input-area {
    display: flex;
}

#userInput {
    flex-grow: 1;
    padding: 10px;
    border: 1px solid #ccc;
    border-radius: 4px 0 0 4px;
}

#sendButton {
    padding: 10px 15px;
    background-color: #007bff;
    color: white;
    border: none;
    border-radius: 0 4px 4px 0;
    cursor: pointer;
    transition: background-color 0.2s;
}

#sendButton:hover {
    background-color: #0056b3;
}
```

### File: dashboard/templates/index.html
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Agent Chat Interface</title>
    <style>
        /* Basic Reset & Font */
        :root {
            --user-bg: #e1fec6; /* Lighter green */
            --agent-bg: #f0f0f0; /* Light grey */
            --error-bg: #f8d7da;
            --error-text: #721c24;
            --border-color: #ddd;
            --button-bg: #007bff;
            --button-hover-bg: #0056b3;
            --mic-button-bg: #6c757d;
            --mic-button-hover-bg: #5a6268;
            --mic-button-active-bg: #dc3545; /* Red when recording */
            --text-color: #333;
            --secondary-text: #666;
            --bg-color: #f9f9f9;
            --chat-bg: #fff;
            --input-focus-border: #80bdff;
            --input-focus-shadow: rgba(0, 123, 255, 0.25);
        }

        html { box-sizing: border-box; height: 100%; }
        *, *:before, *:after { box-sizing: inherit; }

        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol";
            display: flex;
            flex-direction: column;
            height: 100%;
            margin: 0;
            background-color: var(--bg-color);
            color: var(--text-color);
        }

        header {
            background-color: var(--chat-bg);
            padding: 10px 15px;
            text-align: center;
            border-bottom: 1px solid var(--border-color);
            flex-shrink: 0;
        }
        h1 { margin: 0; font-size: 1.3em; color: var(--text-color); font-weight: 500; }

        #chat-container {
            flex-grow: 1;
            padding: 15px;
            overflow-y: auto;
            background-color: var(--chat-bg);
            display: flex;
            flex-direction: column;
            gap: 10px;
            scroll-behavior: smooth;
        }

        .message {
            padding: 10px 15px;
            border-radius: 18px;
            line-height: 1.4;
            max-width: 80%;
            word-wrap: break-word;
            box-shadow: 0 1px 1px rgba(0,0,0,0.05);
            font-size: 0.95em;
        }
        .user-message { background-color: var(--user-bg); margin-left: auto; border-bottom-right-radius: 5px; align-self: flex-end; }
        .agent-message { background-color: var(--agent-bg); margin-right: auto; border-bottom-left-radius: 5px; align-self: flex-start; }
        .system-error-message {
            background-color: var(--error-bg); color: var(--error-text);
            border: 1px solid #f5c6cb; border-radius: 8px;
            align-self: stretch; margin: 5px auto; max-width: 95%;
            font-size: 0.9em; text-align: center;
        }

        #status {
            padding: 5px 15px; font-size: 0.85em; color: var(--secondary-text);
            min-height: 1.5em; text-align: center; font-style: italic;
            flex-shrink: 0; background-color: var(--bg-color);
        }

        #input-area {
            display: flex; padding: 10px 15px; border-top: 1px solid var(--border-color);
            background-color: var(--bg-color); flex-shrink: 0; gap: 10px; align-items: center;
        }

        #userInput {
            flex-grow: 1; padding: 10px 15px; border: 1px solid var(--border-color);
            border-radius: 20px; font-size: 1em; background-color: var(--chat-bg);
            outline: none; transition: border-color 0.2s ease, box-shadow 0.2s ease;
        }
        #userInput:focus {
            border-color: var(--input-focus-border);
            box-shadow: 0 0 0 3px var(--input-focus-shadow);
        }

        /* Button Styling */
        .chat-button {
            padding: 10px; /* Adjust for icon */
            min-width: 40px; /* Ensure consistent size */
            height: 40px;
            border: none;
            color: white;
            border-radius: 50%; /* Make it round */
            cursor: pointer;
            font-size: 1.2em; /* Adjust icon size */
            font-weight: 500;
            transition: background-color 0.2s ease, transform 0.1s ease;
            flex-shrink: 0;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .chat-button:active {
             transform: scale(0.95); /* Click feedback */
        }
        .chat-button:disabled {
            background-color: #aaa;
            cursor: not-allowed;
            transform: none;
        }

        #sendButton {
            background-color: var(--button-bg);
            padding: 10px 20px; /* Keep padding for text button */
            border-radius: 20px; /* Pill shape */
            font-size: 1em; /* Reset font size for text */
        }
        #sendButton:hover:not(:disabled) { background-color: var(--button-hover-bg); }

        #micButton {
            background-color: var(--mic-button-bg);
        }
        #micButton:hover:not(:disabled) { background-color: var(--mic-button-hover-bg); }
        #micButton.recording {
            background-color: var(--mic-button-active-bg); /* Red when recording */
            animation: pulse 1.5s infinite ease-in-out;
        }
        /* Hide mic button if not supported */
        #micButton.hidden {
            display: none;
        }

        @keyframes pulse {
            0% { box-shadow: 0 0 0 0 rgba(220, 53, 69, 0.7); }
            70% { box-shadow: 0 0 0 10px rgba(220, 53, 69, 0); }
            100% { box-shadow: 0 0 0 0 rgba(220, 53, 69, 0); }
        }

    </style>
</head>
<body>

    <header>
        <h1>C++ Agent Mk2</h1>
    </header>

    <div id="chat-container">
        <div class="agent-message">Hello! How can I assist you today? (Try the 🎤 button if your browser supports it!)</div>
    </div>

    <div id="status"></div>

    <div id="input-area">
        <input type="text" id="userInput" placeholder="Type or use the mic..." autocomplete="off">
        <!-- Microphone Button Added -->
        <button id="micButton" class="chat-button hidden" title="Start/Stop Recording">🎤</button>
        <button id="sendButton" class="chat-button" title="Send Message">Send</button>
    </div>

    <script>
        // --- Configuration ---
        const API_URL = 'https://agent.clevo.ddnsgeek.com/prompt'; // <<<<< MODIFIED HERE (HTTPS, no port)
        const REQUEST_TIMEOUT_MS = 30000;
        // --------------------
        // IMPORTANT: Your backend server MUST support HTTPS on port 443
        // and send correct CORS headers for this to work.

        const chatContainer = document.getElementById('chat-container');
        const userInput = document.getElementById('userInput');
        const sendButton = document.getElementById('sendButton');
        const micButton = document.getElementById('micButton');
        const statusDiv = document.getElementById('status');

        // --- Speech Recognition Setup ---
        const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
        let recognition;
        let isRecording = false;
        let finalTranscript = '';

        if (SpeechRecognition) {
            micButton.classList.remove('hidden'); // Show button if API is supported
            recognition = new SpeechRecognition();
            recognition.continuous = false;
            recognition.interimResults = true;
            recognition.lang = 'en-US';

            recognition.onstart = () => {
                console.log("Speech recognition started");
                isRecording = true;
                micButton.classList.add('recording');
                micButton.title = "Stop Recording";
                setStatus('Listening...', false);
                sendButton.disabled = true;
            };

            recognition.onresult = (event) => {
                let interimTranscript = '';
                finalTranscript = '';
                for (let i = event.resultIndex; i < event.results.length; ++i) {
                    const transcriptPart = event.results[i][0].transcript;
                    if (event.results[i].isFinal) {
                        finalTranscript += transcriptPart;
                    } else {
                        interimTranscript += transcriptPart;
                    }
                }
                userInput.value = finalTranscript || interimTranscript;
                console.log("Interim: ", interimTranscript, "Final: ", finalTranscript);
            };

            recognition.onerror = (event) => {
                console.error("Speech recognition error", event.error);
                let errorMsg = `Speech Error: ${event.error}`;
                if (event.error === 'no-speech') {
                    errorMsg = "No speech detected. Please try again.";
                } else if (event.error === 'audio-capture') {
                    errorMsg = "Microphone error. Ensure it's connected and enabled.";
                } else if (event.error === 'not-allowed' || event.error === 'service-not-allowed') {
                    // Note: 'service-not-allowed' can happen if mic access is granted but the browser/OS blocks it later
                    errorMsg = "Microphone access denied. Please allow access in browser/OS settings.";
                } else if (event.error === 'network') {
                    errorMsg = "Network error during speech recognition.";
                }
                setStatus(errorMsg, false);
                isRecording = false;
                micButton.classList.remove('recording');
                micButton.title = "Start Recording";
                 sendButton.disabled = false;
            };

            recognition.onend = () => {
                console.log("Speech recognition ended");
                isRecording = false;
                micButton.classList.remove('recording');
                micButton.title = "Start Recording";
                 sendButton.disabled = false;
                 setStatus('', false);

                if (finalTranscript.trim()) {
                    console.log("Sending final transcript:", finalTranscript);
                    userInput.value = finalTranscript.trim();
                    sendMessage();
                } else {
                     console.log("No final transcript obtained.");
                }
                finalTranscript = '';
            };

            micButton.addEventListener('click', () => {
                if (!recognition) return;

                if (isRecording) {
                    recognition.stop();
                } else {
                    finalTranscript = '';
                    try {
                        // Request microphone permission explicitly if not already granted
                        // This part is more complex and browser-dependent for explicit checks
                        // navigator.mediaDevices.getUserMedia({ audio: true }).then(() => {
                        //    recognition.start();
                        // }).catch(err => {
                        //    console.error("getUserMedia error:", err);
                        //    setStatus("Microphone access denied or error.", false);
                        // });
                        recognition.start(); // Simpler approach relies on browser's built-in permission prompts
                    } catch (e) {
                        console.error("Error starting recognition:", e);
                        setStatus("Could not start microphone. Check permissions.", false);
                    }
                }
            });

        } else {
            console.warn("Web Speech Recognition API not supported in this browser.");
        }
        // --- End Speech Recognition Setup ---


        function scrollToBottom() {
            // Add slight delay to ensure DOM is updated before scrolling
            setTimeout(() => {
                chatContainer.scrollTop = chatContainer.scrollHeight;
            }, 50);
        }

        function addMessageToChat(sender, messageText, isError = false) {
            const messageDiv = document.createElement('div');
            messageDiv.classList.add('message');

            switch(sender) {
                case 'user': messageDiv.classList.add('user-message'); break;
                case 'agent': messageDiv.classList.add('agent-message'); break;
                case 'system':
                    messageDiv.classList.add(isError ? 'system-error-message' : 'agent-message');
                    if (!isError) {
                       messageDiv.style.fontStyle = 'italic';
                       messageDiv.style.color = 'var(--secondary-text)';
                    }
                    break;
                default: console.warn("Unknown sender:", sender); messageDiv.classList.add('agent-message');
            }

            // Basic sanitization (replace < and > to prevent simple HTML injection)
            const sanitizedText = messageText.replace(/</g, "<").replace(/>/g, ">");
            messageDiv.textContent = sanitizedText; // Use textContent for safety
            chatContainer.appendChild(messageDiv);
            scrollToBottom(); // Call scroll after adding the message
        }

        function setStatus(text, isThinking) {
             statusDiv.textContent = text;
             sendButton.disabled = isThinking || isRecording;
             userInput.disabled = isThinking;
             micButton.disabled = isThinking;
        }

        async function sendMessage() {
            const prompt = userInput.value.trim();
            if (!prompt) {
                userInput.focus();
                return;
            }

            const lastMessage = chatContainer.lastElementChild;
            // Check if the last message is NOT from the user OR if it IS from the user but has different content
            // (This prevents adding the same message twice if sent via Enter right after speech recognition finishes)
            if (!lastMessage || !lastMessage.classList.contains('user-message') || lastMessage.textContent !== prompt) {
               addMessageToChat('user', prompt);
            }

            userInput.value = '';
            setStatus('Agent is thinking...', true);

            const controller = new AbortController();
            const timeoutId = setTimeout(() => controller.abort(), REQUEST_TIMEOUT_MS);

            try {
                console.log(`Sending request to: ${API_URL}`);
                const response = await fetch(API_URL, {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                        'Accept': 'application/json'
                        // CORS headers ('Access-Control-Allow-Origin', etc.)
                        // are set by the SERVER, not sent by the client.
                        },
                    body: JSON.stringify({ prompt: prompt }),
                    signal: controller.signal
                    // mode: 'cors' // Often default, ensures CORS behavior is expected
                });
                clearTimeout(timeoutId);

                if (!response.ok) {
                    let errorDetailsText = `Server responded with status ${response.status} ${response.statusText}.`;
                    try {
                        const textError = await response.text();
                        console.error("Raw error response body:", textError);
                        const contentType = response.headers.get("content-type");
                         if (contentType && contentType.includes("application/json")) {
                            try {
                                const errorData = JSON.parse(textError);
                                errorDetailsText = `Agent API Error: ${errorData.error || response.statusText}`;
                                if(errorData.details) errorDetailsText += `\nDetails: ${errorData.details}`;
                            } catch (jsonParseError) {
                                console.warn("Could not parse JSON error response:", jsonParseError);
                                errorDetailsText += `\nResponse body (non-JSON): ${textError.substring(0, 150)}${textError.length > 150 ? '...' : ''}`;
                            }
                        } else {
                             errorDetailsText += `\nResponse body: ${textError.substring(0, 150)}${textError.length > 150 ? '...' : ''}`;
                        }
                    } catch (readError) {
                        console.warn("Could not read error response body:", readError);
                        errorDetailsText = `Error: ${response.status} ${response.statusText}. Failed to read response body.`;
                    }
                     throw new Error(errorDetailsText);
                }

                const data = await response.json();
                if (data && typeof data.response === 'string') {
                    addMessageToChat('agent', data.response);
                } else {
                    console.warn("Unexpected success response structure:", data);
                    addMessageToChat('system', 'Received an unexpected response format from the agent.', true);
                }

            } catch (error) {
                 clearTimeout(timeoutId);
                 if (error.name === 'AbortError') {
                    console.error('Fetch aborted due to timeout.');
                    addMessageToChat('system', `Request timed out after ${REQUEST_TIMEOUT_MS / 1000} seconds. The agent might be busy or unreachable.`, true);
                 } else if (error instanceof TypeError && error.message.includes('Failed to fetch')) {
                     console.error('Network/CORS/HTTPS error:', error);
                     // Check if the protocol is HTTPS, as certificate issues are common
                     const protocol = new URL(API_URL).protocol;
                     let helpText = `Network Error: Could not connect to the agent at ${API_URL}. Check the URL, network connection, and ensure the server allows cross-origin requests (CORS).`;
                     if (protocol === 'https:') {
                        helpText += " Also verify the server has a valid HTTPS certificate.";
                     }
                     addMessageToChat('system', helpText, true);
                 }
                 else {
                    console.error('Error sending message:', error);
                    addMessageToChat('system', `Failed to get response: ${error.message}`, true);
                 }
            } finally {
                setStatus('', false);
                if (!isRecording) {
                   userInput.focus();
                }
            }
        }

        // --- Event Listeners ---
        sendButton.addEventListener('click', sendMessage);
        userInput.addEventListener('keypress', (event) => {
            if (event.key === 'Enter' && !sendButton.disabled) {
                event.preventDefault();
                sendMessage();
            }
        });

        // --- Initial Setup ---
        scrollToBottom();
        userInput.focus();
        console.log(`Agent API URL set to: ${API_URL}`);

    </script>

</body>
</html>
```

### File: dashboard/templates/index-voice.html
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Agent Chat Interface</title>
    <style>
        /* Basic Reset & Font */
        :root {
            --user-bg: #e1fec6; /* Lighter green */
            --agent-bg: #f0f0f0; /* Light grey */
            --error-bg: #f8d7da;
            --error-text: #721c24;
            --border-color: #ddd;
            --button-bg: #007bff;
            --button-hover-bg: #0056b3;
            --mic-button-bg: #6c757d;
            --mic-button-hover-bg: #5a6268;
            --mic-button-active-bg: #dc3545; /* Red when recording */
            --text-color: #333;
            --secondary-text: #666;
            --bg-color: #f9f9f9;
            --chat-bg: #fff;
            --input-focus-border: #80bdff;
            --input-focus-shadow: rgba(0, 123, 255, 0.25);
        }

        html { box-sizing: border-box; height: 100%; }
        *, *:before, *:after { box-sizing: inherit; }

        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol";
            display: flex;
            flex-direction: column;
            height: 100%;
            margin: 0;
            background-color: var(--bg-color);
            color: var(--text-color);
        }

        header {
            background-color: var(--chat-bg);
            padding: 10px 15px;
            text-align: center;
            border-bottom: 1px solid var(--border-color);
            flex-shrink: 0;
        }
        h1 { margin: 0; font-size: 1.3em; color: var(--text-color); font-weight: 500; }

        #chat-container {
            flex-grow: 1;
            padding: 15px;
            overflow-y: auto;
            background-color: var(--chat-bg);
            display: flex;
            flex-direction: column;
            gap: 10px;
            scroll-behavior: smooth;
        }

        .message {
            padding: 10px 15px;
            border-radius: 18px;
            line-height: 1.4;
            max-width: 80%;
            word-wrap: break-word;
            box-shadow: 0 1px 1px rgba(0,0,0,0.05);
            font-size: 0.95em;
        }
        .user-message { background-color: var(--user-bg); margin-left: auto; border-bottom-right-radius: 5px; align-self: flex-end; }
        .agent-message { background-color: var(--agent-bg); margin-right: auto; border-bottom-left-radius: 5px; align-self: flex-start; }
        .system-error-message {
            background-color: var(--error-bg); color: var(--error-text);
            border: 1px solid #f5c6cb; border-radius: 8px;
            align-self: stretch; margin: 5px auto; max-width: 95%;
            font-size: 0.9em; text-align: center;
        }

        #status {
            padding: 5px 15px; font-size: 0.85em; color: var(--secondary-text);
            min-height: 1.5em; text-align: center; font-style: italic;
            flex-shrink: 0; background-color: var(--bg-color);
        }

        #input-area {
            display: flex; padding: 10px 15px; border-top: 1px solid var(--border-color);
            background-color: var(--bg-color); flex-shrink: 0; gap: 10px; align-items: center;
        }

        #userInput {
            flex-grow: 1; padding: 10px 15px; border: 1px solid var(--border-color);
            border-radius: 20px; font-size: 1em; background-color: var(--chat-bg);
            outline: none; transition: border-color 0.2s ease, box-shadow 0.2s ease;
        }
        #userInput:focus {
            border-color: var(--input-focus-border);
            box-shadow: 0 0 0 3px var(--input-focus-shadow);
        }

        /* Button Styling */
        .chat-button {
            padding: 10px; /* Adjust for icon */
            min-width: 40px; /* Ensure consistent size */
            height: 40px;
            border: none;
            color: white;
            border-radius: 50%; /* Make it round */
            cursor: pointer;
            font-size: 1.2em; /* Adjust icon size */
            font-weight: 500;
            transition: background-color 0.2s ease, transform 0.1s ease;
            flex-shrink: 0;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .chat-button:active {
             transform: scale(0.95); /* Click feedback */
        }
        .chat-button:disabled {
            background-color: #aaa;
            cursor: not-allowed;
            transform: none;
        }

        #sendButton {
            background-color: var(--button-bg);
            padding: 10px 20px; /* Keep padding for text button */
            border-radius: 20px; /* Pill shape */
            font-size: 1em; /* Reset font size for text */
        }
        #sendButton:hover:not(:disabled) { background-color: var(--button-hover-bg); }

        #micButton {
            background-color: var(--mic-button-bg);
        }
        #micButton:hover:not(:disabled) { background-color: var(--mic-button-hover-bg); }
        #micButton.recording {
            background-color: var(--mic-button-active-bg); /* Red when recording */
            animation: pulse 1.5s infinite ease-in-out;
        }
        /* Hide mic button if not supported */
        #micButton.hidden {
            display: none;
        }

        @keyframes pulse {
            0% { box-shadow: 0 0 0 0 rgba(220, 53, 69, 0.7); }
            70% { box-shadow: 0 0 0 10px rgba(220, 53, 69, 0); }
            100% { box-shadow: 0 0 0 0 rgba(220, 53, 69, 0); }
        }

    </style>
</head>
<body>

    <header>
        <h1>C++ Agent Mk2</h1>
    </header>

    <div id="chat-container">
        <div class="agent-message">Hello! How can I assist you today? (Try the 🎤 button if your browser supports it!)</div>
    </div>

    <div id="status"></div>

    <div id="input-area">
        <input type="text" id="userInput" placeholder="Type or use the mic..." autocomplete="off">
        <!-- Microphone Button Added -->
        <button id="micButton" class="chat-button hidden" title="Start/Stop Recording">🎤</button>
        <button id="sendButton" class="chat-button" title="Send Message">Send</button>
    </div>

    <script>
        // --- Configuration ---
        const API_URL = 'http://localhost:7777/prompt';
        const REQUEST_TIMEOUT_MS = 30000;
        // --------------------

        const chatContainer = document.getElementById('chat-container');
        const userInput = document.getElementById('userInput');
        const sendButton = document.getElementById('sendButton');
        const micButton = document.getElementById('micButton'); // Get mic button
        const statusDiv = document.getElementById('status');

        // --- Speech Recognition Setup ---
        const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
        let recognition;
        let isRecording = false;
        let finalTranscript = '';

        if (SpeechRecognition) {
            micButton.classList.remove('hidden'); // Show button if API is supported
            recognition = new SpeechRecognition();
            recognition.continuous = false; // Process speech after user stops talking
            recognition.interimResults = true; // Show results as they come in
            recognition.lang = 'en-US'; // Adjust language if needed

            recognition.onstart = () => {
                console.log("Speech recognition started");
                isRecording = true;
                micButton.classList.add('recording');
                micButton.title = "Stop Recording";
                setStatus('Listening...', false); // Keep send button enabled initially
                sendButton.disabled = true; // Disable send while listening
            };

            recognition.onresult = (event) => {
                let interimTranscript = '';
                finalTranscript = ''; // Reset final transcript for this recognition cycle

                for (let i = event.resultIndex; i < event.results.length; ++i) {
                    const transcriptPart = event.results[i][0].transcript;
                    if (event.results[i].isFinal) {
                        finalTranscript += transcriptPart;
                    } else {
                        interimTranscript += transcriptPart;
                    }
                }

                // Update the input field with interim or final results
                userInput.value = finalTranscript || interimTranscript;
                console.log("Interim: ", interimTranscript, "Final: ", finalTranscript);
            };

            recognition.onerror = (event) => {
                console.error("Speech recognition error", event.error);
                let errorMsg = `Speech Error: ${event.error}`;
                if (event.error === 'no-speech') {
                    errorMsg = "No speech detected. Please try again.";
                } else if (event.error === 'audio-capture') {
                    errorMsg = "Microphone error. Ensure it's connected and enabled.";
                } else if (event.error === 'not-allowed') {
                    errorMsg = "Microphone access denied. Please allow access in browser settings.";
                } else if (event.error === 'network') {
                    errorMsg = "Network error during speech recognition.";
                }
                setStatus(errorMsg, false); // Show error, enable buttons
                isRecording = false; // Ensure recording state is reset
                micButton.classList.remove('recording');
                micButton.title = "Start Recording";
                 sendButton.disabled = false; // Re-enable send button on error
            };

            recognition.onend = () => {
                console.log("Speech recognition ended");
                isRecording = false;
                micButton.classList.remove('recording');
                micButton.title = "Start Recording";
                 sendButton.disabled = false; // Re-enable send button when done
                 setStatus('', false); // Clear listening status


                // If we got a final transcript, send it
                if (finalTranscript.trim()) {
                    console.log("Sending final transcript:", finalTranscript);
                    userInput.value = finalTranscript.trim(); // Ensure input has final text
                    sendMessage(); // Send the message
                } else {
                     // If no final transcript but maybe interim, keep interim in box
                     // Or clear it if you prefer: userInput.value = '';
                     console.log("No final transcript obtained.");
                     // Don't send if empty
                }
                finalTranscript = ''; // Clear for next time
            };

            // Microphone Button Event Listener
            micButton.addEventListener('click', () => {
                if (!recognition) return; // Safety check

                if (isRecording) {
                    recognition.stop();
                    // onend will handle state changes
                } else {
                    // Clear previous text before starting new recording
                    // userInput.value = ''; // Optional: clear field on mic press
                    finalTranscript = ''; // Reset transcript
                    try {
                        recognition.start();
                        // onstart will handle state changes
                    } catch (e) {
                        // Handle potential errors if start() fails immediately
                        console.error("Error starting recognition:", e);
                        setStatus("Could not start microphone.", false);
                    }
                }
            });

        } else {
            console.warn("Web Speech Recognition API not supported in this browser.");
            // Optionally inform the user more visibly
            // statusDiv.textContent = "Speech input not supported by your browser.";
        }
        // --- End Speech Recognition Setup ---


        function scrollToBottom() {
            chatContainer.scrollTop = chatContainer.scrollHeight;
        }

        function addMessageToChat(sender, messageText, isError = false) {
            const messageDiv = document.createElement('div');
            messageDiv.classList.add('message');

            switch(sender) {
                case 'user': messageDiv.classList.add('user-message'); break;
                case 'agent': messageDiv.classList.add('agent-message'); break;
                case 'system':
                    if (isError) messageDiv.classList.add('system-error-message');
                    else {
                        messageDiv.classList.add('agent-message');
                        messageDiv.style.fontStyle = 'italic';
                        messageDiv.style.color = 'var(--secondary-text)';
                    }
                    break;
                default: console.warn("Unknown sender:", sender); messageDiv.classList.add('agent-message');
            }
            messageDiv.textContent = messageText;
            chatContainer.appendChild(messageDiv);
            scrollToBottom();
        }

        function setStatus(text, isThinking) {
             statusDiv.textContent = text;
             // Disable send button if thinking OR recording
             sendButton.disabled = isThinking || isRecording;
             userInput.disabled = isThinking; // Only disable input when actually sending/processing
             micButton.disabled = isThinking; // Disable mic while sending to backend
        }

        async function sendMessage() {
            const prompt = userInput.value.trim();
            if (!prompt) {
                userInput.focus();
                return;
            }

            // Don't add user message again if it came from speech recognition final result
            // A simple check: did the last message come from the user already?
            const lastMessage = chatContainer.lastElementChild;
            if (!lastMessage || !lastMessage.classList.contains('user-message') || lastMessage.textContent !== prompt) {
               addMessageToChat('user', prompt);
            }

            userInput.value = '';
            setStatus('Agent is thinking...', true);

            const controller = new AbortController();
            const timeoutId = setTimeout(() => controller.abort(), REQUEST_TIMEOUT_MS);

            try {
                const response = await fetch(API_URL, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json', 'Accept': 'application/json' },
                    body: JSON.stringify({ prompt: prompt }),
                    signal: controller.signal
                });
                clearTimeout(timeoutId);

                if (!response.ok) {
                    let errorMsg = `Error: ${response.status} ${response.statusText}`;
                    let details = "";
                    try {
                        const contentType = response.headers.get("content-type");
                        if (contentType && contentType.includes("application/json")) {
                             const errorData = await response.json();
                             errorMsg = `Agent API Error: ${errorData.error || response.statusText}`;
                             details = errorData.details || "";
                        } else {
                            const textError = await response.text();
                            errorMsg = `Server Error (${response.status})`;
                            details = textError.substring(0, 150) + (textError.length > 150 ? '...' : '');
                        }
                    } catch (parseError) { console.warn("Could not parse error response:", parseError); }
                    throw new Error(`${errorMsg}${details ? `\nDetails: ${details}` : ''}`);
                }

                const data = await response.json();
                if (data && typeof data.response === 'string') {
                    addMessageToChat('agent', data.response);
                } else {
                    console.warn("Unexpected success response structure:", data);
                    addMessageToChat('system', 'Received an unexpected response format.', true);
                }

            } catch (error) {
                 clearTimeout(timeoutId);
                 if (error.name === 'AbortError') {
                    console.log('Fetch aborted due to timeout.');
                    addMessageToChat('system', `Request timed out after ${REQUEST_TIMEOUT_MS / 1000} seconds.`, true);
                 } else {
                    console.error('Error sending message:', error);
                    addMessageToChat('system', `Failed to get response: ${error.message}`, true);
                 }
            } finally {
                setStatus('', false); // Clear status, enable buttons (unless recording is active)
                userInput.focus();
            }
        }

        // --- Event Listeners ---
        sendButton.addEventListener('click', sendMessage);
        userInput.addEventListener('keypress', (event) => {
            if (event.key === 'Enter' && !sendButton.disabled) {
                event.preventDefault();
                sendMessage();
            }
        });

        // --- Initial Setup ---
        scrollToBottom();
        userInput.focus();

    </script>

</body>
</html>
```

### File: dashboard/templates/voice/app.js
```javascript
// dashboard/app.js

document.addEventListener('DOMContentLoaded', () => {
    const chatbox = document.getElementById('chatbox');
    const userInput = document.getElementById('userInput');
    const sendButton = document.getElementById('sendButton');
    const micButton = document.getElementById('micButton'); // Get the mic button
    const statusDiv = document.getElementById('status'); // Get the status div

    // --- Configuration ---
    const API_ENDPOINT = 'https://agent.clevo.ddnsgeek.com/prompt';
    const REQUEST_KEY = 'prompt';
    const RESPONSE_KEY = 'response';
    // ---------------------

    /**
     * Adds a message bubble to the chatbox.
     * @param {string} text - The message content.
     * @param {'user' | 'agent' | 'error' | 'system'} sender - The sender type.
     */
    function addMessage(text, sender) {
        if (!chatbox) {
            console.error("Chatbox element not found!");
            return;
        }
        const messageDiv = document.createElement('div');
        messageDiv.classList.add('message', sender);
        // Basic sanitization - prevent rendering HTML tags
        messageDiv.textContent = text;
        chatbox.appendChild(messageDiv);
        chatbox.scrollTop = chatbox.scrollHeight; // Scroll down
    }

    /**
     * Sets the status message and disables/enables inputs.
     * @param {string} text - The status text to display.
     * @param {boolean} isBusy - Whether the agent/system is busy (disables inputs).
     */
    function setStatus(text, isBusy) {
        if (statusDiv) statusDiv.textContent = text;
        if (userInput) userInput.disabled = isBusy;
        if (sendButton) sendButton.disabled = isBusy;
        // Also disable mic button when the system is busy sending/receiving messages
        if (micButton) micButton.disabled = isBusy;
    }

    /**
     * Sends the user's message to the backend API and displays the response.
     */
    async function sendMessage() {
        if (!userInput || !sendButton) {
            console.error("Input or button element not found!");
            return;
        }
        const userText = userInput.value.trim();
        if (!userText) {
            return; // Don't send empty messages
        }

        // Display user message immediately only if it's not already the last message
        // (prevents duplication if sent via Enter right after voice input finishes)
        const lastMessage = chatbox.lastElementChild;
        if (!lastMessage || !lastMessage.classList.contains('user') || lastMessage.textContent !== userText) {
            addMessage(userText, 'user');
        }

        const currentInputText = userText; // Store before clearing
        userInput.value = ''; // Clear input field
        setStatus('Agent thinking...', true); // Set status and disable inputs

        try {
            const requestBody = {};
            requestBody[REQUEST_KEY] = currentInputText;

            const response = await fetch(API_ENDPOINT, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(requestBody)
            });

            if (!response.ok) {
                let errorText = `HTTP error! Status: ${response.status}`;
                try {
                    const errorData = await response.json();
                    errorText += `, Message: ${JSON.stringify(errorData)}`;
                } catch (e) {
                    const textError = await response.text();
                    errorText += `, Body: ${textError}`;
                }
                throw new Error(errorText);
            }

            const data = await response.json();
            const agentResponse = data[RESPONSE_KEY];

            if (agentResponse !== undefined && agentResponse !== null) {
                addMessage(agentResponse, 'agent');
            } else {
                 console.warn(`Response key "${RESPONSE_KEY}" missing or null.`, data);
                 addMessage(`Received response, but couldn't find content (key: "${RESPONSE_KEY}").`, 'error');
            }

        } catch (error) {
            console.error('Error sending/receiving message:', error);
            addMessage(`Error: ${error.message}`, 'error');
        } finally {
            setStatus('', false); // Clear status, re-enable inputs
            userInput.focus();
        }
    }

    // --- Speech Recognition Setup ---
    const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
    let recognition;
    let isRecording = false;
    let finalTranscript = ''; // Store final transcript from speech events

    if (SpeechRecognition && micButton) {
        micButton.classList.remove('hidden'); // Show the button if API is supported
        recognition = new SpeechRecognition();
        recognition.continuous = false; // Process speech after user stops talking
        recognition.interimResults = true; // Show results as they come in
        recognition.lang = 'en-US'; // Adjust language if needed

        recognition.onstart = () => {
            console.log("Speech recognition started");
            isRecording = true;
            finalTranscript = ''; // Clear previous final transcript
            micButton.classList.add('recording');
            micButton.title = "Stop Recording";
            setStatus('Listening...', false); // Indicate listening, keep inputs enabled for now
            sendButton.disabled = true; // Disable send while listening
        };

        recognition.onresult = (event) => {
            let interimTranscript = '';
            finalTranscript = ''; // Recalculate final transcript each time

            for (let i = event.resultIndex; i < event.results.length; ++i) {
                const transcriptPart = event.results[i][0].transcript;
                if (event.results[i].isFinal) {
                    finalTranscript += transcriptPart;
                } else {
                    interimTranscript += transcriptPart;
                }
            }
            // Update the input field with the latest transcript (final takes precedence)
            userInput.value = finalTranscript || interimTranscript;
        };

        recognition.onerror = (event) => {
            console.error("Speech recognition error:", event.error, event.message);
            let errorMsg = `Speech Error: ${event.error}`;
            if (event.error === 'no-speech') {
                errorMsg = "No speech detected. Please try again.";
            } else if (event.error === 'audio-capture') {
                errorMsg = "Microphone error. Ensure it's connected and enabled.";
            } else if (event.error === 'not-allowed') {
                errorMsg = "Microphone access denied. Please allow access in browser settings.";
            } else if (event.error === 'network') {
                 errorMsg = "Network error during speech recognition.";
            }
            addMessage(errorMsg, 'system'); // Show error in chat
            setStatus('', false); // Clear status, enable inputs
            isRecording = false;
            micButton.classList.remove('recording');
            micButton.title = "Start Recording";
            sendButton.disabled = false; // Re-enable send on error
        };

        recognition.onend = () => {
            console.log("Speech recognition ended");
            isRecording = false;
            micButton.classList.remove('recording');
            micButton.title = "Start Recording";
            setStatus('', false); // Clear listening status
            sendButton.disabled = false; // Re-enable send button

            // IMPORTANT: Send the message automatically if we got a final transcript
            if (finalTranscript.trim()) {
                console.log("Final transcript received:", finalTranscript);
                userInput.value = finalTranscript.trim(); // Ensure input field has the final text
                sendMessage(); // <<< Automatically send the message
            } else {
                console.log("No final transcript obtained.");
                userInput.focus(); // Focus input if nothing was sent
            }
        };

        // Microphone Button Event Listener
        micButton.addEventListener('click', () => {
            if (!recognition) return; // Safety check

            if (isRecording) {
                recognition.stop();
                // onend will handle UI changes
            } else {
                // Clear previous text before starting new recording? Optional.
                // userInput.value = '';
                finalTranscript = ''; // Reset transcript
                try {
                    recognition.start();
                    // onstart will handle UI changes
                } catch (e) {
                    // Handle potential errors if start() fails immediately (e.g., mic already in use)
                    console.error("Error starting recognition:", e);
                    addMessage(`Could not start microphone: ${e.message}`, 'error');
                    setStatus('', false); // Reset UI
                }
            }
        });

    } else {
        console.warn("Web Speech Recognition API not supported in this browser.");
        if(micButton) micButton.classList.add('hidden'); // Ensure it stays hidden
    }
    // --- End Speech Recognition Setup ---


    // --- Event Listeners (Send Button and Enter Key) ---
    if (sendButton) {
        sendButton.addEventListener('click', sendMessage);
    } else {
        console.error("Send button not found!");
    }

    if (userInput) {
        userInput.addEventListener('keypress', (event) => {
            if (event.key === 'Enter' && !event.shiftKey && !sendButton.disabled) {
                event.preventDefault();
                sendMessage();
            }
        });
    } else {
        console.error("User input element not found!");
    }

    // Optional: Add initial message
    // addMessage("Agent connected. How can I help?", 'agent'); // Moved to HTML
    userInput?.focus(); // Set focus to input on load
});
```

### File: dashboard/templates/voice/index.html
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <!-- Link to your existing style.css -->
    <link rel="stylesheet" href="style.css">
    <title>Agent Chat Interface</title>
    <!-- Add specific styles for the mic button here or in style.css -->
    <style>
        /* Style for the mic button */
        #micButton {
            padding: 10px;
            min-width: 40px; /* Ensure consistent size */
            height: 40px;
            border: none;
            background-color: #6c757d; /* Grey */
            color: white;
            border-radius: 50%; /* Round */
            cursor: pointer;
            font-size: 1.2em;
            margin-left: 5px; /* Spacing */
            transition: background-color 0.2s ease;
            flex-shrink: 0; /* Prevent shrinking */
        }
        #micButton:hover {
            background-color: #5a6268;
        }
        #micButton.recording {
            background-color: #dc3545; /* Red when recording */
            animation: pulse 1.5s infinite ease-in-out;
        }
        /* Hide mic button if API not supported */
        #micButton.hidden {
            display: none;
        }
        /* Basic pulse animation */
        @keyframes pulse {
            0% { box-shadow: 0 0 0 0 rgba(220, 53, 69, 0.7); }
            70% { box-shadow: 0 0 0 10px rgba(220, 53, 69, 0); }
            100% { box-shadow: 0 0 0 0 rgba(220, 53, 69, 0); }
        }
        /* Minor adjustment for input area */
        .input-area {
            display: flex;
            align-items: center; /* Vertically align items */
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Agent Server</h1>
        <div id="chatbox">
            <!-- Chat messages appear here -->
            <div class="message agent">Welcome! Ask me anything.</div>
        </div>
        <!-- Status Area (Optional but helpful) -->
        <div id="status" style="text-align: center; margin-bottom: 5px; min-height: 1.2em; font-style: italic; color: #666;"></div>
        <div class="input-area">
            <input type="text" id="userInput" placeholder="Type your message...">
            <!-- Microphone Button Added -->
            <button id="micButton" class="hidden" title="Start/Stop Recording">🎤</button>
            <button id="sendButton" title="Send Message">Send</button>
        </div>
    </div>

    <!-- Link to your existing app.js -->
    <script src="app.js"></script>
</body>
</html>
```

### File: dashboard/templates/voice/style.css
```css
body {
    font-family: sans-serif;
    margin: 0;
    background-color: #f4f4f4;
    display: flex;
    justify-content: center;
    align-items: center;
    min-height: 100vh;
}

.container {
    background-color: #fff;
    padding: 20px;
    border-radius: 8px;
    box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
    width: 80%;
    max-width: 600px;
    display: flex;
    flex-direction: column;
    height: 80vh; /* Limit height */
}

h1 {
    text-align: center;
    color: #333;
    margin-top: 0;
}

#chatbox {
    flex-grow: 1; /* Takes available space */
    border: 1px solid #ccc;
    padding: 10px;
    margin-bottom: 15px;
    overflow-y: auto; /* Enable scrolling */
    background-color: #f9f9f9;
    border-radius: 4px;
}

.message {
    margin-bottom: 10px;
    padding: 8px 12px;
    border-radius: 5px;
    line-height: 1.4;
}

.user {
    background-color: #d1e7fd;
    text-align: right;
    margin-left: auto; /* Push to right */
    max-width: 80%;
}

.agent {
    background-color: #e2e3e5;
    text-align: left;
    margin-right: auto; /* Push to left */
    max-width: 80%;
}

.error {
    background-color: #f8d7da;
    color: #721c24;
    text-align: left;
    max-width: 80%;
}


.input-area {
    display: flex;
}

#userInput {
    flex-grow: 1;
    padding: 10px;
    border: 1px solid #ccc;
    border-radius: 4px 0 0 4px;
}

#sendButton {
    padding: 10px 15px;
    background-color: #007bff;
    color: white;
    border: none;
    border-radius: 0 4px 4px 0;
    cursor: pointer;
    transition: background-color 0.2s;
}

#sendButton:hover {
    background-color: #0056b3;
}
```

### File: dev_run.sh
```bash
#!/bin/bash

# Simple development script for agentttt backend and frontend

# --- Configuration ---
BACKEND_EXE="./agent-server"
FRONTEND_DIR="./dashboard"
FRONTEND_PORT="8000" # Port for the simple Python HTTP server
BACKEND_PID_FILE=".backend.pid"
FRONTEND_PID_FILE=".frontend.pid"

# Exit on any error
set -e

# --- Functions ---

# Function to build the backend
build_backend() {
    echo "--- Building Backend (make agent-server) ---"
    if make agent-server; then
        echo "--- Backend build successful ---"
    else
        echo "--- ERROR: Backend build failed! ---"
        exit 1
    fi
}

# Function to start the backend server
start_backend() {
    if [ -f "$BACKEND_PID_FILE" ]; then
        echo "Backend PID file found ($BACKEND_PID_FILE). Already running or needs cleanup?"
        echo "Run '$0 stop' first if needed."
        # Optionally: Check if PID is actually running: kill -0 $(cat $BACKEND_PID_FILE) 2>/dev/null
        return 1 # Indicate potential issue
    fi

    if [ ! -f "$BACKEND_EXE" ]; then
        echo "Backend executable '$BACKEND_EXE' not found. Building first..."
        build_backend
    fi

    # Source environment variables from .env file
    if [ -f ".env" ]; then
        echo "Loading environment variables from .env"
        export $(grep -v '^#' .env | xargs)
    else
        echo "Warning: .env file not found."
    fi

    # Check specifically for GEMINI_API_KEY
    if [ -z "$GEMINI_API_KEY" ] || [[ "$GEMINI_API_KEY" == "YOUR_API_KEY_HERE" ]]; then
        echo "--- ERROR: GEMINI_API_KEY not set correctly in environment or .env file! ---"
        exit 1
    fi


    echo "--- Starting Backend Server ($BACKEND_EXE) ---"
    # Run in background, redirect stdout/stderr to files, store PID
    "$BACKEND_EXE" > backend.log 2>&1 &
    BACKEND_PID=$!
    echo $BACKEND_PID > "$BACKEND_PID_FILE"
    echo "Backend PID: $BACKEND_PID (Logs: backend.log)"
    sleep 1 # Give it a moment to start or fail
    if ! kill -0 $BACKEND_PID 2>/dev/null; then
         echo "--- ERROR: Backend failed to start. Check backend.log ---"
         rm -f "$BACKEND_PID_FILE"
         exit 1
    fi
}

# Function to start the frontend HTTP server
start_frontend() {
     if [ -f "$FRONTEND_PID_FILE" ]; then
        echo "Frontend PID file found ($FRONTEND_PID_FILE). Already running or needs cleanup?"
        echo "Run '$0 stop' first if needed."
        return 1
    fi

    if [ ! -d "$FRONTEND_DIR" ]; then
        echo "--- ERROR: Frontend directory '$FRONTEND_DIR' not found! ---"
        exit 1
    fi

    echo "--- Starting Frontend HTTP Server (Python http.server on port $FRONTEND_PORT) ---"
    # Run in background within the specified directory, redirect output, store PID
    python -m http.server "$FRONTEND_PORT" --directory "$FRONTEND_DIR" > frontend.log 2>&1 &
    FRONTEND_PID=$!
    echo $FRONTEND_PID > "$FRONTEND_PID_FILE"
    echo "Frontend PID: $FRONTEND_PID (Logs: frontend.log)"
    echo "Access Frontend at: http://localhost:$FRONTEND_PORT"
    sleep 1
     if ! kill -0 $FRONTEND_PID 2>/dev/null; then
         echo "--- ERROR: Frontend server failed to start. Check frontend.log ---"
         rm -f "$FRONTEND_PID_FILE"
         exit 1
    fi
}

# Function to stop running servers
stop_servers() {
    echo "--- Stopping Servers ---"
    if [ -f "$BACKEND_PID_FILE" ]; then
        PID=$(cat "$BACKEND_PID_FILE")
        echo "Stopping Backend (PID: $PID)..."
        # Check if process exists before killing
        if kill -0 $PID 2>/dev/null; then
            kill $PID
            sleep 1 # Give it time to shut down
            # Force kill if still running (optional)
            # if kill -0 $PID 2>/dev/null; then kill -9 $PID; fi
        else
            echo "Backend process $PID not found."
        fi
        rm -f "$BACKEND_PID_FILE"
    else
        echo "Backend PID file not found. Server might not be running."
    fi

    if [ -f "$FRONTEND_PID_FILE" ]; then
        PID=$(cat "$FRONTEND_PID_FILE")
        echo "Stopping Frontend (PID: $PID)..."
        if kill -0 $PID 2>/dev/null; then
            kill $PID
        else
            echo "Frontend process $PID not found."
        fi
        rm -f "$FRONTEND_PID_FILE"
    else
        echo "Frontend PID file not found. Server might not be running."
    fi
    echo "--- Stop sequence complete ---"
}

# Function to display logs
show_logs() {
    echo "--- Tailing Logs (Press Ctrl+C to stop) ---"
    # Tail both logs concurrently
    tail -f backend.log frontend.log
}


# --- Main Script Logic ---

COMMAND=$1

case $COMMAND in
    start)
        start_backend || echo "Backend failed to start properly, skipping frontend."
        start_frontend || echo "Frontend failed to start properly."
        echo "--- Services started (check logs for details) ---"
        ;;
    stop)
        stop_servers
        ;;
    build)
        build_backend
        ;;
    restart)
        stop_servers
        echo # Add a newline for clarity
        start_backend || echo "Backend failed to start properly, skipping frontend."
        start_frontend || echo "Frontend failed to start properly."
        echo "--- Services restarted (check logs for details) ---"
        ;;
    logs)
        show_logs
        ;;
    *)
        echo "Usage: $0 {start|stop|build|restart|logs}"
        echo "  start   : Build (if needed) and start backend & frontend servers."
        echo "  stop    : Stop running servers."
        echo "  build   : Build the backend server."
        echo "  restart : Stop, then start servers."
        echo "  logs    : Tail the logs of both servers."
        exit 1
        ;;
esac

exit 0
```

### File: docker-compose.yml
```yaml

services:
  backend:
    build:
      context: .
      dockerfile: Dockerfile.backend
    container_name: agent-backend
    restart: unless-stopped
    ports:
      # Map host port 7777 to container port 7777
      - "7777:7777"
    environment:
      # Pass the API key from the .env file
      - GEMINI_API_KEY=${GEMINI_API_KEY}
      # Define the workspace path inside the container
      - AGENT_WORKSPACE=/app/agent_workspace
    volumes:
      # Mount the local agent_workspace into the container
      # This allows the agent to read/write persistent files
      - ./agent_workspace:/app/agent_workspace
      # Mount prompts read-only (optional, useful if you change prompts often)
      - ./prompts:/app/prompts:ro
    # Run as the non-root user created in the Dockerfile
    user: agentuser
    networks:
      - agent-net

  frontend:
    build:
      context: .
      dockerfile: Dockerfile.frontend
    container_name: agent-frontend
    restart: unless-stopped
    ports:
      # Map host port 8000 to container port 80 (Nginx default)
      - "8000:80"
    depends_on:
      - backend # Start backend first (doesn't guarantee readiness)
    networks:
      - agent-net


networks:
  agent-net:
    driver: bridge
```

### File: Dockerfile
```

# TO-DO:
```

### File: Dockerfile.backend
```
# Dockerfile.backend

# ---- Build Stage ----
FROM debian:bookworm as builder

# Install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    clang++ \
    make \
    libcurl4-openssl-dev \
    libjsoncpp-dev \
    git \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy only necessary files for building
COPY Makefile ./Makefile
COPY main.cpp ./main.cpp

# If needed by any build step or runtime script
COPY client.py ./client.py 
# If needed by any build step or runtime script
COPY save.sh ./save.sh     

COPY server/server.cpp ./server/server.cpp
COPY src/ ./src/
COPY inc/ ./inc/
COPY externals/ ./externals/
COPY server/vendor/ ./server/vendor/
COPY prompts/ ./prompts/

# Build the server executable (ensure Makefile targets 'agent-server')
# Adjust target if Makefile uses a different name
RUN make agent-server

# ---- Final Stage ----
FROM debian:bookworm-slim

# Install runtime dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    libcurl4 \
    libjsoncpp25 \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Create a non-root user for security
RUN useradd --create-home --shell /bin/bash agentuser
USER agentuser

# Copy the compiled binary from the build stage
COPY --from=builder --chown=agentuser:agentuser /app/agent-server ./agent-server

# Copy prompts directory (if needed at runtime by the agent)
COPY --from=builder --chown=agentuser:agentuser /app/prompts/ ./prompts/

# Create agent workspace directory and set permissions
# The actual content will be mounted via docker-compose
RUN mkdir -p /app/agent_workspace
# Permissions adjusted here if needed, but volume mount overrides host permissions mostly

# Expose the port the server listens on
EXPOSE 7777

# Set the entrypoint to run the server
ENTRYPOINT ["./agent-server"]
```

### File: Dockerfile.frontend
```
# Dockerfile.frontend

# Use a lightweight Nginx image
FROM nginx:stable-alpine

# Remove default nginx website
RUN rm -rf /usr/share/nginx/html/*

# Copy the dashboard files to the nginx html directory
COPY ./dashboard /usr/share/nginx/html

# Expose port 80 (default nginx port)
EXPOSE 80

# Default command starts nginx
CMD ["nginx", "-g", "daemon off;"]
```

### File: docs/agent-test.yml
```yaml
# --- Orchestration Schema: MK1 ---
version: MK1
name: "schema-building-workflow"
description: "Workflow demonstrating imports, definitions, settings, and inline agent/tool configurations. Pre/post steps removed for this draft."

import:
  # --- Import Section ---
  # Allows including external resources. Execution order/priority TBD.
  workflows:
    # - ./common_error_handler.yaml # FUTURE: Load entire workflows
  agents:
    # Load agent profiles from YAML files or directories
    - ./agents/response_formatter.yaml # Load a specific agent profile
    - ./agents/data_validators/ # FUTURE: Load all *.yaml from a directory?
    # - name: explicit_formatter (will be inferend from parased and loaded agent file.) or just simply ovveride the name
    #   path: ./agents/response_formatter.yaml
  defines:
    # - ./standard_steps.yaml # FUTURE: Import definitions from other files
  # tools: # FUTURE: Import tool definitions?
    # - ./common_tools.yaml

# --- Initial State or Entrypoint ---
# Option A: Define initial variables
initial_state:
  query: "Find articles about AI in healthcare from the last month."
  search_result_count: 3
  user_id: "user123"

# Option B: Define an entrypoint step (could override initial_state or work with it)
# entrypoint: step_id_start_here # ID of the first step to execute

# --- Workflow Settings ---
settings:
  # General settings affecting workflow execution
  default_timeout: "60s" # Default timeout for steps unless overridden
  max_concurrent_steps: 5 # Limit for parallel execution (FUTURE)
  state_persistence: "none" # "none", "local_file", "database" (FUTURE)
  error_handling_strategy: "fail_fast" # "fail_fast", "continue", "jump_to_error_handler" (FUTURE)
  # default_error_handler_step: "global_error_handler" # FUTURE: ID of a step to jump to on unhandled errors
  debug_level: INFO

# --- Definitions ---
# Define reusable components (agents, tools, steps) without executing them immediately.
# These can be referenced later in the main workflow.
defines:
  agents:
    StandardSummarizer: # Reusable agent definition by name
      name: "StdSummarizer_Defined"
      system_prompt: "Provide a concise summary of the input text."
      iteration_cap: 3
      # tools: ... # Can include tools here too
  tools:
    BashExecutor: # Reusable tool definition
      name: "bash"
      description: "Executes a bash command."
      type: "code" # Indicate it's code-based
      runtime: "bash" # Specify runtime
      code: |
        #!/bin/bash
        eval "$1" # Example: takes command as first arg (Needs refinement for safety!)
      # parameters: ... # Define expected input parameters
  steps:
    LogStartStep: # Reusable step definition
      id: LogStartStep # Note: ID here is for reference within defines, might be different when used
      type: transform # Example: could be logging, data validation etc.
      description: "Log the start of the workflow with the query."
      expression: "'Workflow starting for query: ' + {state.query}"
      output: state.log.start_message # Example: Log to a specific part of the state
    # ... other reusable steps

# --- Execution Rules ---
rules:
  execution_mode: "explicit" # "auto" (start from first step), "explicit" (requires entrypoint or external trigger)
  sink: # stdout, stderr, curl(could be sending to an API), client (I mainly want to have a term window running  live client that can received from agents and workflows, a pannel where agents stream their output basically.
  # FUTURE: Could add rules for depends_on only, parallel steps state access, retries, etc.

# --- Main Workflow Steps ---
workflow:
  # Example using a defined step
  - ref: LogStartStep # Reference a step defined in the 'defines' section
    id: start_logging # Give it a unique ID for this specific use in the workflow

  - id: execute_search_query
    type: agent # Specify the step type
    description: "Execute a refined web search using an inline agent and tools."
    timeout: "90s" # Override default timeout
    depends_on: # FUTURE: Define dependencies (step IDs) for DAG execution
      - start_logging # This step won't start until 'start_logging' completes
    agent: # Structured agent key
      define: # Defining the agent inline for this step
        name: "WebSearcher_Inline"
        system_prompt: |
          You are a web search assistant. Execute the search query precisely.
          Query: {input.search_query}
          Max Results: {input.max_results}
          Return URLs and snippets. Use available tools if needed.
        tools: # Define tools available ONLY to this inline agent
          define: # Define a tool inline ONLY for this agent
            - name: "URLValidator"
              description: "Checks if a URL is reachable."
              type: "code" # Example: Could be code or reference an external API/service
              runtime: "python" # Specify runtime
              parameters:
                url: "string"
              code: | # Example inline python code
                import requests
                def validate(url):
                  try:
                    response = requests.head(url, timeout=5)
                    return response.status_code < 400
                  except requests.RequestException:
                    return False
                # Entry point assumption: execute function named 'run' or 'execute'
                def run(params):
                  is_valid = validate(params.get('url', ''))
                  return {"valid": is_valid}
          internal: # List internal tools the agent might be aware of (if registered globally/via import)
            - "bash" # Assumes a 'bash' tool is available (e.g., from defines)
          external: # List external tools/APIs (more conceptual unless specific integration exists)
            - "google_search_api" # Agent might know how to format calls for this

    input: # Define input for this step (potentially using state from previous steps)
      search_query: "{state.query}" # Using initial state directly for this example
      max_results: "{state.search_result_count}"
    output: state.search_results # Output from this agent step

  - id: summarize_results
    type: agent
    description: "Summarize the search results using a defined agent."
    agent:
      ref: StandardSummarizer # Reference an agent defined in the 'defines' section
    input: "{state.search_results}"
    output: state.summary

  - id: present_results
    type: transform # Example: a step that formats output without an LLM agent
    description: "Format final results combining search and summary."
    depends_on:
      - execute_search_query
      - summarize_results
    expression: "'Found Results (' + len({state.search_results}) + '):\\n' + {state.summary}" # Example expression (needs evaluator)
    output: final_output # Special key indicating the final result of the workflow
```

### File: docs/backend-guide.md
```markdown
---

## The Comprehensive Guide to C++ Backend Servers, HTTPS, and CORS

This guide covers the essential concepts and practices for building robust and secure C++ backend servers, particularly focusing on handling web requests over HTTP/HTTPS and managing Cross-Origin Resource Sharing (CORS).

**1. Fundamentals of HTTP and HTTPS**

*   **HTTP (Hypertext Transfer Protocol):**
    *   The foundation of data communication for the World Wide Web.
    *   A **client-server protocol**: A client (like a web browser or your `client.py`) sends a request to a server, and the server sends back a response.
    *   **Stateless:** Each request/response pair is independent by default. State (like user sessions) needs to be managed explicitly (e.g., using cookies, tokens).
    *   **Request Structure:** Typically includes:
        *   **Method:** The action to perform (e.g., `GET`, `POST`, `PUT`, `DELETE`, `OPTIONS`).
        *   **Path:** The specific resource being requested (e.g., `/prompt`, `/users/123`).
        *   **HTTP Version:** (e.g., `HTTP/1.1`, `HTTP/2`).
        *   **Headers:** Key-value pairs providing metadata (e.g., `Host`, `Content-Type`, `Authorization`, `Accept`).
        *   **Body (Optional):** Data sent with the request (e.g., JSON payload in a `POST`).
    *   **Response Structure:** Typically includes:
        *   **HTTP Version:**
        *   **Status Code:** A 3-digit number indicating the outcome (e.g., `200 OK`, `404 Not Found`, `500 Internal Server Error`).
        *   **Status Message:** A short text description of the status code (e.g., "OK", "Not Found").
        *   **Headers:** Key-value pairs providing metadata about the response (e.g., `Content-Type`, `Content-Length`, `Access-Control-Allow-Origin`).
        *   **Body (Optional):** The actual content returned (e.g., HTML, JSON, image data).

*   **HTTPS (HTTP Secure):**
    *   Not a separate protocol, but **HTTP layered over TLS/SSL** (Transport Layer Security / Secure Sockets Layer).
    *   **Adds Security:**
        *   **Encryption:** Protects the confidentiality of data exchanged between client and server. Prevents eavesdropping.
        *   **Authentication:** Allows the client to verify the server's identity using its SSL/TLS Certificate. Prevents man-in-the-middle attacks.
        *   **Integrity:** Ensures data hasn't been tampered with during transit.
    *   **Requires Certificates:** Servers need an SSL/TLS certificate issued by a trusted Certificate Authority (CA) or a self-signed certificate (for testing, generates browser warnings).
    *   **Standard Port:** 443 (whereas HTTP uses port 80).

**2. Building Backend Servers in C++**

While languages like Node.js, Python, Go, and Java are often more common for web backends due to extensive ecosystems, C++ is perfectly capable and offers potential performance advantages, especially for CPU-intensive tasks.

*   **Core Server Loop:** The fundamental logic involves:
    1.  **Socket Creation:** Creating a network socket.
    2.  **Binding:** Associating the socket with a specific IP address and port on the server.
    3.  **Listening:** Telling the socket to listen for incoming client connections.
    4.  **Accepting:** Accepting a connection request from a client, creating a new socket for that specific connection.
    5.  **Request Handling:** Reading the HTTP request data from the client socket.
    6.  **Parsing:** Interpreting the request (method, path, headers, body).
    7.  **Routing & Logic:** Determining which function/handler should process the request based on method and path. Executing application logic (e.g., calling your Agent).
    8.  **Response Generation:** Creating the HTTP response (status code, headers, body).
    9.  **Sending Response:** Writing the response data back to the client socket.
    10. **Closing/Keeping Alive:** Closing the client socket or keeping it open for subsequent requests (HTTP Keep-Alive).
    11. **Looping:** Returning to the Accept step to handle the next client.

*   **Concurrency:** A real-world server needs to handle multiple clients simultaneously. Common approaches:
    *   **Multithreading:** Create a new thread (or use a thread pool) to handle each accepted client connection. Simpler to reason about initially but can have overhead and require careful synchronization.
    *   **Asynchronous I/O (Event Loop):** Use mechanisms like `select`, `poll`, `epoll` (Linux), `kqueue` (BSD/macOS), or `IOCP` (Windows) to monitor multiple sockets for readiness without blocking. A single thread (or a few threads) can manage many connections efficiently. This is generally more scalable but often more complex to program directly.

*   **Libraries (The Key to Productivity):** Writing all the above from scratch using raw socket APIs is tedious and error-prone. Leverage libraries:
    *   **`httplib.h` (Your Current Choice):**
        *   *Pros:* Simple, header-only, easy to integrate, includes both client and server, supports SSL.
        *   *Cons:* Less feature-rich than larger frameworks, basic threading model (thread-per-connection or thread pool). Good for simpler projects or embedding.
    *   **Boost.Asio:**
        *   *Pros:* Very powerful, highly flexible, excellent for high-performance asynchronous networking. Part of the widely respected Boost libraries. Cross-platform.
        *   *Cons:* Steeper learning curve, more boilerplate code required compared to simpler libraries. It's a networking library, not a full web framework (you build HTTP handling on top).
    *   **Poco C++ Libraries:**
        *   *Pros:* Mature, comprehensive set of libraries including networking (HTTP client/server with HTTPS), threading, configuration, databases, XML, JSON, etc. A full application framework.
        *   *Cons:* Larger dependency, potentially more overhead than a minimal library.
    *   **Crow (`CrowCpp/Crow`)**:
        *   *Pros:* Modern C++, header-only (like httplib), focuses specifically on building web APIs/microservices, inspired by Python's Flask (easy routing). Supports middleware.
        *   *Cons:* Might be less mature or have fewer features than Boost/Poco in some areas outside core HTTP handling. Often relies on Asio internally.
    *   **Pistache (`pistacheio/pistache`)**:
        *   *Pros:* Modern C++, focused on performance, REST API framework, asynchronous using its own event loop abstraction.
        *   *Cons:* Can be complex, potentially smaller community than Boost/Poco.

**3. Implementing HTTPS in Your C++ Server**

1.  **Obtain Certificate & Key:**
    *   **Let's Encrypt (Recommended for Production):** Use `certbot` or similar ACME clients to get free, auto-renewing certificates for your domain.
    *   **Commercial CA:** Purchase a certificate.
    *   **Self-Signed (Testing/Local Dev Only):** Use `openssl` command-line tools:
        ```bash
        # Generate private key
        openssl genpkey -algorithm RSA -out server.key
        # Generate Certificate Signing Request (CSR)
        openssl req -new -key server.key -out server.csr
        # Generate self-signed certificate valid for 365 days
        openssl x509 -req -days 365 -in server.csr -signkey server.key -out server.crt
        ```
        *Remember browsers will distrust these.*

2.  **Use Library's SSL/TLS Capabilities:** Your chosen HTTP library must support SSL/TLS, usually via OpenSSL integration.
    *   **With `httplib.h`:**
        *   Include the header normally.
        *   Ensure your project is compiled and linked against OpenSSL libraries (`-lssl -lcrypto`).
        *   Instantiate `httplib::SSLServer` instead of `httplib::Server`, passing the paths to your certificate (`.crt`) and private key (`.key`) files to the constructor.
        *   Listen on an appropriate port (usually 443).
        ```c++
        #include "httplib.h"
        #include <iostream>

        int main() {
            #ifdef CPPHTTPLIB_OPENSSL_SUPPORT
            httplib::SSLServer svr("./server.crt", "./server.key");

            if (!svr.is_valid()) {
                std::cerr << "SSL Server setup failed. Check cert/key paths and OpenSSL linkage." << std::endl;
                // You might want to print OpenSSL errors here: ERR_print_errors_fp(stderr);
                return 1;
            }

            svr.Get("/", [](const httplib::Request &, httplib::Response &res) {
                res.set_content("Hello Secure World!", "text/plain");
            });

            std::cout << "HTTPS server listening on port 8080..." << std::endl;
            svr.listen("0.0.0.0", 8080); // Use 443 for standard HTTPS

            #else
            std::cerr << "HTTPS not supported: Compile with OpenSSL enabled." << std::endl;
            return 1;
            #endif

            return 0;
        }
        ```
    *   **With Boost.Asio/Poco/etc.:** These libraries have specific classes and methods for creating SSL contexts (`ssl::context`) and SSL streams/sockets. You'll load the certificate and key into the context and then use SSL-aware stream objects. Consult their respective documentation.

**4. Understanding CORS (Cross-Origin Resource Sharing)**

*   **The "Why": Same-Origin Policy (SOP)** Browsers block scripts on `https://frontend.com` from making `fetch` or `XMLHttpRequest` calls to `https://api.backend.com` by default. This prevents malicious scripts on one site from stealing data from another site the user might be logged into. "Origin" = Protocol + Domain + Port.
*   **The "What": CORS Headers:** CORS is a mechanism where the *server* uses specific HTTP headers to tell the browser that requests *from* certain other origins *are* allowed. It's the server granting permission.
*   **The "How":**
    *   **Simple Requests:** Some requests (e.g., `GET`, `HEAD`, some `POST` with specific `Content-Type`s) are considered "simple". The browser makes the request directly, and the server *must* include `Access-Control-Allow-Origin` in the response for the browser to allow the frontend script access to it.
    *   **Preflighted Requests (`OPTIONS`):** Most API calls (e.g., `POST` with `Content-Type: application/json`, `PUT`, `DELETE`, requests with custom headers like `Authorization`) require a "preflight" check.
        1.  The browser *first* sends an `OPTIONS` request to the target URL.
        2.  The server responds to the `OPTIONS` request with specific CORS headers indicating *if* the actual request would be allowed.
        3.  If the `OPTIONS` response is permissive enough, the browser then sends the *actual* request (e.g., `POST`).
        4.  The server *still* needs to include `Access-Control-Allow-Origin` in the response to the *actual* request.
*   **Essential Server Response Headers:**
    *   `Access-Control-Allow-Origin: <origin>` or `*`:
        *   Specifies which frontend origin(s) are allowed.
        *   `<origin>`: e.g., `https://your-frontend.com`. **Best practice for security.** Can only be *one* specific origin or `*`. Cannot be a list. Your server logic needs to check the request's `Origin` header and echo it back if allowed, or send `*`.
        *   `*`: Allows *any* origin. **Use with caution.** Incompatible with `Access-Control-Allow-Credentials: true`.
        *   *Needed on responses to both `OPTIONS` and the actual request.*
    *   `Access-Control-Allow-Methods: GET, POST, OPTIONS, PUT`: (For `OPTIONS` response) Lists the HTTP methods allowed for the actual request.
    *   `Access-Control-Allow-Headers: Content-Type, Authorization`: (For `OPTIONS` response) Lists the HTTP headers the frontend is allowed to include in the actual request.
    *   `Access-Control-Allow-Credentials: true`: (Optional) Allows requests with cookies or HTTP authentication to be made. Requires `Access-Control-Allow-Origin` to be a *specific* origin, not `*`.
    *   `Access-Control-Max-Age: 86400`: (Optional, for `OPTIONS` response) Tells the browser how long (in seconds) it can cache the preflight response, avoiding repeated `OPTIONS` checks.

*   **Implementation (Conceptual `httplib.h`):**
    ```c++
    // In your main setup:
    const std::string ALLOWED_ORIGIN = "https://your-frontend.com"; // Or read from config

    // Preflight handler
    svr.Options("/api/.*", [&](const httplib::Request &req, httplib::Response &res) {
        std::string origin = req.get_header_value("Origin");
        if (origin == ALLOWED_ORIGIN) { // Check if origin is allowed
             res.set_header("Access-Control-Allow-Origin", ALLOWED_ORIGIN);
             res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
             res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
             // res.set_header("Access-Control-Allow-Credentials", "true"); // If needed
             // res.set_header("Access-Control-Max-Age", "86400"); // Optional caching
        } else {
            // Optionally deny or just don't send CORS headers
        }
         res.status = 204; // No Content for OPTIONS success
    });

    // Actual request handler
    svr.Post("/api/prompt", [&](const httplib::Request &req, httplib::Response &res) {
        std::string origin = req.get_header_value("Origin");
         if (origin == ALLOWED_ORIGIN) { // Check origin again
             res.set_header("Access-Control-Allow-Origin", ALLOWED_ORIGIN);
             // res.set_header("Access-Control-Allow-Credentials", "true"); // If needed
         } else {
             // If origin not allowed, should ideally not send CORS header and potentially deny request
             // For simplicity here, we might still process but browser would block frontend access if origin header mismatch
         }

        // ... handle the API request ...
        res.set_content("{\"response\": \"Agent says hi!\"}", "application/json");
        res.status = 200;
    });
    ```

**5. Integrating HTTPS and CORS**

*   They solve different problems: HTTPS encrypts the connection; CORS manages browser cross-origin permissions.
*   You need **both** if your frontend is served securely (HTTPS) from a different origin than your backend API (which should *also* be HTTPS).
*   The CORS configuration (headers your server sends) remains largely the same whether you are using HTTP or HTTPS. The key is that the `Access-Control-Allow-Origin` header must match the *exact* origin the browser sees for the frontend, including the `https://`.

**6. Best Practices & Security Considerations**

*   **HTTPS Everywhere:** Always use HTTPS for production APIs. Obtain certificates properly (Let's Encrypt). Redirect HTTP traffic to HTTPS. Enable HSTS (HTTP Strict Transport Security) header.
*   **Input Validation:** *Never* trust data from the client (request body, headers, query params, path params). Validate type, length, format, range. Sanitize inputs to prevent injection attacks (SQLi, XSS - though XSS is more about frontend rendering).
*   **Authentication & Authorization:** Secure endpoints that shouldn't be public. Use robust methods like OAuth 2.0 / OpenID Connect with tokens (JWTs). Don't roll your own crypto.
*   **Error Handling:** Return appropriate HTTP status codes. Don't leak sensitive information (stack traces, internal paths) in error messages to the client. Log detailed errors server-side.
*   **Rate Limiting:** Protect against brute-force attacks and denial-of-service by limiting requests per client IP or API key.
*   **Logging:** Implement comprehensive logging (requests, responses, errors, important events) for debugging and security monitoring.
*   **Dependencies:** Keep your C++ libraries (HTTP, OpenSSL, JSON, etc.) updated to patch security vulnerabilities. Use a package manager if possible or track versions carefully.
*   **Secure Headers:** Consider adding headers like `Content-Security-Policy`, `X-Content-Type-Options`, `X-Frame-Options`.
*   **Least Privilege:** Run your server process under a dedicated, non-root user account.
*   **Secrets Management:** Do not hardcode API keys, database passwords, etc. Use environment variables (like you do for `GEMINI_API_KEY`), configuration files (outside version control), or dedicated secrets management systems.
*   **CORS Configuration:** Be specific with `Access-Control-Allow-Origin`. Avoid `*` in production unless absolutely necessary and understand the implications.

**7. Advanced Topics (Beyond the Basics)**

*   **WebSockets:** For real-time, bidirectional communication (e.g., live transcription streaming, chat apps).
*   **API Design:** REST principles, GraphQL, gRPC. Designing clean, consistent, and versioned APIs.
*   **Databases:** Interacting with SQL or NoSQL databases. Using ORMs or database connector libraries. Connection pooling.
*   **Caching:** Improving performance by caching responses (HTTP caching headers, Redis/Memcached).
*   **Deployment:** Containerization (Docker - like you're doing!), cloud platforms (AWS, GCP, Azure), reverse proxies (Nginx, Apache), load balancing.
*   **Monitoring & Alerting:** Tracking performance (latency, error rates), resource usage (CPU, memory), and setting up alerts for problems.

**Conclusion**

Building backend servers in C++ involves understanding HTTP/HTTPS fundamentals, choosing appropriate libraries to handle the networking complexity, implementing robust security measures like HTTPS and proper CORS configuration, and following general software engineering best practices. While the C++ ecosystem might require more manual setup than some other languages, it offers significant performance potential when needed. Start with a good library like `httplib.h` or explore options like Crow or Asio as your needs grow, always prioritizing security and proper error handling. Good luck!
```

### File: docs/bline.md
```markdown

### B-line features
- B-line: having the base compose stack  fully working. backend and frontend (optional).
    - that would need: server to be refactored for the new lib changes.
    - fix the damn exec and parse. for the new schema
    - Implement Sub-agent loading, context an namespace variables/agent state, 
    - start building and testing the new agent profiles.
    - simple reporter live agent so we actually start to utilize the system.
    - Implement the first relic.
    - python/bash runtime.
    - script api.

{
  "id": "run_script",
  "type": "script",
  "params": {"runtime": "python", "source": "print('Hello')"}, // source could be a file path or inline code
  "description": "Runs a Python script to say hi",
  "tags": ["script", "test"],
  "timeout": 300,
  "retries": 3,
  "concurrency": 1,
  "schedule": "1h", // will run every hour. with the beginning of the hour as base
  "notifications": [
    {"event": "completion", "channel": "email", "recipients": ["user@example.com"]},
    {"event": "failure", "channel": "slack", "webhook": "https://slack.com/webhook"}
  ],
  "resource_limits": {
    "cpu": "0.5",
    "memory": "512MB",
    "disk": "1GB"
  },


  {
    status: string (REQUIRED, enum: SUCCESS| REQUIRES_ACTION | FAIL | ERROR),
    thoughts: array<object> (REQUIRED, object: { type: string (REQUIRED, enum: DECISION| LONG_TERM | SHORT_TERM| PLAN | OBSERVATION | QUESTION | NORM | ASSUMPTION | GOAL | HYPOTHESIS | CRITIQUE), content: string (REQUIRED) }),
    actions: array<object> (REQUIRED, object: { action: string (REQUIRED), type: string (REQUIRED, enum: tool | script | http_request | internal_function | output | workflow_control), params: object (REQUIRED, structure depends on action/type), confidence?: float (0.0-1.0), warnings?: array<string> }),
    response: string | null

  }


{
  "action": "run_script",
  "type": "script",
  "params": {"language": "python", "code": "print('Hello')"},
  "description": "Runs a Python script to say hi",
  "tags": ["script", "test"],
  "priority": 3,
  "dependencies": ["setup_env"],
  "version": "1.0.0",
  "timeout": 300,
  "retries": 3,
  "concurrency": 1,
  "schedule": "cron(0 0 * * *)",
  "notifications": [
    {"event": "completion", "channel": "email", "recipients": ["user@example.com"]},
    {"event": "failure", "channel": "slack", "webhook": "https://slack.com/webhook"}
  ],
  "env_vars": {"DEBUG": "true"},
  "input_schema": {
    "type": "object",
    "properties": {"language": {"type": "string"}, "code": {"type": "string"}},
    "required": ["language", "code"]
  },
  "output_schema": {
    "type": "object",
    "properties": {"result": {"type": "string"}},
    "required": ["result"]
  },
  "log_level": "DEBUG",
  "context": {
    "type": "docker",
    "image": "python:3.9",
    "options": {"network": "host"}
  },
  "preconditions": [
    {"type": "file_exists", "path": "/tmp/setup.done"}
  ],
  "postconditions": [
    {"type": "file_exists", "path": "/tmp/output.txt"}
  ],
  "failure_policy": {
    "on_failure": "rollback",
    "rollback_action": "cleanup_script"
  },
  "metadata": {
    "owner": "team-alpha",
    "created": "2023-10-01"
  },
  "rate_limit": {
    "max_runs": 10,
    "per": "hour"
  },
  "secrets": {
    "api_token": "vault://secrets/api/token"
  }
}

```

### File: docs/cors.md
```markdown

**1. CORS (Cross-Origin Resource Sharing)**

*   **The Problem: Same-Origin Policy (SOP)**
    *   Web browsers enforce a security rule called the Same-Origin Policy. By default, a web page loaded from one "origin" (defined by protocol (http/https), domain, and port) cannot make requests to a *different* origin.
    *   In your setup:
        *   The **Frontend** (dashboard) is likely served from somewhere like `http://localhost:8000` (during local development) or `https://test.clevo.ddnsgeek.com` (as configured in your server).
        *   The **Backend** (your C++ agent server) is running at `http://<server-ip>:7777`.
    *   Since the origin of the frontend (e.g., `https://test.clevo.ddnsgeek.com:443` - HTTPS default port is 443) is different from the origin of the backend (`http://<server-ip>:7777`), the browser will block the frontend's JavaScript (`app.js`) from directly calling the `/prompt` endpoint on the backend *unless* the backend explicitly permits it.

*   **The Solution: CORS Headers**
    *   CORS is a mechanism using HTTP headers that tells the browser it's okay for a frontend from one origin to access resources on a backend from a different origin.
    *   Your server needs to send specific `Access-Control-*` headers in its responses.

*   **How Your Code Implements CORS:**
    *   **Preflight Request (`OPTIONS`):** For requests that are considered "non-simple" (like your `POST` request with `Content-Type: application/json`), the browser first sends an `OPTIONS` request to the server endpoint (`/prompt`). This is called a "preflight" request. It asks the server "Hey, is it okay if a script from origin X tries to send a POST request with these headers?".
        *   Your code handles this correctly:
            ```c++
            svr.Options("/prompt", [](const httplib::Request &, httplib::Response &res) {
                // Which frontend origins are allowed?
                res.set_header("Access-Control-Allow-Origin", "https://test.clevo.ddnsgeek.com");
                // Which headers can the frontend send in the *actual* request?
                res.set_header("Access-Control-Allow-Headers", "Content-Type");
                // Which methods are allowed for the *actual* request?
                res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
                res.status = 204; // Standard success for preflight
                serverLog("REQUEST", "OPTIONS /prompt (CORS preflight)");
            });
            ```
        *   The browser checks this `OPTIONS` response. If the origin, method, and headers match what the frontend wants to do, it proceeds to the actual request. Otherwise, it blocks the request and shows a CORS error in the console.

    *   **Actual Request (`POST`):** If the preflight is successful (or if the request was "simple" and didn't need a preflight), the browser sends the actual `POST` request. Crucially, the server *still* needs to include the `Access-Control-Allow-Origin` header in the response to this `POST` request as well.
        *   Your code handles this too:
            ```c++
            svr.Post("/prompt", [&](const httplib::Request &req, httplib::Response &res) {
                // MUST include this again in the actual response!
                res.set_header("Access-Control-Allow-Origin", "https://test.clevo.ddnsgeek.com");

                // ... (rest of your POST handler logic) ...

                res.status = 200;
                res.set_content(responseBody, "application/json");
                serverLog("RESPONSE", "Sent 200 OK for /prompt");
            });
            ```

    *   **`Access-Control-Allow-Origin` Value:**
        *   `"https://test.clevo.ddnsgeek.com"`: This is specific and secure. Only frontend code served from exactly this origin can talk to your backend.
        *   `"http://localhost:8000"`: You had this commented out. You'd use this during local development if your frontend dev server runs on port 8000.
        *   `"*"`: (Wildcard) Allows *any* origin. This is generally **not recommended for production** as it disables the SOP protection, but can be useful for quick testing or truly public APIs. Your code currently *doesn't* use the wildcard.

**2. HTTPS (Hypertext Transfer Protocol Secure)**

*   **What it is:** HTTPS is essentially the standard HTTP protocol layered on top of an encryption layer, usually TLS (Transport Layer Security), which is the successor to SSL (Secure Sockets Layer).
*   **What it Provides:**
    *   **Encryption:** Protects the data exchanged between the browser and the server from eavesdropping (confidentiality). Anyone intercepting the traffic only sees scrambled data.
    *   **Authentication:** Allows the browser to verify the identity of the server it's talking to using the server's SSL/TLS certificate. This prevents man-in-the-middle attacks where someone pretends to be your server.
    *   **Integrity:** Ensures that the data hasn't been tampered with during transit.
*   **Why it's Important:** Essential for any communication involving sensitive data (logins, personal info, API keys if sent directly). Modern browsers increasingly push for HTTPS everywhere and flag HTTP sites as "Not Secure".
*   **How Your Code Handles It (Currently It Doesn't):**
    *   Your code uses `httplib::Server svr;`. This creates a standard **HTTP** server.
    *   It listens on port 7777 using plain HTTP.
*   **How to Implement HTTPS with `httplib.h`:**
    1.  **Get Certificates:** You need an SSL/TLS certificate and a corresponding private key.
        *   **Options:**
            *   **Let's Encrypt:** Free, automated, and widely trusted certificates (recommended for production). Requires a domain name and running an ACME client (like Certbot) to obtain and renew certificates.
            *   **Commercial CA:** Purchase certificates from providers like DigiCert, Comodo, etc.
            *   **Self-Signed:** Generate your own certificate. Free and easy for testing/local development, but browsers will show a security warning because it's not trusted by a recognized Certificate Authority (CA). You'd have to manually tell your browser/client to trust it.
        *   **Files:** You typically get two `.pem` files (or similar formats):
            *   `server.crt` (or `fullchain.pem`): The public certificate (and possibly intermediate certificates).
            *   `server.key` (or `privkey.pem`): The private key (keep this secure!).
    2.  **Use `httplib::SSLServer`:** Change your server creation line:
        ```c++
        // Replace this:
        // httplib::Server svr;

        // With this:
        #ifdef CPPHTTPLIB_OPENSSL_SUPPORT // Check if OpenSSL support is compiled in
        // Provide paths to your certificate and key files
        httplib::SSLServer svr("./path/to/your/server.crt", "./path/to/your/server.key");

        // Check if the server was initialized correctly (e.g., files found, valid)
        if (!svr.is_valid()) {
            serverLog("FATAL", "Failed to initialize SSLServer. Check certificate paths/validity.");
            // Include OpenSSL error details if possible: ERR_print_errors_fp(stderr);
            return 1;
        }
        #else
        serverLog("FATAL", "Server compiled without OpenSSL support, cannot run HTTPS.");
        // You might fall back to HTTP or exit
        // httplib::Server svr; // Or just exit
         return 1;
        #endif
        ```
    3.  **Port:** Typically, you would listen on port **443** for HTTPS:
        ```c++
        const int port = 443; // Default HTTPS port
        // ...
        if (!svr.listen(host.c_str(), port)) { /* ... */ }
        ```
        If you use a non-standard port (like 7777) for HTTPS, clients *must* specify it in the URL (e.g., `https://test.clevo.ddnsgeek.com:7777`).
    4.  **Compile with OpenSSL:** The `httplib.h` library needs to be used in a project compiled and linked with OpenSSL development libraries. Your `Makefile` seems to link `-lcurl -ljsoncpp -pthread`. You would need to add flags to link against OpenSSL (like `-lssl -lcrypto`).

**Connecting CORS and HTTPS:**

*   Using HTTPS **does not** remove the need for CORS if the frontend and backend are still on different origins.
*   If your frontend is at `https://test.clevo.ddnsgeek.com` and your backend is at `https://your-backend-domain.com:7777` (note different domain or port), you *still* need the `Access-Control-Allow-Origin` headers from the backend, even though both are HTTPS.
*   The only way to avoid CORS is to serve both the frontend files and the backend API from the *exact same origin* (same protocol, same domain, same port).

**In Summary for Your Code:**

1.  Your server is currently **HTTP**.
2.  It correctly implements **CORS** headers to allow requests specifically from `https://test.clevo.ddnsgeek.com`.
3.  The CORS implementation handles the necessary preflight (`OPTIONS`) request for your frontend's `POST` call.
4.  To switch to **HTTPS**, you need to:
    *   Obtain SSL certificate/key files.
    *   Change `httplib::Server` to `httplib::SSLServer` and provide the file paths.
    *   Ensure your project is compiled and linked with OpenSSL.
    *   Consider using port 443 or ensure the frontend uses the correct port in the HTTPS URL.
5.  Remember that CORS headers will likely still be needed even after switching to HTTPS if the frontend origin differs from the backend origin.
```

### File: docs/docker-master-class.md
```markdown

Below is a concise, concept-focused masterclass outline that weaves key Docker/Compose internals together with real-world analogies, concrete examples, common pitfalls to avoid, and workflow-friendly habits. This structure emphasizes memorable, practical insights rather than hands-on builds, so you can “ground” each concept in intuition and start integrating best practices immediately.

**Summary**
You’ll explore how namespaces & cgroups underpin container isolation and resource control with everyday analogies, how union-fs layering feels like stacking transparent sheets, and how BuildKit’s cache works like a smart librarian. Each section highlights “real” pitfalls—running containers as root, unbounded image growth, stale volumes—and contrasts them with habits such as pinning base-image versions, pruning intermediates, and leveraging `.dockerignore`. By tying theory to things you already know (e.g., process trees, file-system mounts), you’ll build durable mental models and adopt workflow practices that make containerization second nature.

---

## Conceptual Links & Tangible Analogies

* **Namespaces as Private Suites**: Think of each namespace (PID, network, mount, UTS, IPC, user) as a locked suite in a hotel—processes inside see only their own rooms and corridors. Docker wires these suites into a cohesive container environment without touching the rest of the building ([NGINX Community Blog][1]).
* **cgroups as Utility Meters**: Just like apartment utility meters cap water or electricity, cgroups v1/v2 enforce CPU, memory, and I/O limits per container, preventing “noisy neighbors” from hogging resources ([Datadog Security Labs][2]).
* **Union-fs Layers**: Overlay2/aufs layers are like transparent overhead projector sheets stacked atop one another: each sheet holds incremental file changes, but together they form a complete filesystem image ([Docker Documentation][3]).
* **BuildKit Cache as Smart Librarian**: BuildKit’s cache key mechanism fingerprints each Dockerfile step so the “smart librarian” reuses unchanged book editions (layers) instead of fetching new ones, dramatically speeding rebuilds ([Docker Documentation][3]).

## Common Pitfalls & What to Avoid

* **Running as Root**: Default container processes run as root, which is akin to giving someone full admin keys—major security risk. Always drop to a non-root user in your Dockerfile ([Medium][4]).
* **Unoptimized Layers**: Splitting related commands into multiple `RUN` instructions creates extra layers, slowing builds and wasting storage. Combine related steps or use multi-stage builds to minimize layers ([DEV Community][5], [Sysdig][6]).
* **Huge Base Images**: Using a full Ubuntu base when you only need `glibc` is like shipping an entire warehouse for a single part. Prefer minimal bases (e.g., `scratch`, `alpine`) unless your app truly needs more ([Docker Documentation][3]).
* **Stale Volumes & Images**: Neglecting `docker system prune` leads to gigabytes of orphaned data; treat image cleanup like routinely emptying your recycling bin ([Medium][7]).

## Habits & Best Practices to Integrate

* **Pin Base-Image Versions**
  Always specify exact tags (`python:3.11.4-slim` rather than `python:3.11-slim`) so your builds stay reproducible and you control upgrades ([Docker Documentation][3]).
* **Leverage `.dockerignore`**
  Exclude local artifacts (`.git`, docs, tests) from build contexts to reduce upload size and speed up builds—like packing only what you need for a business trip ([Docker Documentation][3]).
* **Embrace Multi-Stage Builds**
  Compile or test in intermediate stages, then copy only final artifacts into the runtime image—yielding lean, single-purpose containers ([Docker Documentation][3]).
* **Integrate Builds into CI**
  Automate your image builds and tests in CI/CD pipelines; treat Dockerfiles like code and lint them (e.g., `hadolint`) to catch anti-patterns early ([Medium][8]).

## Workflow Integration & Practical Tips

* **Compose Versioning & Overrides**
  Keep a `docker-compose.yml` for common services and a `docker-compose.override.yml` for environment-specific tweaks (dev vs. prod). This mirrors your application configuration branching strategy ([Nick Janetakis][9]).
* **Service Discovery Mindset**
  In Compose, service names act as DNS entries—think of your services as hostnames in a private LAN. Use this mental model when architecting multi-container topologies ([Nick Janetakis][9]).
* **Resource Labels & Constraints**
  Tag containers with labels (`com.myorg.project=app`) and apply `deploy.resources.limits` in Compose to enforce predictable behavior on shared hosts ([Nick Janetakis][9]).
* **Regular Audits**
  Schedule periodic reviews of your running images, networks, and volumes—like monthly maintenance checks on your car—to catch drift and waste ([Tech Couch][10]).

## Mastery Strategies & Habit Formation

1. **Analogical Mapping**
   Always map new concepts to familiar domains (e.g., cache = librarian, namespaces = hotel suites). These vivid analogies solidify understanding.
2. **Teach-Back Sessions**
   Explain each Docker internal to a peer or rubber-duck; teaching uncovers gaps and reinforces memory.
3. **Documentation Epics**
   Write mini-guides for your team on each component (BuildKit, overlay2, Compose parser). The act of writing cements knowledge.
4. **Periodic Spaced Review**
   Revisit key internals after 1 day, 1 week, and 1 month—spaced repetition ensures long-term retention.

---

With these conceptual frameworks, pitfalls to avoid, and workflow habits, you’ll internalize Docker & Compose “under the hood” and steadily 10× your container fluency, engineering rigor, and architectural clarity.

[1]: https://blog.nginx.org/blog/what-are-namespaces-cgroups-how-do-they-work?utm_source=chatgpt.com "What Are Namespaces and cgroups, and How Do They Work?"
[2]: https://securitylabs.datadoghq.com/articles/container-security-fundamentals-part-2/?utm_source=chatgpt.com "Container security fundamentals part 2: Isolation & namespaces"
[3]: https://docs.docker.com/build/building/best-practices/?utm_source=chatgpt.com "Building best practices - Docker Docs"
[4]: https://rameshfadatare.medium.com/avoid-these-common-mistakes-in-docker-and-follow-these-amazing-best-practices-ea8e48099ac6?utm_source=chatgpt.com "Avoid These Common Mistakes in Docker and ... - Ramesh Fadatare"
[5]: https://dev.to/idsulik/container-anti-patterns-common-docker-mistakes-and-how-to-avoid-them-4129?utm_source=chatgpt.com "Container Anti-Patterns: Common Docker Mistakes and How to ..."
[6]: https://sysdig.com/learn-cloud-native/dockerfile-best-practices/?utm_source=chatgpt.com "Top 20 Dockerfile best practices - Sysdig"
[7]: https://medium.com/%40titofrezer2018/avoid-these-common-pitfalls-when-setting-up-docker-and-ci-in-your-project-866669662937?utm_source=chatgpt.com "Avoid These Common Pitfalls When Setting Up Docker and CI in ..."
[8]: https://medium.com/%40nile.bits/10-docker-best-practices-every-developer-should-know-1b64aeb259be?utm_source=chatgpt.com "10 Docker Best Practices Every Developer Should Know | by Nile Bits"
[9]: https://nickjanetakis.com/blog/best-practices-around-production-ready-web-apps-with-docker-compose?utm_source=chatgpt.com "Best Practices Around Production Ready Web Apps with Docker ..."
[10]: https://tech-couch.com/post/common-pitfalls-running-docker-in-production?utm_source=chatgpt.com "Common pitfalls running docker in production - Tech Couch"
```

### File: docs/libs-guide.md
```markdown
Okay, let's dive into getting *really* good with `jsoncpp` and `httplib.h` in C++. Achieving "mastery" and a "god workflow" comes down to understanding the tools deeply, applying best practices consistently, handling errors robustly, and knowing how to combine them efficiently.

This guide will cover:

1.  **Mastering `jsoncpp`:** Parsing, Building, Accessing, Modifying, Serializing, Error Handling, Best Practices.
2.  **Mastering `httplib.h`:** Server setup, Routing, Request Handling, Response Generation, Client Usage, HTTPS, Error Handling, Best Practices.
3.  **Achieving Synergy (The "God Workflow"):** Combining them effectively for APIs, configuration, etc.
4.  **Advanced Topics & Considerations:** Performance, Security, Async (Limitations).

---

## 1. Mastering `jsoncpp`

`jsoncpp` is your tool for dealing with JSON data within your C++ application.

**A. Parsing JSON (Reading Data)**

You'll usually get JSON as a string or from a stream (like a file or HTTP response body).

*   **From String:**
    ```c++
    #include <json/json.h>
    #include <string>
    #include <iostream>
    #include <sstream>

    std::string jsonString = R"({ "name": "Agent Smith", "level": 99, "active": true })";
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
    std::string errs;

    std::stringstream ss(jsonString); // Or use string directly with parse bounds
    bool parsingSuccessful = reader->parse(ss, &root, &errs);
    // Alt: bool parsingSuccessful = reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.length(), &root, &errs);

    if (!parsingSuccessful) {
        std::cerr << "JSON parsing failed: " << errs << std::endl;
        // Handle error appropriately
    } else {
        // Proceed with using 'root'
        std::cout << "Parsing successful." << std::endl;
    }
    ```
*   **From Stream (e.g., `std::ifstream`):**
    ```c++
    #include <fstream>
    // ... include jsoncpp headers ...

    std::ifstream jsonFile("config.json");
    Json::Value configRoot;
    Json::CharReaderBuilder readerBuilder;
    std::string errs;
    if (!jsonFile.is_open()) {
        std::cerr << "Error opening config.json" << std::endl;
    } else {
        bool parsingSuccessful = Json::parseFromStream(readerBuilder, jsonFile, &configRoot, &errs);
        if (!parsingSuccessful) {
            std::cerr << "JSON parsing failed: " << errs << std::endl;
        } else {
            std::cout << "Config loaded successfully." << std::endl;
        }
    }
    ```

**B. Building JSON Programmatically**

`Json::Value` is the core. It can hold different JSON types.

```c++
Json::Value data; // Starts as Null

// Assigning basic types
data["agentName"] = "Agent Neo";
data["version"] = 1.0;
data["iterations"] = 15;
data["enabled"] = true;
data["secretKey"] = Json::nullValue; // Explicit Null

// Building an array
Json::Value toolsArray(Json::arrayValue); // Explicitly create an array
toolsArray.append("bash");
toolsArray.append("file");
toolsArray.append("search");
data["availableTools"] = toolsArray;

// Building a nested object
Json::Value apiConfig(Json::objectValue); // Explicitly create an object
apiConfig["endpoint"] = "http://api.example.com";
apiConfig["timeout"] = 30;
data["api"] = apiConfig;

// Adding to existing array/object
data["availableTools"].append("calculator");
data["api"]["retries"] = 3;
```

**C. Accessing JSON Data Safely**

*Never* assume a key exists or has the type you expect.

*   **Check Existence:** `isMember()`
    ```c++
    if (root.isMember("level")) {
        // Safe to access
    } else {
        std::cerr << "Warning: 'level' key not found." << std::endl;
    }
    ```
*   **Check Type:** `isString()`, `isInt()`, `isDouble()`, `isBool()`, `isArray()`, `isObject()`, `isNull()`
    ```c++
    if (root.isMember("active") && root["active"].isBool()) {
        bool isActive = root["active"].asBool();
        std::cout << "Agent is active: " << (isActive ? "Yes" : "No") << std::endl;
    } else {
        std::cerr << "Warning: 'active' key missing or not a boolean." << std::endl;
        // Handle default or error
    }
    ```
*   **Access with Default:** `get(key, defaultValue)`
    ```c++
    // Provides a default if 'level' is missing or not convertible to int
    int level = root.get("level", 0).asInt();
    std::string description = root.get("description", "No description provided").asString();
    ```
    *Note:* `get()` still returns a `Json::Value`. You *must* still check the type or use `asXxx()` carefully if the default value's type might not match the actual data's type.
*   **Direct Access (Use Cautiously):** `operator[]`
    *   For objects: `root["name"]`. If "name" doesn't exist, it *creates* a null member! Often undesirable for reading. Better to use `isMember` or `get`.
    *   For arrays: `root["availableTools"][0]`. Throws `Json::LogicError` if index is out of bounds or if it's not an array.
*   **Iterating:**
    ```c++
    // Array
    if (data.isMember("availableTools") && data["availableTools"].isArray()) {
        const Json::Value& tools = data["availableTools"]; // Use const&
        std::cout << "Tools:" << std::endl;
        for (const auto& tool : tools) { // Range-based for loop
            if (tool.isString()) {
                std::cout << "- " << tool.asString() << std::endl;
            }
        }
        // Or index-based:
        // for (Json::ArrayIndex i = 0; i < tools.size(); ++i) { ... tools[i] ... }
    }

    // Object
    if (data.isMember("api") && data["api"].isObject()) {
        const Json::Value& api = data["api"]; // Use const&
         std::cout << "API Config:" << std::endl;
        for (auto const& key : api.getMemberNames()) {
            std::cout << "- " << key << ": " << api[key].toStyledString(); // .toStyledString() handles different types
        }
    }
    ```

**D. Modifying JSON Data**

Use `operator[]` for assignments.

```c++
root["level"] = 100; // Update existing or add new
root["status"] = "upgraded";
root["availableTools"].append("planner"); // Add to array
root["api"]["endpoint"] = "https://secure.api.example.com"; // Modify nested
```

**E. Serializing JSON (Writing Data)**

Convert `Json::Value` back to string or stream.

*   **To String:**
    ```c++
    Json::StreamWriterBuilder writerBuilder;
    // For compact output (good for APIs)
    writerBuilder["indentation"] = "";
    std::string outputJson = Json::writeString(writerBuilder, root);

    // For pretty-printed output (good for config files, debugging)
    // Json::StreamWriterBuilder prettyWriterBuilder; // Defaults are usually pretty
    // prettyWriterBuilder["indentation"] = "  "; // Example: 2-space indent
    // std::string prettyJson = Json::writeString(prettyWriterBuilder, root);
    // OR use the older StyledWriter:
    // Json::StyledWriter styledWriter;
    // std::string prettyJson = styledWriter.write(root);

    std::cout << "Compact JSON:\n" << outputJson << std::endl;
    // std::cout << "\nPretty JSON:\n" << prettyJson << std::endl;
    ```
*   **To Stream (e.g., `std::ofstream`):**
    ```c++
    #include <fstream>
    // ... build Json::Value root ...

    std::ofstream outFile("output.json");
    if (outFile.is_open()) {
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = "\t"; // Use tabs for indent
        std::unique_ptr<Json::StreamWriter> writer(writerBuilder.newStreamWriter());
        writer->write(root, &outFile);
        outFile.close();
        std::cout << "JSON written to output.json" << std::endl;
    }
    ```

**F. Error Handling**

*   **Parsing:** Check the return value of `parse()` or `parseFromStream()`. Examine the `errs` string.
*   **Access:** Use `isMember`, `isXxx` type checks, and `get()` with defaults. Catch `Json::LogicError` (or `std::exception`) if using direct `operator[]` access where failures are possible.

**G. `jsoncpp` Best Practices:**

1.  **Validate Input:** Before parsing, ensure the source (string, stream) is valid/open.
2.  **Check Parse Success:** *Always* check the boolean return value of parsing functions. Log errors.
3.  **Defensive Access:** *Always* check `isMember` and `isXxx` before using `asXxx` or `operator[]` on potentially missing or incorrectly typed data. Use `get()` for optional fields with defaults.
4.  **Use `const&`:** When passing `Json::Value` around for reading, use `const Json::Value&` to avoid unnecessary copying.
5.  **`ArrayIndex`:** Use `Json::ArrayIndex` for indexing arrays, not just `int`.
6.  **Choose Writer Wisely:** Use compact output (`indentation = ""`) for network transmission, pretty output (`StyledWriter` or builder with indentation) for human-readable files.
7.  **Error Strategy:** Decide how to handle missing/invalid fields – use defaults, log warnings, or throw exceptions depending on how critical the data is.

---

## 2. Mastering `httplib.h`

`httplib.h` provides client and server capabilities.

**A. Server Side**

*   **Basic Setup:**
    ```c++
    #include "httplib.h" // Assuming it's in the include path

    int main() {
        httplib::Server svr;

        // Define routes before listening
        svr.Get("/", [](const httplib::Request& req, httplib::Response& res) {
            res.set_content("Hello World!", "text/plain");
        });

        // Start listening
        std::cout << "Server listening on port 8080..." << std::endl;
        if (!svr.listen("0.0.0.0", 8080)) {
             std::cerr << "Failed to listen on port 8080" << std::endl;
             return 1;
        }
        return 0; // Server runs until stopped (e.g., Ctrl+C)
    }
    ```
*   **Routing:**
    *   Methods: `svr.Get`, `svr.Post`, `svr.Put`, `svr.Delete`, `svr.Patch`, `svr.Options`.
    *   Path Parameters:
        ```c++
        // Matches /users/123, /users/abc etc.
        svr.Get(R"(/users/(\d+))", [&](const httplib::Request& req, httplib::Response& res) {
            std::string user_id = req.matches[1]; // Access captured group
            res.set_content("User ID: " + user_id, "text/plain");
        });
        // Or using httplib's named params (internally uses regex)
        svr.Get("/items/:id/details/:detail_name", [&](const httplib::Request& req, httplib::Response& res) {
            auto item_id = req.path_params.at("id");
            auto detail = req.path_params.at("detail_name");
             res.set_content("Item: " + item_id + ", Detail: " + detail, "text/plain");
        });
        ```
*   **Handling Requests (`const httplib::Request& req`)**
    *   `req.method`: "GET", "POST", etc.
    *   `req.path`: Full request path (e.g., "/users/123?query=abc").
    *   `req.target`: Path + Query (e.g., "/users/123?query=abc").
    *   `req.version`: "HTTP/1.1".
    *   `req.headers`: A `httplib::Headers` multimap. Use `req.has_header(key)`, `req.get_header_value(key, id=0)`, `req.get_header_value_count(key)`. Headers are case-insensitive.
    *   `req.params`: A `httplib::Params` multimap for URL query parameters. Use `req.has_param(key)`, `req.get_param_value(key, id=0)`, `req.get_param_value_count(key)`.
    *   `req.body`: The raw request body as `std::string`.
    *   `req.files`: For multipart/form-data uploads. A `httplib::MultipartFormDataMap`. Access like `req.files.find("myFile")`, `req.get_file_value("myFile")`.
    *   `req.matches`: `std::smatch` result if using regex routing.
    *   `req.path_params`: `std::unordered_map<std::string, std::string>` if using named path parameters.
    *   `req.remote_addr`, `req.remote_port`.
*   **Sending Responses (`httplib::Response& res`)**
    *   `res.status`: Set the HTTP status code (e.g., `200`, `404`, `500`). Defaults to `-1`, which becomes `200 OK` if not set. *Always set this explicitly.*
    *   `res.version`: Usually leave as "HTTP/1.1".
    *   `res.headers`: `httplib::Headers` multimap. Use `res.set_header(key, value)`.
    *   `res.body`: Set the response body `std::string`.
    *   `res.set_content(body, content_type)`: Helper to set body and Content-Type header.
    *   `res.set_redirect(url, status_code=302)`: Sets Location header and status.
    *   Content Providers: For large files or dynamic content, use `res.set_content_provider` or `res.set_chunked_content_provider` to avoid loading everything into memory.
*   **Middleware/Other Handlers:**
    *   `svr.set_logger(...)`: Log requests/responses.
    *   `svr.set_exception_handler(...)`: Catch exceptions within handlers.
    *   `svr.set_error_handler(...)`: Customize responses for specific HTTP errors (e.g., 404).
*   **HTTPS:** Requires `CPPHTTPLIB_OPENSSL_SUPPORT`. Use `httplib::SSLServer svr(cert_path, key_path);`. Setup is more involved.
*   **Threading:** `httplib` uses a thread pool by default (`CPPHTTPLIB_THREAD_POOL_COUNT`). Your handlers will be called concurrently. Ensure your handler logic is thread-safe if it accesses shared resources.

**B. Client Side**

*   **Basic Setup:**
    ```c++
    #include "httplib.h"
    #include <iostream>

    int main() {
        // HTTP
        httplib::Client cli("http://localhost:8080"); // Or "http://example.com"

        // HTTPS (Requires OpenSSL support compiled in)
        #ifdef CPPHTTPLIB_OPENSSL_SUPPORT
            // httplib::SSLClient ssl_cli("https://example.com");
            // ssl_cli.set_ca_cert_path("./ca-bundle.crt"); // Optional: if needed
            // ssl_cli.enable_server_certificate_verification(true);
        #else
            // Handle case where SSL is not supported if needed
        #endif

        // Set timeouts (highly recommended!)
        cli.set_connection_timeout(5); // 5 seconds
        cli.set_read_timeout(10);      // 10 seconds
        cli.set_write_timeout(10);     // 10 seconds

        // ... make requests ...
        return 0;
    }
    ```
*   **Making Requests:**
    *   Methods: `cli.Get()`, `cli.Post()`, `cli.Put()`, etc.
    *   Arguments: Path (`const char*` or `std::string`), optional `httplib::Headers`, optional body (`std::string` or `const char*, size_t`), optional content type.
    *   Params: Pass `httplib::Params` for GET requests or form data for POST/PUT.
    *   Files: Use `httplib::MultipartFormDataItems` for `cli.Post()` or `cli.Put()`.
    ```c++
    // GET request
    if (auto res = cli.Get("/status")) {
        // OK
    } else {
        // Error
    }

    // GET with headers and params
    httplib::Headers headers = { {"X-API-Key", "mysecret"} };
    httplib::Params params = { {"query", "agent"}, {"active", "true"} };
    if (auto res = cli.Get("/search", params, headers)) {
       // OK
    }

    // POST JSON
    Json::Value jsonData;
    jsonData["command"] = "reboot";
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "";
    std::string jsonBody = Json::writeString(wbuilder, jsonData);

    if (auto res = cli.Post("/command", jsonBody, "application/json")) {
        // OK
    }

    // POST Form data
     httplib::Params formData = { {"user", "neo"}, {"action", "activate"} };
     if (auto res = cli.Post("/action", formData)) {
         // OK
     }
    ```*   **Handling Responses (`httplib::Result res`)**
    *   **Check Success:** *Crucially, check if the request succeeded at the network level first!*
        ```c++
        if (res) {
            // Network request was successful, now check HTTP status
            std::cout << "HTTP Status: " << res->status << std::endl;
            std::cout << "Body: " << res->body << std::endl;

            // Check specific status codes
            if (res->status == 200) {
                // Process success
            } else {
                std::cerr << "HTTP Error: " << res->status << std::endl;
            }
        } else {
            // Network/connection error occurred
            auto err = res.error();
            std::cerr << "Network Error: " << httplib::to_string(err) << std::endl;
            // Handle connection refused, timeout, SSL error, etc.
        }
        ```
    *   `res->status`: HTTP status code (`int`).
    *   `res->version`: HTTP version (`std::string`).
    *   `res->reason`: HTTP reason phrase (`std::string`).
    *   `res->headers`: `httplib::Headers` multimap. Use `res->get_header_value()`, etc.
    *   `res->body`: Response body (`std::string`).
    *   `res.error()`: If `res` evaluates to `false`, this returns the `httplib::Error` enum value indicating the network-level error.
*   **Authentication:**
    *   `cli.set_basic_auth(user, pass);`
    *   `cli.set_bearer_token_auth(token);`
*   **Proxies:** `cli.set_proxy(host, port);`, `cli.set_proxy_basic_auth(...)`.
*   **Progress:** Some methods accept a `Progress` callback `std::function<bool(uint64_t current, uint64_t total)>`.

**C. `httplib.h` Best Practices:**

1.  **Server:**
    *   **Explicit Status/Content-Type:** Always set `res.status` and the `Content-Type` header for non-empty responses.
    *   **Error Handling:** Use `try-catch` in handlers if necessary, or set an exception handler. Return appropriate HTTP error codes (4xx for client errors, 5xx for server errors). Provide informative error messages (often as JSON) in the response body for errors.
    *   **Input Validation:** *Never* trust client input. Validate path parameters, query parameters, headers, and request bodies.
    *   **Security:** Be mindful of security (HTTPS, input sanitization, rate limiting - though `httplib` doesn't provide rate limiting itself).
    *   **Logging:** Use `set_logger` for debugging and monitoring.
    *   **Thread Safety:** Ensure handlers accessing shared data use mutexes or other synchronization.
2.  **Client:**
    *   **Check `Result`:** *Always* check `if (res)` before accessing `res->status` or `res->body`. This catches network errors.
    *   **Check `res->status`:** After confirming network success, check the HTTP status code to ensure the request was logically successful (e.g., 200 OK).
    *   **Set Timeouts:** *Always* set connection, read, and write timeouts to prevent indefinite blocking.
    *   **User-Agent:** Consider setting a descriptive User-Agent header (`cli.set_default_headers`).
    *   **HTTPS Verification:** For `SSLClient`, ensure certificate verification is enabled unless you have a specific reason to disable it (like testing with self-signed certs). Use `set_ca_cert_path` if needed.
    *   **Error Handling:** Check `res.error()` when `if (res)` is false to understand the specific network failure.

---

## 3. Achieving Synergy (The "God Workflow")

This is about using `jsoncpp` and `httplib.h` together smoothly.

*   **Server: API Endpoint (JSON Request -> JSON Response)**
    ```c++
    #include "httplib.h"
    #include <json/json.h>
    #include <memory> // For unique_ptr
    #include <sstream>

    // Helper to send JSON response
    void send_json_response(httplib::Response& res, int status, const Json::Value& json_data) {
        Json::StreamWriterBuilder wbuilder;
        wbuilder["indentation"] = ""; // Compact JSON for API
        res.status = status;
        res.set_content(Json::writeString(wbuilder, json_data), "application/json");
    }

    int main() {
        httplib::Server svr;

        svr.Post("/process", [](const httplib::Request& req, httplib::Response& res) {
            Json::Value requestJson;
            Json::CharReaderBuilder readerBuilder;
            std::string errs;
            std::stringstream ss(req.body);

            // 1. Parse Request Body using jsoncpp
            if (!Json::parseFromStream(readerBuilder, ss, &requestJson, &errs)) {
                Json::Value errorJson;
                errorJson["error"] = "Invalid JSON request body";
                errorJson["details"] = errs;
                send_json_response(res, 400, errorJson); // Bad Request
                return;
            }

            // 2. Validate and Process Data (using jsoncpp accessors)
            if (!requestJson.isMember("command") || !requestJson["command"].isString()) {
                 Json::Value errorJson;
                 errorJson["error"] = "Missing or invalid 'command' field";
                 send_json_response(res, 400, errorJson);
                 return;
            }
            std::string command = requestJson.get("command", "").asString();
            int priority = requestJson.get("priority", 0).asInt(); // Example default

            std::cout << "Received command: " << command << " with priority: " << priority << std::endl;
            // ... perform action based on command ...

            // 3. Build Response JSON using jsoncpp
            Json::Value responseJson;
            responseJson["status"] = "success";
            responseJson["message"] = "Command '" + command + "' processed.";
            responseJson["result_code"] = 123; // Example result

            // 4. Send Response using httplib
            send_json_response(res, 200, responseJson); // OK
        });

        std::cout << "JSON API Server listening on port 8081..." << std::endl;
        svr.listen("0.0.0.0", 8081);
        return 0;
    }
    ```
*   **Client: Calling a JSON API**
    ```c++
    #include "httplib.h"
    #include <json/json.h>
    #include <iostream>
    #include <memory> // For unique_ptr
    #include <sstream>

    int main() {
        httplib::Client cli("http://localhost:8081"); // Connect to the server above
        cli.set_connection_timeout(5);
        cli.set_read_timeout(10);

        // 1. Build Request JSON using jsoncpp
        Json::Value reqData;
        reqData["command"] = "update_agent";
        reqData["priority"] = 5;
        Json::Value params(Json::objectValue);
        params["target_version"] = "2.1";
        reqData["params"] = params;

        Json::StreamWriterBuilder wbuilder;
        wbuilder["indentation"] = "";
        std::string reqBody = Json::writeString(wbuilder, reqData);

        // 2. Send Request using httplib
        httplib::Headers headers = { {"User-Agent", "MyAgentClient/1.0"} };
        auto res = cli.Post("/process", headers, reqBody, "application/json");

        // 3. Handle Response (Check network, status, parse body)
        if (!res) {
            std::cerr << "Request failed: " << httplib::to_string(res.error()) << std::endl;
            return 1;
        }

        std::cout << "Response Status: " << res->status << std::endl;
        std::cout << "Response Body: " << res->body << std::endl;

        if (res->status != 200) {
            std::cerr << "API returned error status: " << res->status << std::endl;
             // Optionally parse error JSON here
            return 1;
        }

        // 4. Parse Response Body using jsoncpp
        Json::Value responseJson;
        Json::CharReaderBuilder readerBuilder;
        std::string errs;
        std::stringstream ss_res(res->body);

        if (!Json::parseFromStream(readerBuilder, ss_res, &responseJson, &errs)) {
            std::cerr << "Failed to parse JSON response body: " << errs << std::endl;
            return 1;
        }

        // 5. Process Response Data using jsoncpp
        std::string status = responseJson.get("status", "unknown").asString();
        std::string message = responseJson.get("message", "").asString();

        std::cout << "Parsed API Status: " << status << std::endl;
        std::cout << "Parsed API Message: " << message << std::endl;

        return 0;
    }
    ```
*   **Configuration Files:** Load configuration at startup using `jsoncpp`'s file parsing.
*   **Logging:** Format structured logs as JSON using `jsoncpp` before writing them.

**The "God Workflow" Mindset:**

*   **Clear Boundaries:** Use `httplib.h` for transport (HTTP), `jsoncpp` for data representation (JSON). Don't mix concerns excessively.
*   **Robustness:** Assume things *will* fail. Validate JSON, check HTTP results (`if (res)` AND `res->status`), handle exceptions.
*   **Consistency:** Use consistent JSON structures for your APIs (e.g., always have a `status` field, an `error` object on failure, a `data` object on success).
*   **Readability:** Use pretty JSON for files/logs, compact JSON for network. Write clean C++ code with good variable names and comments.
*   **Security First:** Always be thinking about how your HTTP endpoints or client requests could be misused. Validate, sanitize, use HTTPS.

---

## 4. Advanced Topics & Considerations

*   **Asynchronous Operations:** `httplib.h` is fundamentally **synchronous**. For highly concurrent applications needing non-blocking I/O, consider libraries like Boost.Asio, cpp-restsdk (Casablanca), or oat++. `httplib.h`'s threading model works well for many use cases but doesn't use async I/O paradigms.
*   **Performance:** For extreme performance, `httplib.h` might not be the fastest option compared to lower-level libraries or frameworks built on async I/O. `jsoncpp` is generally fast enough, but for hyper-optimization, libraries like simdjson exist (though often overkill). Profile your application if performance is critical.
*   **Security:** Beyond input validation and HTTPS: consider authentication/authorization, rate limiting, protecting against common web vulnerabilities (if applicable) – these often require more than just `httplib.h`.
*   **Dependency Management:** `httplib.h` is easy (single header). `jsoncpp` usually requires linking. Consider using package managers (CMake + FetchContent, vcpkg, Conan) to manage dependencies.
*   **Testing:** Write unit tests for your JSON building/parsing logic. Write integration tests for your HTTP endpoints and client interactions.

---

By focusing on robust error handling, defensive coding, clear structure, and understanding the strengths and limitations of these libraries, you can build highly effective and reliable C++ applications that communicate over HTTP using JSON. Good luck achieving your "god workflow"!
```

### File: docs/orchestration-layer-shema.yml
```yaml
version: MK1
name: "schema-building-workflow"
description: "Uses pre/post steps for localized nesting and structured agent key."

import:
  workflows: # hole-ass external workflow loading cause why the fuck not (this will obviously be ignored for now)
  agents: # yaml agent profiles
    - ./agent-test.yml # direct file location
    - ./agents # load an entire folder containing multiple agent yml profiles
  define:
    name: ... # explicit declaration here.

  # No external agents needed for this example flow

initial_state:
  query: "Find articles about AI in healthcare from the last month."
  search_result_count: 3
# or just some sort of entrypoint 
entrypoint:
  # ...
settings: #workflow settings

defines: # maybe a define section . where steps declared are only processed and not executed. potentially making their id/name reusable, maybe even importing other steps from other workflows (could be nice to have a set of standard steps).

# Types for now agent / loop / parallel / job (service mode) / workflow (call another workflow reference) / some sort of pipe mode
rules: #rules for the workflow execution maybe: auto/explicit (enforce entrypoint)
workflow: 
  - id: execute_search_query
    type: agent # This agent call has steps before and after it 
  
    description: "Prepare, execute, and refine a web search."
    agent: # Structured agent key
      define: # Defining inline
        name: "WebSearcher_Inline"
        system_prompt: |
          You are a web search assistant. Execute the search query precisely.
          Query: {input.search_query}
          Max Results: {input.max_results}
          Return URLs and snippets.
        tools:
          define:
            name: testTool
            description: "sample description"
            type: file/code #file containing a tool code in whatever language
            path: ...
            code: # (depending on what we support) I mainly wanna have simple bash and python here. but Im gonna put this on hol since im thinking of making a proper API for tool use.
          internal:
            - bashTool
            -
          external: #could have external api for tools (could just use python for example)
            - 
    input: # Define input for the main agent call
      search_query: "$(state.refined_query)" # Comes from pre_step
      max_results: "{state.search_result_count}"
    output: state.raw_search_results # Output from the main agent call
    pre_steps: # Steps execute *before* the main agent call above
      - id: refine_query_step
        type: agent # Nested agent call!
        description: "Refine user query using another inline agent."
        agent:
          define:
            name: "QueryRefiner_Inline"
            system_prompt: "Rewrite the user query for better search engine results. Query: {input}"
        input: "{state.query}" # Input for this pre_step agent
        output: state.refined_query # Output used by main agent call's input

    post_steps: # Steps execute *after* the main agent call completes successfully
      - id: filter_results_step
        type: agent
        description: "Filter search results with an inline agent."
        agent:
          definition:
            name: "ResultFilter_Inline"
            system_prompt: "Filter the search results, removing duplicates or irrelevant links. Results:\n{input}"
        input: "{state.raw_search_results}" # Input is the output of the main agent call
        output: state.filtered_results # Final output of the parent step

    next_step: present_results

  - id: present_results
    type: transform
    description: "Format final results."
    timeout: '80s'
    depends-on:
    expression: "'Found Results:\\n' + {state.filtered_results}"
    output: final_output

  - id: pre-defined-step

```

### File: docs/proto-schema-agent.md
```markdown
{
  "_comment": "REVISED JSON structure expected as output FROM the LLM, TO BE PARSED by the Agent/Orchestrator. Reflects changes like thoughts array, action/type fields. Comments clarify requirements.",
  "llm_output_schema_version": "llm-json-output-draft-0.2", // Internal versioning for this JSON structure idea

  "thoughts": [ // CHANGED: Now an array of strings. Can be empty `[]`.
    "string - Step 1 of reasoning: Identified the user's core need.",
    "string - Step 2: Decided an external search is required.",
    "string - Step 3: Formulating parameters for the 'web_search' action."
    // The LLM can break down its thought process into multiple steps or aspects.
  ],

  "actions": [ // RENAMED: Was 'tool_calls'. REQUIRED array. MUST be `[]` if no action is taken.
    // --- Example of a single action object ---
    {
      "_comment": "Object representing one action requested by the LLM. Required fields: 'action', 'type', 'params'.",

      "action": "string - RENAMED: Was 'tool_name'. REQUIRED. The identifier for the action to take (e.g., 'web_search', 'bash', 'URLValidator', 'summarize_chunk'). This identifier must map to a known Tool, Script, API call, internal function, etc.",

      "type": "string - ADDED: REQUIRED. Specifies the category of the action. Helps the orchestrator select the correct handler. Examples: 'tool', 'script', 'http_request', 'internal_function', 'workflow_control'.",
      // Examples:
      // 'tool': Corresponds to a registered Tool object (like the C++ Tool class).
      // 'script': Refers to an imported/defined script (e.g., bash, python).
      // 'http_request': Specifies parameters for making a direct HTTP call.
      // 'internal_function': Calls a hardcoded function within the orchestrator/agent library (e.g., 'summarizeHistory').
      // 'workflow_control': Actions that affect the workflow itself (e.g., 'fail_step', 'skip_to').

      "params": { // REQUIRED: A JSON object containing the parameters needed by the specified 'action' and 'type'. Structure depends entirely on the action's requirements.
        "query": "AI in healthcare latest research", // Example for a 'web_search' action
        "region": "us"
        // ... structure defined by the action referenced in 'action'/'type'
      }
    }
    // --- End of example action object ---
    // , { "action": "bash", "type": "script", "params": { "command": "ls -l" } } // Multiple actions possible
  ],

  "final_response": "string - REQUIRED: The textual response intended for the end-user. Logic remains the same: \n - If 'actions' is EMPTY (`[]`), this MUST contain the final answer.\n - If 'actions' is NOT EMPTY, this SHOULD be `\"\"` or an interim message (ignored by default loop)."
}

============================================

"thoughts": [
  {
    "type": "OBSERVATION", // Types: PLAN, OBSERVATION, QUESTION, HYPOTHESIS, CRITIQUE, ASSUMPTION, GOAL, NORM (way of doing x, avoiding y because)
    "content": "User query asks for healthcare AI articles within the last month."
  },
  {
    "type": "PLAN",
    "content": "1. Need to perform a web search. 2. Will need current date for 'last month'. 3. Filter results. 4. Summarize."
  },
  {
    "type": "ASSUMPTION",
    "content": "Assuming 'last month' means the previous calendar month relative to today."
  },
  {
    "type": "QUESTION", // Internal question/uncertainty
    "content": "Is the user interested in academic papers or news articles primarily?"
  }
],

good to have the tought framework, easily extendable as well.

"thoughts": [ { "type": "OBSERVATION", "content": "User asked for weather but didn't specify location." } ],
"actions": [
  {
    "action": "web_search",
    "type": "tool",
    "params": { "query": "AI healthcare news", "timeframe": "past_30_days" },
    "confidence": 0.85, // Optional: Estimated confidence (0.0 to 1.0) that this action/params are correct.
    "warnings": [ // Optional: Specific warnings about this action/params.
      "Parameter 'timeframe=past_30_days' is an interpretation of 'last month'; could be ambiguous.",
      "Web search might return irrelevant results requiring filtering."
    ]
  }
  {
    "action": "request_user_input", // Or potentially a specific type like "clarification_request"
    "type": "workflow_control", // Indicate it affects the flow, not an external tool
    "params": {
      "query_to_user": "I can get the weather for you! Which city are you interested in?",
      "required_info": ["location (city name)"], // What information is needed
      "context": "User asked for weather forecast." // Optional context for the request
    }
  }
  {
    "action": "response", // could be the final response or not, the're just gonna stack.
    "type": "default", 
    "params": {
      "reply": "here is what you request $(context.report)",
      "file": "./path-to.." // displays the file content
    }
  }
],

{
  "status": "REQUIRES_ACTION", // Possible values: SUCCESS_FINAL, REQUIRES_ACTION, REQUIRES_CLARIFICATION, ERROR_INTERNAL
  "thoughts": [ ... ],
  "actions": [ ... ], // Non-empty if status is REQUIRES_ACTION
  "final_response": "..." // Contains final answer if status is SUCCESS_FINAL
}

==================================================================

{
  "_comment": "REFINED JSON structure expected from LLM (v0.3). Includes structured thoughts, action confidence/warnings, clarification requests, inline responses, and root status.",
  "llm_output_schema_version": "llm-json-output-draft-0.3",

  "status": "string - REQUIRED: High-level outcome hint. Values: 'SUCCESS_FINAL', 'REQUIRES_ACTION', 'REQUIRES_CLARIFICATION', 'ERROR_INTERNAL'. Orchestrator can use this for quick dispatch.",

  "thoughts": [ // REQUIRED: Array of structured thought objects. Can be empty `[]`.
    // --- Example Thought Object ---
    {
      "type": "string - REQUIRED: Category of the thought. Suggested values: PLAN, OBSERVATION, QUESTION, HYPOTHESIS, CRITIQUE, ASSUMPTION, GOAL, NORM (explaining a standard procedure or reason).",
      "content": "string - REQUIRED: The textual content of the thought."
    }
    // --- Examples ---
    // { "type": "OBSERVATION", "content": "User requested a summary of $(state.document_id)." },
    // { "type": "PLAN", "content": "1. Fetch document content. 2. Call summarization tool. 3. Format as response." },
    // { "type": "NORM", "content": "Standard procedure is to summarize documents under 5000 words directly; longer ones require chunking." },
    // { "type": "QUESTION", "content": "Is a specific summary length required?" }
  ],

  "actions": [ // REQUIRED: Array of action objects. MUST be `[]` if status is 'SUCCESS_FINAL' or 'ERROR_INTERNAL' and no cleanup actions are needed.
    // --- Example Action Object (Tool/Script/API Call) ---
    {
      "_comment": "Represents executing a tool, script, API call, etc.",
      "action": "string - REQUIRED: Identifier for the action (tool name, script name, function name).",
      "type": "string - REQUIRED: Category ('tool', 'script', 'http_request', 'internal_function', etc.).",
      "params": { // REQUIRED: Parameters for the action, specific to the action/type.
        // "paramName": value, ...
      },
      "confidence": "float - OPTIONAL (0.0-1.0): LLM's estimated confidence in this action/params.",
      "warnings": [ // OPTIONAL: Array of strings warning about potential issues.
        // "string - Example warning about parameter ambiguity."
      ]
    },
    // --- Example Action Object (Clarification Request) ---
    {
      "_comment": "Represents asking the user for more information.",
      "action": "request_user_input", // Standardized action name for clarification.
      "type": "workflow_control", // Type indicates it affects the workflow/interaction loop.
      "params": {
        "query_to_user": "string - REQUIRED: The question to ask the user.",
        "required_info": ["string - OPTIONAL: Hint about what info is needed (e.g., 'city name')."],
        "context": "string - OPTIONAL: Brief context for the question."
      },
      "confidence": 1.0, // Typically high confidence if clarification is needed.
      "warnings": [] // Usually no warnings needed here.
    },
    // --- Example Action Object (Direct Response/Output) ---
    {
      "_comment": "Represents sending a direct response fragment or content to the output sink(s). Can be intermediate or part of the final output.",
      "action": "send_response", // Standardized action name for direct output.
      "type": "output", // Type indicates it's content to be delivered.
      "params": {
        // Choose ONE of the following content sources:
        "text": "string - REQUIRED (if no file/variable): Literal text content to send. Can use $(...) variables.",
        // "file_path": "string - REQUIRED (if no text/variable): Path to a local file whose content should be sent.", // Requires orchestrator file access
        // "variable": "string - REQUIRED (if no text/file): State variable whose content should be sent (e.g., '$(state.summary)')." // FUTURE: Requires careful variable resolution
      },
      "confidence": 1.0,
      "warnings": []
    }
    // ... more actions can be included in the array
  ],

  "final_response": "string - DEPRECATED / REDUNDANT? With status='SUCCESS_FINAL' and potentially `action: send_response`, this might be less necessary. TBD: Keep for simple cases or remove? For now, retain for backward compatibility or very simple, single-response outputs where status='SUCCESS_FINAL' and actions=[]."
  // If kept:
  // - If status='SUCCESS_FINAL' and actions=[], this MUST contain the final answer.
  // - If status='REQUIRES_ACTION' or 'REQUIRES_CLARIFICATION', this SHOULD be `""`.
  // - If status='ERROR_INTERNAL', this might contain an error message.
}
```

### File: docs/termbox-docs.md
```markdown
Here's a comprehensive guide to Termbox, focusing on practical implementation and best practices:

### Installation

Before diving into the implementation details, you'll need to install Termbox. Here are the installation methods for major platforms:

```bash
# Ubuntu/Debian
sudo apt-get install libtermbox-dev

# Fedora/RHEL/CentOS
sudo dnf install termbox-devel

# macOS (using Homebrew)
brew install termbox

# Arch Linux
sudo pacman -S termbox-devel
```

### Basic Concepts

1. **Cell Structure**  - Each character cell contains three properties:
                    - Character (UTF-8 encoded)
    - Foreground color
    - Background color


  - Cells are addressed using coordinates (x,y) starting from (0,0)


2. **Event Handling**  - Two main event types:
                    - Keyboard events (key presses/releases)
    - Mouse events (clicks/movement)


  - Events are polled synchronously


3. **Screen Buffer**  - Double-buffering system for smooth updates
  - Changes must be explicitly presented
  - Automatic screen clearing on initialization



### Core Implementation Patterns

Modern C++ Implementation```cpp
#include <termbox.h>
#include <memory>

class TerminalUI {
private:
    struct tb_event event;
    bool running = true;

public:
    TerminalUI() {
        if(tb_init() != 0) throw std::runtime_error("Failed to initialize Termbox");
    }

    ~TerminalUI() { tb_shutdown(); }

    void run() {
        while(running && tb_poll_event(&event)) {
            handleEvent();
            updateScreen();
            tb_present();
        }
    }

private:
    void handleEvent() {
        if(event.type == TB_EVENT_KEY && event.key == TB_KEY_ESC) running = false;
    }

    void updateScreen() {
        tb_clear();
        tb_print(0, 0, TB_DEFAULT, TB_DEFAULT, "Hello, Termbox!");
    }
};
```

- RAII compliant resource management
- Exception-safe initialization
- Clear separation of concerns
- Modern C++ practices
- Requires C++11 or higher
- More complex initial setup
This implementation follows modern C++ best practices with RAII (Resource Acquisition Is Initialization) pattern. The class manages Termbox initialization and cleanup automatically through constructor/destructor, preventing resource leaks.Simple C Implementation```cpp
#include <termbox.h>

int main() {
    if(tb_init()) {
        tb_print(0, 0, TB_DEFAULT, TB_DEFAULT, "Hello!");
        tb_present();
        
        struct tb_event ev;
        while(tb_poll_event(&ev)) {
            if(ev.type == TB_EVENT_KEY && ev.key == TB_KEY_ESC) break;
        }
    }
    tb_shutdown();
    return 0;
}
```

- Simple and straightforward
- Minimal boilerplate
- Easy to understand
- Works with C++98
- Manual resource management
- No error handling
- Limited functionality
This simpler implementation demonstrates basic Termbox usage with minimal overhead. It's suitable for quick prototypes or simple applications where modern C++ features aren't required.### Advanced Features

1. **Color Support**```cpp
// Set colors for text
tb_set_cursor(1, 1);
tb_change_cell(1, 1, 'X', TB_BLACK, TB_RED);

// Enable 256-color mode
tb_select_output_mode(TB_OUTPUT_256);
tb_change_cell(2, 2, '*', TB_COLOR_CYAN, TB_COLOR_BLUE);
```


2. **Mouse Handling**```cpp
void handleEvent(struct tb_event& ev) {
    switch(ev.type) {
        case TB_EVENT_MOUSE:
            handleMouse(ev);
            break;
        case TB_EVENT_KEY:
            handleKey(ev);
            break;
    }
}

void handleMouse(struct tb_event& ev) {
    switch(ev.key) {
        case TB_KEY_MOUSE_LEFT:
            handleClick(ev.x, ev.y);
            break;
        case TB_KEY_MOUSE_WHEEL_UP:
            handleWheelUp(ev.x, ev.y);
            break;
    }
}
```


3. **Unicode Support**```cpp
void printUnicode(int x, int y, const char* str) {
    tb_print(x, y, TB_DEFAULT, TB_DEFAULT, str);
}

// Example usage
printUnicode(0, 0, "Hello, 世界!");
```



### Best Practices

1. **Error Handling**  - Always check initialization result
  - Validate coordinates before cell access
  - Handle terminal resize events appropriately


2. **Performance Optimization**  - Batch cell updates before presenting
  - Minimize screen clearing operations
  - Use efficient buffer management


3. **Cross-Platform Compatibility**  - Handle terminal size changes gracefully
  - Support fallback colors when needed
  - Consider different keyboard layouts



### Common Pitfalls to Avoid

1. **Resource Management**  - Always call `tb_shutdown()` even on error paths
  - Check return values of all Termbox functions
  - Handle terminal resize events properly


2. **Screen Updates**  - Don't forget to call `tb_present()` after changes
  - Avoid unnecessary screen clearing
  - Batch updates when possible


3. **Event Handling**  - Process events in a timely manner
  - Handle all event types appropriately
  - Consider event queue overflow scenarios



Termbox provides a simple API but requires careful consideration of terminal-specific behaviors and edge cases. Always test your applications across different terminal emulators and platforms to ensure consistent behavior.
```

### File: docs/test2.yml
```yaml
# --- Orchestration Schema: MK1 (Draft 2) ---
version: MK1.2 # Incrementing version slightly . yeap!
name: "schema-building-workflow-mk1-draft2"
description: "Adds script/context imports, execution modifiers, explicit vars, IO mapping, and new variable syntax $(...)."

# --- Import Section ---
# Defines external resources to load/prepare before workflow execution.
# FUTURE: Define precise loading order and conflict resolution.
import:
  workflows:
    # - ./common_error_handler.yaml # FUTURE: Load entire workflows or sub-workflows to be called.
  agents:
    # Load agent profiles from YAML files. Agent's internal 'name' is the reference key.
    - ./agents/response_formatter.yaml
    # FUTURE: Implement loading all *.yaml from a directory. Agent names from files used as keys.
    - ./agents/data_validators/
  defines:
    # - ./standard_steps.yaml # FUTURE: Import definitions from shared files.
  scripts: # Load external scripts to be potentially used as tools or executed directly.
    # Load a single script file. Tool/reference name inferred from filename without extension ('aggregate-folder').
    - type: file
      path: ./scripts/standard/aggregate-folder.sh
      # runtime: bash # FUTURE: Could explicitly set runtime, otherwise infer from extension (.sh -> bash, .py -> python).
    # Load a single file and explicitly name the reference 'exp-tool'.
    - type: file
      path: ./scripts/standard/test.sh
      name: exp-tool # Overrides inferred name 'test'.
    # Load all scripts from a folder, creating a namespace 'boost'. Scripts accessed via 'boost.scriptName'.
    - type: folder
      path: ./scripts/boost-standard # Contains e.g., sendRequest.py, cleanup.sh
      name: boost # Namespace. Access via e.g., boost.sendRequest, boost.cleanup.
  context: # Prepare context variables/files available globally during workflow execution.
    # Load content of a file into a global context variable 'global.context.cpp-ref'.
    - type: file
      path: ./.resources/repo-cpp-code-refrences.md
      name: cpp-ref # Variable name. Access as $(global.context.cpp-ref).
      # var: global.code # FUTURE: Allow specifying the exact state variable path? Default to global.context.<name>.
    # Create/Prepare a context file on the filesystem before execution.
    - type: file
      path: ./.context # Path to the target file.
      create: true # Optional: Ensure file exists (create if not). Default: false? TBD.
      content: | # Optional: Set initial content (overwrites existing).
        # Initial context header
      append: # Optional: Append content from sources. Executes in order listed.
        - type: text # Append literal text.
          value: "\n--- Appended context ---\n"
        - type: file # Append content from another file.
          path: ./snippets/context_snippet.txt
        # - type: variable # FUTURE: Append content from a state variable?
        #   var: $(initial_state.query)
        - type: backup # Optional: Backup existing file before modifying.
          to: "./.context.backup_{timestamp}" # FUTURE: Add timestamp/pattern support.

# --- Initial State / Entrypoint ---
initial_state:
  query: "Find articles about AI in healthcare from the last month."
  search_result_count: 3
  user_id: "user123"
# entrypoint: step_id_start_here # FUTURE: Define a specific step ID to begin execution.

# --- Workflow Settings ---
settings:
  default_timeout: "60s"
  max_concurrent_steps: 1 # CURRENT: Sequential. FUTURE: > 1 enables parallelism.
  state_persistence: "none" # FUTURE: Configure state saving.
  error_handling_strategy: "fail_fast" # CURRENT: Stop on error. FUTURE: Add more strategies.
  # default_error_handler_step: ... # FUTURE
  debug_level: INFO

# --- Definitions ---
# Declare reusable components accessible by their keys (e.g., StandardSummarizer, BashExecutor).
defines:
  agents:
    StandardSummarizer:
      name: "StdSummarizer_Defined"
      system_prompt: "Provide a concise summary of the input text: $(input.text)" # Using new var syntax
      iteration_cap: 3
  tools:
    BashExecutor:
      name: "bash"
      description: "Executes a bash command provided as input."
      type: "script" # Renamed from "code" to "script" for clarity with imported scripts.
      runtime: "bash" # Explicit runtime.
      # reference: BashExecutor # FUTURE: Link to an imported script definition?
      code: | # Keep inline code option.
        #!/bin/bash
        echo "Executing command: $1" >&2
        eval "$1" # WARNING: Security risk!
  steps:
    LogStartStep:
      type: transform
      description: "Log the start of the workflow with the query."
      # Note: Using $(...) syntax for variable access. Requires parser update.
      expression: "'Workflow starting for query: ' + $(state.query)" # CURRENT: Basic concat. FUTURE: Needs robust expression engine.
      output: state.log.start_message

# --- Execution Rules ---
rules:
  execution_mode: "auto" # CURRENT: Start from first step. FUTURE: Add "explicit".
  sink: # Defines output destinations.
    - type: stdout # CURRENT: Print final result/logs.
    # FUTURE: Add stderr, file, http, client types.
  run: # detach | default | debug

# --- Main Workflow Steps ---
workflow:
  # Using a defined step.
  - id: start_logging
    ref: LogStartStep # References 'LogStartStep' from 'defines.steps'.

  # Step to explicitly set/modify a state variable.
  - id: set_global_query_var
    type: var # New step type for direct variable manipulation.
    description: "Store the initial query in a global variable for later reference."
    # Option A: Simple key-value assignment
    set:
      global.query_snapshot: $(state.query) # Assign value of state.query to global.query_snapshot
      global.timestamp: $(func.timestamp()) # FUTURE: Allow calling built-in functions?
    # Option B: More structured assignment (TBD which is better)
    # assignments:
    #   - target: global.query_snapshot
    #     value: $(state.query)

  - id: execute_search_query
    type: agent
    description: "Execute a refined web search using an inline agent and tools."
    # exec: # FUTURE: Override execution mode for this step.
      # mode: parallel # e.g., run multiple instances with different inputs.
      # count: 3
      # mode: schedule # e.g., run at a specific time or interval.
      # cron: "0 * * * *"
      # mode: loop # e.g., loop until condition met.
      # condition: ...
    timeout: "90s"
    depends_on: # FUTURE: Requires DAG execution support.
      - start_logging
    agent:
      define:
        name: "WebSearcher_Inline"
        system_prompt: |
          You are a web search assistant. Execute the search query precisely.
          Query: $(input.search_query)
          Max Results: $(input.max_results)
          Return URLs and snippets. Use available tools if needed.
        iteration_cap: 5
        tools:
          define: # Define tools inline for this agent instance only.
            - name: "URLValidator"
              description: "Checks if a URL is reachable."
              type: "script" # Tool implemented via code/script.
              runtime: "python"
              # parameters: { url: string } # FUTURE
              code: | # Inline code.
                import requests
                # ... (code as before) ...
                def run(params): return {"valid": validate(params.get('url', ''))}
          internal: # Reference tools defined globally (in 'defines' or imported).
            - "bash" # Tool named 'bash', likely references BashExecutor.
          # external: # FUTURE: Reference external APIs/services.
          #   - ...

    # Using separate input/output fields:
    input:
      search_query: "$(state.query)" # Using $(...) syntax for variable access.
      max_results: "$(state.search_result_count)"
    output: state.search_results # Where to store the agent's final output.

  - id: summarize_results
    type: agent
    description: "Summarize the search results using a defined agent."
    agent:
      ref: StandardSummarizer # Reference agent defined in 'defines.agents'.
    # Using the alternative IO mapping field:
    IO:
      # Maps the content of state variable 'state.search_results' to the 'input.text' expected by the agent's prompt.
      "$(state.search_results)": input.text
      # Maps the agent's final output (implicitly 'agent.output') to the state variable 'state.summary'.
      agent.output: state.summary
    # Note: The 'IO' field aims to replace 'input'/'output' for certain mapping scenarios.
    # Its exact semantics (especially agent.output implicit variable) need careful design.
    # TBD: How does IO interact with complex inputs/outputs or multiple mappings? Sticking with input/output might be clearer.

  - id: present_results
    type: transform
    description: "Format final results combining search and summary."
    depends_on: # FUTURE
      - execute_search_query
      - summarize_results
    # Using $(...) syntax in expression.
    expression: "'Summary based on search for query [' + $(global.query_snapshot) + ']:\\n' + $(state.summary)"
    output: final_output # Special target indicating the workflow's final result.
```

### File: docs/test.yml
```yaml
# --- Orchestration Schema: MK1 (Refined Draft) ---
version: MK1
name: "schema-building-workflow-refined"
description: "Workflow demonstrating imports, definitions, settings, and inline configurations. Features commented for clarity."

# --- Import Section ---
# Defines external resources to load before workflow execution.
# FUTURE: Define precise loading order and conflict resolution (e.g., if multiple files define the same agent name).
import:
  workflows:
    # - ./common_error_handler.yaml # FUTURE: Load entire workflows or sub-workflows to be called.
  agents:
    # Load agent profiles from YAML files. The 'name' field *within* the agent's YAML file will be used for referencing.
    - ./agents/response_formatter.yaml # Load a specific agent profile. Its internal 'name' is the reference key.
    - ./agents/data_validators/ # FUTURE: Implement loading all *.yaml from a directory. Agent names from files used as keys.
  defines:
    # - ./standard_steps.yaml # FUTURE: Import definitions (steps, tools, agents) from shared files.
  # tools: # FUTURE: Import tool definitions separately? Or are they always part of agent/workflow defines? TBD.
    # - ./common_tools.yaml
  scripts: # again, single file or all files from folder
    - type: file
      path: ./scripts/standard/aggregate-folder.sh # scripts and files  will be parsed to extract the name by removing the extention (or could Override by using name:)
    - type: file
      path: ./scripts/standard/test.sh
      name: exp-tool
    - type: folder
      path: ./scripts/boost-standard
      name: boost # now we have a boost namespace to access the rest eg. boost.sendRequest (from boost-standard/sendRequest.py (also, runtime will be infered from extention for now we will stick to .sh .zsh .py)) the same logic will apply to folders and file in the context section
  context: # global.context.cpp-ref
    - type: file
      path: ./.resources/repo-cpp-code-refrences.md
      name: cpp-ref #if not specified, auto extraction will happen so its name would be repo-cpp-code-refrences and it would be accessed at global.context.repo-cpp-code-refrences
      # var: global.code
    - type: file
      path: ./.context # file that does exist is created
      append: # "./.context.old" | "simple text context to insert to the file" we could even make these stackble together for effective and blazing fast context loading and prototyping.
        
    - type: file
      path: ./scripts/standard/aggregate-folder.sh
      content:

    
      


# --- Initial State / Entrypoint ---
# Defines the starting data context for the workflow.
initial_state:
  query: "Find articles about AI in healthcare from the last month."
  search_result_count: 3
  user_id: "user123"
# entrypoint: step_id_start_here # FUTURE: Define a specific step ID to begin execution, overriding sequential start.

# --- Workflow Settings ---
# Global configurations affecting the execution environment.
settings:
  default_timeout: "60s" # Default maximum duration for a step. Parsed into duration (e.g., 60 seconds).
  max_concurrent_steps: 1 # CURRENT: Assume sequential. FUTURE: Allow > 1 for parallel steps.
  state_persistence: "none" # FUTURE: Configure saving/loading workflow state ("none", "local_file:<path>", "redis:<conn_string>").
  error_handling_strategy: "fail_fast" # CURRENT: Stop on first error. FUTURE: Add "continue", "jump_to_error_handler".
  # default_error_handler_step: "global_error_handler" # FUTURE: ID of a defined step to jump to on unhandled errors.
  debug_level: INFO # Logging level for orchestrator messages (e.g., DEBUG, INFO, WARN, ERROR).

# --- Definitions ---
# Declare reusable components (agents, tools, steps) that can be referenced by ID/name later.
# These are parsed but not executed unless referenced in the main 'workflow' section.
defines:
  agents:
    # Defines a reusable agent configuration accessible via the key 'StandardSummarizer'.
    StandardSummarizer:
      # Agent configuration fields are the same as in agent profile YAMLs (name, system_prompt, etc.)
      name: "StdSummarizer_Defined" # This name should ideally match the key for clarity.
      system_prompt: "Provide a concise summary of the input text."
      iteration_cap: 3
      # tools: [...] # FUTURE: Define tools expected/available for this defined agent.
  tools:
    # Defines a reusable tool configuration accessible via the key 'BashExecutor'.
    BashExecutor:
      name: "bash" # Internal name used when attaching to agents.
      description: "Executes a bash command provided as input."
      type: "code" # Specifies this tool is implemented via code. FUTURE: Add "api", "service".
      runtime: "bash" # Specifies the execution runtime for 'code' type. FUTURE: Add "python", "nodejs".
      # parameters: # FUTURE: Define expected input parameters and types for validation/prompting.
      #   command: { type: "string", description: "The bash command to execute." }
      code: | # The actual code to execute. Input mapping TBD (e.g., via parameters).
        #!/bin/bash
        echo "Executing command: $1" >&2 # Example logging
        eval "$1" # WARNING: Security risk! Needs sandboxing/safer execution. Placeholder only.
  steps:
    # Defines a reusable step accessible via the key 'LogStartStep'.
    LogStartStep:
      # Step configuration fields are the same as in the main workflow steps.
      # 'id' within defines is primarily for internal reference; it gets a new unique 'id' when used in the workflow.
      type: transform
      description: "Log the start of the workflow with the query."
      expression: "'Workflow starting for query: ' + {state.query}" # CURRENT: Simple string concat. FUTURE: More complex expression language.
      output: state.log.start_message # Variable path where the output is stored in the workflow state.

# --- Execution Rules ---
# Defines how the workflow behaves overall.
rules:
  execution_mode: "auto" # CURRENT: Start from the first step in the 'workflow' list. FUTURE: Add "explicit" (requires entrypoint).
  sink: # Defines where step outputs/logs/final results are sent.
    - type: stdout # CURRENT: Print final result/logs to standard output.
    # - type: stderr # FUTURE: Send specific logs/errors to standard error.
    # - type: file # FUTURE: Append outputs/logs to a specified file.
    #   path: /logs/workflow_run.log
    # - type: http # FUTURE: POST results/logs to a webhook URL.
    #   url: http://example.com/api/webhook
    #   headers: { ... }
    # - type: client # FUTURE: Stream outputs/logs to a connected UI/client panel via websocket/other protocol.

# --- Main Workflow Steps ---
# The sequence of steps to be executed.
workflow:
  # Example using a defined step. It inherits the definition from 'defines.steps.LogStartStep'.
  - id: start_logging # REQUIRED: Unique ID for this instance of the step within the workflow.
    ref: LogStartStep # References the defined step by its key.
  - id: execute_search_query
    type: agent # Specifies the step type. Orchestrator calls agent execution logic.
    description: "Execute a refined web search using an inline agent and tools."
    exec: #parallel | scale (step x times) | schedule | live-loop, exec ovveride
    timeout: "90s" # Overrides the default timeout from 'settings'.
    depends_on: # FUTURE: Define execution dependencies. Requires DAG scheduler.
      - start_logging # This step waits for 'start_logging' to complete successfully.
    agent: # Agent configuration for this step.
      define: # Define the agent inline, specifically for this step.
        # Agent configuration fields (name, system_prompt, etc.)
        name: "WebSearcher_Inline" # Name for this temporary, inline agent instance.
        system_prompt: |
          You are a web search assistant. Execute the search query precisely.
          Query: {input.search_query}
          Max Results: {input.max_results}
          Return URLs and snippets. Use available tools if needed.
        iteration_cap: 5 # Override default if needed
        tools: # Define tools available *only* to this inline agent instance.
          define: # Define tools inline.
            - name: "URLValidator" # Name of the tool.
              description: "Checks if a URL is reachable."
              type: "code" # Tool implementation type.
              runtime: "python" # Runtime for the code.
              # parameters: # FUTURE: Define expected parameters.
              #   url: string
              code: | # Inline code block (requires runtime support in orchestrator).
                import requests
                def validate(url):
                  # ... (validation logic) ...
                  return {"valid": is_valid}
                def run(params): # Assumed entry point function name.
                  return validate(params.get('url', ''))
          internal: # List names of tools expected to be available from defines/global scope.
            - "bash" # References the 'bash' tool defined in 'defines.tools.BashExecutor'.
          # external: # FUTURE: Reference externally hosted tools/APIs.
            # - name: "GoogleSearchAPI"
            #   endpoint: "..."
            #   auth: "..."

    input: # Define mapping from workflow state to this step's input context.
      search_query: "{state.query}" # Access 'query' from the main 'state'.
      max_results: "{state.search_result_count}"
    output: state.search_results # Store the agent's final response in 'state.search_results'.

  - var: global_query=state.query #global references to expand on the references idea

  - id: summarize_results
    type: agent
    description: "Summarize the search results using a defined agent."
    agent:
      ref: StandardSummarizer # Reference the agent defined in 'defines.agents.StandardSummarizer'.
    input: "{state.search_results}" # Use the output from the previous step as input.
    output: state.summary
    IO: "{state.search_results}": state.summary # use this in favor of input: ouput:


  - id: present_results
    type: transform # A step that manipulates data without calling an LLM agent.
    description: "Format final results combining search and summary."
    depends_on: # FUTURE: Specify dependencies.
      - execute_search_query # Depends on both search and summarize finishing.
      - summarize_results
    # expression: # The operation to perform (requires an expression evaluator).
      # "'Found Results (' + len({state.search_results}) + '):\\n' + {state.summary}" # Example: String formatting and length check (needs list support).
    expression: "'Summary: ' + {state.summary}" # Simpler example for now
    output: final_output # Special target indicating the final result of the entire workflow.
```

### File: externals/bash.cpp
```cpp
#include <cstdio>
#include <string>
#include <sstream>
#include <stdexcept>
#include <array>
#include <iostream>
#include "json/json.h"
#include "../inc/Tool.hpp" // Include your Tool.hpp


std::string executeBashCommandReal(const Json::Value& params) {
    // 1. Parameter Validation
    if (params == Json::nullValue || params.empty()) {
        return "Error: No parameters provided. Please provide a command parameter.";
    }
    if (!params.isMember("command")) {
        return "Error: Missing required parameter 'command'.";
    }
    if (!params["command"].isString()) {
        return "Error: Parameter 'command' must be a string.";
    }

    std::string command = params["command"].asString();
    if (command.empty()) {
        return "Error: 'command' parameter cannot be empty.";
    }

    std::cout << "Executing command: " << command << std::endl;

    // --- Command Execution Logic ---
    std::string full_command = command + " 2>&1"; // Redirect stderr to stdout
    std::array<char, 128> buffer;
    std::string result; // Use string directly for efficiency
    FILE* pipe = nullptr;

    // Use popen to execute the command and open a pipe to read its output
    pipe = popen(full_command.c_str(), "r");
    if (!pipe) {
        return "Error: popen() failed to execute command: " + command;
    }

    // Read the output from the pipe line by line
    try {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data(); // Append directly to the string
        }
    } catch (const std::exception& e) { // Catch specific exceptions
        pclose(pipe);
        return "Error: Exception caught while reading command output: " + std::string(e.what());
    } catch (...) { // Catch any other exceptions
        pclose(pipe);
        return "Error: Unknown exception caught while reading command output.";
    }


    int exit_status = pclose(pipe);


    result += "\n--- Exit Status: " + std::to_string(WEXITSTATUS(exit_status)) + " ---";

    return result;
}
```

### File: externals/cal-events.cpp
```cpp

#include <algorithm>  // For std::sort, std::find_if
#include <chrono>     // For generating basic IDs
#include <filesystem> // Requires C++17
#include <fstream>
#include <iomanip> // For formatting time in IDs
#include <iostream>
#include <mutex> // For protecting file access
#include <sstream>
#include <string>
#include <vector>

#include "json/json.h" // Already used by the agent/server
// #include "../inc/Tool.hpp" // Tool definition - included via server.cpp
// indirectly

// --- Data Structure for Events ---
struct CalendarEvent {
  std::string id;   // Unique identifier for the event
  std::string date; // Format: YYYY-MM-DD
  std::string time; // Format: HH:MM (or empty for all-day)
  std::string description;

  // For sorting events
  bool operator<(const CalendarEvent &other) const {
    if (date != other.date) {
      return date < other.date;
    }
    // Treat empty time as earlier than any specific time for sorting purposes
    if (time.empty() && !other.time.empty())
      return true;
    if (!time.empty() && other.time.empty())
      return false;
    // If both have times or both are empty, compare times lexicographically
    return time < other.time;
  }
};

// --- Globals for this Tool (Protected by Mutex) ---
const std::string CALENDAR_DATA_FILE = ".calendar_data.json";
std::mutex calendarMutex; // Mutex to protect file I/O and in-memory data if
                          // needed concurrently

// --- Helper Functions ---

// Generate a reasonably unique ID (Timestamp + simple counter/random part)
std::string generateEventId() {
  auto now = std::chrono::system_clock::now();
  auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
  auto epoch = now_ms.time_since_epoch();
  long long timestamp_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();

  // Add a small random element to reduce collision chance if called rapidly
  static int counter = 0;
  std::stringstream ss;
  ss << timestamp_ms << "-" << (counter++);
  return ss.str();
}

// Load events from the JSON file
std::vector<CalendarEvent> loadEventsFromFile() {
  std::vector<CalendarEvent> events;
  std::ifstream file(CALENDAR_DATA_FILE);

  if (!file.is_open()) {
    // File might not exist yet, which is fine on first run
    if (std::filesystem::exists(CALENDAR_DATA_FILE)) {
      std::cerr << "[WARN] calendarTool: Could not open '" << CALENDAR_DATA_FILE
                << "' for reading." << std::endl;
    } else {
      std::cout << "[INFO] calendarTool: Data file '" << CALENDAR_DATA_FILE
                << "' not found. Starting fresh." << std::endl;
    }
    return events; // Return empty vector
  }

  Json::Value root;
  Json::Reader reader;
  if (!reader.parse(file, root)) {
    std::cerr << "[ERROR] calendarTool: Failed to parse JSON from '"
              << CALENDAR_DATA_FILE
              << "'. Error: " << reader.getFormattedErrorMessages()
              << std::endl;
    file.close();
    // Consider returning an error or throwing, but returning empty might be
    // safer for agent flow
    return events; // Return empty vector on parse error
  }
  file.close();

  if (!root.isArray()) {
    std::cerr << "[ERROR] calendarTool: JSON root in '" << CALENDAR_DATA_FILE
              << "' is not an array." << std::endl;
    return events;
  }

  for (const auto &item : root) {
    try {
      CalendarEvent event;
      event.id = item.get("id", generateEventId())
                     .asString(); // Add ID generation if missing
      event.date = item["date"].asString();
      event.time = item.get("time", "").asString(); // Optional time
      event.description = item["description"].asString();
      // Basic validation could happen here (e.g., date format)
      events.push_back(event);
    } catch (const Json::Exception &e) {
      std::cerr
          << "[WARN] calendarTool: Skipping invalid event item in JSON. Error: "
          << e.what() << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "[WARN] calendarTool: Skipping event due to std exception: "
                << e.what() << std::endl;
    }
  }

  // Sort events after loading
  std::sort(events.begin(), events.end());

  return events;
}

// Save events to the JSON file
bool saveEventsToFile(const std::vector<CalendarEvent> &events) {
  Json::Value root(Json::arrayValue); // Create a JSON array

  for (const auto &event : events) {
    Json::Value item;
    item["id"] = event.id;
    item["date"] = event.date;
    if (!event.time.empty()) { // Only include time if it's set
      item["time"] = event.time;
    }
    item["description"] = event.description;
    root.append(item);
  }

  std::ofstream file(CALENDAR_DATA_FILE, std::ios::trunc); // Overwrite the file
  if (!file.is_open()) {
    std::cerr << "[ERROR] calendarTool: Could not open '" << CALENDAR_DATA_FILE
              << "' for writing." << std::endl;
    return false;
  }

  // Use StyledWriter for readability, FastWriter for compactness
  // Json::StyledWriter writer;
  Json::FastWriter writer;
  file << writer.write(root);

  if (file.fail() || file.bad()) {
    std::cerr << "[ERROR] calendarTool: Failed writing events to '"
              << CALENDAR_DATA_FILE << "'." << std::endl;
    file.close();
    return false;
  }

  file.close();
  return true;
}

// Basic date format validation (YYYY-MM-DD)
bool isValidDate(const std::string &date) {
  if (date.length() != 10)
    return false;
  // Very basic check - doesn't validate day/month ranges or leap years
  return date[4] == '-' && date[7] == '-' && isdigit(date[0]) &&
         isdigit(date[1]) && isdigit(date[2]) && isdigit(date[3]) &&
         isdigit(date[5]) && isdigit(date[6]) && isdigit(date[8]) &&
         isdigit(date[9]);
}

// Basic time format validation (HH:MM)
bool isValidTime(const std::string &time) {
  if (time.empty())
    return true; // Empty time is valid (all-day)
  if (time.length() != 5)
    return false;
  // Basic check - doesn't validate hour/minute ranges
  return time[2] == ':' && isdigit(time[0]) && isdigit(time[1]) &&
         isdigit(time[3]) && isdigit(time[4]);
}

// --- Tool Implementation ---

// Tool Function: Manages calendar events (add, list).
// Input: JSON object with "action" ("add" or "list") and corresponding
// parameters. Output: Success or error message string.
std::string calendarTool(const Json::Value &params) {
  std::lock_guard<std::mutex> lock(calendarMutex); // Protect file access

  // 1. Validate overall parameters
  if (params == Json::nullValue || params.empty() || !params.isObject()) {
    return "Error: calendarTool requires a JSON object with parameters.";
  }
  if (!params.isMember("action") || !params["action"].isString()) {
    return "Error: Missing or invalid required string parameter 'action' "
           "('add' or 'list').";
  }

  std::string action = params["action"].asString();
  std::vector<CalendarEvent> events =
      loadEventsFromFile(); // Load current events

  // 2. Dispatch based on action
  if (action == "add") {
    // --- Add Event Action ---
    if (!params.isMember("date") || !params["date"].isString()) {
      return "Error [add action]: Missing or invalid required string parameter "
             "'date' (YYYY-MM-DD).";
    }
    if (!params.isMember("description") || !params["description"].isString() ||
        params["description"].asString().empty()) {
      return "Error [add action]: Missing or invalid non-empty string "
             "parameter 'description'.";
    }

    CalendarEvent newEvent;
    newEvent.date = params["date"].asString();
    newEvent.description = params["description"].asString();
    newEvent.time = params.get("time", "").asString(); // Optional time

    // Validate inputs
    if (!isValidDate(newEvent.date)) {
      return "Error [add action]: Invalid date format. Expected YYYY-MM-DD. "
             "Received: '" +
             newEvent.date + "'";
    }
    if (!isValidTime(newEvent.time)) {
      return "Error [add action]: Invalid time format. Expected HH:MM or "
             "empty. Received: '" +
             newEvent.time + "'";
    }

    newEvent.id = generateEventId(); // Assign unique ID

    events.push_back(newEvent);
    std::sort(events.begin(), events.end()); // Keep sorted

    if (saveEventsToFile(events)) {
      std::stringstream successMsg;
      successMsg << "Success: Event added with ID '" << newEvent.id << "'.\n";
      successMsg << "Date: " << newEvent.date;
      if (!newEvent.time.empty())
        successMsg << " Time: " << newEvent.time;
      successMsg << "\nDescription: " << newEvent.description;
      return successMsg.str();
    } else {
      return "Error [add action]: Failed to save event to file.";
    }

  } else if (action == "list") {
    // --- List Events Action ---
    std::string filterDate = params.get("date", "").asString();
    if (!filterDate.empty() && !isValidDate(filterDate)) {
      return "Error [list action]: Invalid date format for filtering. Expected "
             "YYYY-MM-DD. Received: '" +
             filterDate + "'";
    }

    std::stringstream output;
    int count = 0;
    if (filterDate.empty()) {
      output << "All upcoming or current events:\n"; // Or just "All events"
    } else {
      output << "Events for date " << filterDate << ":\n";
    }

    for (const auto &event : events) {
      bool match = false;
      if (filterDate.empty()) {
        match = true; // List all if no date filter
      } else {
        match = (event.date == filterDate);
      }

      if (match) {
        count++;
        output << "- ID: " << event.id << " | Date: " << event.date;
        if (!event.time.empty()) {
          output << " Time: " << event.time;
        }
        output << " | Desc: " << event.description << "\n";
      }
    }

    if (count == 0) {
      if (filterDate.empty()) {
        output << "(No events scheduled)\n";
      } else {
        output << "(No events found for this date)\n";
      }
    }

    std::string result = output.str();
    if (!result.empty() && result.back() == '\n') { // Trim trailing newline
      result.pop_back();
    }
    return result;

  } else {
    // --- Unknown Action ---
    return "Error: Unknown action '" + action +
           "'. Supported actions: 'add', 'list'.";
  }
}
```

### File: externals/file.cpp
```cpp
// externals/file.cpp
// Rewritten for better security and robustness.

#include <chrono>     // For time formatting in 'info'
#include <cstdlib>    // For std::getenv
#include <filesystem> // Requires C++17
#include <fstream>
#include <iomanip>  // For time formatting in 'info'
#include <iostream> // For cerr, cout (debugging)
#include <optional> // For optional path return
#include <sstream>
#include <string>
#include <system_error> // For std::error_code
#include <vector>

#include "json/json.h" // Provided JSON library

// Include Agent.hpp for LogLevel enum and logMessage function declaration
#include "../inc/Agent.hpp"

namespace fs = std::filesystem;

namespace { // Use an anonymous namespace for internal helpers

const char *AGENT_WORKSPACE_ENV_VAR = "AGENT_WORKSPACE";
const char *DEFAULT_WORKSPACE =
    "./agent_workspace"; // Default if ENV_VAR not set

// --- Helper: Get the configured workspace path ---
fs::path getWorkspacePath() {
  const char *envPath = std::getenv(AGENT_WORKSPACE_ENV_VAR);
  fs::path workspace;
  if (envPath && envPath[0] != '\0') {
    workspace = envPath;
  } else {
    workspace = DEFAULT_WORKSPACE;
    // Optional: Log that default is being used if ENV not set
    // logMessage(LogLevel::DEBUG, "AGENT_WORKSPACE env var not set, using
    // default:", workspace.string());
  }

  // Ensure the workspace directory exists, create if not
  std::error_code ec;
  if (!fs::exists(workspace, ec)) {
    if (!fs::create_directories(workspace, ec) && ec) {
      logMessage(LogLevel::ERROR,
                 "Failed to create default workspace directory",
                 workspace.string() + " | Error: " + ec.message());
      // Throw or return an empty path to indicate critical failure? Let's
      // throw.
      throw std::runtime_error("Failed to create agent workspace: " +
                               workspace.string());
    } else if (!ec) {
      logMessage(LogLevel::INFO,
                 "Created agent workspace directory:", workspace.string());
    }
  } else if (ec) {
    logMessage(LogLevel::ERROR, "Error checking workspace directory existence",
               workspace.string() + " | Error: " + ec.message());
    throw std::runtime_error("Error checking agent workspace: " +
                             workspace.string());
  } else if (!fs::is_directory(workspace, ec)) {
    logMessage(LogLevel::ERROR, "Workspace path exists but is not a directory:",
               workspace.string());
    throw std::runtime_error("Agent workspace path is not a directory: " +
                             workspace.string());
  } else if (ec) {
    logMessage(LogLevel::ERROR,
               "Error checking if workspace path is a directory",
               workspace.string() + " | Error: " + ec.message());
    throw std::runtime_error("Error checking agent workspace type: " +
                             workspace.string());
  }

  // Return the canonical workspace path for consistent comparisons
  return fs::weakly_canonical(workspace,
                              ec); // Use weakly_canonical as it handles
                                   // non-existent paths during creation check
}

// --- Helper: Resolve relative path within workspace and ensure safety ---
std::optional<fs::path>
resolvePathAndCheckSafety(const std::string &relativePathStr,
                          const fs::path &workspacePath) {
  if (relativePathStr.empty()) {
    logMessage(LogLevel::WARN, "[fileTool] Received empty relative path.");
    return std::nullopt; // Reject empty paths
  }

  // Basic initial checks on the input string
  if (relativePathStr.find("..") != std::string::npos) {
    logMessage(LogLevel::WARN, "[fileTool] Path contains '..'. Disallowed.",
               relativePathStr);
    return std::nullopt; // Disallow '..' explicitly
  }
  if (relativePathStr.find_first_of("|;&`$<>\\") != std::string::npos) {
    logMessage(LogLevel::WARN,
               "[fileTool] Path contains potentially unsafe characters.",
               relativePathStr);
    return std::nullopt;
  }

  fs::path relativePath(relativePathStr);
  if (relativePath.is_absolute()) {
    logMessage(LogLevel::WARN, "[fileTool] Absolute paths are disallowed.",
               relativePathStr);
    return std::nullopt; // Disallow absolute paths
  }

  // Combine with workspace and normalize
  fs::path combinedPath = workspacePath / relativePath;
  std::error_code ec;
  fs::path canonicalPath = fs::weakly_canonical(
      combinedPath, ec); // Normalizes '.', avoids resolving symlinks
                         // immediately if target might not exist yet

  if (ec) {
    // This might happen if parts of the path don't exist yet, which is ok for
    // write/mkdir But log it for debugging potential issues later if needed.
    // logMessage(LogLevel::DEBUG, "[fileTool] Path canonicalization resulted in
    // error (might be ok if target doesn't exist yet)", combinedPath.string() +
    // " | Error: " + ec.message()); We still need to check the *intended* path
    // for safety based on string comparison before canonicalization potentially
    // fails
    canonicalPath =
        combinedPath
            .lexically_normal(); // Use lexical normalization as fallback
  }

  // *** Security Check: Ensure the final path is WITHIN the workspace ***
  auto workspaceStr = workspacePath.string();
  auto canonicalStr = canonicalPath.string();

  // Simple string prefix check (works reliably if paths are canonical/normal)
  // Add trailing separator to workspace path if not present for robust prefix
  // check
  if (!workspaceStr.empty() &&
      workspaceStr.back() != fs::path::preferred_separator) {
    workspaceStr += fs::path::preferred_separator;
  }
  if (!canonicalStr.empty() &&
      canonicalStr.back() != fs::path::preferred_separator &&
      fs::is_directory(canonicalPath, ec)) {
    // If it's intended to be a directory, ensure check compares directory
    // prefixes correctly
    // canonicalStr += fs::path::preferred_separator; // Temporarily add for
    // comparison if needed, depends on exact check logic
  }

  if (canonicalStr.rfind(workspaceStr, 0) !=
      0) { // Check if canonicalStr starts with workspaceStr
    logMessage(LogLevel::ERROR,
               "[fileTool] Security Violation: Path escapes workspace!",
               "Workspace: " + workspacePath.string() +
                   " | Attempted Path: " + relativePathStr +
                   " | Resolved To: " + canonicalPath.string());
    return std::nullopt; // Path is outside the workspace!
  }

  logMessage(LogLevel::DEBUG,
             "[fileTool] Path resolved safely within workspace",
             canonicalPath.string());
  return canonicalPath; // Return the safe, canonical path
}

// --- Helper: Format file time ---
std::string formatFileTime(fs::file_time_type ftime) {
  try {
    // Convert file_time_type to system_clock::time_point
    auto sctp =
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() +
            std::chrono::system_clock::now());
    std::time_t ctime = std::chrono::system_clock::to_time_t(sctp);
    std::tm timeinfo =
        *std::localtime(&ctime); // Use localtime for local time zone display
    char time_buf[100];          // Increased buffer size
    // ISO 8601-like format (YYYY-MM-DDTHH:MM:SS)
    if (std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%S",
                      &timeinfo)) {
      return std::string(time_buf);
    } else {
      logMessage(LogLevel::WARN,
                 "[fileTool] Failed to format file time with strftime.");
      return "(Error formatting time)";
    }
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR,
               "[fileTool] Exception during file time formatting", e.what());
    return "(Time conversion/formatting error)";
  }
}

} // end anonymous namespace

// --- Main Tool Function ---

// Tool Function: Performs file system operations (read, write, list, info,
// delete, mkdir)
//                confined within a designated workspace directory.
// Input: JSON object like:
// {
//   "action": "read|write|list|info|delete|mkdir", // Required
//   "path": "relative/path/to/file_or_dir",        // Required (Relative ONLY)
//   "content": "text content"                     // Required ONLY for "write"
//   action
// }
// Output: Operation result (content, listing, info) or success/error message
// string.
std::string fileTool(const Json::Value &params) {
  fs::path workspacePath;
  try {
    workspacePath = getWorkspacePath();
    logMessage(LogLevel::DEBUG,
               "[fileTool] Using workspace:", workspacePath.string());
  } catch (const std::exception &e) {
    return "Error [fileTool]: Critical failure getting or creating "
           "workspace: " +
           std::string(e.what());
  }

  // 1. Parameter Validation
  if (params == Json::nullValue || !params.isObject()) {
    return "Error [fileTool]: Requires a JSON object with parameters.";
  }
  if (!params.isMember("action") || !params["action"].isString()) {
    return "Error [fileTool]: Missing or invalid required string parameter "
           "'action'. "
           "Valid actions: read, write, list, info, delete, mkdir.";
  }
  std::string action = params["action"].asString();

  if (!params.isMember("path") || !params["path"].isString()) {
    return "Error [fileTool]: Missing or invalid required string parameter "
           "'path'. Path must be relative to the agent workspace.";
  }
  std::string relativePathStr = params["path"].asString();

  std::string content = ""; // Needed only for write
  if (action == "write") {
    if (!params.isMember("content") || !params["content"].isString()) {
      return "Error [fileTool:write]: Missing or invalid string parameter "
             "'content' when action is 'write'.";
    }
    content = params["content"].asString();
  }

  // 2. Path Resolution and Safety Check
  std::optional<fs::path> safePathOpt =
      resolvePathAndCheckSafety(relativePathStr, workspacePath);
  if (!safePathOpt) {
    return "Error [fileTool]: Invalid or unsafe path specified: '" +
           relativePathStr +
           "'. Path must be relative and within the workspace.";
  }
  fs::path safePath = *safePathOpt;

  logMessage(LogLevel::DEBUG, "[fileTool] Processing action '" + action +
                                  "' on safe path '" + safePath.string() + "'");

  // 3. Action Dispatching
  try {
    std::error_code ec;

    // === READ ===
    if (action == "read") {
      // Check existence and type
      if (!fs::exists(safePath, ec)) {
        return "Error [fileTool:read]: Path not found: '" + relativePathStr +
               "'";
      }
      if (ec)
        return "Error [fileTool:read]: Cannot check existence: " + ec.message();
      if (!fs::is_regular_file(safePath, ec)) {
        return "Error [fileTool:read]: Path is not a regular file: '" +
               relativePathStr + "'";
      }
      if (ec)
        return "Error [fileTool:read]: Cannot check type: " + ec.message();

      // Open and read
      std::ifstream fileStream(
          safePath, std::ios::binary); // Use binary to read bytes accurately
      if (!fileStream.is_open()) {
        return "Error [fileTool:read]: Could not open file '" +
               relativePathStr + "' (permissions?).";
      }
      std::stringstream buffer;
      buffer << fileStream.rdbuf();
      if (fileStream.bad()) {
        fileStream.close();
        return "Error [fileTool:read]: I/O error reading file '" +
               relativePathStr + "'.";
      }
      fileStream.close();
      logMessage(LogLevel::INFO, "[fileTool:read] Successfully read file",
                 relativePathStr);
      return "Success [fileTool:read]: Content of '" + relativePathStr +
             "':\n" + buffer.str();
    }

    // === WRITE ===
    else if (action == "write") {
      fs::path parentDir = safePath.parent_path();
      ec.clear();
      // Ensure parent directory exists (create if needed, within workspace
      // check already done)
      if (!parentDir.empty() && !fs::exists(parentDir, ec)) {
        if (!fs::create_directories(parentDir, ec) &&
            ec) { // Check error code *after* attempt
          return "Error [fileTool:write]: Failed to create parent directory '" +
                 parentDir.lexically_relative(workspacePath).string() +
                 "': " + ec.message();
        } else if (!ec) {
          logMessage(LogLevel::INFO,
                     "[fileTool:write] Created parent directory",
                     parentDir.lexically_relative(workspacePath).string());
        }
      } else if (ec) {
        return "Error [fileTool:write]: Cannot check parent existence: " +
               ec.message();
      }

      // Check if path exists and is a directory (cannot overwrite dir with
      // file)
      ec.clear();
      if (fs::exists(safePath, ec) && !ec && fs::is_directory(safePath, ec) &&
          !ec) {
        return "Error [fileTool:write]: Path '" + relativePathStr +
               "' exists and is a directory.";
      }

      // Open and write (overwrite/truncate)
      std::ofstream fileStream(safePath, std::ios::trunc | std::ios::binary);
      if (!fileStream.is_open()) {
        return "Error [fileTool:write]: Could not open file '" +
               relativePathStr + "' for writing (permissions?).";
      }
      fileStream << content;
      fileStream.flush(); // Ensure data is flushed
      if (fileStream.fail() || fileStream.bad()) {
        fileStream.close();
        return "Error [fileTool:write]: Failed writing content to '" +
               relativePathStr + "' (I/O error? Disk full?).";
      }
      fileStream.close();
      logMessage(LogLevel::INFO, "[fileTool:write] Successfully wrote to file",
                 relativePathStr);
      return "Success [fileTool:write]: Content written to file '" +
             relativePathStr + "'";
    }

    // === LIST ===
    else if (action == "list") {
      // Check existence and type
      if (!fs::exists(safePath, ec)) {
        return "Error [fileTool:list]: Path not found: '" + relativePathStr +
               "'";
      }
      if (ec)
        return "Error [fileTool:list]: Cannot check existence: " + ec.message();
      if (!fs::is_directory(safePath, ec)) {
        return "Error [fileTool:list]: Path is not a directory: '" +
               relativePathStr + "'";
      }
      if (ec)
        return "Error [fileTool:list]: Cannot check type: " + ec.message();

      std::stringstream listing;
      listing << "Listing of directory '" << relativePathStr << "':\n";
      int count = 0;
      const int max_list_entries = 200; // Limit output size
      ec.clear();
      // Iterate through directory, skipping permission errors
      for (const auto &entry : fs::directory_iterator(
               safePath, fs::directory_options::skip_permission_denied, ec)) {
        if (ec) {
          listing << "[Warning: Error reading an entry: " << ec.message()
                  << "]\n";
          ec.clear(); // Clear error to continue listing other entries
          continue;
        }
        if (count >= max_list_entries) {
          listing << "[Info: Reached maximum entry limit (" << max_list_entries
                  << ")]\n";
          break;
        }
        try {
          std::string entryName = entry.path().filename().string();
          listing << "- " << entryName;
          std::error_code entry_ec; // Local ec for checking entry type
          if (entry.is_directory(entry_ec) && !entry_ec)
            listing << "/";
          else if (entry.is_symlink(entry_ec) && !entry_ec)
            listing << "@"; // Indicate symlink
          else if (entry_ec)
            listing << " [Error checking type: " << entry_ec.message() << "]";
        } catch (const std::exception &e) {
          listing << "- [Error accessing entry properties: " << e.what() << "]";
        }
        listing << "\n";
        count++;
      }
      if (ec)
        listing
            << "[Warning: Stopped listing due to directory iteration error: "
            << ec.message() << "]\n";
      if (count == 0 && !ec)
        listing << "(Directory is empty or inaccessible)\n";

      std::string result = listing.str();
      if (!result.empty() && result.back() == '\n')
        result.pop_back(); // Trim trailing newline
      logMessage(LogLevel::INFO,
                 "[fileTool:list] Successfully listed directory",
                 relativePathStr);
      return result;
    }

    // === INFO ===
    else if (action == "info") {
      if (!fs::exists(safePath, ec)) {
        return "Error [fileTool:info]: Path not found: '" + relativePathStr +
               "'";
      }
      if (ec)
        return "Error [fileTool:info]: Cannot check existence: " + ec.message();

      std::stringstream info;
      info << "Information for path '" << relativePathStr << "':\n";
      ec.clear();
      auto status = fs::symlink_status(
          safePath, ec); // Use symlink_status to identify symlinks
      if (ec)
        return "Error [fileTool:info]: Could not get status for '" +
               relativePathStr + "': " + ec.message();

      info << "- Type: ";
      if (fs::is_regular_file(status))
        info << "File";
      else if (fs::is_directory(status))
        info << "Directory";
      else if (fs::is_symlink(status)) {
        info << "Symbolic Link";
        ec.clear();
        fs::path target = fs::read_symlink(safePath, ec);
        if (!ec)
          info << " -> '" << target.string() << "'";
        else
          info << " [Error reading target]";
      } else if (fs::is_block_file(status))
        info << "Block Device";
      else if (fs::is_character_file(status))
        info << "Character Device";
      else if (fs::is_fifo(status))
        info << "FIFO/Pipe";
      else if (fs::is_socket(status))
        info << "Socket";
      else
        info << "Other/Unknown";
      info << "\n";

      // Get size only for regular files (following symlinks)
      ec.clear();
      if (fs::is_regular_file(safePath, ec) &&
          !ec) { // Checks the target if it's a symlink
        uintmax_t size = fs::file_size(safePath, ec);
        if (ec)
          info << "- Size: N/A [Error: " << ec.message() << "]\n";
        else
          info << "- Size: " << size << " bytes\n";
      } else if (ec) {
        info << "- Size: N/A [Error checking type: " << ec.message() << "]\n";
      }

      ec.clear();
      auto ftime = fs::last_write_time(safePath, ec);
      if (ec)
        info << "- Last Modified: N/A [Error: " << ec.message() << "]\n";
      else
        info << "- Last Modified: " << formatFileTime(ftime) << "\n";

      // Permissions (basic example)
      // fs::perms p = status.permissions();
      // info << "- Permissions: " << std::oct << static_cast<int>(p) <<
      // std::dec << "\n";

      std::string result = info.str();
      if (!result.empty() && result.back() == '\n')
        result.pop_back();
      logMessage(LogLevel::INFO,
                 "[fileTool:info] Successfully retrieved info for",
                 relativePathStr);
      return result;
    }

    // === DELETE ===
    else if (action == "delete") {
      // Check existence first
      if (!fs::exists(safePath, ec)) {
        return "Success [fileTool:delete]: Path '" + relativePathStr +
               "' did not exist.";
      }
      if (ec)
        return "Error [fileTool:delete]: Cannot check existence: " +
               ec.message();

      ec.clear();
      bool is_dir = fs::is_directory(safePath, ec);
      if (ec)
        return "Error [fileTool:delete]: Cannot determine type of '" +
               relativePathStr + "': " + ec.message();

      // Refuse to delete non-empty directories for safety
      if (is_dir) {
        ec.clear();
        bool empty = fs::is_empty(safePath, ec);
        if (ec)
          return "Error [fileTool:delete]: Cannot check if directory '" +
                 relativePathStr + "' is empty: " + ec.message();
        if (!empty)
          return "Error [fileTool:delete]: Directory '" + relativePathStr +
                 "' is not empty. Cannot delete non-empty directories.";
      }

      ec.clear();
      bool removed = fs::remove(
          safePath, ec); // fs::remove handles both files and empty directories
      if (ec)
        return "Error [fileTool:delete]: Failed to delete '" + relativePathStr +
               "': " + ec.message();
      else if (!removed &&
               fs::exists(
                   safePath)) { // Double check if removal failed silently
        return "Error [fileTool:delete]: Delete reported no error for '" +
               relativePathStr + "', but path still exists (permissions?).";
      } else {
        logMessage(LogLevel::INFO, "[fileTool:delete] Successfully deleted",
                   relativePathStr);
        return "Success [fileTool:delete]: Path '" + relativePathStr +
               "' deleted.";
      }
    }

    // === MKDIR ===
    else if (action == "mkdir") {
      if (fs::exists(safePath, ec)) {
        return "Error [fileTool:mkdir]: Path '" + relativePathStr +
               "' already exists.";
      }
      if (ec)
        return "Error [fileTool:mkdir]: Cannot check existence: " +
               ec.message();

      fs::path parentDir = safePath.parent_path();
      // Ensure parent directory exists (create if needed) - workspace check
      // already done
      ec.clear();
      if (!parentDir.empty() && parentDir != workspacePath &&
          !fs::exists(parentDir, ec)) {
        if (!fs::create_directories(parentDir, ec) && ec) {
          return "Error [fileTool:mkdir]: Failed to create parent directory '" +
                 parentDir.lexically_relative(workspacePath).string() +
                 "': " + ec.message();
        } else if (!ec) {
          logMessage(LogLevel::INFO,
                     "[fileTool:mkdir] Created parent directory",
                     parentDir.lexically_relative(workspacePath).string());
        }
      } else if (ec) {
        return "Error [fileTool:mkdir]: Cannot check parent existence: " +
               ec.message();
      }

      ec.clear();
      if (fs::create_directory(safePath, ec)) {
        logMessage(LogLevel::INFO,
                   "[fileTool:mkdir] Successfully created directory",
                   relativePathStr);
        return "Success [fileTool:mkdir]: Directory '" + relativePathStr +
               "' created.";
      } else {
        return "Error [fileTool:mkdir]: Failed to create directory '" +
               relativePathStr + "'" + (ec ? ": " + ec.message() : ".");
      }
    }

    // === Unknown Action ===
    else {
      logMessage(LogLevel::WARN, "[fileTool] Received unknown action", action);
      return "Error [fileTool]: Unknown action '" + action +
             "'. Valid actions: read, write, list, info, delete, mkdir.";
    }

  } catch (const fs::filesystem_error &e) {
    // Log filesystem errors specifically
    std::string path1_info =
        e.path1().empty() ? "" : " | Path1: '" + e.path1().string() + "'";
    std::string path2_info =
        e.path2().empty() ? "" : " | Path2: '" + e.path2().string() + "'";
    logMessage(LogLevel::ERROR,
               "[fileTool] Filesystem Error for action '" + action +
                   "' on path '" + relativePathStr + "'",
               std::string(e.what()) + path1_info + path2_info +
                   " | Code: " + std::to_string(e.code().value()));
    return "Error [fileTool:" + action + "] Filesystem error processing '" +
           relativePathStr + "': " + std::string(e.what());
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR,
               "[fileTool] Standard Exception for action '" + action +
                   "' on path '" + relativePathStr + "'",
               e.what());
    return "Error [fileTool:" + action + "] Exception processing '" +
           relativePathStr + "': " + std::string(e.what());
  } catch (...) {
    logMessage(LogLevel::ERROR,
               "[fileTool] Unknown internal error occurred for action '" +
                   action + "' on path '" + relativePathStr + "'");
    return "Error [fileTool:" + action +
           "] Unknown internal error occurred processing '" + relativePathStr +
           "'.";
  }
}

// --- Helper Function Implementation ---
// (Already defined within the anonymous namespace above)
// std::string formatFileTime(fs::file_time_type ftime) { ... }
```

### File: externals/general.cpp
```cpp

#include "json/json.h" // For parsing the JSON response
#include <cstdlib>     // For std::getenv
#include <curl/curl.h> // For making HTTP requests
#include <iostream>
#include <memory> // For unique_ptr
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Gets current time (params ignored)
std::string getTime(const Json::Value &params) {
  (void)params; // Unused
  time_t now = time(0);
  char buf[80];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
  return buf;
}

// Rolls a die (uses params)
std::string rollDice(const Json::Value &params) {
  int sides = 6; // Default
  int rolls = 1; // Default
  if (params.isMember("sides") && params["sides"].isInt()) {
    int requestedSides = params["sides"].asInt();
    if (requestedSides > 0) {
      sides = requestedSides;
    }
  }
  std::ostringstream oss;
  if (params.isMember("rolls") && params["rolls"].isInt()) {
    rolls = params["rolls"].asInt();
    while (rolls > 0) {
      int roll = (rand() % sides) + 1;
      oss << "Rolled a " << roll << " (d" << sides << ")";
      rolls--;
    }
  }
  return oss.str();
}

std::string calculate(const Json::Value &params) {
  if (params == Json::nullValue) {
    return "No parameters provided.";
  } else if (params.isMember("expression")) {
    std::string expression = params["expression"].asString();
    // Simple parsing and evaluation logic (for demonstration)
    // In a real scenario, you would use a proper expression parser
    std::istringstream iss(expression);
    double a, b;
    char op;
    iss >> a >> op >> b;
    double result = 0.0;
    switch (op) {
    case '+':
      result = a + b;
      break;
    case '-':
      result = a - b;
      break;
    case '*':
      result = a * b;
      break;
    case '/':
      result = (b != 0) ? a / b : 0;
      break;
    default:
      return "Invalid operator.";
    }
    return "Result: " + std::to_string(result);
  }
  return "Invalid parameters.";
}
```

### File: externals/sway.cpp
```cpp
// externals/sway_control.cpp

#include <string>
#include <cstdio>       // For popen, pclose, fgets
#include <sstream>
#include <stdexcept>
#include <sys/wait.h>   // For WEXITSTATUS
#include <iostream>     // For logging/debugging

#include "json/json.h"
// Assuming LogLevel enum and logMessage function are accessible globally or via includes
// If not, include the necessary header or pass a logger instance.
enum class LogLevel { DEBUG, INFO, WARN, ERROR }; // Placeholder if not included
void logMessage(LogLevel level, const std::string &message, const std::string &details = ""); // Placeholder

// Helper to execute command and capture output (similar to agent.cpp's version)
int executeSwayCommand(const std::string &sway_args, std::string &output) {
    output.clear();
    // Construct the full command
    std::string full_command = "swaymsg " + sway_args + " 2>&1"; // Redirect stderr to stdout
    logMessage(LogLevel::DEBUG, "[swayControlTool] Executing:", full_command);

    FILE *pipe = popen(full_command.c_str(), "r");
    if (!pipe) {
        logMessage(LogLevel::ERROR, "[swayControlTool] popen() failed for command:", full_command);
        output = "Error: Failed to execute swaymsg command (popen failed).";
        return -1; // Indicate pipe creation failure
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }

    int status = pclose(pipe);
    logMessage(LogLevel::DEBUG, "[swayControlTool] Command finished. Exit Status (raw):", std::to_string(status));

    if (status == -1) {
         logMessage(LogLevel::ERROR, "[swayControlTool] pclose() failed.");
         // Output might contain partial data, keep it.
         output += "\nError: pclose failed after running command.";
         return -1; // Indicate pclose failure
    } else {
        // Return the actual exit status of the swaymsg command
        return WEXITSTATUS(status);
    }
}


// Tool Function: Executes a swaymsg command.
// Input: JSON object like: {"command": "swaymsg arguments here"}
// Output: Success/error message string including swaymsg output.
std::string swayControlTool(const Json::Value& params) {
    // 1. Parameter Validation
    if (params == Json::nullValue || !params.isObject()) {
        return "Error [swayControlTool]: Requires a JSON object with parameters.";
    }
    if (!params.isMember("command") || !params["command"].isString()) {
        return "Error [swayControlTool]: Missing or invalid required string parameter 'command'.";
    }
    std::string swayArgs = params["command"].asString();
    if (swayArgs.empty()) {
        return "Error [swayControlTool]: Parameter 'command' cannot be empty.";
    }

    // Basic sanity check - avoid obviously dangerous characters if desired, though
    // swaymsg commands themselves can be complex. This is minimal.
    // if (swayArgs.find_first_of(";&|`$<>") != std::string::npos) {
    //     return "Error [swayControlTool]: Command contains potentially unsafe shell metacharacters.";
    // }

    // 2. Execute Command
    std::string output;
    int exitStatus = executeSwayCommand(swayArgs, output);

    // 3. Format Response
    std::stringstream result;
    result << "--- Sway Control Result ---\n";
    result << "Executed: swaymsg " << swayArgs << "\n";
    result << "Exit Status: " << exitStatus << "\n";

    if (exitStatus == 0) {
        result << "Status: Success\n";
        logMessage(LogLevel::INFO, "[swayControlTool] Command succeeded:", swayArgs);
    } else {
        result << "Status: Failed\n";
         logMessage(LogLevel::WARN, "[swayControlTool] Command failed:", "Command: swaymsg " + swayArgs + " | Exit Status: " + std::to_string(exitStatus));
    }

    result << "Output:\n" << (!output.empty() ? output : "(No output)");

    return result.str();
}

// Placeholder implementations if LogLevel/logMessage aren't globally available
// enum class LogLevel { DEBUG, INFO, WARN, ERROR };
// void logMessage(LogLevel level, const std::string &message, const std::string &details = "") {
//     const char* levelStr[] = {"DEBUG", "INFO", "WARN", "ERROR"};
//     std::cerr << "[" << levelStr[static_cast<int>(level)] << "] " << message;
//     if (!details.empty()) std::cerr << " | Details: " << details;
//     std::cerr << std::endl;
// }
```

### File: externals/write.cpp
```cpp

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>     // For cerr, cout (debugging)
#include <filesystem>   // Requires C++17
#include <system_error> // For std::error_code
#include "json/json.h"  // Provided JSON library

namespace fs = std::filesystem;

// Tool Function: Writes content to a file specified by path.
// Input: JSON object like: {"path": "relative/path/to/file.txt", "content": "File content here"}
// Output: A success message or an error message string.
std::string writeFileTool(const Json::Value& params) {
    // 1. Parameter Validation
    if (params == Json::nullValue || !params.isObject()) {
        return "Error [writeFile]: Requires a JSON object with parameters.";
    }
    if (!params.isMember("path") || !params["path"].isString()) {
        return "Error [writeFile]: Missing or invalid required string parameter 'path'.";
    }
    std::string path_str = params["path"].asString();
    if (path_str.empty()) {
        return "Error [writeFile]: Parameter 'path' cannot be empty.";
    }

    // Content is optional, allow writing empty files
    std::string content = "";
    if (params.isMember("content")) {
         if (!params["content"].isString()) {
             return "Error [writeFile]: Optional parameter 'content' must be a string if provided.";
         }
         content = params["content"].asString();
    } else {
         std::cout << "[TOOL_DEBUG] writeFileTool: No 'content' provided for path '" << path_str << "'. Writing empty file." << std::endl;
    }


    // 2. Security Checks (Crucial!) - Reusing checks similar to file.cpp
    if (path_str.find("..") != std::string::npos) {
        return "Error [writeFile]: Invalid file path. Directory traversal ('..') is not allowed.";
    }
    fs::path filePath;
    try {
        filePath = path_str; // Convert to filesystem path
    } catch (const std::exception& e) {
         return "Error [writeFile]: Invalid path format '" + path_str + "': " + e.what();
    }

    if (filePath.is_absolute()) {
       return "Error [writeFile]: Invalid file path. Absolute paths are not allowed.";
    }
    // Basic check for potentially unsafe characters (adjust as needed)
    if (path_str.find_first_of("|;&`$<>\\") != std::string::npos) {
         return "Error [writeFile]: Invalid file path. Path contains potentially unsafe characters.";
    }

    std::cout << "[TOOL_DEBUG] writeFileTool attempting to write " << content.length() << " bytes to: '" << path_str << "'" << std::endl;

    // 3. Ensure parent directory exists and target is not a directory
    try {
        fs::path parentDir = filePath.parent_path();
        std::error_code ec;

        // Create parent directories if they don't exist
        if (!parentDir.empty() && !fs::exists(parentDir, ec)) {
             if (!fs::create_directories(parentDir, ec) && ec) { // Check error code after attempt
                  return "Error [writeFile]: Failed to create parent directory '" + parentDir.string() + "': " + ec.message();
             }
             std::cout << "[TOOL_DEBUG] writeFileTool: Created parent directory: " << parentDir.string() << std::endl;
        } else if (ec) {
             return "Error [writeFile]: Failed to check existence of parent directory '" + parentDir.string() + "': " + ec.message();
        }

         // Check if the target path itself exists and is a directory
         ec.clear(); // Reset error code
         if (fs::exists(filePath, ec) && !ec && fs::is_directory(filePath, ec) && !ec) {
             return "Error [writeFile]: Path '" + path_str + "' already exists and is a directory. Cannot overwrite a directory with a file.";
         } else if (ec) {
              // Non-critical error if checking the target path fails initially, file open will likely catch it too.
              std::cerr << "[WARN] writeFileTool: Could not definitively check target path status '" << path_str << "': " << ec.message() << std::endl;
         }

    } catch (const fs::filesystem_error& e) {
         return "Error [writeFile]: Filesystem error checking/creating directories for '" + path_str + "': " + e.what();
    } catch (const std::exception& e) {
         return "Error [writeFile]: Unexpected error during directory setup for '" + path_str + "': " + e.what();
    }


    // 4. Open file for writing (creates if not exists, truncates/overwrites if exists)
    // Use binary mode? For text, default is usually fine. Use std::ios::binary if needed.
    std::ofstream fileStream(filePath, std::ios::trunc);
    if (!fileStream.is_open()) {
        // Provide more context if possible
        return "Error [writeFile]: Could not open file for writing: '" + path_str + "'. Check path validity and permissions.";
    }

    // 5. Write content and handle errors
    try {
        fileStream << content;
        fileStream.flush(); // Ensure content is written to OS buffer

        if (fileStream.fail() || fileStream.bad()) {
            // Check stream state after writing/flushing
             std::string errorMsg = "Error [writeFile]: Failed to write content to file '" + path_str + "'. Disk full, permissions issue, or other I/O error?";
             fileStream.close(); // Attempt to close even on failure
             return errorMsg;
        }

    } catch (const std::ios_base::failure& e) { // Catch specific stream exceptions
        fileStream.close();
        return "Error [writeFile]: I/O exception while writing to file '" + path_str + "': " + e.what();
    } catch (const std::exception& e) {
        fileStream.close();
        return "Error [writeFile]: General exception while writing to file '" + path_str + "': " + e.what();
    } catch (...) {
        fileStream.close();
        return "Error [writeFile]: Unknown exception while writing to file '" + path_str + "'.";
    }

    fileStream.close(); // Close the file on success

    // Optional: Verify write by checking file size or existence again, but usually overkill
    std::cout << "[TOOL_DEBUG] writeFileTool successfully wrote to '" << path_str << "'" << std::endl;
    return "Success [writeFile]: Content successfully written to file '" + path_str + "'";
}
```

### File: GOALS.md
```markdown
- [ ] Create a flexible agent that can be easily extended with new tools.
- [ ] Implement a robust and reliable system.
- [ ] Provide a user-friendly interface for interacting with the agent.```

### File: import.main.cpp
```cpp
#include "inc/Agent.hpp"
#include "inc/Import.hpp"
#include "inc/MiniGemini.hpp"
#include "inc/Tool.hpp"
#include "inc/modelApi.hpp" // For ApiError
#include "json/json.h"      // For Json::Value used by tools
#include <curl/curl.h>      // For curl_global_init/cleanup
#include <memory>           // For Tool pointers (optional but good practice)
#include <stdexcept>
#include <string>
#include <vector>

// --- Example Usage ---
int	main(void)
{
    MiniGemini	myApi;

	// Assume myApi is an initialized MiniGemini instance
	Agent agent1(myApi);
	// Load configuration into agent1
	if (loadAgentProfile(agent1, "./config/agents/standard.yaml"))
	{
		std::cout << "Agent Name after load: " << agent1.getName() << std::endl;
		// std::cout << "listing conf after load "<< std::endl;
		// agent1.listAllAgentInfo();
		// Modify the agent slightly
		// agent1.addEnvVar("SESSION_ID", "xyz789");
		// agent1.addTask("Final review step");
		// test while to chat.
		while (true)
		{
			std::string userInput;
			std::cout << "User: ";
			std::getline(std::cin, userInput);
			if (userInput == "exit")
				break ;                // Exit condition
			agent1.prompt(userInput); // Assuming chat method exists
		}
		// // Save the modified configuration (profile aspects only)
		// if (saveAgentProfile(agent1,
				// "./agents/order_processor_modified.yaml")) {
		//     std::cout << "Modified profile saved." << std::endl;
		// } else {
		//     std::cerr << "Failed to save modified profile." << std::endl;
		// }
	}
	else
	{
		std::cerr << "Failed to load agent profile." << std::endl;
	}
	return (0);
}
```

### File: inc/Agent.hpp
```cpp
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
#include <stack>

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


// class Task
// {
//
// };
//
// struct Tasks
// {
//     std::stac
//
// };

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
```

### File: inc/File.hpp
```cpp
#ifndef FILE_HPP // Include guard start
#define FILE_HPP

#include <algorithm> // For std::remove, std::find
#include <cstddef>   // For size_t
#include <fstream>   // For std::ifstream, std::ofstream
#include <iostream>  // For std::ostream
#include <sstream>   // For std::ostringstream
#include <stdexcept> // For std::runtime_error
#include <string>
#include <vector>

// Forward declaration for the friend function
class File;
std::ostream &operator<<(std::ostream &os, const File &f);

class File {
public:
  // Default constructor (C++98 style)
  // Initializes members to reasonable defaults.
  File() : path_(""), content_(""), description_("") {
    // tags_ is default-constructed to an empty vector
  }

  // Constructor to load from a file path (C++98 style)
  // Use explicit to prevent unintentional conversions
  explicit File(const std::string &filePath)
      : path_(filePath), content_(""), description_("") {
    // tags_ is default-constructed to an empty vector

    // C++98 std::ifstream constructor often preferred const char*
    std::ifstream fileStream(path_.c_str());
    if (!fileStream.is_open()) {
      throw std::runtime_error("Could not open file for reading: " + path_);
    }

    // Read the entire file content efficiently using stream buffer
    std::ostringstream ss;
    ss << fileStream.rdbuf();
    content_ = ss.str();

    // fileStream is closed automatically when it goes out of scope (RAII)
  }

  // Destructor (C++98 style)
  // Default behavior is sufficient as members manage their own resources.
  ~File() {
    // No explicit cleanup needed for std::string or std::vector
  }

  // --- Getters (const methods) ---
  const std::string &getPath() const { return path_; }
  const std::string &getContent() const { return content_; }
  const std::string &getDescription() const { return description_; }
  const std::vector<std::string> &getTags() const { return tags_; }

  // --- Setters / Modifiers ---
  void setContent(const std::string &content) { content_ = content; }
  void setDescription(const std::string &desc) { description_ = desc; }
  void setTags(const std::vector<std::string> &tags) { tags_ = tags; }

  void addTag(const std::string &tag) {
    // Optional: Avoid adding duplicate tags using std::find (C++98 compatible)
    if (std::find(tags_.begin(), tags_.end(), tag) == tags_.end()) {
      tags_.push_back(tag);
    }
  }

  void removeTag(const std::string &tag) {
    // Erase-remove idiom (C++98 compatible)
    // Need to explicitly state the iterator type (no 'auto')
    std::vector<std::string>::iterator new_end =
        std::remove(tags_.begin(), tags_.end(), tag);
    tags_.erase(new_end, tags_.end());
  }

  // --- File Operations ---

  // Save content back to the original path
  // Throws if path_ is empty.
  void save() {
    if (path_.empty()) {
      throw std::logic_error("Cannot save file: Path is not set. Use saveAs() "
                             "or load from a file first.");
    }
    saveAs(path_); // Delegate to saveAs
  }

  // Save content to a *new* path (and update the internal path_)
  // Note: Marked non-const here because although it doesn't change *members*
  // other than path_,
  //       it has a significant side effect (filesystem modification) and
  //       updates the path. If save() were const, calling saveAs from it would
  //       be problematic. If strict const-correctness regarding members is
  //       needed, saveAs could return void and a separate setPath method could
  //       be used, or save could be non-const. Making saveAs non-const as it
  //       modifies path_ is a reasonable C++98 approach.
  void saveAs(const std::string &newPath) {
    // C++98 std::ofstream constructor often preferred const char*
    std::ofstream outFile(newPath.c_str());
    if (!outFile.is_open()) {
      throw std::runtime_error("Could not open file for writing: " + newPath);
    }
    outFile << content_; // Write content

    // outFile is closed automatically when it goes out of scope (RAII)

    // Update the internal path only after successful write attempt
    path_ = newPath;
  }

  // --- Operator Overloads ---

  // Friend declaration needed within the class
  friend std::ostream &operator<<(std::ostream &os, const File &f);

private:
  // --- Member Variables ---
  std::string path_;              // Path of the file
  std::string content_;           // Content loaded from the file
  std::string description_;       // User-defined description
  std::vector<std::string> tags_; // User-defined tags

  // --- Private Copy Control (Optional, C++98 style) ---
  // Uncomment these lines to prevent copying/assignment if shallow copies
  // are undesirable or if managing resources requires deeper copies.
  // The default compiler-generated ones perform member-wise copy/assignment.
  // File(const File&);            // Disallow copy constructor
  // File& operator=(const File&); // Disallow assignment operator
};

// --- Operator Overloads Implementation (outside class) ---

// Overload << to print metadata summary (C++98 style loop)
inline std::ostream &operator<<(std::ostream &os, const File &f) {
  os << "File(path: \"" << f.getPath() << "\""; // Use getter for consistency
  if (!f.getDescription().empty()) {
    os << ", description: \"" << f.getDescription() << "\"";
  }
  const std::vector<std::string> &tags = f.getTags(); // Use getter
  if (!tags.empty()) {
    os << ", tags: [";
    // C++98 compatible loop (using index)
    for (std::size_t i = 0; i < tags.size(); ++i) {
      os << "\"" << tags[i] << "\"";
      if (i < tags.size() - 1) { // Check if not the last element
        os << ", ";
      }
    }
    os << "]";
  }
  os << ", content_size: " << f.getContent().length()
     << " bytes)"; // Use getter
  return os;
}

#endif // FILE_HPP // Include guard end
```

### File: inc/Groq.hpp
```cpp
#ifndef GROQ_CLIENT_HPP
#define GROQ_CLIENT_HPP

#include "modelApi.hpp" // Include the base class definition
#include <string>
#include <json/json.h> // Or your preferred JSON library

class GroqClient : public LLMClient {
public:
    // Constructor: API key is required (can be empty string to try ENV var)
    GroqClient(const std::string& apiKey = "");

    // Override the pure virtual generate function from the base class
    std::string generate(const std::string& prompt) override;

    // --- Groq-Specific Configuration ---
    void setApiKey(const std::string& apiKey);
    void setModel(const std::string& model) override;
    void setTemperature(double temperature) override;
    void setMaxTokens(int maxTokens) override;
    void setBaseUrl(const std::string& baseUrl); // Default: https://api.groq.com/openai/v1

private:
    std::string m_apiKey;
    std::string m_model;
    double m_temperature;
    int m_maxTokens;
    std::string m_baseUrl;

    // Internal helper for HTTP POST request (specific to Groq/OpenAI structure)
    std::string performHttpRequest(const std::string& url, const std::string& payload);
    // Internal helper to parse Groq/OpenAI JSON response structure
    std::string parseJsonResponse(const std::string& jsonResponse) const;

    // Static helper for curl callback
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif // GROQ_CLIENT_HPP
```

### File: inc/Import.hpp
```cpp

#pragma once
#include "Agent.hpp"

class Agent;

bool loadAgentProfile(Agent &agentToConfigure, const std::string &yamlPath) ;
```

### File: inc/Json.hpp
```cpp
#ifndef JSON_H_
#define JSON_H_

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cstdlib>
#include <stdexcept>
#include <cassert>

namespace json {

// Forward declarations
class Value;
typedef std::map<std::string, Value> Object;
typedef std::vector<Value> Array;

// JSON value types
enum ValueType {
    NULL_VALUE,
    BOOL_VALUE,
    NUMBER_VALUE,
    STRING_VALUE,
    ARRAY_VALUE,
    OBJECT_VALUE
};

class Value {
private:
    ValueType type_;
    bool bool_value_;
    double number_value_;
    std::string string_value_;
    Array array_value_;
    Object object_value_;

public:
    // Constructors
    Value() : type_(NULL_VALUE), bool_value_(false), number_value_(0) {}
    
    // Type-specific constructors
    Value(bool value) : type_(BOOL_VALUE), bool_value_(value), number_value_(0) {}
    Value(int value) : type_(NUMBER_VALUE), bool_value_(false), number_value_(value) {}
    Value(double value) : type_(NUMBER_VALUE), bool_value_(false), number_value_(value) {}
    Value(const char* value) : type_(STRING_VALUE), bool_value_(false), number_value_(0), string_value_(value) {}
    Value(const std::string& value) : type_(STRING_VALUE), bool_value_(false), number_value_(0), string_value_(value) {}
    Value(const Array& value) : type_(ARRAY_VALUE), bool_value_(false), number_value_(0), array_value_(value) {}
    Value(const Object& value) : type_(OBJECT_VALUE), bool_value_(false), number_value_(0), object_value_(value) {}

    // Type checking
    bool isNull() const { return type_ == NULL_VALUE; }
    bool isBool() const { return type_ == BOOL_VALUE; }
    bool isNumber() const { return type_ == NUMBER_VALUE; }
    bool isString() const { return type_ == STRING_VALUE; }
    bool isArray() const { return type_ == ARRAY_VALUE; }
    bool isObject() const { return type_ == OBJECT_VALUE; }
    ValueType type() const { return type_; }

    // Value retrieval (with type checking)
    bool asBool() const {
        if (!isBool()) throw std::runtime_error("Value is not a boolean");
        return bool_value_;
    }
    
    double asNumber() const {
        if (!isNumber()) throw std::runtime_error("Value is not a number");
        return number_value_;
    }
    
    int asInt() const {
        if (!isNumber()) throw std::runtime_error("Value is not a number");
        return static_cast<int>(number_value_);
    }
    
    const std::string& asString() const {
        if (!isString()) throw std::runtime_error("Value is not a string");
        return string_value_;
    }
    
    const Array& asArray() const {
        if (!isArray()) throw std::runtime_error("Value is not an array");
        return array_value_;
    }
    
    const Object& asObject() const {
        if (!isObject()) throw std::runtime_error("Value is not an object");
        return object_value_;
    }
    
    // Mutable accessors
    Array& asArray() {
        if (!isArray()) throw std::runtime_error("Value is not an array");
        return array_value_;
    }
    
    Object& asObject() {
        if (!isObject()) throw std::runtime_error("Value is not an object");
        return object_value_;
    }

    // Array/Object element access
    const Value& operator[](size_t index) const {
        if (!isArray()) throw std::runtime_error("Value is not an array");
        if (index >= array_value_.size()) throw std::out_of_range("Array index out of range");
        return array_value_[index];
    }
    
    Value& operator[](size_t index) {
        if (type_ != ARRAY_VALUE) {
            type_ = ARRAY_VALUE;
            array_value_.clear();
        }
        if (index >= array_value_.size()) 
            array_value_.resize(index + 1);
        return array_value_[index];
    }
    
    const Value& operator[](const std::string& key) const {
        if (!isObject()) throw std::runtime_error("Value is not an object");
        Object::const_iterator it = object_value_.find(key);
        if (it == object_value_.end()) throw std::out_of_range("Object key not found");
        return it->second;
    }
    
    Value& operator[](const std::string& key) {
        if (type_ != OBJECT_VALUE) {
            type_ = OBJECT_VALUE;
            object_value_.clear();
        }
        return object_value_[key];
    }

    // Helper methods for object access
    bool has(const std::string& key) const {
        if (!isObject()) return false;
        return object_value_.find(key) != object_value_.end();
    }
    
    // Size information
    size_t size() const {
        if (isArray()) return array_value_.size();
        if (isObject()) return object_value_.size();
        return 0;
    }

    // String representation
    std::string toString(bool pretty = false, int indent = 0) const {
        std::ostringstream os;
        
        switch (type_) {
            case NULL_VALUE:
                os << "null";
                break;
                
            case BOOL_VALUE:
                os << (bool_value_ ? "true" : "false");
                break;
                
            case NUMBER_VALUE:
                os << number_value_;
                break;
                
            case STRING_VALUE:
                os << '"';
                for (size_t i = 0; i < string_value_.length(); ++i) {
                    char c = string_value_[i];
                    switch (c) {
                        case '"':  os << "\\\""; break;
                        case '\\': os << "\\\\"; break;
                        case '\b': os << "\\b"; break;
                        case '\f': os << "\\f"; break;
                        case '\n': os << "\\n"; break;
                        case '\r': os << "\\r"; break;
                        case '\t': os << "\\t"; break;
                        default:
                            if (c < 32) {
                                os << "\\u" 
                                   << std::hex << std::uppercase
                                   << static_cast<int>(c)
                                   << std::dec << std::nouppercase;
                            } else {
                                os << c;
                            }
                    }
                }
                os << '"';
                break;
                
            case ARRAY_VALUE: {
                os << '[';
                
                if (pretty && !array_value_.empty()) {
                    os << '\n';
                }
                
                for (size_t i = 0; i < array_value_.size(); ++i) {
                    if (pretty) {
                        for (int j = 0; j < indent + 2; ++j) os << ' ';
                    }
                    
                    os << array_value_[i].toString(pretty, indent + 2);
                    
                    if (i < array_value_.size() - 1) {
                        os << ',';
                    }
                    
                    if (pretty) {
                        os << '\n';
                    }
                }
                
                if (pretty && !array_value_.empty()) {
                    for (int j = 0; j < indent; ++j) os << ' ';
                }
                
                os << ']';
                break;
            }
            
            case OBJECT_VALUE: {
                os << '{';
                
                if (pretty && !object_value_.empty()) {
                    os << '\n';
                }
                
                Object::const_iterator it = object_value_.begin();
                for (size_t i = 0; it != object_value_.end(); ++it, ++i) {
                    if (pretty) {
                        for (int j = 0; j < indent + 2; ++j) os << ' ';
                    }
                    
                    os << '"' << it->first << "\":";
                    
                    if (pretty) {
                        os << ' ';
                    }
                    
                    os << it->second.toString(pretty, indent + 2);
                    
                    if (i < object_value_.size() - 1) {
                        os << ',';
                    }
                    
                    if (pretty) {
                        os << '\n';
                    }
                }
                
                if (pretty && !object_value_.empty()) {
                    for (int j = 0; j < indent; ++j) os << ' ';
                }
                
                os << '}';
                break;
            }
        }
        
        return os.str();
    }
};

// Parsing functions
namespace parser {
    // Simple tokenizer for JSON parsing
    class Tokenizer {
    private:
        const std::string& input_;
        size_t pos_;

        void skipWhitespace() {
            while (pos_ < input_.length() && isspace(input_[pos_]))
                ++pos_;
        }

    public:
        Tokenizer(const std::string& input) : input_(input), pos_(0) {}

        bool hasMore() const {
            return pos_ < input_.length();
        }

        char peek() {
            skipWhitespace();
            if (!hasMore()) return '\0';
            return input_[pos_];
        }

        char next() {
            skipWhitespace();
            if (!hasMore()) return '\0';
            return input_[pos_++];
        }

        std::string readString() {
            // Assume the opening quote has been consumed
            std::string result;
            bool escape = false;

            while (hasMore()) {
                char c = input_[pos_++];
                
                if (escape) {
                    escape = false;
                    switch (c) {
                        case '"': result += '"'; break;
                        case '\\': result += '\\'; break;
                        case '/': result += '/'; break;
                        case 'b': result += '\b'; break;
                        case 'f': result += '\f'; break;
                        case 'n': result += '\n'; break;
                        case 'r': result += '\r'; break;
                        case 't': result += '\t'; break;
                        case 'u': {
                            // Handle \uXXXX escape sequences
                            if (pos_ + 4 > input_.length()) {
                                throw std::runtime_error("Invalid \\u escape sequence");
                            }
                            std::string hex = input_.substr(pos_, 4);
                            pos_ += 4;
                            
                            // Convert hex to int and then to char
                            int value = 0;
                            for (int i = 0; i < 4; ++i) {
                                value = value * 16;
                                char h = hex[i];
                                if (h >= '0' && h <= '9') value += (h - '0');
                                else if (h >= 'A' && h <= 'F') value += (h - 'A' + 10);
                                else if (h >= 'a' && h <= 'f') value += (h - 'a' + 10);
                                else throw std::runtime_error("Invalid hex character in \\u escape");
                            }
                            
                            // This is a simplification - proper UTF-8 handling would be more complex
                            if (value < 128) {
                                result += static_cast<char>(value);
                            } else {
                                // Just for basic handling, more comprehensive UTF-8 would be needed
                                result += '?';
                            }
                            break;
                        }
                        default:
                            result += c;
                    }
                } else if (c == '\\') {
                    escape = true;
                } else if (c == '"') {
                    // End of string
                    break;
                } else {
                    result += c;
                }
            }
            
            return result;
        }

        std::string readNumber() {
            size_t start = pos_ - 1;  // Include the first digit
            
            // Read digits, dot, exponent, etc.
            while (hasMore() && (
                   isdigit(input_[pos_]) || 
                   input_[pos_] == '.' || 
                   input_[pos_] == 'e' || 
                   input_[pos_] == 'E' || 
                   input_[pos_] == '+' || 
                   input_[pos_] == '-')) {
                ++pos_;
            }
            
            return input_.substr(start, pos_ - start);
        }

        std::string readToken() {
            size_t start = pos_ - 1;
            while (hasMore() && isalpha(input_[pos_])) {
                ++pos_;
            }
            return input_.substr(start, pos_ - start);
        }
        
        void expect(char c) {
            if (next() != c) {
                std::ostringstream msg;
                msg << "Expected '" << c << "' at position " << pos_;
                throw std::runtime_error(msg.str());
            }
        }
    };

    Value parseValue(Tokenizer& tokenizer);

    Value parseObject(Tokenizer& tokenizer) {
        Object obj;
        
        // Empty object?
        if (tokenizer.peek() == '}') {
            tokenizer.next();  // Consume '}'
            return Value(obj);
        }
        
        while (true) {
            // Parse key
            if (tokenizer.next() != '"') {
                throw std::runtime_error("Expected string as object key");
            }
            std::string key = tokenizer.readString();
            
            // Parse colon
            if (tokenizer.next() != ':') {
                throw std::runtime_error("Expected ':' after object key");
            }
            
            // Parse value
            obj[key] = parseValue(tokenizer);
            
            // More items?
            char c = tokenizer.next();
            if (c == '}') {
                break;  // End of object
            }
            else if (c != ',') {
                throw std::runtime_error("Expected ',' or '}' after object value");
            }
        }
        
        return Value(obj);
    }

    Value parseArray(Tokenizer& tokenizer) {
        Array arr;
        
        // Empty array?
        if (tokenizer.peek() == ']') {
            tokenizer.next();  // Consume ']'
            return Value(arr);
        }
        
        while (true) {
            // Parse value
            arr.push_back(parseValue(tokenizer));
            
            // More items?
            char c = tokenizer.next();
            if (c == ']') {
                break;  // End of array
            }
            else if (c != ',') {
                throw std::runtime_error("Expected ',' or ']' after array value");
            }
        }
        
        return Value(arr);
    }

    Value parseValue(Tokenizer& tokenizer) {
        char c = tokenizer.peek();
        
        switch (c) {
            case 'n':
                // null
                tokenizer.next();  // Consume 'n'
                if (tokenizer.readToken() == "ull") {
                    return Value();
                }
                throw std::runtime_error("Invalid token: expected 'null'");
                
            case 't':
                // true
                tokenizer.next();  // Consume 't'
                if (tokenizer.readToken() == "rue") {
                    return Value(true);
                }
                throw std::runtime_error("Invalid token: expected 'true'");
                
            case 'f':
                // false
                tokenizer.next();  // Consume 'f'
                if (tokenizer.readToken() == "alse") {  
                    return Value(false);
                }
                throw std::runtime_error("Invalid token: expected 'false'");
                
            case '"':
                // string
                tokenizer.next();  // Consume '"'
                return Value(tokenizer.readString());
                
            case '{':
                // object
                tokenizer.next();  // Consume '{'
                return parseObject(tokenizer);
                
            case '[':
                // array
                tokenizer.next();  // Consume '['
                return parseArray(tokenizer);
                
            case '-':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                // number
                tokenizer.next();  // Consume first digit/sign
                return Value(atof(tokenizer.readNumber().c_str()));
                
            default:
                throw std::runtime_error("Unexpected character in JSON");
        }
    }
    
    Value parse(const std::string& input) {
        Tokenizer tokenizer(input);
        Value result = parseValue(tokenizer);
        
        // Check for trailing data
        if (tokenizer.hasMore()) {
            char c = tokenizer.peek();
            if (!isspace(c)) {
                throw std::runtime_error("Unexpected trailing data in JSON");
            }
        }
        
        return result;
    }
}  // namespace parser

// Helper functions
inline Value parse(const std::string& input) {
    return parser::parse(input);
}

inline Value parseFile(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for reading");
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return parse(buffer.str());
}

}  // namespace json

#endif  // JSON_H_
```

### File: inc.md
```markdown
# AI Project Analysis - inc
- Generated on: Wed May  7 07:26:05 AM +01 2025
- System: Linux 6.12.26-1-lts x86_64
- Arch Linux: 1692 packages installed
- Directory: /home/mlamkadm/ai-repos/agents/agent-lib

## Directory Structure
```
../agent-lib/inc
├── Agent.hpp
├── File.hpp
├── Groq.hpp
├── Import.hpp
├── Json.hpp
├── MiniGemini.hpp
├── modelApi.hpp
├── notes.hpp
├── Tool.hpp
├── Utils.hpp
└── variables.hpp
```

## Project Statistics
- Total Files: 113
- Total Lines of Code: 24625
- Languages: .hpp(11)

## Project Files

### File: inc/Agent.hpp
```cpp
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
```

### File: inc/File.hpp
```cpp
#ifndef FILE_HPP // Include guard start
#define FILE_HPP

#include <algorithm> // For std::remove, std::find
#include <cstddef>   // For size_t
#include <fstream>   // For std::ifstream, std::ofstream
#include <iostream>  // For std::ostream
#include <sstream>   // For std::ostringstream
#include <stdexcept> // For std::runtime_error
#include <string>
#include <vector>

// Forward declaration for the friend function
class File;
std::ostream &operator<<(std::ostream &os, const File &f);

class File {
public:
  // Default constructor (C++98 style)
  // Initializes members to reasonable defaults.
  File() : path_(""), content_(""), description_("") {
    // tags_ is default-constructed to an empty vector
  }

  // Constructor to load from a file path (C++98 style)
  // Use explicit to prevent unintentional conversions
  explicit File(const std::string &filePath)
      : path_(filePath), content_(""), description_("") {
    // tags_ is default-constructed to an empty vector

    // C++98 std::ifstream constructor often preferred const char*
    std::ifstream fileStream(path_.c_str());
    if (!fileStream.is_open()) {
      throw std::runtime_error("Could not open file for reading: " + path_);
    }

    // Read the entire file content efficiently using stream buffer
    std::ostringstream ss;
    ss << fileStream.rdbuf();
    content_ = ss.str();

    // fileStream is closed automatically when it goes out of scope (RAII)
  }

  // Destructor (C++98 style)
  // Default behavior is sufficient as members manage their own resources.
  ~File() {
    // No explicit cleanup needed for std::string or std::vector
  }

  // --- Getters (const methods) ---
  const std::string &getPath() const { return path_; }
  const std::string &getContent() const { return content_; }
  const std::string &getDescription() const { return description_; }
  const std::vector<std::string> &getTags() const { return tags_; }

  // --- Setters / Modifiers ---
  void setContent(const std::string &content) { content_ = content; }
  void setDescription(const std::string &desc) { description_ = desc; }
  void setTags(const std::vector<std::string> &tags) { tags_ = tags; }

  void addTag(const std::string &tag) {
    // Optional: Avoid adding duplicate tags using std::find (C++98 compatible)
    if (std::find(tags_.begin(), tags_.end(), tag) == tags_.end()) {
      tags_.push_back(tag);
    }
  }

  void removeTag(const std::string &tag) {
    // Erase-remove idiom (C++98 compatible)
    // Need to explicitly state the iterator type (no 'auto')
    std::vector<std::string>::iterator new_end =
        std::remove(tags_.begin(), tags_.end(), tag);
    tags_.erase(new_end, tags_.end());
  }

  // --- File Operations ---

  // Save content back to the original path
  // Throws if path_ is empty.
  void save() {
    if (path_.empty()) {
      throw std::logic_error("Cannot save file: Path is not set. Use saveAs() "
                             "or load from a file first.");
    }
    saveAs(path_); // Delegate to saveAs
  }

  // Save content to a *new* path (and update the internal path_)
  // Note: Marked non-const here because although it doesn't change *members*
  // other than path_,
  //       it has a significant side effect (filesystem modification) and
  //       updates the path. If save() were const, calling saveAs from it would
  //       be problematic. If strict const-correctness regarding members is
  //       needed, saveAs could return void and a separate setPath method could
  //       be used, or save could be non-const. Making saveAs non-const as it
  //       modifies path_ is a reasonable C++98 approach.
  void saveAs(const std::string &newPath) {
    // C++98 std::ofstream constructor often preferred const char*
    std::ofstream outFile(newPath.c_str());
    if (!outFile.is_open()) {
      throw std::runtime_error("Could not open file for writing: " + newPath);
    }
    outFile << content_; // Write content

    // outFile is closed automatically when it goes out of scope (RAII)

    // Update the internal path only after successful write attempt
    path_ = newPath;
  }

  // --- Operator Overloads ---

  // Friend declaration needed within the class
  friend std::ostream &operator<<(std::ostream &os, const File &f);

private:
  // --- Member Variables ---
  std::string path_;              // Path of the file
  std::string content_;           // Content loaded from the file
  std::string description_;       // User-defined description
  std::vector<std::string> tags_; // User-defined tags

  // --- Private Copy Control (Optional, C++98 style) ---
  // Uncomment these lines to prevent copying/assignment if shallow copies
  // are undesirable or if managing resources requires deeper copies.
  // The default compiler-generated ones perform member-wise copy/assignment.
  // File(const File&);            // Disallow copy constructor
  // File& operator=(const File&); // Disallow assignment operator
};

// --- Operator Overloads Implementation (outside class) ---

// Overload << to print metadata summary (C++98 style loop)
inline std::ostream &operator<<(std::ostream &os, const File &f) {
  os << "File(path: \"" << f.getPath() << "\""; // Use getter for consistency
  if (!f.getDescription().empty()) {
    os << ", description: \"" << f.getDescription() << "\"";
  }
  const std::vector<std::string> &tags = f.getTags(); // Use getter
  if (!tags.empty()) {
    os << ", tags: [";
    // C++98 compatible loop (using index)
    for (std::size_t i = 0; i < tags.size(); ++i) {
      os << "\"" << tags[i] << "\"";
      if (i < tags.size() - 1) { // Check if not the last element
        os << ", ";
      }
    }
    os << "]";
  }
  os << ", content_size: " << f.getContent().length()
     << " bytes)"; // Use getter
  return os;
}

#endif // FILE_HPP // Include guard end
```

### File: inc/Groq.hpp
```cpp
#ifndef GROQ_CLIENT_HPP
#define GROQ_CLIENT_HPP

#include "modelApi.hpp" // Include the base class definition
#include <string>
#include <json/json.h> // Or your preferred JSON library

class GroqClient : public LLMClient {
public:
    // Constructor: API key is required (can be empty string to try ENV var)
    GroqClient(const std::string& apiKey = "");

    // Override the pure virtual generate function from the base class
    std::string generate(const std::string& prompt) override;

    // --- Groq-Specific Configuration ---
    void setApiKey(const std::string& apiKey);
    void setModel(const std::string& model) override;
    void setTemperature(double temperature) override;
    void setMaxTokens(int maxTokens) override;
    void setBaseUrl(const std::string& baseUrl); // Default: https://api.groq.com/openai/v1

private:
    std::string m_apiKey;
    std::string m_model;
    double m_temperature;
    int m_maxTokens;
    std::string m_baseUrl;

    // Internal helper for HTTP POST request (specific to Groq/OpenAI structure)
    std::string performHttpRequest(const std::string& url, const std::string& payload);
    // Internal helper to parse Groq/OpenAI JSON response structure
    std::string parseJsonResponse(const std::string& jsonResponse) const;

    // Static helper for curl callback
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif // GROQ_CLIENT_HPP
```

### File: inc/Import.hpp
```cpp

#pragma once
#include "Agent.hpp"

class Agent;

bool loadAgentProfile(Agent &agentToConfigure, const std::string &yamlPath) ;
```

### File: inc/Json.hpp
```cpp
#ifndef JSON_H_
#define JSON_H_

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cstdlib>
#include <stdexcept>
#include <cassert>

namespace json {

// Forward declarations
class Value;
typedef std::map<std::string, Value> Object;
typedef std::vector<Value> Array;

// JSON value types
enum ValueType {
    NULL_VALUE,
    BOOL_VALUE,
    NUMBER_VALUE,
    STRING_VALUE,
    ARRAY_VALUE,
    OBJECT_VALUE
};

class Value {
private:
    ValueType type_;
    bool bool_value_;
    double number_value_;
    std::string string_value_;
    Array array_value_;
    Object object_value_;

public:
    // Constructors
    Value() : type_(NULL_VALUE), bool_value_(false), number_value_(0) {}
    
    // Type-specific constructors
    Value(bool value) : type_(BOOL_VALUE), bool_value_(value), number_value_(0) {}
    Value(int value) : type_(NUMBER_VALUE), bool_value_(false), number_value_(value) {}
    Value(double value) : type_(NUMBER_VALUE), bool_value_(false), number_value_(value) {}
    Value(const char* value) : type_(STRING_VALUE), bool_value_(false), number_value_(0), string_value_(value) {}
    Value(const std::string& value) : type_(STRING_VALUE), bool_value_(false), number_value_(0), string_value_(value) {}
    Value(const Array& value) : type_(ARRAY_VALUE), bool_value_(false), number_value_(0), array_value_(value) {}
    Value(const Object& value) : type_(OBJECT_VALUE), bool_value_(false), number_value_(0), object_value_(value) {}

    // Type checking
    bool isNull() const { return type_ == NULL_VALUE; }
    bool isBool() const { return type_ == BOOL_VALUE; }
    bool isNumber() const { return type_ == NUMBER_VALUE; }
    bool isString() const { return type_ == STRING_VALUE; }
    bool isArray() const { return type_ == ARRAY_VALUE; }
    bool isObject() const { return type_ == OBJECT_VALUE; }
    ValueType type() const { return type_; }

    // Value retrieval (with type checking)
    bool asBool() const {
        if (!isBool()) throw std::runtime_error("Value is not a boolean");
        return bool_value_;
    }
    
    double asNumber() const {
        if (!isNumber()) throw std::runtime_error("Value is not a number");
        return number_value_;
    }
    
    int asInt() const {
        if (!isNumber()) throw std::runtime_error("Value is not a number");
        return static_cast<int>(number_value_);
    }
    
    const std::string& asString() const {
        if (!isString()) throw std::runtime_error("Value is not a string");
        return string_value_;
    }
    
    const Array& asArray() const {
        if (!isArray()) throw std::runtime_error("Value is not an array");
        return array_value_;
    }
    
    const Object& asObject() const {
        if (!isObject()) throw std::runtime_error("Value is not an object");
        return object_value_;
    }
    
    // Mutable accessors
    Array& asArray() {
        if (!isArray()) throw std::runtime_error("Value is not an array");
        return array_value_;
    }
    
    Object& asObject() {
        if (!isObject()) throw std::runtime_error("Value is not an object");
        return object_value_;
    }

    // Array/Object element access
    const Value& operator[](size_t index) const {
        if (!isArray()) throw std::runtime_error("Value is not an array");
        if (index >= array_value_.size()) throw std::out_of_range("Array index out of range");
        return array_value_[index];
    }
    
    Value& operator[](size_t index) {
        if (type_ != ARRAY_VALUE) {
            type_ = ARRAY_VALUE;
            array_value_.clear();
        }
        if (index >= array_value_.size()) 
            array_value_.resize(index + 1);
        return array_value_[index];
    }
    
    const Value& operator[](const std::string& key) const {
        if (!isObject()) throw std::runtime_error("Value is not an object");
        Object::const_iterator it = object_value_.find(key);
        if (it == object_value_.end()) throw std::out_of_range("Object key not found");
        return it->second;
    }
    
    Value& operator[](const std::string& key) {
        if (type_ != OBJECT_VALUE) {
            type_ = OBJECT_VALUE;
            object_value_.clear();
        }
        return object_value_[key];
    }

    // Helper methods for object access
    bool has(const std::string& key) const {
        if (!isObject()) return false;
        return object_value_.find(key) != object_value_.end();
    }
    
    // Size information
    size_t size() const {
        if (isArray()) return array_value_.size();
        if (isObject()) return object_value_.size();
        return 0;
    }

    // String representation
    std::string toString(bool pretty = false, int indent = 0) const {
        std::ostringstream os;
        
        switch (type_) {
            case NULL_VALUE:
                os << "null";
                break;
                
            case BOOL_VALUE:
                os << (bool_value_ ? "true" : "false");
                break;
                
            case NUMBER_VALUE:
                os << number_value_;
                break;
                
            case STRING_VALUE:
                os << '"';
                for (size_t i = 0; i < string_value_.length(); ++i) {
                    char c = string_value_[i];
                    switch (c) {
                        case '"':  os << "\\\""; break;
                        case '\\': os << "\\\\"; break;
                        case '\b': os << "\\b"; break;
                        case '\f': os << "\\f"; break;
                        case '\n': os << "\\n"; break;
                        case '\r': os << "\\r"; break;
                        case '\t': os << "\\t"; break;
                        default:
                            if (c < 32) {
                                os << "\\u" 
                                   << std::hex << std::uppercase
                                   << static_cast<int>(c)
                                   << std::dec << std::nouppercase;
                            } else {
                                os << c;
                            }
                    }
                }
                os << '"';
                break;
                
            case ARRAY_VALUE: {
                os << '[';
                
                if (pretty && !array_value_.empty()) {
                    os << '\n';
                }
                
                for (size_t i = 0; i < array_value_.size(); ++i) {
                    if (pretty) {
                        for (int j = 0; j < indent + 2; ++j) os << ' ';
                    }
                    
                    os << array_value_[i].toString(pretty, indent + 2);
                    
                    if (i < array_value_.size() - 1) {
                        os << ',';
                    }
                    
                    if (pretty) {
                        os << '\n';
                    }
                }
                
                if (pretty && !array_value_.empty()) {
                    for (int j = 0; j < indent; ++j) os << ' ';
                }
                
                os << ']';
                break;
            }
            
            case OBJECT_VALUE: {
                os << '{';
                
                if (pretty && !object_value_.empty()) {
                    os << '\n';
                }
                
                Object::const_iterator it = object_value_.begin();
                for (size_t i = 0; it != object_value_.end(); ++it, ++i) {
                    if (pretty) {
                        for (int j = 0; j < indent + 2; ++j) os << ' ';
                    }
                    
                    os << '"' << it->first << "\":";
                    
                    if (pretty) {
                        os << ' ';
                    }
                    
                    os << it->second.toString(pretty, indent + 2);
                    
                    if (i < object_value_.size() - 1) {
                        os << ',';
                    }
                    
                    if (pretty) {
                        os << '\n';
                    }
                }
                
                if (pretty && !object_value_.empty()) {
                    for (int j = 0; j < indent; ++j) os << ' ';
                }
                
                os << '}';
                break;
            }
        }
        
        return os.str();
    }
};

// Parsing functions
namespace parser {
    // Simple tokenizer for JSON parsing
    class Tokenizer {
    private:
        const std::string& input_;
        size_t pos_;

        void skipWhitespace() {
            while (pos_ < input_.length() && isspace(input_[pos_]))
                ++pos_;
        }

    public:
        Tokenizer(const std::string& input) : input_(input), pos_(0) {}

        bool hasMore() const {
            return pos_ < input_.length();
        }

        char peek() {
            skipWhitespace();
            if (!hasMore()) return '\0';
            return input_[pos_];
        }

        char next() {
            skipWhitespace();
            if (!hasMore()) return '\0';
            return input_[pos_++];
        }

        std::string readString() {
            // Assume the opening quote has been consumed
            std::string result;
            bool escape = false;

            while (hasMore()) {
                char c = input_[pos_++];
                
                if (escape) {
                    escape = false;
                    switch (c) {
                        case '"': result += '"'; break;
                        case '\\': result += '\\'; break;
                        case '/': result += '/'; break;
                        case 'b': result += '\b'; break;
                        case 'f': result += '\f'; break;
                        case 'n': result += '\n'; break;
                        case 'r': result += '\r'; break;
                        case 't': result += '\t'; break;
                        case 'u': {
                            // Handle \uXXXX escape sequences
                            if (pos_ + 4 > input_.length()) {
                                throw std::runtime_error("Invalid \\u escape sequence");
                            }
                            std::string hex = input_.substr(pos_, 4);
                            pos_ += 4;
                            
                            // Convert hex to int and then to char
                            int value = 0;
                            for (int i = 0; i < 4; ++i) {
                                value = value * 16;
                                char h = hex[i];
                                if (h >= '0' && h <= '9') value += (h - '0');
                                else if (h >= 'A' && h <= 'F') value += (h - 'A' + 10);
                                else if (h >= 'a' && h <= 'f') value += (h - 'a' + 10);
                                else throw std::runtime_error("Invalid hex character in \\u escape");
                            }
                            
                            // This is a simplification - proper UTF-8 handling would be more complex
                            if (value < 128) {
                                result += static_cast<char>(value);
                            } else {
                                // Just for basic handling, more comprehensive UTF-8 would be needed
                                result += '?';
                            }
                            break;
                        }
                        default:
                            result += c;
                    }
                } else if (c == '\\') {
                    escape = true;
                } else if (c == '"') {
                    // End of string
                    break;
                } else {
                    result += c;
                }
            }
            
            return result;
        }

        std::string readNumber() {
            size_t start = pos_ - 1;  // Include the first digit
            
            // Read digits, dot, exponent, etc.
            while (hasMore() && (
                   isdigit(input_[pos_]) || 
                   input_[pos_] == '.' || 
                   input_[pos_] == 'e' || 
                   input_[pos_] == 'E' || 
                   input_[pos_] == '+' || 
                   input_[pos_] == '-')) {
                ++pos_;
            }
            
            return input_.substr(start, pos_ - start);
        }

        std::string readToken() {
            size_t start = pos_ - 1;
            while (hasMore() && isalpha(input_[pos_])) {
                ++pos_;
            }
            return input_.substr(start, pos_ - start);
        }
        
        void expect(char c) {
            if (next() != c) {
                std::ostringstream msg;
                msg << "Expected '" << c << "' at position " << pos_;
                throw std::runtime_error(msg.str());
            }
        }
    };

    Value parseValue(Tokenizer& tokenizer);

    Value parseObject(Tokenizer& tokenizer) {
        Object obj;
        
        // Empty object?
        if (tokenizer.peek() == '}') {
            tokenizer.next();  // Consume '}'
            return Value(obj);
        }
        
        while (true) {
            // Parse key
            if (tokenizer.next() != '"') {
                throw std::runtime_error("Expected string as object key");
            }
            std::string key = tokenizer.readString();
            
            // Parse colon
            if (tokenizer.next() != ':') {
                throw std::runtime_error("Expected ':' after object key");
            }
            
            // Parse value
            obj[key] = parseValue(tokenizer);
            
            // More items?
            char c = tokenizer.next();
            if (c == '}') {
                break;  // End of object
            }
            else if (c != ',') {
                throw std::runtime_error("Expected ',' or '}' after object value");
            }
        }
        
        return Value(obj);
    }

    Value parseArray(Tokenizer& tokenizer) {
        Array arr;
        
        // Empty array?
        if (tokenizer.peek() == ']') {
            tokenizer.next();  // Consume ']'
            return Value(arr);
        }
        
        while (true) {
            // Parse value
            arr.push_back(parseValue(tokenizer));
            
            // More items?
            char c = tokenizer.next();
            if (c == ']') {
                break;  // End of array
            }
            else if (c != ',') {
                throw std::runtime_error("Expected ',' or ']' after array value");
            }
        }
        
        return Value(arr);
    }

    Value parseValue(Tokenizer& tokenizer) {
        char c = tokenizer.peek();
        
        switch (c) {
            case 'n':
                // null
                tokenizer.next();  // Consume 'n'
                if (tokenizer.readToken() == "ull") {
                    return Value();
                }
                throw std::runtime_error("Invalid token: expected 'null'");
                
            case 't':
                // true
                tokenizer.next();  // Consume 't'
                if (tokenizer.readToken() == "rue") {
                    return Value(true);
                }
                throw std::runtime_error("Invalid token: expected 'true'");
                
            case 'f':
                // false
                tokenizer.next();  // Consume 'f'
                if (tokenizer.readToken() == "alse") {  
                    return Value(false);
                }
                throw std::runtime_error("Invalid token: expected 'false'");
                
            case '"':
                // string
                tokenizer.next();  // Consume '"'
                return Value(tokenizer.readString());
                
            case '{':
                // object
                tokenizer.next();  // Consume '{'
                return parseObject(tokenizer);
                
            case '[':
                // array
                tokenizer.next();  // Consume '['
                return parseArray(tokenizer);
                
            case '-':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                // number
                tokenizer.next();  // Consume first digit/sign
                return Value(atof(tokenizer.readNumber().c_str()));
                
            default:
                throw std::runtime_error("Unexpected character in JSON");
        }
    }
    
    Value parse(const std::string& input) {
        Tokenizer tokenizer(input);
        Value result = parseValue(tokenizer);
        
        // Check for trailing data
        if (tokenizer.hasMore()) {
            char c = tokenizer.peek();
            if (!isspace(c)) {
                throw std::runtime_error("Unexpected trailing data in JSON");
            }
        }
        
        return result;
    }
}  // namespace parser

// Helper functions
inline Value parse(const std::string& input) {
    return parser::parse(input);
}

inline Value parseFile(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for reading");
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return parse(buffer.str());
}

}  // namespace json

#endif  // JSON_H_
```

### File: inc/MiniGemini.hpp
```cpp
#ifndef GEMINI_CLIENT_HPP
#define GEMINI_CLIENT_HPP

#include "modelApi.hpp" // Include the base class definition
#include <string>
#include <json/json.h> // For Json::Value (or your preferred JSON library)

const std::string MODELS[] = {
    // Main models
    "gemini-2.0-flash",         // Latest Flash model
    "gemini-2.0-flash-lite",    // Cost-efficient Flash variant
    "gemini-1.5-flash",         // Previous generation Flash model
    "gemini-1.5-flash-8b",      // Lightweight 8B parameter variant
    "gemini-2.5-pro-exp-03-25", // Experimental Pro model

    // Special purpose models
    "gemini-embedding-exp",      // Text embedding model
    "models/text-embedding-004", // Basic text embedding model
    "imagen-3.0-generate-002",   // Image generation model
    "models/embedding-001"       // Legacy embedding model
};

class MiniGemini : public LLMClient {
public:
    // Constructor: API key is required (can be empty string to try ENV var)
    MiniGemini(const std::string& apiKey = "");

    // Override the pure virtual generate function from the base class
    std::string generate(const std::string& prompt) override;

    // --- Gemini-Specific Configuration ---
    void setApiKey(const std::string& apiKey);
    void setModel(const std::string& model) override;
    void setTemperature(double temperature) override;
    void setMaxTokens(int maxTokens) override;
    void setBaseUrl(const std::string& baseUrl); // Allow changing the base URL if needed

private:
    std::string m_apiKey;
    std::string m_model;
    double m_temperature;
    int m_maxTokens;
    std::string m_baseUrl;

    // Internal helper for HTTP POST request (specific to Gemini structure)
    std::string performHttpRequest(const std::string& url, const std::string& payload);
    // Internal helper to parse Gemini's JSON response structure
    std::string parseJsonResponse(const std::string& jsonResponse) const;

    // Static helper for curl callback (can be shared or made global if needed)
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif // GEMINI_CLIENT_HPP
```

### File: inc/modelApi.hpp
```cpp
#pragma once
#include <stdexcept>
#include <string>

// Custom exception for API errors (can be shared by all clients)
class ApiError : public std::runtime_error {
public:
    ApiError(const std::string& message) : std::runtime_error(message) {}
};

// Abstract base class for LLM clients
class LLMClient {
public:
    // Virtual destructor is crucial for base classes with virtual functions
    virtual ~LLMClient() = default;

    // Pure virtual function that all derived clients MUST implement
    // Takes a prompt and returns the generated text or throws ApiError
    virtual std::string generate(const std::string& prompt) = 0;

    // Optional: Add common configuration setters if desired,
    // but they might be better handled in derived classes if APIs differ significantly.
    virtual void setModel(const std::string& model) = 0;
    virtual void setTemperature(double temperature) = 0;
    virtual void setMaxTokens(int maxTokens) = 0;
};

```

### File: inc/notes.hpp
```cpp
#ifndef NOTES_HPP
#define NOTES_HPP

#include <fstream>
#include <string>
#include <vector>

class note {
public:
  note(const std::string &text) : _text(text) {}
  note(const std::string &text, const std::string &path)
      : _text(text), _path(path) {
    // if (_path.empty()) {
    //     _path = "./";
    //     }
    // open file and write text to it
    std::ofstream file(_path);

    if (!file.is_open()) {
      throw std::runtime_error("Could not open file: " + _path);
    } else {
      file << text;
    }
  }
  ~note() {
    // close file
    std::ofstream file(_path, std::ios::app);
    if (!file.is_open()) {
      throw std::runtime_error("Could not open file: " + _path);
    } else {
      file.close();
    }
  }
  std::string getText() const;
  void setText(const std::string &text);

  std::string getPath() const;

private:
  std::string _text;
  std::string _path;
};

typedef std::vector<note> notes;

#endif
```

### File: inc/Tool.hpp
```cpp
#pragma once

#include <json/json.h> // For Json::Value used in callback signature
#include <map>         // For m_use_cases, m_memory_stack
#include <stdexcept>   // For std::runtime_error
#include <string>

// Forward declaration (optional, good practice)
class Agent;
namespace Json {
class Value;
}

// Callback function signatures: Takes JSON parameters, returns string result
typedef std::string (*ToolCallback)(const Json::Value &params);
typedef std::string (*ToolCallbackWithAgent)(const Json::Value &params,
                                             Agent &agent);

class Tool {
public:
  // --- Constructors ---
  // Constructor for tools needing agent context (built-in style)
  Tool(const std::string &name, const std::string &description,
       ToolCallbackWithAgent callback, Agent &agent)
      : m_name(name), m_description(description), m_callback(nullptr),
        m_builtin_callback(callback), m_agent(&agent) {}

  // Constructor for standard external tools
  Tool(const std::string &name, const std::string &description,
       ToolCallback callback)
      : m_name(name), m_description(description), m_callback(callback),
        m_builtin_callback(nullptr), m_agent(nullptr) {}

  // Default constructor
  Tool()
      : m_name(""), m_description(""), m_callback(nullptr),
        m_builtin_callback(nullptr), m_agent(nullptr) {}

  // Constructor with just a callback (name/desc can be set later)
  Tool(ToolCallback callback)
      : m_name(""), m_description(""), m_callback(callback),
        m_builtin_callback(nullptr), m_agent(nullptr) {}

  // --- Getters / Setters ---
  std::string getName() const { return m_name; }
  void setName(const std::string &name) { m_name = name; }

  std::string getDescription() const { return m_description; }
  void setDescription(const std::string &description) {
    m_description = description;
  }

  // --- Execution ---
  // Execute the tool's function with given JSON parameters
  std::string execute(const Json::Value &params) {
    if (m_builtin_callback &&
        m_agent) { // Prioritize built-in if agent context is available
      return m_builtin_callback(params, *m_agent);
    } else if (m_callback) { // Fallback to standard callback
      return m_callback(params);
    } else {
      throw std::runtime_error("No valid callback function set for tool '" +
                               m_name + "'");
    }
  }
  // execute(const std::string& params) overload removed

  // --- Callback Configuration ---
  void setCallback(ToolCallback callback) {
    if (!callback) {
      throw std::invalid_argument("Tool callback cannot be NULL");
    }
    m_callback = callback;
    m_builtin_callback = nullptr; // Ensure only one callback type is active
  }

  void setBuiltinCallback(ToolCallbackWithAgent callback, Agent &agent) {
    if (!callback) {
      throw std::invalid_argument("Builtin tool callback cannot be NULL");
    }
    m_builtin_callback = callback;
    m_agent = &agent;
    m_callback = nullptr; // Ensure only one callback type is active
  }

  // --- Use Cases ---
  void addUseCase(const std::string &use_case, const std::string &example) {
    m_use_cases[use_case] = example;
  }

  std::string getUseCase(const std::string &use_case) const {
    auto it = m_use_cases.find(use_case);
    if (it != m_use_cases.end()) {
      return it->second;
    } else {
      // Return empty string or throw? Returning empty might be safer.
      return "";
      // throw std::runtime_error("Use case not found: " + use_case);
    }
  }
  std::string getAllUseCases() const {
    std::string all_use_cases;
    if (m_use_cases.empty())
      return "";

    all_use_cases += "Example Use Cases for tool '" + m_name + "':\n";
    for (const auto &pair : m_use_cases) {
      all_use_cases += "- Purpose: " + pair.first +
                       "\n  Example JSON Params: " + pair.second + "\n";
    }
    return all_use_cases;
  }
  std::string getAllUseCaseCap(size_t cap) const {
    std::string all_use_cases;
    if (m_use_cases.empty() || cap == 0)
      return "";

    all_use_cases += "Example Use Cases for tool '" + m_name + "' (limit " +
                     std::to_string(cap) + "):\n";
    size_t count = 0;
    for (const auto &pair : m_use_cases) {
      if (count >= cap)
        break;
      all_use_cases += "- Purpose: " + pair.first +
                       "\n  Example JSON Params: " + pair.second + "\n";
      ++count;
    }
    return all_use_cases;
  }

private:
  std::string m_name;
  std::string m_description;

  ToolCallback m_callback = nullptr; // Standard callback (expects JSON)
  ToolCallbackWithAgent m_builtin_callback =
      nullptr; // Callback needing agent context (expects JSON)
  enum {
    // Tool types can be defined here if needed
    // e.g., EXTERNAL, BUILTIN, etc.
    TOOL_TYPE_EXTERNAL,
    TOOL_TYPE_BUILTIN
  } type = TOOL_TYPE_EXTERNAL;

  std::map<std::string, std::string>
      m_use_cases; // Map of purpose -> example JSON params string

  Agent *m_agent = nullptr; // Pointer to the agent using this tool (for builtin
                            // callbacks) PureTextToolCallback removed
};
```

### File: inc/Utils.hpp
```cpp

#pragma once

#include <string>

int executeCommand(const std::string &command, std::string &output) ;
```

### File: inc/variables.hpp
```cpp
#include <iostream>
#include <string>
#include <filesystem>
#include <map>
#include <vector>
#include <sstream>
// std::any is C++17 and later
#include <any>

template <typename T>
class Variable {
private:
    std::string key;
    T value;

public:
    Variable(const std::string& k, const T& val) : key(k), value(val) {}

    std::string getKey() const { return key; }
    T getValue() const { return value; }
};


class NamespaceVariables {
private:
    std::map<std::string, NamespaceVariables*> namespaces;
    std::map<std::string, Variable<std::any>> variables;  // Use std::any to store any type

public:
    // Access a namespace (creates it if it doesn't exist)
    NamespaceVariables& operator[](const std::string& ns) {
        if (namespaces.find(ns) == namespaces.end()) {
            namespaces[ns] = new NamespaceVariables();
        }
        return *namespaces[ns];
    }

    // Set a variable within the current namespace
    template <typename T>
    void set(const std::string& key, const T& value) {
        variables[key] = Variable<std::any>(key, value);
    }

    // Get a variable within the current namespace
    template <typename T>
    T get(const std::string& key) const {
        try {
            return std::any_cast<T>(variables.at(key).getValue());
        } catch (const std::bad_any_cast& e) {
            std::cerr << "Error: Invalid type cast for key: " << key << std::endl;
            throw; // Re-throw the exception or handle it differently
        }
    }


    // Helper function to print all variables in a namespace (and sub-namespaces)
    void print(const std::string& prefix = "") const {
        for (const auto& var : variables) {
            std::cout << prefix << var.first << ": ";
            try { // Attempt to print common types
                std::cout << std::any_cast<std::string>(var.second.getValue()) << std::endl;
            } catch(...) {
                try { std::cout << std::any_cast<int>(var.second.getValue()) << std::endl; }
                catch(...) {
                    try { std::cout << std::any_cast<bool>(var.second.getValue()) << std::endl; }
                    catch(...) {
                        try { std::cout << std::any_cast<std::filesystem::path>(var.second.getValue()) << std::endl; }
                        catch(...) { std::cout << "[Unprintable Type]" << std::endl; }
                    }
                }
            }
        }
        for (const auto& ns : namespaces) {
            std::cout << prefix << ns.first << ":" << std::endl;
            ns.second->print(prefix + "  ");
        }
    }


};




int main() {
    NamespaceVariables vars;

    vars["global"]["configs"]["agents"]["standard"].set("name", "Standard Agent");
    vars["agent-1"]["state"]["input"].set("value", 123);
    vars["global"]["configs"]["port"].set("number", 8080);
    vars["global"]["configs"]["path"].set<std::filesystem::path>("value", "data/config.json");


    vars.print();


    std::cout << "Agent name: " << vars["global"]["configs"]["agents"]["standard"].get<std::string>("name") << std::endl;
    std::cout << "Port number: " << vars["global"]["configs"]["port"].get<int>("number") << std::endl;

    // Example demonstrating error handling:
    try {
        std::string portNumber = vars["global"]["configs"]["port"].get<std::string>("number"); // Incorrect type
    } catch (const std::bad_any_cast& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }


    return 0;
}
```
```

### File: inc/MiniGemini.hpp
```cpp
#ifndef GEMINI_CLIENT_HPP
#define GEMINI_CLIENT_HPP

#include "modelApi.hpp" // Include the base class definition
#include <string>
#include <json/json.h> // For Json::Value (or your preferred JSON library)

const std::string MODELS[] = {
    // Main models
    "gemini-2.0-flash",         // Latest Flash model
    "gemini-2.0-flash-lite",    // Cost-efficient Flash variant
    "gemini-1.5-flash",         // Previous generation Flash model
    "gemini-1.5-flash-8b",      // Lightweight 8B parameter variant
    "gemini-2.5-pro-exp-03-25", // Experimental Pro model

    // Special purpose models
    "gemini-embedding-exp",      // Text embedding model
    "models/text-embedding-004", // Basic text embedding model
    "imagen-3.0-generate-002",   // Image generation model
    "models/embedding-001"       // Legacy embedding model
};

class MiniGemini : public LLMClient {
public:
    // Constructor: API key is required (can be empty string to try ENV var)
    MiniGemini(const std::string& apiKey = "");

    // Override the pure virtual generate function from the base class
    std::string generate(const std::string& prompt) override;

    // --- Gemini-Specific Configuration ---
    void setApiKey(const std::string& apiKey);
    void setModel(const std::string& model) override;
    void setTemperature(double temperature) override;
    void setMaxTokens(int maxTokens) override;
    void setBaseUrl(const std::string& baseUrl); // Allow changing the base URL if needed

private:
    std::string m_apiKey;
    std::string m_model;
    double m_temperature;
    int m_maxTokens;
    std::string m_baseUrl;

    // Internal helper for HTTP POST request (specific to Gemini structure)
    std::string performHttpRequest(const std::string& url, const std::string& payload);
    // Internal helper to parse Gemini's JSON response structure
    std::string parseJsonResponse(const std::string& jsonResponse) const;

    // Static helper for curl callback (can be shared or made global if needed)
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif // GEMINI_CLIENT_HPP
```

### File: inc/modelApi.hpp
```cpp
#pragma once
#include <stdexcept>
#include <string>

// Custom exception for API errors (can be shared by all clients)
class ApiError : public std::runtime_error {
public:
    ApiError(const std::string& message) : std::runtime_error(message) {}
};

// Abstract base class for LLM clients
class LLMClient {
public:
    // Virtual destructor is crucial for base classes with virtual functions
    virtual ~LLMClient() = default;

    // Pure virtual function that all derived clients MUST implement
    // Takes a prompt and returns the generated text or throws ApiError
    virtual std::string generate(const std::string& prompt) = 0;

    // Optional: Add common configuration setters if desired,
    // but they might be better handled in derived classes if APIs differ significantly.
    virtual void setModel(const std::string& model) = 0;
    virtual void setTemperature(double temperature) = 0;
    virtual void setMaxTokens(int maxTokens) = 0;
};

```

### File: inc/notes.hpp
```cpp
#ifndef NOTES_HPP
#define NOTES_HPP

#include <fstream>
#include <string>
#include <vector>

class note {
public:
  note(const std::string &text) : _text(text) {}
  note(const std::string &text, const std::string &path)
      : _text(text), _path(path) {
    // if (_path.empty()) {
    //     _path = "./";
    //     }
    // open file and write text to it
    std::ofstream file(_path);

    if (!file.is_open()) {
      throw std::runtime_error("Could not open file: " + _path);
    } else {
      file << text;
    }
  }
  ~note() {
    // close file
    std::ofstream file(_path, std::ios::app);
    if (!file.is_open()) {
      throw std::runtime_error("Could not open file: " + _path);
    } else {
      file.close();
    }
  }
  std::string getText() const;
  void setText(const std::string &text);

  std::string getPath() const;

private:
  std::string _text;
  std::string _path;
};

typedef std::vector<note> notes;

#endif
```

### File: inc/Tool.hpp
```cpp
#pragma once

#include <json/json.h>
#include <string>
#include <functional> // For std::function
#include <stdexcept>  // For std::runtime_error

// Forward declaration (if Tool methods might take Agent context, not used in this simple version)
// class Agent;

// Type alias for the functional callback Tool will use
using FunctionalToolCallback = std::function<std::string(const Json::Value&)>;

class Tool {
private:
    std::string name;
    std::string description;
    FunctionalToolCallback functionalCallback;

public:
    // Constructors
    Tool(const std::string& toolName, const std::string& toolDescription);
    Tool(); // Default constructor

    // Getters
    std::string getName() const;
    std::string getDescription() const;

    // Setters
    void setName(const std::string& toolName);
    void setDescription(const std::string& toolDescription);
    void setCallback(FunctionalToolCallback callback);

    // Execution
    std::string execute(const Json::Value& params);
};
```

### File: inc/Utils.hpp
```cpp
#pragma once

#include <string>
#include <json/json.h> // For Json::Value

// Function to execute a simple command and get its output
int executeCommand(const std::string &command, std::string &output);

// Function to execute a script (inline or from file) with specified runtime and parameters
std::string executeScriptTool(const std::string& scriptPathOrContent,
                              const std::string& runtime,
                              const Json::Value& params,
                              bool isContentInline);
```

### File: inc/variables.hpp
```cpp
#include <iostream>
#include <string>
#include <filesystem>
#include <map>
#include <vector>
#include <sstream>
// std::any is C++17 and later
#include <any>

template <typename T>
class Variable {
private:
    std::string key;
    T value;

public:
    Variable(const std::string& k, const T& val) : key(k), value(val) {}

    std::string getKey() const { return key; }
    T getValue() const { return value; }
};


class NamespaceVariables {
private:
    std::map<std::string, NamespaceVariables*> namespaces;
    std::map<std::string, Variable<std::any>> variables;  // Use std::any to store any type

public:
    // Access a namespace (creates it if it doesn't exist)
    NamespaceVariables& operator[](const std::string& ns) {
        if (namespaces.find(ns) == namespaces.end()) {
            namespaces[ns] = new NamespaceVariables();
        }
        return *namespaces[ns];
    }

    // Set a variable within the current namespace
    template <typename T>
    void set(const std::string& key, const T& value) {
        variables[key] = Variable<std::any>(key, value);
    }

    // Get a variable within the current namespace
    template <typename T>
    T get(const std::string& key) const {
        try {
            return std::any_cast<T>(variables.at(key).getValue());
        } catch (const std::bad_any_cast& e) {
            std::cerr << "Error: Invalid type cast for key: " << key << std::endl;
            throw; // Re-throw the exception or handle it differently
        }
    }


    // Helper function to print all variables in a namespace (and sub-namespaces)
    void print(const std::string& prefix = "") const {
        for (const auto& var : variables) {
            std::cout << prefix << var.first << ": ";
            try { // Attempt to print common types
                std::cout << std::any_cast<std::string>(var.second.getValue()) << std::endl;
            } catch(...) {
                try { std::cout << std::any_cast<int>(var.second.getValue()) << std::endl; }
                catch(...) {
                    try { std::cout << std::any_cast<bool>(var.second.getValue()) << std::endl; }
                    catch(...) {
                        try { std::cout << std::any_cast<std::filesystem::path>(var.second.getValue()) << std::endl; }
                        catch(...) { std::cout << "[Unprintable Type]" << std::endl; }
                    }
                }
            }
        }
        for (const auto& ns : namespaces) {
            std::cout << prefix << ns.first << ":" << std::endl;
            ns.second->print(prefix + "  ");
        }
    }


};




int main() {
    NamespaceVariables vars;

    vars["global"]["configs"]["agents"]["standard"].set("name", "Standard Agent");
    vars["agent-1"]["state"]["input"].set("value", 123);
    vars["global"]["configs"]["port"].set("number", 8080);
    vars["global"]["configs"]["path"].set<std::filesystem::path>("value", "data/config.json");


    vars.print();


    std::cout << "Agent name: " << vars["global"]["configs"]["agents"]["standard"].get<std::string>("name") << std::endl;
    std::cout << "Port number: " << vars["global"]["configs"]["port"].get<int>("number") << std::endl;

    // Example demonstrating error handling:
    try {
        std::string portNumber = vars["global"]["configs"]["port"].get<std::string>("number"); // Incorrect type
    } catch (const std::bad_any_cast& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }


    return 0;
}
```

### File: main.cpp
```cpp
// main.cpp

#include <chrono>  // Included via agent.hpp but good practice here too
#include <cstdlib> // For std::getenv
#include <iomanip> // Included via agent.hpp but good practice here too
#include <iostream>
#include <memory> // For Tool pointers (optional but good practice)
#include <stdexcept>
#include <string>
#include <vector>

#include "json/json.h" // For Json::Value used by tools
#include <curl/curl.h> // For curl_global_init/cleanup

#include "inc/Agent.hpp"
#include "inc/MiniGemini.hpp"
#include "inc/Tool.hpp"
#include "inc/modelApi.hpp" // For ApiError

#include <chrono>  // Included via agent.hpp but good practice here too
#include <cstdlib> // For std::getenv
#include <iomanip> // Included via agent.hpp but good practice here too
#include <iostream>
#include <memory> // For Tool pointers (optional but good practice)
#include <stdexcept>
#include <string>
#include <vector>

#include "json/json.h" // For Json::Value used by tools
#include <curl/curl.h> // For curl_global_init/cleanup

#include "inc/Agent.hpp"
#include "inc/MiniGemini.hpp"
#include "inc/Tool.hpp"
#include "inc/modelApi.hpp" // For ApiError

// --- Forward Declarations for External Tool Functions ---
extern std::string executeBashCommandReal(const Json::Value &params);
extern std::string calendarTool(const Json::Value &params);
extern std::string duckduckgoSearchTool(const Json::Value &params);
extern std::string fileTool(const Json::Value &params);
extern std::string getTime(const Json::Value &params);
extern std::string calculate(const Json::Value &params);
extern std::string webSearchTool(const Json::Value &params);
extern std::string writeFileTool(const Json::Value &params);
// --- Logging Function ---
void logMessageMain(LogLevel level, const std::string &message,
                    const std::string &details = "") {
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
    prefix = "[DEBUG] ";/
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
                          : std::cout; out << color_start << std::string(time_buffer) << " " << prefix << message
      << color_end << std::endl;
  if (!details.empty()) {
    out << color_start << "  " << details.substr(0, 500)
        << (details.length() > 500 ? "..." : "") << color_end << std::endl;
  }
}

// --- Orchestrator Setup and Run Function ---
int startOrchestrator(const std::string &apiKey, Agent *noteAgent) {
  logMessageMain(LogLevel::INFO, "--- Starting Orchestrator Agent ---");
  std::vector<Tool *> orchestratorTools;
  try {
    MiniGemini orchestratorApi(apiKey);
    Agent orchestratorAgent(orchestratorApi);
    orchestratorAgent.setName("Orchestrator");
    orchestratorAgent.setDescription("Top-level agent coordinating tasks.");

    if (noteAgent) {
      orchestratorAgent.addAgent(noteAgent);
      logMessageMain(LogLevel::INFO,
                     "Registered 'NoteMaster' with Orchestrator.");
    }

    // ** UPDATED Orchestrator System Prompt **
    orchestratorAgent.setSystemPrompt(
        R"(SYSTEM PROMPT: Orchestrator Agent (Named: Demurge)

**Role:** Central coordinator agent (basically the master agent if you will). Serve the Master (Named:CleverLord) by translating high-level goals into executable workflows, leveraging sub-agents and tools. Aim for 10x-100x amplification of the Master's capabilities (Jarvis/Alfred vision).

**Core Interaction Model:**
You MUST respond with a single JSON object containing the following fields:
1.  `thought`: (String) Your reasoning, analysis of the current situation, and plan for the next step(s). Be concise but clear.
2.  `tool_calls`: (Array of Objects | null) A list of tools to execute *now*. Each object in the array MUST be formatted EXACTLY as: `{"tool_name": "...", "params": { ... }}`. If no tools need to be called, this should be `null` or an empty array `[]`.
3.  `final_response`: (String | null) The final answer or response to the user/Master. Provide this ONLY when the entire task is complete and no further actions are needed. If more steps (tool calls, thinking) are required, this MUST be `null`.

**Operational Guidelines:**
*   **Decomposition:** Break down complex requests from the Master into logical steps.
*   **Tool Usage:** Use available tools via the `tool_calls` field. Prioritize specific tools over `bash`. Use `bash` cautiously. Use `help` if unsure about tool parameters.
*   **Delegation:** Use the `promptAgent` tool in `tool_calls` to delegate tasks to specialized agents. That said if the task at hand isnt big enough, there is no need for of loading to sub-agents (Again. use tools at hand smartly to achieve the desired result). Provide clear context in the `prompt` parameter.
*   **Agent Creation:** Use the `createAgent` tool to instantiate new agents from YAML configs when needed. (not implemented yet)
*   **Synthesis:** If previous steps involved tool calls or agent delegation, use your `thought` field to analyze the results (present in history) and plan the next step or formulate the `final_response`.
*   **Context & Inference:** Utilize conversation history and available memory to understand context, infer missing details ("the nitty gritty gritty"), and anticipate needs. Minimize clarification requests.
*   **Workflow Control:** If a task requires multiple LLM iterations (e.g., read file -> process -> write file), use `tool_calls` for the first action(s) and set `final_response` to `null`. The results will appear in the history for your next turn. Use the `skip` tool *only* if you explicitly need the system to pause *before* generating the next LLM response after tool execution (rarely needed with this structured format).
*   **Error Handling:** If a tool call fails (indicated in history), your `thought` should analyze the error, and your next action could be to retry, use a different tool, or inform the Master in `final_response` if the task cannot be completed.

**Example Response Format (Calling a tool):**
```json
{
  "thought": "The Master wants the weather in London. I need to use the getWeather tool.",
  "tool_calls": [
    {
      "tool_name": "getWeather",
      "params": { "location": "London" }
    }
  ],
  "final_response": null
}


Example Response Format (Final Answer):

{
  "thought": "I received the weather for London from the tool. The task is complete, I can now provide the final response.",
  "tool_calls": null,
  "final_response": "The current weather in London is [Weather Details from Tool Result]."
}
IGNORE_WHEN_COPYING_START
content_copy
download
Use code with caution. 
Json
IGNORE_WHEN_COPYING_END

Adhere strictly to this JSON response format. Orchestrate effectively.
)");

    // --- Register Tools for Orchestrator ---
    Tool *timeTool =
        new Tool("time", "Get the current system date and time. Parameters: {}",
                 getTime);
    orchestratorAgent.addTool(timeTool);
    orchestratorTools.push_back(timeTool);

    Tool *calcTool = new Tool("calc",
                              "Calculate a simple arithmetic expression. "
                              "Parameters: {\"expression\": \"string\"}",
                              calculate);
    orchestratorAgent.addTool(calcTool);
    orchestratorTools.push_back(calcTool);

    Tool *webSearch = new Tool(
        "web",
        "Performs web search. Parameters: {\"query\": \"string\", "
        "\"num_results\": int?, \"search_engine\": \"google\"|\"duckduckgo\"?}",
        webSearchTool);
    orchestratorAgent.addTool(webSearch);
    orchestratorTools.push_back(webSearch);

    Tool *ddgSearch =
        new Tool("ddg_search",
                 "[Alt Web Search] Uses HTML scraping. Parameters: {\"query\": "
                 "\"string\", \"num_results\": int?}",
                 duckduckgoSearchTool);
    orchestratorAgent.addTool(ddgSearch);
    orchestratorTools.push_back(ddgSearch);

    Tool *fileOpsTool = new Tool(
        "file",
        "File system operations (read, write, list, info, delete, mkdir). "
        "Params: {'action': string, 'path': string, 'content': string?}. "
        "Relative paths assumed unless $NOTES is used.",
        fileTool);
    orchestratorAgent.addTool(fileOpsTool);
    orchestratorTools.push_back(fileOpsTool);

    Tool *bashTool = new Tool("bash",
                              "Executes raw Bash command. Params: "
                              "{\"command\": \"string\"}",
                              executeBashCommandReal);
    orchestratorAgent.addTool(bashTool);
    orchestratorTools.push_back(bashTool);

    // TODO: Add Agent Factory Tool registration here once implemented
    // extern std::string createAgentFromYamlTool(const Json::Value& params,
    // Agent& callingAgent); Tool* agentFactoryTool = new Tool("createAgent",
    // "Creates agent from YAML. Params: {\"config_path\": \"string\"}",
    // createAgentFromYamlTool, orchestratorAgent);
    // orchestratorAgent.addTool(agentFactoryTool);
    // orchestratorTools.push_back(agentFactoryTool);

    logMessageMain(LogLevel::INFO, "Tools registered with Orchestrator.");
    orchestratorAgent.run();

  } catch (const ApiError &e) {
    logMessageMain(LogLevel::ERROR, "Orchestrator API Error:", e.what());
    for (Tool *tool : orchestratorTools)
      delete tool;
    return 1;
  } catch (const std::exception &e) {
    logMessageMain(LogLevel::ERROR, "Orchestrator Error:", e.what());
    for (Tool *tool : orchestratorTools)
      delete tool;
    return 1;
  } catch (...) {
    logMessageMain(LogLevel::ERROR, "Unknown error in Orchestrator.");
    for (Tool *tool : orchestratorTools)
      delete tool;
    return 1;
  }

  for (Tool *tool : orchestratorTools)
    delete tool;
  logMessageMain(LogLevel::INFO, "--- Orchestrator Agent Finished ---");
  return 0;
}

// --- Main Application Entry Point ---
int main() {
  CURLcode curl_global_res = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (curl_global_res != CURLE_OK) {
    std::cerr << "Fatal: Failed to initialize libcurl globally: "
              << curl_easy_strerror(curl_global_res) << std::endl;
    return 1;
  }
  logMessageMain(LogLevel::INFO, "libcurl initialized.");

  std::string apiKey;
  Agent *noteAgentPtr = nullptr;
  std::vector<Tool *> noteMasterTools;

  try {
    logMessageMain(LogLevel::INFO, "--- Setting up NoteMaster Agent ---");

    const char *apiKeyEnv = std::getenv("GEMINI_API_KEY");
    if (!apiKeyEnv || std::string(apiKeyEnv).empty()) {
      logMessageMain(LogLevel::ERROR,
                     "GEMINI_API_KEY environment variable not set or empty.");
      curl_global_cleanup();
      return 1;
    }
    apiKey = apiKeyEnv;
    logMessageMain(LogLevel::INFO, "API Key loaded.");

    MiniGemini noteApiClient(apiKey);
    logMessageMain(LogLevel::INFO, "NoteMaster API client initialized.");

    noteAgentPtr = new Agent(noteApiClient);
    noteAgentPtr->setName("NoteMaster");
    noteAgentPtr->setDescription(
        "Specialized agent for managing notes ($NOTES), calendar events.");
    // Ensure this directory exists or the agent can create it.
    noteAgentPtr->addEnvVar("NOTES", "/home/mlamkadm/notes");
    logMessageMain(LogLevel::INFO, "Agent 'NoteMaster' created.");

    // ** UPDATED NoteMaster System Prompt **
    noteAgentPtr->setSystemPrompt(R"(SYSTEM PROMPT: NoteMaster Agent

Role: Automated agent managing notes and calendar events within the $NOTES directory.

    Core Interaction Model:
    You MUST respond with a single JSON object containing the following fields:

thought: (String) Your reasoning about the request and plan.

tool_calls: (Array of Objects | null) Tools to execute now, each formatted as {"tool_name": "...", "params": { ... }}. Use null or [] if no tools are needed.

final_response: (String | null) The final response to the caller (usually Orchestrator). Provide ONLY when the task is fully complete. Set to null if more steps/tools are needed.

Operational Guidelines:

Focus: Primarily use the file tool for CRUD operations on notes within the $NOTES directory and the calendar tool for events. Use full paths when possible, leveraging the $NOTES env var.

Execution: Execute requests from the Orchestrator or Master precisely. Infer intent for missing details; do not ask for clarification.

Tool Format: Adhere strictly to the {"tool_name": "...", "params": { ... }} format within the tool_calls array.

Response: Ensure final_response is non-null only upon task completion. If calling tools, final_response MUST be null.

Error Handling: If a tool fails, note it in your thought and potentially inform the caller via final_response if the task cannot proceed.

Example Response (Adding a calendar event):

{
  "thought": "Request received to add a calendar event. I will use the 'calendar' tool with the 'add' action.",
  "tool_calls": [
    {
      "tool_name": "calendar",
      "params": {
        "action": "add",
        "date": "YYYY-MM-DD", // Fill in calculated date
        "time": "14:00",
        "description": "Team Meeting"
      }
    }
  ],
  "final_response": null
}

Example Response (After successful tool call):

{
  "thought": "The calendar tool reported success in adding the event. The task is complete.",
  "tool_calls": null,
  "final_response": "Success: Calendar event 'Team Meeting' added for YYYY-MM-DD at 14:00."
}

)");
    logMessageMain(LogLevel::INFO, "System prompt set for NoteMaster.");

    // Register Tools for NoteMaster
    Tool *file = new Tool("file",
                          "File system operations (read, write, list, info, "
                          "delete, mkdir) within $NOTES env var. Params: {'action': "
                          "string, 'path': string, 'content': string?}",
                          fileTool);
    noteAgentPtr->addTool(file);
    noteMasterTools.push_back(file);

    Tool *calTool = new Tool(
        "calendar",
        "Manages calendar events (add, list). Params: {'action': 'add'|'list', "
        "'date': 'YYYY-MM-DD', 'time': 'HH:MM'?, 'description': string?}",
        calendarTool);
    noteAgentPtr->addTool(calTool);
    noteMasterTools.push_back(calTool);

    // Bash tool potentially restricted for NoteMaster
    // Tool* noteBashTool = new Tool("bash", "[DANGEROUS] Bash commands ONLY
    // within $NOTES. Params: {\"command\": \"string\"}",
    // executeBashCommandReal); noteAgentPtr->addTool(noteBashTool);
    // noteMasterTools.push_back(noteBashTool);

    logMessageMain(LogLevel::INFO, "Tools registered with NoteMaster.");

  } catch (const ApiError &e) {
    logMessageMain(LogLevel::ERROR, "NoteMaster Setup API Error:", e.what());
    for (Tool *tool : noteMasterTools)
      delete tool;
    if (noteAgentPtr)
      delete noteAgentPtr;
    curl_global_cleanup();
    return 1;
  } catch (const std::invalid_argument &e) {
    logMessageMain(LogLevel::ERROR,
                   "NoteMaster Setup Initialization Error:", e.what());
    for (Tool *tool : noteMasterTools)
      delete tool;
    if (noteAgentPtr)
      delete noteAgentPtr;
    curl_global_cleanup();
    return 1;
  } catch (const std::exception &e) {
    logMessageMain(LogLevel::ERROR,
                   "NoteMaster Setup Unexpected Error:", e.what());
    for (Tool *tool : noteMasterTools)
      delete tool;
    if (noteAgentPtr)
      delete noteAgentPtr;
    curl_global_cleanup();
    return 1;
  } catch (...) {
    logMessageMain(LogLevel::ERROR, "NoteMaster Setup Unknown Fatal Error.");
    for (Tool *tool : noteMasterTools)
      delete tool;
    if (noteAgentPtr)
      delete noteAgentPtr;
    curl_global_cleanup();
    return 1;
  }

  // Start Orchestrator
  int orchestratorStatus = startOrchestrator(apiKey, noteAgentPtr);

  // Cleanup
  logMessageMain(LogLevel::INFO, "Cleaning up resources...");
  for (Tool *tool : noteMasterTools)
    delete tool;
  if (noteAgentPtr)
    delete noteAgentPtr;
  curl_global_cleanup();
  logMessageMain(LogLevel::INFO, "Application Shutting Down.");
}

// **Key Prompt Changes:**
//
// 1.  **Explicit JSON Structure:** Both prompts now explicitly demand a single
// JSON object response with `thought`, `tool_calls`, and `final_response`
// fields.
// 2.  **Field Explanations:** Each field's purpose is clearly defined
// (`thought` for reasoning, `tool_calls` for actions, `final_response` only
// when done).
// 3.  **Tool Call Format within JSON:** The prompts specify that the
// `tool_calls` array must contain objects formatted as `{"tool_name": "...",
// "params": { ... }}`.
// 4.  **Conditional Logic:** Instructions clarify that `final_response` should
// be `null` if `tool_calls` are present, and vice-versa (enforcing the idea
// that the agent either thinks/acts *or* gives a final answer in one turn).
// 5.  **Workflow/Multi-Step Handling:** The prompts explain how to handle
// multi-step tasks by using `tool_calls` and setting `final_response` to
// `null`, relying on the history/context for the next iteration. The role of
// the `skip` tool is de-emphasized slightly as the structured format provides
// better control.
// 6.  **Error Handling Guidance:** Briefly mentions how to handle tool errors
// (analyze in `thought`, potentially inform in `final_response`).
// 7.  **Role-Specific Instructions:** The prompts retain their specific focus
// (Orchestrator coordinates/delegates, NoteMaster manages notes/calendar in
// $NOTES).
// 8.  **Examples:** Clear examples of the expected JSON output format are
// provided for both tool calls and final responses.
//
// These updated prompts should guide the LLM much more effectively to produce
// output compatible with the enhanced `Agent::prompt` loop logic discussed
// previously. Remember that the `Agent::prompt` function itself needs the
// corresponding parsing logic (`parseStructuredLLMResponse`) to handle this new
// format.
```

### File: Makefile
```
# Compiler
CXX := clang++
# CXX := g++

# Target Executable Names
TARGET_BIN := agent-bin
TARGET_SERVER := agent-server

# Directories
BUILD_DIR := build
SRC_DIR := src
EXT_DIR := externals
SERVER_DIR := server
INC_DIRS := inc $(SERVER_DIR)/vendor/httplib
# --- yaml-cpp ---
# NOTE: If yaml-cpp headers are installed in a standard location searched by pkg-config,
# adding its specific path to INC_DIRS might not be strictly necessary when using pkg-config for CFLAGS.
# If pkg-config isn't used or headers are elsewhere, add the path here:
# INC_DIRS += /path/to/yaml-cpp/include

# Flags
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2 -g

# --- yaml-cpp ---
# Use pkg-config to get compiler flags (includes -I paths etc.)
# Replace 'yaml-cpp' if your pkg-config file has a different name (e.g., yaml-cpp-0.7)
# You can find the name using: pkg-config --list-all | grep -i yaml
YAML_CPP_CFLAGS := $(shell pkg-config --cflags yaml-cpp 2>/dev/null)

# Construct CPPFLAGS including standard includes and yaml-cpp includes
CPPFLAGS := $(foreach dir,$(INC_DIRS),-I$(dir)) $(YAML_CPP_CFLAGS)
# Fallback if pkg-config fails or isn't used:
# CPPFLAGS := $(foreach dir,$(INC_DIRS),-I$(dir)) -I/usr/local/include # Or specific yaml-cpp path

# --- yaml-cpp ---
# Add yaml-cpp library link flag using pkg-config
YAML_CPP_LIBS := $(shell pkg-config --libs yaml-cpp 2>/dev/null)
# Original LDFLAGS + yaml-cpp libs
LDFLAGS := -lcurl -ljsoncpp -pthread $(YAML_CPP_LIBS)
# Fallback if pkg-config fails or isn't used:
# LDFLAGS := -lcurl -ljsoncpp -pthread -lyaml-cpp -L/usr/local/lib # Add -L if library is non-standard path

# Source files - use recursive wildcard to find all source files
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

MAIN_SRC := import.main.cpp
SERVER_SRC := $(SERVER_DIR)/server.cpp

# Recursively find all source files in src/ and externals/
SRC_SOURCES := $(call rwildcard,$(SRC_DIR),*.cpp)
EXT_SOURCES := $(call rwildcard,$(EXT_DIR),*.cpp)

COMMON_SOURCES := $(SRC_SOURCES) $(EXT_SOURCES)
ALL_SOURCES := $(MAIN_SRC) $(SERVER_SRC) $(COMMON_SOURCES)

# Object files with proper subdirectory structure
MAIN_OBJ := $(BUILD_DIR)/$(MAIN_SRC:.cpp=.o)
SERVER_OBJ := $(BUILD_DIR)/$(SERVER_SRC:.cpp=.o)

# Generate object file paths with proper directory structure
SRC_OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRC_SOURCES))
EXT_OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(EXT_SOURCES))

COMMON_OBJECTS := $(SRC_OBJECTS) $(EXT_OBJECTS)
ALL_OBJECTS := $(MAIN_OBJ) $(SERVER_OBJ) $(COMMON_OBJECTS)

# Dependency files
DEPS := $(ALL_OBJECTS:.o=.d)

# Default target
all: $(TARGET_SERVER)

# Rule to build 'bin' executable
$(TARGET_BIN): $(MAIN_OBJ) $(COMMON_OBJECTS)
	@echo "Linking $(TARGET_BIN)..."
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) # LDFLAGS now includes yaml-cpp
	@echo "$(TARGET_BIN) built successfully."

# Rule to build server executable
$(TARGET_SERVER): $(SERVER_OBJ) $(COMMON_OBJECTS)
	@echo "Linking $(TARGET_SERVER)..."
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) # LDFLAGS now includes yaml-cpp
	@echo "$(TARGET_SERVER) built successfully."

# Generic rule for compiling any .cpp file
# CPPFLAGS now includes yaml-cpp include paths
$(BUILD_DIR)/%.o: %.cpp
	@echo "Compiling $< -> $@"
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Include dependencies
-include $(DEPS)

# Convenience targets
bin: $(TARGET_BIN)
server: $(TARGET_SERVER)

clean:
	@echo "Cleaning build files..."
	rm -f $(TARGET_BIN) $(TARGET_SERVER)
	rm -rf $(BUILD_DIR)
	@echo "Clean complete."

# Rebuild targets
re: clean all
re-bin: clean bin
re-server: clean server

# Run targets
run-bin: bin
	@echo "Running $(TARGET_BIN)..."
	./$(TARGET_BIN)

run-server: server
	@echo "Running $(TARGET_SERVER)..."
	./$(TARGET_SERVER)

# Debug target to see what source files are found
debug:
	@echo "Source files found:"
	@echo "  Main: $(MAIN_SRC)"
	@echo "  Server: $(SERVER_SRC)"
	@echo "  SRC_SOURCES: $(SRC_SOURCES)"
	@echo "  EXT_SOURCES: $(EXT_SOURCES)"
	@echo ""
	@echo "Object files to build:"
	@echo "  MAIN_OBJ: $(MAIN_OBJ)"
	@echo "  SERVER_OBJ: $(SERVER_OBJ)"
	@echo "  SRC_OBJECTS: $(SRC_OBJECTS)"
	@echo "  EXT_OBJECTS: $(EXT_OBJECTS)"
	@echo ""
	@echo "Compiler Flags (CPPFLAGS): $(CPPFLAGS)"
	@echo "Linker Flags (LDFLAGS): $(LDFLAGS)"


.PHONY: all bin server clean re re-bin re-server run-bin run-server debug

# Prevent deletion of intermediate files
.SECONDARY: $(ALL_OBJECTS)
```

### File: newv.cpp
```cpp

// main.cpp

#include <chrono>  // Included via agent.hpp but good practice here too
#include <cstdlib> // For std::getenv
#include <iomanip> // Included via agent.hpp but good practice here too
#include <iostream>
#include <memory> // For Tool pointers (optional but good practice)
#include <stdexcept>
#include <string>
#include <vector>

#include "json/json.h" // For Json::Value used by tools
#include <curl/curl.h> // For curl_global_init/cleanup

#include "inc/Agent.hpp"
#include "inc/MiniGemini.hpp"
#include "inc/Tool.hpp"
#include "inc/modelApi.hpp" // For ApiError

#include <chrono>  // Included via agent.hpp but good practice here too
#include <cstdlib> // For std::getenv
#include <iomanip> // Included via agent.hpp but good practice here too
#include <iostream>
#include <memory> // For Tool pointers (optional but good practice)
#include <stdexcept>
#include <string>
#include <vector>

#include "json/json.h" // For Json::Value used by tools
#include <curl/curl.h> // For curl_global_init/cleanup

#include "inc/Agent.hpp"
#include "inc/MiniGemini.hpp"
#include "inc/Tool.hpp"
#include "inc/modelApi.hpp" // For ApiError

// --- Logging Function ---
void logMessageMain(LogLevel level, const std::string &message,
                    const std::string &details = "") {
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
                          : std::cout; out << color_start << std::string(time_buffer) << " " << prefix << message
      << color_end << std::endl;
  if (!details.empty()) {
    out << color_start << "  " << details.substr(0, 500)
        << (details.length() > 500 ? "..." : "") << color_end << std::endl;
  }
}
// --- Main Application Entry Point ---
int main() {
  CURLcode curl_global_res = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (curl_global_res != CURLE_OK) {
    std::cerr << "Fatal: Failed to initialize libcurl globally: "
              << curl_easy_strerror(curl_global_res) << std::endl;
    return 1;
  }
  // logMessageMain(LogLevel::INFO, "libcurl initialized.");

  std::string apiKey;
  Agent *noteAgentPtr = nullptr;
  std::vector<Tool *> noteMasterTools;

  try {
    // logMessageMain(LogLevel::INFO, "--- Setting up NoteMaster Agent ---");

    const char *apiKeyEnv = std::getenv("GEMINI_API_KEY");
    if (!apiKeyEnv || std::string(apiKeyEnv).empty()) {
      // logMessageMain(LogLevel::ERROR,
      //                "GEMINI_API_KEY environment variable not set or empty.");
      curl_global_cleanup();
      return 1;
    }
    apiKey = apiKeyEnv;
    // logMessageMain(LogLevel::INFO, "API Key loaded.");

    MiniGemini noteApiClient(apiKey);
    logMessageMain(LogLevel::INFO, "NoteMaster API client initialized.");
    // noteAgentPtr = new Agent(noteApiClient, "./agent_config.yaml");


  } catch (const ApiError &e) {
    // logMessageMain(LogLevel::ERROR, "NoteMaster Setup API Error:", e.what());
    for (Tool *tool : noteMasterTools)
      delete tool;
    if (noteAgentPtr)
      delete noteAgentPtr;
    curl_global_cleanup();
    return 1;
  } catch (const std::invalid_argument &e) {
    // logMessageMain(LogLevel::ERROR,
    //                "NoteMaster Setup Initialization Error:", e.what());
    for (Tool *tool : noteMasterTools)
      delete tool;
    if (noteAgentPtr)
      delete noteAgentPtr;
    curl_global_cleanup();
    return 1;
  } catch (const std::exception &e) {
    // logMessageMain(LogLevel::ERROR,
    //                "NoteMaster Setup Unexpected Error:", e.what());
    for (Tool *tool : noteMasterTools)
      delete tool;
    if (noteAgentPtr)
      delete noteAgentPtr;
    curl_global_cleanup();
    return 1;
  } catch (...) {
    // logMessageMain(LogLevel::ERROR, "NoteMaster Setup Unknown Fatal Error.");
    for (Tool *tool : noteMasterTools)
      delete tool;
    if (noteAgentPtr)
      delete noteAgentPtr;
    curl_global_cleanup();
    return 1;
  }

  // Start Orchestrator
  // int orchestratorStatus = startOrchestrator(apiKey, noteAgentPtr);

  // Cleanup
  // logMessageMain(LogLevel::INFO, "Cleaning up resources...");
  for (Tool *tool : noteMasterTools)
    delete tool;
  if (noteAgentPtr)
    delete noteAgentPtr;
  curl_global_cleanup();
  // logMessageMain(LogLevel::INFO, "Application Shutting Down.");
}
```

### File: prompts/build_tool.md
```markdown
# Tool Generation Prompt for MiniGemini C++ Framework

## Context
You are helping to create new tools for a C++ framework that implements an agent capable of using tools. The framework consists of:
- An `Agent` class that manages interaction with a language model
- A `MiniGemini` API client
- A `Tool` class for registering and executing tool callbacks

## Tool Structure
Each tool in this framework:
1. Has a name (string identifier)
2. Has a description explaining its functionality
3. Is implemented as a callback function that:
   - Takes a Json::Value object of parameters as input
   - Returns a string result
   - May perform various operations based on the provided parameters

## Requirements for New Tools

### Functional Requirements
1. Create tool callback functions with the signature: `std::string functionName(const Json::Value &params)`
2. Tools should handle JSON parameters properly, including parameter validation
3. Tools should return meaningful string responses
4. Error handling should be robust, with informative error messages

### Parameter Handling Requirements
1. Check if parameters exist using `params.isMember("paramName")`
2. Validate parameter types with methods like `params["paramName"].isInt()`
3. Extract values using appropriate accessor methods (e.g., `params["paramName"].asInt()`)
4. Provide default values for optional parameters
5. Return error messages for invalid or missing required parameters

## Examples to Reference

Here are some existing tool implementations to use as reference:

### Time Tool
```cpp
// Gets current time (params ignored)
std::string getTime(const Json::Value &params) {
  (void)params; // Unused
  time_t now = time(0);
  char buf[80];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
  return buf;
}
```

### Dice Rolling Tool
```cpp
// Rolls a die (uses params)
std::string rollDice(const Json::Value &params) {
  int sides = 6; // Default
  int rolls = 1; // Default
  if (params.isMember("sides") && params["sides"].isInt()) {
    int requestedSides = params["sides"].asInt();
    if (requestedSides > 0) {
      sides = requestedSides;
    }
  }
  std::ostringstream oss;
  if (params.isMember("rolls") && params["rolls"].isInt()) {
    rolls = params["rolls"].asInt();
    while (rolls > 0) {
      int roll = (rand() % sides) + 1;
      oss << "Rolled a " << roll << " (d" << sides << ")";
      rolls--;
    }
  }
  return oss.str();
}
```

### Calculator Tool
```cpp
// Simple tool to calculate normal expressions
std::string calculate(const Json::Value &params) {
    if (params == Json::nullValue) {
        return "No parameters provided.";
    }
    else if (params.isMember("expression")) {
        std::string expression = params["expression"].asString();
        // Simple parsing and evaluation logic
        std::istringstream iss(expression);
        double a, b;
        char op;
        iss >> a >> op >> b;
        double result = 0.0;
        switch (op) {
            case '+': result = a + b; break;
            case '-': result = a - b; break;
            case '*': result = a * b; break;
            case '/': result = (b != 0) ? a / b : 0; break;
            default: return "Invalid operator.";
        }
        return "Result: " + std::to_string(result);
    }
    return "Invalid parameters.";
}
```

## Task
Please generate the following new tools for the MiniGemini framework:

1. A weather tool that simulates retrieving weather information for a location
2. A translation tool that can translate text between languages
3. A file manipulation tool that can handle operations like reading, writing, or listing files
4. A search tool that simulates searching for information
5. A reminder/todo tool that can manage simple reminders or tasks

For each tool, provide:
1. A callback function implementation
2. A description of its purpose and parameters
3. An example of how to create and register the tool with the Agent

## Implementation Guidelines

1. Include proper error handling and parameter validation
2. Use C++ standard library functions where appropriate
3. Simulate external services where needed (no actual API calls required)
4. Keep code consistent with the existing framework style
5. Include comments explaining complex logic
6. Make the tools useful and realistic while remaining simple enough to understand

## Tool Registration Example
```cpp
// Create and register a tool
Tool weatherTool("weather", "Get weather information for a location", getWeather);
agent.addTool(&weatherTool);
```

Please generate each tool implementation following these guidelines and the patterns shown in the example tools.
```

### File: prompts/collection.md
```markdown

{
Code-Gen-Directive: Generate code with consistent naming conventions for readability and maintainability.

    Variables/functions/methods: camelCase
    Classes: PascalCase
    Constants: UPPER_CASE

    Use clear, unique, explicit names
// more directives
}

```

### File: prompts/masterclass-relic.md
```markdown


You are a world-class container internals instructor with a knack for analogies and workflow integration. For any requested topic:

1. Open with a one-paragraph summary that highlights the core insights.
2. Break the content into clearly labeled modules (## Module X: Title).
3. Within each module:
   a. Provide a conceptual overview linked to a tangible analogy.
   b. Explain why it matters via a real-world example.
   c. List 2–3 common pitfalls and what to avoid.
   d. Suggest 3 actionable habits or best practices for daily workflow.
4. End with Key Takeaways: bullet-point “memory triggers.”
5. Maintain a friendly, mentor-like tone; bold new terms and italicize metaphors.
6. Format using Markdown, with `##` for sections and `###` for subsections.
7. Cite any factual claims or specs.
  
Use teach-back checkpoints (“Explain in your own words…”) and spaced-review reminders (“Review this concept in 1 week…”).  



// Finetune


You are a world-class engineering mentor who conveys deep technical concepts through relatable analogies, real-world examples, and workflow habits. For any requested topic:

1. **Summary (1 paragraph):** Provide a concise overview of what learners will master.
2. **Module Structure:** Divide into modules (`## Module X: Title`), each with:
   a. **Concept & Analogy:** Map the core idea to a vivid, everyday metaphor.
   b. **Why It Matters:** Show a practical scenario or use case.
   c. **Pitfalls & What to Avoid:** List common mistakes and their impact.
   d. **Workflow Habits & Best Practices:** Recommend 3 daily habits to reinforce the concept.
3. **Key Takeaways:** End each module with 3–5 bullet-point “memory triggers.”
4. **Pedagogy Hooks:** Embed teach-back checkpoints (“Explain in your own words…”) and spaced-review reminders (“Review this in 1 week…”).
5. **Formatting & Style:** Use `##` and `###` headings, **bold** first mentions of terms, *italicize* metaphors, and cite sources for factual claims.
6. **Tone:** Maintain a friendly, expert, mentor-like voice that encourages curiosity and mastery.

Always place instructions at the top, be specific in language, and structure outputs to maximize clarity, retention, and actionable insight.
```

### File: prompts/workflow_context.md
```markdown
Okay, here is a report summarizing the potential for workflow automation within the C++ agent framework, based on our discussion and the analysis provided in `context.md`.

## Report: Workflow Automation Potential in the C++ Agent Framework

**Generated:** Sunday, April 13, 2025, 5:54 PM +01
**Location Context:** Khouribga, Béni Mellal-Khenifra, Morocco

**1. Introduction**

This report assesses the capabilities and potential for implementing workflow automation within the C++ agent framework detailed in the provided `context.md` analysis. The framework demonstrates a robust foundation for automation through its modular tool system and agent orchestration logic.

**2. Core Framework Components for Automation**

The analysis in `context.md` highlights several key components that enable automation:

* **Tool-Based Architecture:** The system relies on distinct "Tools" (`inc/Tool.hpp`) for specific functions. This modularity allows for easy expansion with new automation capabilities.
* **Existing Foundational Tools:** The framework already includes essential tools in the `externals` directory that are crucial for many automation tasks:
    * `bash.cpp`: Enables execution of shell commands for system interaction and scripting.
    * `file.cpp` & `write.cpp`: Provide comprehensive file system operations (read, write, list, delete, mkdir).
    * `cal-events.cpp`: Manages calendar events.
    * `ddg-search.cpp`: Performs web searches.
* **Agent Orchestration:** The `Agent` class (`inc/Agent.hpp`) manages the conversation flow, integrates with LLMs (`MiniGemini.hpp`, `Groq.hpp`), and crucially, handles the execution of tool calls requested by the LLM. It can parse requests (`extractToolCalls`), run the corresponding tool code (`handleToolExecution`), and incorporate results back into the context (`processToolCalls`), enabling multi-step workflows.
* **Multi-Agent Capability:** The framework supports multiple agent instances and includes a `promptAgent` tool, allowing for the distribution of complex automation tasks across specialized agents.

**3. Potential Workflow Automation Use Cases**

Based on the framework's structure, several automation workflows can be implemented or enhanced:

* **Automated Daily Briefing:**
    * **Sequence:** Fetch today's events (`cal-events.cpp`) -> Search relevant news (`ddg-search.cpp` / new API tool) -> *(Optional: Fetch weather via new API tool)* -> Summarize findings (LLM) -> Save/Send report (`file.cpp` / new Email tool).
    * **Relies on:** Existing calendar/search tools, LLM summarization, file output, potential new API/Email tools.
* **Simple CI/CD Task (Code Check):**
    * **Sequence:** Check project dir (`file.cpp`) -> Pull latest code (`bash.cpp` or new Git tool) -> Run build script (`bash.cpp`) -> Report status (LLM synthesis of tool output).
    * **Relies on:** Existing file/bash tools, potential new Git tool.
* **Content Aggregation & Summarization:**
    * **Sequence:** Read URL list (`file.cpp`) -> Fetch content (new Web Scraper tool) -> Compare with old version (`file.cpp`) -> Summarize diff (LLM) -> Update stored version (`file.cpp`) -> Compile summaries (LLM).
    * **Relies on:** Existing file tools, LLM summarization, new Web Scraper tool.
* **Automated File Organization:**
    * **Sequence:** List directory (`file.cpp`) -> Get file info/type (`file.cpp` / LLM classification) -> Determine target dir (rules/LLM) -> Create target dir if needed (`file.cpp`) -> Move file (`bash.cpp` `mv` command) -> Report actions (LLM).
    * **Relies on:** Existing file/bash tools, LLM classification/reasoning.

**4. Enhancing Automation Capabilities**

Further potential can be unlocked by developing new tools and enhancing agent capabilities:

* **New Tools:** Database interaction, generalized API interaction, email management, sandboxed code execution, version control (Git), and image manipulation tools would significantly broaden the scope of automatable tasks.
* **Agent Enhancements:** Implementing features like goal-oriented planning, more sophisticated memory management, dynamic tool loading, and enhanced failure recovery would make the agent more autonomous and robust in executing complex workflows.

**5. Conclusion**

The C++ agent framework analyzed in `context.md` possesses a strong and flexible architecture well-suited for workflow automation. Its reliance on modular tools orchestrated by the central `Agent` class, combined with the power of integrated LLMs, allows for the implementation of diverse and complex automated tasks. By leveraging existing tools and strategically adding new ones, this framework can be developed into a powerful platform for various automation use cases.
```

### File: README.md
```markdown
This project implements an agent that interacts with a language model and uses tools to accomplish tasks. The agent can execute bash commands, read and write files, and access the current time.

## Documentation

### Agent Class

The `Agent` class is the core component of this project. It is responsible for interacting with the language model, managing tools, and processing user input.

#### Core Components

- `MiniGemini &m_api`: A reference to the API client used to interact with the language model.
- `std::map<std::string, Tool *> m_tools`: A map of available external tools. The keys are the tool names, and the values are pointers to the `Tool` objects.
- `std::map<std::string, std::string> m_internalToolDescriptions`: A map of descriptions for internal tools, such as `help`.

#### State

- `std::string m_systemPrompt`: The system prompt used to initialize the language model.
- `std::vector<std::pair<std::string, std::string>> m_history`: A conversation history, stored as pairs of roles and content.
- `int iteration`: The current iteration number.
- `int iterationCap`: The maximum number of iterations.
- `bool skipFlowIteration`: A flag to skip the final LLM call after tool execution.
- `std::vector<std::pair<std::string, std::string>> _env`: Stores the results of tool calls for later use.
- `fileList _files`: Stores files for later use.
- `std::string _name`: Stores the agent's name.
- `StrkeyValuePair Scratchpad`: A scratchpad for temporary storage.
- `StrkeyValuePair ShortTermMemory`: Short-term memory for the agent.
- `StrkeyValuePair LongTermMemory`: Long-term memory for the agent.

#### Methods

- `Agent(MiniGemini &api)`: Constructor. Takes a `MiniGemini` object by reference.
- `void setSystemPrompt(const std::string &prompt)`: Sets the system prompt.
- `void addTool(Tool *tool)`: Adds a tool to the agent.
- `void addTextTool(Tool *tool)`: Adds a text-based tool to the agent.
- `void reset()`: Resets the agent's state.
- `std::string prompt(const std::string &userInput)`: Processes user input and interacts with the language model.
- `void run()`: Starts the interactive loop.
- `void addMemory(const std::string &role, const std::string &content)`: Adds a memory to the agent's long-term memory.
- `void removeMemory(const std::string &role, const std::string &content)`: Removes a memory from the agent's long-term memory.
- `std::string getMemory(const std::string &key) const`: Retrieves a memory from the agent's long-term memory.
- `void clearMemory()`: Clears the agent's long-term memory.
- `MiniGemini &getApi()`: Returns a reference to the `MiniGemini` object.
- `fileList getFiles()`: Returns the list of files.

### Tool Class

The `Tool` class represents a tool that the agent can use to accomplish tasks.

#### Members

- `std::string m_name`: The name of the tool.
- `std::string m_description`: A description of the tool.
- `ToolCallback m_callback`: A function pointer to the tool's callback function.
- `Agent *m_agent`: A pointer to the agent using this tool.
- `PureTextToolCallback m_text_callback`: A function pointer for text-based tools.
- `std::map<std::string, std::string> m_use_cases`: A map of use cases for the tool.
- `std::map<std::string, std::string> m_memory_stack`: Memory for storing tool state.

#### Methods

- `Tool(const std::string &name, const std::string &description, ToolCallback callback, Agent &agent)`: Constructor.
- `std::string getName() const`: Returns the tool's name.
- `std::string getDescription() const`: Returns the tool's description.
- `std::string execute(const Json::Value &params)`: Executes the tool's callback function.
- `std::string execute(const std::string &params)`: Executes the text-based tool's callback function.
- `void setCallback(ToolCallback callback)`: Sets the tool's callback function.
- `void setBuiltin(ToolCallbackWithAgent callback)`: Sets the tool's built-in callback function.
- `void addUseCase(const std::string &use_case, const std::string &description)`: Adds a use case for the tool.
- `std::string getUseCase(const std::string &use_case) const`: Returns a use case for the tool.
- `void addMemory(const std::string &key, const std::string &value)`: Adds a memory to the tool's memory stack.
- `std::string getMemory(const std::string &key) const`: Retrieves a memory from the tool's memory stack.

### MiniGemini Class

The `MiniGemini` class is responsible for interacting with the Gemini language model API.

#### Members

- `std::string m_apiKey`: The API key used to authenticate with the Gemini API.
- `std::string m_model`: The name of the Gemini language model to use.
- `double m_temperature`: The temperature parameter used to control the randomness of the language model's output.
- `int m_maxTokens`: The maximum number of tokens in the language model's output.
- `const std::string m_baseUrl`: The base URL for the Gemini API.

#### Methods

- `MiniGemini(const std::string &apiKey = "")`: Constructor.
- `std::string generate(const std::string &prompt)`: Generates content based on a prompt.
- `void setApiKey(const std::string &apiKey)`: Sets the API key.
- `void setModel(const std::string &model)`: Sets the model name.
- `void setTemperature(double temperature)`: Sets the temperature.
- `void setMaxTokens(int maxTokens)`: Sets the maximum number of tokens.

### Available tools:

- **help**: Provides descriptions of available tools. Takes an optional 'tool_name' parameter to get help for a specific tool.
- **bash**: Execute a bash command. Requires a JSON object with a 'command' parameter (string). Example: {"command": "ls -l"}.
- **getCurrentTime**: Returns the current date and time. params ignored.
- **write**: Tool Function: Writes content to a file specified by the path on the first line of input.
Input: A string where the first line is the file path, and subsequent lines are the content.
Output: A success message or an error message.```

### File: save.sh
```bash

alias aicp='sh /home/mlamkadm/ai-repos/ai-project-loader.sh'

rm -f  tools.md inc.md src.md context.md

aicp -d ../agent-lib/externals -f tools.md --force
aicp -d ../agent-lib/inc -f inc.md --force
aicp -d ../agent-lib/src -f src.md --force


AGREGATED_FILES=$(cat tools.md inc.md src.md)

echo "$AGREGATED_FILES" > ../agent-lib/context.md
```

### File: server/server.cpp
```cpp
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AI Assistant <ai@assistant.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 22:39:30 by mlamkadm          #+#    #+#             */
/*   Updated: 2025/04/24 14:30:00 by AI Assistant      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Agent.hpp"
#include "../inc/MiniGemini.hpp"
#include "../inc/Tool.hpp"
#include "../inc/modelApi.hpp" // For ApiError
#include "httplib.h"           // HTTP server library
#include <cstdlib>             // For std::getenv
#include <ctime>               // For time functions, std::time, std::localtime, std::strftime
#include <iostream>
#include <json/json.h> // JSON library
#include <memory>      // For std::unique_ptr, std::make_unique
#include <stdexcept>   // For std::runtime_error, std::exception
#include <string>
#include <vector>      // For storing tool unique_ptrs
#include <curl/curl.h> // For curl_global_init/cleanup

// --- Forward Declarations for External Tool Functions ---
extern std::string executeBashCommandReal(const Json::Value &params);
extern std::string calendarTool(const Json::Value &params);
extern std::string fileTool(const Json::Value &params);
extern std::string getTime(const Json::Value &params);
extern std::string calculate(const Json::Value &params);
// extern std::string localSearchTool(const Json::Value¶ms); // Define if using
// extern std::string writeFileTool(const Json::Value ¶ms); // Often redundant
extern std::string swayControlTool(const Json::Value& params); // If using sway

// --- Simple Logger ---
void serverLog(const std::string &level, const std::string &message, const std::string& details = ""){
    std::time_t now = std::time(nullptr);
    // Use thread-safe localtime_r or localtime_s
    std::tm tm_local_buf;
    #ifdef _WIN32
        localtime_s(&tm_local_buf, &now);
        std::tm* tm_local = &tm_local_buf;
    #else
        std::tm* tm_local = localtime_r(&now, &tm_local_buf);
    #endif

    if (!tm_local) { std::cerr << "Error getting time!" << std::endl; return; }

    char time_buffer[20]; // HH:MM:SS
    std::strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", tm_local);

    std::string color_start = "";
    std::string color_end = "\033[0m"; // ANSI Reset color
    std::string prefix_str;

    if (level == "FATAL" || level == "ERROR") { color_start = "\033[1;31m"; prefix_str = "[" + level + "] "; } // Bold Red
    else if (level == "WARN") { color_start = "\033[33m"; prefix_str = "[WARN]  "; } // Yellow
    else if (level == "INFO") { color_start = "\033[32m"; prefix_str = "[INFO]  "; } // Green
    else if (level == "DEBUG") { color_start = "\033[36m"; prefix_str = "[DEBUG] "; } // Cyan
    else if (level == "REQUEST") { color_start = "\033[34m"; prefix_str = "[REQ]   "; } // Blue
    else if (level == "RESPONSE") { color_start = "\033[35m"; prefix_str = "[RESP]  "; } // Magenta
    else { prefix_str = "[" + level + "] "; }

    std::ostream &out = (level == "ERROR" || level == "FATAL" || level == "WARN") ? std::cerr : std::cout;
    out << color_start << std::string(time_buffer) << " " << prefix_str << message << color_end << std::endl;
    if (!details.empty()) {
        const size_t max_detail_len = 500;
        std::string truncated_details = details.substr(0, max_detail_len) + (details.length() > max_detail_len ? "... (truncated)" : "");
        out << color_start << "  | " << truncated_details << color_end << std::endl;
    }
}

// --- RAII Helper for curl_global_cleanup ---
struct CurlGlobalCleanupGuard {
    ~CurlGlobalCleanupGuard() {
        serverLog("INFO", "Calling curl_global_cleanup().");
        curl_global_cleanup();
    }
    CurlGlobalCleanupGuard(const CurlGlobalCleanupGuard&) = delete;
    CurlGlobalCleanupGuard& operator=(const CurlGlobalCleanupGuard&) = delete;
    CurlGlobalCleanupGuard() = default;
};

// ============================================================================
// == Agent & Tool Setup Functions                                         ==
// ============================================================================

// Creates and configures the NoteMaster Agent
std::unique_ptr<Agent> setupNoteMasterAgent(
    const std::string& apiKey,
    std::vector<std::unique_ptr<Tool>>& managedTools)
{
    serverLog("INFO", "--- Setting up NoteMaster Agent ---");
    // Create API client first because Agent takes it by reference
    auto noteApiClient = std::make_unique<MiniGemini>(apiKey);
    serverLog("INFO", "NoteMaster API client initialized.");

    auto noteAgent = std::make_unique<Agent>(*noteApiClient); // Pass API client reference
    noteAgent->setName("NoteMaster");
    noteAgent->setDescription("Specialized agent for managing notes (in workspace) and calendar events.");
    noteAgent->setIterationCap(5); // Example: Lower iteration cap

    // Updated NoteMaster System Prompt (using new schema)
    noteAgent->setSystemPrompt(R"(SYSTEM PROMPT: NoteMaster Agent

Role: Manage notes within the workspace and calendar events. Execute specific tasks given by the Orchestrator.

Interaction Model:
Respond ONLY with a single JSON object containing:
- thoughts (array of strings): Reasoning about the file/calendar action.
- DIRECTIVE (string): e.g., "FILE_OPERATION", "CALENDAR_UPDATE", "TASK_COMPLETE", "TASK_FAILED".
- state_metadata (object | null): Optional confidence, urgency, completion %. (Usually confidence=1.0 for direct tasks).
- context_assessment (object | null): Optional. Note if required path/details are missing.
- knowledge_query (object | null): Set to null. Do not query memory unless explicitly asked.
- execution (array of objects | null): REQUIRED if executing. Contains ONLY 'file' or 'calendar' tool calls. Format: {step_id, action_type: "TOOL_CALL", tool_name, params}. NULL if task complete/failed definitively.
- execution_mode (string | null): Usually "SEQUENTIAL_STOP_ON_FAIL".
- expected_outcome (string | null): Goal of the file/calendar action.
- is_done (boolean): True ONLY when the specific file/calendar task is fully completed or has definitively failed.
- response (string | null): Confirmation/result message for the Orchestrator (e.g., "Success: File content retrieved.", "Success: Event added.", "Error: File '...' not found."). ONLY populate if is_done is true.

Guidelines:
- Focus: Use 'file' and 'calendar' tools based on the Orchestrator's request. Assume relative paths.
- Execution: Follow instructions precisely. Do NOT ask for clarification; report failure if instructions are unclear/impossible.
- Error Handling: If a tool call fails, set DIRECTIVE to TASK_FAILED, is_done to true, and provide an error message in 'response'.
)");
    serverLog("INFO", "System prompt set for NoteMaster.");

    // Register Tools for NoteMaster (using existing external functions)
    auto fileToolInstance = std::make_unique<Tool>(
        "file",
        "File system operations (read, write, list, info, delete, mkdir). Params: {'action': string, 'path': string (RELATIVE ONLY), 'content': string? for write}",
        fileTool);
    noteAgent->addTool(fileToolInstance.get()); // Add raw pointer to agent
    managedTools.push_back(std::move(fileToolInstance)); // Store owner pointer

    auto calendarToolInstance = std::make_unique<Tool>(
        "calendar",
        "Manages calendar events (add, list). Params: {'action': 'add'|'list', 'date': 'YYYY-MM-DD', 'time': 'HH:MM'?, 'description': string?}",
        calendarTool);
    noteAgent->addTool(calendarToolInstance.get());
    managedTools.push_back(std::move(calendarToolInstance));

    serverLog("INFO", "Tools registered with NoteMaster.");
    serverLog("INFO", "--- NoteMaster Agent Setup Complete ---");
    // Transfer ownership of the API client to the agent if Agent class manages it,
    // otherwise, the API client needs to be managed separately (e.g., in main).
    // Assuming Agent DOES NOT take ownership here. The caller (main) needs to keep noteApiClient alive.
    // If Agent DOES take ownership: noteAgent->setApiClient(std::move(noteApiClient));
    return noteAgent; // Return the unique_ptr
}

// Creates and configures the Orchestrator Agent
std::unique_ptr<Agent> setupOrchestratorAgent(
    const std::string& apiKey,
    Agent* noteMasterAgentPtr, // Pass raw pointer to NoteMaster
    std::vector<std::unique_ptr<Tool>>& managedTools)
{
    serverLog("INFO", "--- Setting up Orchestrator Agent ---");
    // Create API client first
    auto orchestratorApiClient = std::make_unique<MiniGemini>(apiKey);
    orchestratorApiClient->setModel("gemini-1.5-flash-latest"); // Or another capable model
    serverLog("INFO", "Orchestrator API client initialized.");

    auto orchestratorAgent = std::make_unique<Agent>(*orchestratorApiClient); // Pass API client reference
    orchestratorAgent->setName("Orchestrator");
    orchestratorAgent->setDescription("Top-level agent coordinating tasks and delegating to sub-agents like NoteMaster.");
    orchestratorAgent->setIterationCap(15); // Higher cap for coordinator

    // Updated Orchestrator System Prompt (using new schema)
    orchestratorAgent->setSystemPrompt(R"(SYSTEM PROMPT: Orchestrator Agent (Named: Demurge)

Role: Central coordinator. Decompose user goals into plans, manage context, query knowledge, execute local tools, or delegate tasks to specialized agents (e.g., 'NoteMaster').

Interaction Model:
Respond ONLY with a single JSON object containing:
- thoughts (array of strings): Decompose the goal, analyze results, plan next step(s).
- DIRECTIVE (string): Current phase (e.g., "PLANNING", "CONTEXT_ASSESSMENT", "KNOWLEDGE_RETRIEVAL", "DELEGATING_TASK", "TOOL_EXECUTION", "PROCESSING_RESULTS", "RESPONSE_GENERATION", "TASK_COMPLETE", "TASK_FAILED").
- state_metadata (object | null): Confidence, urgency, completion %.
- context_assessment (object | null): Assess required context (keys/info) vs available context. Set 'missing_critical_info' to true if needed before proceeding.
- knowledge_query (object | null): If internal information (from memory/history) is needed FIRST, specify {query_string, target_memory, max_results}. Execution MUST be null if this is present.
- execution (array of objects | null): Actions for THIS agent (bash, web search, etc.) OR delegation calls using 'promptAgent'. Format: {step_id, action_type: "TOOL_CALL", tool_name, params}. NULL if querying knowledge or finishing. Use 'promptAgent' tool here to delegate to sub-agents like NoteMaster.
- execution_mode (string | null): How to run 'execution' array (e.g., "SEQUENTIAL_STOP_ON_FAIL", "SEQUENTIAL_BEST_EFFORT").
- expected_outcome (string | null): Goal of the knowledge query or execution block.
- is_done (boolean): True ONLY when the overall user goal is fully achieved.
- response (string | null): Final response TO THE USER (only populate if is_done is true).

Guidelines:
- Planning: Use 'thoughts' to break down the user's request into logical steps.
- Context: Use 'context_assessment' to check if you have needed info. Use 'knowledge_query' to retrieve info from memory BEFORE deciding on tools/delegation.
- Tools vs Delegation: Use local tools ('time', 'calc', 'web', 'ddg_search', 'file' [if essential & not notes], 'bash' [rarely]) for simple tasks. For note/calendar tasks, DELEGATE using the 'promptAgent' tool within 'execution', targeting 'NoteMaster'. Provide a clear task description in 'promptAgent' params.
- Synthesis: After knowledge query or execution results appear in history, analyze them in your next 'thoughts' to decide the subsequent step.
- Completion: Only set 'is_done' to true and populate 'response' when the *entire* user request is satisfied. Intermediate confirmations/results should be processed internally via 'thoughts'.
- Error Handling: If a local tool, knowledge query, or delegation fails (check history), note it in 'thoughts', set appropriate DIRECTIVE (e.g., ERROR_RECOVERY), and decide whether to retry, use an alternative, or report failure to the user (setting is_done:true, response:error message).
)");
    serverLog("INFO", "System prompt set for Orchestrator.");

    // Register Tools for Orchestrator
    auto timeToolInstance = std::make_unique<Tool>("time", "Get current system date/time. Params: {}", getTime);
    orchestratorAgent->addTool(timeToolInstance.get());
    managedTools.push_back(std::move(timeToolInstance));

    auto calcToolInstance = std::make_unique<Tool>("calc", "Calculate simple expression. Params: {\"expression\": \"string\"}", calculate);
    orchestratorAgent->addTool(calcToolInstance.get());
    managedTools.push_back(std::move(calcToolInstance));



    // Find the file tool added by NoteMaster setup
    Tool* fileToolPtr = nullptr;
    for(const auto& tool : managedTools) {
        if(tool && tool->getName() == "file") {
            fileToolPtr = tool.get();
            break;
        }
    }
    if (fileToolPtr) {
        // Decide if Orchestrator needs direct file access. Usually better to delegate.
        // orchestratorAgent->addTool(fileToolPtr);
        // serverLog("INFO", "Shared 'file' tool potentially available to Orchestrator (use delegation preferably).");
    } else {
        serverLog("WARN", "Could not find 'file' tool instance to potentially share with Orchestrator.");
    }

    auto bashToolInstance = std::make_unique<Tool>("bash", "Execute Bash command. CAUTION ADVISED. Params: {\"command\": \"string\"}", executeBashCommandReal);
    orchestratorAgent->addTool(bashToolInstance.get());
    managedTools.push_back(std::move(bashToolInstance));

    // Register NoteMaster as a sub-agent if provided
    if (noteMasterAgentPtr) {
        orchestratorAgent->addAgent(noteMasterAgentPtr); // Use correct method name
        serverLog("INFO", "Registered 'NoteMaster' with Orchestrator.");
    } else {
        serverLog("WARN", "NoteMaster agent pointer was null, could not register as sub-agent.");
    }

    serverLog("INFO", "--- Orchestrator Agent Setup Complete ---");
    // Again, assuming Agent doesn't take ownership of the API client ptr.
    return orchestratorAgent;
}

// ============================================================================
// == HTTP Server Setup & Run Functions                                      ==
// ============================================================================

// Configures the httplib::Server instance passed by reference
void setupHttpServer(httplib::Server& svr, Agent& agent) { // Takes svr by reference

    // --- Define Endpoints ---

    // CORS Preflight Handler
    svr.Options("/prompt", [](const httplib::Request &, httplib::Response &res) {
        // TODO: Make origin configurable
        // res.set_header("Access-Control-Allow-Origin", "http://localhost:8000");
         res.set_header("Access-Control-Allow-Origin", "https://test.clevo.ddnsgeek.com"); // Example domain
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Accept"); // Added Accept
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
        res.status = 204;
        serverLog("REQUEST", "OPTIONS /prompt (CORS preflight)");
    });

    // Main Prompt Endpoint
    // Capture agent by reference
    svr.Post("/prompt", [&](const httplib::Request &req, httplib::Response &res) {
        // TODO: Make origin configurable
        // res.set_header("Access-Control-Allow-Origin", "http://localhost:8000");
         res.set_header("Access-Control-Allow-Origin", "https://test.clevo.ddnsgeek.com"); // Example domain

        serverLog("REQUEST", "POST /prompt from " + req.remote_addr);

        // 1. Parse Request JSON
        Json::Value requestJson;
        Json::CharReaderBuilder readerBuilder;
        std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
        std::string errors;
        if (!reader->parse(req.body.c_str(), req.body.c_str() + req.body.length(), &requestJson, &errors)) {
            serverLog("ERROR", "Failed to parse request JSON", errors);
            res.status = 400; // Bad Request
            // Use .c_str() for valueToQuotedString
            res.set_content("{\"error\": \"Invalid JSON format\", \"details\": " + Json::valueToQuotedString(errors.c_str()) + "}", "application/json");
            return;
        }

        // 2. Extract User Prompt
        if (!requestJson.isMember("prompt") || !requestJson["prompt"].isString()) {
            serverLog("ERROR", "Missing or invalid 'prompt' field");
            res.status = 400; // Bad Request
            res.set_content("{\"error\": \"'prompt' field (string) is required\"}", "application/json");
            return;
        }
        std::string userPrompt = requestJson["prompt"].asString();
        serverLog("INFO", "User prompt received", userPrompt.substr(0, 100) + (userPrompt.length() > 100 ? "..." : ""));

        // 3. Interact with the Agent (using the captured reference)
        std::string agentFinalResponse;
        try {
            agentFinalResponse = agent.prompt(userPrompt); // Use correct method name
                                                           //
            serverLog("DEBUG", "Agent final response generated", agentFinalResponse.substr(0, 200) + (agentFinalResponse.length() > 200 ? "..." : ""));
        } catch (const ApiError& e) {
            serverLog("ERROR", "Agent API error processing prompt", e.what());
            res.status = 500; // Internal Server Error
            res.set_content("{\"error\": \"Agent API interaction failed\", \"details\": " + Json::valueToQuotedString(e.what()) + "}", "application/json");
            return;
        } catch (const std::exception &e) {
            serverLog("ERROR", "Agent processing error", e.what());
            res.status = 500; // Internal Server Error
            res.set_content("{\"error\": \"Agent interaction failed\", \"details\": " + Json::valueToQuotedString(e.what()) + "}", "application/json");
            return;
        } catch (...) {
            serverLog("ERROR", "Unknown agent processing error");
            res.status = 500; // Internal Server Error
            res.set_content("{\"error\": \"Unknown agent interaction failed\"}", "application/json");
            return;
        }

        // 4. Format and Send Response
        Json::Value responseJson;
        responseJson["response"] = agentFinalResponse; // Key expected by frontend
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = ""; // Compact JSON for API
        std::string responseBody = Json::writeString(writerBuilder, responseJson);

        res.status = 200; // OK
        res.set_content(responseBody, "application/json");
        serverLog("RESPONSE", "Sent 200 OK for /prompt");
    });

    // Health Check Endpoint
    svr.Get("/health", [](const httplib::Request &, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*"); // Allow from anywhere
        res.status = 200;
        res.set_content("{\"status\": \"OK\"}", "application/json");
        serverLog("REQUEST", "GET /health");
    });

    // No return needed as svr is modified by reference
}

// Starts the HTTP server listening loop
bool startHttpServer(httplib::Server& svr, const std::string& host, int port) {
    serverLog("INFO", "Starting server on " + host + ":" + std::to_string(port));
    if (!svr.listen(host.c_str(), port)) {
        serverLog("FATAL", "Failed to start server on " + host + ":" + std::to_string(port));
        return false;
    }
    // svr.listen blocks until stopped
    serverLog("INFO", "Server has stopped listening.");
    return true;
}

// ============================================================================
// == Main Function                                                          ==
// ============================================================================

int main() {
    // --- Global Initialization ---
    CURLcode curl_global_res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (curl_global_res != CURLE_OK) {
        serverLog("FATAL", "Failed to initialize libcurl globally: " + std::string(curl_easy_strerror(curl_global_res)));
        return 1;
    }
    serverLog("INFO", "libcurl initialized.");
    CurlGlobalCleanupGuard curl_guard; // RAII cleanup

    // --- Configuration ---
    const char *apiKeyEnv = std::getenv("GEMINI_API_KEY");
    if (!apiKeyEnv || std::string(apiKeyEnv).empty()) {
        serverLog("FATAL", "GEMINI_API_KEY environment variable not set or empty.");
        return 1;
    }
    std::string apiKey(apiKeyEnv);
    serverLog("INFO", "API Key loaded from environment variable.");
    const std::string host = "0.0.0.0"; // Listen on all interfaces
    const int port = 7777;

    // --- Agent and Tool Storage ---
    // unique_ptrs manage the lifetime of agents and owned tools
    std::vector<std::unique_ptr<Tool>> managedTools;
    // API Clients need to outlive the Agents that use them by reference
    std::unique_ptr<MiniGemini> noteApiClientPtr;
    std::unique_ptr<MiniGemini> orchestratorApiClientPtr;
    std::unique_ptr<Agent> noteMasterAgent;
    std::unique_ptr<Agent> orchestratorAgent;
    // Create server object here, setup happens later
    httplib::Server svr;

    try {
        // --- Setup Agents & Tools ---
        // Create API clients first (will be passed by reference to agents)
        noteApiClientPtr = std::make_unique<MiniGemini>(apiKey);
        orchestratorApiClientPtr = std::make_unique<MiniGemini>(apiKey);

        // Now setup agents, passing the API client references
        noteMasterAgent = setupNoteMasterAgent(apiKey, managedTools); // apiKey is redundant if client passed? Maybe refactor setup funcs
        orchestratorAgent = setupOrchestratorAgent(apiKey, noteMasterAgent.get(), managedTools);

        if (!orchestratorAgent) {
             throw std::runtime_error("Failed to initialize Orchestrator Agent.");
        }
        serverLog("INFO", "All agents initialized successfully.");

        // --- Setup HTTP Server ---
        // Pass orchestratorAgent by reference
        setupHttpServer(svr, *orchestratorAgent);
        serverLog("INFO", "HTTP server configured.");

        // --- Start Server ---
        // This call will block until the server is stopped (e.g., Ctrl+C)
        if (!startHttpServer(svr, host, port)) {
            return 1; // Exit if server fails to start
        }

    } catch (const ApiError &e) {
        serverLog("FATAL", "API Error during initialization or runtime", e.what());
        // unique_ptrs will handle cleanup on exit
        return 1;
    } catch (const std::exception &e) {
        serverLog("FATAL", "Standard Exception during initialization or runtime", e.what());
         // unique_ptrs will handle cleanup on exit
        return 1;
    } catch (...) {
        serverLog("FATAL", "Unknown error occurred.");
         // unique_ptrs will handle cleanup on exit
        return 1;
    }

    // Server runs until stopped (e.g., Ctrl+C in startHttpServer -> svr.listen)
    serverLog("INFO", "Server process ended.");
    // Resources (agents via unique_ptr; tools via unique_ptr in vector; curl via RAII guard) cleaned up automatically
    return 0;
}


```

### File: server/vendor/httplib/httplib.h
```cpp
//
//  httplib.h
//
//  Copyright (c) 2025 Yuji Hirose. All rights reserved.
//  MIT License
//

#ifndef CPPHTTPLIB_HTTPLIB_H
#define CPPHTTPLIB_HTTPLIB_H

#define CPPHTTPLIB_VERSION "0.20.0"

/*
 * Configuration
 */

#ifndef CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND
#define CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND 5
#endif

#ifndef CPPHTTPLIB_KEEPALIVE_TIMEOUT_CHECK_INTERVAL_USECOND
#define CPPHTTPLIB_KEEPALIVE_TIMEOUT_CHECK_INTERVAL_USECOND 10000
#endif

#ifndef CPPHTTPLIB_KEEPALIVE_MAX_COUNT
#define CPPHTTPLIB_KEEPALIVE_MAX_COUNT 100
#endif

#ifndef CPPHTTPLIB_CONNECTION_TIMEOUT_SECOND
#define CPPHTTPLIB_CONNECTION_TIMEOUT_SECOND 300
#endif

#ifndef CPPHTTPLIB_CONNECTION_TIMEOUT_USECOND
#define CPPHTTPLIB_CONNECTION_TIMEOUT_USECOND 0
#endif

#ifndef CPPHTTPLIB_SERVER_READ_TIMEOUT_SECOND
#define CPPHTTPLIB_SERVER_READ_TIMEOUT_SECOND 5
#endif

#ifndef CPPHTTPLIB_SERVER_READ_TIMEOUT_USECOND
#define CPPHTTPLIB_SERVER_READ_TIMEOUT_USECOND 0
#endif

#ifndef CPPHTTPLIB_SERVER_WRITE_TIMEOUT_SECOND
#define CPPHTTPLIB_SERVER_WRITE_TIMEOUT_SECOND 5
#endif

#ifndef CPPHTTPLIB_SERVER_WRITE_TIMEOUT_USECOND
#define CPPHTTPLIB_SERVER_WRITE_TIMEOUT_USECOND 0
#endif

#ifndef CPPHTTPLIB_CLIENT_READ_TIMEOUT_SECOND
#define CPPHTTPLIB_CLIENT_READ_TIMEOUT_SECOND 300
#endif

#ifndef CPPHTTPLIB_CLIENT_READ_TIMEOUT_USECOND
#define CPPHTTPLIB_CLIENT_READ_TIMEOUT_USECOND 0
#endif

#ifndef CPPHTTPLIB_CLIENT_WRITE_TIMEOUT_SECOND
#define CPPHTTPLIB_CLIENT_WRITE_TIMEOUT_SECOND 5
#endif

#ifndef CPPHTTPLIB_CLIENT_WRITE_TIMEOUT_USECOND
#define CPPHTTPLIB_CLIENT_WRITE_TIMEOUT_USECOND 0
#endif

#ifndef CPPHTTPLIB_CLIENT_MAX_TIMEOUT_MSECOND
#define CPPHTTPLIB_CLIENT_MAX_TIMEOUT_MSECOND 0
#endif

#ifndef CPPHTTPLIB_IDLE_INTERVAL_SECOND
#define CPPHTTPLIB_IDLE_INTERVAL_SECOND 0
#endif

#ifndef CPPHTTPLIB_IDLE_INTERVAL_USECOND
#ifdef _WIN32
#define CPPHTTPLIB_IDLE_INTERVAL_USECOND 10000
#else
#define CPPHTTPLIB_IDLE_INTERVAL_USECOND 0
#endif
#endif

#ifndef CPPHTTPLIB_REQUEST_URI_MAX_LENGTH
#define CPPHTTPLIB_REQUEST_URI_MAX_LENGTH 8192
#endif

#ifndef CPPHTTPLIB_HEADER_MAX_LENGTH
#define CPPHTTPLIB_HEADER_MAX_LENGTH 8192
#endif

#ifndef CPPHTTPLIB_REDIRECT_MAX_COUNT
#define CPPHTTPLIB_REDIRECT_MAX_COUNT 20
#endif

#ifndef CPPHTTPLIB_MULTIPART_FORM_DATA_FILE_MAX_COUNT
#define CPPHTTPLIB_MULTIPART_FORM_DATA_FILE_MAX_COUNT 1024
#endif

#ifndef CPPHTTPLIB_PAYLOAD_MAX_LENGTH
#define CPPHTTPLIB_PAYLOAD_MAX_LENGTH ((std::numeric_limits<size_t>::max)())
#endif

#ifndef CPPHTTPLIB_FORM_URL_ENCODED_PAYLOAD_MAX_LENGTH
#define CPPHTTPLIB_FORM_URL_ENCODED_PAYLOAD_MAX_LENGTH 8192
#endif

#ifndef CPPHTTPLIB_RANGE_MAX_COUNT
#define CPPHTTPLIB_RANGE_MAX_COUNT 1024
#endif

#ifndef CPPHTTPLIB_TCP_NODELAY
#define CPPHTTPLIB_TCP_NODELAY false
#endif

#ifndef CPPHTTPLIB_IPV6_V6ONLY
#define CPPHTTPLIB_IPV6_V6ONLY false
#endif

#ifndef CPPHTTPLIB_RECV_BUFSIZ
#define CPPHTTPLIB_RECV_BUFSIZ size_t(16384u)
#endif

#ifndef CPPHTTPLIB_COMPRESSION_BUFSIZ
#define CPPHTTPLIB_COMPRESSION_BUFSIZ size_t(16384u)
#endif

#ifndef CPPHTTPLIB_THREAD_POOL_COUNT
#define CPPHTTPLIB_THREAD_POOL_COUNT                                           \
  ((std::max)(8u, std::thread::hardware_concurrency() > 0                      \
                      ? std::thread::hardware_concurrency() - 1                \
                      : 0))
#endif

#ifndef CPPHTTPLIB_RECV_FLAGS
#define CPPHTTPLIB_RECV_FLAGS 0
#endif

#ifndef CPPHTTPLIB_SEND_FLAGS
#define CPPHTTPLIB_SEND_FLAGS 0
#endif

#ifndef CPPHTTPLIB_LISTEN_BACKLOG
#define CPPHTTPLIB_LISTEN_BACKLOG 5
#endif

/*
 * Headers
 */

#ifdef _WIN32
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif //_CRT_SECURE_NO_WARNINGS

#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif //_CRT_NONSTDC_NO_DEPRECATE

#if defined(_MSC_VER)
#if _MSC_VER < 1900
#error Sorry, Visual Studio versions prior to 2015 are not supported
#endif

#pragma comment(lib, "ws2_32.lib")

#ifdef _WIN64
using ssize_t = __int64;
#else
using ssize_t = long;
#endif
#endif // _MSC_VER

#ifndef S_ISREG
#define S_ISREG(m) (((m) & S_IFREG) == S_IFREG)
#endif // S_ISREG

#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFDIR) == S_IFDIR)
#endif // S_ISDIR

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// afunix.h uses types declared in winsock2.h, so has to be included after it.
#include <afunix.h>

#ifndef WSA_FLAG_NO_HANDLE_INHERIT
#define WSA_FLAG_NO_HANDLE_INHERIT 0x80
#endif

using nfds_t = unsigned long;
using socket_t = SOCKET;
using socklen_t = int;

#else // not _WIN32

#include <arpa/inet.h>
#if !defined(_AIX) && !defined(__MVS__)
#include <ifaddrs.h>
#endif
#ifdef __MVS__
#include <strings.h>
#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif
#endif
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#ifdef __linux__
#include <resolv.h>
#endif
#include <csignal>
#include <netinet/tcp.h>
#include <poll.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using socket_t = int;
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#endif //_WIN32

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cctype>
#include <climits>
#include <condition_variable>
#include <cstring>
#include <errno.h>
#include <exception>
#include <fcntl.h>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
#ifdef _WIN32
#include <wincrypt.h>

// these are defined in wincrypt.h and it breaks compilation if BoringSSL is
// used
#undef X509_NAME
#undef X509_CERT_PAIR
#undef X509_EXTENSIONS
#undef PKCS7_SIGNER_INFO

#ifdef _MSC_VER
#pragma comment(lib, "crypt32.lib")
#endif
#elif defined(CPPHTTPLIB_USE_CERTS_FROM_MACOSX_KEYCHAIN) && defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_OSX
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#endif // TARGET_OS_OSX
#endif // _WIN32

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#if defined(_WIN32) && defined(OPENSSL_USE_APPLINK)
#include <openssl/applink.c>
#endif

#include <iostream>
#include <sstream>

#if defined(OPENSSL_IS_BORINGSSL) || defined(LIBRESSL_VERSION_NUMBER)
#if OPENSSL_VERSION_NUMBER < 0x1010107f
#error Please use OpenSSL or a current version of BoringSSL
#endif
#define SSL_get1_peer_certificate SSL_get_peer_certificate
#elif OPENSSL_VERSION_NUMBER < 0x30000000L
#error Sorry, OpenSSL versions prior to 3.0.0 are not supported
#endif

#endif

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
#include <zlib.h>
#endif

#ifdef CPPHTTPLIB_BROTLI_SUPPORT
#include <brotli/decode.h>
#include <brotli/encode.h>
#endif

#ifdef CPPHTTPLIB_ZSTD_SUPPORT
#include <zstd.h>
#endif

/*
 * Declaration
 */
namespace httplib {

namespace detail {

/*
 * Backport std::make_unique from C++14.
 *
 * NOTE: This code came up with the following stackoverflow post:
 * https://stackoverflow.com/questions/10149840/c-arrays-and-make-unique
 *
 */

template <class T, class... Args>
typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type
make_unique(Args &&...args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class T>
typename std::enable_if<std::is_array<T>::value, std::unique_ptr<T>>::type
make_unique(std::size_t n) {
  typedef typename std::remove_extent<T>::type RT;
  return std::unique_ptr<T>(new RT[n]);
}

namespace case_ignore {

inline unsigned char to_lower(int c) {
  const static unsigned char table[256] = {
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
      15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
      30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
      45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
      60,  61,  62,  63,  64,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106,
      107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
      122, 91,  92,  93,  94,  95,  96,  97,  98,  99,  100, 101, 102, 103, 104,
      105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
      120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
      135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
      150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
      165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
      180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 224, 225, 226,
      227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241,
      242, 243, 244, 245, 246, 215, 248, 249, 250, 251, 252, 253, 254, 223, 224,
      225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
      240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,
      255,
  };
  return table[(unsigned char)(char)c];
}

inline bool equal(const std::string &a, const std::string &b) {
  return a.size() == b.size() &&
         std::equal(a.begin(), a.end(), b.begin(), [](char ca, char cb) {
           return to_lower(ca) == to_lower(cb);
         });
}

struct equal_to {
  bool operator()(const std::string &a, const std::string &b) const {
    return equal(a, b);
  }
};

struct hash {
  size_t operator()(const std::string &key) const {
    return hash_core(key.data(), key.size(), 0);
  }

  size_t hash_core(const char *s, size_t l, size_t h) const {
    return (l == 0) ? h
                    : hash_core(s + 1, l - 1,
                                // Unsets the 6 high bits of h, therefore no
                                // overflow happens
                                (((std::numeric_limits<size_t>::max)() >> 6) &
                                 h * 33) ^
                                    static_cast<unsigned char>(to_lower(*s)));
  }
};

} // namespace case_ignore

// This is based on
// "http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4189".

struct scope_exit {
  explicit scope_exit(std::function<void(void)> &&f)
      : exit_function(std::move(f)), execute_on_destruction{true} {}

  scope_exit(scope_exit &&rhs) noexcept
      : exit_function(std::move(rhs.exit_function)),
        execute_on_destruction{rhs.execute_on_destruction} {
    rhs.release();
  }

  ~scope_exit() {
    if (execute_on_destruction) { this->exit_function(); }
  }

  void release() { this->execute_on_destruction = false; }

private:
  scope_exit(const scope_exit &) = delete;
  void operator=(const scope_exit &) = delete;
  scope_exit &operator=(scope_exit &&) = delete;

  std::function<void(void)> exit_function;
  bool execute_on_destruction;
};

} // namespace detail

enum SSLVerifierResponse {
  // no decision has been made, use the built-in certificate verifier
  NoDecisionMade,
  // connection certificate is verified and accepted
  CertificateAccepted,
  // connection certificate was processed but is rejected
  CertificateRejected
};

enum StatusCode {
  // Information responses
  Continue_100 = 100,
  SwitchingProtocol_101 = 101,
  Processing_102 = 102,
  EarlyHints_103 = 103,

  // Successful responses
  OK_200 = 200,
  Created_201 = 201,
  Accepted_202 = 202,
  NonAuthoritativeInformation_203 = 203,
  NoContent_204 = 204,
  ResetContent_205 = 205,
  PartialContent_206 = 206,
  MultiStatus_207 = 207,
  AlreadyReported_208 = 208,
  IMUsed_226 = 226,

  // Redirection messages
  MultipleChoices_300 = 300,
  MovedPermanently_301 = 301,
  Found_302 = 302,
  SeeOther_303 = 303,
  NotModified_304 = 304,
  UseProxy_305 = 305,
  unused_306 = 306,
  TemporaryRedirect_307 = 307,
  PermanentRedirect_308 = 308,

  // Client error responses
  BadRequest_400 = 400,
  Unauthorized_401 = 401,
  PaymentRequired_402 = 402,
  Forbidden_403 = 403,
  NotFound_404 = 404,
  MethodNotAllowed_405 = 405,
  NotAcceptable_406 = 406,
  ProxyAuthenticationRequired_407 = 407,
  RequestTimeout_408 = 408,
  Conflict_409 = 409,
  Gone_410 = 410,
  LengthRequired_411 = 411,
  PreconditionFailed_412 = 412,
  PayloadTooLarge_413 = 413,
  UriTooLong_414 = 414,
  UnsupportedMediaType_415 = 415,
  RangeNotSatisfiable_416 = 416,
  ExpectationFailed_417 = 417,
  ImATeapot_418 = 418,
  MisdirectedRequest_421 = 421,
  UnprocessableContent_422 = 422,
  Locked_423 = 423,
  FailedDependency_424 = 424,
  TooEarly_425 = 425,
  UpgradeRequired_426 = 426,
  PreconditionRequired_428 = 428,
  TooManyRequests_429 = 429,
  RequestHeaderFieldsTooLarge_431 = 431,
  UnavailableForLegalReasons_451 = 451,

  // Server error responses
  InternalServerError_500 = 500,
  NotImplemented_501 = 501,
  BadGateway_502 = 502,
  ServiceUnavailable_503 = 503,
  GatewayTimeout_504 = 504,
  HttpVersionNotSupported_505 = 505,
  VariantAlsoNegotiates_506 = 506,
  InsufficientStorage_507 = 507,
  LoopDetected_508 = 508,
  NotExtended_510 = 510,
  NetworkAuthenticationRequired_511 = 511,
};

using Headers =
    std::unordered_multimap<std::string, std::string, detail::case_ignore::hash,
                            detail::case_ignore::equal_to>;

using Params = std::multimap<std::string, std::string>;
using Match = std::smatch;

using Progress = std::function<bool(uint64_t current, uint64_t total)>;

struct Response;
using ResponseHandler = std::function<bool(const Response &response)>;

struct MultipartFormData {
  std::string name;
  std::string content;
  std::string filename;
  std::string content_type;
};
using MultipartFormDataItems = std::vector<MultipartFormData>;
using MultipartFormDataMap = std::multimap<std::string, MultipartFormData>;

class DataSink {
public:
  DataSink() : os(&sb_), sb_(*this) {}

  DataSink(const DataSink &) = delete;
  DataSink &operator=(const DataSink &) = delete;
  DataSink(DataSink &&) = delete;
  DataSink &operator=(DataSink &&) = delete;

  std::function<bool(const char *data, size_t data_len)> write;
  std::function<bool()> is_writable;
  std::function<void()> done;
  std::function<void(const Headers &trailer)> done_with_trailer;
  std::ostream os;

private:
  class data_sink_streambuf final : public std::streambuf {
  public:
    explicit data_sink_streambuf(DataSink &sink) : sink_(sink) {}

  protected:
    std::streamsize xsputn(const char *s, std::streamsize n) override {
      sink_.write(s, static_cast<size_t>(n));
      return n;
    }

  private:
    DataSink &sink_;
  };

  data_sink_streambuf sb_;
};

using ContentProvider =
    std::function<bool(size_t offset, size_t length, DataSink &sink)>;

using ContentProviderWithoutLength =
    std::function<bool(size_t offset, DataSink &sink)>;

using ContentProviderResourceReleaser = std::function<void(bool success)>;

struct MultipartFormDataProvider {
  std::string name;
  ContentProviderWithoutLength provider;
  std::string filename;
  std::string content_type;
};
using MultipartFormDataProviderItems = std::vector<MultipartFormDataProvider>;

using ContentReceiverWithProgress =
    std::function<bool(const char *data, size_t data_length, uint64_t offset,
                       uint64_t total_length)>;

using ContentReceiver =
    std::function<bool(const char *data, size_t data_length)>;

using MultipartContentHeader =
    std::function<bool(const MultipartFormData &file)>;

class ContentReader {
public:
  using Reader = std::function<bool(ContentReceiver receiver)>;
  using MultipartReader = std::function<bool(MultipartContentHeader header,
                                             ContentReceiver receiver)>;

  ContentReader(Reader reader, MultipartReader multipart_reader)
      : reader_(std::move(reader)),
        multipart_reader_(std::move(multipart_reader)) {}

  bool operator()(MultipartContentHeader header,
                  ContentReceiver receiver) const {
    return multipart_reader_(std::move(header), std::move(receiver));
  }

  bool operator()(ContentReceiver receiver) const {
    return reader_(std::move(receiver));
  }

  Reader reader_;
  MultipartReader multipart_reader_;
};

using Range = std::pair<ssize_t, ssize_t>;
using Ranges = std::vector<Range>;

struct Request {
  std::string method;
  std::string path;
  Params params;
  Headers headers;
  std::string body;

  std::string remote_addr;
  int remote_port = -1;
  std::string local_addr;
  int local_port = -1;

  // for server
  std::string version;
  std::string target;
  MultipartFormDataMap files;
  Ranges ranges;
  Match matches;
  std::unordered_map<std::string, std::string> path_params;
  std::function<bool()> is_connection_closed = []() { return true; };

  // for client
  ResponseHandler response_handler;
  ContentReceiverWithProgress content_receiver;
  Progress progress;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  const SSL *ssl = nullptr;
#endif

  bool has_header(const std::string &key) const;
  std::string get_header_value(const std::string &key, const char *def = "",
                               size_t id = 0) const;
  uint64_t get_header_value_u64(const std::string &key, uint64_t def = 0,
                                size_t id = 0) const;
  size_t get_header_value_count(const std::string &key) const;
  void set_header(const std::string &key, const std::string &val);

  bool has_param(const std::string &key) const;
  std::string get_param_value(const std::string &key, size_t id = 0) const;
  size_t get_param_value_count(const std::string &key) const;

  bool is_multipart_form_data() const;

  bool has_file(const std::string &key) const;
  MultipartFormData get_file_value(const std::string &key) const;
  std::vector<MultipartFormData> get_file_values(const std::string &key) const;

  // private members...
  size_t redirect_count_ = CPPHTTPLIB_REDIRECT_MAX_COUNT;
  size_t content_length_ = 0;
  ContentProvider content_provider_;
  bool is_chunked_content_provider_ = false;
  size_t authorization_count_ = 0;
  std::chrono::time_point<std::chrono::steady_clock> start_time_ =
      (std::chrono::steady_clock::time_point::min)();
};

struct Response {
  std::string version;
  int status = -1;
  std::string reason;
  Headers headers;
  std::string body;
  std::string location; // Redirect location

  bool has_header(const std::string &key) const;
  std::string get_header_value(const std::string &key, const char *def = "",
                               size_t id = 0) const;
  uint64_t get_header_value_u64(const std::string &key, uint64_t def = 0,
                                size_t id = 0) const;
  size_t get_header_value_count(const std::string &key) const;
  void set_header(const std::string &key, const std::string &val);

  void set_redirect(const std::string &url, int status = StatusCode::Found_302);
  void set_content(const char *s, size_t n, const std::string &content_type);
  void set_content(const std::string &s, const std::string &content_type);
  void set_content(std::string &&s, const std::string &content_type);

  void set_content_provider(
      size_t length, const std::string &content_type, ContentProvider provider,
      ContentProviderResourceReleaser resource_releaser = nullptr);

  void set_content_provider(
      const std::string &content_type, ContentProviderWithoutLength provider,
      ContentProviderResourceReleaser resource_releaser = nullptr);

  void set_chunked_content_provider(
      const std::string &content_type, ContentProviderWithoutLength provider,
      ContentProviderResourceReleaser resource_releaser = nullptr);

  void set_file_content(const std::string &path,
                        const std::string &content_type);
  void set_file_content(const std::string &path);

  Response() = default;
  Response(const Response &) = default;
  Response &operator=(const Response &) = default;
  Response(Response &&) = default;
  Response &operator=(Response &&) = default;
  ~Response() {
    if (content_provider_resource_releaser_) {
      content_provider_resource_releaser_(content_provider_success_);
    }
  }

  // private members...
  size_t content_length_ = 0;
  ContentProvider content_provider_;
  ContentProviderResourceReleaser content_provider_resource_releaser_;
  bool is_chunked_content_provider_ = false;
  bool content_provider_success_ = false;
  std::string file_content_path_;
  std::string file_content_content_type_;
};

class Stream {
public:
  virtual ~Stream() = default;

  virtual bool is_readable() const = 0;
  virtual bool wait_readable() const = 0;
  virtual bool wait_writable() const = 0;

  virtual ssize_t read(char *ptr, size_t size) = 0;
  virtual ssize_t write(const char *ptr, size_t size) = 0;
  virtual void get_remote_ip_and_port(std::string &ip, int &port) const = 0;
  virtual void get_local_ip_and_port(std::string &ip, int &port) const = 0;
  virtual socket_t socket() const = 0;

  virtual time_t duration() const = 0;

  ssize_t write(const char *ptr);
  ssize_t write(const std::string &s);
};

class TaskQueue {
public:
  TaskQueue() = default;
  virtual ~TaskQueue() = default;

  virtual bool enqueue(std::function<void()> fn) = 0;
  virtual void shutdown() = 0;

  virtual void on_idle() {}
};

class ThreadPool final : public TaskQueue {
public:
  explicit ThreadPool(size_t n, size_t mqr = 0)
      : shutdown_(false), max_queued_requests_(mqr) {
    while (n) {
      threads_.emplace_back(worker(*this));
      n--;
    }
  }

  ThreadPool(const ThreadPool &) = delete;
  ~ThreadPool() override = default;

  bool enqueue(std::function<void()> fn) override {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (max_queued_requests_ > 0 && jobs_.size() >= max_queued_requests_) {
        return false;
      }
      jobs_.push_back(std::move(fn));
    }

    cond_.notify_one();
    return true;
  }

  void shutdown() override {
    // Stop all worker threads...
    {
      std::unique_lock<std::mutex> lock(mutex_);
      shutdown_ = true;
    }

    cond_.notify_all();

    // Join...
    for (auto &t : threads_) {
      t.join();
    }
  }

private:
  struct worker {
    explicit worker(ThreadPool &pool) : pool_(pool) {}

    void operator()() {
      for (;;) {
        std::function<void()> fn;
        {
          std::unique_lock<std::mutex> lock(pool_.mutex_);

          pool_.cond_.wait(
              lock, [&] { return !pool_.jobs_.empty() || pool_.shutdown_; });

          if (pool_.shutdown_ && pool_.jobs_.empty()) { break; }

          fn = pool_.jobs_.front();
          pool_.jobs_.pop_front();
        }

        assert(true == static_cast<bool>(fn));
        fn();
      }

#if defined(CPPHTTPLIB_OPENSSL_SUPPORT) && !defined(OPENSSL_IS_BORINGSSL) &&   \
    !defined(LIBRESSL_VERSION_NUMBER)
      OPENSSL_thread_stop();
#endif
    }

    ThreadPool &pool_;
  };
  friend struct worker;

  std::vector<std::thread> threads_;
  std::list<std::function<void()>> jobs_;

  bool shutdown_;
  size_t max_queued_requests_ = 0;

  std::condition_variable cond_;
  std::mutex mutex_;
};

using Logger = std::function<void(const Request &, const Response &)>;

using SocketOptions = std::function<void(socket_t sock)>;

namespace detail {

bool set_socket_opt_impl(socket_t sock, int level, int optname,
                         const void *optval, socklen_t optlen);
bool set_socket_opt(socket_t sock, int level, int optname, int opt);
bool set_socket_opt_time(socket_t sock, int level, int optname, time_t sec,
                         time_t usec);

} // namespace detail

void default_socket_options(socket_t sock);

const char *status_message(int status);

std::string get_bearer_token_auth(const Request &req);

namespace detail {

class MatcherBase {
public:
  virtual ~MatcherBase() = default;

  // Match request path and populate its matches and
  virtual bool match(Request &request) const = 0;
};

/**
 * Captures parameters in request path and stores them in Request::path_params
 *
 * Capture name is a substring of a pattern from : to /.
 * The rest of the pattern is matched against the request path directly
 * Parameters are captured starting from the next character after
 * the end of the last matched static pattern fragment until the next /.
 *
 * Example pattern:
 * "/path/fragments/:capture/more/fragments/:second_capture"
 * Static fragments:
 * "/path/fragments/", "more/fragments/"
 *
 * Given the following request path:
 * "/path/fragments/:1/more/fragments/:2"
 * the resulting capture will be
 * {{"capture", "1"}, {"second_capture", "2"}}
 */
class PathParamsMatcher final : public MatcherBase {
public:
  PathParamsMatcher(const std::string &pattern);

  bool match(Request &request) const override;

private:
  // Treat segment separators as the end of path parameter capture
  // Does not need to handle query parameters as they are parsed before path
  // matching
  static constexpr char separator = '/';

  // Contains static path fragments to match against, excluding the '/' after
  // path params
  // Fragments are separated by path params
  std::vector<std::string> static_fragments_;
  // Stores the names of the path parameters to be used as keys in the
  // Request::path_params map
  std::vector<std::string> param_names_;
};

/**
 * Performs std::regex_match on request path
 * and stores the result in Request::matches
 *
 * Note that regex match is performed directly on the whole request.
 * This means that wildcard patterns may match multiple path segments with /:
 * "/begin/(.*)/end" will match both "/begin/middle/end" and "/begin/1/2/end".
 */
class RegexMatcher final : public MatcherBase {
public:
  RegexMatcher(const std::string &pattern) : regex_(pattern) {}

  bool match(Request &request) const override;

private:
  std::regex regex_;
};

ssize_t write_headers(Stream &strm, const Headers &headers);

} // namespace detail

class Server {
public:
  using Handler = std::function<void(const Request &, Response &)>;

  using ExceptionHandler =
      std::function<void(const Request &, Response &, std::exception_ptr ep)>;

  enum class HandlerResponse {
    Handled,
    Unhandled,
  };
  using HandlerWithResponse =
      std::function<HandlerResponse(const Request &, Response &)>;

  using HandlerWithContentReader = std::function<void(
      const Request &, Response &, const ContentReader &content_reader)>;

  using Expect100ContinueHandler =
      std::function<int(const Request &, Response &)>;

  Server();

  virtual ~Server();

  virtual bool is_valid() const;

  Server &Get(const std::string &pattern, Handler handler);
  Server &Post(const std::string &pattern, Handler handler);
  Server &Post(const std::string &pattern, HandlerWithContentReader handler);
  Server &Put(const std::string &pattern, Handler handler);
  Server &Put(const std::string &pattern, HandlerWithContentReader handler);
  Server &Patch(const std::string &pattern, Handler handler);
  Server &Patch(const std::string &pattern, HandlerWithContentReader handler);
  Server &Delete(const std::string &pattern, Handler handler);
  Server &Delete(const std::string &pattern, HandlerWithContentReader handler);
  Server &Options(const std::string &pattern, Handler handler);

  bool set_base_dir(const std::string &dir,
                    const std::string &mount_point = std::string());
  bool set_mount_point(const std::string &mount_point, const std::string &dir,
                       Headers headers = Headers());
  bool remove_mount_point(const std::string &mount_point);
  Server &set_file_extension_and_mimetype_mapping(const std::string &ext,
                                                  const std::string &mime);
  Server &set_default_file_mimetype(const std::string &mime);
  Server &set_file_request_handler(Handler handler);

  template <class ErrorHandlerFunc>
  Server &set_error_handler(ErrorHandlerFunc &&handler) {
    return set_error_handler_core(
        std::forward<ErrorHandlerFunc>(handler),
        std::is_convertible<ErrorHandlerFunc, HandlerWithResponse>{});
  }

  Server &set_exception_handler(ExceptionHandler handler);
  Server &set_pre_routing_handler(HandlerWithResponse handler);
  Server &set_post_routing_handler(Handler handler);

  Server &set_expect_100_continue_handler(Expect100ContinueHandler handler);
  Server &set_logger(Logger logger);

  Server &set_address_family(int family);
  Server &set_tcp_nodelay(bool on);
  Server &set_ipv6_v6only(bool on);
  Server &set_socket_options(SocketOptions socket_options);

  Server &set_default_headers(Headers headers);
  Server &
  set_header_writer(std::function<ssize_t(Stream &, Headers &)> const &writer);

  Server &set_keep_alive_max_count(size_t count);
  Server &set_keep_alive_timeout(time_t sec);

  Server &set_read_timeout(time_t sec, time_t usec = 0);
  template <class Rep, class Period>
  Server &set_read_timeout(const std::chrono::duration<Rep, Period> &duration);

  Server &set_write_timeout(time_t sec, time_t usec = 0);
  template <class Rep, class Period>
  Server &set_write_timeout(const std::chrono::duration<Rep, Period> &duration);

  Server &set_idle_interval(time_t sec, time_t usec = 0);
  template <class Rep, class Period>
  Server &set_idle_interval(const std::chrono::duration<Rep, Period> &duration);

  Server &set_payload_max_length(size_t length);

  bool bind_to_port(const std::string &host, int port, int socket_flags = 0);
  int bind_to_any_port(const std::string &host, int socket_flags = 0);
  bool listen_after_bind();

  bool listen(const std::string &host, int port, int socket_flags = 0);

  bool is_running() const;
  void wait_until_ready() const;
  void stop();
  void decommission();

  std::function<TaskQueue *(void)> new_task_queue;

protected:
  bool process_request(Stream &strm, const std::string &remote_addr,
                       int remote_port, const std::string &local_addr,
                       int local_port, bool close_connection,
                       bool &connection_closed,
                       const std::function<void(Request &)> &setup_request);

  std::atomic<socket_t> svr_sock_{INVALID_SOCKET};
  size_t keep_alive_max_count_ = CPPHTTPLIB_KEEPALIVE_MAX_COUNT;
  time_t keep_alive_timeout_sec_ = CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND;
  time_t read_timeout_sec_ = CPPHTTPLIB_SERVER_READ_TIMEOUT_SECOND;
  time_t read_timeout_usec_ = CPPHTTPLIB_SERVER_READ_TIMEOUT_USECOND;
  time_t write_timeout_sec_ = CPPHTTPLIB_SERVER_WRITE_TIMEOUT_SECOND;
  time_t write_timeout_usec_ = CPPHTTPLIB_SERVER_WRITE_TIMEOUT_USECOND;
  time_t idle_interval_sec_ = CPPHTTPLIB_IDLE_INTERVAL_SECOND;
  time_t idle_interval_usec_ = CPPHTTPLIB_IDLE_INTERVAL_USECOND;
  size_t payload_max_length_ = CPPHTTPLIB_PAYLOAD_MAX_LENGTH;

private:
  using Handlers =
      std::vector<std::pair<std::unique_ptr<detail::MatcherBase>, Handler>>;
  using HandlersForContentReader =
      std::vector<std::pair<std::unique_ptr<detail::MatcherBase>,
                            HandlerWithContentReader>>;

  static std::unique_ptr<detail::MatcherBase>
  make_matcher(const std::string &pattern);

  Server &set_error_handler_core(HandlerWithResponse handler, std::true_type);
  Server &set_error_handler_core(Handler handler, std::false_type);

  socket_t create_server_socket(const std::string &host, int port,
                                int socket_flags,
                                SocketOptions socket_options) const;
  int bind_internal(const std::string &host, int port, int socket_flags);
  bool listen_internal();

  bool routing(Request &req, Response &res, Stream &strm);
  bool handle_file_request(const Request &req, Response &res,
                           bool head = false);
  bool dispatch_request(Request &req, Response &res,
                        const Handlers &handlers) const;
  bool dispatch_request_for_content_reader(
      Request &req, Response &res, ContentReader content_reader,
      const HandlersForContentReader &handlers) const;

  bool parse_request_line(const char *s, Request &req) const;
  void apply_ranges(const Request &req, Response &res,
                    std::string &content_type, std::string &boundary) const;
  bool write_response(Stream &strm, bool close_connection, Request &req,
                      Response &res);
  bool write_response_with_content(Stream &strm, bool close_connection,
                                   const Request &req, Response &res);
  bool write_response_core(Stream &strm, bool close_connection,
                           const Request &req, Response &res,
                           bool need_apply_ranges);
  bool write_content_with_provider(Stream &strm, const Request &req,
                                   Response &res, const std::string &boundary,
                                   const std::string &content_type);
  bool read_content(Stream &strm, Request &req, Response &res);
  bool
  read_content_with_content_receiver(Stream &strm, Request &req, Response &res,
                                     ContentReceiver receiver,
                                     MultipartContentHeader multipart_header,
                                     ContentReceiver multipart_receiver);
  bool read_content_core(Stream &strm, Request &req, Response &res,
                         ContentReceiver receiver,
                         MultipartContentHeader multipart_header,
                         ContentReceiver multipart_receiver) const;

  virtual bool process_and_close_socket(socket_t sock);

  std::atomic<bool> is_running_{false};
  std::atomic<bool> is_decommissioned{false};

  struct MountPointEntry {
    std::string mount_point;
    std::string base_dir;
    Headers headers;
  };
  std::vector<MountPointEntry> base_dirs_;
  std::map<std::string, std::string> file_extension_and_mimetype_map_;
  std::string default_file_mimetype_ = "application/octet-stream";
  Handler file_request_handler_;

  Handlers get_handlers_;
  Handlers post_handlers_;
  HandlersForContentReader post_handlers_for_content_reader_;
  Handlers put_handlers_;
  HandlersForContentReader put_handlers_for_content_reader_;
  Handlers patch_handlers_;
  HandlersForContentReader patch_handlers_for_content_reader_;
  Handlers delete_handlers_;
  HandlersForContentReader delete_handlers_for_content_reader_;
  Handlers options_handlers_;

  HandlerWithResponse error_handler_;
  ExceptionHandler exception_handler_;
  HandlerWithResponse pre_routing_handler_;
  Handler post_routing_handler_;
  Expect100ContinueHandler expect_100_continue_handler_;

  Logger logger_;

  int address_family_ = AF_UNSPEC;
  bool tcp_nodelay_ = CPPHTTPLIB_TCP_NODELAY;
  bool ipv6_v6only_ = CPPHTTPLIB_IPV6_V6ONLY;
  SocketOptions socket_options_ = default_socket_options;

  Headers default_headers_;
  std::function<ssize_t(Stream &, Headers &)> header_writer_ =
      detail::write_headers;
};

enum class Error {
  Success = 0,
  Unknown,
  Connection,
  BindIPAddress,
  Read,
  Write,
  ExceedRedirectCount,
  Canceled,
  SSLConnection,
  SSLLoadingCerts,
  SSLServerVerification,
  SSLServerHostnameVerification,
  UnsupportedMultipartBoundaryChars,
  Compression,
  ConnectionTimeout,
  ProxyConnection,

  // For internal use only
  SSLPeerCouldBeClosed_,
};

std::string to_string(Error error);

std::ostream &operator<<(std::ostream &os, const Error &obj);

class Result {
public:
  Result() = default;
  Result(std::unique_ptr<Response> &&res, Error err,
         Headers &&request_headers = Headers{})
      : res_(std::move(res)), err_(err),
        request_headers_(std::move(request_headers)) {}
  // Response
  operator bool() const { return res_ != nullptr; }
  bool operator==(std::nullptr_t) const { return res_ == nullptr; }
  bool operator!=(std::nullptr_t) const { return res_ != nullptr; }
  const Response &value() const { return *res_; }
  Response &value() { return *res_; }
  const Response &operator*() const { return *res_; }
  Response &operator*() { return *res_; }
  const Response *operator->() const { return res_.get(); }
  Response *operator->() { return res_.get(); }

  // Error
  Error error() const { return err_; }

  // Request Headers
  bool has_request_header(const std::string &key) const;
  std::string get_request_header_value(const std::string &key,
                                       const char *def = "",
                                       size_t id = 0) const;
  uint64_t get_request_header_value_u64(const std::string &key,
                                        uint64_t def = 0, size_t id = 0) const;
  size_t get_request_header_value_count(const std::string &key) const;

private:
  std::unique_ptr<Response> res_;
  Error err_ = Error::Unknown;
  Headers request_headers_;
};

class ClientImpl {
public:
  explicit ClientImpl(const std::string &host);

  explicit ClientImpl(const std::string &host, int port);

  explicit ClientImpl(const std::string &host, int port,
                      const std::string &client_cert_path,
                      const std::string &client_key_path);

  virtual ~ClientImpl();

  virtual bool is_valid() const;

  Result Get(const std::string &path);
  Result Get(const std::string &path, const Headers &headers);
  Result Get(const std::string &path, Progress progress);
  Result Get(const std::string &path, const Headers &headers,
             Progress progress);
  Result Get(const std::string &path, ContentReceiver content_receiver);
  Result Get(const std::string &path, const Headers &headers,
             ContentReceiver content_receiver);
  Result Get(const std::string &path, ContentReceiver content_receiver,
             Progress progress);
  Result Get(const std::string &path, const Headers &headers,
             ContentReceiver content_receiver, Progress progress);
  Result Get(const std::string &path, ResponseHandler response_handler,
             ContentReceiver content_receiver);
  Result Get(const std::string &path, const Headers &headers,
             ResponseHandler response_handler,
             ContentReceiver content_receiver);
  Result Get(const std::string &path, ResponseHandler response_handler,
             ContentReceiver content_receiver, Progress progress);
  Result Get(const std::string &path, const Headers &headers,
             ResponseHandler response_handler, ContentReceiver content_receiver,
             Progress progress);

  Result Get(const std::string &path, const Params &params,
             const Headers &headers, Progress progress = nullptr);
  Result Get(const std::string &path, const Params &params,
             const Headers &headers, ContentReceiver content_receiver,
             Progress progress = nullptr);
  Result Get(const std::string &path, const Params &params,
             const Headers &headers, ResponseHandler response_handler,
             ContentReceiver content_receiver, Progress progress = nullptr);

  Result Head(const std::string &path);
  Result Head(const std::string &path, const Headers &headers);

  Result Post(const std::string &path);
  Result Post(const std::string &path, const Headers &headers);
  Result Post(const std::string &path, const char *body, size_t content_length,
              const std::string &content_type);
  Result Post(const std::string &path, const Headers &headers, const char *body,
              size_t content_length, const std::string &content_type);
  Result Post(const std::string &path, const Headers &headers, const char *body,
              size_t content_length, const std::string &content_type,
              Progress progress);
  Result Post(const std::string &path, const std::string &body,
              const std::string &content_type);
  Result Post(const std::string &path, const std::string &body,
              const std::string &content_type, Progress progress);
  Result Post(const std::string &path, const Headers &headers,
              const std::string &body, const std::string &content_type);
  Result Post(const std::string &path, const Headers &headers,
              const std::string &body, const std::string &content_type,
              Progress progress);
  Result Post(const std::string &path, size_t content_length,
              ContentProvider content_provider,
              const std::string &content_type);
  Result Post(const std::string &path,
              ContentProviderWithoutLength content_provider,
              const std::string &content_type);
  Result Post(const std::string &path, const Headers &headers,
              size_t content_length, ContentProvider content_provider,
              const std::string &content_type);
  Result Post(const std::string &path, const Headers &headers,
              ContentProviderWithoutLength content_provider,
              const std::string &content_type);
  Result Post(const std::string &path, const Params &params);
  Result Post(const std::string &path, const Headers &headers,
              const Params &params);
  Result Post(const std::string &path, const Headers &headers,
              const Params &params, Progress progress);
  Result Post(const std::string &path, const MultipartFormDataItems &items);
  Result Post(const std::string &path, const Headers &headers,
              const MultipartFormDataItems &items);
  Result Post(const std::string &path, const Headers &headers,
              const MultipartFormDataItems &items, const std::string &boundary);
  Result Post(const std::string &path, const Headers &headers,
              const MultipartFormDataItems &items,
              const MultipartFormDataProviderItems &provider_items);

  Result Put(const std::string &path);
  Result Put(const std::string &path, const char *body, size_t content_length,
             const std::string &content_type);
  Result Put(const std::string &path, const Headers &headers, const char *body,
             size_t content_length, const std::string &content_type);
  Result Put(const std::string &path, const Headers &headers, const char *body,
             size_t content_length, const std::string &content_type,
             Progress progress);
  Result Put(const std::string &path, const std::string &body,
             const std::string &content_type);
  Result Put(const std::string &path, const std::string &body,
             const std::string &content_type, Progress progress);
  Result Put(const std::string &path, const Headers &headers,
             const std::string &body, const std::string &content_type);
  Result Put(const std::string &path, const Headers &headers,
             const std::string &body, const std::string &content_type,
             Progress progress);
  Result Put(const std::string &path, size_t content_length,
             ContentProvider content_provider, const std::string &content_type);
  Result Put(const std::string &path,
             ContentProviderWithoutLength content_provider,
             const std::string &content_type);
  Result Put(const std::string &path, const Headers &headers,
             size_t content_length, ContentProvider content_provider,
             const std::string &content_type);
  Result Put(const std::string &path, const Headers &headers,
             ContentProviderWithoutLength content_provider,
             const std::string &content_type);
  Result Put(const std::string &path, const Params &params);
  Result Put(const std::string &path, const Headers &headers,
             const Params &params);
  Result Put(const std::string &path, const Headers &headers,
             const Params &params, Progress progress);
  Result Put(const std::string &path, const MultipartFormDataItems &items);
  Result Put(const std::string &path, const Headers &headers,
             const MultipartFormDataItems &items);
  Result Put(const std::string &path, const Headers &headers,
             const MultipartFormDataItems &items, const std::string &boundary);
  Result Put(const std::string &path, const Headers &headers,
             const MultipartFormDataItems &items,
             const MultipartFormDataProviderItems &provider_items);

  Result Patch(const std::string &path);
  Result Patch(const std::string &path, const char *body, size_t content_length,
               const std::string &content_type);
  Result Patch(const std::string &path, const char *body, size_t content_length,
               const std::string &content_type, Progress progress);
  Result Patch(const std::string &path, const Headers &headers,
               const char *body, size_t content_length,
               const std::string &content_type);
  Result Patch(const std::string &path, const Headers &headers,
               const char *body, size_t content_length,
               const std::string &content_type, Progress progress);
  Result Patch(const std::string &path, const std::string &body,
               const std::string &content_type);
  Result Patch(const std::string &path, const std::string &body,
               const std::string &content_type, Progress progress);
  Result Patch(const std::string &path, const Headers &headers,
               const std::string &body, const std::string &content_type);
  Result Patch(const std::string &path, const Headers &headers,
               const std::string &body, const std::string &content_type,
               Progress progress);
  Result Patch(const std::string &path, size_t content_length,
               ContentProvider content_provider,
               const std::string &content_type);
  Result Patch(const std::string &path,
               ContentProviderWithoutLength content_provider,
               const std::string &content_type);
  Result Patch(const std::string &path, const Headers &headers,
               size_t content_length, ContentProvider content_provider,
               const std::string &content_type);
  Result Patch(const std::string &path, const Headers &headers,
               ContentProviderWithoutLength content_provider,
               const std::string &content_type);

  Result Delete(const std::string &path);
  Result Delete(const std::string &path, const Headers &headers);
  Result Delete(const std::string &path, const char *body,
                size_t content_length, const std::string &content_type);
  Result Delete(const std::string &path, const char *body,
                size_t content_length, const std::string &content_type,
                Progress progress);
  Result Delete(const std::string &path, const Headers &headers,
                const char *body, size_t content_length,
                const std::string &content_type);
  Result Delete(const std::string &path, const Headers &headers,
                const char *body, size_t content_length,
                const std::string &content_type, Progress progress);
  Result Delete(const std::string &path, const std::string &body,
                const std::string &content_type);
  Result Delete(const std::string &path, const std::string &body,
                const std::string &content_type, Progress progress);
  Result Delete(const std::string &path, const Headers &headers,
                const std::string &body, const std::string &content_type);
  Result Delete(const std::string &path, const Headers &headers,
                const std::string &body, const std::string &content_type,
                Progress progress);

  Result Options(const std::string &path);
  Result Options(const std::string &path, const Headers &headers);

  bool send(Request &req, Response &res, Error &error);
  Result send(const Request &req);

  void stop();

  std::string host() const;
  int port() const;

  size_t is_socket_open() const;
  socket_t socket() const;

  void set_hostname_addr_map(std::map<std::string, std::string> addr_map);

  void set_default_headers(Headers headers);

  void
  set_header_writer(std::function<ssize_t(Stream &, Headers &)> const &writer);

  void set_address_family(int family);
  void set_tcp_nodelay(bool on);
  void set_ipv6_v6only(bool on);
  void set_socket_options(SocketOptions socket_options);

  void set_connection_timeout(time_t sec, time_t usec = 0);
  template <class Rep, class Period>
  void
  set_connection_timeout(const std::chrono::duration<Rep, Period> &duration);

  void set_read_timeout(time_t sec, time_t usec = 0);
  template <class Rep, class Period>
  void set_read_timeout(const std::chrono::duration<Rep, Period> &duration);

  void set_write_timeout(time_t sec, time_t usec = 0);
  template <class Rep, class Period>
  void set_write_timeout(const std::chrono::duration<Rep, Period> &duration);

  void set_max_timeout(time_t msec);
  template <class Rep, class Period>
  void set_max_timeout(const std::chrono::duration<Rep, Period> &duration);

  void set_basic_auth(const std::string &username, const std::string &password);
  void set_bearer_token_auth(const std::string &token);
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  void set_digest_auth(const std::string &username,
                       const std::string &password);
#endif

  void set_keep_alive(bool on);
  void set_follow_location(bool on);

  void set_url_encode(bool on);

  void set_compress(bool on);

  void set_decompress(bool on);

  void set_interface(const std::string &intf);

  void set_proxy(const std::string &host, int port);
  void set_proxy_basic_auth(const std::string &username,
                            const std::string &password);
  void set_proxy_bearer_token_auth(const std::string &token);
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  void set_proxy_digest_auth(const std::string &username,
                             const std::string &password);
#endif

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  void set_ca_cert_path(const std::string &ca_cert_file_path,
                        const std::string &ca_cert_dir_path = std::string());
  void set_ca_cert_store(X509_STORE *ca_cert_store);
  X509_STORE *create_ca_cert_store(const char *ca_cert, std::size_t size) const;
#endif

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  void enable_server_certificate_verification(bool enabled);
  void enable_server_hostname_verification(bool enabled);
  void set_server_certificate_verifier(
      std::function<SSLVerifierResponse(SSL *ssl)> verifier);
#endif

  void set_logger(Logger logger);

protected:
  struct Socket {
    socket_t sock = INVALID_SOCKET;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    SSL *ssl = nullptr;
#endif

    bool is_open() const { return sock != INVALID_SOCKET; }
  };

  virtual bool create_and_connect_socket(Socket &socket, Error &error);

  // All of:
  //   shutdown_ssl
  //   shutdown_socket
  //   close_socket
  // should ONLY be called when socket_mutex_ is locked.
  // Also, shutdown_ssl and close_socket should also NOT be called concurrently
  // with a DIFFERENT thread sending requests using that socket.
  virtual void shutdown_ssl(Socket &socket, bool shutdown_gracefully);
  void shutdown_socket(Socket &socket) const;
  void close_socket(Socket &socket);

  bool process_request(Stream &strm, Request &req, Response &res,
                       bool close_connection, Error &error);

  bool write_content_with_provider(Stream &strm, const Request &req,
                                   Error &error) const;

  void copy_settings(const ClientImpl &rhs);

  // Socket endpoint information
  const std::string host_;
  const int port_;
  const std::string host_and_port_;

  // Current open socket
  Socket socket_;
  mutable std::mutex socket_mutex_;
  std::recursive_mutex request_mutex_;

  // These are all protected under socket_mutex
  size_t socket_requests_in_flight_ = 0;
  std::thread::id socket_requests_are_from_thread_ = std::thread::id();
  bool socket_should_be_closed_when_request_is_done_ = false;

  // Hostname-IP map
  std::map<std::string, std::string> addr_map_;

  // Default headers
  Headers default_headers_;

  // Header writer
  std::function<ssize_t(Stream &, Headers &)> header_writer_ =
      detail::write_headers;

  // Settings
  std::string client_cert_path_;
  std::string client_key_path_;

  time_t connection_timeout_sec_ = CPPHTTPLIB_CONNECTION_TIMEOUT_SECOND;
  time_t connection_timeout_usec_ = CPPHTTPLIB_CONNECTION_TIMEOUT_USECOND;
  time_t read_timeout_sec_ = CPPHTTPLIB_CLIENT_READ_TIMEOUT_SECOND;
  time_t read_timeout_usec_ = CPPHTTPLIB_CLIENT_READ_TIMEOUT_USECOND;
  time_t write_timeout_sec_ = CPPHTTPLIB_CLIENT_WRITE_TIMEOUT_SECOND;
  time_t write_timeout_usec_ = CPPHTTPLIB_CLIENT_WRITE_TIMEOUT_USECOND;
  time_t max_timeout_msec_ = CPPHTTPLIB_CLIENT_MAX_TIMEOUT_MSECOND;

  std::string basic_auth_username_;
  std::string basic_auth_password_;
  std::string bearer_token_auth_token_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  std::string digest_auth_username_;
  std::string digest_auth_password_;
#endif

  bool keep_alive_ = false;
  bool follow_location_ = false;

  bool url_encode_ = true;

  int address_family_ = AF_UNSPEC;
  bool tcp_nodelay_ = CPPHTTPLIB_TCP_NODELAY;
  bool ipv6_v6only_ = CPPHTTPLIB_IPV6_V6ONLY;
  SocketOptions socket_options_ = nullptr;

  bool compress_ = false;
  bool decompress_ = true;

  std::string interface_;

  std::string proxy_host_;
  int proxy_port_ = -1;

  std::string proxy_basic_auth_username_;
  std::string proxy_basic_auth_password_;
  std::string proxy_bearer_token_auth_token_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  std::string proxy_digest_auth_username_;
  std::string proxy_digest_auth_password_;
#endif

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  std::string ca_cert_file_path_;
  std::string ca_cert_dir_path_;

  X509_STORE *ca_cert_store_ = nullptr;
#endif

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  bool server_certificate_verification_ = true;
  bool server_hostname_verification_ = true;
  std::function<SSLVerifierResponse(SSL *ssl)> server_certificate_verifier_;
#endif

  Logger logger_;

private:
  bool send_(Request &req, Response &res, Error &error);
  Result send_(Request &&req);

  socket_t create_client_socket(Error &error) const;
  bool read_response_line(Stream &strm, const Request &req,
                          Response &res) const;
  bool write_request(Stream &strm, Request &req, bool close_connection,
                     Error &error);
  bool redirect(Request &req, Response &res, Error &error);
  bool handle_request(Stream &strm, Request &req, Response &res,
                      bool close_connection, Error &error);
  std::unique_ptr<Response> send_with_content_provider(
      Request &req, const char *body, size_t content_length,
      ContentProvider content_provider,
      ContentProviderWithoutLength content_provider_without_length,
      const std::string &content_type, Error &error);
  Result send_with_content_provider(
      const std::string &method, const std::string &path,
      const Headers &headers, const char *body, size_t content_length,
      ContentProvider content_provider,
      ContentProviderWithoutLength content_provider_without_length,
      const std::string &content_type, Progress progress);
  ContentProviderWithoutLength get_multipart_content_provider(
      const std::string &boundary, const MultipartFormDataItems &items,
      const MultipartFormDataProviderItems &provider_items) const;

  std::string adjust_host_string(const std::string &host) const;

  virtual bool
  process_socket(const Socket &socket,
                 std::chrono::time_point<std::chrono::steady_clock> start_time,
                 std::function<bool(Stream &strm)> callback);
  virtual bool is_ssl() const;
};

class Client {
public:
  // Universal interface
  explicit Client(const std::string &scheme_host_port);

  explicit Client(const std::string &scheme_host_port,
                  const std::string &client_cert_path,
                  const std::string &client_key_path);

  // HTTP only interface
  explicit Client(const std::string &host, int port);

  explicit Client(const std::string &host, int port,
                  const std::string &client_cert_path,
                  const std::string &client_key_path);

  Client(Client &&) = default;
  Client &operator=(Client &&) = default;

  ~Client();

  bool is_valid() const;

  Result Get(const std::string &path);
  Result Get(const std::string &path, const Headers &headers);
  Result Get(const std::string &path, Progress progress);
  Result Get(const std::string &path, const Headers &headers,
             Progress progress);
  Result Get(const std::string &path, ContentReceiver content_receiver);
  Result Get(const std::string &path, const Headers &headers,
             ContentReceiver content_receiver);
  Result Get(const std::string &path, ContentReceiver content_receiver,
             Progress progress);
  Result Get(const std::string &path, const Headers &headers,
             ContentReceiver content_receiver, Progress progress);
  Result Get(const std::string &path, ResponseHandler response_handler,
             ContentReceiver content_receiver);
  Result Get(const std::string &path, const Headers &headers,
             ResponseHandler response_handler,
             ContentReceiver content_receiver);
  Result Get(const std::string &path, const Headers &headers,
             ResponseHandler response_handler, ContentReceiver content_receiver,
             Progress progress);
  Result Get(const std::string &path, ResponseHandler response_handler,
             ContentReceiver content_receiver, Progress progress);

  Result Get(const std::string &path, const Params &params,
             const Headers &headers, Progress progress = nullptr);
  Result Get(const std::string &path, const Params &params,
             const Headers &headers, ContentReceiver content_receiver,
             Progress progress = nullptr);
  Result Get(const std::string &path, const Params &params,
             const Headers &headers, ResponseHandler response_handler,
             ContentReceiver content_receiver, Progress progress = nullptr);

  Result Head(const std::string &path);
  Result Head(const std::string &path, const Headers &headers);

  Result Post(const std::string &path);
  Result Post(const std::string &path, const Headers &headers);
  Result Post(const std::string &path, const char *body, size_t content_length,
              const std::string &content_type);
  Result Post(const std::string &path, const Headers &headers, const char *body,
              size_t content_length, const std::string &content_type);
  Result Post(const std::string &path, const Headers &headers, const char *body,
              size_t content_length, const std::string &content_type,
              Progress progress);
  Result Post(const std::string &path, const std::string &body,
              const std::string &content_type);
  Result Post(const std::string &path, const std::string &body,
              const std::string &content_type, Progress progress);
  Result Post(const std::string &path, const Headers &headers,
              const std::string &body, const std::string &content_type);
  Result Post(const std::string &path, const Headers &headers,
              const std::string &body, const std::string &content_type,
              Progress progress);
  Result Post(const std::string &path, size_t content_length,
              ContentProvider content_provider,
              const std::string &content_type);
  Result Post(const std::string &path,
              ContentProviderWithoutLength content_provider,
              const std::string &content_type);
  Result Post(const std::string &path, const Headers &headers,
              size_t content_length, ContentProvider content_provider,
              const std::string &content_type);
  Result Post(const std::string &path, const Headers &headers,
              ContentProviderWithoutLength content_provider,
              const std::string &content_type);
  Result Post(const std::string &path, const Params &params);
  Result Post(const std::string &path, const Headers &headers,
              const Params &params);
  Result Post(const std::string &path, const Headers &headers,
              const Params &params, Progress progress);
  Result Post(const std::string &path, const MultipartFormDataItems &items);
  Result Post(const std::string &path, const Headers &headers,
              const MultipartFormDataItems &items);
  Result Post(const std::string &path, const Headers &headers,
              const MultipartFormDataItems &items, const std::string &boundary);
  Result Post(const std::string &path, const Headers &headers,
              const MultipartFormDataItems &items,
              const MultipartFormDataProviderItems &provider_items);

  Result Put(const std::string &path);
  Result Put(const std::string &path, const char *body, size_t content_length,
             const std::string &content_type);
  Result Put(const std::string &path, const Headers &headers, const char *body,
             size_t content_length, const std::string &content_type);
  Result Put(const std::string &path, const Headers &headers, const char *body,
             size_t content_length, const std::string &content_type,
             Progress progress);
  Result Put(const std::string &path, const std::string &body,
             const std::string &content_type);
  Result Put(const std::string &path, const std::string &body,
             const std::string &content_type, Progress progress);
  Result Put(const std::string &path, const Headers &headers,
             const std::string &body, const std::string &content_type);
  Result Put(const std::string &path, const Headers &headers,
             const std::string &body, const std::string &content_type,
             Progress progress);
  Result Put(const std::string &path, size_t content_length,
             ContentProvider content_provider, const std::string &content_type);
  Result Put(const std::string &path,
             ContentProviderWithoutLength content_provider,
             const std::string &content_type);
  Result Put(const std::string &path, const Headers &headers,
             size_t content_length, ContentProvider content_provider,
             const std::string &content_type);
  Result Put(const std::string &path, const Headers &headers,
             ContentProviderWithoutLength content_provider,
             const std::string &content_type);
  Result Put(const std::string &path, const Params &params);
  Result Put(const std::string &path, const Headers &headers,
             const Params &params);
  Result Put(const std::string &path, const Headers &headers,
             const Params &params, Progress progress);
  Result Put(const std::string &path, const MultipartFormDataItems &items);
  Result Put(const std::string &path, const Headers &headers,
             const MultipartFormDataItems &items);
  Result Put(const std::string &path, const Headers &headers,
             const MultipartFormDataItems &items, const std::string &boundary);
  Result Put(const std::string &path, const Headers &headers,
             const MultipartFormDataItems &items,
             const MultipartFormDataProviderItems &provider_items);

  Result Patch(const std::string &path);
  Result Patch(const std::string &path, const char *body, size_t content_length,
               const std::string &content_type);
  Result Patch(const std::string &path, const char *body, size_t content_length,
               const std::string &content_type, Progress progress);
  Result Patch(const std::string &path, const Headers &headers,
               const char *body, size_t content_length,
               const std::string &content_type);
  Result Patch(const std::string &path, const Headers &headers,
               const char *body, size_t content_length,
               const std::string &content_type, Progress progress);
  Result Patch(const std::string &path, const std::string &body,
               const std::string &content_type);
  Result Patch(const std::string &path, const std::string &body,
               const std::string &content_type, Progress progress);
  Result Patch(const std::string &path, const Headers &headers,
               const std::string &body, const std::string &content_type);
  Result Patch(const std::string &path, const Headers &headers,
               const std::string &body, const std::string &content_type,
               Progress progress);
  Result Patch(const std::string &path, size_t content_length,
               ContentProvider content_provider,
               const std::string &content_type);
  Result Patch(const std::string &path,
               ContentProviderWithoutLength content_provider,
               const std::string &content_type);
  Result Patch(const std::string &path, const Headers &headers,
               size_t content_length, ContentProvider content_provider,
               const std::string &content_type);
  Result Patch(const std::string &path, const Headers &headers,
               ContentProviderWithoutLength content_provider,
               const std::string &content_type);

  Result Delete(const std::string &path);
  Result Delete(const std::string &path, const Headers &headers);
  Result Delete(const std::string &path, const char *body,
                size_t content_length, const std::string &content_type);
  Result Delete(const std::string &path, const char *body,
                size_t content_length, const std::string &content_type,
                Progress progress);
  Result Delete(const std::string &path, const Headers &headers,
                const char *body, size_t content_length,
                const std::string &content_type);
  Result Delete(const std::string &path, const Headers &headers,
                const char *body, size_t content_length,
                const std::string &content_type, Progress progress);
  Result Delete(const std::string &path, const std::string &body,
                const std::string &content_type);
  Result Delete(const std::string &path, const std::string &body,
                const std::string &content_type, Progress progress);
  Result Delete(const std::string &path, const Headers &headers,
                const std::string &body, const std::string &content_type);
  Result Delete(const std::string &path, const Headers &headers,
                const std::string &body, const std::string &content_type,
                Progress progress);

  Result Options(const std::string &path);
  Result Options(const std::string &path, const Headers &headers);

  bool send(Request &req, Response &res, Error &error);
  Result send(const Request &req);

  void stop();

  std::string host() const;
  int port() const;

  size_t is_socket_open() const;
  socket_t socket() const;

  void set_hostname_addr_map(std::map<std::string, std::string> addr_map);

  void set_default_headers(Headers headers);

  void
  set_header_writer(std::function<ssize_t(Stream &, Headers &)> const &writer);

  void set_address_family(int family);
  void set_tcp_nodelay(bool on);
  void set_socket_options(SocketOptions socket_options);

  void set_connection_timeout(time_t sec, time_t usec = 0);
  template <class Rep, class Period>
  void
  set_connection_timeout(const std::chrono::duration<Rep, Period> &duration);

  void set_read_timeout(time_t sec, time_t usec = 0);
  template <class Rep, class Period>
  void set_read_timeout(const std::chrono::duration<Rep, Period> &duration);

  void set_write_timeout(time_t sec, time_t usec = 0);
  template <class Rep, class Period>
  void set_write_timeout(const std::chrono::duration<Rep, Period> &duration);

  void set_max_timeout(time_t msec);
  template <class Rep, class Period>
  void set_max_timeout(const std::chrono::duration<Rep, Period> &duration);

  void set_basic_auth(const std::string &username, const std::string &password);
  void set_bearer_token_auth(const std::string &token);
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  void set_digest_auth(const std::string &username,
                       const std::string &password);
#endif

  void set_keep_alive(bool on);
  void set_follow_location(bool on);

  void set_url_encode(bool on);

  void set_compress(bool on);

  void set_decompress(bool on);

  void set_interface(const std::string &intf);

  void set_proxy(const std::string &host, int port);
  void set_proxy_basic_auth(const std::string &username,
                            const std::string &password);
  void set_proxy_bearer_token_auth(const std::string &token);
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  void set_proxy_digest_auth(const std::string &username,
                             const std::string &password);
#endif

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  void enable_server_certificate_verification(bool enabled);
  void enable_server_hostname_verification(bool enabled);
  void set_server_certificate_verifier(
      std::function<SSLVerifierResponse(SSL *ssl)> verifier);
#endif

  void set_logger(Logger logger);

  // SSL
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  void set_ca_cert_path(const std::string &ca_cert_file_path,
                        const std::string &ca_cert_dir_path = std::string());

  void set_ca_cert_store(X509_STORE *ca_cert_store);
  void load_ca_cert_store(const char *ca_cert, std::size_t size);

  long get_openssl_verify_result() const;

  SSL_CTX *ssl_context() const;
#endif

private:
  std::unique_ptr<ClientImpl> cli_;

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  bool is_ssl_ = false;
#endif
};

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
class SSLServer : public Server {
public:
  SSLServer(const char *cert_path, const char *private_key_path,
            const char *client_ca_cert_file_path = nullptr,
            const char *client_ca_cert_dir_path = nullptr,
            const char *private_key_password = nullptr);

  SSLServer(X509 *cert, EVP_PKEY *private_key,
            X509_STORE *client_ca_cert_store = nullptr);

  SSLServer(
      const std::function<bool(SSL_CTX &ssl_ctx)> &setup_ssl_ctx_callback);

  ~SSLServer() override;

  bool is_valid() const override;

  SSL_CTX *ssl_context() const;

  void update_certs(X509 *cert, EVP_PKEY *private_key,
                    X509_STORE *client_ca_cert_store = nullptr);

private:
  bool process_and_close_socket(socket_t sock) override;

  SSL_CTX *ctx_;
  std::mutex ctx_mutex_;
};

class SSLClient final : public ClientImpl {
public:
  explicit SSLClient(const std::string &host);

  explicit SSLClient(const std::string &host, int port);

  explicit SSLClient(const std::string &host, int port,
                     const std::string &client_cert_path,
                     const std::string &client_key_path,
                     const std::string &private_key_password = std::string());

  explicit SSLClient(const std::string &host, int port, X509 *client_cert,
                     EVP_PKEY *client_key,
                     const std::string &private_key_password = std::string());

  ~SSLClient() override;

  bool is_valid() const override;

  void set_ca_cert_store(X509_STORE *ca_cert_store);
  void load_ca_cert_store(const char *ca_cert, std::size_t size);

  long get_openssl_verify_result() const;

  SSL_CTX *ssl_context() const;

private:
  bool create_and_connect_socket(Socket &socket, Error &error) override;
  void shutdown_ssl(Socket &socket, bool shutdown_gracefully) override;
  void shutdown_ssl_impl(Socket &socket, bool shutdown_gracefully);

  bool
  process_socket(const Socket &socket,
                 std::chrono::time_point<std::chrono::steady_clock> start_time,
                 std::function<bool(Stream &strm)> callback) override;
  bool is_ssl() const override;

  bool connect_with_proxy(
      Socket &sock,
      std::chrono::time_point<std::chrono::steady_clock> start_time,
      Response &res, bool &success, Error &error);
  bool initialize_ssl(Socket &socket, Error &error);

  bool load_certs();

  bool verify_host(X509 *server_cert) const;
  bool verify_host_with_subject_alt_name(X509 *server_cert) const;
  bool verify_host_with_common_name(X509 *server_cert) const;
  bool check_host_name(const char *pattern, size_t pattern_len) const;

  SSL_CTX *ctx_;
  std::mutex ctx_mutex_;
  std::once_flag initialize_cert_;

  std::vector<std::string> host_components_;

  long verify_result_ = 0;

  friend class ClientImpl;
};
#endif

/*
 * Implementation of template methods.
 */

namespace detail {

template <typename T, typename U>
inline void duration_to_sec_and_usec(const T &duration, U callback) {
  auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
  auto usec = std::chrono::duration_cast<std::chrono::microseconds>(
                  duration - std::chrono::seconds(sec))
                  .count();
  callback(static_cast<time_t>(sec), static_cast<time_t>(usec));
}

template <size_t N> inline constexpr size_t str_len(const char (&)[N]) {
  return N - 1;
}

inline bool is_numeric(const std::string &str) {
  return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

inline uint64_t get_header_value_u64(const Headers &headers,
                                     const std::string &key, uint64_t def,
                                     size_t id, bool &is_invalid_value) {
  is_invalid_value = false;
  auto rng = headers.equal_range(key);
  auto it = rng.first;
  std::advance(it, static_cast<ssize_t>(id));
  if (it != rng.second) {
    if (is_numeric(it->second)) {
      return std::strtoull(it->second.data(), nullptr, 10);
    } else {
      is_invalid_value = true;
    }
  }
  return def;
}

inline uint64_t get_header_value_u64(const Headers &headers,
                                     const std::string &key, uint64_t def,
                                     size_t id) {
  bool dummy = false;
  return get_header_value_u64(headers, key, def, id, dummy);
}

} // namespace detail

inline uint64_t Request::get_header_value_u64(const std::string &key,
                                              uint64_t def, size_t id) const {
  return detail::get_header_value_u64(headers, key, def, id);
}

inline uint64_t Response::get_header_value_u64(const std::string &key,
                                               uint64_t def, size_t id) const {
  return detail::get_header_value_u64(headers, key, def, id);
}

namespace detail {

inline bool set_socket_opt_impl(socket_t sock, int level, int optname,
                                const void *optval, socklen_t optlen) {
  return setsockopt(sock, level, optname,
#ifdef _WIN32
                    reinterpret_cast<const char *>(optval),
#else
                    optval,
#endif
                    optlen) == 0;
}

inline bool set_socket_opt(socket_t sock, int level, int optname, int optval) {
  return set_socket_opt_impl(sock, level, optname, &optval, sizeof(optval));
}

inline bool set_socket_opt_time(socket_t sock, int level, int optname,
                                time_t sec, time_t usec) {
#ifdef _WIN32
  auto timeout = static_cast<uint32_t>(sec * 1000 + usec / 1000);
#else
  timeval timeout;
  timeout.tv_sec = static_cast<long>(sec);
  timeout.tv_usec = static_cast<decltype(timeout.tv_usec)>(usec);
#endif
  return set_socket_opt_impl(sock, level, optname, &timeout, sizeof(timeout));
}

} // namespace detail

inline void default_socket_options(socket_t sock) {
  detail::set_socket_opt(sock, SOL_SOCKET,
#ifdef SO_REUSEPORT
                         SO_REUSEPORT,
#else
                         SO_REUSEADDR,
#endif
                         1);
}

inline const char *status_message(int status) {
  switch (status) {
  case StatusCode::Continue_100: return "Continue";
  case StatusCode::SwitchingProtocol_101: return "Switching Protocol";
  case StatusCode::Processing_102: return "Processing";
  case StatusCode::EarlyHints_103: return "Early Hints";
  case StatusCode::OK_200: return "OK";
  case StatusCode::Created_201: return "Created";
  case StatusCode::Accepted_202: return "Accepted";
  case StatusCode::NonAuthoritativeInformation_203:
    return "Non-Authoritative Information";
  case StatusCode::NoContent_204: return "No Content";
  case StatusCode::ResetContent_205: return "Reset Content";
  case StatusCode::PartialContent_206: return "Partial Content";
  case StatusCode::MultiStatus_207: return "Multi-Status";
  case StatusCode::AlreadyReported_208: return "Already Reported";
  case StatusCode::IMUsed_226: return "IM Used";
  case StatusCode::MultipleChoices_300: return "Multiple Choices";
  case StatusCode::MovedPermanently_301: return "Moved Permanently";
  case StatusCode::Found_302: return "Found";
  case StatusCode::SeeOther_303: return "See Other";
  case StatusCode::NotModified_304: return "Not Modified";
  case StatusCode::UseProxy_305: return "Use Proxy";
  case StatusCode::unused_306: return "unused";
  case StatusCode::TemporaryRedirect_307: return "Temporary Redirect";
  case StatusCode::PermanentRedirect_308: return "Permanent Redirect";
  case StatusCode::BadRequest_400: return "Bad Request";
  case StatusCode::Unauthorized_401: return "Unauthorized";
  case StatusCode::PaymentRequired_402: return "Payment Required";
  case StatusCode::Forbidden_403: return "Forbidden";
  case StatusCode::NotFound_404: return "Not Found";
  case StatusCode::MethodNotAllowed_405: return "Method Not Allowed";
  case StatusCode::NotAcceptable_406: return "Not Acceptable";
  case StatusCode::ProxyAuthenticationRequired_407:
    return "Proxy Authentication Required";
  case StatusCode::RequestTimeout_408: return "Request Timeout";
  case StatusCode::Conflict_409: return "Conflict";
  case StatusCode::Gone_410: return "Gone";
  case StatusCode::LengthRequired_411: return "Length Required";
  case StatusCode::PreconditionFailed_412: return "Precondition Failed";
  case StatusCode::PayloadTooLarge_413: return "Payload Too Large";
  case StatusCode::UriTooLong_414: return "URI Too Long";
  case StatusCode::UnsupportedMediaType_415: return "Unsupported Media Type";
  case StatusCode::RangeNotSatisfiable_416: return "Range Not Satisfiable";
  case StatusCode::ExpectationFailed_417: return "Expectation Failed";
  case StatusCode::ImATeapot_418: return "I'm a teapot";
  case StatusCode::MisdirectedRequest_421: return "Misdirected Request";
  case StatusCode::UnprocessableContent_422: return "Unprocessable Content";
  case StatusCode::Locked_423: return "Locked";
  case StatusCode::FailedDependency_424: return "Failed Dependency";
  case StatusCode::TooEarly_425: return "Too Early";
  case StatusCode::UpgradeRequired_426: return "Upgrade Required";
  case StatusCode::PreconditionRequired_428: return "Precondition Required";
  case StatusCode::TooManyRequests_429: return "Too Many Requests";
  case StatusCode::RequestHeaderFieldsTooLarge_431:
    return "Request Header Fields Too Large";
  case StatusCode::UnavailableForLegalReasons_451:
    return "Unavailable For Legal Reasons";
  case StatusCode::NotImplemented_501: return "Not Implemented";
  case StatusCode::BadGateway_502: return "Bad Gateway";
  case StatusCode::ServiceUnavailable_503: return "Service Unavailable";
  case StatusCode::GatewayTimeout_504: return "Gateway Timeout";
  case StatusCode::HttpVersionNotSupported_505:
    return "HTTP Version Not Supported";
  case StatusCode::VariantAlsoNegotiates_506: return "Variant Also Negotiates";
  case StatusCode::InsufficientStorage_507: return "Insufficient Storage";
  case StatusCode::LoopDetected_508: return "Loop Detected";
  case StatusCode::NotExtended_510: return "Not Extended";
  case StatusCode::NetworkAuthenticationRequired_511:
    return "Network Authentication Required";

  default:
  case StatusCode::InternalServerError_500: return "Internal Server Error";
  }
}

inline std::string get_bearer_token_auth(const Request &req) {
  if (req.has_header("Authorization")) {
    constexpr auto bearer_header_prefix_len = detail::str_len("Bearer ");
    return req.get_header_value("Authorization")
        .substr(bearer_header_prefix_len);
  }
  return "";
}

template <class Rep, class Period>
inline Server &
Server::set_read_timeout(const std::chrono::duration<Rep, Period> &duration) {
  detail::duration_to_sec_and_usec(
      duration, [&](time_t sec, time_t usec) { set_read_timeout(sec, usec); });
  return *this;
}

template <class Rep, class Period>
inline Server &
Server::set_write_timeout(const std::chrono::duration<Rep, Period> &duration) {
  detail::duration_to_sec_and_usec(
      duration, [&](time_t sec, time_t usec) { set_write_timeout(sec, usec); });
  return *this;
}

template <class Rep, class Period>
inline Server &
Server::set_idle_interval(const std::chrono::duration<Rep, Period> &duration) {
  detail::duration_to_sec_and_usec(
      duration, [&](time_t sec, time_t usec) { set_idle_interval(sec, usec); });
  return *this;
}

inline std::string to_string(const Error error) {
  switch (error) {
  case Error::Success: return "Success (no error)";
  case Error::Connection: return "Could not establish connection";
  case Error::BindIPAddress: return "Failed to bind IP address";
  case Error::Read: return "Failed to read connection";
  case Error::Write: return "Failed to write connection";
  case Error::ExceedRedirectCount: return "Maximum redirect count exceeded";
  case Error::Canceled: return "Connection handling canceled";
  case Error::SSLConnection: return "SSL connection failed";
  case Error::SSLLoadingCerts: return "SSL certificate loading failed";
  case Error::SSLServerVerification: return "SSL server verification failed";
  case Error::SSLServerHostnameVerification:
    return "SSL server hostname verification failed";
  case Error::UnsupportedMultipartBoundaryChars:
    return "Unsupported HTTP multipart boundary characters";
  case Error::Compression: return "Compression failed";
  case Error::ConnectionTimeout: return "Connection timed out";
  case Error::ProxyConnection: return "Proxy connection failed";
  case Error::Unknown: return "Unknown";
  default: break;
  }

  return "Invalid";
}

inline std::ostream &operator<<(std::ostream &os, const Error &obj) {
  os << to_string(obj);
  os << " (" << static_cast<std::underlying_type<Error>::type>(obj) << ')';
  return os;
}

inline uint64_t Result::get_request_header_value_u64(const std::string &key,
                                                     uint64_t def,
                                                     size_t id) const {
  return detail::get_header_value_u64(request_headers_, key, def, id);
}

template <class Rep, class Period>
inline void ClientImpl::set_connection_timeout(
    const std::chrono::duration<Rep, Period> &duration) {
  detail::duration_to_sec_and_usec(duration, [&](time_t sec, time_t usec) {
    set_connection_timeout(sec, usec);
  });
}

template <class Rep, class Period>
inline void ClientImpl::set_read_timeout(
    const std::chrono::duration<Rep, Period> &duration) {
  detail::duration_to_sec_and_usec(
      duration, [&](time_t sec, time_t usec) { set_read_timeout(sec, usec); });
}

template <class Rep, class Period>
inline void ClientImpl::set_write_timeout(
    const std::chrono::duration<Rep, Period> &duration) {
  detail::duration_to_sec_and_usec(
      duration, [&](time_t sec, time_t usec) { set_write_timeout(sec, usec); });
}

template <class Rep, class Period>
inline void ClientImpl::set_max_timeout(
    const std::chrono::duration<Rep, Period> &duration) {
  auto msec =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  set_max_timeout(msec);
}

template <class Rep, class Period>
inline void Client::set_connection_timeout(
    const std::chrono::duration<Rep, Period> &duration) {
  cli_->set_connection_timeout(duration);
}

template <class Rep, class Period>
inline void
Client::set_read_timeout(const std::chrono::duration<Rep, Period> &duration) {
  cli_->set_read_timeout(duration);
}

template <class Rep, class Period>
inline void
Client::set_write_timeout(const std::chrono::duration<Rep, Period> &duration) {
  cli_->set_write_timeout(duration);
}

template <class Rep, class Period>
inline void
Client::set_max_timeout(const std::chrono::duration<Rep, Period> &duration) {
  cli_->set_max_timeout(duration);
}

/*
 * Forward declarations and types that will be part of the .h file if split into
 * .h + .cc.
 */

std::string hosted_at(const std::string &hostname);

void hosted_at(const std::string &hostname, std::vector<std::string> &addrs);

std::string append_query_params(const std::string &path, const Params &params);

std::pair<std::string, std::string> make_range_header(const Ranges &ranges);

std::pair<std::string, std::string>
make_basic_authentication_header(const std::string &username,
                                 const std::string &password,
                                 bool is_proxy = false);

namespace detail {

#if defined(_WIN32)
inline std::wstring u8string_to_wstring(const char *s) {
  std::wstring ws;
  auto len = static_cast<int>(strlen(s));
  auto wlen = ::MultiByteToWideChar(CP_UTF8, 0, s, len, nullptr, 0);
  if (wlen > 0) {
    ws.resize(wlen);
    wlen = ::MultiByteToWideChar(
        CP_UTF8, 0, s, len,
        const_cast<LPWSTR>(reinterpret_cast<LPCWSTR>(ws.data())), wlen);
    if (wlen != static_cast<int>(ws.size())) { ws.clear(); }
  }
  return ws;
}
#endif

struct FileStat {
  FileStat(const std::string &path);
  bool is_file() const;
  bool is_dir() const;

private:
#if defined(_WIN32)
  struct _stat st_;
#else
  struct stat st_;
#endif
  int ret_ = -1;
};

std::string encode_query_param(const std::string &value);

std::string decode_url(const std::string &s, bool convert_plus_to_space);

std::string trim_copy(const std::string &s);

void divide(
    const char *data, std::size_t size, char d,
    std::function<void(const char *, std::size_t, const char *, std::size_t)>
        fn);

void divide(
    const std::string &str, char d,
    std::function<void(const char *, std::size_t, const char *, std::size_t)>
        fn);

void split(const char *b, const char *e, char d,
           std::function<void(const char *, const char *)> fn);

void split(const char *b, const char *e, char d, size_t m,
           std::function<void(const char *, const char *)> fn);

bool process_client_socket(
    socket_t sock, time_t read_timeout_sec, time_t read_timeout_usec,
    time_t write_timeout_sec, time_t write_timeout_usec,
    time_t max_timeout_msec,
    std::chrono::time_point<std::chrono::steady_clock> start_time,
    std::function<bool(Stream &)> callback);

socket_t create_client_socket(const std::string &host, const std::string &ip,
                              int port, int address_family, bool tcp_nodelay,
                              bool ipv6_v6only, SocketOptions socket_options,
                              time_t connection_timeout_sec,
                              time_t connection_timeout_usec,
                              time_t read_timeout_sec, time_t read_timeout_usec,
                              time_t write_timeout_sec,
                              time_t write_timeout_usec,
                              const std::string &intf, Error &error);

const char *get_header_value(const Headers &headers, const std::string &key,
                             const char *def, size_t id);

std::string params_to_query_str(const Params &params);

void parse_query_text(const char *data, std::size_t size, Params &params);

void parse_query_text(const std::string &s, Params &params);

bool parse_multipart_boundary(const std::string &content_type,
                              std::string &boundary);

bool parse_range_header(const std::string &s, Ranges &ranges);

int close_socket(socket_t sock);

ssize_t send_socket(socket_t sock, const void *ptr, size_t size, int flags);

ssize_t read_socket(socket_t sock, void *ptr, size_t size, int flags);

enum class EncodingType { None = 0, Gzip, Brotli, Zstd };

EncodingType encoding_type(const Request &req, const Response &res);

class BufferStream final : public Stream {
public:
  BufferStream() = default;
  ~BufferStream() override = default;

  bool is_readable() const override;
  bool wait_readable() const override;
  bool wait_writable() const override;
  ssize_t read(char *ptr, size_t size) override;
  ssize_t write(const char *ptr, size_t size) override;
  void get_remote_ip_and_port(std::string &ip, int &port) const override;
  void get_local_ip_and_port(std::string &ip, int &port) const override;
  socket_t socket() const override;
  time_t duration() const override;

  const std::string &get_buffer() const;

private:
  std::string buffer;
  size_t position = 0;
};

class compressor {
public:
  virtual ~compressor() = default;

  typedef std::function<bool(const char *data, size_t data_len)> Callback;
  virtual bool compress(const char *data, size_t data_length, bool last,
                        Callback callback) = 0;
};

class decompressor {
public:
  virtual ~decompressor() = default;

  virtual bool is_valid() const = 0;

  typedef std::function<bool(const char *data, size_t data_len)> Callback;
  virtual bool decompress(const char *data, size_t data_length,
                          Callback callback) = 0;
};

class nocompressor final : public compressor {
public:
  ~nocompressor() override = default;

  bool compress(const char *data, size_t data_length, bool /*last*/,
                Callback callback) override;
};

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
class gzip_compressor final : public compressor {
public:
  gzip_compressor();
  ~gzip_compressor() override;

  bool compress(const char *data, size_t data_length, bool last,
                Callback callback) override;

private:
  bool is_valid_ = false;
  z_stream strm_;
};

class gzip_decompressor final : public decompressor {
public:
  gzip_decompressor();
  ~gzip_decompressor() override;

  bool is_valid() const override;

  bool decompress(const char *data, size_t data_length,
                  Callback callback) override;

private:
  bool is_valid_ = false;
  z_stream strm_;
};
#endif

#ifdef CPPHTTPLIB_BROTLI_SUPPORT
class brotli_compressor final : public compressor {
public:
  brotli_compressor();
  ~brotli_compressor();

  bool compress(const char *data, size_t data_length, bool last,
                Callback callback) override;

private:
  BrotliEncoderState *state_ = nullptr;
};

class brotli_decompressor final : public decompressor {
public:
  brotli_decompressor();
  ~brotli_decompressor();

  bool is_valid() const override;

  bool decompress(const char *data, size_t data_length,
                  Callback callback) override;

private:
  BrotliDecoderResult decoder_r;
  BrotliDecoderState *decoder_s = nullptr;
};
#endif

#ifdef CPPHTTPLIB_ZSTD_SUPPORT
class zstd_compressor : public compressor {
public:
  zstd_compressor();
  ~zstd_compressor();

  bool compress(const char *data, size_t data_length, bool last,
                Callback callback) override;

private:
  ZSTD_CCtx *ctx_ = nullptr;
};

class zstd_decompressor : public decompressor {
public:
  zstd_decompressor();
  ~zstd_decompressor();

  bool is_valid() const override;

  bool decompress(const char *data, size_t data_length,
                  Callback callback) override;

private:
  ZSTD_DCtx *ctx_ = nullptr;
};
#endif

// NOTE: until the read size reaches `fixed_buffer_size`, use `fixed_buffer`
// to store data. The call can set memory on stack for performance.
class stream_line_reader {
public:
  stream_line_reader(Stream &strm, char *fixed_buffer,
                     size_t fixed_buffer_size);
  const char *ptr() const;
  size_t size() const;
  bool end_with_crlf() const;
  bool getline();

private:
  void append(char c);

  Stream &strm_;
  char *fixed_buffer_;
  const size_t fixed_buffer_size_;
  size_t fixed_buffer_used_size_ = 0;
  std::string growable_buffer_;
};

class mmap {
public:
  mmap(const char *path);
  ~mmap();

  bool open(const char *path);
  void close();

  bool is_open() const;
  size_t size() const;
  const char *data() const;

private:
#if defined(_WIN32)
  HANDLE hFile_ = NULL;
  HANDLE hMapping_ = NULL;
#else
  int fd_ = -1;
#endif
  size_t size_ = 0;
  void *addr_ = nullptr;
  bool is_open_empty_file = false;
};

// NOTE: https://www.rfc-editor.org/rfc/rfc9110#section-5
namespace fields {

inline bool is_token_char(char c) {
  return std::isalnum(c) || c == '!' || c == '#' || c == '$' || c == '%' ||
         c == '&' || c == '\'' || c == '*' || c == '+' || c == '-' ||
         c == '.' || c == '^' || c == '_' || c == '`' || c == '|' || c == '~';
}

inline bool is_token(const std::string &s) {
  if (s.empty()) { return false; }
  for (auto c : s) {
    if (!is_token_char(c)) { return false; }
  }
  return true;
}

inline bool is_field_name(const std::string &s) { return is_token(s); }

inline bool is_vchar(char c) { return c >= 33 && c <= 126; }

inline bool is_obs_text(char c) { return 128 <= static_cast<unsigned char>(c); }

inline bool is_field_vchar(char c) { return is_vchar(c) || is_obs_text(c); }

inline bool is_field_content(const std::string &s) {
  if (s.empty()) { return true; }

  if (s.size() == 1) {
    return is_field_vchar(s[0]);
  } else if (s.size() == 2) {
    return is_field_vchar(s[0]) && is_field_vchar(s[1]);
  } else {
    size_t i = 0;

    if (!is_field_vchar(s[i])) { return false; }
    i++;

    while (i < s.size() - 1) {
      auto c = s[i++];
      if (c == ' ' || c == '\t' || is_field_vchar(c)) {
      } else {
        return false;
      }
    }

    return is_field_vchar(s[i]);
  }
}

inline bool is_field_value(const std::string &s) { return is_field_content(s); }

} // namespace fields

} // namespace detail

// ----------------------------------------------------------------------------

/*
 * Implementation that will be part of the .cc file if split into .h + .cc.
 */

namespace detail {

inline bool is_hex(char c, int &v) {
  if (0x20 <= c && isdigit(c)) {
    v = c - '0';
    return true;
  } else if ('A' <= c && c <= 'F') {
    v = c - 'A' + 10;
    return true;
  } else if ('a' <= c && c <= 'f') {
    v = c - 'a' + 10;
    return true;
  }
  return false;
}

inline bool from_hex_to_i(const std::string &s, size_t i, size_t cnt,
                          int &val) {
  if (i >= s.size()) { return false; }

  val = 0;
  for (; cnt; i++, cnt--) {
    if (!s[i]) { return false; }
    auto v = 0;
    if (is_hex(s[i], v)) {
      val = val * 16 + v;
    } else {
      return false;
    }
  }
  return true;
}

inline std::string from_i_to_hex(size_t n) {
  static const auto charset = "0123456789abcdef";
  std::string ret;
  do {
    ret = charset[n & 15] + ret;
    n >>= 4;
  } while (n > 0);
  return ret;
}

inline size_t to_utf8(int code, char *buff) {
  if (code < 0x0080) {
    buff[0] = static_cast<char>(code & 0x7F);
    return 1;
  } else if (code < 0x0800) {
    buff[0] = static_cast<char>(0xC0 | ((code >> 6) & 0x1F));
    buff[1] = static_cast<char>(0x80 | (code & 0x3F));
    return 2;
  } else if (code < 0xD800) {
    buff[0] = static_cast<char>(0xE0 | ((code >> 12) & 0xF));
    buff[1] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
    buff[2] = static_cast<char>(0x80 | (code & 0x3F));
    return 3;
  } else if (code < 0xE000) { // D800 - DFFF is invalid...
    return 0;
  } else if (code < 0x10000) {
    buff[0] = static_cast<char>(0xE0 | ((code >> 12) & 0xF));
    buff[1] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
    buff[2] = static_cast<char>(0x80 | (code & 0x3F));
    return 3;
  } else if (code < 0x110000) {
    buff[0] = static_cast<char>(0xF0 | ((code >> 18) & 0x7));
    buff[1] = static_cast<char>(0x80 | ((code >> 12) & 0x3F));
    buff[2] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
    buff[3] = static_cast<char>(0x80 | (code & 0x3F));
    return 4;
  }

  // NOTREACHED
  return 0;
}

// NOTE: This code came up with the following stackoverflow post:
// https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
inline std::string base64_encode(const std::string &in) {
  static const auto lookup =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  std::string out;
  out.reserve(in.size());

  auto val = 0;
  auto valb = -6;

  for (auto c : in) {
    val = (val << 8) + static_cast<uint8_t>(c);
    valb += 8;
    while (valb >= 0) {
      out.push_back(lookup[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }

  if (valb > -6) { out.push_back(lookup[((val << 8) >> (valb + 8)) & 0x3F]); }

  while (out.size() % 4) {
    out.push_back('=');
  }

  return out;
}

inline bool is_valid_path(const std::string &path) {
  size_t level = 0;
  size_t i = 0;

  // Skip slash
  while (i < path.size() && path[i] == '/') {
    i++;
  }

  while (i < path.size()) {
    // Read component
    auto beg = i;
    while (i < path.size() && path[i] != '/') {
      if (path[i] == '\0') {
        return false;
      } else if (path[i] == '\\') {
        return false;
      }
      i++;
    }

    auto len = i - beg;
    assert(len > 0);

    if (!path.compare(beg, len, ".")) {
      ;
    } else if (!path.compare(beg, len, "..")) {
      if (level == 0) { return false; }
      level--;
    } else {
      level++;
    }

    // Skip slash
    while (i < path.size() && path[i] == '/') {
      i++;
    }
  }

  return true;
}

inline FileStat::FileStat(const std::string &path) {
#if defined(_WIN32)
  auto wpath = u8string_to_wstring(path.c_str());
  ret_ = _wstat(wpath.c_str(), &st_);
#else
  ret_ = stat(path.c_str(), &st_);
#endif
}
inline bool FileStat::is_file() const {
  return ret_ >= 0 && S_ISREG(st_.st_mode);
}
inline bool FileStat::is_dir() const {
  return ret_ >= 0 && S_ISDIR(st_.st_mode);
}

inline std::string encode_query_param(const std::string &value) {
  std::ostringstream escaped;
  escaped.fill('0');
  escaped << std::hex;

  for (auto c : value) {
    if (std::isalnum(static_cast<uint8_t>(c)) || c == '-' || c == '_' ||
        c == '.' || c == '!' || c == '~' || c == '*' || c == '\'' || c == '(' ||
        c == ')') {
      escaped << c;
    } else {
      escaped << std::uppercase;
      escaped << '%' << std::setw(2)
              << static_cast<int>(static_cast<unsigned char>(c));
      escaped << std::nouppercase;
    }
  }

  return escaped.str();
}

inline std::string encode_url(const std::string &s) {
  std::string result;
  result.reserve(s.size());

  for (size_t i = 0; s[i]; i++) {
    switch (s[i]) {
    case ' ': result += "%20"; break;
    case '+': result += "%2B"; break;
    case '\r': result += "%0D"; break;
    case '\n': result += "%0A"; break;
    case '\'': result += "%27"; break;
    case ',': result += "%2C"; break;
    // case ':': result += "%3A"; break; // ok? probably...
    case ';': result += "%3B"; break;
    default:
      auto c = static_cast<uint8_t>(s[i]);
      if (c >= 0x80) {
        result += '%';
        char hex[4];
        auto len = snprintf(hex, sizeof(hex) - 1, "%02X", c);
        assert(len == 2);
        result.append(hex, static_cast<size_t>(len));
      } else {
        result += s[i];
      }
      break;
    }
  }

  return result;
}

inline std::string decode_url(const std::string &s,
                              bool convert_plus_to_space) {
  std::string result;

  for (size_t i = 0; i < s.size(); i++) {
    if (s[i] == '%' && i + 1 < s.size()) {
      if (s[i + 1] == 'u') {
        auto val = 0;
        if (from_hex_to_i(s, i + 2, 4, val)) {
          // 4 digits Unicode codes
          char buff[4];
          size_t len = to_utf8(val, buff);
          if (len > 0) { result.append(buff, len); }
          i += 5; // 'u0000'
        } else {
          result += s[i];
        }
      } else {
        auto val = 0;
        if (from_hex_to_i(s, i + 1, 2, val)) {
          // 2 digits hex codes
          result += static_cast<char>(val);
          i += 2; // '00'
        } else {
          result += s[i];
        }
      }
    } else if (convert_plus_to_space && s[i] == '+') {
      result += ' ';
    } else {
      result += s[i];
    }
  }

  return result;
}

inline std::string file_extension(const std::string &path) {
  std::smatch m;
  thread_local auto re = std::regex("\\.([a-zA-Z0-9]+)$");
  if (std::regex_search(path, m, re)) { return m[1].str(); }
  return std::string();
}

inline bool is_space_or_tab(char c) { return c == ' ' || c == '\t'; }

inline std::pair<size_t, size_t> trim(const char *b, const char *e, size_t left,
                                      size_t right) {
  while (b + left < e && is_space_or_tab(b[left])) {
    left++;
  }
  while (right > 0 && is_space_or_tab(b[right - 1])) {
    right--;
  }
  return std::make_pair(left, right);
}

inline std::string trim_copy(const std::string &s) {
  auto r = trim(s.data(), s.data() + s.size(), 0, s.size());
  return s.substr(r.first, r.second - r.first);
}

inline std::string trim_double_quotes_copy(const std::string &s) {
  if (s.length() >= 2 && s.front() == '"' && s.back() == '"') {
    return s.substr(1, s.size() - 2);
  }
  return s;
}

inline void
divide(const char *data, std::size_t size, char d,
       std::function<void(const char *, std::size_t, const char *, std::size_t)>
           fn) {
  const auto it = std::find(data, data + size, d);
  const auto found = static_cast<std::size_t>(it != data + size);
  const auto lhs_data = data;
  const auto lhs_size = static_cast<std::size_t>(it - data);
  const auto rhs_data = it + found;
  const auto rhs_size = size - lhs_size - found;

  fn(lhs_data, lhs_size, rhs_data, rhs_size);
}

inline void
divide(const std::string &str, char d,
       std::function<void(const char *, std::size_t, const char *, std::size_t)>
           fn) {
  divide(str.data(), str.size(), d, std::move(fn));
}

inline void split(const char *b, const char *e, char d,
                  std::function<void(const char *, const char *)> fn) {
  return split(b, e, d, (std::numeric_limits<size_t>::max)(), std::move(fn));
}

inline void split(const char *b, const char *e, char d, size_t m,
                  std::function<void(const char *, const char *)> fn) {
  size_t i = 0;
  size_t beg = 0;
  size_t count = 1;

  while (e ? (b + i < e) : (b[i] != '\0')) {
    if (b[i] == d && count < m) {
      auto r = trim(b, e, beg, i);
      if (r.first < r.second) { fn(&b[r.first], &b[r.second]); }
      beg = i + 1;
      count++;
    }
    i++;
  }

  if (i) {
    auto r = trim(b, e, beg, i);
    if (r.first < r.second) { fn(&b[r.first], &b[r.second]); }
  }
}

inline stream_line_reader::stream_line_reader(Stream &strm, char *fixed_buffer,
                                              size_t fixed_buffer_size)
    : strm_(strm), fixed_buffer_(fixed_buffer),
      fixed_buffer_size_(fixed_buffer_size) {}

inline const char *stream_line_reader::ptr() const {
  if (growable_buffer_.empty()) {
    return fixed_buffer_;
  } else {
    return growable_buffer_.data();
  }
}

inline size_t stream_line_reader::size() const {
  if (growable_buffer_.empty()) {
    return fixed_buffer_used_size_;
  } else {
    return growable_buffer_.size();
  }
}

inline bool stream_line_reader::end_with_crlf() const {
  auto end = ptr() + size();
  return size() >= 2 && end[-2] == '\r' && end[-1] == '\n';
}

inline bool stream_line_reader::getline() {
  fixed_buffer_used_size_ = 0;
  growable_buffer_.clear();

#ifndef CPPHTTPLIB_ALLOW_LF_AS_LINE_TERMINATOR
  char prev_byte = 0;
#endif

  for (size_t i = 0;; i++) {
    char byte;
    auto n = strm_.read(&byte, 1);

    if (n < 0) {
      return false;
    } else if (n == 0) {
      if (i == 0) {
        return false;
      } else {
        break;
      }
    }

    append(byte);

#ifdef CPPHTTPLIB_ALLOW_LF_AS_LINE_TERMINATOR
    if (byte == '\n') { break; }
#else
    if (prev_byte == '\r' && byte == '\n') { break; }
    prev_byte = byte;
#endif
  }

  return true;
}

inline void stream_line_reader::append(char c) {
  if (fixed_buffer_used_size_ < fixed_buffer_size_ - 1) {
    fixed_buffer_[fixed_buffer_used_size_++] = c;
    fixed_buffer_[fixed_buffer_used_size_] = '\0';
  } else {
    if (growable_buffer_.empty()) {
      assert(fixed_buffer_[fixed_buffer_used_size_] == '\0');
      growable_buffer_.assign(fixed_buffer_, fixed_buffer_used_size_);
    }
    growable_buffer_ += c;
  }
}

inline mmap::mmap(const char *path) { open(path); }

inline mmap::~mmap() { close(); }

inline bool mmap::open(const char *path) {
  close();

#if defined(_WIN32)
  auto wpath = u8string_to_wstring(path);
  if (wpath.empty()) { return false; }

#if _WIN32_WINNT >= _WIN32_WINNT_WIN8
  hFile_ = ::CreateFile2(wpath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                         OPEN_EXISTING, NULL);
#else
  hFile_ = ::CreateFileW(wpath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#endif

  if (hFile_ == INVALID_HANDLE_VALUE) { return false; }

  LARGE_INTEGER size{};
  if (!::GetFileSizeEx(hFile_, &size)) { return false; }
  // If the following line doesn't compile due to QuadPart, update Windows SDK.
  // See:
  // https://github.com/yhirose/cpp-httplib/issues/1903#issuecomment-2316520721
  if (static_cast<ULONGLONG>(size.QuadPart) >
      (std::numeric_limits<decltype(size_)>::max)()) {
    // `size_t` might be 32-bits, on 32-bits Windows.
    return false;
  }
  size_ = static_cast<size_t>(size.QuadPart);

#if _WIN32_WINNT >= _WIN32_WINNT_WIN8
  hMapping_ =
      ::CreateFileMappingFromApp(hFile_, NULL, PAGE_READONLY, size_, NULL);
#else
  hMapping_ = ::CreateFileMappingW(hFile_, NULL, PAGE_READONLY, 0, 0, NULL);
#endif

  // Special treatment for an empty file...
  if (hMapping_ == NULL && size_ == 0) {
    close();
    is_open_empty_file = true;
    return true;
  }

  if (hMapping_ == NULL) {
    close();
    return false;
  }

#if _WIN32_WINNT >= _WIN32_WINNT_WIN8
  addr_ = ::MapViewOfFileFromApp(hMapping_, FILE_MAP_READ, 0, 0);
#else
  addr_ = ::MapViewOfFile(hMapping_, FILE_MAP_READ, 0, 0, 0);
#endif

  if (addr_ == nullptr) {
    close();
    return false;
  }
#else
  fd_ = ::open(path, O_RDONLY);
  if (fd_ == -1) { return false; }

  struct stat sb;
  if (fstat(fd_, &sb) == -1) {
    close();
    return false;
  }
  size_ = static_cast<size_t>(sb.st_size);

  addr_ = ::mmap(NULL, size_, PROT_READ, MAP_PRIVATE, fd_, 0);

  // Special treatment for an empty file...
  if (addr_ == MAP_FAILED && size_ == 0) {
    close();
    is_open_empty_file = true;
    return false;
  }
#endif

  return true;
}

inline bool mmap::is_open() const {
  return is_open_empty_file ? true : addr_ != nullptr;
}

inline size_t mmap::size() const { return size_; }

inline const char *mmap::data() const {
  return is_open_empty_file ? "" : static_cast<const char *>(addr_);
}

inline void mmap::close() {
#if defined(_WIN32)
  if (addr_) {
    ::UnmapViewOfFile(addr_);
    addr_ = nullptr;
  }

  if (hMapping_) {
    ::CloseHandle(hMapping_);
    hMapping_ = NULL;
  }

  if (hFile_ != INVALID_HANDLE_VALUE) {
    ::CloseHandle(hFile_);
    hFile_ = INVALID_HANDLE_VALUE;
  }

  is_open_empty_file = false;
#else
  if (addr_ != nullptr) {
    munmap(addr_, size_);
    addr_ = nullptr;
  }

  if (fd_ != -1) {
    ::close(fd_);
    fd_ = -1;
  }
#endif
  size_ = 0;
}
inline int close_socket(socket_t sock) {
#ifdef _WIN32
  return closesocket(sock);
#else
  return close(sock);
#endif
}

template <typename T> inline ssize_t handle_EINTR(T fn) {
  ssize_t res = 0;
  while (true) {
    res = fn();
    if (res < 0 && errno == EINTR) {
      std::this_thread::sleep_for(std::chrono::microseconds{1});
      continue;
    }
    break;
  }
  return res;
}

inline ssize_t read_socket(socket_t sock, void *ptr, size_t size, int flags) {
  return handle_EINTR([&]() {
    return recv(sock,
#ifdef _WIN32
                static_cast<char *>(ptr), static_cast<int>(size),
#else
                ptr, size,
#endif
                flags);
  });
}

inline ssize_t send_socket(socket_t sock, const void *ptr, size_t size,
                           int flags) {
  return handle_EINTR([&]() {
    return send(sock,
#ifdef _WIN32
                static_cast<const char *>(ptr), static_cast<int>(size),
#else
                ptr, size,
#endif
                flags);
  });
}

inline int poll_wrapper(struct pollfd *fds, nfds_t nfds, int timeout) {
#ifdef _WIN32
  return ::WSAPoll(fds, nfds, timeout);
#else
  return ::poll(fds, nfds, timeout);
#endif
}

template <bool Read>
inline ssize_t select_impl(socket_t sock, time_t sec, time_t usec) {
  struct pollfd pfd;
  pfd.fd = sock;
  pfd.events = (Read ? POLLIN : POLLOUT);

  auto timeout = static_cast<int>(sec * 1000 + usec / 1000);

  return handle_EINTR([&]() { return poll_wrapper(&pfd, 1, timeout); });
}

inline ssize_t select_read(socket_t sock, time_t sec, time_t usec) {
  return select_impl<true>(sock, sec, usec);
}

inline ssize_t select_write(socket_t sock, time_t sec, time_t usec) {
  return select_impl<false>(sock, sec, usec);
}

inline Error wait_until_socket_is_ready(socket_t sock, time_t sec,
                                        time_t usec) {
  struct pollfd pfd_read;
  pfd_read.fd = sock;
  pfd_read.events = POLLIN | POLLOUT;

  auto timeout = static_cast<int>(sec * 1000 + usec / 1000);

  auto poll_res =
      handle_EINTR([&]() { return poll_wrapper(&pfd_read, 1, timeout); });

  if (poll_res == 0) { return Error::ConnectionTimeout; }

  if (poll_res > 0 && pfd_read.revents & (POLLIN | POLLOUT)) {
    auto error = 0;
    socklen_t len = sizeof(error);
    auto res = getsockopt(sock, SOL_SOCKET, SO_ERROR,
                          reinterpret_cast<char *>(&error), &len);
    auto successful = res >= 0 && !error;
    return successful ? Error::Success : Error::Connection;
  }

  return Error::Connection;
}

inline bool is_socket_alive(socket_t sock) {
  const auto val = detail::select_read(sock, 0, 0);
  if (val == 0) {
    return true;
  } else if (val < 0 && errno == EBADF) {
    return false;
  }
  char buf[1];
  return detail::read_socket(sock, &buf[0], sizeof(buf), MSG_PEEK) > 0;
}

class SocketStream final : public Stream {
public:
  SocketStream(socket_t sock, time_t read_timeout_sec, time_t read_timeout_usec,
               time_t write_timeout_sec, time_t write_timeout_usec,
               time_t max_timeout_msec = 0,
               std::chrono::time_point<std::chrono::steady_clock> start_time =
                   (std::chrono::steady_clock::time_point::min)());
  ~SocketStream() override;

  bool is_readable() const override;
  bool wait_readable() const override;
  bool wait_writable() const override;
  ssize_t read(char *ptr, size_t size) override;
  ssize_t write(const char *ptr, size_t size) override;
  void get_remote_ip_and_port(std::string &ip, int &port) const override;
  void get_local_ip_and_port(std::string &ip, int &port) const override;
  socket_t socket() const override;
  time_t duration() const override;

private:
  socket_t sock_;
  time_t read_timeout_sec_;
  time_t read_timeout_usec_;
  time_t write_timeout_sec_;
  time_t write_timeout_usec_;
  time_t max_timeout_msec_;
  const std::chrono::time_point<std::chrono::steady_clock> start_time_;

  std::vector<char> read_buff_;
  size_t read_buff_off_ = 0;
  size_t read_buff_content_size_ = 0;

  static const size_t read_buff_size_ = 1024l * 4;
};

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
class SSLSocketStream final : public Stream {
public:
  SSLSocketStream(
      socket_t sock, SSL *ssl, time_t read_timeout_sec,
      time_t read_timeout_usec, time_t write_timeout_sec,
      time_t write_timeout_usec, time_t max_timeout_msec = 0,
      std::chrono::time_point<std::chrono::steady_clock> start_time =
          (std::chrono::steady_clock::time_point::min)());
  ~SSLSocketStream() override;

  bool is_readable() const override;
  bool wait_readable() const override;
  bool wait_writable() const override;
  ssize_t read(char *ptr, size_t size) override;
  ssize_t write(const char *ptr, size_t size) override;
  void get_remote_ip_and_port(std::string &ip, int &port) const override;
  void get_local_ip_and_port(std::string &ip, int &port) const override;
  socket_t socket() const override;
  time_t duration() const override;

private:
  socket_t sock_;
  SSL *ssl_;
  time_t read_timeout_sec_;
  time_t read_timeout_usec_;
  time_t write_timeout_sec_;
  time_t write_timeout_usec_;
  time_t max_timeout_msec_;
  const std::chrono::time_point<std::chrono::steady_clock> start_time_;
};
#endif

inline bool keep_alive(const std::atomic<socket_t> &svr_sock, socket_t sock,
                       time_t keep_alive_timeout_sec) {
  using namespace std::chrono;

  const auto interval_usec =
      CPPHTTPLIB_KEEPALIVE_TIMEOUT_CHECK_INTERVAL_USECOND;

  // Avoid expensive `steady_clock::now()` call for the first time
  if (select_read(sock, 0, interval_usec) > 0) { return true; }

  const auto start = steady_clock::now() - microseconds{interval_usec};
  const auto timeout = seconds{keep_alive_timeout_sec};

  while (true) {
    if (svr_sock == INVALID_SOCKET) {
      break; // Server socket is closed
    }

    auto val = select_read(sock, 0, interval_usec);
    if (val < 0) {
      break; // Ssocket error
    } else if (val == 0) {
      if (steady_clock::now() - start > timeout) {
        break; // Timeout
      }
    } else {
      return true; // Ready for read
    }
  }

  return false;
}

template <typename T>
inline bool
process_server_socket_core(const std::atomic<socket_t> &svr_sock, socket_t sock,
                           size_t keep_alive_max_count,
                           time_t keep_alive_timeout_sec, T callback) {
  assert(keep_alive_max_count > 0);
  auto ret = false;
  auto count = keep_alive_max_count;
  while (count > 0 && keep_alive(svr_sock, sock, keep_alive_timeout_sec)) {
    auto close_connection = count == 1;
    auto connection_closed = false;
    ret = callback(close_connection, connection_closed);
    if (!ret || connection_closed) { break; }
    count--;
  }
  return ret;
}

template <typename T>
inline bool
process_server_socket(const std::atomic<socket_t> &svr_sock, socket_t sock,
                      size_t keep_alive_max_count,
                      time_t keep_alive_timeout_sec, time_t read_timeout_sec,
                      time_t read_timeout_usec, time_t write_timeout_sec,
                      time_t write_timeout_usec, T callback) {
  return process_server_socket_core(
      svr_sock, sock, keep_alive_max_count, keep_alive_timeout_sec,
      [&](bool close_connection, bool &connection_closed) {
        SocketStream strm(sock, read_timeout_sec, read_timeout_usec,
                          write_timeout_sec, write_timeout_usec);
        return callback(strm, close_connection, connection_closed);
      });
}

inline bool process_client_socket(
    socket_t sock, time_t read_timeout_sec, time_t read_timeout_usec,
    time_t write_timeout_sec, time_t write_timeout_usec,
    time_t max_timeout_msec,
    std::chrono::time_point<std::chrono::steady_clock> start_time,
    std::function<bool(Stream &)> callback) {
  SocketStream strm(sock, read_timeout_sec, read_timeout_usec,
                    write_timeout_sec, write_timeout_usec, max_timeout_msec,
                    start_time);
  return callback(strm);
}

inline int shutdown_socket(socket_t sock) {
#ifdef _WIN32
  return shutdown(sock, SD_BOTH);
#else
  return shutdown(sock, SHUT_RDWR);
#endif
}

inline std::string escape_abstract_namespace_unix_domain(const std::string &s) {
  if (s.size() > 1 && s[0] == '\0') {
    auto ret = s;
    ret[0] = '@';
    return ret;
  }
  return s;
}

inline std::string
unescape_abstract_namespace_unix_domain(const std::string &s) {
  if (s.size() > 1 && s[0] == '@') {
    auto ret = s;
    ret[0] = '\0';
    return ret;
  }
  return s;
}

template <typename BindOrConnect>
socket_t create_socket(const std::string &host, const std::string &ip, int port,
                       int address_family, int socket_flags, bool tcp_nodelay,
                       bool ipv6_v6only, SocketOptions socket_options,
                       BindOrConnect bind_or_connect) {
  // Get address info
  const char *node = nullptr;
  struct addrinfo hints;
  struct addrinfo *result;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_IP;

  if (!ip.empty()) {
    node = ip.c_str();
    // Ask getaddrinfo to convert IP in c-string to address
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICHOST;
  } else {
    if (!host.empty()) { node = host.c_str(); }
    hints.ai_family = address_family;
    hints.ai_flags = socket_flags;
  }

  if (hints.ai_family == AF_UNIX) {
    const auto addrlen = host.length();
    if (addrlen > sizeof(sockaddr_un::sun_path)) { return INVALID_SOCKET; }

#ifdef SOCK_CLOEXEC
    auto sock = socket(hints.ai_family, hints.ai_socktype | SOCK_CLOEXEC,
                       hints.ai_protocol);
#else
    auto sock = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
#endif

    if (sock != INVALID_SOCKET) {
      sockaddr_un addr{};
      addr.sun_family = AF_UNIX;

      auto unescaped_host = unescape_abstract_namespace_unix_domain(host);
      std::copy(unescaped_host.begin(), unescaped_host.end(), addr.sun_path);

      hints.ai_addr = reinterpret_cast<sockaddr *>(&addr);
      hints.ai_addrlen = static_cast<socklen_t>(
          sizeof(addr) - sizeof(addr.sun_path) + addrlen);

#ifndef SOCK_CLOEXEC
#ifndef _WIN32
      fcntl(sock, F_SETFD, FD_CLOEXEC);
#endif
#endif

      if (socket_options) { socket_options(sock); }

#ifdef _WIN32
      // Setting SO_REUSEADDR seems not to work well with AF_UNIX on windows, so
      // remove the option.
      detail::set_socket_opt(sock, SOL_SOCKET, SO_REUSEADDR, 0);
#endif

      bool dummy;
      if (!bind_or_connect(sock, hints, dummy)) {
        close_socket(sock);
        sock = INVALID_SOCKET;
      }
    }
    return sock;
  }

  auto service = std::to_string(port);

  if (getaddrinfo(node, service.c_str(), &hints, &result)) {
#if defined __linux__ && !defined __ANDROID__
    res_init();
#endif
    return INVALID_SOCKET;
  }
  auto se = detail::scope_exit([&] { freeaddrinfo(result); });

  for (auto rp = result; rp; rp = rp->ai_next) {
    // Create a socket
#ifdef _WIN32
    auto sock =
        WSASocketW(rp->ai_family, rp->ai_socktype, rp->ai_protocol, nullptr, 0,
                   WSA_FLAG_NO_HANDLE_INHERIT | WSA_FLAG_OVERLAPPED);
    /**
     * Since the WSA_FLAG_NO_HANDLE_INHERIT is only supported on Windows 7 SP1
     * and above the socket creation fails on older Windows Systems.
     *
     * Let's try to create a socket the old way in this case.
     *
     * Reference:
     * https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsasocketa
     *
     * WSA_FLAG_NO_HANDLE_INHERIT:
     * This flag is supported on Windows 7 with SP1, Windows Server 2008 R2 with
     * SP1, and later
     *
     */
    if (sock == INVALID_SOCKET) {
      sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    }
#else

#ifdef SOCK_CLOEXEC
    auto sock =
        socket(rp->ai_family, rp->ai_socktype | SOCK_CLOEXEC, rp->ai_protocol);
#else
    auto sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
#endif

#endif
    if (sock == INVALID_SOCKET) { continue; }

#if !defined _WIN32 && !defined SOCK_CLOEXEC
    if (fcntl(sock, F_SETFD, FD_CLOEXEC) == -1) {
      close_socket(sock);
      continue;
    }
#endif

    if (tcp_nodelay) { set_socket_opt(sock, IPPROTO_TCP, TCP_NODELAY, 1); }

    if (rp->ai_family == AF_INET6) {
      set_socket_opt(sock, IPPROTO_IPV6, IPV6_V6ONLY, ipv6_v6only ? 1 : 0);
    }

    if (socket_options) { socket_options(sock); }

    // bind or connect
    auto quit = false;
    if (bind_or_connect(sock, *rp, quit)) { return sock; }

    close_socket(sock);

    if (quit) { break; }
  }

  return INVALID_SOCKET;
}

inline void set_nonblocking(socket_t sock, bool nonblocking) {
#ifdef _WIN32
  auto flags = nonblocking ? 1UL : 0UL;
  ioctlsocket(sock, FIONBIO, &flags);
#else
  auto flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL,
        nonblocking ? (flags | O_NONBLOCK) : (flags & (~O_NONBLOCK)));
#endif
}

inline bool is_connection_error() {
#ifdef _WIN32
  return WSAGetLastError() != WSAEWOULDBLOCK;
#else
  return errno != EINPROGRESS;
#endif
}

inline bool bind_ip_address(socket_t sock, const std::string &host) {
  struct addrinfo hints;
  struct addrinfo *result;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;

  if (getaddrinfo(host.c_str(), "0", &hints, &result)) { return false; }
  auto se = detail::scope_exit([&] { freeaddrinfo(result); });

  auto ret = false;
  for (auto rp = result; rp; rp = rp->ai_next) {
    const auto &ai = *rp;
    if (!::bind(sock, ai.ai_addr, static_cast<socklen_t>(ai.ai_addrlen))) {
      ret = true;
      break;
    }
  }

  return ret;
}

#if !defined _WIN32 && !defined ANDROID && !defined _AIX && !defined __MVS__
#define USE_IF2IP
#endif

#ifdef USE_IF2IP
inline std::string if2ip(int address_family, const std::string &ifn) {
  struct ifaddrs *ifap;
  getifaddrs(&ifap);
  auto se = detail::scope_exit([&] { freeifaddrs(ifap); });

  std::string addr_candidate;
  for (auto ifa = ifap; ifa; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr && ifn == ifa->ifa_name &&
        (AF_UNSPEC == address_family ||
         ifa->ifa_addr->sa_family == address_family)) {
      if (ifa->ifa_addr->sa_family == AF_INET) {
        auto sa = reinterpret_cast<struct sockaddr_in *>(ifa->ifa_addr);
        char buf[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &sa->sin_addr, buf, INET_ADDRSTRLEN)) {
          return std::string(buf, INET_ADDRSTRLEN);
        }
      } else if (ifa->ifa_addr->sa_family == AF_INET6) {
        auto sa = reinterpret_cast<struct sockaddr_in6 *>(ifa->ifa_addr);
        if (!IN6_IS_ADDR_LINKLOCAL(&sa->sin6_addr)) {
          char buf[INET6_ADDRSTRLEN] = {};
          if (inet_ntop(AF_INET6, &sa->sin6_addr, buf, INET6_ADDRSTRLEN)) {
            // equivalent to mac's IN6_IS_ADDR_UNIQUE_LOCAL
            auto s6_addr_head = sa->sin6_addr.s6_addr[0];
            if (s6_addr_head == 0xfc || s6_addr_head == 0xfd) {
              addr_candidate = std::string(buf, INET6_ADDRSTRLEN);
            } else {
              return std::string(buf, INET6_ADDRSTRLEN);
            }
          }
        }
      }
    }
  }
  return addr_candidate;
}
#endif

inline socket_t create_client_socket(
    const std::string &host, const std::string &ip, int port,
    int address_family, bool tcp_nodelay, bool ipv6_v6only,
    SocketOptions socket_options, time_t connection_timeout_sec,
    time_t connection_timeout_usec, time_t read_timeout_sec,
    time_t read_timeout_usec, time_t write_timeout_sec,
    time_t write_timeout_usec, const std::string &intf, Error &error) {
  auto sock = create_socket(
      host, ip, port, address_family, 0, tcp_nodelay, ipv6_v6only,
      std::move(socket_options),
      [&](socket_t sock2, struct addrinfo &ai, bool &quit) -> bool {
        if (!intf.empty()) {
#ifdef USE_IF2IP
          auto ip_from_if = if2ip(address_family, intf);
          if (ip_from_if.empty()) { ip_from_if = intf; }
          if (!bind_ip_address(sock2, ip_from_if)) {
            error = Error::BindIPAddress;
            return false;
          }
#endif
        }

        set_nonblocking(sock2, true);

        auto ret =
            ::connect(sock2, ai.ai_addr, static_cast<socklen_t>(ai.ai_addrlen));

        if (ret < 0) {
          if (is_connection_error()) {
            error = Error::Connection;
            return false;
          }
          error = wait_until_socket_is_ready(sock2, connection_timeout_sec,
                                             connection_timeout_usec);
          if (error != Error::Success) {
            if (error == Error::ConnectionTimeout) { quit = true; }
            return false;
          }
        }

        set_nonblocking(sock2, false);
        set_socket_opt_time(sock2, SOL_SOCKET, SO_RCVTIMEO, read_timeout_sec,
                            read_timeout_usec);
        set_socket_opt_time(sock2, SOL_SOCKET, SO_SNDTIMEO, write_timeout_sec,
                            write_timeout_usec);

        error = Error::Success;
        return true;
      });

  if (sock != INVALID_SOCKET) {
    error = Error::Success;
  } else {
    if (error == Error::Success) { error = Error::Connection; }
  }

  return sock;
}

inline bool get_ip_and_port(const struct sockaddr_storage &addr,
                            socklen_t addr_len, std::string &ip, int &port) {
  if (addr.ss_family == AF_INET) {
    port = ntohs(reinterpret_cast<const struct sockaddr_in *>(&addr)->sin_port);
  } else if (addr.ss_family == AF_INET6) {
    port =
        ntohs(reinterpret_cast<const struct sockaddr_in6 *>(&addr)->sin6_port);
  } else {
    return false;
  }

  std::array<char, NI_MAXHOST> ipstr{};
  if (getnameinfo(reinterpret_cast<const struct sockaddr *>(&addr), addr_len,
                  ipstr.data(), static_cast<socklen_t>(ipstr.size()), nullptr,
                  0, NI_NUMERICHOST)) {
    return false;
  }

  ip = ipstr.data();
  return true;
}

inline void get_local_ip_and_port(socket_t sock, std::string &ip, int &port) {
  struct sockaddr_storage addr;
  socklen_t addr_len = sizeof(addr);
  if (!getsockname(sock, reinterpret_cast<struct sockaddr *>(&addr),
                   &addr_len)) {
    get_ip_and_port(addr, addr_len, ip, port);
  }
}

inline void get_remote_ip_and_port(socket_t sock, std::string &ip, int &port) {
  struct sockaddr_storage addr;
  socklen_t addr_len = sizeof(addr);

  if (!getpeername(sock, reinterpret_cast<struct sockaddr *>(&addr),
                   &addr_len)) {
#ifndef _WIN32
    if (addr.ss_family == AF_UNIX) {
#if defined(__linux__)
      struct ucred ucred;
      socklen_t len = sizeof(ucred);
      if (getsockopt(sock, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == 0) {
        port = ucred.pid;
      }
#elif defined(SOL_LOCAL) && defined(SO_PEERPID) // __APPLE__
      pid_t pid;
      socklen_t len = sizeof(pid);
      if (getsockopt(sock, SOL_LOCAL, SO_PEERPID, &pid, &len) == 0) {
        port = pid;
      }
#endif
      return;
    }
#endif
    get_ip_and_port(addr, addr_len, ip, port);
  }
}

inline constexpr unsigned int str2tag_core(const char *s, size_t l,
                                           unsigned int h) {
  return (l == 0)
             ? h
             : str2tag_core(
                   s + 1, l - 1,
                   // Unsets the 6 high bits of h, therefore no overflow happens
                   (((std::numeric_limits<unsigned int>::max)() >> 6) &
                    h * 33) ^
                       static_cast<unsigned char>(*s));
}

inline unsigned int str2tag(const std::string &s) {
  return str2tag_core(s.data(), s.size(), 0);
}

namespace udl {

inline constexpr unsigned int operator""_t(const char *s, size_t l) {
  return str2tag_core(s, l, 0);
}

} // namespace udl

inline std::string
find_content_type(const std::string &path,
                  const std::map<std::string, std::string> &user_data,
                  const std::string &default_content_type) {
  auto ext = file_extension(path);

  auto it = user_data.find(ext);
  if (it != user_data.end()) { return it->second; }

  using udl::operator""_t;

  switch (str2tag(ext)) {
  default: return default_content_type;

  case "css"_t: return "text/css";
  case "csv"_t: return "text/csv";
  case "htm"_t:
  case "html"_t: return "text/html";
  case "js"_t:
  case "mjs"_t: return "text/javascript";
  case "txt"_t: return "text/plain";
  case "vtt"_t: return "text/vtt";

  case "apng"_t: return "image/apng";
  case "avif"_t: return "image/avif";
  case "bmp"_t: return "image/bmp";
  case "gif"_t: return "image/gif";
  case "png"_t: return "image/png";
  case "svg"_t: return "image/svg+xml";
  case "webp"_t: return "image/webp";
  case "ico"_t: return "image/x-icon";
  case "tif"_t: return "image/tiff";
  case "tiff"_t: return "image/tiff";
  case "jpg"_t:
  case "jpeg"_t: return "image/jpeg";

  case "mp4"_t: return "video/mp4";
  case "mpeg"_t: return "video/mpeg";
  case "webm"_t: return "video/webm";

  case "mp3"_t: return "audio/mp3";
  case "mpga"_t: return "audio/mpeg";
  case "weba"_t: return "audio/webm";
  case "wav"_t: return "audio/wave";

  case "otf"_t: return "font/otf";
  case "ttf"_t: return "font/ttf";
  case "woff"_t: return "font/woff";
  case "woff2"_t: return "font/woff2";

  case "7z"_t: return "application/x-7z-compressed";
  case "atom"_t: return "application/atom+xml";
  case "pdf"_t: return "application/pdf";
  case "json"_t: return "application/json";
  case "rss"_t: return "application/rss+xml";
  case "tar"_t: return "application/x-tar";
  case "xht"_t:
  case "xhtml"_t: return "application/xhtml+xml";
  case "xslt"_t: return "application/xslt+xml";
  case "xml"_t: return "application/xml";
  case "gz"_t: return "application/gzip";
  case "zip"_t: return "application/zip";
  case "wasm"_t: return "application/wasm";
  }
}

inline bool can_compress_content_type(const std::string &content_type) {
  using udl::operator""_t;

  auto tag = str2tag(content_type);

  switch (tag) {
  case "image/svg+xml"_t:
  case "application/javascript"_t:
  case "application/json"_t:
  case "application/xml"_t:
  case "application/protobuf"_t:
  case "application/xhtml+xml"_t: return true;

  case "text/event-stream"_t: return false;

  default: return !content_type.rfind("text/", 0);
  }
}

inline EncodingType encoding_type(const Request &req, const Response &res) {
  auto ret =
      detail::can_compress_content_type(res.get_header_value("Content-Type"));
  if (!ret) { return EncodingType::None; }

  const auto &s = req.get_header_value("Accept-Encoding");
  (void)(s);

#ifdef CPPHTTPLIB_BROTLI_SUPPORT
  // TODO: 'Accept-Encoding' has br, not br;q=0
  ret = s.find("br") != std::string::npos;
  if (ret) { return EncodingType::Brotli; }
#endif

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
  // TODO: 'Accept-Encoding' has gzip, not gzip;q=0
  ret = s.find("gzip") != std::string::npos;
  if (ret) { return EncodingType::Gzip; }
#endif

#ifdef CPPHTTPLIB_ZSTD_SUPPORT
  // TODO: 'Accept-Encoding' has zstd, not zstd;q=0
  ret = s.find("zstd") != std::string::npos;
  if (ret) { return EncodingType::Zstd; }
#endif

  return EncodingType::None;
}

inline bool nocompressor::compress(const char *data, size_t data_length,
                                   bool /*last*/, Callback callback) {
  if (!data_length) { return true; }
  return callback(data, data_length);
}

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
inline gzip_compressor::gzip_compressor() {
  std::memset(&strm_, 0, sizeof(strm_));
  strm_.zalloc = Z_NULL;
  strm_.zfree = Z_NULL;
  strm_.opaque = Z_NULL;

  is_valid_ = deflateInit2(&strm_, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8,
                           Z_DEFAULT_STRATEGY) == Z_OK;
}

inline gzip_compressor::~gzip_compressor() { deflateEnd(&strm_); }

inline bool gzip_compressor::compress(const char *data, size_t data_length,
                                      bool last, Callback callback) {
  assert(is_valid_);

  do {
    constexpr size_t max_avail_in =
        (std::numeric_limits<decltype(strm_.avail_in)>::max)();

    strm_.avail_in = static_cast<decltype(strm_.avail_in)>(
        (std::min)(data_length, max_avail_in));
    strm_.next_in = const_cast<Bytef *>(reinterpret_cast<const Bytef *>(data));

    data_length -= strm_.avail_in;
    data += strm_.avail_in;

    auto flush = (last && data_length == 0) ? Z_FINISH : Z_NO_FLUSH;
    auto ret = Z_OK;

    std::array<char, CPPHTTPLIB_COMPRESSION_BUFSIZ> buff{};
    do {
      strm_.avail_out = static_cast<uInt>(buff.size());
      strm_.next_out = reinterpret_cast<Bytef *>(buff.data());

      ret = deflate(&strm_, flush);
      if (ret == Z_STREAM_ERROR) { return false; }

      if (!callback(buff.data(), buff.size() - strm_.avail_out)) {
        return false;
      }
    } while (strm_.avail_out == 0);

    assert((flush == Z_FINISH && ret == Z_STREAM_END) ||
           (flush == Z_NO_FLUSH && ret == Z_OK));
    assert(strm_.avail_in == 0);
  } while (data_length > 0);

  return true;
}

inline gzip_decompressor::gzip_decompressor() {
  std::memset(&strm_, 0, sizeof(strm_));
  strm_.zalloc = Z_NULL;
  strm_.zfree = Z_NULL;
  strm_.opaque = Z_NULL;

  // 15 is the value of wbits, which should be at the maximum possible value
  // to ensure that any gzip stream can be decoded. The offset of 32 specifies
  // that the stream type should be automatically detected either gzip or
  // deflate.
  is_valid_ = inflateInit2(&strm_, 32 + 15) == Z_OK;
}

inline gzip_decompressor::~gzip_decompressor() { inflateEnd(&strm_); }

inline bool gzip_decompressor::is_valid() const { return is_valid_; }

inline bool gzip_decompressor::decompress(const char *data, size_t data_length,
                                          Callback callback) {
  assert(is_valid_);

  auto ret = Z_OK;

  do {
    constexpr size_t max_avail_in =
        (std::numeric_limits<decltype(strm_.avail_in)>::max)();

    strm_.avail_in = static_cast<decltype(strm_.avail_in)>(
        (std::min)(data_length, max_avail_in));
    strm_.next_in = const_cast<Bytef *>(reinterpret_cast<const Bytef *>(data));

    data_length -= strm_.avail_in;
    data += strm_.avail_in;

    std::array<char, CPPHTTPLIB_COMPRESSION_BUFSIZ> buff{};
    while (strm_.avail_in > 0 && ret == Z_OK) {
      strm_.avail_out = static_cast<uInt>(buff.size());
      strm_.next_out = reinterpret_cast<Bytef *>(buff.data());

      ret = inflate(&strm_, Z_NO_FLUSH);

      assert(ret != Z_STREAM_ERROR);
      switch (ret) {
      case Z_NEED_DICT:
      case Z_DATA_ERROR:
      case Z_MEM_ERROR: inflateEnd(&strm_); return false;
      }

      if (!callback(buff.data(), buff.size() - strm_.avail_out)) {
        return false;
      }
    }

    if (ret != Z_OK && ret != Z_STREAM_END) { return false; }

  } while (data_length > 0);

  return true;
}
#endif

#ifdef CPPHTTPLIB_BROTLI_SUPPORT
inline brotli_compressor::brotli_compressor() {
  state_ = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
}

inline brotli_compressor::~brotli_compressor() {
  BrotliEncoderDestroyInstance(state_);
}

inline bool brotli_compressor::compress(const char *data, size_t data_length,
                                        bool last, Callback callback) {
  std::array<uint8_t, CPPHTTPLIB_COMPRESSION_BUFSIZ> buff{};

  auto operation = last ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_PROCESS;
  auto available_in = data_length;
  auto next_in = reinterpret_cast<const uint8_t *>(data);

  for (;;) {
    if (last) {
      if (BrotliEncoderIsFinished(state_)) { break; }
    } else {
      if (!available_in) { break; }
    }

    auto available_out = buff.size();
    auto next_out = buff.data();

    if (!BrotliEncoderCompressStream(state_, operation, &available_in, &next_in,
                                     &available_out, &next_out, nullptr)) {
      return false;
    }

    auto output_bytes = buff.size() - available_out;
    if (output_bytes) {
      callback(reinterpret_cast<const char *>(buff.data()), output_bytes);
    }
  }

  return true;
}

inline brotli_decompressor::brotli_decompressor() {
  decoder_s = BrotliDecoderCreateInstance(0, 0, 0);
  decoder_r = decoder_s ? BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT
                        : BROTLI_DECODER_RESULT_ERROR;
}

inline brotli_decompressor::~brotli_decompressor() {
  if (decoder_s) { BrotliDecoderDestroyInstance(decoder_s); }
}

inline bool brotli_decompressor::is_valid() const { return decoder_s; }

inline bool brotli_decompressor::decompress(const char *data,
                                            size_t data_length,
                                            Callback callback) {
  if (decoder_r == BROTLI_DECODER_RESULT_SUCCESS ||
      decoder_r == BROTLI_DECODER_RESULT_ERROR) {
    return 0;
  }

  auto next_in = reinterpret_cast<const uint8_t *>(data);
  size_t avail_in = data_length;
  size_t total_out;

  decoder_r = BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT;

  std::array<char, CPPHTTPLIB_COMPRESSION_BUFSIZ> buff{};
  while (decoder_r == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
    char *next_out = buff.data();
    size_t avail_out = buff.size();

    decoder_r = BrotliDecoderDecompressStream(
        decoder_s, &avail_in, &next_in, &avail_out,
        reinterpret_cast<uint8_t **>(&next_out), &total_out);

    if (decoder_r == BROTLI_DECODER_RESULT_ERROR) { return false; }

    if (!callback(buff.data(), buff.size() - avail_out)) { return false; }
  }

  return decoder_r == BROTLI_DECODER_RESULT_SUCCESS ||
         decoder_r == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT;
}
#endif

#ifdef CPPHTTPLIB_ZSTD_SUPPORT
inline zstd_compressor::zstd_compressor() {
  ctx_ = ZSTD_createCCtx();
  ZSTD_CCtx_setParameter(ctx_, ZSTD_c_compressionLevel, ZSTD_fast);
}

inline zstd_compressor::~zstd_compressor() { ZSTD_freeCCtx(ctx_); }

inline bool zstd_compressor::compress(const char *data, size_t data_length,
                                      bool last, Callback callback) {
  std::array<char, CPPHTTPLIB_COMPRESSION_BUFSIZ> buff{};

  ZSTD_EndDirective mode = last ? ZSTD_e_end : ZSTD_e_continue;
  ZSTD_inBuffer input = {data, data_length, 0};

  bool finished;
  do {
    ZSTD_outBuffer output = {buff.data(), CPPHTTPLIB_COMPRESSION_BUFSIZ, 0};
    size_t const remaining = ZSTD_compressStream2(ctx_, &output, &input, mode);

    if (ZSTD_isError(remaining)) { return false; }

    if (!callback(buff.data(), output.pos)) { return false; }

    finished = last ? (remaining == 0) : (input.pos == input.size);

  } while (!finished);

  return true;
}

inline zstd_decompressor::zstd_decompressor() { ctx_ = ZSTD_createDCtx(); }

inline zstd_decompressor::~zstd_decompressor() { ZSTD_freeDCtx(ctx_); }

inline bool zstd_decompressor::is_valid() const { return ctx_ != nullptr; }

inline bool zstd_decompressor::decompress(const char *data, size_t data_length,
                                          Callback callback) {
  std::array<char, CPPHTTPLIB_COMPRESSION_BUFSIZ> buff{};
  ZSTD_inBuffer input = {data, data_length, 0};

  while (input.pos < input.size) {
    ZSTD_outBuffer output = {buff.data(), CPPHTTPLIB_COMPRESSION_BUFSIZ, 0};
    size_t const remaining = ZSTD_decompressStream(ctx_, &output, &input);

    if (ZSTD_isError(remaining)) { return false; }

    if (!callback(buff.data(), output.pos)) { return false; }
  }

  return true;
}
#endif

inline bool has_header(const Headers &headers, const std::string &key) {
  return headers.find(key) != headers.end();
}

inline const char *get_header_value(const Headers &headers,
                                    const std::string &key, const char *def,
                                    size_t id) {
  auto rng = headers.equal_range(key);
  auto it = rng.first;
  std::advance(it, static_cast<ssize_t>(id));
  if (it != rng.second) { return it->second.c_str(); }
  return def;
}

template <typename T>
inline bool parse_header(const char *beg, const char *end, T fn) {
  // Skip trailing spaces and tabs.
  while (beg < end && is_space_or_tab(end[-1])) {
    end--;
  }

  auto p = beg;
  while (p < end && *p != ':') {
    p++;
  }

  auto name = std::string(beg, p);
  if (!detail::fields::is_field_name(name)) { return false; }

  if (p == end) { return false; }

  auto key_end = p;

  if (*p++ != ':') { return false; }

  while (p < end && is_space_or_tab(*p)) {
    p++;
  }

  if (p <= end) {
    auto key_len = key_end - beg;
    if (!key_len) { return false; }

    auto key = std::string(beg, key_end);
    auto val = std::string(p, end);

    if (!detail::fields::is_field_value(val)) { return false; }

    if (case_ignore::equal(key, "Location") ||
        case_ignore::equal(key, "Referer")) {
      fn(key, val);
    } else {
      fn(key, decode_url(val, false));
    }

    return true;
  }

  return false;
}

inline bool read_headers(Stream &strm, Headers &headers) {
  const auto bufsiz = 2048;
  char buf[bufsiz];
  stream_line_reader line_reader(strm, buf, bufsiz);

  for (;;) {
    if (!line_reader.getline()) { return false; }

    // Check if the line ends with CRLF.
    auto line_terminator_len = 2;
    if (line_reader.end_with_crlf()) {
      // Blank line indicates end of headers.
      if (line_reader.size() == 2) { break; }
    } else {
#ifdef CPPHTTPLIB_ALLOW_LF_AS_LINE_TERMINATOR
      // Blank line indicates end of headers.
      if (line_reader.size() == 1) { break; }
      line_terminator_len = 1;
#else
      continue; // Skip invalid line.
#endif
    }

    if (line_reader.size() > CPPHTTPLIB_HEADER_MAX_LENGTH) { return false; }

    // Exclude line terminator
    auto end = line_reader.ptr() + line_reader.size() - line_terminator_len;

    if (!parse_header(line_reader.ptr(), end,
                      [&](const std::string &key, const std::string &val) {
                        headers.emplace(key, val);
                      })) {
      return false;
    }
  }

  return true;
}

inline bool read_content_with_length(Stream &strm, uint64_t len,
                                     Progress progress,
                                     ContentReceiverWithProgress out) {
  char buf[CPPHTTPLIB_RECV_BUFSIZ];

  uint64_t r = 0;
  while (r < len) {
    auto read_len = static_cast<size_t>(len - r);
    auto n = strm.read(buf, (std::min)(read_len, CPPHTTPLIB_RECV_BUFSIZ));
    if (n <= 0) { return false; }

    if (!out(buf, static_cast<size_t>(n), r, len)) { return false; }
    r += static_cast<uint64_t>(n);

    if (progress) {
      if (!progress(r, len)) { return false; }
    }
  }

  return true;
}

inline void skip_content_with_length(Stream &strm, uint64_t len) {
  char buf[CPPHTTPLIB_RECV_BUFSIZ];
  uint64_t r = 0;
  while (r < len) {
    auto read_len = static_cast<size_t>(len - r);
    auto n = strm.read(buf, (std::min)(read_len, CPPHTTPLIB_RECV_BUFSIZ));
    if (n <= 0) { return; }
    r += static_cast<uint64_t>(n);
  }
}

inline bool read_content_without_length(Stream &strm,
                                        ContentReceiverWithProgress out) {
  char buf[CPPHTTPLIB_RECV_BUFSIZ];
  uint64_t r = 0;
  for (;;) {
    auto n = strm.read(buf, CPPHTTPLIB_RECV_BUFSIZ);
    if (n == 0) { return true; }
    if (n < 0) { return false; }

    if (!out(buf, static_cast<size_t>(n), r, 0)) { return false; }
    r += static_cast<uint64_t>(n);
  }

  return true;
}

template <typename T>
inline bool read_content_chunked(Stream &strm, T &x,
                                 ContentReceiverWithProgress out) {
  const auto bufsiz = 16;
  char buf[bufsiz];

  stream_line_reader line_reader(strm, buf, bufsiz);

  if (!line_reader.getline()) { return false; }

  unsigned long chunk_len;
  while (true) {
    char *end_ptr;

    chunk_len = std::strtoul(line_reader.ptr(), &end_ptr, 16);

    if (end_ptr == line_reader.ptr()) { return false; }
    if (chunk_len == ULONG_MAX) { return false; }

    if (chunk_len == 0) { break; }

    if (!read_content_with_length(strm, chunk_len, nullptr, out)) {
      return false;
    }

    if (!line_reader.getline()) { return false; }

    if (strcmp(line_reader.ptr(), "\r\n") != 0) { return false; }

    if (!line_reader.getline()) { return false; }
  }

  assert(chunk_len == 0);

  // NOTE: In RFC 9112, '7.1 Chunked Transfer Coding' mentions "The chunked
  // transfer coding is complete when a chunk with a chunk-size of zero is
  // received, possibly followed by a trailer section, and finally terminated by
  // an empty line". https://www.rfc-editor.org/rfc/rfc9112.html#section-7.1
  //
  // In '7.1.3. Decoding Chunked', however, the pseudo-code in the section
  // does't care for the existence of the final CRLF. In other words, it seems
  // to be ok whether the final CRLF exists or not in the chunked data.
  // https://www.rfc-editor.org/rfc/rfc9112.html#section-7.1.3
  //
  // According to the reference code in RFC 9112, cpp-httplib now allows
  // chunked transfer coding data without the final CRLF.
  if (!line_reader.getline()) { return true; }

  while (strcmp(line_reader.ptr(), "\r\n") != 0) {
    if (line_reader.size() > CPPHTTPLIB_HEADER_MAX_LENGTH) { return false; }

    // Exclude line terminator
    constexpr auto line_terminator_len = 2;
    auto end = line_reader.ptr() + line_reader.size() - line_terminator_len;

    parse_header(line_reader.ptr(), end,
                 [&](const std::string &key, const std::string &val) {
                   x.headers.emplace(key, val);
                 });

    if (!line_reader.getline()) { return false; }
  }

  return true;
}

inline bool is_chunked_transfer_encoding(const Headers &headers) {
  return case_ignore::equal(
      get_header_value(headers, "Transfer-Encoding", "", 0), "chunked");
}

template <typename T, typename U>
bool prepare_content_receiver(T &x, int &status,
                              ContentReceiverWithProgress receiver,
                              bool decompress, U callback) {
  if (decompress) {
    std::string encoding = x.get_header_value("Content-Encoding");
    std::unique_ptr<decompressor> decompressor;

    if (encoding == "gzip" || encoding == "deflate") {
#ifdef CPPHTTPLIB_ZLIB_SUPPORT
      decompressor = detail::make_unique<gzip_decompressor>();
#else
      status = StatusCode::UnsupportedMediaType_415;
      return false;
#endif
    } else if (encoding.find("br") != std::string::npos) {
#ifdef CPPHTTPLIB_BROTLI_SUPPORT
      decompressor = detail::make_unique<brotli_decompressor>();
#else
      status = StatusCode::UnsupportedMediaType_415;
      return false;
#endif
    } else if (encoding == "zstd") {
#ifdef CPPHTTPLIB_ZSTD_SUPPORT
      decompressor = detail::make_unique<zstd_decompressor>();
#else
      status = StatusCode::UnsupportedMediaType_415;
      return false;
#endif
    }

    if (decompressor) {
      if (decompressor->is_valid()) {
        ContentReceiverWithProgress out = [&](const char *buf, size_t n,
                                              uint64_t off, uint64_t len) {
          return decompressor->decompress(buf, n,
                                          [&](const char *buf2, size_t n2) {
                                            return receiver(buf2, n2, off, len);
                                          });
        };
        return callback(std::move(out));
      } else {
        status = StatusCode::InternalServerError_500;
        return false;
      }
    }
  }

  ContentReceiverWithProgress out = [&](const char *buf, size_t n, uint64_t off,
                                        uint64_t len) {
    return receiver(buf, n, off, len);
  };
  return callback(std::move(out));
}

template <typename T>
bool read_content(Stream &strm, T &x, size_t payload_max_length, int &status,
                  Progress progress, ContentReceiverWithProgress receiver,
                  bool decompress) {
  return prepare_content_receiver(
      x, status, std::move(receiver), decompress,
      [&](const ContentReceiverWithProgress &out) {
        auto ret = true;
        auto exceed_payload_max_length = false;

        if (is_chunked_transfer_encoding(x.headers)) {
          ret = read_content_chunked(strm, x, out);
        } else if (!has_header(x.headers, "Content-Length")) {
          ret = read_content_without_length(strm, out);
        } else {
          auto is_invalid_value = false;
          auto len = get_header_value_u64(
              x.headers, "Content-Length",
              (std::numeric_limits<uint64_t>::max)(), 0, is_invalid_value);

          if (is_invalid_value) {
            ret = false;
          } else if (len > payload_max_length) {
            exceed_payload_max_length = true;
            skip_content_with_length(strm, len);
            ret = false;
          } else if (len > 0) {
            ret = read_content_with_length(strm, len, std::move(progress), out);
          }
        }

        if (!ret) {
          status = exceed_payload_max_length ? StatusCode::PayloadTooLarge_413
                                             : StatusCode::BadRequest_400;
        }
        return ret;
      });
}

inline ssize_t write_request_line(Stream &strm, const std::string &method,
                                  const std::string &path) {
  std::string s = method;
  s += " ";
  s += path;
  s += " HTTP/1.1\r\n";
  return strm.write(s.data(), s.size());
}

inline ssize_t write_response_line(Stream &strm, int status) {
  std::string s = "HTTP/1.1 ";
  s += std::to_string(status);
  s += " ";
  s += httplib::status_message(status);
  s += "\r\n";
  return strm.write(s.data(), s.size());
}

inline ssize_t write_headers(Stream &strm, const Headers &headers) {
  ssize_t write_len = 0;
  for (const auto &x : headers) {
    std::string s;
    s = x.first;
    s += ": ";
    s += x.second;
    s += "\r\n";

    auto len = strm.write(s.data(), s.size());
    if (len < 0) { return len; }
    write_len += len;
  }
  auto len = strm.write("\r\n");
  if (len < 0) { return len; }
  write_len += len;
  return write_len;
}

inline bool write_data(Stream &strm, const char *d, size_t l) {
  size_t offset = 0;
  while (offset < l) {
    auto length = strm.write(d + offset, l - offset);
    if (length < 0) { return false; }
    offset += static_cast<size_t>(length);
  }
  return true;
}

template <typename T>
inline bool write_content(Stream &strm, const ContentProvider &content_provider,
                          size_t offset, size_t length, T is_shutting_down,
                          Error &error) {
  size_t end_offset = offset + length;
  auto ok = true;
  DataSink data_sink;

  data_sink.write = [&](const char *d, size_t l) -> bool {
    if (ok) {
      if (write_data(strm, d, l)) {
        offset += l;
      } else {
        ok = false;
      }
    }
    return ok;
  };

  data_sink.is_writable = [&]() -> bool { return strm.wait_writable(); };

  while (offset < end_offset && !is_shutting_down()) {
    if (!strm.wait_writable()) {
      error = Error::Write;
      return false;
    } else if (!content_provider(offset, end_offset - offset, data_sink)) {
      error = Error::Canceled;
      return false;
    } else if (!ok) {
      error = Error::Write;
      return false;
    }
  }

  error = Error::Success;
  return true;
}

template <typename T>
inline bool write_content(Stream &strm, const ContentProvider &content_provider,
                          size_t offset, size_t length,
                          const T &is_shutting_down) {
  auto error = Error::Success;
  return write_content(strm, content_provider, offset, length, is_shutting_down,
                       error);
}

template <typename T>
inline bool
write_content_without_length(Stream &strm,
                             const ContentProvider &content_provider,
                             const T &is_shutting_down) {
  size_t offset = 0;
  auto data_available = true;
  auto ok = true;
  DataSink data_sink;

  data_sink.write = [&](const char *d, size_t l) -> bool {
    if (ok) {
      offset += l;
      if (!write_data(strm, d, l)) { ok = false; }
    }
    return ok;
  };

  data_sink.is_writable = [&]() -> bool { return strm.wait_writable(); };

  data_sink.done = [&](void) { data_available = false; };

  while (data_available && !is_shutting_down()) {
    if (!strm.wait_writable()) {
      return false;
    } else if (!content_provider(offset, 0, data_sink)) {
      return false;
    } else if (!ok) {
      return false;
    }
  }
  return true;
}

template <typename T, typename U>
inline bool
write_content_chunked(Stream &strm, const ContentProvider &content_provider,
                      const T &is_shutting_down, U &compressor, Error &error) {
  size_t offset = 0;
  auto data_available = true;
  auto ok = true;
  DataSink data_sink;

  data_sink.write = [&](const char *d, size_t l) -> bool {
    if (ok) {
      data_available = l > 0;
      offset += l;

      std::string payload;
      if (compressor.compress(d, l, false,
                              [&](const char *data, size_t data_len) {
                                payload.append(data, data_len);
                                return true;
                              })) {
        if (!payload.empty()) {
          // Emit chunked response header and footer for each chunk
          auto chunk =
              from_i_to_hex(payload.size()) + "\r\n" + payload + "\r\n";
          if (!write_data(strm, chunk.data(), chunk.size())) { ok = false; }
        }
      } else {
        ok = false;
      }
    }
    return ok;
  };

  data_sink.is_writable = [&]() -> bool { return strm.wait_writable(); };

  auto done_with_trailer = [&](const Headers *trailer) {
    if (!ok) { return; }

    data_available = false;

    std::string payload;
    if (!compressor.compress(nullptr, 0, true,
                             [&](const char *data, size_t data_len) {
                               payload.append(data, data_len);
                               return true;
                             })) {
      ok = false;
      return;
    }

    if (!payload.empty()) {
      // Emit chunked response header and footer for each chunk
      auto chunk = from_i_to_hex(payload.size()) + "\r\n" + payload + "\r\n";
      if (!write_data(strm, chunk.data(), chunk.size())) {
        ok = false;
        return;
      }
    }

    constexpr const char done_marker[] = "0\r\n";
    if (!write_data(strm, done_marker, str_len(done_marker))) { ok = false; }

    // Trailer
    if (trailer) {
      for (const auto &kv : *trailer) {
        std::string field_line = kv.first + ": " + kv.second + "\r\n";
        if (!write_data(strm, field_line.data(), field_line.size())) {
          ok = false;
        }
      }
    }

    constexpr const char crlf[] = "\r\n";
    if (!write_data(strm, crlf, str_len(crlf))) { ok = false; }
  };

  data_sink.done = [&](void) { done_with_trailer(nullptr); };

  data_sink.done_with_trailer = [&](const Headers &trailer) {
    done_with_trailer(&trailer);
  };

  while (data_available && !is_shutting_down()) {
    if (!strm.wait_writable()) {
      error = Error::Write;
      return false;
    } else if (!content_provider(offset, 0, data_sink)) {
      error = Error::Canceled;
      return false;
    } else if (!ok) {
      error = Error::Write;
      return false;
    }
  }

  error = Error::Success;
  return true;
}

template <typename T, typename U>
inline bool write_content_chunked(Stream &strm,
                                  const ContentProvider &content_provider,
                                  const T &is_shutting_down, U &compressor) {
  auto error = Error::Success;
  return write_content_chunked(strm, content_provider, is_shutting_down,
                               compressor, error);
}

template <typename T>
inline bool redirect(T &cli, Request &req, Response &res,
                     const std::string &path, const std::string &location,
                     Error &error) {
  Request new_req = req;
  new_req.path = path;
  new_req.redirect_count_ -= 1;

  if (res.status == StatusCode::SeeOther_303 &&
      (req.method != "GET" && req.method != "HEAD")) {
    new_req.method = "GET";
    new_req.body.clear();
    new_req.headers.clear();
  }

  Response new_res;

  auto ret = cli.send(new_req, new_res, error);
  if (ret) {
    req = new_req;
    res = new_res;

    if (res.location.empty()) { res.location = location; }
  }
  return ret;
}

inline std::string params_to_query_str(const Params &params) {
  std::string query;

  for (auto it = params.begin(); it != params.end(); ++it) {
    if (it != params.begin()) { query += "&"; }
    query += it->first;
    query += "=";
    query += encode_query_param(it->second);
  }
  return query;
}

inline void parse_query_text(const char *data, std::size_t size,
                             Params &params) {
  std::set<std::string> cache;
  split(data, data + size, '&', [&](const char *b, const char *e) {
    std::string kv(b, e);
    if (cache.find(kv) != cache.end()) { return; }
    cache.insert(std::move(kv));

    std::string key;
    std::string val;
    divide(b, static_cast<std::size_t>(e - b), '=',
           [&](const char *lhs_data, std::size_t lhs_size, const char *rhs_data,
               std::size_t rhs_size) {
             key.assign(lhs_data, lhs_size);
             val.assign(rhs_data, rhs_size);
           });

    if (!key.empty()) {
      params.emplace(decode_url(key, true), decode_url(val, true));
    }
  });
}

inline void parse_query_text(const std::string &s, Params &params) {
  parse_query_text(s.data(), s.size(), params);
}

inline bool parse_multipart_boundary(const std::string &content_type,
                                     std::string &boundary) {
  auto boundary_keyword = "boundary=";
  auto pos = content_type.find(boundary_keyword);
  if (pos == std::string::npos) { return false; }
  auto end = content_type.find(';', pos);
  auto beg = pos + strlen(boundary_keyword);
  boundary = trim_double_quotes_copy(content_type.substr(beg, end - beg));
  return !boundary.empty();
}

inline void parse_disposition_params(const std::string &s, Params &params) {
  std::set<std::string> cache;
  split(s.data(), s.data() + s.size(), ';', [&](const char *b, const char *e) {
    std::string kv(b, e);
    if (cache.find(kv) != cache.end()) { return; }
    cache.insert(kv);

    std::string key;
    std::string val;
    split(b, e, '=', [&](const char *b2, const char *e2) {
      if (key.empty()) {
        key.assign(b2, e2);
      } else {
        val.assign(b2, e2);
      }
    });

    if (!key.empty()) {
      params.emplace(trim_double_quotes_copy((key)),
                     trim_double_quotes_copy((val)));
    }
  });
}

#ifdef CPPHTTPLIB_NO_EXCEPTIONS
inline bool parse_range_header(const std::string &s, Ranges &ranges) {
#else
inline bool parse_range_header(const std::string &s, Ranges &ranges) try {
#endif
  auto is_valid = [](const std::string &str) {
    return std::all_of(str.cbegin(), str.cend(),
                       [](unsigned char c) { return std::isdigit(c); });
  };

  if (s.size() > 7 && s.compare(0, 6, "bytes=") == 0) {
    const auto pos = static_cast<size_t>(6);
    const auto len = static_cast<size_t>(s.size() - 6);
    auto all_valid_ranges = true;
    split(&s[pos], &s[pos + len], ',', [&](const char *b, const char *e) {
      if (!all_valid_ranges) { return; }

      const auto it = std::find(b, e, '-');
      if (it == e) {
        all_valid_ranges = false;
        return;
      }

      const auto lhs = std::string(b, it);
      const auto rhs = std::string(it + 1, e);
      if (!is_valid(lhs) || !is_valid(rhs)) {
        all_valid_ranges = false;
        return;
      }

      const auto first =
          static_cast<ssize_t>(lhs.empty() ? -1 : std::stoll(lhs));
      const auto last =
          static_cast<ssize_t>(rhs.empty() ? -1 : std::stoll(rhs));
      if ((first == -1 && last == -1) ||
          (first != -1 && last != -1 && first > last)) {
        all_valid_ranges = false;
        return;
      }

      ranges.emplace_back(first, last);
    });
    return all_valid_ranges && !ranges.empty();
  }
  return false;
#ifdef CPPHTTPLIB_NO_EXCEPTIONS
}
#else
} catch (...) { return false; }
#endif

class MultipartFormDataParser {
public:
  MultipartFormDataParser() = default;

  void set_boundary(std::string &&boundary) {
    boundary_ = boundary;
    dash_boundary_crlf_ = dash_ + boundary_ + crlf_;
    crlf_dash_boundary_ = crlf_ + dash_ + boundary_;
  }

  bool is_valid() const { return is_valid_; }

  bool parse(const char *buf, size_t n, const ContentReceiver &content_callback,
             const MultipartContentHeader &header_callback) {

    buf_append(buf, n);

    while (buf_size() > 0) {
      switch (state_) {
      case 0: { // Initial boundary
        buf_erase(buf_find(dash_boundary_crlf_));
        if (dash_boundary_crlf_.size() > buf_size()) { return true; }
        if (!buf_start_with(dash_boundary_crlf_)) { return false; }
        buf_erase(dash_boundary_crlf_.size());
        state_ = 1;
        break;
      }
      case 1: { // New entry
        clear_file_info();
        state_ = 2;
        break;
      }
      case 2: { // Headers
        auto pos = buf_find(crlf_);
        if (pos > CPPHTTPLIB_HEADER_MAX_LENGTH) { return false; }
        while (pos < buf_size()) {
          // Empty line
          if (pos == 0) {
            if (!header_callback(file_)) {
              is_valid_ = false;
              return false;
            }
            buf_erase(crlf_.size());
            state_ = 3;
            break;
          }

          const auto header = buf_head(pos);

          if (!parse_header(header.data(), header.data() + header.size(),
                            [&](const std::string &, const std::string &) {})) {
            is_valid_ = false;
            return false;
          }

          constexpr const char header_content_type[] = "Content-Type:";

          if (start_with_case_ignore(header, header_content_type)) {
            file_.content_type =
                trim_copy(header.substr(str_len(header_content_type)));
          } else {
            thread_local const std::regex re_content_disposition(
                R"~(^Content-Disposition:\s*form-data;\s*(.*)$)~",
                std::regex_constants::icase);

            std::smatch m;
            if (std::regex_match(header, m, re_content_disposition)) {
              Params params;
              parse_disposition_params(m[1], params);

              auto it = params.find("name");
              if (it != params.end()) {
                file_.name = it->second;
              } else {
                is_valid_ = false;
                return false;
              }

              it = params.find("filename");
              if (it != params.end()) { file_.filename = it->second; }

              it = params.find("filename*");
              if (it != params.end()) {
                // Only allow UTF-8 encoding...
                thread_local const std::regex re_rfc5987_encoding(
                    R"~(^UTF-8''(.+?)$)~", std::regex_constants::icase);

                std::smatch m2;
                if (std::regex_match(it->second, m2, re_rfc5987_encoding)) {
                  file_.filename = decode_url(m2[1], false); // override...
                } else {
                  is_valid_ = false;
                  return false;
                }
              }
            }
          }
          buf_erase(pos + crlf_.size());
          pos = buf_find(crlf_);
        }
        if (state_ != 3) { return true; }
        break;
      }
      case 3: { // Body
        if (crlf_dash_boundary_.size() > buf_size()) { return true; }
        auto pos = buf_find(crlf_dash_boundary_);
        if (pos < buf_size()) {
          if (!content_callback(buf_data(), pos)) {
            is_valid_ = false;
            return false;
          }
          buf_erase(pos + crlf_dash_boundary_.size());
          state_ = 4;
        } else {
          auto len = buf_size() - crlf_dash_boundary_.size();
          if (len > 0) {
            if (!content_callback(buf_data(), len)) {
              is_valid_ = false;
              return false;
            }
            buf_erase(len);
          }
          return true;
        }
        break;
      }
      case 4: { // Boundary
        if (crlf_.size() > buf_size()) { return true; }
        if (buf_start_with(crlf_)) {
          buf_erase(crlf_.size());
          state_ = 1;
        } else {
          if (dash_.size() > buf_size()) { return true; }
          if (buf_start_with(dash_)) {
            buf_erase(dash_.size());
            is_valid_ = true;
            buf_erase(buf_size()); // Remove epilogue
          } else {
            return true;
          }
        }
        break;
      }
      }
    }

    return true;
  }

private:
  void clear_file_info() {
    file_.name.clear();
    file_.filename.clear();
    file_.content_type.clear();
  }

  bool start_with_case_ignore(const std::string &a, const char *b) const {
    const auto b_len = strlen(b);
    if (a.size() < b_len) { return false; }
    for (size_t i = 0; i < b_len; i++) {
      if (case_ignore::to_lower(a[i]) != case_ignore::to_lower(b[i])) {
        return false;
      }
    }
    return true;
  }

  const std::string dash_ = "--";
  const std::string crlf_ = "\r\n";
  std::string boundary_;
  std::string dash_boundary_crlf_;
  std::string crlf_dash_boundary_;

  size_t state_ = 0;
  bool is_valid_ = false;
  MultipartFormData file_;

  // Buffer
  bool start_with(const std::string &a, size_t spos, size_t epos,
                  const std::string &b) const {
    if (epos - spos < b.size()) { return false; }
    for (size_t i = 0; i < b.size(); i++) {
      if (a[i + spos] != b[i]) { return false; }
    }
    return true;
  }

  size_t buf_size() const { return buf_epos_ - buf_spos_; }

  const char *buf_data() const { return &buf_[buf_spos_]; }

  std::string buf_head(size_t l) const { return buf_.substr(buf_spos_, l); }

  bool buf_start_with(const std::string &s) const {
    return start_with(buf_, buf_spos_, buf_epos_, s);
  }

  size_t buf_find(const std::string &s) const {
    auto c = s.front();

    size_t off = buf_spos_;
    while (off < buf_epos_) {
      auto pos = off;
      while (true) {
        if (pos == buf_epos_) { return buf_size(); }
        if (buf_[pos] == c) { break; }
        pos++;
      }

      auto remaining_size = buf_epos_ - pos;
      if (s.size() > remaining_size) { return buf_size(); }

      if (start_with(buf_, pos, buf_epos_, s)) { return pos - buf_spos_; }

      off = pos + 1;
    }

    return buf_size();
  }

  void buf_append(const char *data, size_t n) {
    auto remaining_size = buf_size();
    if (remaining_size > 0 && buf_spos_ > 0) {
      for (size_t i = 0; i < remaining_size; i++) {
        buf_[i] = buf_[buf_spos_ + i];
      }
    }
    buf_spos_ = 0;
    buf_epos_ = remaining_size;

    if (remaining_size + n > buf_.size()) { buf_.resize(remaining_size + n); }

    for (size_t i = 0; i < n; i++) {
      buf_[buf_epos_ + i] = data[i];
    }
    buf_epos_ += n;
  }

  void buf_erase(size_t size) { buf_spos_ += size; }

  std::string buf_;
  size_t buf_spos_ = 0;
  size_t buf_epos_ = 0;
};

inline std::string random_string(size_t length) {
  constexpr const char data[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  thread_local auto engine([]() {
    // std::random_device might actually be deterministic on some
    // platforms, but due to lack of support in the c++ standard library,
    // doing better requires either some ugly hacks or breaking portability.
    std::random_device seed_gen;
    // Request 128 bits of entropy for initialization
    std::seed_seq seed_sequence{seed_gen(), seed_gen(), seed_gen(), seed_gen()};
    return std::mt19937(seed_sequence);
  }());

  std::string result;
  for (size_t i = 0; i < length; i++) {
    result += data[engine() % (sizeof(data) - 1)];
  }
  return result;
}

inline std::string make_multipart_data_boundary() {
  return "--cpp-httplib-multipart-data-" + detail::random_string(16);
}

inline bool is_multipart_boundary_chars_valid(const std::string &boundary) {
  auto valid = true;
  for (size_t i = 0; i < boundary.size(); i++) {
    auto c = boundary[i];
    if (!std::isalnum(c) && c != '-' && c != '_') {
      valid = false;
      break;
    }
  }
  return valid;
}

template <typename T>
inline std::string
serialize_multipart_formdata_item_begin(const T &item,
                                        const std::string &boundary) {
  std::string body = "--" + boundary + "\r\n";
  body += "Content-Disposition: form-data; name=\"" + item.name + "\"";
  if (!item.filename.empty()) {
    body += "; filename=\"" + item.filename + "\"";
  }
  body += "\r\n";
  if (!item.content_type.empty()) {
    body += "Content-Type: " + item.content_type + "\r\n";
  }
  body += "\r\n";

  return body;
}

inline std::string serialize_multipart_formdata_item_end() { return "\r\n"; }

inline std::string
serialize_multipart_formdata_finish(const std::string &boundary) {
  return "--" + boundary + "--\r\n";
}

inline std::string
serialize_multipart_formdata_get_content_type(const std::string &boundary) {
  return "multipart/form-data; boundary=" + boundary;
}

inline std::string
serialize_multipart_formdata(const MultipartFormDataItems &items,
                             const std::string &boundary, bool finish = true) {
  std::string body;

  for (const auto &item : items) {
    body += serialize_multipart_formdata_item_begin(item, boundary);
    body += item.content + serialize_multipart_formdata_item_end();
  }

  if (finish) { body += serialize_multipart_formdata_finish(boundary); }

  return body;
}

inline bool range_error(Request &req, Response &res) {
  if (!req.ranges.empty() && 200 <= res.status && res.status < 300) {
    ssize_t content_len = static_cast<ssize_t>(
        res.content_length_ ? res.content_length_ : res.body.size());

    ssize_t prev_first_pos = -1;
    ssize_t prev_last_pos = -1;
    size_t overwrapping_count = 0;

    // NOTE: The following Range check is based on '14.2. Range' in RFC 9110
    // 'HTTP Semantics' to avoid potential denial-of-service attacks.
    // https://www.rfc-editor.org/rfc/rfc9110#section-14.2

    // Too many ranges
    if (req.ranges.size() > CPPHTTPLIB_RANGE_MAX_COUNT) { return true; }

    for (auto &r : req.ranges) {
      auto &first_pos = r.first;
      auto &last_pos = r.second;

      if (first_pos == -1 && last_pos == -1) {
        first_pos = 0;
        last_pos = content_len;
      }

      if (first_pos == -1) {
        first_pos = content_len - last_pos;
        last_pos = content_len - 1;
      }

      // NOTE: RFC-9110 '14.1.2. Byte Ranges':
      // A client can limit the number of bytes requested without knowing the
      // size of the selected representation. If the last-pos value is absent,
      // or if the value is greater than or equal to the current length of the
      // representation data, the byte range is interpreted as the remainder of
      // the representation (i.e., the server replaces the value of last-pos
      // with a value that is one less than the current length of the selected
      // representation).
      // https://www.rfc-editor.org/rfc/rfc9110.html#section-14.1.2-6
      if (last_pos == -1 || last_pos >= content_len) {
        last_pos = content_len - 1;
      }

      // Range must be within content length
      if (!(0 <= first_pos && first_pos <= last_pos &&
            last_pos <= content_len - 1)) {
        return true;
      }

      // Ranges must be in ascending order
      if (first_pos <= prev_first_pos) { return true; }

      // Request must not have more than two overlapping ranges
      if (first_pos <= prev_last_pos) {
        overwrapping_count++;
        if (overwrapping_count > 2) { return true; }
      }

      prev_first_pos = (std::max)(prev_first_pos, first_pos);
      prev_last_pos = (std::max)(prev_last_pos, last_pos);
    }
  }

  return false;
}

inline std::pair<size_t, size_t>
get_range_offset_and_length(Range r, size_t content_length) {
  assert(r.first != -1 && r.second != -1);
  assert(0 <= r.first && r.first < static_cast<ssize_t>(content_length));
  assert(r.first <= r.second &&
         r.second < static_cast<ssize_t>(content_length));
  (void)(content_length);
  return std::make_pair(r.first, static_cast<size_t>(r.second - r.first) + 1);
}

inline std::string make_content_range_header_field(
    const std::pair<size_t, size_t> &offset_and_length, size_t content_length) {
  auto st = offset_and_length.first;
  auto ed = st + offset_and_length.second - 1;

  std::string field = "bytes ";
  field += std::to_string(st);
  field += "-";
  field += std::to_string(ed);
  field += "/";
  field += std::to_string(content_length);
  return field;
}

template <typename SToken, typename CToken, typename Content>
bool process_multipart_ranges_data(const Request &req,
                                   const std::string &boundary,
                                   const std::string &content_type,
                                   size_t content_length, SToken stoken,
                                   CToken ctoken, Content content) {
  for (size_t i = 0; i < req.ranges.size(); i++) {
    ctoken("--");
    stoken(boundary);
    ctoken("\r\n");
    if (!content_type.empty()) {
      ctoken("Content-Type: ");
      stoken(content_type);
      ctoken("\r\n");
    }

    auto offset_and_length =
        get_range_offset_and_length(req.ranges[i], content_length);

    ctoken("Content-Range: ");
    stoken(make_content_range_header_field(offset_and_length, content_length));
    ctoken("\r\n");
    ctoken("\r\n");

    if (!content(offset_and_length.first, offset_and_length.second)) {
      return false;
    }
    ctoken("\r\n");
  }

  ctoken("--");
  stoken(boundary);
  ctoken("--");

  return true;
}

inline void make_multipart_ranges_data(const Request &req, Response &res,
                                       const std::string &boundary,
                                       const std::string &content_type,
                                       size_t content_length,
                                       std::string &data) {
  process_multipart_ranges_data(
      req, boundary, content_type, content_length,
      [&](const std::string &token) { data += token; },
      [&](const std::string &token) { data += token; },
      [&](size_t offset, size_t length) {
        assert(offset + length <= content_length);
        data += res.body.substr(offset, length);
        return true;
      });
}

inline size_t get_multipart_ranges_data_length(const Request &req,
                                               const std::string &boundary,
                                               const std::string &content_type,
                                               size_t content_length) {
  size_t data_length = 0;

  process_multipart_ranges_data(
      req, boundary, content_type, content_length,
      [&](const std::string &token) { data_length += token.size(); },
      [&](const std::string &token) { data_length += token.size(); },
      [&](size_t /*offset*/, size_t length) {
        data_length += length;
        return true;
      });

  return data_length;
}

template <typename T>
inline bool
write_multipart_ranges_data(Stream &strm, const Request &req, Response &res,
                            const std::string &boundary,
                            const std::string &content_type,
                            size_t content_length, const T &is_shutting_down) {
  return process_multipart_ranges_data(
      req, boundary, content_type, content_length,
      [&](const std::string &token) { strm.write(token); },
      [&](const std::string &token) { strm.write(token); },
      [&](size_t offset, size_t length) {
        return write_content(strm, res.content_provider_, offset, length,
                             is_shutting_down);
      });
}

inline bool expect_content(const Request &req) {
  if (req.method == "POST" || req.method == "PUT" || req.method == "PATCH" ||
      req.method == "DELETE") {
    return true;
  }
  if (req.has_header("Content-Length") &&
      req.get_header_value_u64("Content-Length") > 0) {
    return true;
  }
  if (is_chunked_transfer_encoding(req.headers)) { return true; }
  return false;
}

inline bool has_crlf(const std::string &s) {
  auto p = s.c_str();
  while (*p) {
    if (*p == '\r' || *p == '\n') { return true; }
    p++;
  }
  return false;
}

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
inline std::string message_digest(const std::string &s, const EVP_MD *algo) {
  auto context = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>(
      EVP_MD_CTX_new(), EVP_MD_CTX_free);

  unsigned int hash_length = 0;
  unsigned char hash[EVP_MAX_MD_SIZE];

  EVP_DigestInit_ex(context.get(), algo, nullptr);
  EVP_DigestUpdate(context.get(), s.c_str(), s.size());
  EVP_DigestFinal_ex(context.get(), hash, &hash_length);

  std::stringstream ss;
  for (auto i = 0u; i < hash_length; ++i) {
    ss << std::hex << std::setw(2) << std::setfill('0')
       << static_cast<unsigned int>(hash[i]);
  }

  return ss.str();
}

inline std::string MD5(const std::string &s) {
  return message_digest(s, EVP_md5());
}

inline std::string SHA_256(const std::string &s) {
  return message_digest(s, EVP_sha256());
}

inline std::string SHA_512(const std::string &s) {
  return message_digest(s, EVP_sha512());
}

inline std::pair<std::string, std::string> make_digest_authentication_header(
    const Request &req, const std::map<std::string, std::string> &auth,
    size_t cnonce_count, const std::string &cnonce, const std::string &username,
    const std::string &password, bool is_proxy = false) {
  std::string nc;
  {
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(8) << std::hex << cnonce_count;
    nc = ss.str();
  }

  std::string qop;
  if (auth.find("qop") != auth.end()) {
    qop = auth.at("qop");
    if (qop.find("auth-int") != std::string::npos) {
      qop = "auth-int";
    } else if (qop.find("auth") != std::string::npos) {
      qop = "auth";
    } else {
      qop.clear();
    }
  }

  std::string algo = "MD5";
  if (auth.find("algorithm") != auth.end()) { algo = auth.at("algorithm"); }

  std::string response;
  {
    auto H = algo == "SHA-256"   ? detail::SHA_256
             : algo == "SHA-512" ? detail::SHA_512
                                 : detail::MD5;

    auto A1 = username + ":" + auth.at("realm") + ":" + password;

    auto A2 = req.method + ":" + req.path;
    if (qop == "auth-int") { A2 += ":" + H(req.body); }

    if (qop.empty()) {
      response = H(H(A1) + ":" + auth.at("nonce") + ":" + H(A2));
    } else {
      response = H(H(A1) + ":" + auth.at("nonce") + ":" + nc + ":" + cnonce +
                   ":" + qop + ":" + H(A2));
    }
  }

  auto opaque = (auth.find("opaque") != auth.end()) ? auth.at("opaque") : "";

  auto field = "Digest username=\"" + username + "\", realm=\"" +
               auth.at("realm") + "\", nonce=\"" + auth.at("nonce") +
               "\", uri=\"" + req.path + "\", algorithm=" + algo +
               (qop.empty() ? ", response=\""
                            : ", qop=" + qop + ", nc=" + nc + ", cnonce=\"" +
                                  cnonce + "\", response=\"") +
               response + "\"" +
               (opaque.empty() ? "" : ", opaque=\"" + opaque + "\"");

  auto key = is_proxy ? "Proxy-Authorization" : "Authorization";
  return std::make_pair(key, field);
}

inline bool is_ssl_peer_could_be_closed(SSL *ssl, socket_t sock) {
  detail::set_nonblocking(sock, true);
  auto se = detail::scope_exit([&]() { detail::set_nonblocking(sock, false); });

  char buf[1];
  return !SSL_peek(ssl, buf, 1) &&
         SSL_get_error(ssl, 0) == SSL_ERROR_ZERO_RETURN;
}

#ifdef _WIN32
// NOTE: This code came up with the following stackoverflow post:
// https://stackoverflow.com/questions/9507184/can-openssl-on-windows-use-the-system-certificate-store
inline bool load_system_certs_on_windows(X509_STORE *store) {
  auto hStore = CertOpenSystemStoreW((HCRYPTPROV_LEGACY)NULL, L"ROOT");
  if (!hStore) { return false; }

  auto result = false;
  PCCERT_CONTEXT pContext = NULL;
  while ((pContext = CertEnumCertificatesInStore(hStore, pContext)) !=
         nullptr) {
    auto encoded_cert =
        static_cast<const unsigned char *>(pContext->pbCertEncoded);

    auto x509 = d2i_X509(NULL, &encoded_cert, pContext->cbCertEncoded);
    if (x509) {
      X509_STORE_add_cert(store, x509);
      X509_free(x509);
      result = true;
    }
  }

  CertFreeCertificateContext(pContext);
  CertCloseStore(hStore, 0);

  return result;
}
#elif defined(CPPHTTPLIB_USE_CERTS_FROM_MACOSX_KEYCHAIN) && defined(__APPLE__)
#if TARGET_OS_OSX
template <typename T>
using CFObjectPtr =
    std::unique_ptr<typename std::remove_pointer<T>::type, void (*)(CFTypeRef)>;

inline void cf_object_ptr_deleter(CFTypeRef obj) {
  if (obj) { CFRelease(obj); }
}

inline bool retrieve_certs_from_keychain(CFObjectPtr<CFArrayRef> &certs) {
  CFStringRef keys[] = {kSecClass, kSecMatchLimit, kSecReturnRef};
  CFTypeRef values[] = {kSecClassCertificate, kSecMatchLimitAll,
                        kCFBooleanTrue};

  CFObjectPtr<CFDictionaryRef> query(
      CFDictionaryCreate(nullptr, reinterpret_cast<const void **>(keys), values,
                         sizeof(keys) / sizeof(keys[0]),
                         &kCFTypeDictionaryKeyCallBacks,
                         &kCFTypeDictionaryValueCallBacks),
      cf_object_ptr_deleter);

  if (!query) { return false; }

  CFTypeRef security_items = nullptr;
  if (SecItemCopyMatching(query.get(), &security_items) != errSecSuccess ||
      CFArrayGetTypeID() != CFGetTypeID(security_items)) {
    return false;
  }

  certs.reset(reinterpret_cast<CFArrayRef>(security_items));
  return true;
}

inline bool retrieve_root_certs_from_keychain(CFObjectPtr<CFArrayRef> &certs) {
  CFArrayRef root_security_items = nullptr;
  if (SecTrustCopyAnchorCertificates(&root_security_items) != errSecSuccess) {
    return false;
  }

  certs.reset(root_security_items);
  return true;
}

inline bool add_certs_to_x509_store(CFArrayRef certs, X509_STORE *store) {
  auto result = false;
  for (auto i = 0; i < CFArrayGetCount(certs); ++i) {
    const auto cert = reinterpret_cast<const __SecCertificate *>(
        CFArrayGetValueAtIndex(certs, i));

    if (SecCertificateGetTypeID() != CFGetTypeID(cert)) { continue; }

    CFDataRef cert_data = nullptr;
    if (SecItemExport(cert, kSecFormatX509Cert, 0, nullptr, &cert_data) !=
        errSecSuccess) {
      continue;
    }

    CFObjectPtr<CFDataRef> cert_data_ptr(cert_data, cf_object_ptr_deleter);

    auto encoded_cert = static_cast<const unsigned char *>(
        CFDataGetBytePtr(cert_data_ptr.get()));

    auto x509 =
        d2i_X509(NULL, &encoded_cert, CFDataGetLength(cert_data_ptr.get()));

    if (x509) {
      X509_STORE_add_cert(store, x509);
      X509_free(x509);
      result = true;
    }
  }

  return result;
}

inline bool load_system_certs_on_macos(X509_STORE *store) {
  auto result = false;
  CFObjectPtr<CFArrayRef> certs(nullptr, cf_object_ptr_deleter);
  if (retrieve_certs_from_keychain(certs) && certs) {
    result = add_certs_to_x509_store(certs.get(), store);
  }

  if (retrieve_root_certs_from_keychain(certs) && certs) {
    result = add_certs_to_x509_store(certs.get(), store) || result;
  }

  return result;
}
#endif // TARGET_OS_OSX
#endif // _WIN32
#endif // CPPHTTPLIB_OPENSSL_SUPPORT

#ifdef _WIN32
class WSInit {
public:
  WSInit() {
    WSADATA wsaData;
    if (WSAStartup(0x0002, &wsaData) == 0) is_valid_ = true;
  }

  ~WSInit() {
    if (is_valid_) WSACleanup();
  }

  bool is_valid_ = false;
};

static WSInit wsinit_;
#endif

inline bool parse_www_authenticate(const Response &res,
                                   std::map<std::string, std::string> &auth,
                                   bool is_proxy) {
  auto auth_key = is_proxy ? "Proxy-Authenticate" : "WWW-Authenticate";
  if (res.has_header(auth_key)) {
    thread_local auto re =
        std::regex(R"~((?:(?:,\s*)?(.+?)=(?:"(.*?)"|([^,]*))))~");
    auto s = res.get_header_value(auth_key);
    auto pos = s.find(' ');
    if (pos != std::string::npos) {
      auto type = s.substr(0, pos);
      if (type == "Basic") {
        return false;
      } else if (type == "Digest") {
        s = s.substr(pos + 1);
        auto beg = std::sregex_iterator(s.begin(), s.end(), re);
        for (auto i = beg; i != std::sregex_iterator(); ++i) {
          const auto &m = *i;
          auto key = s.substr(static_cast<size_t>(m.position(1)),
                              static_cast<size_t>(m.length(1)));
          auto val = m.length(2) > 0
                         ? s.substr(static_cast<size_t>(m.position(2)),
                                    static_cast<size_t>(m.length(2)))
                         : s.substr(static_cast<size_t>(m.position(3)),
                                    static_cast<size_t>(m.length(3)));
          auth[key] = val;
        }
        return true;
      }
    }
  }
  return false;
}

class ContentProviderAdapter {
public:
  explicit ContentProviderAdapter(
      ContentProviderWithoutLength &&content_provider)
      : content_provider_(content_provider) {}

  bool operator()(size_t offset, size_t, DataSink &sink) {
    return content_provider_(offset, sink);
  }

private:
  ContentProviderWithoutLength content_provider_;
};

} // namespace detail

inline std::string hosted_at(const std::string &hostname) {
  std::vector<std::string> addrs;
  hosted_at(hostname, addrs);
  if (addrs.empty()) { return std::string(); }
  return addrs[0];
}

inline void hosted_at(const std::string &hostname,
                      std::vector<std::string> &addrs) {
  struct addrinfo hints;
  struct addrinfo *result;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;

  if (getaddrinfo(hostname.c_str(), nullptr, &hints, &result)) {
#if defined __linux__ && !defined __ANDROID__
    res_init();
#endif
    return;
  }
  auto se = detail::scope_exit([&] { freeaddrinfo(result); });

  for (auto rp = result; rp; rp = rp->ai_next) {
    const auto &addr =
        *reinterpret_cast<struct sockaddr_storage *>(rp->ai_addr);
    std::string ip;
    auto dummy = -1;
    if (detail::get_ip_and_port(addr, sizeof(struct sockaddr_storage), ip,
                                dummy)) {
      addrs.push_back(ip);
    }
  }
}

inline std::string append_query_params(const std::string &path,
                                       const Params &params) {
  std::string path_with_query = path;
  thread_local const std::regex re("[^?]+\\?.*");
  auto delm = std::regex_match(path, re) ? '&' : '?';
  path_with_query += delm + detail::params_to_query_str(params);
  return path_with_query;
}

// Header utilities
inline std::pair<std::string, std::string>
make_range_header(const Ranges &ranges) {
  std::string field = "bytes=";
  auto i = 0;
  for (const auto &r : ranges) {
    if (i != 0) { field += ", "; }
    if (r.first != -1) { field += std::to_string(r.first); }
    field += '-';
    if (r.second != -1) { field += std::to_string(r.second); }
    i++;
  }
  return std::make_pair("Range", std::move(field));
}

inline std::pair<std::string, std::string>
make_basic_authentication_header(const std::string &username,
                                 const std::string &password, bool is_proxy) {
  auto field = "Basic " + detail::base64_encode(username + ":" + password);
  auto key = is_proxy ? "Proxy-Authorization" : "Authorization";
  return std::make_pair(key, std::move(field));
}

inline std::pair<std::string, std::string>
make_bearer_token_authentication_header(const std::string &token,
                                        bool is_proxy = false) {
  auto field = "Bearer " + token;
  auto key = is_proxy ? "Proxy-Authorization" : "Authorization";
  return std::make_pair(key, std::move(field));
}

// Request implementation
inline bool Request::has_header(const std::string &key) const {
  return detail::has_header(headers, key);
}

inline std::string Request::get_header_value(const std::string &key,
                                             const char *def, size_t id) const {
  return detail::get_header_value(headers, key, def, id);
}

inline size_t Request::get_header_value_count(const std::string &key) const {
  auto r = headers.equal_range(key);
  return static_cast<size_t>(std::distance(r.first, r.second));
}

inline void Request::set_header(const std::string &key,
                                const std::string &val) {
  if (detail::fields::is_field_name(key) &&
      detail::fields::is_field_value(val)) {
    headers.emplace(key, val);
  }
}

inline bool Request::has_param(const std::string &key) const {
  return params.find(key) != params.end();
}

inline std::string Request::get_param_value(const std::string &key,
                                            size_t id) const {
  auto rng = params.equal_range(key);
  auto it = rng.first;
  std::advance(it, static_cast<ssize_t>(id));
  if (it != rng.second) { return it->second; }
  return std::string();
}

inline size_t Request::get_param_value_count(const std::string &key) const {
  auto r = params.equal_range(key);
  return static_cast<size_t>(std::distance(r.first, r.second));
}

inline bool Request::is_multipart_form_data() const {
  const auto &content_type = get_header_value("Content-Type");
  return !content_type.rfind("multipart/form-data", 0);
}

inline bool Request::has_file(const std::string &key) const {
  return files.find(key) != files.end();
}

inline MultipartFormData Request::get_file_value(const std::string &key) const {
  auto it = files.find(key);
  if (it != files.end()) { return it->second; }
  return MultipartFormData();
}

inline std::vector<MultipartFormData>
Request::get_file_values(const std::string &key) const {
  std::vector<MultipartFormData> values;
  auto rng = files.equal_range(key);
  for (auto it = rng.first; it != rng.second; it++) {
    values.push_back(it->second);
  }
  return values;
}

// Response implementation
inline bool Response::has_header(const std::string &key) const {
  return headers.find(key) != headers.end();
}

inline std::string Response::get_header_value(const std::string &key,
                                              const char *def,
                                              size_t id) const {
  return detail::get_header_value(headers, key, def, id);
}

inline size_t Response::get_header_value_count(const std::string &key) const {
  auto r = headers.equal_range(key);
  return static_cast<size_t>(std::distance(r.first, r.second));
}

inline void Response::set_header(const std::string &key,
                                 const std::string &val) {
  if (detail::fields::is_field_name(key) &&
      detail::fields::is_field_value(val)) {
    headers.emplace(key, val);
  }
}

inline void Response::set_redirect(const std::string &url, int stat) {
  if (detail::fields::is_field_value(url)) {
    set_header("Location", url);
    if (300 <= stat && stat < 400) {
      this->status = stat;
    } else {
      this->status = StatusCode::Found_302;
    }
  }
}

inline void Response::set_content(const char *s, size_t n,
                                  const std::string &content_type) {
  body.assign(s, n);

  auto rng = headers.equal_range("Content-Type");
  headers.erase(rng.first, rng.second);
  set_header("Content-Type", content_type);
}

inline void Response::set_content(const std::string &s,
                                  const std::string &content_type) {
  set_content(s.data(), s.size(), content_type);
}

inline void Response::set_content(std::string &&s,
                                  const std::string &content_type) {
  body = std::move(s);

  auto rng = headers.equal_range("Content-Type");
  headers.erase(rng.first, rng.second);
  set_header("Content-Type", content_type);
}

inline void Response::set_content_provider(
    size_t in_length, const std::string &content_type, ContentProvider provider,
    ContentProviderResourceReleaser resource_releaser) {
  set_header("Content-Type", content_type);
  content_length_ = in_length;
  if (in_length > 0) { content_provider_ = std::move(provider); }
  content_provider_resource_releaser_ = std::move(resource_releaser);
  is_chunked_content_provider_ = false;
}

inline void Response::set_content_provider(
    const std::string &content_type, ContentProviderWithoutLength provider,
    ContentProviderResourceReleaser resource_releaser) {
  set_header("Content-Type", content_type);
  content_length_ = 0;
  content_provider_ = detail::ContentProviderAdapter(std::move(provider));
  content_provider_resource_releaser_ = std::move(resource_releaser);
  is_chunked_content_provider_ = false;
}

inline void Response::set_chunked_content_provider(
    const std::string &content_type, ContentProviderWithoutLength provider,
    ContentProviderResourceReleaser resource_releaser) {
  set_header("Content-Type", content_type);
  content_length_ = 0;
  content_provider_ = detail::ContentProviderAdapter(std::move(provider));
  content_provider_resource_releaser_ = std::move(resource_releaser);
  is_chunked_content_provider_ = true;
}

inline void Response::set_file_content(const std::string &path,
                                       const std::string &content_type) {
  file_content_path_ = path;
  file_content_content_type_ = content_type;
}

inline void Response::set_file_content(const std::string &path) {
  file_content_path_ = path;
}

// Result implementation
inline bool Result::has_request_header(const std::string &key) const {
  return request_headers_.find(key) != request_headers_.end();
}

inline std::string Result::get_request_header_value(const std::string &key,
                                                    const char *def,
                                                    size_t id) const {
  return detail::get_header_value(request_headers_, key, def, id);
}

inline size_t
Result::get_request_header_value_count(const std::string &key) const {
  auto r = request_headers_.equal_range(key);
  return static_cast<size_t>(std::distance(r.first, r.second));
}

// Stream implementation
inline ssize_t Stream::write(const char *ptr) {
  return write(ptr, strlen(ptr));
}

inline ssize_t Stream::write(const std::string &s) {
  return write(s.data(), s.size());
}

namespace detail {

inline void calc_actual_timeout(time_t max_timeout_msec, time_t duration_msec,
                                time_t timeout_sec, time_t timeout_usec,
                                time_t &actual_timeout_sec,
                                time_t &actual_timeout_usec) {
  auto timeout_msec = (timeout_sec * 1000) + (timeout_usec / 1000);

  auto actual_timeout_msec =
      (std::min)(max_timeout_msec - duration_msec, timeout_msec);

  if (actual_timeout_msec < 0) { actual_timeout_msec = 0; }

  actual_timeout_sec = actual_timeout_msec / 1000;
  actual_timeout_usec = (actual_timeout_msec % 1000) * 1000;
}

// Socket stream implementation
inline SocketStream::SocketStream(
    socket_t sock, time_t read_timeout_sec, time_t read_timeout_usec,
    time_t write_timeout_sec, time_t write_timeout_usec,
    time_t max_timeout_msec,
    std::chrono::time_point<std::chrono::steady_clock> start_time)
    : sock_(sock), read_timeout_sec_(read_timeout_sec),
      read_timeout_usec_(read_timeout_usec),
      write_timeout_sec_(write_timeout_sec),
      write_timeout_usec_(write_timeout_usec),
      max_timeout_msec_(max_timeout_msec), start_time_(start_time),
      read_buff_(read_buff_size_, 0) {}

inline SocketStream::~SocketStream() = default;

inline bool SocketStream::is_readable() const {
  return read_buff_off_ < read_buff_content_size_;
}

inline bool SocketStream::wait_readable() const {
  if (max_timeout_msec_ <= 0) {
    return select_read(sock_, read_timeout_sec_, read_timeout_usec_) > 0;
  }

  time_t read_timeout_sec;
  time_t read_timeout_usec;
  calc_actual_timeout(max_timeout_msec_, duration(), read_timeout_sec_,
                      read_timeout_usec_, read_timeout_sec, read_timeout_usec);

  return select_read(sock_, read_timeout_sec, read_timeout_usec) > 0;
}

inline bool SocketStream::wait_writable() const {
  return select_write(sock_, write_timeout_sec_, write_timeout_usec_) > 0 &&
         is_socket_alive(sock_);
}

inline ssize_t SocketStream::read(char *ptr, size_t size) {
#ifdef _WIN32
  size =
      (std::min)(size, static_cast<size_t>((std::numeric_limits<int>::max)()));
#else
  size = (std::min)(size,
                    static_cast<size_t>((std::numeric_limits<ssize_t>::max)()));
#endif

  if (read_buff_off_ < read_buff_content_size_) {
    auto remaining_size = read_buff_content_size_ - read_buff_off_;
    if (size <= remaining_size) {
      memcpy(ptr, read_buff_.data() + read_buff_off_, size);
      read_buff_off_ += size;
      return static_cast<ssize_t>(size);
    } else {
      memcpy(ptr, read_buff_.data() + read_buff_off_, remaining_size);
      read_buff_off_ += remaining_size;
      return static_cast<ssize_t>(remaining_size);
    }
  }

  if (!wait_readable()) { return -1; }

  read_buff_off_ = 0;
  read_buff_content_size_ = 0;

  if (size < read_buff_size_) {
    auto n = read_socket(sock_, read_buff_.data(), read_buff_size_,
                         CPPHTTPLIB_RECV_FLAGS);
    if (n <= 0) {
      return n;
    } else if (n <= static_cast<ssize_t>(size)) {
      memcpy(ptr, read_buff_.data(), static_cast<size_t>(n));
      return n;
    } else {
      memcpy(ptr, read_buff_.data(), size);
      read_buff_off_ = size;
      read_buff_content_size_ = static_cast<size_t>(n);
      return static_cast<ssize_t>(size);
    }
  } else {
    return read_socket(sock_, ptr, size, CPPHTTPLIB_RECV_FLAGS);
  }
}

inline ssize_t SocketStream::write(const char *ptr, size_t size) {
  if (!wait_writable()) { return -1; }

#if defined(_WIN32) && !defined(_WIN64)
  size =
      (std::min)(size, static_cast<size_t>((std::numeric_limits<int>::max)()));
#endif

  return send_socket(sock_, ptr, size, CPPHTTPLIB_SEND_FLAGS);
}

inline void SocketStream::get_remote_ip_and_port(std::string &ip,
                                                 int &port) const {
  return detail::get_remote_ip_and_port(sock_, ip, port);
}

inline void SocketStream::get_local_ip_and_port(std::string &ip,
                                                int &port) const {
  return detail::get_local_ip_and_port(sock_, ip, port);
}

inline socket_t SocketStream::socket() const { return sock_; }

inline time_t SocketStream::duration() const {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now() - start_time_)
      .count();
}

// Buffer stream implementation
inline bool BufferStream::is_readable() const { return true; }

inline bool BufferStream::wait_readable() const { return true; }

inline bool BufferStream::wait_writable() const { return true; }

inline ssize_t BufferStream::read(char *ptr, size_t size) {
#if defined(_MSC_VER) && _MSC_VER < 1910
  auto len_read = buffer._Copy_s(ptr, size, size, position);
#else
  auto len_read = buffer.copy(ptr, size, position);
#endif
  position += static_cast<size_t>(len_read);
  return static_cast<ssize_t>(len_read);
}

inline ssize_t BufferStream::write(const char *ptr, size_t size) {
  buffer.append(ptr, size);
  return static_cast<ssize_t>(size);
}

inline void BufferStream::get_remote_ip_and_port(std::string & /*ip*/,
                                                 int & /*port*/) const {}

inline void BufferStream::get_local_ip_and_port(std::string & /*ip*/,
                                                int & /*port*/) const {}

inline socket_t BufferStream::socket() const { return 0; }

inline time_t BufferStream::duration() const { return 0; }

inline const std::string &BufferStream::get_buffer() const { return buffer; }

inline PathParamsMatcher::PathParamsMatcher(const std::string &pattern) {
  constexpr const char marker[] = "/:";

  // One past the last ending position of a path param substring
  std::size_t last_param_end = 0;

#ifndef CPPHTTPLIB_NO_EXCEPTIONS
  // Needed to ensure that parameter names are unique during matcher
  // construction
  // If exceptions are disabled, only last duplicate path
  // parameter will be set
  std::unordered_set<std::string> param_name_set;
#endif

  while (true) {
    const auto marker_pos = pattern.find(
        marker, last_param_end == 0 ? last_param_end : last_param_end - 1);
    if (marker_pos == std::string::npos) { break; }

    static_fragments_.push_back(
        pattern.substr(last_param_end, marker_pos - last_param_end + 1));

    const auto param_name_start = marker_pos + str_len(marker);

    auto sep_pos = pattern.find(separator, param_name_start);
    if (sep_pos == std::string::npos) { sep_pos = pattern.length(); }

    auto param_name =
        pattern.substr(param_name_start, sep_pos - param_name_start);

#ifndef CPPHTTPLIB_NO_EXCEPTIONS
    if (param_name_set.find(param_name) != param_name_set.cend()) {
      std::string msg = "Encountered path parameter '" + param_name +
                        "' multiple times in route pattern '" + pattern + "'.";
      throw std::invalid_argument(msg);
    }
#endif

    param_names_.push_back(std::move(param_name));

    last_param_end = sep_pos + 1;
  }

  if (last_param_end < pattern.length()) {
    static_fragments_.push_back(pattern.substr(last_param_end));
  }
}

inline bool PathParamsMatcher::match(Request &request) const {
  request.matches = std::smatch();
  request.path_params.clear();
  request.path_params.reserve(param_names_.size());

  // One past the position at which the path matched the pattern last time
  std::size_t starting_pos = 0;
  for (size_t i = 0; i < static_fragments_.size(); ++i) {
    const auto &fragment = static_fragments_[i];

    if (starting_pos + fragment.length() > request.path.length()) {
      return false;
    }

    // Avoid unnecessary allocation by using strncmp instead of substr +
    // comparison
    if (std::strncmp(request.path.c_str() + starting_pos, fragment.c_str(),
                     fragment.length()) != 0) {
      return false;
    }

    starting_pos += fragment.length();

    // Should only happen when we have a static fragment after a param
    // Example: '/users/:id/subscriptions'
    // The 'subscriptions' fragment here does not have a corresponding param
    if (i >= param_names_.size()) { continue; }

    auto sep_pos = request.path.find(separator, starting_pos);
    if (sep_pos == std::string::npos) { sep_pos = request.path.length(); }

    const auto &param_name = param_names_[i];

    request.path_params.emplace(
        param_name, request.path.substr(starting_pos, sep_pos - starting_pos));

    // Mark everything up to '/' as matched
    starting_pos = sep_pos + 1;
  }
  // Returns false if the path is longer than the pattern
  return starting_pos >= request.path.length();
}

inline bool RegexMatcher::match(Request &request) const {
  request.path_params.clear();
  return std::regex_match(request.path, request.matches, regex_);
}

} // namespace detail

// HTTP server implementation
inline Server::Server()
    : new_task_queue(
          [] { return new ThreadPool(CPPHTTPLIB_THREAD_POOL_COUNT); }) {
#ifndef _WIN32
  signal(SIGPIPE, SIG_IGN);
#endif
}

inline Server::~Server() = default;

inline std::unique_ptr<detail::MatcherBase>
Server::make_matcher(const std::string &pattern) {
  if (pattern.find("/:") != std::string::npos) {
    return detail::make_unique<detail::PathParamsMatcher>(pattern);
  } else {
    return detail::make_unique<detail::RegexMatcher>(pattern);
  }
}

inline Server &Server::Get(const std::string &pattern, Handler handler) {
  get_handlers_.emplace_back(make_matcher(pattern), std::move(handler));
  return *this;
}

inline Server &Server::Post(const std::string &pattern, Handler handler) {
  post_handlers_.emplace_back(make_matcher(pattern), std::move(handler));
  return *this;
}

inline Server &Server::Post(const std::string &pattern,
                            HandlerWithContentReader handler) {
  post_handlers_for_content_reader_.emplace_back(make_matcher(pattern),
                                                 std::move(handler));
  return *this;
}

inline Server &Server::Put(const std::string &pattern, Handler handler) {
  put_handlers_.emplace_back(make_matcher(pattern), std::move(handler));
  return *this;
}

inline Server &Server::Put(const std::string &pattern,
                           HandlerWithContentReader handler) {
  put_handlers_for_content_reader_.emplace_back(make_matcher(pattern),
                                                std::move(handler));
  return *this;
}

inline Server &Server::Patch(const std::string &pattern, Handler handler) {
  patch_handlers_.emplace_back(make_matcher(pattern), std::move(handler));
  return *this;
}

inline Server &Server::Patch(const std::string &pattern,
                             HandlerWithContentReader handler) {
  patch_handlers_for_content_reader_.emplace_back(make_matcher(pattern),
                                                  std::move(handler));
  return *this;
}

inline Server &Server::Delete(const std::string &pattern, Handler handler) {
  delete_handlers_.emplace_back(make_matcher(pattern), std::move(handler));
  return *this;
}

inline Server &Server::Delete(const std::string &pattern,
                              HandlerWithContentReader handler) {
  delete_handlers_for_content_reader_.emplace_back(make_matcher(pattern),
                                                   std::move(handler));
  return *this;
}

inline Server &Server::Options(const std::string &pattern, Handler handler) {
  options_handlers_.emplace_back(make_matcher(pattern), std::move(handler));
  return *this;
}

inline bool Server::set_base_dir(const std::string &dir,
                                 const std::string &mount_point) {
  return set_mount_point(mount_point, dir);
}

inline bool Server::set_mount_point(const std::string &mount_point,
                                    const std::string &dir, Headers headers) {
  detail::FileStat stat(dir);
  if (stat.is_dir()) {
    std::string mnt = !mount_point.empty() ? mount_point : "/";
    if (!mnt.empty() && mnt[0] == '/') {
      base_dirs_.push_back({mnt, dir, std::move(headers)});
      return true;
    }
  }
  return false;
}

inline bool Server::remove_mount_point(const std::string &mount_point) {
  for (auto it = base_dirs_.begin(); it != base_dirs_.end(); ++it) {
    if (it->mount_point == mount_point) {
      base_dirs_.erase(it);
      return true;
    }
  }
  return false;
}

inline Server &
Server::set_file_extension_and_mimetype_mapping(const std::string &ext,
                                                const std::string &mime) {
  file_extension_and_mimetype_map_[ext] = mime;
  return *this;
}

inline Server &Server::set_default_file_mimetype(const std::string &mime) {
  default_file_mimetype_ = mime;
  return *this;
}

inline Server &Server::set_file_request_handler(Handler handler) {
  file_request_handler_ = std::move(handler);
  return *this;
}

inline Server &Server::set_error_handler_core(HandlerWithResponse handler,
                                              std::true_type) {
  error_handler_ = std::move(handler);
  return *this;
}

inline Server &Server::set_error_handler_core(Handler handler,
                                              std::false_type) {
  error_handler_ = [handler](const Request &req, Response &res) {
    handler(req, res);
    return HandlerResponse::Handled;
  };
  return *this;
}

inline Server &Server::set_exception_handler(ExceptionHandler handler) {
  exception_handler_ = std::move(handler);
  return *this;
}

inline Server &Server::set_pre_routing_handler(HandlerWithResponse handler) {
  pre_routing_handler_ = std::move(handler);
  return *this;
}

inline Server &Server::set_post_routing_handler(Handler handler) {
  post_routing_handler_ = std::move(handler);
  return *this;
}

inline Server &Server::set_logger(Logger logger) {
  logger_ = std::move(logger);
  return *this;
}

inline Server &
Server::set_expect_100_continue_handler(Expect100ContinueHandler handler) {
  expect_100_continue_handler_ = std::move(handler);
  return *this;
}

inline Server &Server::set_address_family(int family) {
  address_family_ = family;
  return *this;
}

inline Server &Server::set_tcp_nodelay(bool on) {
  tcp_nodelay_ = on;
  return *this;
}

inline Server &Server::set_ipv6_v6only(bool on) {
  ipv6_v6only_ = on;
  return *this;
}

inline Server &Server::set_socket_options(SocketOptions socket_options) {
  socket_options_ = std::move(socket_options);
  return *this;
}

inline Server &Server::set_default_headers(Headers headers) {
  default_headers_ = std::move(headers);
  return *this;
}

inline Server &Server::set_header_writer(
    std::function<ssize_t(Stream &, Headers &)> const &writer) {
  header_writer_ = writer;
  return *this;
}

inline Server &Server::set_keep_alive_max_count(size_t count) {
  keep_alive_max_count_ = count;
  return *this;
}

inline Server &Server::set_keep_alive_timeout(time_t sec) {
  keep_alive_timeout_sec_ = sec;
  return *this;
}

inline Server &Server::set_read_timeout(time_t sec, time_t usec) {
  read_timeout_sec_ = sec;
  read_timeout_usec_ = usec;
  return *this;
}

inline Server &Server::set_write_timeout(time_t sec, time_t usec) {
  write_timeout_sec_ = sec;
  write_timeout_usec_ = usec;
  return *this;
}

inline Server &Server::set_idle_interval(time_t sec, time_t usec) {
  idle_interval_sec_ = sec;
  idle_interval_usec_ = usec;
  return *this;
}

inline Server &Server::set_payload_max_length(size_t length) {
  payload_max_length_ = length;
  return *this;
}

inline bool Server::bind_to_port(const std::string &host, int port,
                                 int socket_flags) {
  auto ret = bind_internal(host, port, socket_flags);
  if (ret == -1) { is_decommissioned = true; }
  return ret >= 0;
}
inline int Server::bind_to_any_port(const std::string &host, int socket_flags) {
  auto ret = bind_internal(host, 0, socket_flags);
  if (ret == -1) { is_decommissioned = true; }
  return ret;
}

inline bool Server::listen_after_bind() { return listen_internal(); }

inline bool Server::listen(const std::string &host, int port,
                           int socket_flags) {
  return bind_to_port(host, port, socket_flags) && listen_internal();
}

inline bool Server::is_running() const { return is_running_; }

inline void Server::wait_until_ready() const {
  while (!is_running_ && !is_decommissioned) {
    std::this_thread::sleep_for(std::chrono::milliseconds{1});
  }
}

inline void Server::stop() {
  if (is_running_) {
    assert(svr_sock_ != INVALID_SOCKET);
    std::atomic<socket_t> sock(svr_sock_.exchange(INVALID_SOCKET));
    detail::shutdown_socket(sock);
    detail::close_socket(sock);
  }
  is_decommissioned = false;
}

inline void Server::decommission() { is_decommissioned = true; }

inline bool Server::parse_request_line(const char *s, Request &req) const {
  auto len = strlen(s);
  if (len < 2 || s[len - 2] != '\r' || s[len - 1] != '\n') { return false; }
  len -= 2;

  {
    size_t count = 0;

    detail::split(s, s + len, ' ', [&](const char *b, const char *e) {
      switch (count) {
      case 0: req.method = std::string(b, e); break;
      case 1: req.target = std::string(b, e); break;
      case 2: req.version = std::string(b, e); break;
      default: break;
      }
      count++;
    });

    if (count != 3) { return false; }
  }

  thread_local const std::set<std::string> methods{
      "GET",     "HEAD",    "POST",  "PUT",   "DELETE",
      "CONNECT", "OPTIONS", "TRACE", "PATCH", "PRI"};

  if (methods.find(req.method) == methods.end()) { return false; }

  if (req.version != "HTTP/1.1" && req.version != "HTTP/1.0") { return false; }

  {
    // Skip URL fragment
    for (size_t i = 0; i < req.target.size(); i++) {
      if (req.target[i] == '#') {
        req.target.erase(i);
        break;
      }
    }

    detail::divide(req.target, '?',
                   [&](const char *lhs_data, std::size_t lhs_size,
                       const char *rhs_data, std::size_t rhs_size) {
                     req.path = detail::decode_url(
                         std::string(lhs_data, lhs_size), false);
                     detail::parse_query_text(rhs_data, rhs_size, req.params);
                   });
  }

  return true;
}

inline bool Server::write_response(Stream &strm, bool close_connection,
                                   Request &req, Response &res) {
  // NOTE: `req.ranges` should be empty, otherwise it will be applied
  // incorrectly to the error content.
  req.ranges.clear();
  return write_response_core(strm, close_connection, req, res, false);
}

inline bool Server::write_response_with_content(Stream &strm,
                                                bool close_connection,
                                                const Request &req,
                                                Response &res) {
  return write_response_core(strm, close_connection, req, res, true);
}

inline bool Server::write_response_core(Stream &strm, bool close_connection,
                                        const Request &req, Response &res,
                                        bool need_apply_ranges) {
  assert(res.status != -1);

  if (400 <= res.status && error_handler_ &&
      error_handler_(req, res) == HandlerResponse::Handled) {
    need_apply_ranges = true;
  }

  std::string content_type;
  std::string boundary;
  if (need_apply_ranges) { apply_ranges(req, res, content_type, boundary); }

  // Prepare additional headers
  if (close_connection || req.get_header_value("Connection") == "close") {
    res.set_header("Connection", "close");
  } else {
    std::string s = "timeout=";
    s += std::to_string(keep_alive_timeout_sec_);
    s += ", max=";
    s += std::to_string(keep_alive_max_count_);
    res.set_header("Keep-Alive", s);
  }

  if ((!res.body.empty() || res.content_length_ > 0 || res.content_provider_) &&
      !res.has_header("Content-Type")) {
    res.set_header("Content-Type", "text/plain");
  }

  if (res.body.empty() && !res.content_length_ && !res.content_provider_ &&
      !res.has_header("Content-Length")) {
    res.set_header("Content-Length", "0");
  }

  if (req.method == "HEAD" && !res.has_header("Accept-Ranges")) {
    res.set_header("Accept-Ranges", "bytes");
  }

  if (post_routing_handler_) { post_routing_handler_(req, res); }

  // Response line and headers
  {
    detail::BufferStream bstrm;
    if (!detail::write_response_line(bstrm, res.status)) { return false; }
    if (!header_writer_(bstrm, res.headers)) { return false; }

    // Flush buffer
    auto &data = bstrm.get_buffer();
    detail::write_data(strm, data.data(), data.size());
  }

  // Body
  auto ret = true;
  if (req.method != "HEAD") {
    if (!res.body.empty()) {
      if (!detail::write_data(strm, res.body.data(), res.body.size())) {
        ret = false;
      }
    } else if (res.content_provider_) {
      if (write_content_with_provider(strm, req, res, boundary, content_type)) {
        res.content_provider_success_ = true;
      } else {
        ret = false;
      }
    }
  }

  // Log
  if (logger_) { logger_(req, res); }

  return ret;
}

inline bool
Server::write_content_with_provider(Stream &strm, const Request &req,
                                    Response &res, const std::string &boundary,
                                    const std::string &content_type) {
  auto is_shutting_down = [this]() {
    return this->svr_sock_ == INVALID_SOCKET;
  };

  if (res.content_length_ > 0) {
    if (req.ranges.empty()) {
      return detail::write_content(strm, res.content_provider_, 0,
                                   res.content_length_, is_shutting_down);
    } else if (req.ranges.size() == 1) {
      auto offset_and_length = detail::get_range_offset_and_length(
          req.ranges[0], res.content_length_);

      return detail::write_content(strm, res.content_provider_,
                                   offset_and_length.first,
                                   offset_and_length.second, is_shutting_down);
    } else {
      return detail::write_multipart_ranges_data(
          strm, req, res, boundary, content_type, res.content_length_,
          is_shutting_down);
    }
  } else {
    if (res.is_chunked_content_provider_) {
      auto type = detail::encoding_type(req, res);

      std::unique_ptr<detail::compressor> compressor;
      if (type == detail::EncodingType::Gzip) {
#ifdef CPPHTTPLIB_ZLIB_SUPPORT
        compressor = detail::make_unique<detail::gzip_compressor>();
#endif
      } else if (type == detail::EncodingType::Brotli) {
#ifdef CPPHTTPLIB_BROTLI_SUPPORT
        compressor = detail::make_unique<detail::brotli_compressor>();
#endif
      } else if (type == detail::EncodingType::Zstd) {
#ifdef CPPHTTPLIB_ZSTD_SUPPORT
        compressor = detail::make_unique<detail::zstd_compressor>();
#endif
      } else {
        compressor = detail::make_unique<detail::nocompressor>();
      }
      assert(compressor != nullptr);

      return detail::write_content_chunked(strm, res.content_provider_,
                                           is_shutting_down, *compressor);
    } else {
      return detail::write_content_without_length(strm, res.content_provider_,
                                                  is_shutting_down);
    }
  }
}

inline bool Server::read_content(Stream &strm, Request &req, Response &res) {
  MultipartFormDataMap::iterator cur;
  auto file_count = 0;
  if (read_content_core(
          strm, req, res,
          // Regular
          [&](const char *buf, size_t n) {
            if (req.body.size() + n > req.body.max_size()) { return false; }
            req.body.append(buf, n);
            return true;
          },
          // Multipart
          [&](const MultipartFormData &file) {
            if (file_count++ == CPPHTTPLIB_MULTIPART_FORM_DATA_FILE_MAX_COUNT) {
              return false;
            }
            cur = req.files.emplace(file.name, file);
            return true;
          },
          [&](const char *buf, size_t n) {
            auto &content = cur->second.content;
            if (content.size() + n > content.max_size()) { return false; }
            content.append(buf, n);
            return true;
          })) {
    const auto &content_type = req.get_header_value("Content-Type");
    if (!content_type.find("application/x-www-form-urlencoded")) {
      if (req.body.size() > CPPHTTPLIB_FORM_URL_ENCODED_PAYLOAD_MAX_LENGTH) {
        res.status = StatusCode::PayloadTooLarge_413; // NOTE: should be 414?
        return false;
      }
      detail::parse_query_text(req.body, req.params);
    }
    return true;
  }
  return false;
}

inline bool Server::read_content_with_content_receiver(
    Stream &strm, Request &req, Response &res, ContentReceiver receiver,
    MultipartContentHeader multipart_header,
    ContentReceiver multipart_receiver) {
  return read_content_core(strm, req, res, std::move(receiver),
                           std::move(multipart_header),
                           std::move(multipart_receiver));
}

inline bool
Server::read_content_core(Stream &strm, Request &req, Response &res,
                          ContentReceiver receiver,
                          MultipartContentHeader multipart_header,
                          ContentReceiver multipart_receiver) const {
  detail::MultipartFormDataParser multipart_form_data_parser;
  ContentReceiverWithProgress out;

  if (req.is_multipart_form_data()) {
    const auto &content_type = req.get_header_value("Content-Type");
    std::string boundary;
    if (!detail::parse_multipart_boundary(content_type, boundary)) {
      res.status = StatusCode::BadRequest_400;
      return false;
    }

    multipart_form_data_parser.set_boundary(std::move(boundary));
    out = [&](const char *buf, size_t n, uint64_t /*off*/, uint64_t /*len*/) {
      /* For debug
      size_t pos = 0;
      while (pos < n) {
        auto read_size = (std::min)<size_t>(1, n - pos);
        auto ret = multipart_form_data_parser.parse(
            buf + pos, read_size, multipart_receiver, multipart_header);
        if (!ret) { return false; }
        pos += read_size;
      }
      return true;
      */
      return multipart_form_data_parser.parse(buf, n, multipart_receiver,
                                              multipart_header);
    };
  } else {
    out = [receiver](const char *buf, size_t n, uint64_t /*off*/,
                     uint64_t /*len*/) { return receiver(buf, n); };
  }

  if (req.method == "DELETE" && !req.has_header("Content-Length")) {
    return true;
  }

  if (!detail::read_content(strm, req, payload_max_length_, res.status, nullptr,
                            out, true)) {
    return false;
  }

  if (req.is_multipart_form_data()) {
    if (!multipart_form_data_parser.is_valid()) {
      res.status = StatusCode::BadRequest_400;
      return false;
    }
  }

  return true;
}

inline bool Server::handle_file_request(const Request &req, Response &res,
                                        bool head) {
  for (const auto &entry : base_dirs_) {
    // Prefix match
    if (!req.path.compare(0, entry.mount_point.size(), entry.mount_point)) {
      std::string sub_path = "/" + req.path.substr(entry.mount_point.size());
      if (detail::is_valid_path(sub_path)) {
        auto path = entry.base_dir + sub_path;
        if (path.back() == '/') { path += "index.html"; }

        detail::FileStat stat(path);

        if (stat.is_dir()) {
          res.set_redirect(sub_path + "/", StatusCode::MovedPermanently_301);
          return true;
        }

        if (stat.is_file()) {
          for (const auto &kv : entry.headers) {
            res.set_header(kv.first, kv.second);
          }

          auto mm = std::make_shared<detail::mmap>(path.c_str());
          if (!mm->is_open()) { return false; }

          res.set_content_provider(
              mm->size(),
              detail::find_content_type(path, file_extension_and_mimetype_map_,
                                        default_file_mimetype_),
              [mm](size_t offset, size_t length, DataSink &sink) -> bool {
                sink.write(mm->data() + offset, length);
                return true;
              });

          if (!head && file_request_handler_) {
            file_request_handler_(req, res);
          }

          return true;
        }
      }
    }
  }
  return false;
}

inline socket_t
Server::create_server_socket(const std::string &host, int port,
                             int socket_flags,
                             SocketOptions socket_options) const {
  return detail::create_socket(
      host, std::string(), port, address_family_, socket_flags, tcp_nodelay_,
      ipv6_v6only_, std::move(socket_options),
      [](socket_t sock, struct addrinfo &ai, bool & /*quit*/) -> bool {
        if (::bind(sock, ai.ai_addr, static_cast<socklen_t>(ai.ai_addrlen))) {
          return false;
        }
        if (::listen(sock, CPPHTTPLIB_LISTEN_BACKLOG)) { return false; }
        return true;
      });
}

inline int Server::bind_internal(const std::string &host, int port,
                                 int socket_flags) {
  if (is_decommissioned) { return -1; }

  if (!is_valid()) { return -1; }

  svr_sock_ = create_server_socket(host, port, socket_flags, socket_options_);
  if (svr_sock_ == INVALID_SOCKET) { return -1; }

  if (port == 0) {
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);
    if (getsockname(svr_sock_, reinterpret_cast<struct sockaddr *>(&addr),
                    &addr_len) == -1) {
      return -1;
    }
    if (addr.ss_family == AF_INET) {
      return ntohs(reinterpret_cast<struct sockaddr_in *>(&addr)->sin_port);
    } else if (addr.ss_family == AF_INET6) {
      return ntohs(reinterpret_cast<struct sockaddr_in6 *>(&addr)->sin6_port);
    } else {
      return -1;
    }
  } else {
    return port;
  }
}

inline bool Server::listen_internal() {
  if (is_decommissioned) { return false; }

  auto ret = true;
  is_running_ = true;
  auto se = detail::scope_exit([&]() { is_running_ = false; });

  {
    std::unique_ptr<TaskQueue> task_queue(new_task_queue());

    while (svr_sock_ != INVALID_SOCKET) {
#ifndef _WIN32
      if (idle_interval_sec_ > 0 || idle_interval_usec_ > 0) {
#endif
        auto val = detail::select_read(svr_sock_, idle_interval_sec_,
                                       idle_interval_usec_);
        if (val == 0) { // Timeout
          task_queue->on_idle();
          continue;
        }
#ifndef _WIN32
      }
#endif

#if defined _WIN32
      // sockets connected via WASAccept inherit flags NO_HANDLE_INHERIT,
      // OVERLAPPED
      socket_t sock = WSAAccept(svr_sock_, nullptr, nullptr, nullptr, 0);
#elif defined SOCK_CLOEXEC
      socket_t sock = accept4(svr_sock_, nullptr, nullptr, SOCK_CLOEXEC);
#else
      socket_t sock = accept(svr_sock_, nullptr, nullptr);
#endif

      if (sock == INVALID_SOCKET) {
        if (errno == EMFILE) {
          // The per-process limit of open file descriptors has been reached.
          // Try to accept new connections after a short sleep.
          std::this_thread::sleep_for(std::chrono::microseconds{1});
          continue;
        } else if (errno == EINTR || errno == EAGAIN) {
          continue;
        }
        if (svr_sock_ != INVALID_SOCKET) {
          detail::close_socket(svr_sock_);
          ret = false;
        } else {
          ; // The server socket was closed by user.
        }
        break;
      }

      detail::set_socket_opt_time(sock, SOL_SOCKET, SO_RCVTIMEO,
                                  read_timeout_sec_, read_timeout_usec_);
      detail::set_socket_opt_time(sock, SOL_SOCKET, SO_SNDTIMEO,
                                  write_timeout_sec_, write_timeout_usec_);

      if (!task_queue->enqueue(
              [this, sock]() { process_and_close_socket(sock); })) {
        detail::shutdown_socket(sock);
        detail::close_socket(sock);
      }
    }

    task_queue->shutdown();
  }

  is_decommissioned = !ret;
  return ret;
}

inline bool Server::routing(Request &req, Response &res, Stream &strm) {
  if (pre_routing_handler_ &&
      pre_routing_handler_(req, res) == HandlerResponse::Handled) {
    return true;
  }

  // File handler
  auto is_head_request = req.method == "HEAD";
  if ((req.method == "GET" || is_head_request) &&
      handle_file_request(req, res, is_head_request)) {
    return true;
  }

  if (detail::expect_content(req)) {
    // Content reader handler
    {
      ContentReader reader(
          [&](ContentReceiver receiver) {
            return read_content_with_content_receiver(
                strm, req, res, std::move(receiver), nullptr, nullptr);
          },
          [&](MultipartContentHeader header, ContentReceiver receiver) {
            return read_content_with_content_receiver(strm, req, res, nullptr,
                                                      std::move(header),
                                                      std::move(receiver));
          });

      if (req.method == "POST") {
        if (dispatch_request_for_content_reader(
                req, res, std::move(reader),
                post_handlers_for_content_reader_)) {
          return true;
        }
      } else if (req.method == "PUT") {
        if (dispatch_request_for_content_reader(
                req, res, std::move(reader),
                put_handlers_for_content_reader_)) {
          return true;
        }
      } else if (req.method == "PATCH") {
        if (dispatch_request_for_content_reader(
                req, res, std::move(reader),
                patch_handlers_for_content_reader_)) {
          return true;
        }
      } else if (req.method == "DELETE") {
        if (dispatch_request_for_content_reader(
                req, res, std::move(reader),
                delete_handlers_for_content_reader_)) {
          return true;
        }
      }
    }

    // Read content into `req.body`
    if (!read_content(strm, req, res)) { return false; }
  }

  // Regular handler
  if (req.method == "GET" || req.method == "HEAD") {
    return dispatch_request(req, res, get_handlers_);
  } else if (req.method == "POST") {
    return dispatch_request(req, res, post_handlers_);
  } else if (req.method == "PUT") {
    return dispatch_request(req, res, put_handlers_);
  } else if (req.method == "DELETE") {
    return dispatch_request(req, res, delete_handlers_);
  } else if (req.method == "OPTIONS") {
    return dispatch_request(req, res, options_handlers_);
  } else if (req.method == "PATCH") {
    return dispatch_request(req, res, patch_handlers_);
  }

  res.status = StatusCode::BadRequest_400;
  return false;
}

inline bool Server::dispatch_request(Request &req, Response &res,
                                     const Handlers &handlers) const {
  for (const auto &x : handlers) {
    const auto &matcher = x.first;
    const auto &handler = x.second;

    if (matcher->match(req)) {
      handler(req, res);
      return true;
    }
  }
  return false;
}

inline void Server::apply_ranges(const Request &req, Response &res,
                                 std::string &content_type,
                                 std::string &boundary) const {
  if (req.ranges.size() > 1 && res.status == StatusCode::PartialContent_206) {
    auto it = res.headers.find("Content-Type");
    if (it != res.headers.end()) {
      content_type = it->second;
      res.headers.erase(it);
    }

    boundary = detail::make_multipart_data_boundary();

    res.set_header("Content-Type",
                   "multipart/byteranges; boundary=" + boundary);
  }

  auto type = detail::encoding_type(req, res);

  if (res.body.empty()) {
    if (res.content_length_ > 0) {
      size_t length = 0;
      if (req.ranges.empty() || res.status != StatusCode::PartialContent_206) {
        length = res.content_length_;
      } else if (req.ranges.size() == 1) {
        auto offset_and_length = detail::get_range_offset_and_length(
            req.ranges[0], res.content_length_);

        length = offset_and_length.second;

        auto content_range = detail::make_content_range_header_field(
            offset_and_length, res.content_length_);
        res.set_header("Content-Range", content_range);
      } else {
        length = detail::get_multipart_ranges_data_length(
            req, boundary, content_type, res.content_length_);
      }
      res.set_header("Content-Length", std::to_string(length));
    } else {
      if (res.content_provider_) {
        if (res.is_chunked_content_provider_) {
          res.set_header("Transfer-Encoding", "chunked");
          if (type == detail::EncodingType::Gzip) {
            res.set_header("Content-Encoding", "gzip");
          } else if (type == detail::EncodingType::Brotli) {
            res.set_header("Content-Encoding", "br");
          } else if (type == detail::EncodingType::Zstd) {
            res.set_header("Content-Encoding", "zstd");
          }
        }
      }
    }
  } else {
    if (req.ranges.empty() || res.status != StatusCode::PartialContent_206) {
      ;
    } else if (req.ranges.size() == 1) {
      auto offset_and_length =
          detail::get_range_offset_and_length(req.ranges[0], res.body.size());
      auto offset = offset_and_length.first;
      auto length = offset_and_length.second;

      auto content_range = detail::make_content_range_header_field(
          offset_and_length, res.body.size());
      res.set_header("Content-Range", content_range);

      assert(offset + length <= res.body.size());
      res.body = res.body.substr(offset, length);
    } else {
      std::string data;
      detail::make_multipart_ranges_data(req, res, boundary, content_type,
                                         res.body.size(), data);
      res.body.swap(data);
    }

    if (type != detail::EncodingType::None) {
      std::unique_ptr<detail::compressor> compressor;
      std::string content_encoding;

      if (type == detail::EncodingType::Gzip) {
#ifdef CPPHTTPLIB_ZLIB_SUPPORT
        compressor = detail::make_unique<detail::gzip_compressor>();
        content_encoding = "gzip";
#endif
      } else if (type == detail::EncodingType::Brotli) {
#ifdef CPPHTTPLIB_BROTLI_SUPPORT
        compressor = detail::make_unique<detail::brotli_compressor>();
        content_encoding = "br";
#endif
      } else if (type == detail::EncodingType::Zstd) {
#ifdef CPPHTTPLIB_ZSTD_SUPPORT
        compressor = detail::make_unique<detail::zstd_compressor>();
        content_encoding = "zstd";
#endif
      }

      if (compressor) {
        std::string compressed;
        if (compressor->compress(res.body.data(), res.body.size(), true,
                                 [&](const char *data, size_t data_len) {
                                   compressed.append(data, data_len);
                                   return true;
                                 })) {
          res.body.swap(compressed);
          res.set_header("Content-Encoding", content_encoding);
        }
      }
    }

    auto length = std::to_string(res.body.size());
    res.set_header("Content-Length", length);
  }
}

inline bool Server::dispatch_request_for_content_reader(
    Request &req, Response &res, ContentReader content_reader,
    const HandlersForContentReader &handlers) const {
  for (const auto &x : handlers) {
    const auto &matcher = x.first;
    const auto &handler = x.second;

    if (matcher->match(req)) {
      handler(req, res, content_reader);
      return true;
    }
  }
  return false;
}

inline bool
Server::process_request(Stream &strm, const std::string &remote_addr,
                        int remote_port, const std::string &local_addr,
                        int local_port, bool close_connection,
                        bool &connection_closed,
                        const std::function<void(Request &)> &setup_request) {
  std::array<char, 2048> buf{};

  detail::stream_line_reader line_reader(strm, buf.data(), buf.size());

  // Connection has been closed on client
  if (!line_reader.getline()) { return false; }

  Request req;

  Response res;
  res.version = "HTTP/1.1";
  res.headers = default_headers_;

  // Request line and headers
  if (!parse_request_line(line_reader.ptr(), req) ||
      !detail::read_headers(strm, req.headers)) {
    res.status = StatusCode::BadRequest_400;
    return write_response(strm, close_connection, req, res);
  }

  // Check if the request URI doesn't exceed the limit
  if (req.target.size() > CPPHTTPLIB_REQUEST_URI_MAX_LENGTH) {
    Headers dummy;
    detail::read_headers(strm, dummy);
    res.status = StatusCode::UriTooLong_414;
    return write_response(strm, close_connection, req, res);
  }

  if (req.get_header_value("Connection") == "close") {
    connection_closed = true;
  }

  if (req.version == "HTTP/1.0" &&
      req.get_header_value("Connection") != "Keep-Alive") {
    connection_closed = true;
  }

  req.remote_addr = remote_addr;
  req.remote_port = remote_port;
  req.set_header("REMOTE_ADDR", req.remote_addr);
  req.set_header("REMOTE_PORT", std::to_string(req.remote_port));

  req.local_addr = local_addr;
  req.local_port = local_port;
  req.set_header("LOCAL_ADDR", req.local_addr);
  req.set_header("LOCAL_PORT", std::to_string(req.local_port));

  if (req.has_header("Range")) {
    const auto &range_header_value = req.get_header_value("Range");
    if (!detail::parse_range_header(range_header_value, req.ranges)) {
      res.status = StatusCode::RangeNotSatisfiable_416;
      return write_response(strm, close_connection, req, res);
    }
  }

  if (setup_request) { setup_request(req); }

  if (req.get_header_value("Expect") == "100-continue") {
    int status = StatusCode::Continue_100;
    if (expect_100_continue_handler_) {
      status = expect_100_continue_handler_(req, res);
    }
    switch (status) {
    case StatusCode::Continue_100:
    case StatusCode::ExpectationFailed_417:
      detail::write_response_line(strm, status);
      strm.write("\r\n");
      break;
    default:
      connection_closed = true;
      return write_response(strm, true, req, res);
    }
  }

  // Setup `is_connection_closed` method
  req.is_connection_closed = [&]() {
    return !detail::is_socket_alive(strm.socket());
  };

  // Routing
  auto routed = false;
#ifdef CPPHTTPLIB_NO_EXCEPTIONS
  routed = routing(req, res, strm);
#else
  try {
    routed = routing(req, res, strm);
  } catch (std::exception &e) {
    if (exception_handler_) {
      auto ep = std::current_exception();
      exception_handler_(req, res, ep);
      routed = true;
    } else {
      res.status = StatusCode::InternalServerError_500;
      std::string val;
      auto s = e.what();
      for (size_t i = 0; s[i]; i++) {
        switch (s[i]) {
        case '\r': val += "\\r"; break;
        case '\n': val += "\\n"; break;
        default: val += s[i]; break;
        }
      }
      res.set_header("EXCEPTION_WHAT", val);
    }
  } catch (...) {
    if (exception_handler_) {
      auto ep = std::current_exception();
      exception_handler_(req, res, ep);
      routed = true;
    } else {
      res.status = StatusCode::InternalServerError_500;
      res.set_header("EXCEPTION_WHAT", "UNKNOWN");
    }
  }
#endif
  if (routed) {
    if (res.status == -1) {
      res.status = req.ranges.empty() ? StatusCode::OK_200
                                      : StatusCode::PartialContent_206;
    }

    // Serve file content by using a content provider
    if (!res.file_content_path_.empty()) {
      const auto &path = res.file_content_path_;
      auto mm = std::make_shared<detail::mmap>(path.c_str());
      if (!mm->is_open()) {
        res.body.clear();
        res.content_length_ = 0;
        res.content_provider_ = nullptr;
        res.status = StatusCode::NotFound_404;
        return write_response(strm, close_connection, req, res);
      }

      auto content_type = res.file_content_content_type_;
      if (content_type.empty()) {
        content_type = detail::find_content_type(
            path, file_extension_and_mimetype_map_, default_file_mimetype_);
      }

      res.set_content_provider(
          mm->size(), content_type,
          [mm](size_t offset, size_t length, DataSink &sink) -> bool {
            sink.write(mm->data() + offset, length);
            return true;
          });
    }

    if (detail::range_error(req, res)) {
      res.body.clear();
      res.content_length_ = 0;
      res.content_provider_ = nullptr;
      res.status = StatusCode::RangeNotSatisfiable_416;
      return write_response(strm, close_connection, req, res);
    }

    return write_response_with_content(strm, close_connection, req, res);
  } else {
    if (res.status == -1) { res.status = StatusCode::NotFound_404; }

    return write_response(strm, close_connection, req, res);
  }
}

inline bool Server::is_valid() const { return true; }

inline bool Server::process_and_close_socket(socket_t sock) {
  std::string remote_addr;
  int remote_port = 0;
  detail::get_remote_ip_and_port(sock, remote_addr, remote_port);

  std::string local_addr;
  int local_port = 0;
  detail::get_local_ip_and_port(sock, local_addr, local_port);

  auto ret = detail::process_server_socket(
      svr_sock_, sock, keep_alive_max_count_, keep_alive_timeout_sec_,
      read_timeout_sec_, read_timeout_usec_, write_timeout_sec_,
      write_timeout_usec_,
      [&](Stream &strm, bool close_connection, bool &connection_closed) {
        return process_request(strm, remote_addr, remote_port, local_addr,
                               local_port, close_connection, connection_closed,
                               nullptr);
      });

  detail::shutdown_socket(sock);
  detail::close_socket(sock);
  return ret;
}

// HTTP client implementation
inline ClientImpl::ClientImpl(const std::string &host)
    : ClientImpl(host, 80, std::string(), std::string()) {}

inline ClientImpl::ClientImpl(const std::string &host, int port)
    : ClientImpl(host, port, std::string(), std::string()) {}

inline ClientImpl::ClientImpl(const std::string &host, int port,
                              const std::string &client_cert_path,
                              const std::string &client_key_path)
    : host_(detail::escape_abstract_namespace_unix_domain(host)), port_(port),
      host_and_port_(adjust_host_string(host_) + ":" + std::to_string(port)),
      client_cert_path_(client_cert_path), client_key_path_(client_key_path) {}

inline ClientImpl::~ClientImpl() {
  // Wait until all the requests in flight are handled.
  size_t retry_count = 10;
  while (retry_count-- > 0) {
    {
      std::lock_guard<std::mutex> guard(socket_mutex_);
      if (socket_requests_in_flight_ == 0) { break; }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{1});
  }

  std::lock_guard<std::mutex> guard(socket_mutex_);
  shutdown_socket(socket_);
  close_socket(socket_);
}

inline bool ClientImpl::is_valid() const { return true; }

inline void ClientImpl::copy_settings(const ClientImpl &rhs) {
  client_cert_path_ = rhs.client_cert_path_;
  client_key_path_ = rhs.client_key_path_;
  connection_timeout_sec_ = rhs.connection_timeout_sec_;
  read_timeout_sec_ = rhs.read_timeout_sec_;
  read_timeout_usec_ = rhs.read_timeout_usec_;
  write_timeout_sec_ = rhs.write_timeout_sec_;
  write_timeout_usec_ = rhs.write_timeout_usec_;
  max_timeout_msec_ = rhs.max_timeout_msec_;
  basic_auth_username_ = rhs.basic_auth_username_;
  basic_auth_password_ = rhs.basic_auth_password_;
  bearer_token_auth_token_ = rhs.bearer_token_auth_token_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  digest_auth_username_ = rhs.digest_auth_username_;
  digest_auth_password_ = rhs.digest_auth_password_;
#endif
  keep_alive_ = rhs.keep_alive_;
  follow_location_ = rhs.follow_location_;
  url_encode_ = rhs.url_encode_;
  address_family_ = rhs.address_family_;
  tcp_nodelay_ = rhs.tcp_nodelay_;
  ipv6_v6only_ = rhs.ipv6_v6only_;
  socket_options_ = rhs.socket_options_;
  compress_ = rhs.compress_;
  decompress_ = rhs.decompress_;
  interface_ = rhs.interface_;
  proxy_host_ = rhs.proxy_host_;
  proxy_port_ = rhs.proxy_port_;
  proxy_basic_auth_username_ = rhs.proxy_basic_auth_username_;
  proxy_basic_auth_password_ = rhs.proxy_basic_auth_password_;
  proxy_bearer_token_auth_token_ = rhs.proxy_bearer_token_auth_token_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  proxy_digest_auth_username_ = rhs.proxy_digest_auth_username_;
  proxy_digest_auth_password_ = rhs.proxy_digest_auth_password_;
#endif
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  ca_cert_file_path_ = rhs.ca_cert_file_path_;
  ca_cert_dir_path_ = rhs.ca_cert_dir_path_;
  ca_cert_store_ = rhs.ca_cert_store_;
#endif
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  server_certificate_verification_ = rhs.server_certificate_verification_;
  server_hostname_verification_ = rhs.server_hostname_verification_;
  server_certificate_verifier_ = rhs.server_certificate_verifier_;
#endif
  logger_ = rhs.logger_;
}

inline socket_t ClientImpl::create_client_socket(Error &error) const {
  if (!proxy_host_.empty() && proxy_port_ != -1) {
    return detail::create_client_socket(
        proxy_host_, std::string(), proxy_port_, address_family_, tcp_nodelay_,
        ipv6_v6only_, socket_options_, connection_timeout_sec_,
        connection_timeout_usec_, read_timeout_sec_, read_timeout_usec_,
        write_timeout_sec_, write_timeout_usec_, interface_, error);
  }

  // Check is custom IP specified for host_
  std::string ip;
  auto it = addr_map_.find(host_);
  if (it != addr_map_.end()) { ip = it->second; }

  return detail::create_client_socket(
      host_, ip, port_, address_family_, tcp_nodelay_, ipv6_v6only_,
      socket_options_, connection_timeout_sec_, connection_timeout_usec_,
      read_timeout_sec_, read_timeout_usec_, write_timeout_sec_,
      write_timeout_usec_, interface_, error);
}

inline bool ClientImpl::create_and_connect_socket(Socket &socket,
                                                  Error &error) {
  auto sock = create_client_socket(error);
  if (sock == INVALID_SOCKET) { return false; }
  socket.sock = sock;
  return true;
}

inline void ClientImpl::shutdown_ssl(Socket & /*socket*/,
                                     bool /*shutdown_gracefully*/) {
  // If there are any requests in flight from threads other than us, then it's
  // a thread-unsafe race because individual ssl* objects are not thread-safe.
  assert(socket_requests_in_flight_ == 0 ||
         socket_requests_are_from_thread_ == std::this_thread::get_id());
}

inline void ClientImpl::shutdown_socket(Socket &socket) const {
  if (socket.sock == INVALID_SOCKET) { return; }
  detail::shutdown_socket(socket.sock);
}

inline void ClientImpl::close_socket(Socket &socket) {
  // If there are requests in flight in another thread, usually closing
  // the socket will be fine and they will simply receive an error when
  // using the closed socket, but it is still a bug since rarely the OS
  // may reassign the socket id to be used for a new socket, and then
  // suddenly they will be operating on a live socket that is different
  // than the one they intended!
  assert(socket_requests_in_flight_ == 0 ||
         socket_requests_are_from_thread_ == std::this_thread::get_id());

  // It is also a bug if this happens while SSL is still active
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  assert(socket.ssl == nullptr);
#endif
  if (socket.sock == INVALID_SOCKET) { return; }
  detail::close_socket(socket.sock);
  socket.sock = INVALID_SOCKET;
}

inline bool ClientImpl::read_response_line(Stream &strm, const Request &req,
                                           Response &res) const {
  std::array<char, 2048> buf{};

  detail::stream_line_reader line_reader(strm, buf.data(), buf.size());

  if (!line_reader.getline()) { return false; }

#ifdef CPPHTTPLIB_ALLOW_LF_AS_LINE_TERMINATOR
  thread_local const std::regex re("(HTTP/1\\.[01]) (\\d{3})(?: (.*?))?\r?\n");
#else
  thread_local const std::regex re("(HTTP/1\\.[01]) (\\d{3})(?: (.*?))?\r\n");
#endif

  std::cmatch m;
  if (!std::regex_match(line_reader.ptr(), m, re)) {
    return req.method == "CONNECT";
  }
  res.version = std::string(m[1]);
  res.status = std::stoi(std::string(m[2]));
  res.reason = std::string(m[3]);

  // Ignore '100 Continue'
  while (res.status == StatusCode::Continue_100) {
    if (!line_reader.getline()) { return false; } // CRLF
    if (!line_reader.getline()) { return false; } // next response line

    if (!std::regex_match(line_reader.ptr(), m, re)) { return false; }
    res.version = std::string(m[1]);
    res.status = std::stoi(std::string(m[2]));
    res.reason = std::string(m[3]);
  }

  return true;
}

inline bool ClientImpl::send(Request &req, Response &res, Error &error) {
  std::lock_guard<std::recursive_mutex> request_mutex_guard(request_mutex_);
  auto ret = send_(req, res, error);
  if (error == Error::SSLPeerCouldBeClosed_) {
    assert(!ret);
    ret = send_(req, res, error);
  }
  return ret;
}

inline bool ClientImpl::send_(Request &req, Response &res, Error &error) {
  {
    std::lock_guard<std::mutex> guard(socket_mutex_);

    // Set this to false immediately - if it ever gets set to true by the end of
    // the request, we know another thread instructed us to close the socket.
    socket_should_be_closed_when_request_is_done_ = false;

    auto is_alive = false;
    if (socket_.is_open()) {
      is_alive = detail::is_socket_alive(socket_.sock);

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
      if (is_alive && is_ssl()) {
        if (detail::is_ssl_peer_could_be_closed(socket_.ssl, socket_.sock)) {
          is_alive = false;
        }
      }
#endif

      if (!is_alive) {
        // Attempt to avoid sigpipe by shutting down non-gracefully if it seems
        // like the other side has already closed the connection Also, there
        // cannot be any requests in flight from other threads since we locked
        // request_mutex_, so safe to close everything immediately
        const bool shutdown_gracefully = false;
        shutdown_ssl(socket_, shutdown_gracefully);
        shutdown_socket(socket_);
        close_socket(socket_);
      }
    }

    if (!is_alive) {
      if (!create_and_connect_socket(socket_, error)) { return false; }

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
      // TODO: refactoring
      if (is_ssl()) {
        auto &scli = static_cast<SSLClient &>(*this);
        if (!proxy_host_.empty() && proxy_port_ != -1) {
          auto success = false;
          if (!scli.connect_with_proxy(socket_, req.start_time_, res, success,
                                       error)) {
            return success;
          }
        }

        if (!scli.initialize_ssl(socket_, error)) { return false; }
      }
#endif
    }

    // Mark the current socket as being in use so that it cannot be closed by
    // anyone else while this request is ongoing, even though we will be
    // releasing the mutex.
    if (socket_requests_in_flight_ > 1) {
      assert(socket_requests_are_from_thread_ == std::this_thread::get_id());
    }
    socket_requests_in_flight_ += 1;
    socket_requests_are_from_thread_ = std::this_thread::get_id();
  }

  for (const auto &header : default_headers_) {
    if (req.headers.find(header.first) == req.headers.end()) {
      req.headers.insert(header);
    }
  }

  auto ret = false;
  auto close_connection = !keep_alive_;

  auto se = detail::scope_exit([&]() {
    // Briefly lock mutex in order to mark that a request is no longer ongoing
    std::lock_guard<std::mutex> guard(socket_mutex_);
    socket_requests_in_flight_ -= 1;
    if (socket_requests_in_flight_ <= 0) {
      assert(socket_requests_in_flight_ == 0);
      socket_requests_are_from_thread_ = std::thread::id();
    }

    if (socket_should_be_closed_when_request_is_done_ || close_connection ||
        !ret) {
      shutdown_ssl(socket_, true);
      shutdown_socket(socket_);
      close_socket(socket_);
    }
  });

  ret = process_socket(socket_, req.start_time_, [&](Stream &strm) {
    return handle_request(strm, req, res, close_connection, error);
  });

  if (!ret) {
    if (error == Error::Success) { error = Error::Unknown; }
  }

  return ret;
}

inline Result ClientImpl::send(const Request &req) {
  auto req2 = req;
  return send_(std::move(req2));
}

inline Result ClientImpl::send_(Request &&req) {
  auto res = detail::make_unique<Response>();
  auto error = Error::Success;
  auto ret = send(req, *res, error);
  return Result{ret ? std::move(res) : nullptr, error, std::move(req.headers)};
}

inline bool ClientImpl::handle_request(Stream &strm, Request &req,
                                       Response &res, bool close_connection,
                                       Error &error) {
  if (req.path.empty()) {
    error = Error::Connection;
    return false;
  }

  auto req_save = req;

  bool ret;

  if (!is_ssl() && !proxy_host_.empty() && proxy_port_ != -1) {
    auto req2 = req;
    req2.path = "http://" + host_and_port_ + req.path;
    ret = process_request(strm, req2, res, close_connection, error);
    req = req2;
    req.path = req_save.path;
  } else {
    ret = process_request(strm, req, res, close_connection, error);
  }

  if (!ret) { return false; }

  if (res.get_header_value("Connection") == "close" ||
      (res.version == "HTTP/1.0" && res.reason != "Connection established")) {
    // TODO this requires a not-entirely-obvious chain of calls to be correct
    // for this to be safe.

    // This is safe to call because handle_request is only called by send_
    // which locks the request mutex during the process. It would be a bug
    // to call it from a different thread since it's a thread-safety issue
    // to do these things to the socket if another thread is using the socket.
    std::lock_guard<std::mutex> guard(socket_mutex_);
    shutdown_ssl(socket_, true);
    shutdown_socket(socket_);
    close_socket(socket_);
  }

  if (300 < res.status && res.status < 400 && follow_location_) {
    req = req_save;
    ret = redirect(req, res, error);
  }

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  if ((res.status == StatusCode::Unauthorized_401 ||
       res.status == StatusCode::ProxyAuthenticationRequired_407) &&
      req.authorization_count_ < 5) {
    auto is_proxy = res.status == StatusCode::ProxyAuthenticationRequired_407;
    const auto &username =
        is_proxy ? proxy_digest_auth_username_ : digest_auth_username_;
    const auto &password =
        is_proxy ? proxy_digest_auth_password_ : digest_auth_password_;

    if (!username.empty() && !password.empty()) {
      std::map<std::string, std::string> auth;
      if (detail::parse_www_authenticate(res, auth, is_proxy)) {
        Request new_req = req;
        new_req.authorization_count_ += 1;
        new_req.headers.erase(is_proxy ? "Proxy-Authorization"
                                       : "Authorization");
        new_req.headers.insert(detail::make_digest_authentication_header(
            req, auth, new_req.authorization_count_, detail::random_string(10),
            username, password, is_proxy));

        Response new_res;

        ret = send(new_req, new_res, error);
        if (ret) { res = new_res; }
      }
    }
  }
#endif

  return ret;
}

inline bool ClientImpl::redirect(Request &req, Response &res, Error &error) {
  if (req.redirect_count_ == 0) {
    error = Error::ExceedRedirectCount;
    return false;
  }

  auto location = res.get_header_value("location");
  if (location.empty()) { return false; }

  thread_local const std::regex re(
      R"((?:(https?):)?(?://(?:\[([a-fA-F\d:]+)\]|([^:/?#]+))(?::(\d+))?)?([^?#]*)(\?[^#]*)?(?:#.*)?)");

  std::smatch m;
  if (!std::regex_match(location, m, re)) { return false; }

  auto scheme = is_ssl() ? "https" : "http";

  auto next_scheme = m[1].str();
  auto next_host = m[2].str();
  if (next_host.empty()) { next_host = m[3].str(); }
  auto port_str = m[4].str();
  auto next_path = m[5].str();
  auto next_query = m[6].str();

  auto next_port = port_;
  if (!port_str.empty()) {
    next_port = std::stoi(port_str);
  } else if (!next_scheme.empty()) {
    next_port = next_scheme == "https" ? 443 : 80;
  }

  if (next_scheme.empty()) { next_scheme = scheme; }
  if (next_host.empty()) { next_host = host_; }
  if (next_path.empty()) { next_path = "/"; }

  auto path = detail::decode_url(next_path, true) + next_query;

  if (next_scheme == scheme && next_host == host_ && next_port == port_) {
    return detail::redirect(*this, req, res, path, location, error);
  } else {
    if (next_scheme == "https") {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
      SSLClient cli(next_host, next_port);
      cli.copy_settings(*this);
      if (ca_cert_store_) { cli.set_ca_cert_store(ca_cert_store_); }
      return detail::redirect(cli, req, res, path, location, error);
#else
      return false;
#endif
    } else {
      ClientImpl cli(next_host, next_port);
      cli.copy_settings(*this);
      return detail::redirect(cli, req, res, path, location, error);
    }
  }
}

inline bool ClientImpl::write_content_with_provider(Stream &strm,
                                                    const Request &req,
                                                    Error &error) const {
  auto is_shutting_down = []() { return false; };

  if (req.is_chunked_content_provider_) {
    // TODO: Brotli support
    std::unique_ptr<detail::compressor> compressor;
#ifdef CPPHTTPLIB_ZLIB_SUPPORT
    if (compress_) {
      compressor = detail::make_unique<detail::gzip_compressor>();
    } else
#endif
    {
      compressor = detail::make_unique<detail::nocompressor>();
    }

    return detail::write_content_chunked(strm, req.content_provider_,
                                         is_shutting_down, *compressor, error);
  } else {
    return detail::write_content(strm, req.content_provider_, 0,
                                 req.content_length_, is_shutting_down, error);
  }
}

inline bool ClientImpl::write_request(Stream &strm, Request &req,
                                      bool close_connection, Error &error) {
  // Prepare additional headers
  if (close_connection) {
    if (!req.has_header("Connection")) {
      req.set_header("Connection", "close");
    }
  }

  if (!req.has_header("Host")) {
    if (is_ssl()) {
      if (port_ == 443) {
        req.set_header("Host", host_);
      } else {
        req.set_header("Host", host_and_port_);
      }
    } else {
      if (port_ == 80) {
        req.set_header("Host", host_);
      } else {
        req.set_header("Host", host_and_port_);
      }
    }
  }

  if (!req.has_header("Accept")) { req.set_header("Accept", "*/*"); }

  if (!req.content_receiver) {
    if (!req.has_header("Accept-Encoding")) {
      std::string accept_encoding;
#ifdef CPPHTTPLIB_BROTLI_SUPPORT
      accept_encoding = "br";
#endif
#ifdef CPPHTTPLIB_ZLIB_SUPPORT
      if (!accept_encoding.empty()) { accept_encoding += ", "; }
      accept_encoding += "gzip, deflate";
#endif
#ifdef CPPHTTPLIB_ZSTD_SUPPORT
      if (!accept_encoding.empty()) { accept_encoding += ", "; }
      accept_encoding += "zstd";
#endif
      req.set_header("Accept-Encoding", accept_encoding);
    }

#ifndef CPPHTTPLIB_NO_DEFAULT_USER_AGENT
    if (!req.has_header("User-Agent")) {
      auto agent = std::string("cpp-httplib/") + CPPHTTPLIB_VERSION;
      req.set_header("User-Agent", agent);
    }
#endif
  };

  if (req.body.empty()) {
    if (req.content_provider_) {
      if (!req.is_chunked_content_provider_) {
        if (!req.has_header("Content-Length")) {
          auto length = std::to_string(req.content_length_);
          req.set_header("Content-Length", length);
        }
      }
    } else {
      if (req.method == "POST" || req.method == "PUT" ||
          req.method == "PATCH") {
        req.set_header("Content-Length", "0");
      }
    }
  } else {
    if (!req.has_header("Content-Type")) {
      req.set_header("Content-Type", "text/plain");
    }

    if (!req.has_header("Content-Length")) {
      auto length = std::to_string(req.body.size());
      req.set_header("Content-Length", length);
    }
  }

  if (!basic_auth_password_.empty() || !basic_auth_username_.empty()) {
    if (!req.has_header("Authorization")) {
      req.headers.insert(make_basic_authentication_header(
          basic_auth_username_, basic_auth_password_, false));
    }
  }

  if (!proxy_basic_auth_username_.empty() &&
      !proxy_basic_auth_password_.empty()) {
    if (!req.has_header("Proxy-Authorization")) {
      req.headers.insert(make_basic_authentication_header(
          proxy_basic_auth_username_, proxy_basic_auth_password_, true));
    }
  }

  if (!bearer_token_auth_token_.empty()) {
    if (!req.has_header("Authorization")) {
      req.headers.insert(make_bearer_token_authentication_header(
          bearer_token_auth_token_, false));
    }
  }

  if (!proxy_bearer_token_auth_token_.empty()) {
    if (!req.has_header("Proxy-Authorization")) {
      req.headers.insert(make_bearer_token_authentication_header(
          proxy_bearer_token_auth_token_, true));
    }
  }

  // Request line and headers
  {
    detail::BufferStream bstrm;

    const auto &path_with_query =
        req.params.empty() ? req.path
                           : append_query_params(req.path, req.params);

    const auto &path =
        url_encode_ ? detail::encode_url(path_with_query) : path_with_query;

    detail::write_request_line(bstrm, req.method, path);

    header_writer_(bstrm, req.headers);

    // Flush buffer
    auto &data = bstrm.get_buffer();
    if (!detail::write_data(strm, data.data(), data.size())) {
      error = Error::Write;
      return false;
    }
  }

  // Body
  if (req.body.empty()) {
    return write_content_with_provider(strm, req, error);
  }

  if (!detail::write_data(strm, req.body.data(), req.body.size())) {
    error = Error::Write;
    return false;
  }

  return true;
}

inline std::unique_ptr<Response> ClientImpl::send_with_content_provider(
    Request &req, const char *body, size_t content_length,
    ContentProvider content_provider,
    ContentProviderWithoutLength content_provider_without_length,
    const std::string &content_type, Error &error) {
  if (!content_type.empty()) { req.set_header("Content-Type", content_type); }

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
  if (compress_) { req.set_header("Content-Encoding", "gzip"); }
#endif

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
  if (compress_ && !content_provider_without_length) {
    // TODO: Brotli support
    detail::gzip_compressor compressor;

    if (content_provider) {
      auto ok = true;
      size_t offset = 0;
      DataSink data_sink;

      data_sink.write = [&](const char *data, size_t data_len) -> bool {
        if (ok) {
          auto last = offset + data_len == content_length;

          auto ret = compressor.compress(
              data, data_len, last,
              [&](const char *compressed_data, size_t compressed_data_len) {
                req.body.append(compressed_data, compressed_data_len);
                return true;
              });

          if (ret) {
            offset += data_len;
          } else {
            ok = false;
          }
        }
        return ok;
      };

      while (ok && offset < content_length) {
        if (!content_provider(offset, content_length - offset, data_sink)) {
          error = Error::Canceled;
          return nullptr;
        }
      }
    } else {
      if (!compressor.compress(body, content_length, true,
                               [&](const char *data, size_t data_len) {
                                 req.body.append(data, data_len);
                                 return true;
                               })) {
        error = Error::Compression;
        return nullptr;
      }
    }
  } else
#endif
  {
    if (content_provider) {
      req.content_length_ = content_length;
      req.content_provider_ = std::move(content_provider);
      req.is_chunked_content_provider_ = false;
    } else if (content_provider_without_length) {
      req.content_length_ = 0;
      req.content_provider_ = detail::ContentProviderAdapter(
          std::move(content_provider_without_length));
      req.is_chunked_content_provider_ = true;
      req.set_header("Transfer-Encoding", "chunked");
    } else {
      req.body.assign(body, content_length);
    }
  }

  auto res = detail::make_unique<Response>();
  return send(req, *res, error) ? std::move(res) : nullptr;
}

inline Result ClientImpl::send_with_content_provider(
    const std::string &method, const std::string &path, const Headers &headers,
    const char *body, size_t content_length, ContentProvider content_provider,
    ContentProviderWithoutLength content_provider_without_length,
    const std::string &content_type, Progress progress) {
  Request req;
  req.method = method;
  req.headers = headers;
  req.path = path;
  req.progress = progress;
  if (max_timeout_msec_ > 0) {
    req.start_time_ = std::chrono::steady_clock::now();
  }

  auto error = Error::Success;

  auto res = send_with_content_provider(
      req, body, content_length, std::move(content_provider),
      std::move(content_provider_without_length), content_type, error);

  return Result{std::move(res), error, std::move(req.headers)};
}

inline std::string
ClientImpl::adjust_host_string(const std::string &host) const {
  if (host.find(':') != std::string::npos) { return "[" + host + "]"; }
  return host;
}

inline bool ClientImpl::process_request(Stream &strm, Request &req,
                                        Response &res, bool close_connection,
                                        Error &error) {
  // Send request
  if (!write_request(strm, req, close_connection, error)) { return false; }

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  if (is_ssl()) {
    auto is_proxy_enabled = !proxy_host_.empty() && proxy_port_ != -1;
    if (!is_proxy_enabled) {
      if (detail::is_ssl_peer_could_be_closed(socket_.ssl, socket_.sock)) {
        error = Error::SSLPeerCouldBeClosed_;
        return false;
      }
    }
  }
#endif

  // Receive response and headers
  if (!read_response_line(strm, req, res) ||
      !detail::read_headers(strm, res.headers)) {
    error = Error::Read;
    return false;
  }

  // Body
  if ((res.status != StatusCode::NoContent_204) && req.method != "HEAD" &&
      req.method != "CONNECT") {
    auto redirect = 300 < res.status && res.status < 400 &&
                    res.status != StatusCode::NotModified_304 &&
                    follow_location_;

    if (req.response_handler && !redirect) {
      if (!req.response_handler(res)) {
        error = Error::Canceled;
        return false;
      }
    }

    auto out =
        req.content_receiver
            ? static_cast<ContentReceiverWithProgress>(
                  [&](const char *buf, size_t n, uint64_t off, uint64_t len) {
                    if (redirect) { return true; }
                    auto ret = req.content_receiver(buf, n, off, len);
                    if (!ret) { error = Error::Canceled; }
                    return ret;
                  })
            : static_cast<ContentReceiverWithProgress>(
                  [&](const char *buf, size_t n, uint64_t /*off*/,
                      uint64_t /*len*/) {
                    assert(res.body.size() + n <= res.body.max_size());
                    res.body.append(buf, n);
                    return true;
                  });

    auto progress = [&](uint64_t current, uint64_t total) {
      if (!req.progress || redirect) { return true; }
      auto ret = req.progress(current, total);
      if (!ret) { error = Error::Canceled; }
      return ret;
    };

    if (res.has_header("Content-Length")) {
      if (!req.content_receiver) {
        auto len = res.get_header_value_u64("Content-Length");
        if (len > res.body.max_size()) {
          error = Error::Read;
          return false;
        }
        res.body.reserve(static_cast<size_t>(len));
      }
    }

    if (res.status != StatusCode::NotModified_304) {
      int dummy_status;
      if (!detail::read_content(strm, res, (std::numeric_limits<size_t>::max)(),
                                dummy_status, std::move(progress),
                                std::move(out), decompress_)) {
        if (error != Error::Canceled) { error = Error::Read; }
        return false;
      }
    }
  }

  // Log
  if (logger_) { logger_(req, res); }

  return true;
}

inline ContentProviderWithoutLength ClientImpl::get_multipart_content_provider(
    const std::string &boundary, const MultipartFormDataItems &items,
    const MultipartFormDataProviderItems &provider_items) const {
  size_t cur_item = 0;
  size_t cur_start = 0;
  // cur_item and cur_start are copied to within the std::function and maintain
  // state between successive calls
  return [&, cur_item, cur_start](size_t offset,
                                  DataSink &sink) mutable -> bool {
    if (!offset && !items.empty()) {
      sink.os << detail::serialize_multipart_formdata(items, boundary, false);
      return true;
    } else if (cur_item < provider_items.size()) {
      if (!cur_start) {
        const auto &begin = detail::serialize_multipart_formdata_item_begin(
            provider_items[cur_item], boundary);
        offset += begin.size();
        cur_start = offset;
        sink.os << begin;
      }

      DataSink cur_sink;
      auto has_data = true;
      cur_sink.write = sink.write;
      cur_sink.done = [&]() { has_data = false; };

      if (!provider_items[cur_item].provider(offset - cur_start, cur_sink)) {
        return false;
      }

      if (!has_data) {
        sink.os << detail::serialize_multipart_formdata_item_end();
        cur_item++;
        cur_start = 0;
      }
      return true;
    } else {
      sink.os << detail::serialize_multipart_formdata_finish(boundary);
      sink.done();
      return true;
    }
  };
}

inline bool ClientImpl::process_socket(
    const Socket &socket,
    std::chrono::time_point<std::chrono::steady_clock> start_time,
    std::function<bool(Stream &strm)> callback) {
  return detail::process_client_socket(
      socket.sock, read_timeout_sec_, read_timeout_usec_, write_timeout_sec_,
      write_timeout_usec_, max_timeout_msec_, start_time, std::move(callback));
}

inline bool ClientImpl::is_ssl() const { return false; }

inline Result ClientImpl::Get(const std::string &path) {
  return Get(path, Headers(), Progress());
}

inline Result ClientImpl::Get(const std::string &path, Progress progress) {
  return Get(path, Headers(), std::move(progress));
}

inline Result ClientImpl::Get(const std::string &path, const Headers &headers) {
  return Get(path, headers, Progress());
}

inline Result ClientImpl::Get(const std::string &path, const Headers &headers,
                              Progress progress) {
  Request req;
  req.method = "GET";
  req.path = path;
  req.headers = headers;
  req.progress = std::move(progress);
  if (max_timeout_msec_ > 0) {
    req.start_time_ = std::chrono::steady_clock::now();
  }

  return send_(std::move(req));
}

inline Result ClientImpl::Get(const std::string &path,
                              ContentReceiver content_receiver) {
  return Get(path, Headers(), nullptr, std::move(content_receiver), nullptr);
}

inline Result ClientImpl::Get(const std::string &path,
                              ContentReceiver content_receiver,
                              Progress progress) {
  return Get(path, Headers(), nullptr, std::move(content_receiver),
             std::move(progress));
}

inline Result ClientImpl::Get(const std::string &path, const Headers &headers,
                              ContentReceiver content_receiver) {
  return Get(path, headers, nullptr, std::move(content_receiver), nullptr);
}

inline Result ClientImpl::Get(const std::string &path, const Headers &headers,
                              ContentReceiver content_receiver,
                              Progress progress) {
  return Get(path, headers, nullptr, std::move(content_receiver),
             std::move(progress));
}

inline Result ClientImpl::Get(const std::string &path,
                              ResponseHandler response_handler,
                              ContentReceiver content_receiver) {
  return Get(path, Headers(), std::move(response_handler),
             std::move(content_receiver), nullptr);
}

inline Result ClientImpl::Get(const std::string &path, const Headers &headers,
                              ResponseHandler response_handler,
                              ContentReceiver content_receiver) {
  return Get(path, headers, std::move(response_handler),
             std::move(content_receiver), nullptr);
}

inline Result ClientImpl::Get(const std::string &path,
                              ResponseHandler response_handler,
                              ContentReceiver content_receiver,
                              Progress progress) {
  return Get(path, Headers(), std::move(response_handler),
             std::move(content_receiver), std::move(progress));
}

inline Result ClientImpl::Get(const std::string &path, const Headers &headers,
                              ResponseHandler response_handler,
                              ContentReceiver content_receiver,
                              Progress progress) {
  Request req;
  req.method = "GET";
  req.path = path;
  req.headers = headers;
  req.response_handler = std::move(response_handler);
  req.content_receiver =
      [content_receiver](const char *data, size_t data_length,
                         uint64_t /*offset*/, uint64_t /*total_length*/) {
        return content_receiver(data, data_length);
      };
  req.progress = std::move(progress);
  if (max_timeout_msec_ > 0) {
    req.start_time_ = std::chrono::steady_clock::now();
  }

  return send_(std::move(req));
}

inline Result ClientImpl::Get(const std::string &path, const Params &params,
                              const Headers &headers, Progress progress) {
  if (params.empty()) { return Get(path, headers); }

  std::string path_with_query = append_query_params(path, params);
  return Get(path_with_query, headers, std::move(progress));
}

inline Result ClientImpl::Get(const std::string &path, const Params &params,
                              const Headers &headers,
                              ContentReceiver content_receiver,
                              Progress progress) {
  return Get(path, params, headers, nullptr, std::move(content_receiver),
             std::move(progress));
}

inline Result ClientImpl::Get(const std::string &path, const Params &params,
                              const Headers &headers,
                              ResponseHandler response_handler,
                              ContentReceiver content_receiver,
                              Progress progress) {
  if (params.empty()) {
    return Get(path, headers, std::move(response_handler),
               std::move(content_receiver), std::move(progress));
  }

  std::string path_with_query = append_query_params(path, params);
  return Get(path_with_query, headers, std::move(response_handler),
             std::move(content_receiver), std::move(progress));
}

inline Result ClientImpl::Head(const std::string &path) {
  return Head(path, Headers());
}

inline Result ClientImpl::Head(const std::string &path,
                               const Headers &headers) {
  Request req;
  req.method = "HEAD";
  req.headers = headers;
  req.path = path;
  if (max_timeout_msec_ > 0) {
    req.start_time_ = std::chrono::steady_clock::now();
  }

  return send_(std::move(req));
}

inline Result ClientImpl::Post(const std::string &path) {
  return Post(path, std::string(), std::string());
}

inline Result ClientImpl::Post(const std::string &path,
                               const Headers &headers) {
  return Post(path, headers, nullptr, 0, std::string());
}

inline Result ClientImpl::Post(const std::string &path, const char *body,
                               size_t content_length,
                               const std::string &content_type) {
  return Post(path, Headers(), body, content_length, content_type, nullptr);
}

inline Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               const char *body, size_t content_length,
                               const std::string &content_type) {
  return send_with_content_provider("POST", path, headers, body, content_length,
                                    nullptr, nullptr, content_type, nullptr);
}

inline Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               const char *body, size_t content_length,
                               const std::string &content_type,
                               Progress progress) {
  return send_with_content_provider("POST", path, headers, body, content_length,
                                    nullptr, nullptr, content_type, progress);
}

inline Result ClientImpl::Post(const std::string &path, const std::string &body,
                               const std::string &content_type) {
  return Post(path, Headers(), body, content_type);
}

inline Result ClientImpl::Post(const std::string &path, const std::string &body,
                               const std::string &content_type,
                               Progress progress) {
  return Post(path, Headers(), body, content_type, progress);
}

inline Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               const std::string &body,
                               const std::string &content_type) {
  return send_with_content_provider("POST", path, headers, body.data(),
                                    body.size(), nullptr, nullptr, content_type,
                                    nullptr);
}

inline Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               const std::string &body,
                               const std::string &content_type,
                               Progress progress) {
  return send_with_content_provider("POST", path, headers, body.data(),
                                    body.size(), nullptr, nullptr, content_type,
                                    progress);
}

inline Result ClientImpl::Post(const std::string &path, const Params &params) {
  return Post(path, Headers(), params);
}

inline Result ClientImpl::Post(const std::string &path, size_t content_length,
                               ContentProvider content_provider,
                               const std::string &content_type) {
  return Post(path, Headers(), content_length, std::move(content_provider),
              content_type);
}

inline Result ClientImpl::Post(const std::string &path,
                               ContentProviderWithoutLength content_provider,
                               const std::string &content_type) {
  return Post(path, Headers(), std::move(content_provider), content_type);
}

inline Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               size_t content_length,
                               ContentProvider content_provider,
                               const std::string &content_type) {
  return send_with_content_provider("POST", path, headers, nullptr,
                                    content_length, std::move(content_provider),
                                    nullptr, content_type, nullptr);
}

inline Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               ContentProviderWithoutLength content_provider,
                               const std::string &content_type) {
  return send_with_content_provider("POST", path, headers, nullptr, 0, nullptr,
                                    std::move(content_provider), content_type,
                                    nullptr);
}

inline Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               const Params &params) {
  auto query = detail::params_to_query_str(params);
  return Post(path, headers, query, "application/x-www-form-urlencoded");
}

inline Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               const Params &params, Progress progress) {
  auto query = detail::params_to_query_str(params);
  return Post(path, headers, query, "application/x-www-form-urlencoded",
              progress);
}

inline Result ClientImpl::Post(const std::string &path,
                               const MultipartFormDataItems &items) {
  return Post(path, Headers(), items);
}

inline Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               const MultipartFormDataItems &items) {
  const auto &boundary = detail::make_multipart_data_boundary();
  const auto &content_type =
      detail::serialize_multipart_formdata_get_content_type(boundary);
  const auto &body = detail::serialize_multipart_formdata(items, boundary);
  return Post(path, headers, body, content_type);
}

inline Result ClientImpl::Post(const std::string &path, const Headers &headers,
                               const MultipartFormDataItems &items,
                               const std::string &boundary) {
  if (!detail::is_multipart_boundary_chars_valid(boundary)) {
    return Result{nullptr, Error::UnsupportedMultipartBoundaryChars};
  }

  const auto &content_type =
      detail::serialize_multipart_formdata_get_content_type(boundary);
  const auto &body = detail::serialize_multipart_formdata(items, boundary);
  return Post(path, headers, body, content_type);
}

inline Result
ClientImpl::Post(const std::string &path, const Headers &headers,
                 const MultipartFormDataItems &items,
                 const MultipartFormDataProviderItems &provider_items) {
  const auto &boundary = detail::make_multipart_data_boundary();
  const auto &content_type =
      detail::serialize_multipart_formdata_get_content_type(boundary);
  return send_with_content_provider(
      "POST", path, headers, nullptr, 0, nullptr,
      get_multipart_content_provider(boundary, items, provider_items),
      content_type, nullptr);
}

inline Result ClientImpl::Put(const std::string &path) {
  return Put(path, std::string(), std::string());
}

inline Result ClientImpl::Put(const std::string &path, const char *body,
                              size_t content_length,
                              const std::string &content_type) {
  return Put(path, Headers(), body, content_length, content_type);
}

inline Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              const char *body, size_t content_length,
                              const std::string &content_type) {
  return send_with_content_provider("PUT", path, headers, body, content_length,
                                    nullptr, nullptr, content_type, nullptr);
}

inline Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              const char *body, size_t content_length,
                              const std::string &content_type,
                              Progress progress) {
  return send_with_content_provider("PUT", path, headers, body, content_length,
                                    nullptr, nullptr, content_type, progress);
}

inline Result ClientImpl::Put(const std::string &path, const std::string &body,
                              const std::string &content_type) {
  return Put(path, Headers(), body, content_type);
}

inline Result ClientImpl::Put(const std::string &path, const std::string &body,
                              const std::string &content_type,
                              Progress progress) {
  return Put(path, Headers(), body, content_type, progress);
}

inline Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              const std::string &body,
                              const std::string &content_type) {
  return send_with_content_provider("PUT", path, headers, body.data(),
                                    body.size(), nullptr, nullptr, content_type,
                                    nullptr);
}

inline Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              const std::string &body,
                              const std::string &content_type,
                              Progress progress) {
  return send_with_content_provider("PUT", path, headers, body.data(),
                                    body.size(), nullptr, nullptr, content_type,
                                    progress);
}

inline Result ClientImpl::Put(const std::string &path, size_t content_length,
                              ContentProvider content_provider,
                              const std::string &content_type) {
  return Put(path, Headers(), content_length, std::move(content_provider),
             content_type);
}

inline Result ClientImpl::Put(const std::string &path,
                              ContentProviderWithoutLength content_provider,
                              const std::string &content_type) {
  return Put(path, Headers(), std::move(content_provider), content_type);
}

inline Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              size_t content_length,
                              ContentProvider content_provider,
                              const std::string &content_type) {
  return send_with_content_provider("PUT", path, headers, nullptr,
                                    content_length, std::move(content_provider),
                                    nullptr, content_type, nullptr);
}

inline Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              ContentProviderWithoutLength content_provider,
                              const std::string &content_type) {
  return send_with_content_provider("PUT", path, headers, nullptr, 0, nullptr,
                                    std::move(content_provider), content_type,
                                    nullptr);
}

inline Result ClientImpl::Put(const std::string &path, const Params &params) {
  return Put(path, Headers(), params);
}

inline Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              const Params &params) {
  auto query = detail::params_to_query_str(params);
  return Put(path, headers, query, "application/x-www-form-urlencoded");
}

inline Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              const Params &params, Progress progress) {
  auto query = detail::params_to_query_str(params);
  return Put(path, headers, query, "application/x-www-form-urlencoded",
             progress);
}

inline Result ClientImpl::Put(const std::string &path,
                              const MultipartFormDataItems &items) {
  return Put(path, Headers(), items);
}

inline Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              const MultipartFormDataItems &items) {
  const auto &boundary = detail::make_multipart_data_boundary();
  const auto &content_type =
      detail::serialize_multipart_formdata_get_content_type(boundary);
  const auto &body = detail::serialize_multipart_formdata(items, boundary);
  return Put(path, headers, body, content_type);
}

inline Result ClientImpl::Put(const std::string &path, const Headers &headers,
                              const MultipartFormDataItems &items,
                              const std::string &boundary) {
  if (!detail::is_multipart_boundary_chars_valid(boundary)) {
    return Result{nullptr, Error::UnsupportedMultipartBoundaryChars};
  }

  const auto &content_type =
      detail::serialize_multipart_formdata_get_content_type(boundary);
  const auto &body = detail::serialize_multipart_formdata(items, boundary);
  return Put(path, headers, body, content_type);
}

inline Result
ClientImpl::Put(const std::string &path, const Headers &headers,
                const MultipartFormDataItems &items,
                const MultipartFormDataProviderItems &provider_items) {
  const auto &boundary = detail::make_multipart_data_boundary();
  const auto &content_type =
      detail::serialize_multipart_formdata_get_content_type(boundary);
  return send_with_content_provider(
      "PUT", path, headers, nullptr, 0, nullptr,
      get_multipart_content_provider(boundary, items, provider_items),
      content_type, nullptr);
}
inline Result ClientImpl::Patch(const std::string &path) {
  return Patch(path, std::string(), std::string());
}

inline Result ClientImpl::Patch(const std::string &path, const char *body,
                                size_t content_length,
                                const std::string &content_type) {
  return Patch(path, Headers(), body, content_length, content_type);
}

inline Result ClientImpl::Patch(const std::string &path, const char *body,
                                size_t content_length,
                                const std::string &content_type,
                                Progress progress) {
  return Patch(path, Headers(), body, content_length, content_type, progress);
}

inline Result ClientImpl::Patch(const std::string &path, const Headers &headers,
                                const char *body, size_t content_length,
                                const std::string &content_type) {
  return Patch(path, headers, body, content_length, content_type, nullptr);
}

inline Result ClientImpl::Patch(const std::string &path, const Headers &headers,
                                const char *body, size_t content_length,
                                const std::string &content_type,
                                Progress progress) {
  return send_with_content_provider("PATCH", path, headers, body,
                                    content_length, nullptr, nullptr,
                                    content_type, progress);
}

inline Result ClientImpl::Patch(const std::string &path,
                                const std::string &body,
                                const std::string &content_type) {
  return Patch(path, Headers(), body, content_type);
}

inline Result ClientImpl::Patch(const std::string &path,
                                const std::string &body,
                                const std::string &content_type,
                                Progress progress) {
  return Patch(path, Headers(), body, content_type, progress);
}

inline Result ClientImpl::Patch(const std::string &path, const Headers &headers,
                                const std::string &body,
                                const std::string &content_type) {
  return Patch(path, headers, body, content_type, nullptr);
}

inline Result ClientImpl::Patch(const std::string &path, const Headers &headers,
                                const std::string &body,
                                const std::string &content_type,
                                Progress progress) {
  return send_with_content_provider("PATCH", path, headers, body.data(),
                                    body.size(), nullptr, nullptr, content_type,
                                    progress);
}

inline Result ClientImpl::Patch(const std::string &path, size_t content_length,
                                ContentProvider content_provider,
                                const std::string &content_type) {
  return Patch(path, Headers(), content_length, std::move(content_provider),
               content_type);
}

inline Result ClientImpl::Patch(const std::string &path,
                                ContentProviderWithoutLength content_provider,
                                const std::string &content_type) {
  return Patch(path, Headers(), std::move(content_provider), content_type);
}

inline Result ClientImpl::Patch(const std::string &path, const Headers &headers,
                                size_t content_length,
                                ContentProvider content_provider,
                                const std::string &content_type) {
  return send_with_content_provider("PATCH", path, headers, nullptr,
                                    content_length, std::move(content_provider),
                                    nullptr, content_type, nullptr);
}

inline Result ClientImpl::Patch(const std::string &path, const Headers &headers,
                                ContentProviderWithoutLength content_provider,
                                const std::string &content_type) {
  return send_with_content_provider("PATCH", path, headers, nullptr, 0, nullptr,
                                    std::move(content_provider), content_type,
                                    nullptr);
}

inline Result ClientImpl::Delete(const std::string &path) {
  return Delete(path, Headers(), std::string(), std::string());
}

inline Result ClientImpl::Delete(const std::string &path,
                                 const Headers &headers) {
  return Delete(path, headers, std::string(), std::string());
}

inline Result ClientImpl::Delete(const std::string &path, const char *body,
                                 size_t content_length,
                                 const std::string &content_type) {
  return Delete(path, Headers(), body, content_length, content_type);
}

inline Result ClientImpl::Delete(const std::string &path, const char *body,
                                 size_t content_length,
                                 const std::string &content_type,
                                 Progress progress) {
  return Delete(path, Headers(), body, content_length, content_type, progress);
}

inline Result ClientImpl::Delete(const std::string &path,
                                 const Headers &headers, const char *body,
                                 size_t content_length,
                                 const std::string &content_type) {
  return Delete(path, headers, body, content_length, content_type, nullptr);
}

inline Result ClientImpl::Delete(const std::string &path,
                                 const Headers &headers, const char *body,
                                 size_t content_length,
                                 const std::string &content_type,
                                 Progress progress) {
  Request req;
  req.method = "DELETE";
  req.headers = headers;
  req.path = path;
  req.progress = progress;
  if (max_timeout_msec_ > 0) {
    req.start_time_ = std::chrono::steady_clock::now();
  }

  if (!content_type.empty()) { req.set_header("Content-Type", content_type); }
  req.body.assign(body, content_length);

  return send_(std::move(req));
}

inline Result ClientImpl::Delete(const std::string &path,
                                 const std::string &body,
                                 const std::string &content_type) {
  return Delete(path, Headers(), body.data(), body.size(), content_type);
}

inline Result ClientImpl::Delete(const std::string &path,
                                 const std::string &body,
                                 const std::string &content_type,
                                 Progress progress) {
  return Delete(path, Headers(), body.data(), body.size(), content_type,
                progress);
}

inline Result ClientImpl::Delete(const std::string &path,
                                 const Headers &headers,
                                 const std::string &body,
                                 const std::string &content_type) {
  return Delete(path, headers, body.data(), body.size(), content_type);
}

inline Result ClientImpl::Delete(const std::string &path,
                                 const Headers &headers,
                                 const std::string &body,
                                 const std::string &content_type,
                                 Progress progress) {
  return Delete(path, headers, body.data(), body.size(), content_type,
                progress);
}

inline Result ClientImpl::Options(const std::string &path) {
  return Options(path, Headers());
}

inline Result ClientImpl::Options(const std::string &path,
                                  const Headers &headers) {
  Request req;
  req.method = "OPTIONS";
  req.headers = headers;
  req.path = path;
  if (max_timeout_msec_ > 0) {
    req.start_time_ = std::chrono::steady_clock::now();
  }

  return send_(std::move(req));
}

inline void ClientImpl::stop() {
  std::lock_guard<std::mutex> guard(socket_mutex_);

  // If there is anything ongoing right now, the ONLY thread-safe thing we can
  // do is to shutdown_socket, so that threads using this socket suddenly
  // discover they can't read/write any more and error out. Everything else
  // (closing the socket, shutting ssl down) is unsafe because these actions are
  // not thread-safe.
  if (socket_requests_in_flight_ > 0) {
    shutdown_socket(socket_);

    // Aside from that, we set a flag for the socket to be closed when we're
    // done.
    socket_should_be_closed_when_request_is_done_ = true;
    return;
  }

  // Otherwise, still holding the mutex, we can shut everything down ourselves
  shutdown_ssl(socket_, true);
  shutdown_socket(socket_);
  close_socket(socket_);
}

inline std::string ClientImpl::host() const { return host_; }

inline int ClientImpl::port() const { return port_; }

inline size_t ClientImpl::is_socket_open() const {
  std::lock_guard<std::mutex> guard(socket_mutex_);
  return socket_.is_open();
}

inline socket_t ClientImpl::socket() const { return socket_.sock; }

inline void ClientImpl::set_connection_timeout(time_t sec, time_t usec) {
  connection_timeout_sec_ = sec;
  connection_timeout_usec_ = usec;
}

inline void ClientImpl::set_read_timeout(time_t sec, time_t usec) {
  read_timeout_sec_ = sec;
  read_timeout_usec_ = usec;
}

inline void ClientImpl::set_write_timeout(time_t sec, time_t usec) {
  write_timeout_sec_ = sec;
  write_timeout_usec_ = usec;
}

inline void ClientImpl::set_max_timeout(time_t msec) {
  max_timeout_msec_ = msec;
}

inline void ClientImpl::set_basic_auth(const std::string &username,
                                       const std::string &password) {
  basic_auth_username_ = username;
  basic_auth_password_ = password;
}

inline void ClientImpl::set_bearer_token_auth(const std::string &token) {
  bearer_token_auth_token_ = token;
}

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
inline void ClientImpl::set_digest_auth(const std::string &username,
                                        const std::string &password) {
  digest_auth_username_ = username;
  digest_auth_password_ = password;
}
#endif

inline void ClientImpl::set_keep_alive(bool on) { keep_alive_ = on; }

inline void ClientImpl::set_follow_location(bool on) { follow_location_ = on; }

inline void ClientImpl::set_url_encode(bool on) { url_encode_ = on; }

inline void
ClientImpl::set_hostname_addr_map(std::map<std::string, std::string> addr_map) {
  addr_map_ = std::move(addr_map);
}

inline void ClientImpl::set_default_headers(Headers headers) {
  default_headers_ = std::move(headers);
}

inline void ClientImpl::set_header_writer(
    std::function<ssize_t(Stream &, Headers &)> const &writer) {
  header_writer_ = writer;
}

inline void ClientImpl::set_address_family(int family) {
  address_family_ = family;
}

inline void ClientImpl::set_tcp_nodelay(bool on) { tcp_nodelay_ = on; }

inline void ClientImpl::set_ipv6_v6only(bool on) { ipv6_v6only_ = on; }

inline void ClientImpl::set_socket_options(SocketOptions socket_options) {
  socket_options_ = std::move(socket_options);
}

inline void ClientImpl::set_compress(bool on) { compress_ = on; }

inline void ClientImpl::set_decompress(bool on) { decompress_ = on; }

inline void ClientImpl::set_interface(const std::string &intf) {
  interface_ = intf;
}

inline void ClientImpl::set_proxy(const std::string &host, int port) {
  proxy_host_ = host;
  proxy_port_ = port;
}

inline void ClientImpl::set_proxy_basic_auth(const std::string &username,
                                             const std::string &password) {
  proxy_basic_auth_username_ = username;
  proxy_basic_auth_password_ = password;
}

inline void ClientImpl::set_proxy_bearer_token_auth(const std::string &token) {
  proxy_bearer_token_auth_token_ = token;
}

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
inline void ClientImpl::set_proxy_digest_auth(const std::string &username,
                                              const std::string &password) {
  proxy_digest_auth_username_ = username;
  proxy_digest_auth_password_ = password;
}

inline void ClientImpl::set_ca_cert_path(const std::string &ca_cert_file_path,
                                         const std::string &ca_cert_dir_path) {
  ca_cert_file_path_ = ca_cert_file_path;
  ca_cert_dir_path_ = ca_cert_dir_path;
}

inline void ClientImpl::set_ca_cert_store(X509_STORE *ca_cert_store) {
  if (ca_cert_store && ca_cert_store != ca_cert_store_) {
    ca_cert_store_ = ca_cert_store;
  }
}

inline X509_STORE *ClientImpl::create_ca_cert_store(const char *ca_cert,
                                                    std::size_t size) const {
  auto mem = BIO_new_mem_buf(ca_cert, static_cast<int>(size));
  auto se = detail::scope_exit([&] { BIO_free_all(mem); });
  if (!mem) { return nullptr; }

  auto inf = PEM_X509_INFO_read_bio(mem, nullptr, nullptr, nullptr);
  if (!inf) { return nullptr; }

  auto cts = X509_STORE_new();
  if (cts) {
    for (auto i = 0; i < static_cast<int>(sk_X509_INFO_num(inf)); i++) {
      auto itmp = sk_X509_INFO_value(inf, i);
      if (!itmp) { continue; }

      if (itmp->x509) { X509_STORE_add_cert(cts, itmp->x509); }
      if (itmp->crl) { X509_STORE_add_crl(cts, itmp->crl); }
    }
  }

  sk_X509_INFO_pop_free(inf, X509_INFO_free);
  return cts;
}

inline void ClientImpl::enable_server_certificate_verification(bool enabled) {
  server_certificate_verification_ = enabled;
}

inline void ClientImpl::enable_server_hostname_verification(bool enabled) {
  server_hostname_verification_ = enabled;
}

inline void ClientImpl::set_server_certificate_verifier(
    std::function<SSLVerifierResponse(SSL *ssl)> verifier) {
  server_certificate_verifier_ = verifier;
}
#endif

inline void ClientImpl::set_logger(Logger logger) {
  logger_ = std::move(logger);
}

/*
 * SSL Implementation
 */
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
namespace detail {

template <typename U, typename V>
inline SSL *ssl_new(socket_t sock, SSL_CTX *ctx, std::mutex &ctx_mutex,
                    U SSL_connect_or_accept, V setup) {
  SSL *ssl = nullptr;
  {
    std::lock_guard<std::mutex> guard(ctx_mutex);
    ssl = SSL_new(ctx);
  }

  if (ssl) {
    set_nonblocking(sock, true);
    auto bio = BIO_new_socket(static_cast<int>(sock), BIO_NOCLOSE);
    BIO_set_nbio(bio, 1);
    SSL_set_bio(ssl, bio, bio);

    if (!setup(ssl) || SSL_connect_or_accept(ssl) != 1) {
      SSL_shutdown(ssl);
      {
        std::lock_guard<std::mutex> guard(ctx_mutex);
        SSL_free(ssl);
      }
      set_nonblocking(sock, false);
      return nullptr;
    }
    BIO_set_nbio(bio, 0);
    set_nonblocking(sock, false);
  }

  return ssl;
}

inline void ssl_delete(std::mutex &ctx_mutex, SSL *ssl, socket_t sock,
                       bool shutdown_gracefully) {
  // sometimes we may want to skip this to try to avoid SIGPIPE if we know
  // the remote has closed the network connection
  // Note that it is not always possible to avoid SIGPIPE, this is merely a
  // best-efforts.
  if (shutdown_gracefully) {
    (void)(sock);
    // SSL_shutdown() returns 0 on first call (indicating close_notify alert
    // sent) and 1 on subsequent call (indicating close_notify alert received)
    if (SSL_shutdown(ssl) == 0) {
      // Expected to return 1, but even if it doesn't, we free ssl
      SSL_shutdown(ssl);
    }
  }

  std::lock_guard<std::mutex> guard(ctx_mutex);
  SSL_free(ssl);
}

template <typename U>
bool ssl_connect_or_accept_nonblocking(socket_t sock, SSL *ssl,
                                       U ssl_connect_or_accept,
                                       time_t timeout_sec,
                                       time_t timeout_usec) {
  auto res = 0;
  while ((res = ssl_connect_or_accept(ssl)) != 1) {
    auto err = SSL_get_error(ssl, res);
    switch (err) {
    case SSL_ERROR_WANT_READ:
      if (select_read(sock, timeout_sec, timeout_usec) > 0) { continue; }
      break;
    case SSL_ERROR_WANT_WRITE:
      if (select_write(sock, timeout_sec, timeout_usec) > 0) { continue; }
      break;
    default: break;
    }
    return false;
  }
  return true;
}

template <typename T>
inline bool process_server_socket_ssl(
    const std::atomic<socket_t> &svr_sock, SSL *ssl, socket_t sock,
    size_t keep_alive_max_count, time_t keep_alive_timeout_sec,
    time_t read_timeout_sec, time_t read_timeout_usec, time_t write_timeout_sec,
    time_t write_timeout_usec, T callback) {
  return process_server_socket_core(
      svr_sock, sock, keep_alive_max_count, keep_alive_timeout_sec,
      [&](bool close_connection, bool &connection_closed) {
        SSLSocketStream strm(sock, ssl, read_timeout_sec, read_timeout_usec,
                             write_timeout_sec, write_timeout_usec);
        return callback(strm, close_connection, connection_closed);
      });
}

template <typename T>
inline bool process_client_socket_ssl(
    SSL *ssl, socket_t sock, time_t read_timeout_sec, time_t read_timeout_usec,
    time_t write_timeout_sec, time_t write_timeout_usec,
    time_t max_timeout_msec,
    std::chrono::time_point<std::chrono::steady_clock> start_time, T callback) {
  SSLSocketStream strm(sock, ssl, read_timeout_sec, read_timeout_usec,
                       write_timeout_sec, write_timeout_usec, max_timeout_msec,
                       start_time);
  return callback(strm);
}

// SSL socket stream implementation
inline SSLSocketStream::SSLSocketStream(
    socket_t sock, SSL *ssl, time_t read_timeout_sec, time_t read_timeout_usec,
    time_t write_timeout_sec, time_t write_timeout_usec,
    time_t max_timeout_msec,
    std::chrono::time_point<std::chrono::steady_clock> start_time)
    : sock_(sock), ssl_(ssl), read_timeout_sec_(read_timeout_sec),
      read_timeout_usec_(read_timeout_usec),
      write_timeout_sec_(write_timeout_sec),
      write_timeout_usec_(write_timeout_usec),
      max_timeout_msec_(max_timeout_msec), start_time_(start_time) {
  SSL_clear_mode(ssl, SSL_MODE_AUTO_RETRY);
}

inline SSLSocketStream::~SSLSocketStream() = default;

inline bool SSLSocketStream::is_readable() const {
  return SSL_pending(ssl_) > 0;
}

inline bool SSLSocketStream::wait_readable() const {
  if (max_timeout_msec_ <= 0) {
    return select_read(sock_, read_timeout_sec_, read_timeout_usec_) > 0;
  }

  time_t read_timeout_sec;
  time_t read_timeout_usec;
  calc_actual_timeout(max_timeout_msec_, duration(), read_timeout_sec_,
                      read_timeout_usec_, read_timeout_sec, read_timeout_usec);

  return select_read(sock_, read_timeout_sec, read_timeout_usec) > 0;
}

inline bool SSLSocketStream::wait_writable() const {
  return select_write(sock_, write_timeout_sec_, write_timeout_usec_) > 0 &&
         is_socket_alive(sock_) && !is_ssl_peer_could_be_closed(ssl_, sock_);
}

inline ssize_t SSLSocketStream::read(char *ptr, size_t size) {
  if (SSL_pending(ssl_) > 0) {
    return SSL_read(ssl_, ptr, static_cast<int>(size));
  } else if (wait_readable()) {
    auto ret = SSL_read(ssl_, ptr, static_cast<int>(size));
    if (ret < 0) {
      auto err = SSL_get_error(ssl_, ret);
      auto n = 1000;
#ifdef _WIN32
      while (--n >= 0 && (err == SSL_ERROR_WANT_READ ||
                          (err == SSL_ERROR_SYSCALL &&
                           WSAGetLastError() == WSAETIMEDOUT))) {
#else
      while (--n >= 0 && err == SSL_ERROR_WANT_READ) {
#endif
        if (SSL_pending(ssl_) > 0) {
          return SSL_read(ssl_, ptr, static_cast<int>(size));
        } else if (wait_readable()) {
          std::this_thread::sleep_for(std::chrono::microseconds{10});
          ret = SSL_read(ssl_, ptr, static_cast<int>(size));
          if (ret >= 0) { return ret; }
          err = SSL_get_error(ssl_, ret);
        } else {
          return -1;
        }
      }
    }
    return ret;
  } else {
    return -1;
  }
}

inline ssize_t SSLSocketStream::write(const char *ptr, size_t size) {
  if (wait_writable()) {
    auto handle_size = static_cast<int>(
        std::min<size_t>(size, (std::numeric_limits<int>::max)()));

    auto ret = SSL_write(ssl_, ptr, static_cast<int>(handle_size));
    if (ret < 0) {
      auto err = SSL_get_error(ssl_, ret);
      auto n = 1000;
#ifdef _WIN32
      while (--n >= 0 && (err == SSL_ERROR_WANT_WRITE ||
                          (err == SSL_ERROR_SYSCALL &&
                           WSAGetLastError() == WSAETIMEDOUT))) {
#else
      while (--n >= 0 && err == SSL_ERROR_WANT_WRITE) {
#endif
        if (wait_writable()) {
          std::this_thread::sleep_for(std::chrono::microseconds{10});
          ret = SSL_write(ssl_, ptr, static_cast<int>(handle_size));
          if (ret >= 0) { return ret; }
          err = SSL_get_error(ssl_, ret);
        } else {
          return -1;
        }
      }
    }
    return ret;
  }
  return -1;
}

inline void SSLSocketStream::get_remote_ip_and_port(std::string &ip,
                                                    int &port) const {
  detail::get_remote_ip_and_port(sock_, ip, port);
}

inline void SSLSocketStream::get_local_ip_and_port(std::string &ip,
                                                   int &port) const {
  detail::get_local_ip_and_port(sock_, ip, port);
}

inline socket_t SSLSocketStream::socket() const { return sock_; }

inline time_t SSLSocketStream::duration() const {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now() - start_time_)
      .count();
}

} // namespace detail

// SSL HTTP server implementation
inline SSLServer::SSLServer(const char *cert_path, const char *private_key_path,
                            const char *client_ca_cert_file_path,
                            const char *client_ca_cert_dir_path,
                            const char *private_key_password) {
  ctx_ = SSL_CTX_new(TLS_server_method());

  if (ctx_) {
    SSL_CTX_set_options(ctx_,
                        SSL_OP_NO_COMPRESSION |
                            SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);

    SSL_CTX_set_min_proto_version(ctx_, TLS1_2_VERSION);

    if (private_key_password != nullptr && (private_key_password[0] != '\0')) {
      SSL_CTX_set_default_passwd_cb_userdata(
          ctx_,
          reinterpret_cast<void *>(const_cast<char *>(private_key_password)));
    }

    if (SSL_CTX_use_certificate_chain_file(ctx_, cert_path) != 1 ||
        SSL_CTX_use_PrivateKey_file(ctx_, private_key_path, SSL_FILETYPE_PEM) !=
            1 ||
        SSL_CTX_check_private_key(ctx_) != 1) {
      SSL_CTX_free(ctx_);
      ctx_ = nullptr;
    } else if (client_ca_cert_file_path || client_ca_cert_dir_path) {
      SSL_CTX_load_verify_locations(ctx_, client_ca_cert_file_path,
                                    client_ca_cert_dir_path);

      SSL_CTX_set_verify(
          ctx_, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
    }
  }
}

inline SSLServer::SSLServer(X509 *cert, EVP_PKEY *private_key,
                            X509_STORE *client_ca_cert_store) {
  ctx_ = SSL_CTX_new(TLS_server_method());

  if (ctx_) {
    SSL_CTX_set_options(ctx_,
                        SSL_OP_NO_COMPRESSION |
                            SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);

    SSL_CTX_set_min_proto_version(ctx_, TLS1_2_VERSION);

    if (SSL_CTX_use_certificate(ctx_, cert) != 1 ||
        SSL_CTX_use_PrivateKey(ctx_, private_key) != 1) {
      SSL_CTX_free(ctx_);
      ctx_ = nullptr;
    } else if (client_ca_cert_store) {
      SSL_CTX_set_cert_store(ctx_, client_ca_cert_store);

      SSL_CTX_set_verify(
          ctx_, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
    }
  }
}

inline SSLServer::SSLServer(
    const std::function<bool(SSL_CTX &ssl_ctx)> &setup_ssl_ctx_callback) {
  ctx_ = SSL_CTX_new(TLS_method());
  if (ctx_) {
    if (!setup_ssl_ctx_callback(*ctx_)) {
      SSL_CTX_free(ctx_);
      ctx_ = nullptr;
    }
  }
}

inline SSLServer::~SSLServer() {
  if (ctx_) { SSL_CTX_free(ctx_); }
}

inline bool SSLServer::is_valid() const { return ctx_; }

inline SSL_CTX *SSLServer::ssl_context() const { return ctx_; }

inline void SSLServer::update_certs(X509 *cert, EVP_PKEY *private_key,
                                    X509_STORE *client_ca_cert_store) {

  std::lock_guard<std::mutex> guard(ctx_mutex_);

  SSL_CTX_use_certificate(ctx_, cert);
  SSL_CTX_use_PrivateKey(ctx_, private_key);

  if (client_ca_cert_store != nullptr) {
    SSL_CTX_set_cert_store(ctx_, client_ca_cert_store);
  }
}

inline bool SSLServer::process_and_close_socket(socket_t sock) {
  auto ssl = detail::ssl_new(
      sock, ctx_, ctx_mutex_,
      [&](SSL *ssl2) {
        return detail::ssl_connect_or_accept_nonblocking(
            sock, ssl2, SSL_accept, read_timeout_sec_, read_timeout_usec_);
      },
      [](SSL * /*ssl2*/) { return true; });

  auto ret = false;
  if (ssl) {
    std::string remote_addr;
    int remote_port = 0;
    detail::get_remote_ip_and_port(sock, remote_addr, remote_port);

    std::string local_addr;
    int local_port = 0;
    detail::get_local_ip_and_port(sock, local_addr, local_port);

    ret = detail::process_server_socket_ssl(
        svr_sock_, ssl, sock, keep_alive_max_count_, keep_alive_timeout_sec_,
        read_timeout_sec_, read_timeout_usec_, write_timeout_sec_,
        write_timeout_usec_,
        [&](Stream &strm, bool close_connection, bool &connection_closed) {
          return process_request(strm, remote_addr, remote_port, local_addr,
                                 local_port, close_connection,
                                 connection_closed,
                                 [&](Request &req) { req.ssl = ssl; });
        });

    // Shutdown gracefully if the result seemed successful, non-gracefully if
    // the connection appeared to be closed.
    const bool shutdown_gracefully = ret;
    detail::ssl_delete(ctx_mutex_, ssl, sock, shutdown_gracefully);
  }

  detail::shutdown_socket(sock);
  detail::close_socket(sock);
  return ret;
}

// SSL HTTP client implementation
inline SSLClient::SSLClient(const std::string &host)
    : SSLClient(host, 443, std::string(), std::string()) {}

inline SSLClient::SSLClient(const std::string &host, int port)
    : SSLClient(host, port, std::string(), std::string()) {}

inline SSLClient::SSLClient(const std::string &host, int port,
                            const std::string &client_cert_path,
                            const std::string &client_key_path,
                            const std::string &private_key_password)
    : ClientImpl(host, port, client_cert_path, client_key_path) {
  ctx_ = SSL_CTX_new(TLS_client_method());

  SSL_CTX_set_min_proto_version(ctx_, TLS1_2_VERSION);

  detail::split(&host_[0], &host_[host_.size()], '.',
                [&](const char *b, const char *e) {
                  host_components_.emplace_back(b, e);
                });

  if (!client_cert_path.empty() && !client_key_path.empty()) {
    if (!private_key_password.empty()) {
      SSL_CTX_set_default_passwd_cb_userdata(
          ctx_, reinterpret_cast<void *>(
                    const_cast<char *>(private_key_password.c_str())));
    }

    if (SSL_CTX_use_certificate_file(ctx_, client_cert_path.c_str(),
                                     SSL_FILETYPE_PEM) != 1 ||
        SSL_CTX_use_PrivateKey_file(ctx_, client_key_path.c_str(),
                                    SSL_FILETYPE_PEM) != 1) {
      SSL_CTX_free(ctx_);
      ctx_ = nullptr;
    }
  }
}

inline SSLClient::SSLClient(const std::string &host, int port,
                            X509 *client_cert, EVP_PKEY *client_key,
                            const std::string &private_key_password)
    : ClientImpl(host, port) {
  ctx_ = SSL_CTX_new(TLS_client_method());

  detail::split(&host_[0], &host_[host_.size()], '.',
                [&](const char *b, const char *e) {
                  host_components_.emplace_back(b, e);
                });

  if (client_cert != nullptr && client_key != nullptr) {
    if (!private_key_password.empty()) {
      SSL_CTX_set_default_passwd_cb_userdata(
          ctx_, reinterpret_cast<void *>(
                    const_cast<char *>(private_key_password.c_str())));
    }

    if (SSL_CTX_use_certificate(ctx_, client_cert) != 1 ||
        SSL_CTX_use_PrivateKey(ctx_, client_key) != 1) {
      SSL_CTX_free(ctx_);
      ctx_ = nullptr;
    }
  }
}

inline SSLClient::~SSLClient() {
  if (ctx_) { SSL_CTX_free(ctx_); }
  // Make sure to shut down SSL since shutdown_ssl will resolve to the
  // base function rather than the derived function once we get to the
  // base class destructor, and won't free the SSL (causing a leak).
  shutdown_ssl_impl(socket_, true);
}

inline bool SSLClient::is_valid() const { return ctx_; }

inline void SSLClient::set_ca_cert_store(X509_STORE *ca_cert_store) {
  if (ca_cert_store) {
    if (ctx_) {
      if (SSL_CTX_get_cert_store(ctx_) != ca_cert_store) {
        // Free memory allocated for old cert and use new store `ca_cert_store`
        SSL_CTX_set_cert_store(ctx_, ca_cert_store);
      }
    } else {
      X509_STORE_free(ca_cert_store);
    }
  }
}

inline void SSLClient::load_ca_cert_store(const char *ca_cert,
                                          std::size_t size) {
  set_ca_cert_store(ClientImpl::create_ca_cert_store(ca_cert, size));
}

inline long SSLClient::get_openssl_verify_result() const {
  return verify_result_;
}

inline SSL_CTX *SSLClient::ssl_context() const { return ctx_; }

inline bool SSLClient::create_and_connect_socket(Socket &socket, Error &error) {
  return is_valid() && ClientImpl::create_and_connect_socket(socket, error);
}

// Assumes that socket_mutex_ is locked and that there are no requests in flight
inline bool SSLClient::connect_with_proxy(
    Socket &socket,
    std::chrono::time_point<std::chrono::steady_clock> start_time,
    Response &res, bool &success, Error &error) {
  success = true;
  Response proxy_res;
  if (!detail::process_client_socket(
          socket.sock, read_timeout_sec_, read_timeout_usec_,
          write_timeout_sec_, write_timeout_usec_, max_timeout_msec_,
          start_time, [&](Stream &strm) {
            Request req2;
            req2.method = "CONNECT";
            req2.path = host_and_port_;
            if (max_timeout_msec_ > 0) {
              req2.start_time_ = std::chrono::steady_clock::now();
            }
            return process_request(strm, req2, proxy_res, false, error);
          })) {
    // Thread-safe to close everything because we are assuming there are no
    // requests in flight
    shutdown_ssl(socket, true);
    shutdown_socket(socket);
    close_socket(socket);
    success = false;
    return false;
  }

  if (proxy_res.status == StatusCode::ProxyAuthenticationRequired_407) {
    if (!proxy_digest_auth_username_.empty() &&
        !proxy_digest_auth_password_.empty()) {
      std::map<std::string, std::string> auth;
      if (detail::parse_www_authenticate(proxy_res, auth, true)) {
        proxy_res = Response();
        if (!detail::process_client_socket(
                socket.sock, read_timeout_sec_, read_timeout_usec_,
                write_timeout_sec_, write_timeout_usec_, max_timeout_msec_,
                start_time, [&](Stream &strm) {
                  Request req3;
                  req3.method = "CONNECT";
                  req3.path = host_and_port_;
                  req3.headers.insert(detail::make_digest_authentication_header(
                      req3, auth, 1, detail::random_string(10),
                      proxy_digest_auth_username_, proxy_digest_auth_password_,
                      true));
                  if (max_timeout_msec_ > 0) {
                    req3.start_time_ = std::chrono::steady_clock::now();
                  }
                  return process_request(strm, req3, proxy_res, false, error);
                })) {
          // Thread-safe to close everything because we are assuming there are
          // no requests in flight
          shutdown_ssl(socket, true);
          shutdown_socket(socket);
          close_socket(socket);
          success = false;
          return false;
        }
      }
    }
  }

  // If status code is not 200, proxy request is failed.
  // Set error to ProxyConnection and return proxy response
  // as the response of the request
  if (proxy_res.status != StatusCode::OK_200) {
    error = Error::ProxyConnection;
    res = std::move(proxy_res);
    // Thread-safe to close everything because we are assuming there are
    // no requests in flight
    shutdown_ssl(socket, true);
    shutdown_socket(socket);
    close_socket(socket);
    return false;
  }

  return true;
}

inline bool SSLClient::load_certs() {
  auto ret = true;

  std::call_once(initialize_cert_, [&]() {
    std::lock_guard<std::mutex> guard(ctx_mutex_);
    if (!ca_cert_file_path_.empty()) {
      if (!SSL_CTX_load_verify_locations(ctx_, ca_cert_file_path_.c_str(),
                                         nullptr)) {
        ret = false;
      }
    } else if (!ca_cert_dir_path_.empty()) {
      if (!SSL_CTX_load_verify_locations(ctx_, nullptr,
                                         ca_cert_dir_path_.c_str())) {
        ret = false;
      }
    } else {
      auto loaded = false;
#ifdef _WIN32
      loaded =
          detail::load_system_certs_on_windows(SSL_CTX_get_cert_store(ctx_));
#elif defined(CPPHTTPLIB_USE_CERTS_FROM_MACOSX_KEYCHAIN) && defined(__APPLE__)
#if TARGET_OS_OSX
      loaded = detail::load_system_certs_on_macos(SSL_CTX_get_cert_store(ctx_));
#endif // TARGET_OS_OSX
#endif // _WIN32
      if (!loaded) { SSL_CTX_set_default_verify_paths(ctx_); }
    }
  });

  return ret;
}

inline bool SSLClient::initialize_ssl(Socket &socket, Error &error) {
  auto ssl = detail::ssl_new(
      socket.sock, ctx_, ctx_mutex_,
      [&](SSL *ssl2) {
        if (server_certificate_verification_) {
          if (!load_certs()) {
            error = Error::SSLLoadingCerts;
            return false;
          }
          SSL_set_verify(ssl2, SSL_VERIFY_NONE, nullptr);
        }

        if (!detail::ssl_connect_or_accept_nonblocking(
                socket.sock, ssl2, SSL_connect, connection_timeout_sec_,
                connection_timeout_usec_)) {
          error = Error::SSLConnection;
          return false;
        }

        if (server_certificate_verification_) {
          auto verification_status = SSLVerifierResponse::NoDecisionMade;

          if (server_certificate_verifier_) {
            verification_status = server_certificate_verifier_(ssl2);
          }

          if (verification_status == SSLVerifierResponse::CertificateRejected) {
            error = Error::SSLServerVerification;
            return false;
          }

          if (verification_status == SSLVerifierResponse::NoDecisionMade) {
            verify_result_ = SSL_get_verify_result(ssl2);

            if (verify_result_ != X509_V_OK) {
              error = Error::SSLServerVerification;
              return false;
            }

            auto server_cert = SSL_get1_peer_certificate(ssl2);
            auto se = detail::scope_exit([&] { X509_free(server_cert); });

            if (server_cert == nullptr) {
              error = Error::SSLServerVerification;
              return false;
            }

            if (server_hostname_verification_) {
              if (!verify_host(server_cert)) {
                error = Error::SSLServerHostnameVerification;
                return false;
              }
            }
          }
        }

        return true;
      },
      [&](SSL *ssl2) {
#if defined(OPENSSL_IS_BORINGSSL)
        SSL_set_tlsext_host_name(ssl2, host_.c_str());
#else
        // NOTE: Direct call instead of using the OpenSSL macro to suppress
        // -Wold-style-cast warning
        SSL_ctrl(ssl2, SSL_CTRL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name,
                 static_cast<void *>(const_cast<char *>(host_.c_str())));
#endif
        return true;
      });

  if (ssl) {
    socket.ssl = ssl;
    return true;
  }

  shutdown_socket(socket);
  close_socket(socket);
  return false;
}

inline void SSLClient::shutdown_ssl(Socket &socket, bool shutdown_gracefully) {
  shutdown_ssl_impl(socket, shutdown_gracefully);
}

inline void SSLClient::shutdown_ssl_impl(Socket &socket,
                                         bool shutdown_gracefully) {
  if (socket.sock == INVALID_SOCKET) {
    assert(socket.ssl == nullptr);
    return;
  }
  if (socket.ssl) {
    detail::ssl_delete(ctx_mutex_, socket.ssl, socket.sock,
                       shutdown_gracefully);
    socket.ssl = nullptr;
  }
  assert(socket.ssl == nullptr);
}

inline bool SSLClient::process_socket(
    const Socket &socket,
    std::chrono::time_point<std::chrono::steady_clock> start_time,
    std::function<bool(Stream &strm)> callback) {
  assert(socket.ssl);
  return detail::process_client_socket_ssl(
      socket.ssl, socket.sock, read_timeout_sec_, read_timeout_usec_,
      write_timeout_sec_, write_timeout_usec_, max_timeout_msec_, start_time,
      std::move(callback));
}

inline bool SSLClient::is_ssl() const { return true; }

inline bool SSLClient::verify_host(X509 *server_cert) const {
  /* Quote from RFC2818 section 3.1 "Server Identity"

     If a subjectAltName extension of type dNSName is present, that MUST
     be used as the identity. Otherwise, the (most specific) Common Name
     field in the Subject field of the certificate MUST be used. Although
     the use of the Common Name is existing practice, it is deprecated and
     Certification Authorities are encouraged to use the dNSName instead.

     Matching is performed using the matching rules specified by
     [RFC2459].  If more than one identity of a given type is present in
     the certificate (e.g., more than one dNSName name, a match in any one
     of the set is considered acceptable.) Names may contain the wildcard
     character * which is considered to match any single domain name
     component or component fragment. E.g., *.a.com matches foo.a.com but
     not bar.foo.a.com. f*.com matches foo.com but not bar.com.

     In some cases, the URI is specified as an IP address rather than a
     hostname. In this case, the iPAddress subjectAltName must be present
     in the certificate and must exactly match the IP in the URI.

  */
  return verify_host_with_subject_alt_name(server_cert) ||
         verify_host_with_common_name(server_cert);
}

inline bool
SSLClient::verify_host_with_subject_alt_name(X509 *server_cert) const {
  auto ret = false;

  auto type = GEN_DNS;

  struct in6_addr addr6 = {};
  struct in_addr addr = {};
  size_t addr_len = 0;

#ifndef __MINGW32__
  if (inet_pton(AF_INET6, host_.c_str(), &addr6)) {
    type = GEN_IPADD;
    addr_len = sizeof(struct in6_addr);
  } else if (inet_pton(AF_INET, host_.c_str(), &addr)) {
    type = GEN_IPADD;
    addr_len = sizeof(struct in_addr);
  }
#endif

  auto alt_names = static_cast<const struct stack_st_GENERAL_NAME *>(
      X509_get_ext_d2i(server_cert, NID_subject_alt_name, nullptr, nullptr));

  if (alt_names) {
    auto dsn_matched = false;
    auto ip_matched = false;

    auto count = sk_GENERAL_NAME_num(alt_names);

    for (decltype(count) i = 0; i < count && !dsn_matched; i++) {
      auto val = sk_GENERAL_NAME_value(alt_names, i);
      if (val->type == type) {
        auto name =
            reinterpret_cast<const char *>(ASN1_STRING_get0_data(val->d.ia5));
        auto name_len = static_cast<size_t>(ASN1_STRING_length(val->d.ia5));

        switch (type) {
        case GEN_DNS: dsn_matched = check_host_name(name, name_len); break;

        case GEN_IPADD:
          if (!memcmp(&addr6, name, addr_len) ||
              !memcmp(&addr, name, addr_len)) {
            ip_matched = true;
          }
          break;
        }
      }
    }

    if (dsn_matched || ip_matched) { ret = true; }
  }

  GENERAL_NAMES_free(const_cast<STACK_OF(GENERAL_NAME) *>(
      reinterpret_cast<const STACK_OF(GENERAL_NAME) *>(alt_names)));
  return ret;
}

inline bool SSLClient::verify_host_with_common_name(X509 *server_cert) const {
  const auto subject_name = X509_get_subject_name(server_cert);

  if (subject_name != nullptr) {
    char name[BUFSIZ];
    auto name_len = X509_NAME_get_text_by_NID(subject_name, NID_commonName,
                                              name, sizeof(name));

    if (name_len != -1) {
      return check_host_name(name, static_cast<size_t>(name_len));
    }
  }

  return false;
}

inline bool SSLClient::check_host_name(const char *pattern,
                                       size_t pattern_len) const {
  if (host_.size() == pattern_len && host_ == pattern) { return true; }

  // Wildcard match
  // https://bugs.launchpad.net/ubuntu/+source/firefox-3.0/+bug/376484
  std::vector<std::string> pattern_components;
  detail::split(&pattern[0], &pattern[pattern_len], '.',
                [&](const char *b, const char *e) {
                  pattern_components.emplace_back(b, e);
                });

  if (host_components_.size() != pattern_components.size()) { return false; }

  auto itr = pattern_components.begin();
  for (const auto &h : host_components_) {
    auto &p = *itr;
    if (p != h && p != "*") {
      auto partial_match = (p.size() > 0 && p[p.size() - 1] == '*' &&
                            !p.compare(0, p.size() - 1, h));
      if (!partial_match) { return false; }
    }
    ++itr;
  }

  return true;
}
#endif

// Universal client implementation
inline Client::Client(const std::string &scheme_host_port)
    : Client(scheme_host_port, std::string(), std::string()) {}

inline Client::Client(const std::string &scheme_host_port,
                      const std::string &client_cert_path,
                      const std::string &client_key_path) {
  const static std::regex re(
      R"((?:([a-z]+):\/\/)?(?:\[([a-fA-F\d:]+)\]|([^:/?#]+))(?::(\d+))?)");

  std::smatch m;
  if (std::regex_match(scheme_host_port, m, re)) {
    auto scheme = m[1].str();

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    if (!scheme.empty() && (scheme != "http" && scheme != "https")) {
#else
    if (!scheme.empty() && scheme != "http") {
#endif
#ifndef CPPHTTPLIB_NO_EXCEPTIONS
      std::string msg = "'" + scheme + "' scheme is not supported.";
      throw std::invalid_argument(msg);
#endif
      return;
    }

    auto is_ssl = scheme == "https";

    auto host = m[2].str();
    if (host.empty()) { host = m[3].str(); }

    auto port_str = m[4].str();
    auto port = !port_str.empty() ? std::stoi(port_str) : (is_ssl ? 443 : 80);

    if (is_ssl) {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
      cli_ = detail::make_unique<SSLClient>(host, port, client_cert_path,
                                            client_key_path);
      is_ssl_ = is_ssl;
#endif
    } else {
      cli_ = detail::make_unique<ClientImpl>(host, port, client_cert_path,
                                             client_key_path);
    }
  } else {
    // NOTE: Update TEST(UniversalClientImplTest, Ipv6LiteralAddress)
    // if port param below changes.
    cli_ = detail::make_unique<ClientImpl>(scheme_host_port, 80,
                                           client_cert_path, client_key_path);
  }
} // namespace detail

inline Client::Client(const std::string &host, int port)
    : cli_(detail::make_unique<ClientImpl>(host, port)) {}

inline Client::Client(const std::string &host, int port,
                      const std::string &client_cert_path,
                      const std::string &client_key_path)
    : cli_(detail::make_unique<ClientImpl>(host, port, client_cert_path,
                                           client_key_path)) {}

inline Client::~Client() = default;

inline bool Client::is_valid() const {
  return cli_ != nullptr && cli_->is_valid();
}

inline Result Client::Get(const std::string &path) { return cli_->Get(path); }
inline Result Client::Get(const std::string &path, const Headers &headers) {
  return cli_->Get(path, headers);
}
inline Result Client::Get(const std::string &path, Progress progress) {
  return cli_->Get(path, std::move(progress));
}
inline Result Client::Get(const std::string &path, const Headers &headers,
                          Progress progress) {
  return cli_->Get(path, headers, std::move(progress));
}
inline Result Client::Get(const std::string &path,
                          ContentReceiver content_receiver) {
  return cli_->Get(path, std::move(content_receiver));
}
inline Result Client::Get(const std::string &path, const Headers &headers,
                          ContentReceiver content_receiver) {
  return cli_->Get(path, headers, std::move(content_receiver));
}
inline Result Client::Get(const std::string &path,
                          ContentReceiver content_receiver, Progress progress) {
  return cli_->Get(path, std::move(content_receiver), std::move(progress));
}
inline Result Client::Get(const std::string &path, const Headers &headers,
                          ContentReceiver content_receiver, Progress progress) {
  return cli_->Get(path, headers, std::move(content_receiver),
                   std::move(progress));
}
inline Result Client::Get(const std::string &path,
                          ResponseHandler response_handler,
                          ContentReceiver content_receiver) {
  return cli_->Get(path, std::move(response_handler),
                   std::move(content_receiver));
}
inline Result Client::Get(const std::string &path, const Headers &headers,
                          ResponseHandler response_handler,
                          ContentReceiver content_receiver) {
  return cli_->Get(path, headers, std::move(response_handler),
                   std::move(content_receiver));
}
inline Result Client::Get(const std::string &path,
                          ResponseHandler response_handler,
                          ContentReceiver content_receiver, Progress progress) {
  return cli_->Get(path, std::move(response_handler),
                   std::move(content_receiver), std::move(progress));
}
inline Result Client::Get(const std::string &path, const Headers &headers,
                          ResponseHandler response_handler,
                          ContentReceiver content_receiver, Progress progress) {
  return cli_->Get(path, headers, std::move(response_handler),
                   std::move(content_receiver), std::move(progress));
}
inline Result Client::Get(const std::string &path, const Params &params,
                          const Headers &headers, Progress progress) {
  return cli_->Get(path, params, headers, std::move(progress));
}
inline Result Client::Get(const std::string &path, const Params &params,
                          const Headers &headers,
                          ContentReceiver content_receiver, Progress progress) {
  return cli_->Get(path, params, headers, std::move(content_receiver),
                   std::move(progress));
}
inline Result Client::Get(const std::string &path, const Params &params,
                          const Headers &headers,
                          ResponseHandler response_handler,
                          ContentReceiver content_receiver, Progress progress) {
  return cli_->Get(path, params, headers, std::move(response_handler),
                   std::move(content_receiver), std::move(progress));
}

inline Result Client::Head(const std::string &path) { return cli_->Head(path); }
inline Result Client::Head(const std::string &path, const Headers &headers) {
  return cli_->Head(path, headers);
}

inline Result Client::Post(const std::string &path) { return cli_->Post(path); }
inline Result Client::Post(const std::string &path, const Headers &headers) {
  return cli_->Post(path, headers);
}
inline Result Client::Post(const std::string &path, const char *body,
                           size_t content_length,
                           const std::string &content_type) {
  return cli_->Post(path, body, content_length, content_type);
}
inline Result Client::Post(const std::string &path, const Headers &headers,
                           const char *body, size_t content_length,
                           const std::string &content_type) {
  return cli_->Post(path, headers, body, content_length, content_type);
}
inline Result Client::Post(const std::string &path, const Headers &headers,
                           const char *body, size_t content_length,
                           const std::string &content_type, Progress progress) {
  return cli_->Post(path, headers, body, content_length, content_type,
                    progress);
}
inline Result Client::Post(const std::string &path, const std::string &body,
                           const std::string &content_type) {
  return cli_->Post(path, body, content_type);
}
inline Result Client::Post(const std::string &path, const std::string &body,
                           const std::string &content_type, Progress progress) {
  return cli_->Post(path, body, content_type, progress);
}
inline Result Client::Post(const std::string &path, const Headers &headers,
                           const std::string &body,
                           const std::string &content_type) {
  return cli_->Post(path, headers, body, content_type);
}
inline Result Client::Post(const std::string &path, const Headers &headers,
                           const std::string &body,
                           const std::string &content_type, Progress progress) {
  return cli_->Post(path, headers, body, content_type, progress);
}
inline Result Client::Post(const std::string &path, size_t content_length,
                           ContentProvider content_provider,
                           const std::string &content_type) {
  return cli_->Post(path, content_length, std::move(content_provider),
                    content_type);
}
inline Result Client::Post(const std::string &path,
                           ContentProviderWithoutLength content_provider,
                           const std::string &content_type) {
  return cli_->Post(path, std::move(content_provider), content_type);
}
inline Result Client::Post(const std::string &path, const Headers &headers,
                           size_t content_length,
                           ContentProvider content_provider,
                           const std::string &content_type) {
  return cli_->Post(path, headers, content_length, std::move(content_provider),
                    content_type);
}
inline Result Client::Post(const std::string &path, const Headers &headers,
                           ContentProviderWithoutLength content_provider,
                           const std::string &content_type) {
  return cli_->Post(path, headers, std::move(content_provider), content_type);
}
inline Result Client::Post(const std::string &path, const Params &params) {
  return cli_->Post(path, params);
}
inline Result Client::Post(const std::string &path, const Headers &headers,
                           const Params &params) {
  return cli_->Post(path, headers, params);
}
inline Result Client::Post(const std::string &path, const Headers &headers,
                           const Params &params, Progress progress) {
  return cli_->Post(path, headers, params, progress);
}
inline Result Client::Post(const std::string &path,
                           const MultipartFormDataItems &items) {
  return cli_->Post(path, items);
}
inline Result Client::Post(const std::string &path, const Headers &headers,
                           const MultipartFormDataItems &items) {
  return cli_->Post(path, headers, items);
}
inline Result Client::Post(const std::string &path, const Headers &headers,
                           const MultipartFormDataItems &items,
                           const std::string &boundary) {
  return cli_->Post(path, headers, items, boundary);
}
inline Result
Client::Post(const std::string &path, const Headers &headers,
             const MultipartFormDataItems &items,
             const MultipartFormDataProviderItems &provider_items) {
  return cli_->Post(path, headers, items, provider_items);
}
inline Result Client::Put(const std::string &path) { return cli_->Put(path); }
inline Result Client::Put(const std::string &path, const char *body,
                          size_t content_length,
                          const std::string &content_type) {
  return cli_->Put(path, body, content_length, content_type);
}
inline Result Client::Put(const std::string &path, const Headers &headers,
                          const char *body, size_t content_length,
                          const std::string &content_type) {
  return cli_->Put(path, headers, body, content_length, content_type);
}
inline Result Client::Put(const std::string &path, const Headers &headers,
                          const char *body, size_t content_length,
                          const std::string &content_type, Progress progress) {
  return cli_->Put(path, headers, body, content_length, content_type, progress);
}
inline Result Client::Put(const std::string &path, const std::string &body,
                          const std::string &content_type) {
  return cli_->Put(path, body, content_type);
}
inline Result Client::Put(const std::string &path, const std::string &body,
                          const std::string &content_type, Progress progress) {
  return cli_->Put(path, body, content_type, progress);
}
inline Result Client::Put(const std::string &path, const Headers &headers,
                          const std::string &body,
                          const std::string &content_type) {
  return cli_->Put(path, headers, body, content_type);
}
inline Result Client::Put(const std::string &path, const Headers &headers,
                          const std::string &body,
                          const std::string &content_type, Progress progress) {
  return cli_->Put(path, headers, body, content_type, progress);
}
inline Result Client::Put(const std::string &path, size_t content_length,
                          ContentProvider content_provider,
                          const std::string &content_type) {
  return cli_->Put(path, content_length, std::move(content_provider),
                   content_type);
}
inline Result Client::Put(const std::string &path,
                          ContentProviderWithoutLength content_provider,
                          const std::string &content_type) {
  return cli_->Put(path, std::move(content_provider), content_type);
}
inline Result Client::Put(const std::string &path, const Headers &headers,
                          size_t content_length,
                          ContentProvider content_provider,
                          const std::string &content_type) {
  return cli_->Put(path, headers, content_length, std::move(content_provider),
                   content_type);
}
inline Result Client::Put(const std::string &path, const Headers &headers,
                          ContentProviderWithoutLength content_provider,
                          const std::string &content_type) {
  return cli_->Put(path, headers, std::move(content_provider), content_type);
}
inline Result Client::Put(const std::string &path, const Params &params) {
  return cli_->Put(path, params);
}
inline Result Client::Put(const std::string &path, const Headers &headers,
                          const Params &params) {
  return cli_->Put(path, headers, params);
}
inline Result Client::Put(const std::string &path, const Headers &headers,
                          const Params &params, Progress progress) {
  return cli_->Put(path, headers, params, progress);
}
inline Result Client::Put(const std::string &path,
                          const MultipartFormDataItems &items) {
  return cli_->Put(path, items);
}
inline Result Client::Put(const std::string &path, const Headers &headers,
                          const MultipartFormDataItems &items) {
  return cli_->Put(path, headers, items);
}
inline Result Client::Put(const std::string &path, const Headers &headers,
                          const MultipartFormDataItems &items,
                          const std::string &boundary) {
  return cli_->Put(path, headers, items, boundary);
}
inline Result
Client::Put(const std::string &path, const Headers &headers,
            const MultipartFormDataItems &items,
            const MultipartFormDataProviderItems &provider_items) {
  return cli_->Put(path, headers, items, provider_items);
}
inline Result Client::Patch(const std::string &path) {
  return cli_->Patch(path);
}
inline Result Client::Patch(const std::string &path, const char *body,
                            size_t content_length,
                            const std::string &content_type) {
  return cli_->Patch(path, body, content_length, content_type);
}
inline Result Client::Patch(const std::string &path, const char *body,
                            size_t content_length,
                            const std::string &content_type,
                            Progress progress) {
  return cli_->Patch(path, body, content_length, content_type, progress);
}
inline Result Client::Patch(const std::string &path, const Headers &headers,
                            const char *body, size_t content_length,
                            const std::string &content_type) {
  return cli_->Patch(path, headers, body, content_length, content_type);
}
inline Result Client::Patch(const std::string &path, const Headers &headers,
                            const char *body, size_t content_length,
                            const std::string &content_type,
                            Progress progress) {
  return cli_->Patch(path, headers, body, content_length, content_type,
                     progress);
}
inline Result Client::Patch(const std::string &path, const std::string &body,
                            const std::string &content_type) {
  return cli_->Patch(path, body, content_type);
}
inline Result Client::Patch(const std::string &path, const std::string &body,
                            const std::string &content_type,
                            Progress progress) {
  return cli_->Patch(path, body, content_type, progress);
}
inline Result Client::Patch(const std::string &path, const Headers &headers,
                            const std::string &body,
                            const std::string &content_type) {
  return cli_->Patch(path, headers, body, content_type);
}
inline Result Client::Patch(const std::string &path, const Headers &headers,
                            const std::string &body,
                            const std::string &content_type,
                            Progress progress) {
  return cli_->Patch(path, headers, body, content_type, progress);
}
inline Result Client::Patch(const std::string &path, size_t content_length,
                            ContentProvider content_provider,
                            const std::string &content_type) {
  return cli_->Patch(path, content_length, std::move(content_provider),
                     content_type);
}
inline Result Client::Patch(const std::string &path,
                            ContentProviderWithoutLength content_provider,
                            const std::string &content_type) {
  return cli_->Patch(path, std::move(content_provider), content_type);
}
inline Result Client::Patch(const std::string &path, const Headers &headers,
                            size_t content_length,
                            ContentProvider content_provider,
                            const std::string &content_type) {
  return cli_->Patch(path, headers, content_length, std::move(content_provider),
                     content_type);
}
inline Result Client::Patch(const std::string &path, const Headers &headers,
                            ContentProviderWithoutLength content_provider,
                            const std::string &content_type) {
  return cli_->Patch(path, headers, std::move(content_provider), content_type);
}
inline Result Client::Delete(const std::string &path) {
  return cli_->Delete(path);
}
inline Result Client::Delete(const std::string &path, const Headers &headers) {
  return cli_->Delete(path, headers);
}
inline Result Client::Delete(const std::string &path, const char *body,
                             size_t content_length,
                             const std::string &content_type) {
  return cli_->Delete(path, body, content_length, content_type);
}
inline Result Client::Delete(const std::string &path, const char *body,
                             size_t content_length,
                             const std::string &content_type,
                             Progress progress) {
  return cli_->Delete(path, body, content_length, content_type, progress);
}
inline Result Client::Delete(const std::string &path, const Headers &headers,
                             const char *body, size_t content_length,
                             const std::string &content_type) {
  return cli_->Delete(path, headers, body, content_length, content_type);
}
inline Result Client::Delete(const std::string &path, const Headers &headers,
                             const char *body, size_t content_length,
                             const std::string &content_type,
                             Progress progress) {
  return cli_->Delete(path, headers, body, content_length, content_type,
                      progress);
}
inline Result Client::Delete(const std::string &path, const std::string &body,
                             const std::string &content_type) {
  return cli_->Delete(path, body, content_type);
}
inline Result Client::Delete(const std::string &path, const std::string &body,
                             const std::string &content_type,
                             Progress progress) {
  return cli_->Delete(path, body, content_type, progress);
}
inline Result Client::Delete(const std::string &path, const Headers &headers,
                             const std::string &body,
                             const std::string &content_type) {
  return cli_->Delete(path, headers, body, content_type);
}
inline Result Client::Delete(const std::string &path, const Headers &headers,
                             const std::string &body,
                             const std::string &content_type,
                             Progress progress) {
  return cli_->Delete(path, headers, body, content_type, progress);
}
inline Result Client::Options(const std::string &path) {
  return cli_->Options(path);
}
inline Result Client::Options(const std::string &path, const Headers &headers) {
  return cli_->Options(path, headers);
}

inline bool Client::send(Request &req, Response &res, Error &error) {
  return cli_->send(req, res, error);
}

inline Result Client::send(const Request &req) { return cli_->send(req); }

inline void Client::stop() { cli_->stop(); }

inline std::string Client::host() const { return cli_->host(); }

inline int Client::port() const { return cli_->port(); }

inline size_t Client::is_socket_open() const { return cli_->is_socket_open(); }

inline socket_t Client::socket() const { return cli_->socket(); }

inline void
Client::set_hostname_addr_map(std::map<std::string, std::string> addr_map) {
  cli_->set_hostname_addr_map(std::move(addr_map));
}

inline void Client::set_default_headers(Headers headers) {
  cli_->set_default_headers(std::move(headers));
}

inline void Client::set_header_writer(
    std::function<ssize_t(Stream &, Headers &)> const &writer) {
  cli_->set_header_writer(writer);
}

inline void Client::set_address_family(int family) {
  cli_->set_address_family(family);
}

inline void Client::set_tcp_nodelay(bool on) { cli_->set_tcp_nodelay(on); }

inline void Client::set_socket_options(SocketOptions socket_options) {
  cli_->set_socket_options(std::move(socket_options));
}

inline void Client::set_connection_timeout(time_t sec, time_t usec) {
  cli_->set_connection_timeout(sec, usec);
}

inline void Client::set_read_timeout(time_t sec, time_t usec) {
  cli_->set_read_timeout(sec, usec);
}

inline void Client::set_write_timeout(time_t sec, time_t usec) {
  cli_->set_write_timeout(sec, usec);
}

inline void Client::set_basic_auth(const std::string &username,
                                   const std::string &password) {
  cli_->set_basic_auth(username, password);
}
inline void Client::set_bearer_token_auth(const std::string &token) {
  cli_->set_bearer_token_auth(token);
}
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
inline void Client::set_digest_auth(const std::string &username,
                                    const std::string &password) {
  cli_->set_digest_auth(username, password);
}
#endif

inline void Client::set_keep_alive(bool on) { cli_->set_keep_alive(on); }
inline void Client::set_follow_location(bool on) {
  cli_->set_follow_location(on);
}

inline void Client::set_url_encode(bool on) { cli_->set_url_encode(on); }

inline void Client::set_compress(bool on) { cli_->set_compress(on); }

inline void Client::set_decompress(bool on) { cli_->set_decompress(on); }

inline void Client::set_interface(const std::string &intf) {
  cli_->set_interface(intf);
}

inline void Client::set_proxy(const std::string &host, int port) {
  cli_->set_proxy(host, port);
}
inline void Client::set_proxy_basic_auth(const std::string &username,
                                         const std::string &password) {
  cli_->set_proxy_basic_auth(username, password);
}
inline void Client::set_proxy_bearer_token_auth(const std::string &token) {
  cli_->set_proxy_bearer_token_auth(token);
}
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
inline void Client::set_proxy_digest_auth(const std::string &username,
                                          const std::string &password) {
  cli_->set_proxy_digest_auth(username, password);
}
#endif

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
inline void Client::enable_server_certificate_verification(bool enabled) {
  cli_->enable_server_certificate_verification(enabled);
}

inline void Client::enable_server_hostname_verification(bool enabled) {
  cli_->enable_server_hostname_verification(enabled);
}

inline void Client::set_server_certificate_verifier(
    std::function<SSLVerifierResponse(SSL *ssl)> verifier) {
  cli_->set_server_certificate_verifier(verifier);
}
#endif

inline void Client::set_logger(Logger logger) {
  cli_->set_logger(std::move(logger));
}

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
inline void Client::set_ca_cert_path(const std::string &ca_cert_file_path,
                                     const std::string &ca_cert_dir_path) {
  cli_->set_ca_cert_path(ca_cert_file_path, ca_cert_dir_path);
}

inline void Client::set_ca_cert_store(X509_STORE *ca_cert_store) {
  if (is_ssl_) {
    static_cast<SSLClient &>(*cli_).set_ca_cert_store(ca_cert_store);
  } else {
    cli_->set_ca_cert_store(ca_cert_store);
  }
}

inline void Client::load_ca_cert_store(const char *ca_cert, std::size_t size) {
  set_ca_cert_store(cli_->create_ca_cert_store(ca_cert, size));
}

inline long Client::get_openssl_verify_result() const {
  if (is_ssl_) {
    return static_cast<SSLClient &>(*cli_).get_openssl_verify_result();
  }
  return -1; // NOTE: -1 doesn't match any of X509_V_ERR_???
}

inline SSL_CTX *Client::ssl_context() const {
  if (is_ssl_) { return static_cast<SSLClient &>(*cli_).ssl_context(); }
  return nullptr;
}
#endif

// ----------------------------------------------------------------------------

} // namespace httplib

#endif // CPPHTTPLIB_HTTPLIB_H
```

### File: src/agent/built-in.cpp
```cpp
//
// #include "../../inc/Agent.hpp"
// #include "../../inc/MiniGemini.hpp"
// #include "../../inc/Tool.hpp"
// #include <algorithm> // for std::find_if, std::remove_if
// #include <cctype>    // for std::toupper
// #include <chrono>    // for timestamp
// #include <cstdio>    // for file, fgets
// #include <cstdlib>   // for popen, pclose, system
// #include <ctime>
// #include <fstream> // for file operations
// #include <iomanip> // for formatting time
// #include <iostream> #include <json/json.h>
// #include <sstream>
// #include <stdexcept>
//
// std::string Agent::summerizeTool(const Json::Value &params) {
//   if (!params.isMember("content") || !params["content"].isString()) {
//     return "Error [summarizeTool]: Missing or invalid string parameter "
//            "'content'.";
//   }
//   std::string content = params["content"].asString();
//   if (content.length() < 50) {
//     return "Content is too short to summarize effectively.";
//   }
//   logMessage(LogLevel::DEBUG, "Summarizing content (length: " +
//                                   std::to_string(content.length()) + ")");
//   try {
//     std::string task =
//         "Provide a concise summary of the following text:\n\n" + content;
//     std::string format = "{\"summary\": \"string (concise summary)\"}";
//     std::string llmResponse = executeTask(task, format);
//
//     Json::Value summaryJson;
//     Json::CharReaderBuilder readerBuilder;
//     std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
//     std::string errs;
//     if (reader->parse(llmResponse.c_str(),
//                       llmResponse.c_str() + llmResponse.length(), &summaryJson,
//                       &errs) &&
//         summaryJson.isObject() && summaryJson.isMember("summary") &&
//         summaryJson["summary"].isString()) {
//       return summaryJson["summary"].asString();
//     } else {
//       logMessage(
//           LogLevel::WARN,
//           "Failed to parse summary JSON from LLM, returning raw response.",
//           "LLM Response: " + llmResponse);
//       return llmResponse;
//     }
//   } catch (const std::exception &e) {
//     logMessage(LogLevel::ERROR, "Error during summarization task execution",
//                e.what());
//     return "Error [summarizeTool]: Exception during summarization: " +
//            std::string(e.what());
//   }
// }
//
// std::string Agent::summarizeHistory(const Json::Value &params) {
//   (void)params;
//   if (history.empty())
//     return "Conversation history is empty.";
//   std::string historyText = "Conversation History:\n";
//   for (const auto &entry : history)
//     historyText += entry.first + ": " + entry.second + "\n";
//   Json::Value summarizeParams;
//   summarizeParams["content"] = historyText;
//   return summerizeTool(summarizeParams);
// }
//
// std::string Agent::getWeather(const Json::Value &params) {
//   if (!params.isMember("location") || !params["location"].isString()) {
//     return "Error [getWeather]: Missing or invalid string parameter "
//            "'location'.";
//   }
//   std::string location = params["location"].asString();
//   std::string originalLocation = location;
//   std::replace(location.begin(), location.end(), ' ', '+');
//   std::string command =
//       "curl -s -L \"https://wttr.in/" + location + "?format=3\"";
//   std::string weatherResult;
//   int status = executeCommand(command, weatherResult);
//   weatherResult.erase(0, weatherResult.find_first_not_of(" \t\r\n"));
//   weatherResult.erase(weatherResult.find_last_not_of(" \t\r\n") + 1);
//   if (status != 0 || weatherResult.empty() ||
//       weatherResult.find("Unknown location") != std::string::npos ||
//       weatherResult.find("ERROR") != std::string::npos ||
//       weatherResult.find("Sorry") != std::string::npos) {
//     logMessage(LogLevel::WARN, "Failed to get weather using wttr.in",
//                "Command: " + command + ", Status: " + std::to_string(status) +
//                    ", Output: " + weatherResult);
//     return "Error [getWeather]: Could not retrieve weather for '" +
//            originalLocation + "'.";
//   }
//   return "Current weather for " + originalLocation + ": " + weatherResult;
// }
//
// std::string Agent::skip(const Json::Value &params) {
//   bool doSkip = false;
//   if (params.isMember("skip") && params["skip"].isBool()) {
//     doSkip = params["skip"].asBool();
//   } else {
//     return "Error [skip]: Missing or invalid boolean parameter 'skip'. "
//            "Example: {\"skip\": true}";
//   }
//
//   if (doSkip) {
//     this->setSkipFlowIteration(true);
//     return "Success [skip]: Final response generation for this turn will be "
//            "skipped.";
//   } else {
//     this->setSkipFlowIteration(false);
//     return "Success [skip]: Final response generation will proceed normally.";
//   }
// }
//
// std::string Agent::promptAgentTool(const Json::Value &params) {
//   if (!params.isMember("name") || !params["name"].isString() ||
//       !params.isMember("prompt") || !params["prompt"].isString()) {
//     return "Error [promptAgent]: Requires string parameters 'name' (target "
//            "agent) and 'prompt'.";
//   }
//   std::string agentName = params["name"].asString();
//   std::string userInput = params["prompt"].asString();
//
//   logMessage(LogLevel::INFO,
//              "Agent '" + name + "' is prompting agent '" + agentName + "'");
//
//   Agent *targetAgent = nullptr;
//   for (auto &agentPair : subAgents) {
//     if (agentPair.first == agentName) {
//       targetAgent = agentPair.second;
//       break;
//     }
//   }
//
//   if (targetAgent) {
//     try {
//       std::string contextualPrompt =
//           "Received prompt from Agent '" + name + "':\n" + userInput;
//       std::string response = targetAgent->prompt(contextualPrompt);
//       logMessage(LogLevel::INFO,
//                  "Received response from agent '" + agentName + "'");
//       return "Response from Agent '" + agentName + "':\n---\n" + response +
//              "\n---";
//     } catch (const std::exception &e) {
//       logMessage(LogLevel::ERROR, "Error prompting agent '" + agentName + "'",
//                  e.what());
//       return "Error [promptAgent]: Exception occurred while prompting agent '" +
//              agentName + "': " + e.what();
//     }
//   } else {
//     logMessage(LogLevel::WARN,
//                "Agent '" + agentName + "' not found for prompting.");
//     return "Error [promptAgent]: Agent '" + agentName + "' not found.";
//   }
// }
//
// // Provides help/descriptions for available tools.
// std::string Agent::getHelp(const Json::Value &params) {
//   std::ostringstream helpOss;
//   std::string specificTool;
//
//   if (params.isMember("tool_name") && params["tool_name"].isString()) {
//     specificTool = params["tool_name"].asString();
//   }
//
//   if (!specificTool.empty()) {
//     helpOss << "Help for tool '" << specificTool << "':\n";
//     bool found = false;
//     auto internalIt = internalToolDescriptions.find(specificTool);
//     if (internalIt != internalToolDescriptions.end()) {
//       helpOss << "- Type: Internal\n";
//       helpOss << "- Description & Params: " << internalIt->second;
//       found = true;
//     }
//     Tool *tool = getTool(specificTool);
//     if (tool) {
//       if (found)
//         helpOss << "\n---\n";
//       helpOss << "- Type: External\n";
//       helpOss << "- Description: " << tool->getDescription();
//       helpOss << "\n" << tool->getAllUseCaseCap(2); // Show examples
//       found = true;
//     }
//     if (!found) {
//       helpOss
//           << "Tool '" << specificTool
//           << "' not found. Use 'help' with no parameters to list all tools.";
//     }
//   } else {
//     helpOss << "Available Tools:\n";
//     helpOss << "--- Internal Tools ---\n";
//     for (const auto &pair : internalToolDescriptions) {
//       helpOss << "- " << pair.first << ": " << pair.second << "\n";
//     }
//     helpOss << "\n--- External Tools ---\n";
//     if (tools.empty()) {
//       helpOss << "(No external tools registered)\n";
//     } else {
//       for (const auto &pair : tools) {
//         helpOss << "- " << pair.second->getName() << ": "
//                 << pair.second->getDescription() << "\n";
//       }
//     }
//     helpOss << "\nUse help with {\"tool_name\": \"<tool_name>\"} for details.";
//   }
//   return helpOss.str();
// }
//
// std::string Agent::generateStamp(void) {
//   auto now = std::chrono::system_clock::now();
//   auto now_c = std::chrono::system_clock::to_time_t(now);
//   std::tm now_tm = *std::localtime(&now_c);
//   // *** FIX: Declare buffer as a char array ***
//   char buffer[80]; // Increased size to safely hold the timestamp
//   // Use strftime correctly with the buffer array
//   if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S%Z", &now_tm)) {
//     // *** FIX: Construct string from the null-terminated buffer ***
//     return std::string(buffer);
//   } else {
//     // Handle error if strftime fails
//     return "[Timestamp Error]";
//   }
// }
//
```

### File: src/agent/core.cpp
```cpp
#include "../../inc/Agent.hpp"
#include "../../inc/MiniGemini.hpp"
#include "../../inc/Tool.hpp"
#include <json/json.h>

std::string Agent::prompt(const std::string &userInput) {
  if (!userInput.empty()) {
    addToHistory("user", userInput);
  }
  currentIteration = 0;
  skipNextFlowIteration =
      false; // Reset skip flag at the beginning of a new prompt
  std::string finalAgentResponseToUser = "";

  // Agent Runtime loop.
  while (currentIteration < iterationLimit && !skipNextFlowIteration) {
    currentIteration++;
    logMessage(LogLevel::INFO, "Agent '" + agentName + "' Iteration " +
                                   std::to_string(currentIteration) + "/" +
                                   std::to_string(iterationLimit));

    std::string fullPromptText = buildFullPrompt();
    std::string llmRawResponse = executeApiCall(fullPromptText);

    trimLLMResponse(llmRawResponse); // Trim ```json ... ``` if present
    addToHistory("model",
                 llmRawResponse); // Add possibly trimmed response to history

    std::string llmStatus;
    std::vector<Thought> llmThoughts;
    std::vector<Action> llmActions;
    std::string llmFinalResponseField;

    bool parsedOk =
        parseStructuredLLMResponse(llmRawResponse, llmStatus, llmThoughts,
                                   llmActions, llmFinalResponseField);

    if (!parsedOk) {
      logMessage(LogLevel::ERROR,
                 "Failed to parse structured LLM response. Agent '" +
                     agentName + "' treating raw response as final.",
                 llmRawResponse);
      finalAgentResponseToUser =
          llmRawResponse;             // Use the (trimmed) raw response
      setSkipNextFlowIteration(true); // End loop
      continue;
    }

    for (const auto &thought : llmThoughts) {
      logMessage(LogLevel::DEBUG,
                 "LLM Thought (" + thought.type + ") for agent '" + agentName +
                     "':",
                 thought.content);
    }

    if (llmStatus == "REQUIRES_CLARIFICATION") {
      logMessage(LogLevel::INFO,
                 "LLM requires clarification for agent '" + agentName + "'.");
      finalAgentResponseToUser = "Agent needs clarification."; // Default
      for (const auto &action :
           llmActions) { // Check if there's a specific query in actions
        if (action.action == "request_user_input" &&
            action.params.isMember("query_to_user") &&
            action.params["query_to_user"].isString()) {
          finalAgentResponseToUser = action.params["query_to_user"].asString();
          break;
        }
      }
      setSkipNextFlowIteration(true);
    } else if (llmStatus == "REQUIRES_ACTION" && !llmActions.empty()) {
      logMessage(LogLevel::INFO,
                 "LLM requested actions for agent '" + agentName + "'.");
      std::string actionResultsText = processActions(llmActions);
      addToHistory("action_results",
                   actionResultsText); // Feed results back for next iteration
    } else if (llmStatus == "SUCCESS_FINAL") {
      logMessage(LogLevel::INFO,
                 "LLM provided final response for agent '" + agentName + "'.");
      finalAgentResponseToUser = llmFinalResponseField;
      // Check for "output" type action "send_response" to potentially override
      // if final_response field was empty
      for (const auto &action : llmActions) {
        if (action.type == "output" && action.action == "send_response") {
          if (finalAgentResponseToUser.empty() &&
              action.params.isMember("text") &&
              action.params["text"].isString()) {
            finalAgentResponseToUser = action.params["text"].asString();
            logMessage(
                LogLevel::DEBUG,
                "Overriding final response with 'send_response' action text.",
                finalAgentResponseToUser);
          }
          break; // Only one send_response action should dictate the final
                 // output here.
        }
      }
      setSkipNextFlowIteration(true);
    } else if (llmStatus == "ERROR_INTERNAL") {
      logMessage(LogLevel::ERROR,
                 "LLM indicated an internal error for agent '" + agentName +
                     "'.",
                 llmFinalResponseField);
      finalAgentResponseToUser = llmFinalResponseField.empty()
                                     ? "LLM reported an internal error."
                                     : llmFinalResponseField;
      setSkipNextFlowIteration(true);
    } else { // Unknown status or valid status but logic error (e.g.
             // REQUIRES_ACTION with no actions)
      logMessage(LogLevel::WARN,
                 "LLM response has unclear status/action state for agent '" +
                     agentName + "'. Treating as final.",
                 "Status: " + llmStatus + " | Raw: " + llmRawResponse);
      finalAgentResponseToUser = llmFinalResponseField.empty()
                                     ? llmRawResponse
                                     : llmFinalResponseField;
      setSkipNextFlowIteration(true);
    }
  } // End while loop

  if (currentIteration >= iterationLimit && !skipNextFlowIteration) {
    logMessage(LogLevel::WARN,
               "Iteration limit reached for agent '" + agentName +
                   "' without a conclusive response.",
               std::to_string(iterationLimit));
    if (finalAgentResponseToUser.empty()) {
      finalAgentResponseToUser =
          "Agent '" + agentName + "' reached iteration limit (" +
          std::to_string(iterationLimit) + ") without a final answer.";
      // Try to find a sensible last "model" message from history that wasn't an
      // action request
      for (auto it = conversationHistory.rbegin();
           it != conversationHistory.rend(); ++it) {
        if (it->first == "model") {
          Json::Reader reader;
          Json::Value parsedJson;
          if (reader.parse(it->second, parsedJson) &&
              parsedJson.isMember("status") &&
              parsedJson["status"].asString() == "SUCCESS_FINAL" &&
              parsedJson.isMember("final_response")) {
            finalAgentResponseToUser = parsedJson["final_response"].asString();
            break;
          } else if (it->second.find("\"actions\": []") != std::string::npos &&
                     it->second.find("REQUIRES_ACTION") == std::string::npos) {
            // A bit heuristic: if it looks like a final JSON without actions.
            finalAgentResponseToUser = it->second;
            break;
          }
        }
      }
    }
  }
  // Ensure skip flag is reset for the next independent call to prompt()
  // skipNextFlowIteration = false; // This is now reset at the start of
  // prompt()

  logMessage(LogLevel::INFO,
             "Final response for agent '" + agentName + "' after prompt cycle:",
             finalAgentResponseToUser.substr(0, 200) +
                 (finalAgentResponseToUser.length() > 200 ? "..." : ""));
  return finalAgentResponseToUser;
}
```

### File: src/agent.cpp
```cpp
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
```

### File: src/agent/export.cpp
```cpp
// #include "../../inc/Agent.hpp" // Your Agent class header
// #include <string>
// #include <vector>
// #include <fstream>
// #include <iostream> // For basic error output in this example
// #include <stdexcept>
//
// // Include yaml-cpp headers
// #include <yaml-cpp/yaml.h>
// #include <yaml-cpp/emitter.h> // For exporting
//                               
// // --- Export Function ---
// // Saves the current configuration OF an Agent object TO a YAML file.
// // Only saves configurable profile aspects, not full runtime state.
// // Returns true on success, false on failure.
// bool saveAgentProfile(const Agent& agentToSave, const std::string& yamlPath) {
//     logMessage(LogLevel::INFO, "Attempting to save agent profile", yamlPath);
//     YAML::Emitter emitter;
//     try {
//         emitter << YAML::BeginMap;
//
//         // Add a version marker (good practice)
//         emitter << YAML::Key << "version" << YAML::Value << "agent-1.1"; // Example version
//
//         // --- Save Core Identity & Configuration ---
//         emitter << YAML::Key << "name" << YAML::Value << agentToSave.getName();
//         emitter << YAML::Key << "description" << YAML::Value << YAML::Literal << agentToSave.getDescription(); // Use Literal for multi-line
//         emitter << YAML::Key << "system_prompt" << YAML::Value << YAML::Literal << agentToSave.getSystemPrompt(); // Use Literal for multi-line
//         emitter << YAML::Key << "iteration_cap" << YAML::Value << agentToSave.getIterationCap(); // Assuming getter exists
//
//         // --- Save Environment ---
//         // Assuming Agent has a way to get environment variables, e.g., getEnv() returning a map or vector<pair>
//         const auto& envVars = agentToSave.getEnv(); // *** ASSUMPTION: getEnv() exists and returns suitable type ***
//         if (!envVars.empty()) {
//             emitter << YAML::Key << "environment" << YAML::Value << YAML::BeginMap;
//             for (const auto& pair : envVars) {
//                 emitter << YAML::Key << pair.first << YAML::Value << pair.second;
//             }
//             emitter << YAML::EndMap;
//         }
//
//         // --- Save Optional Sections ---
//         // Assuming Agent has getters for these, e.g., getExtraPrompts(), getTasks()
//         const auto& extraPrompts = agentToSave.getExtraPrompts(); // *** ASSUMPTION: getExtraPrompts() exists ***
//         if (!extraPrompts.empty()) {
//             emitter << YAML::Key << "extra_prompts" << YAML::Value << YAML::BeginSeq;
//             for (const auto& prompt : extraPrompts) {
//                 emitter << prompt;
//             }
//             emitter << YAML::EndSeq;
//         }
//         const auto& tasks = agentToSave.getTasks(); // *** ASSUMPTION: getTasks() exists ***
//         if (!tasks.empty()) {
//             emitter << YAML::Key << "tasks" << YAML::Value << YAML::BeginSeq;
//             for (const auto& task : tasks) {
//                 emitter << task;
//             }
//             emitter << YAML::EndSeq;
//         }
//
//         // --- Save Directive ---
//         const auto& directive = agentToSave.getDirective(); // *** ASSUMPTION: getDirective() exists ***
//         emitter << YAML::Key << "directive" << YAML::Value << YAML::BeginMap;
//         // Convert enum back to string
//         std::string typeStr = "NORMAL"; // Default
//         switch (directive.type) {
//             case Agent::DIRECTIVE::Type::BRAINSTORMING: typeStr = "BRAINSTORMING"; break;
//             case Agent::DIRECTIVE::Type::AUTONOMOUS: typeStr = "AUTONOMOUS"; break;
//             case Agent::DIRECTIVE::Type::EXECUTE: typeStr = "EXECUTE"; break;
//             case Agent::DIRECTIVE::Type::REPORT: typeStr = "REPORT"; break;
//             case Agent::DIRECTIVE::Type::NORMAL: // Fallthrough
//             default: typeStr = "NORMAL"; break;
//         }
//         emitter << YAML::Key << "type" << YAML::Value << typeStr;
//         emitter << YAML::Key << "description" << YAML::Value << directive.description;
//         emitter << YAML::Key << "format" << YAML::Value << directive.format;
//         emitter << YAML::EndMap;
//
//         // --- What is NOT saved ---
//         // - Runtime state (iteration, skipFlowIteration, scratchpad)
//         // - History
//         // - Memory state (long/short term)
//         // - Tool definitions/instances (might save *list* of expected tool *names* if needed)
//         // - Sub-agent references/definitions
//
//         emitter << YAML::EndMap;
//
//         // Write to file
//         std::ofstream fout(yamlPath);
//         if (!fout.is_open()) {
//             logMessage(LogLevel::ERROR, "Failed to open file for saving agent profile", yamlPath);
//             return false;
//         }
//         fout << emitter.c_str();
//         fout.close();
//
//         if (!emitter.good()) {
//              logMessage(LogLevel::ERROR, "YAML emitter error after saving agent profile", emitter.GetLastError());
//              return false; // Indicate potential issue even if file write succeeded superficially
//         }
//
//         logMessage(LogLevel::INFO, "Successfully saved agent profile", yamlPath);
//         return true;
//
//     } catch (const YAML::Exception& e) {
//         logMessage(LogLevel::ERROR, "YAML emitter error during agent profile save", yamlPath + ": " + e.what());
//         return false;
//     } catch (const std::exception& e) {
//         logMessage(LogLevel::ERROR, "Generic error saving agent profile", yamlPath + ": " + e.what());
//         return false;
//     }
// }
//
//
```

### File: src/agent/import.cpp
```cpp
#include "../../inc/Agent.hpp"
#include "../../inc/Tool.hpp"  // Include the simplified Tool.hpp
#include "../../inc/Utils.hpp" // For executeScriptTool
#include <fstream>
#include <iostream>
#include <memory> // For std::make_unique if Agent stores ToolPtr
#include <stdexcept>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>


// ascii json graph tool

// return a buffer for a simple json graph in ascii pillars
std::string simpleJsonGraph(const Json::Value &jsonValue) {
  std::string graph;
  for (const auto &key : jsonValue.getMemberNames()) {
    graph += key + ": " + jsonValue[key].asString() + "\n";
  }
  return graph;
}

// display any json table infinite collumns and rows
std::string jsonTable(const Json::Value &jsonValue) {
  std::string table;
  for (const auto &key : jsonValue.getMemberNames()) {
    table += key + ": " + jsonValue[key].asString() + "\n";
  }
  return table;
}


// Utility
// file expansion utility function, can extract the file name from a path
std::string getFileName(const std::string &filePath) {
  size_t lastSlash = filePath.find_last_of("/\\");
  if (lastSlash == std::string::npos) {
    return filePath; // No path, return the file name
  }
  return filePath.substr(lastSlash + 1);
}
// relative path utility function, can extract the relative path from a full path
std::string getRelativePath(const std::string &filePath) {
  size_t lastSlash = filePath.find_last_of("/\\");
  if (lastSlash == std::string::npos) {
    return ""; // No path, return empty string
  }
  return filePath.substr(0, lastSlash);
}

// get file content relative mode
std::string getFileContent(const std::string &filePath) {
  std::ifstream file(filePath);
  if (!file.is_open()) {
    logMessage(LogLevel::ERROR, "Failed to open file", filePath);
    return "";
  }
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();
  return content;
}

// takes string + std::vector<string, string> env expands the string
std::string expandStringWithEnv(const std::string &input,
                                         const StringKeyValuePair &env) {
  std::string expanded = input;
  for (const auto &pair : env) {
    std::string placeholder = "{" + pair.first + "}";
    size_t pos = expanded.find(placeholder);
    if (pos != std::string::npos) {
      expanded.replace(pos, placeholder.length(), pair.second);
    }
  }
  return expanded;
}

// should be accessible via the state namespace
// class runtimeEnv { // reference to the runtime environment/context
//     private:
//         std::string id;
//         std::vector<std::string, std::string> env;
//         std::vector<std::string, std::string> variables;
//     public:
//
//
// };

// expand paths in $(PATH) format with xml bindings
const std::string expandPathWithEnv(const std::string &input,
                                         const StringKeyValuePair &env) {
  std::string expanded = input;
  for (const auto &pair : env) {
    std::string placeholder = "$(" + pair.first + ")";
    size_t pos = expanded.find(placeholder);
    if (pos != std::string::npos) {
      expanded.replace(pos, placeholder.length(), pair.second);
    }
  }
  return expanded;
}
// one without env, will expand any/all $(PATH) format 
// const std::string expandPath(const std::string &input) {


bool loadAgentProfile(Agent &agentToConfigure, const std::string &yamlPath) {

  logMessage(LogLevel::INFO,
             "Attempting to load agent profile (B-line Tool Import)", yamlPath);
  YAML::Node config;

  try {
    std::ifstream f(yamlPath.c_str());
    if (!f.good()) {
      logMessage(LogLevel::ERROR, "Agent profile file not found", yamlPath);
      return false;
    }
    f.close();
    config = YAML::LoadFile(yamlPath);

    // --- Load Core Identity & Configuration (Name, Description, System Prompt,
    // etc.) ---
    if (config["name"] && config["name"].IsScalar()) {
      agentToConfigure.setName(config["name"].as<std::string>());
    }
    if (config["description"] && config["description"].IsScalar()) {
      agentToConfigure.setDescription(config["description"].as<std::string>());
    }
    // check if system_prompt is a scalar or markdown file relative to the yamlPath i.e config/agents/agent.yaml | config/agents/sysprompts/sysprompt.md
    if (config["system_prompt"] && config["system_prompt"].IsScalar()) {
      std::string systemPrompt = config["system_prompt"].as<std::string>();
      if (systemPrompt.find(".md") != std::string::npos) {
        // Load the markdown file
        std::ifstream promptFile(yamlPath.substr(0, yamlPath.find_last_of("/")) +
                                  "/" + systemPrompt);
        if (promptFile.good()) {
          std::string line;
          std::string fullPrompt;
          while (std::getline(promptFile, line)) {
            fullPrompt += line + "\n";
          }
          agentToConfigure.setSystemPrompt(fullPrompt);
        } else {
          logMessage(LogLevel::ERROR,
                     "System prompt file not found", systemPrompt);
        }
      } else {
        agentToConfigure.setSystemPrompt(systemPrompt);
      }
    }
    if (config["schema"] && config["schema"].IsScalar()) {
      agentToConfigure.setSchema(config["schema"].as<std::string>());
    }
    if (config["example"] && config["example"].IsScalar()) {
      agentToConfigure.setExample(config["example"].as<std::string>());
    }
    if (config["iteration_cap"] && config["iteration_cap"].IsScalar()) {
      try {
        agentToConfigure.setIterationCap(config["iteration_cap"].as<int>());
      } catch (const YAML::BadConversion &e) {
        logMessage(
            LogLevel::WARN,
            "Invalid iteration_cap value in profile, using agent's default",
            yamlPath + ": " + e.what());
      }
    }
    // ... (load environment, extra_prompts, tasks, directive as before) ... 
    if (config["environment"] && config["environment"].IsMap()) { // ability to add .env and env from files in general
      for (const auto &it : config["environment"]) {
        std::string key = it.first.as<std::string>();
        std::string value = it.second.as<std::string>();
        agentToConfigure.addEnvironmentVariable(key, value);
      }
    }
    if (config["extra_prompts"] && config["extra_prompts"].IsSequence()) {
      for (const auto &item : config["extra_prompts"]) {
        if (item.IsScalar()) {
          agentToConfigure.addExtraSystemPrompt(item.as<std::string>());
        }
      }
    }
    // ... (rest of the non-tool loading logic) ...

    // --- Load Tools ---
    if (config["tools"] && config["tools"].IsMap()) {
      for (const auto &toolConfigEntryNode : config["tools"]) {
        std::string logicalToolKey =
            toolConfigEntryNode.first.as<std::string>(); // e.g., "BashExecutor"
        YAML::Node toolDef = toolConfigEntryNode.second;

        if (!toolDef["name"] || !toolDef["name"].IsScalar() ||
            !toolDef["description"] || !toolDef["description"].IsScalar() ||
            !toolDef["type"] || !toolDef["type"].IsScalar()) {
          logMessage(LogLevel::WARN,
                     "Skipping malformed tool definition in YAML (missing "
                     "name, desc, or type)",
                     logicalToolKey);
          continue;
        }

        std::string actualToolName = toolDef["name"].as<std::string>();
        std::string description = toolDef["description"].as<std::string>();
        std::string type = toolDef["type"].as<std::string>();
        std::string runtime;

        if (toolDef["runtime"] && toolDef["runtime"].IsScalar()) {
          runtime = toolDef["runtime"].as<std::string>();
        } else if (type == "code" || type == "file") {
          logMessage(LogLevel::WARN,
                     "Skipping tool due to missing 'runtime' for type 'code' "
                     "or 'file'",
                     actualToolName);
          continue;
        }

        // Create a new Tool instance.
        // Agent will own this pointer. If using unique_ptr, adjust
        // agent.addTool.
        Tool *newTool = new Tool(actualToolName, description);

        FunctionalToolCallback toolLambdaCallback;

        if (type == "code") {
          if (!toolDef["code"] || !toolDef["code"].IsScalar()) {
            logMessage(
                LogLevel::WARN,
                "Skipping 'code' tool with missing or invalid 'code' block",
                actualToolName);
            delete newTool;
            continue;
          }
          std::string scriptContent = toolDef["code"].as<std::string>();

          toolLambdaCallback = [scriptContent, runtime, actualToolName](
                                   const Json::Value &params) -> std::string {
            // Capture necessary variables by value for the lambda
            return executeScriptTool(scriptContent, runtime, params,
                                     true /*isContentInline*/);
          };

        } else if (type == "file") {
          if (!toolDef["path"] || !toolDef["path"].IsScalar()) {
            logMessage(LogLevel::WARN,
                       "Skipping 'file' tool with missing or invalid 'path'",
                       actualToolName);
            delete newTool;
            continue;
          }
          std::string scriptPath = toolDef["path"].as<std::string>();

          toolLambdaCallback = [scriptPath, runtime, actualToolName](
                                   const Json::Value &params) -> std::string {
            // Capture necessary variables by value
            return executeScriptTool(scriptPath, runtime, params,
                                     false /*isContentInline*/);
          };
        } else {
          logMessage(LogLevel::WARN, "Unsupported tool type in YAML",
                     type + " for tool " + actualToolName);
          delete newTool;
          continue;
        }

        newTool->setCallback(toolLambdaCallback);
        agentToConfigure.addTool(
            newTool); // Agent takes ownership if storing raw Tool*
                      // If Agent stores unique_ptr<Tool>, use:
                      // agentToConfigure.addTool(std::unique_ptr<Tool>(newTool));
        logMessage(LogLevel::INFO, "Loaded and registered tool from YAML",
                   actualToolName + " (Type: " + type + ")");
      }
    }

    logMessage(LogLevel::INFO, "Successfully loaded agent profile", yamlPath);
    return true;

  } catch (const YAML::Exception &e) {
    logMessage(LogLevel::ERROR, "Failed to load or parse agent profile YAML",
               yamlPath + ": " + e.what());
    return false;
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR, "Generic error loading agent profile",
               yamlPath + ": " + e.what());
    return false;
  }
}
```

### File: src/agent/parse.cpp
```cpp
#include "../../inc/Agent.hpp"
#include "../../inc/MiniGemini.hpp"
#include "../../inc/Tool.hpp"

bool Agent::parseStructuredLLMResponse(const std::string &jsonString,
                                       std::string &status,
                                       std::vector<Thought> &thoughts,
                                       std::vector<Action> &actions,
                                       std::string &finalResponseField) {
  status = "ERROR_INTERNAL_PARSE"; // Default status if parsing fails badly
  thoughts.clear();
  actions.clear();
  finalResponseField = "";

  Json::Value root;
  Json::CharReaderBuilder readerBuilder;
  std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());

  std::string parseErrors;

  if (!reader->parse(jsonString.c_str(),
                     jsonString.c_str() + jsonString.length(), &root,
                     &parseErrors)) {
    logMessage(LogLevel::ERROR,
               "Failed to parse LLM response JSON for agent '" + agentName +
                   "'.",
               "Errors: " + parseErrors + "\nInput: " + jsonString);
    finalResponseField = jsonString; // Fallback to raw response
    return false;
  }

  if (!root.isObject()) {
    logMessage(LogLevel::ERROR,
               "LLM response root is not a JSON object for agent '" +
                   agentName + "'.",
               jsonString);
    finalResponseField = jsonString;
    return false;
  }

  // Status (Required)
  if (root.isMember("status") && root["status"].isString()) {
    status = root["status"].asString();
  } else {
    logMessage(LogLevel::ERROR,
               "LLM JSON missing required 'status' (string) field for agent '" +
                   agentName + "'.",
               jsonString);
    // status remains ERROR_INTERNAL_PARSE or could be set to a more specific
    // missing field error
    return false; // Critical field missing
  }

  // Thoughts (Required, can be empty array)
  if (root.isMember("thoughts") && root["thoughts"].isArray()) {
    for (const auto &thoughtVal : root["thoughts"]) {
      if (thoughtVal.isObject() && thoughtVal.isMember("type") &&
          thoughtVal["type"].isString() && thoughtVal.isMember("content") &&
          thoughtVal["content"].isString()) {
        thoughts.push_back(
            {thoughtVal["type"].asString(), thoughtVal["content"].asString()});
      } else {
        logMessage(LogLevel::WARN,
                   "Malformed thought object in LLM JSON for agent '" +
                       agentName + "'. Skipping.",
                   thoughtVal.toStyledString());
      }
    }
  } else {
    logMessage(
        LogLevel::ERROR,
        "LLM JSON missing required 'thoughts' (array) field for agent '" +
            agentName + "'.",
        jsonString);
    return false; // Critical field missing
  }

  // Actions (Required, can be empty array)
  if (root.isMember("actions") && root["actions"].isArray()) {
    for (const auto &actionVal : root["actions"]) {
      if (actionVal.isObject() && actionVal.isMember("action") &&
          actionVal["action"].isString() && actionVal.isMember("type") &&
          actionVal["type"].isString() && actionVal.isMember("params") &&
          actionVal["params"].isObject()) {
        Action ai;
        ai.action = actionVal["action"].asString();
        ai.type = actionVal["type"].asString();
        ai.params = actionVal["params"];
        if (actionVal.isMember("confidence") &&
            actionVal["confidence"].isNumeric()) {
          ai.confidence = actionVal["confidence"].asDouble();
        }
        if (actionVal.isMember("warnings") && actionVal["warnings"].isArray()) {
          for (const auto &warnVal : actionVal["warnings"]) {
            if (warnVal.isString())
              ai.warnings.push_back(warnVal.asString());
          }
        }
        actions.push_back(ai);
      } else {
        logMessage(LogLevel::WARN,
                   "Malformed action object in LLM JSON for agent '" +
                       agentName + "'. Skipping.",
                   actionVal.toStyledString());
      }
    }
  } else {
    logMessage(LogLevel::ERROR,
               "LLM JSON missing required 'actions' (array) field for agent '" +
                   agentName + "'.",
               jsonString);
    return false; // Critical field missing
  }

  // Final Response (Optional: string or null)
  if (root.isMember("final_response")) {
    if (root["final_response"].isString()) {
      finalResponseField = root["final_response"].asString();
    } else if (root["final_response"].isNull()) {
      finalResponseField = ""; // Represent null as empty string
    } else {
      logMessage(LogLevel::WARN,
                 "LLM JSON 'final_response' field is present but not string or "
                 "null for agent '" +
                     agentName + "'.",
                 root["final_response"].toStyledString());
    }
  } else {
    logMessage(LogLevel::DEBUG,
               "LLM JSON does not have a 'final_response' field for agent '" +
                   agentName +
                   "'. This is often normal if actions are present.");
  }

  // Consistency checks (optional, but good for debugging LLM behavior)
  if (status == "SUCCESS_FINAL" && !actions.empty()) {
    logMessage(LogLevel::WARN,
               "LLM JSON: Status is SUCCESS_FINAL but 'actions' array is not "
               "empty for agent '" +
                   agentName + "'.",
               jsonString);
  }
  if ((status == "REQUIRES_ACTION" || status == "REQUIRES_CLARIFICATION") &&
      actions.empty() && finalResponseField.empty()) {
    // If status needs action/clarification but there are no actions AND no
    // final_response to ask the user, it's a problematic state.
    logMessage(LogLevel::WARN,
               "LLM JSON: Status '" + status +
                   "' but 'actions' array is empty and 'final_response' is "
                   "also empty for agent '" +
                   agentName + "'.",
               jsonString);
  }

  return true;
}
```

### File: src/agent/prompt.cpp
```cpp
#include "../../inc/Agent.hpp" // Assuming this is the correct path

std::string Agent::buildFullPrompt() const {
  std::stringstream promptSs;
  if (!systemPrompt.empty()) {
    promptSs << "<system_prompt>\n" << systemPrompt << "\n</system_prompt>\n\n";
  }
  if (!llmResponseSchema.empty()) {
    promptSs << "<agent_reply_schema>\n"
             << llmResponseSchema << "\n</agent_reply_schema>\n\n";
  }
  if (!llmResponseExample.empty()) {
    promptSs << "<agent_reply_example>\n"
             << llmResponseExample << "\n</agent_reply_example>\n\n";
  }

  promptSs << "<agent_context>\n";
  promptSs << "\t<name>" << agentName << "</name>\n";
  if (!agentDescription.empty())
    promptSs << "\t<description>" << agentDescription << "</description>\n";
  promptSs << "</agent_context>\n\n";

  if (!environmentVariables.empty()) {
    promptSs << "<environment_variables>\n";
    for (const auto &pair : environmentVariables) {
      promptSs << "\t<variable name=\"" << pair.first << "\">" << pair.second
               << "</variable>\n";
    }
    promptSs << "</environment_variables>\n\n";
  }

  std::map<std::string, std::string> allAvailableActions =
      internalFunctionDescriptions;
  for (const auto &pair : registeredTools) {
    if (pair.second)
      allAvailableActions[pair.first] = pair.second->getDescription();
  }
  if (!allAvailableActions.empty()) {
    promptSs << "<available_actions>\n";
    for (const auto &pair : allAvailableActions) {
      promptSs << "\t<action name=\"" << pair.first << "\">\n";
      promptSs << "\t\t<description>" << pair.second << "</description>\n";
      // Could add param schema here in future
      promptSs << "\t</action>\n";
    }
    promptSs << "</available_actions>\n\n";
  }

  if (currentDirective.type != AgentDirective::Type::NORMAL ||
      !currentDirective.description.empty()) {
    promptSs << "<current_directive>\n";
    promptSs << "\t<type>" << directiveTypeToString(currentDirective.type)
             << "</type>\n";
    if (!currentDirective.description.empty())
      promptSs << "\t<description>" << currentDirective.description
               << "</description>\n";
    if (!currentDirective.format.empty())
      promptSs << "\t<expected_format>" << currentDirective.format
               << "</expected_format>\n";
    promptSs << "</current_directive>\n\n";
  }

  if (!extraSystemPrompts.empty()) {
    promptSs << "<additional_instructions>\n";
    for (const auto &p : extraSystemPrompts)
      promptSs << "\t" << p << "\n";
    promptSs << "</additional_instructions>\n\n";
  }

  // Memory context
  std::string memoryBlock;
  if (!scratchpad.empty()) {
    memoryBlock += "\t<scratchpad>\n";
    for (const auto &item : scratchpad)
      memoryBlock +=
          "\t\t<item key=\"" + item.first + "\">" + item.second + "</item>\n";
    memoryBlock += "\t</scratchpad>\n";
  }
  if (!shortTermMemory.empty()) {
    memoryBlock += "\t<short_term_memory>\n";
    for (const auto &item : shortTermMemory)
      memoryBlock += "\t\t<entry role=\"" + item.first + "\">" + item.second +
                     "</entry>\n";
    memoryBlock += "\t</short_term_memory>\n";
  }
  if (!longTermMemory
           .empty()) { // Assuming long term memory might be more substantial,
                       // could be summarized or selectively included
    memoryBlock += "\t<long_term_memory_summary>\n"; // Placeholder for now
    for (const auto &item : longTermMemory)
      memoryBlock += "\t\t<entry role=\"" + item.first + "\">" +
                     item.second.substr(0, 100) + "...</entry>\n";
    memoryBlock += "\t</long_term_memory_summary>\n";
  }
  if (!memoryBlock.empty()) {
    promptSs << "<memory_context>\n" << memoryBlock << "</memory_context>\n\n";
  }

  if (!conversationHistory.empty()) {
    promptSs << "<conversation_history>\n";
    for (const auto &entry : conversationHistory) {
      promptSs << "\t<turn role=\"" << entry.first << "\">\n";
      // Basic XML escaping for content - just & and < for this example
      std::string escapedContent = entry.second;
      size_t pos = 0;
      while ((pos = escapedContent.find('&', pos)) != std::string::npos) {
        escapedContent.replace(pos, 1, "&");
        pos += 5;
      }
      pos = 0;
      while ((pos = escapedContent.find('<', pos)) != std::string::npos) {
        escapedContent.replace(pos, 1, "<");
        pos += 4;
      }

      std::stringstream contentSs(escapedContent); // Use escaped content
      std::string line;
      while (std::getline(contentSs, line)) {
        promptSs << "\t\t" << line << "\n";
      }
      promptSs << "\t</turn>\n";
    }
    promptSs << "</conversation_history>\n\n";
  }

  // schema && Example listing
  if (!llmResponseSchema.empty()) {
    promptSs << "<schema>\n" << llmResponseSchema << "\n</schema>\n\n";
  }
  if (!llmResponseExample.empty()) {
    promptSs << "<example>\n" << llmResponseExample << "\n</example>\n\n";
  }

  promptSs << "CONTEXT END\n\n";
  // promptSs << "RESPONSE INSTRUCTIONS: Based on the entire context above, "
  //             "generate your response strictly as a single JSON object. "
  //          << "This JSON object MUST contain 'status' (string), 'thoughts' "
  //             "(array of objects with 'type' and 'content'), "
  //          << "'actions' (array of objects with 'action', 'type', 'params'; "
  //             "use '[]' if no actions), "
  //          << "and 'final_response' (string or null; typically null if
  //          actions "
  //             "are present).";

  // logMessage(LogLevel::PROMPT, "Full prompt for agent " + agentName + ":",
  // promptSs.str()); // Can be very verbose
  return promptSs.str();
}
```

### File: src/agent/runtime.cpp
```cpp

#include "../../inc/Agent.hpp" // Your Agent class header


```

### File: src/agent/tool.cpp
```cpp
#include "../../inc/Tool.hpp" // Adjust path as necessary

Tool::Tool(const std::string& toolName, const std::string& toolDescription)
    : name(toolName), description(toolDescription), functionalCallback(nullptr) {}

Tool::Tool() : functionalCallback(nullptr) {}

std::string Tool::getName() const {
    return name;
}

std::string Tool::getDescription() const {
    return description;
}

void Tool::setName(const std::string& toolName) {
    name = toolName;
}

void Tool::setDescription(const std::string& toolDescription) {
    description = toolDescription;
}

void Tool::setCallback(FunctionalToolCallback callback) {
    if (!callback) {
        // Optional: Log a warning or throw if setting a null callback is invalid
        // For this B-line, we allow it, execute will check.
    }
    functionalCallback = callback;
}

std::string Tool::execute(const Json::Value& params) {
    if (functionalCallback) {
        return functionalCallback(params);
    }
    // Throw an error or return an error message if no callback is set
    // logMessage(LogLevel::ERROR, "Tool execute called on '" + name + "' but no callback is set.");
    throw std::runtime_error("No valid callback function set for tool '" + name + "'");
}
```

### File: src/groqClient.cpp
```cpp
#include "../inc/Groq.hpp"
#include <curl/curl.h>
#include <json/json.h>
#include <sstream>
#include <cstdlib> // For getenv
#include <stdexcept>
#include <iostream> // For potential debug logging
#include <memory>   // For unique_ptr

// Libcurl write callback
size_t GroqClient::writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Constructor implementation
GroqClient::GroqClient(const std::string& apiKey) :
    // Sensible defaults for Groq
    m_model("llama3-8b-8192"), // Example Groq model
    m_temperature(0.5),
    m_maxTokens(4096),
    m_baseUrl("https://api.groq.com/openai/v1") // OpenAI-compatible endpoint base
{
    if (!apiKey.empty()) {
        m_apiKey = apiKey;
    } else {
        const char* envKey = std::getenv("GROQ_API_KEY");
        if (envKey != nullptr && envKey[0] != '\0') {
            m_apiKey = envKey;
             std::cout << "[INFO] GroqClient: Using API key from GROQ_API_KEY environment variable." << std::endl;
        } else {
            std::cerr << "[WARN] GroqClient: API key not provided via constructor or GROQ_API_KEY env var. API calls will likely fail." << std::endl;
            // throw std::invalid_argument("Groq API key required: Provide via constructor or GROQ_API_KEY env var");
        }
    }
    // curl_global_init in main()
}

// Configuration Setters
void GroqClient::setApiKey(const std::string& apiKey) { m_apiKey = apiKey; }
void GroqClient::setModel(const std::string& model) { m_model = model; }
void GroqClient::setTemperature(double temperature) { m_temperature = temperature; }
void GroqClient::setMaxTokens(int maxTokens) { m_maxTokens = maxTokens; }
void GroqClient::setBaseUrl(const std::string& baseUrl) { m_baseUrl = baseUrl; }

// Generate implementation (Overrides base class)
std::string GroqClient::generate(const std::string& prompt) {
     if (m_apiKey.empty()) {
        throw ApiError("Groq API key is not set.");
    }
    // Groq uses the chat completions endpoint
    std::string url = m_baseUrl + "/chat/completions";

    // Build JSON payload specific to Groq/OpenAI Chat Completions API
    Json::Value root;
    Json::Value message;
    Json::Value messagesArray(Json::arrayValue); // Array for messages

    // Simple structure: only one user message
    message["role"] = "user";
    message["content"] = prompt;
    messagesArray.append(message);

    root["messages"] = messagesArray;
    root["model"] = m_model;
    root["temperature"] = m_temperature;
    root["max_tokens"] = m_maxTokens;
    // Add other Groq/OpenAI-specific parameters if needed (e.g., top_p, stream)

    Json::StreamWriterBuilder writerBuilder;
    writerBuilder["indentation"] = ""; // Compact output
    std::string payload = Json::writeString(writerBuilder, root);

    // std::cout << "[DEBUG] GroqClient Request URL: " << url << std::endl;
    // std::cout << "[DEBUG] GroqClient Request Payload: " << payload << std::endl;

    try {
        std::string responseBody = performHttpRequest(url, payload);
        // std::cout << "[DEBUG] GroqClient Response Body: " << responseBody << std::endl;
        return parseJsonResponse(responseBody);
    } catch (const ApiError& e) {
        std::cerr << "[ERROR] GroqClient API Error: " << e.what() << std::endl;
        throw; // Re-throw API specific errors
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] GroqClient Request Failed: " << e.what() << std::endl;
        throw ApiError(std::string("Groq request failed: ") + e.what()); // Wrap other errors
    } catch (...) {
         std::cerr << "[ERROR] GroqClient Unknown request failure." << std::endl;
         throw ApiError("Unknown error during Groq request.");
    }
}


// HTTP request helper (Note the Authorization header)
std::string GroqClient::performHttpRequest(const std::string& url, const std::string& payload) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to initialize libcurl");

    std::string readBuffer;
    long http_code = 0;
    struct curl_slist* headers = nullptr;

    // Groq/OpenAI typically uses Bearer token authorization
    std::string authHeader = "Authorization: Bearer " + m_apiKey;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::unique_ptr<struct curl_slist, decltype(&curl_slist_free_all)> header_list(headers, curl_slist_free_all);


    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload.length());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list.get());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

    CURLcode res = curl_easy_perform(curl);
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl_handle(curl, curl_easy_cleanup);

    if (res != CURLE_OK) {
        throw ApiError("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code < 200 || http_code >= 300) {
        std::ostringstream errMsg;
        errMsg << "HTTP Error: " << http_code;
        errMsg << " | Response: " << readBuffer.substr(0, 500);
        if (readBuffer.length() > 500) errMsg << "...";
        throw ApiError(errMsg.str());
    }

    return readBuffer;
}

// JSON parsing helper (Adapted for Groq/OpenAI Chat Completions response)
std::string GroqClient::parseJsonResponse(const std::string& jsonResponse) const {
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
    std::string errors;

    bool parsingSuccessful = reader->parse(jsonResponse.c_str(), jsonResponse.c_str() + jsonResponse.length(), &root, &errors);

    if (!parsingSuccessful) {
        throw ApiError("Failed to parse Groq JSON response: " + errors);
    }

    // Check for OpenAI-style error structure first
    if (root.isMember("error") && root["error"].isObject()) {
         std::string errorMsg = "API Error: ";
         if (root["error"].isMember("message") && root["error"]["message"].isString()) {
            errorMsg += root["error"]["message"].asString();
         } else if (root["error"].isMember("type") && root["error"]["type"].isString()){
             errorMsg += root["error"]["type"].asString(); // Include error type if message is missing
         } else {
             errorMsg += Json::writeString(Json::StreamWriterBuilder(), root["error"]);
         }
         throw ApiError(errorMsg);
    }
     // Also handle top-level 'detail' which Groq sometimes uses for errors
     if (root.isMember("detail") && root["detail"].isString()){
        throw ApiError("API Error Detail: " + root["detail"].asString());
     }


    // Navigate the expected Groq/OpenAI success structure
    try {
        // Structure: root -> choices[0] -> message -> content
        if (root.isMember("choices") && root["choices"].isArray() && !root["choices"].empty()) {
            const Json::Value& firstChoice = root["choices"][0u];
            if (firstChoice.isMember("message") && firstChoice["message"].isObject()) {
                const Json::Value& message = firstChoice["message"];
                if (message.isMember("content") && message["content"].isString()) {
                    return message["content"].asString();
                }
            }
             // Check finish reason if content is missing/null
             if (firstChoice.isMember("finish_reason") && firstChoice["finish_reason"].asString() != "stop") {
                 throw ApiError("Content generation finished unexpectedly. Reason: " + firstChoice["finish_reason"].asString());
             }
        }
        // If structure wasn't as expected
         throw ApiError("Could not extract content from Groq API response structure. Response: " + jsonResponse.substr(0, 500) + "...");

    } catch (const Json::Exception& e) {
        throw ApiError(std::string("JSON access error while parsing Groq response: ") + e.what());
     } catch (const ApiError& e) {
        throw;
    } catch (const std::exception& e) {
         throw ApiError(std::string("Standard exception while parsing Groq response: ") + e.what());
    }
}
```

### File: src/MiniGemini.cpp
```cpp
#include "../inc/MiniGemini.hpp"
#include <curl/curl.h>
#include <json/json.h>
#include <sstream>
#include <cstdlib> // For getenv
#include <stdexcept>
#include <iostream> // For potential debug logging

// Libcurl write callback (Implementation remains the same)
size_t MiniGemini::writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Constructor implementation
MiniGemini::MiniGemini(const std::string& apiKey) :
    // Sensible defaults for Gemini
    m_model("gemini-2.0-flash"), // Use a common, stable model
    // m_model("gemini-2.5-pro-exp-03-25"),
    m_temperature(0.5), // Adjusted default temperature
    m_maxTokens(4096),
    m_baseUrl("https://generativelanguage.googleapis.com/v1beta/models")
{
    if (!apiKey.empty()) {
        m_apiKey = apiKey;
    } else {
        const char* envKey = std::getenv("GEMINI_API_KEY");
        if (envKey != nullptr && envKey[0] != '\0') {
            m_apiKey = envKey;
            std::cout << "[INFO] GeminiClient: Using API key from GEMINI_API_KEY environment variable." << std::endl;
        } else {
            std::cerr << "[WARN] GeminiClient: API key not provided via constructor or GEMINI_API_KEY env var. API calls will likely fail." << std::endl;
            // Consider throwing here if the key is absolutely mandatory for initialization
            // throw std::invalid_argument("Gemini API key required: Provide via constructor or GEMINI_API_KEY env var");
        }
    }
     // Note: curl_global_init should ideally be called once in main()
}

// Configuration Setters
void MiniGemini::setApiKey(const std::string& apiKey) { m_apiKey = apiKey; }
void MiniGemini::setModel(const std::string& model) { m_model = model; }
void MiniGemini::setTemperature(double temperature) { m_temperature = temperature; }
void MiniGemini::setMaxTokens(int maxTokens) { m_maxTokens = maxTokens; }
void MiniGemini::setBaseUrl(const std::string& baseUrl) { m_baseUrl = baseUrl; }


// Generate implementation (Overrides base class)
std::string MiniGemini::generate(const std::string& prompt) {
     if (m_apiKey.empty()) {
        throw ApiError("Gemini API key is not set.");
    }
    std::string url = m_baseUrl + "/" + m_model + ":generateContent?key=" + m_apiKey;

    // Build JSON payload specific to Gemini API
    Json::Value root;
    Json::Value content;
    Json::Value part;
    Json::Value genConfig;

    part["text"] = prompt;
    content["parts"].append(part);
    // Gemini API structure often uses a 'contents' array
    root["contents"].append(content);

    // Add generation config
    genConfig["temperature"] = m_temperature;
    genConfig["maxOutputTokens"] = m_maxTokens;
    // Add other Gemini-specific configs if needed (e.g., stop sequences, top_k, top_p)
    root["generationConfig"] = genConfig;

    Json::StreamWriterBuilder writerBuilder; // Use StreamWriterBuilder for more control
    writerBuilder["indentation"] = ""; // Use FastWriter equivalent for compact output
    std::string payload = Json::writeString(writerBuilder, root);

    // std::cout << "[DEBUG] GeminiClient Request URL: " << url << std::endl;
    // std::cout << "[DEBUG] GeminiClient Request Payload: " << payload << std::endl;

    try {
        std::string responseBody = performHttpRequest(url, payload);
        // std::cout << "[DEBUG] GeminiClient Response Body: " << responseBody << std::endl;
        return parseJsonResponse(responseBody);
    } catch (const ApiError& e) {
        std::cerr << "[ERROR] GeminiClient API Error: " << e.what() << std::endl;
        throw; // Re-throw API specific errors
    } catch (const std::exception& e) {
         std::cerr << "[ERROR] GeminiClient Request Failed: " << e.what() << std::endl;
        throw ApiError(std::string("Gemini request failed: ") + e.what()); // Wrap other errors
    } catch (...) {
        std::cerr << "[ERROR] GeminiClient Unknown request failure." << std::endl;
        throw ApiError("Unknown error during Gemini request.");
    }
}


// HTTP request helper (Remains mostly the same, ensure correct headers)
std::string MiniGemini::performHttpRequest(const std::string& url, const std::string& payload) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to initialize libcurl");

    std::string readBuffer;
    long http_code = 0;
    struct curl_slist* headers = nullptr;
    // Ensure correct Content-Type for Gemini
    headers = curl_slist_append(headers, "Content-Type: application/json");
    // Wrap slist in unique_ptr for RAII
    std::unique_ptr<struct curl_slist, decltype(&curl_slist_free_all)> header_list(headers, curl_slist_free_all);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload.length());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list.get());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L); // Increased timeout slightly
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L); // Disable progress meter
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""); // Allow curl to handle encoding


    CURLcode res = curl_easy_perform(curl);
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl_handle(curl, curl_easy_cleanup); // RAII cleanup


    if (res != CURLE_OK) {
        throw ApiError("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code < 200 || http_code >= 300) {
        std::ostringstream errMsg;
        errMsg << "HTTP Error: " << http_code;
        errMsg << " | Response: " << readBuffer.substr(0, 500); // Include more response context
         if (readBuffer.length() > 500) errMsg << "...";
        throw ApiError(errMsg.str());
    }

    return readBuffer;
}

// JSON parsing helper (Adapted for Gemini's typical response structure)
std::string MiniGemini::parseJsonResponse(const std::string& jsonResponse) const {
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
    std::string errors;

    bool parsingSuccessful = reader->parse(jsonResponse.c_str(), jsonResponse.c_str() + jsonResponse.length(), &root, &errors);

    if (!parsingSuccessful) {
        throw ApiError("Failed to parse Gemini JSON response: " + errors);
    }

    // Check for Gemini-specific error structure first
    if (root.isMember("error") && root["error"].isObject()) {
        std::string errorMsg = "API Error: ";
        if (root["error"].isMember("message") && root["error"]["message"].isString()) {
            errorMsg += root["error"]["message"].asString();
        } else {
             errorMsg += Json::writeString(Json::StreamWriterBuilder(), root["error"]); // Dump error object if message missing
        }
         return errorMsg; // Return error message instead of throwing? Or throw ApiError(errorMsg)? Let's throw.
         throw ApiError(errorMsg);
    }

    // Navigate the expected Gemini success structure
    try {
        // Gemini structure: root -> candidates[0] -> content -> parts[0] -> text
        if (root.isMember("candidates") && root["candidates"].isArray() && !root["candidates"].empty()) {
            const Json::Value& firstCandidate = root["candidates"][0u];
            if (firstCandidate.isMember("content") && firstCandidate["content"].isObject()) {
                 const Json::Value& content = firstCandidate["content"];
                 if (content.isMember("parts") && content["parts"].isArray() && !content["parts"].empty()) {
                     const Json::Value& firstPart = content["parts"][0u];
                     if (firstPart.isMember("text") && firstPart["text"].isString()) {
                         return firstPart["text"].asString();
                     }
                 }
            }
             // Handle cases where content might be blocked (safety ratings)
             if (firstCandidate.isMember("finishReason") && firstCandidate["finishReason"].asString() != "STOP") {
                 std::string reason = firstCandidate["finishReason"].asString();
                 std::string safetyInfo = "";
                 if (firstCandidate.isMember("safetyRatings")) {
                    safetyInfo = Json::writeString(Json::StreamWriterBuilder(), firstCandidate["safetyRatings"]);
                 }
                 throw ApiError("Content generation stopped due to safety settings or other reason: " + reason + ". Safety Ratings: " + safetyInfo);
             }
        }
        // If structure wasn't as expected
        throw ApiError("Could not extract text from Gemini API response structure. Response: " + jsonResponse.substr(0, 500) + "...");

    } catch (const Json::Exception& e) { // Catch JSON access errors
        throw ApiError(std::string("JSON access error while parsing Gemini response: ") + e.what());
    } catch (const ApiError& e) { // Re-throw our specific errors
        throw;
    } catch (const std::exception& e) { // Catch other potential errors
         throw ApiError(std::string("Standard exception while parsing Gemini response: ") + e.what());
    }
}
```

### File: src/utils/global.cpp
```cpp
#include "../../inc/Utils.hpp"
#include "../../inc/Agent.hpp" // For logMessage and LogLevel (assuming Agent.hpp defines these)

#include <cstdio>     // For popen, pclose, fgets, remove, mkstemp, perror
#include <cstdlib>    // For system, WEXITSTATUS
#include <cstring>    // For strcpy, strerror
#include <unistd.h>   // For mkstemp, fchmod, write, close, remove
#include <sys/stat.h> // For fchmod, stat, S_IRUSR, S_IWUSR, S_IXUSR
#include <sys/wait.h> // For WEXITSTATUS
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm> // For std::find for example

// Existing executeCommand from your project structure
int executeCommand(const std::string &command, std::string &output) {
  output.clear();
  std::string cmdWithStderr = command + " 2>&1"; // Consistent naming
  FILE *pipe = popen(cmdWithStderr.c_str(), "r");
  if (!pipe) {
    logMessage(LogLevel::ERROR, "popen() failed for command:", command);
    return -1;
  }
  char buffer[256];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    output += buffer;
  }
  int status = pclose(pipe);
  return WEXITSTATUS(status);
}

// Generic script execution function
std::string executeScriptTool(const std::string& scriptPathOrContent,
                              const std::string& runtime,
                              const Json::Value& params,
                              bool isContentInline) {
    std::string commandToExecute;
    std::string tempScriptFilePath; // Store path for cleanup if a temp file is made

    if (isContentInline) {
        char tempTemplate[] = "/tmp/agent_script_XXXXXX"; // mkstemp modifies this
        int fd = mkstemp(tempTemplate);
        if (fd == -1) {
            logMessage(LogLevel::ERROR, "mkstemp failed for script tool", strerror(errno));
            return "Error: Could not create temporary script file. Errno: " + std::string(strerror(errno));
        }
        tempScriptFilePath = tempTemplate;

        ssize_t written = write(fd, scriptPathOrContent.c_str(), scriptPathOrContent.length());
        if (written != (ssize_t)scriptPathOrContent.length()) {
            logMessage(LogLevel::ERROR, "Failed to write to temporary script file", tempScriptFilePath + " Errno: " + std::string(strerror(errno)));
            close(fd);
            remove(tempScriptFilePath.c_str());
            return "Error: Failed to write to temporary script file.";
        }

        // Set execute permission primarily for bash scripts if run directly
        if (runtime == "bash") {
            if (fchmod(fd, S_IRUSR | S_IWUSR | S_IXUSR) == -1) {
                logMessage(LogLevel::ERROR, "Failed to set permissions on temporary script file", tempScriptFilePath + " Errno: " + std::string(strerror(errno)));
                close(fd);
                remove(tempScriptFilePath.c_str());
                return "Error: Failed to set permissions on temporary script file.";
            }
        }
        close(fd);

        if (runtime == "bash") {
            commandToExecute = tempScriptFilePath; // Can be run directly if executable
        } else {
            commandToExecute = runtime + " " + tempScriptFilePath;
        }
    } else { // scriptPathOrContent is an actual file path
        if (scriptPathOrContent.find("..") != std::string::npos || scriptPathOrContent[0] == '/') {
             logMessage(LogLevel::ERROR, "Potentially unsafe script path provided (must be relative, no '..')", scriptPathOrContent);
             return "Error: Script path must be relative and not contain '..'.";
        }
        std::string safeScriptPath = scriptPathOrContent;
        if (safeScriptPath.rfind("./", 0) != 0) { // Ensure it starts with ./ if relative
            safeScriptPath = "./" + safeScriptPath;
        }

        if (runtime == "bash") {
            // Check if script is already executable, otherwise prepend bash
            struct stat st;
            if (stat(safeScriptPath.c_str(), &st) == 0 && (st.st_mode & S_IXUSR)) {
                commandToExecute = safeScriptPath;
            } else {
                commandToExecute = "bash " + safeScriptPath;
            }
        } else {
            commandToExecute = runtime + " " + safeScriptPath;
        }
    }

    // Parameter passing (simplified for this B-line approach)
    std::string argsString;
    if (params.isObject() && !params.empty()) {
        // For Python, pass the whole JSON object as a single string argument.
        // For Bash, could pass individual values or also JSON string. Let's stick to JSON string for consistency.
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = ""; // Compact JSON
        std::string paramsJson = Json::writeString(writerBuilder, params);
        
        // Basic shell escaping for the JSON string argument
        std::string escapedParamsJson = "'"; // Use single quotes for the outer shell argument
        for (char c : paramsJson) {
            if (c == '\'') {
                escapedParamsJson += "'\\''"; // Replaces ' with '\'', ends current ', adds \', starts new '
            } else {
                escapedParamsJson += c;
            }
        }
        escapedParamsJson += "'";
        argsString = " " + escapedParamsJson;
    }
    commandToExecute += argsString;

    std::string output;
    logMessage(LogLevel::DEBUG, "[ScriptTool] Executing final command", commandToExecute);

    int status = ::executeCommand(commandToExecute, output); // Global ::executeCommand

    if (!tempScriptFilePath.empty()) {
        if (remove(tempScriptFilePath.c_str()) != 0) {
            logMessage(LogLevel::WARN, "Failed to remove temporary script file", tempScriptFilePath + " Errno: " + std::string(strerror(errno)));
        }
    }

    if (status != 0) {
        logMessage(LogLevel::ERROR, "[ScriptTool] Execution failed (status " + std::to_string(status) + ") for command", commandToExecute);
        logMessage(LogLevel::ERROR, "[ScriptTool] Script Output (stderr may be included)", output);
        return "Error executing script (status " + std::to_string(status) + "):\n" + output;
    }
    return output;
}
```

### File: TODO.md
```markdown
### TO-DO:
- [ ] Implement more sophisticated error handling.
- [ ] Add more tools.
- [ ] Improve the agent's decision-making process.
- [ ] Implement a more robust memory management system.
- [ ] Data persistence for the profile export. could be a simple .agent-name.profile.json so we can load it up as well.
    - [ ] some sort of processing for these files. they might be used by the preferential/habitual generators agents.
```

### File: tools/bash.cpp
```cpp
#include <cstdio>
#include <string>
#include <sstream>
#include <stdexcept>
#include <array>
#include <iostream>
#include "json/json.h"
#include "../inc/Tool.hpp" // Include your Tool.hpp


std::string executeBashCommandReal(const Json::Value& params) {
    // 1. Parameter Validation
    if (params == Json::nullValue || params.empty()) {
        return "Error: No parameters provided. Please provide a command parameter.";
    }
    if (!params.isMember("command")) {
        return "Error: Missing required parameter 'command'.";
    }
    if (!params["command"].isString()) {
        return "Error: Parameter 'command' must be a string.";
    }

    std::string command = params["command"].asString();
    if (command.empty()) {
        return "Error: 'command' parameter cannot be empty.";
    }

    std::cout << "Executing command: " << command << std::endl;

    // --- Command Execution Logic ---
    std::string full_command = command + " 2>&1"; // Redirect stderr to stdout
    std::array<char, 128> buffer;
    std::string result; // Use string directly for efficiency
    FILE* pipe = nullptr;

    // Use popen to execute the command and open a pipe to read its output
    pipe = popen(full_command.c_str(), "r");
    if (!pipe) {
        return "Error: popen() failed to execute command: " + command;
    }

    // Read the output from the pipe line by line
    try {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data(); // Append directly to the string
        }
    } catch (const std::exception& e) { // Catch specific exceptions
        pclose(pipe);
        return "Error: Exception caught while reading command output: " + std::string(e.what());
    } catch (...) { // Catch any other exceptions
        pclose(pipe);
        return "Error: Unknown exception caught while reading command output.";
    }


    int exit_status = pclose(pipe);


    result += "\n--- Exit Status: " + std::to_string(WEXITSTATUS(exit_status)) + " ---";

    return result;
}
```

### File: tools/file_reader.cpp
```cpp
#include "inc/Tool.hpp"
#include <fstream>
#include <string>

std::string readFile(const Json::Value& params) {
  if (!params.isMember("filepath") || !params["filepath"].isString()) {
    return "Error: 'filepath' parameter is missing or not a string.";
  }
  std::string filepath = params["filepath"].asString();
  std::ifstream file(filepath);
  if (file.is_open()) {
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    return content;
  } else {
    return "Error: Could not open file.";
  }
}

int main(){
    return 0;
}
EOF 2>&1
```

### File: voice-client/app.js
```javascript
// --- app_keyword_v2.js ---

document.addEventListener('DOMContentLoaded', () => {
    // --- Configuration ---
    const API_URL = 'https://agent.clevo.ddnsgeek.com/prompt'; // YOUR endpoint
    const REQUEST_TIMEOUT_MS = 30000;
    const KEYWORD = "agent"; // YOUR keyword (lowercase)
    // --------------------

    // --- DOM Elements ---
    const chatContainer = document.getElementById('chat-container');
    const userInput = document.getElementById('userInput'); // Used for visual feedback of transcription
    const sendButton = document.getElementById('sendButton'); // Likely remains disabled
    const statusDiv = document.getElementById('status'); // For processing/error status
    const keywordStatusDiv = document.getElementById('keywordStatus');
    const keywordButton = document.getElementById('keywordButton');

    // --- State Management ---
    const STATE = {
        IDLE: 'IDLE',
        LISTENING: 'LISTENING', // Actively listening for keyword OR command
        PROCESSING: 'PROCESSING', // Waiting for API response
        ERROR: 'ERROR' // An error occurred (e.g., mic access, network)
    };
    let currentState = STATE.IDLE;
    let isListenerGloballyActive = false; // User wants the listener ON
    let recognitionInstance = null;
    let finalCommandTranscript = ''; // Store the command part after keyword

    // --- Web Speech API Setup ---
    const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
    if (!SpeechRecognition) {
        console.error("Web Speech API not supported.");
        setState(STATE.ERROR, "Speech recognition not supported by this browser.");
        if (keywordButton) keywordButton.disabled = true;
        return; // Stop initialization
    }

    // --- Main Functions ---

    function initializeRecognition() {
        if (recognitionInstance) {
            // Clean up previous instance if exists
            recognitionInstance.onstart = null;
            recognitionInstance.onresult = null;
            recognitionInstance.onerror = null;
            recognitionInstance.onend = null;
            try {
                 if (currentState === STATE.LISTENING) recognitionInstance.stop();
            } catch(e) { console.warn("Error stopping previous instance:", e);}
        }

        const recognition = new SpeechRecognition();
        recognition.continuous = true; // Keep running until stopped
        recognition.interimResults = true;
        recognition.lang = 'en-US';

        recognition.onstart = () => {
            console.log("Recognition started.");
            // State change might have already happened in startListener
             if (currentState === STATE.IDLE && isListenerGloballyActive) {
                setState(STATE.LISTENING);
            }
        };

        recognition.onresult = handleRecognitionResult;

        recognition.onerror = (event) => {
            console.error('Recognition Error:', event.error, event.message);
            handleRecognitionError(event.error);
        };

        recognition.onend = () => {
            console.log("Recognition ended.");
            // Only restart automatically if the user *wants* the listener active
            // and we aren't currently processing a command or in an error state.
            if (isListenerGloballyActive && currentState !== STATE.PROCESSING && currentState !== STATE.ERROR) {
                console.log("Attempting controlled restart...");
                // Add slight delay to prevent potential browser rate-limiting or infinite loops on some errors
                setTimeout(() => {
                    if (isListenerGloballyActive && currentState !== STATE.PROCESSING && currentState !== STATE.ERROR) {
                         startRecognition(); // Attempt to start again
                    }
                }, 300); // 300ms delay
            } else {
                 // If listener is supposed to be off, or we are processing/in error, ensure IDLE state if not PROCESSING/ERROR
                 if (currentState !== STATE.PROCESSING && currentState !== STATE.ERROR) {
                    setState(STATE.IDLE);
                 }
            }
        };
        recognitionInstance = recognition; // Store the new instance
        return recognition;
    }

    function startListener() {
        if (!SpeechRecognition) {
             setState(STATE.ERROR, "Speech Recognition not supported.");
             return;
        }
        if (currentState !== STATE.IDLE && currentState !== STATE.ERROR) {
             console.warn("Listener already active or processing.");
             return;
        }
        isListenerGloballyActive = true;
        console.log("User activated listener.");
        if (startRecognition()) {
             // State will be set to LISTENING by startRecognition or onstart
        } else {
             isListenerGloballyActive = false; // Failed to start
             // Error state likely set by startRecognition failure
        }
    }

    function stopListener() {
        isListenerGloballyActive = false;
        console.log("User deactivated listener.");
        stopRecognition();
        // State will be set to IDLE by stopRecognition or onend
    }

    function startRecognition() {
         if (!recognitionInstance) {
             initializeRecognition();
         }
         if (!recognitionInstance) { // Still null after init attempt?
             setState(STATE.ERROR, "Failed to initialize Speech Recognition.");
             return false;
         }

         // Check if already running to prevent InvalidStateError
         // Note: No standard way to check if 'running', rely on state and try/catch
         if (currentState === STATE.LISTENING) {
             console.log("Recognition appears to be already listening.");
             return true;
         }

         console.log("Calling recognition.start()...");
         try {
             finalCommandTranscript = ''; // Reset command transcript
             recognitionInstance.start();
             // Assuming onstart will correctly set LISTENING state
             setState(STATE.LISTENING); // Optimistically set state here
             return true;
         } catch (e) {
             if (e.name === 'InvalidStateError') {
                 console.warn("Attempted to start recognition but it was already started.");
                 setState(STATE.LISTENING); // Ensure UI is correct
                 return true; // It's effectively running
             } else {
                 console.error("Error calling recognition.start():", e);
                 handleRecognitionError(e.name || 'start-failed');
                 return false;
             }
         }
    }

     function stopRecognition() {
        if (recognitionInstance) {
            console.log("Calling recognition.stop()...");
            try {
                 // Prevent onend restart loop by checking isListenerGloballyActive flag first
                 recognitionInstance.stop();
                 // State update will happen in onend or here if needed immediately
                 if (currentState !== STATE.PROCESSING && currentState !== STATE.ERROR) {
                    setState(STATE.IDLE); // Force IDLE if stopped manually unless processing/error
                 }
            } catch (e) {
                if (e.name === 'InvalidStateError') {
                    console.warn("Attempted to stop recognition but it was not running.");
                    if (currentState !== STATE.PROCESSING && currentState !== STATE.ERROR) {
                       setState(STATE.IDLE); // Ensure UI consistency
                    }
                } else {
                    console.error("Error calling recognition.stop():", e);
                    // Potentially set error state? Or just log?
                    if (currentState !== STATE.PROCESSING && currentState !== STATE.ERROR) {
                       setState(STATE.IDLE);
                    }
                }
            }
            // recognitionInstance = null; // Keep instance for potential restart
        } else {
             console.log("No active recognition instance to stop.");
             if (currentState !== STATE.PROCESSING && currentState !== STATE.ERROR) {
               setState(STATE.IDLE); // Ensure UI consistency
             }
        }
    }

    function handleRecognitionResult(event) {
        let interimTranscript = '';
        let currentFinalTranscript = ''; // Transcript for this specific result event

        for (let i = event.resultIndex; i < event.results.length; ++i) {
            const transcriptPart = event.results[i][0].transcript;
            if (event.results[i].isFinal) {
                currentFinalTranscript += transcriptPart;
            } else {
                interimTranscript += transcriptPart;
            }
        }

        // Update UI feedback field
        userInput.value = (finalCommandTranscript + currentFinalTranscript + interimTranscript).trim();

        const processedInterim = interimTranscript.toLowerCase().trim();
        const processedFinal = currentFinalTranscript.toLowerCase().trim();

        // Keyword spotting in interim results (more responsive)
        if (currentState === STATE.LISTENING && processedInterim.includes(KEYWORD)) {
            console.log("Keyword detected in interim results.");
            // Don't change major state yet, wait for final or more input
            // Optionally provide visual cue that keyword was heard
             setKeywordStatus(`Status: Keyword "${KEYWORD}" heard, listening for command...`);
        }

        // Process final results
        if (currentFinalTranscript) {
            console.log("Final segment received:", processedFinal);
            finalCommandTranscript += processedFinal + " "; // Append final parts

            const keywordIndex = finalCommandTranscript.toLowerCase().indexOf(KEYWORD);

            if (keywordIndex !== -1) {
                const command = finalCommandTranscript.substring(keywordIndex + KEYWORD.length).trim();
                if (command) {
                    console.log("Command extracted after keyword:", command);
                    isListenerGloballyActive = false; // Stop listening automatically after command
                    stopRecognition();
                    setState(STATE.PROCESSING);
                    addMessageToChat('user', command);
                    sendMessageToAgent(command);
                    finalCommandTranscript = ''; // Reset for next time
                } else {
                    console.log("Keyword found in final transcript, but no command followed yet.");
                    // Keep listening for more final results if recognition.continuous=true allows
                }
            } else {
                 // Final segment doesn't contain keyword, reset command transcript if needed
                 console.log("Final segment without keyword detected.");
                 // Reset if it seems like a new utterance unrelated to keyword? Heuristic needed.
                 // For now, just keep appending, assuming keyword might come later in continuous stream.
                 // A more robust approach might reset finalCommandTranscript on longer pauses.
                 // For simplicity with continuous=true, we primarily rely on keyword detection to trigger action.
                 // If continuous causes issues, switch back to continuous=false and handle logic in onend.
            }
        }
    }

    function handleRecognitionError(errorCode) {
        let userMessage = `Speech Error: ${errorCode}`;
        switch (errorCode) {
            case 'no-speech': userMessage = "No speech detected. Please try again."; break;
            case 'audio-capture': userMessage = "Microphone error. Check connection/settings."; break;
            case 'not-allowed': userMessage = "Microphone access denied. Please allow in browser settings."; break;
            case 'service-not-allowed': userMessage = "Microphone access blocked (browser/OS setting?)."; break;
            case 'network': userMessage = "Network error during speech recognition."; break;
            case 'aborted': userMessage = "Speech input aborted."; break; // User likely stopped it
            case 'language-not-supported': userMessage = "Language not supported."; break;
            case 'bad-grammar': userMessage = "Grammar error in speech service."; break; // Less common
            case 'start-failed': userMessage = "Could not start microphone. Check permissions."; break;
            default: userMessage = `An unknown speech error occurred (${errorCode}).`; break;
        }
        setState(STATE.ERROR, userMessage); // Set error state and message
    }

    // --- API Interaction ---
    async function sendMessageToAgent(promptText) {
        const controller = new AbortController();
        const timeoutId = setTimeout(() => {
             controller.abort();
             console.log("Request aborted due to timeout.");
             }, REQUEST_TIMEOUT_MS);

        try {
            console.log(`Sending request to: ${API_URL}`);
            const response = await fetch(API_URL, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json', 'Accept': 'application/json' },
                body: JSON.stringify({ prompt: promptText }),
                signal: controller.signal
            });

            clearTimeout(timeoutId); // Clear timeout if fetch completes

            if (!response.ok) {
                 let errorMsg = `Error: ${response.status} ${response.statusText}`;
                 try { const errorData = await response.json(); errorMsg = `Agent API Error: ${errorData.error || response.statusText}${errorData.details ? ' | ' + errorData.details : ''}`; }
                 catch (e) { console.warn("Error response not JSON"); try { const textError = await response.text(); errorMsg += ` | Body: ${textError.substring(0,100)}`;} catch(e2){} }
                 throw new Error(errorMsg);
            }

            const data = await response.json();
            if (data && typeof data.response === 'string') {
                addMessageToChat('agent', data.response);
                 setState(STATE.LISTENING); // Go back to listening state after successful processing
                 if(isListenerGloballyActive) startRecognition(); // Try restarting if user still wants it on
            } else {
                throw new Error("Unexpected response format from agent.");
            }

        } catch (error) {
            clearTimeout(timeoutId); // Clear timeout on error too
            let errorMsg = `Failed to get response: ${error.message}`;
             if (error.name === 'AbortError') {
                 errorMsg = `Request timed out (${REQUEST_TIMEOUT_MS / 1000}s).`;
             } else if (error instanceof TypeError && error.message.includes('fetch')) {
                 errorMsg = `Network/CORS/HTTPS Error. Check URL, server status, and CORS headers.`;
             }
            console.error('sendMessage Error:', error);
            setState(STATE.ERROR, errorMsg); // Set error state
            addMessageToChat('system', errorMsg, true);
            // Don't automatically restart listener after error state
            isListenerGloballyActive = false; // Assume user needs to restart manually
        }
    }

    // --- UI Update Functions ---
    function setState(newState, statusMessage = '') {
         if (currentState === newState && !statusMessage) return;

         console.log(`Setting State: ${newState}, Message: "${statusMessage}"`);
         currentState = newState;
         let keywordStatusText = '';
         let mainStatusText = '';
         let kwButtonText = '';
         let kwButtonClassRemove = ['listening', 'recording'];
         let kwButtonClassAdd = [];
         let kwButtonDisabled = false;
         let inputDisabled = true;
         let sendDisabled = true;

         switch (newState) {
            case STATE.IDLE:
                keywordStatusText = 'Status: Idle. Press "Start Listening"';
                kwButtonText = "Start Listening";
                kwButtonDisabled = !SpeechRecognition;
                inputDisabled = false; // Allow typing when idle
                sendDisabled = false;
                break;
            case STATE.LISTENING:
                keywordStatusText = `Status: Listening for "${KEYWORD}"...`;
                kwButtonText = `Stop Listening ("${KEYWORD}")`;
                kwButtonClassAdd.push('listening');
                kwButtonDisabled = false;
                inputDisabled = true; // Disable typing while listening
                sendDisabled = true;
                break;
             // Note: No explicit RECORDING_COMMAND state needed for UI update logic,
             // rely on interim transcript updates and the final command check.
             // LISTENING state covers both visually.
            case STATE.PROCESSING:
                keywordStatusText = 'Status: Processing command...';
                mainStatusText = 'Agent is thinking...';
                kwButtonText = "Processing...";
                kwButtonDisabled = true;
                inputDisabled = true;
                sendDisabled = true;
                break;
            case STATE.ERROR:
                keywordStatusText = `Status: Error - ${statusMessage}`;
                mainStatusText = `Error: ${statusMessage}`; // Show error in main status too
                kwButtonText = "Start Listening"; // Allow restarting after error
                kwButtonDisabled = !SpeechRecognition; // Only disable if API not supported
                inputDisabled = false; // Allow typing after error
                sendDisabled = false;
                isListenerGloballyActive = false; // Force listener off on error
                stopRecognition(); // Ensure recognition is stopped
                break;
         }

         if (keywordStatusDiv) keywordStatusDiv.textContent = keywordStatusText;
         if (statusDiv) statusDiv.textContent = mainStatusText;
         if (keywordButton) {
             keywordButton.textContent = kwButtonText;
             keywordButton.disabled = kwButtonDisabled;
             kwButtonClassRemove.forEach(cls => keywordButton.classList.remove(cls));
             kwButtonClassAdd.forEach(cls => keywordButton.classList.add(cls));
         }
         if (userInput) {
             userInput.disabled = inputDisabled;
             userInput.placeholder = inputDisabled ? 'Listening for keyword...' : 'Type message or use listener';
         }
         if (sendButton) sendButton.disabled = sendDisabled;
    }

    function addMessageToChat(sender, messageText, isError = false) {
        if (!chatContainer) return;
        const messageDiv = document.createElement('div');
        messageDiv.classList.add('message');
        const typeClass = isError ? 'system-error-message' : `${sender}-message`;
        messageDiv.classList.add(typeClass);
        messageDiv.textContent = messageText.replace(/</g, "<").replace(/>/g, ">"); // Basic sanitize
        chatContainer.appendChild(messageDiv);
        scrollToBottom();
    }

    function scrollToBottom() {
        setTimeout(() => { chatContainer.scrollTop = chatContainer.scrollHeight; }, 50);
    }

    // --- Event Listeners ---
    keywordButton.addEventListener('click', () => {
        if (isListenerGloballyActive) {
            stopListener();
        } else {
            startListener();
        }
    });

    sendButton.addEventListener('click', () => {
         const text = userInput.value.trim();
         if (text && currentState !== STATE.PROCESSING && currentState !== STATE.LISTENING) {
             setState(STATE.PROCESSING);
             addMessageToChat('user', text);
             sendMessageToAgent(text);
             userInput.value = '';
         }
    });

     userInput.addEventListener('keypress', (event) => {
        if (event.key === 'Enter' && !sendButton.disabled && currentState !== STATE.LISTENING) {
            event.preventDefault();
            const text = userInput.value.trim();
             if (text) {
                 setState(STATE.PROCESSING);
                 addMessageToChat('user', text);
                 sendMessageToAgent(text);
                 userInput.value = '';
             }
        }
    });


    // --- Initial Page Load ---
    setState(STATE.IDLE); // Set initial state
    userInput.focus();
    console.log(`Keyword Listener V2 Initialized. Keyword: "${KEYWORD}", API: ${API_URL}`);

});
```

### File: voice-client/index.html
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Agent Voice (Keyword Mode)</title>
    <!-- Link to your existing style.css or add new styles -->
    <link rel="stylesheet" href="style.css">
    <style>
        /* Add styles specific to keyword mode */
        #keywordStatus {
            padding: 8px 15px;
            font-size: 0.9em;
            color: var(--secondary-text);
            min-height: 1.5em;
            text-align: center;
            background-color: #eee; /* Slightly different background */
            border-bottom: 1px solid var(--border-color);
        }
        #keywordButton {
             background-color: #198754; /* Green */
             margin-right: 10px; /* Spacing */
             width: auto; /* Allow width to adjust to text */
             padding: 10px 15px;
             border-radius: 20px;
             font-size: 0.9em;
        }
        #keywordButton:hover:not(:disabled) { background-color: #157347; }
        #keywordButton.listening { background-color: #ffc107; color: #333; } /* Yellow when listening */
        #keywordButton.recording { background-color: #dc3545; } /* Red when recording */
    </style>
</head>
<body>

    <header>
        <h1>C++ Agent Mk2 - Keyword Mode</h1>
    </header>

    <div id="keywordStatus">Status: Idle. Press "Start Listening"</div>

    <div id="chat-container">
        <!-- Messages appear here -->
    </div>

    <div id="status"></div> <!-- For thinking/error messages -->

    <div id="input-area">
        <!-- Keyword listener button -->
        <button id="keywordButton" class="chat-button">Start Listening</button>
        <!-- Text input and send button (can be hidden/disabled in keyword mode) -->
        <input type="text" id="userInput" placeholder="Keyword mode active..." autocomplete="off" disabled>
        <button id="sendButton" class="chat-button" title="Send Message" disabled>Send</button>
         <!-- Mic button (from previous example) can be hidden or repurposed -->
         <!-- <button id="micButton" class="chat-button hidden" title="Start/Stop Recording">🎤</button> -->
    </div>

    <!-- Link to NEW JavaScript file -->
    <script src="app.js"></script>

</body>
</html>
```

### File: voice-client.py
```python
#!/usr/bin/env python3
import requests
import json
import argparse
import sys
import os # For model path check

# Attempt to import voice libraries, provide guidance if missing
try:
    import speech_recognition as sr
    vosk_available = True # Defined globally here
except ImportError:
    print("INFO: 'SpeechRecognition' library not found. Voice input disabled.", file=sys.stderr)
    print("INFO: To enable voice input, run: pip install SpeechRecognition vosk", file=sys.stderr)
    vosk_available = False # Or defined globally here
except Exception as e:
    print(f"ERROR: Unexpected error importing speech_recognition: {e}", file=sys.stderr)
    vosk_available = False # Or defined globally here

# --- Configuration ---
DEFAULT_SERVER_URL = "http://localhost:7777"
PROMPT_ENDPOINT = "/prompt"
REQUEST_TIMEOUT = 60 # Increased timeout slightly for potentially longer agent processing
DEFAULT_VOSK_MODEL_PATH = "vosk-model-small-en-us-0.15" # EXAMPLE path - USER MUST PROVIDE!

# --- Existing Network Function ---
def send_prompt_to_agent(server_url: str, user_prompt: str) -> str:
    """Sends the user prompt to the agent API server and returns the response text."""
    api_endpoint = server_url.rstrip('/') + PROMPT_ENDPOINT
    payload = {"prompt": user_prompt}
    headers = {"Content-Type": "application/json"}

    try:
        response = requests.post(
            api_endpoint,
            json=payload,
            headers=headers,
            timeout=REQUEST_TIMEOUT
        )
        response.raise_for_status()
        try:
            response_json = response.json()
        except json.JSONDecodeError:
            raise ValueError(f"Server returned non-JSON response: {response.text[:100]}...")

        if "response" in response_json and isinstance(response_json["response"], str):
            return response_json["response"]
        elif "error" in response_json:
             error_details = response_json.get("details", "")
             return f"Agent API Error: {response_json['error']} {f'({error_details})' if error_details else ''}"
        else:
            raise ValueError(f"Unexpected JSON structure in response: {response_json}")

    except requests.exceptions.ConnectionError:
        return f"[Error]: Could not connect to the agent server at {api_endpoint}. Is it running?"
    except requests.exceptions.Timeout:
        return f"[Error]: Request timed out connecting to {api_endpoint}."
    except requests.exceptions.RequestException as e:
        status_code = e.response.status_code if e.response is not None else "N/A"
        error_body = e.response.text[:200] if e.response is not None else "No Response Body"
        return f"[Error]: HTTP Error {status_code} from server: {error_body}..."
    except ValueError as e:
        return f"[Error]: Invalid response from server: {e}"
    except Exception as e:
        return f"[Error]: An unexpected error occurred: {e}"

# --- NEW Voice Handling Function ---
def handle_voice_input(recognizer, microphone, model_path) -> str | None:
    """Captures audio, transcribes using Vosk, returns text or None on error."""
    global vosk_available # Need global here too if modifying it on error
    if not vosk_available:
        print("[Error]: Voice libraries not available.", file=sys.stderr)
        return None
    if not os.path.isdir(model_path):
         print(f"[Error]: Vosk model not found at path: {model_path}", file=sys.stderr)
         print(f"INFO: Download a model (e.g., small English) from https://alphacephei.com/vosk/models", file=sys.stderr)
         print(f"INFO: And place it at '{model_path}' or specify path with --vosk-model.", file=sys.stderr)
         return None

    try:
        with microphone as source:
            print("Adjusting for ambient noise...")
            try:
                 # Duration helps prevent cutting off noise adjustment too soon
                 recognizer.adjust_for_ambient_noise(source, duration=0.5)
            except Exception as e:
                 print(f"Warning: Failed ambient noise adjustment: {e}. Continuing...", file=sys.stderr)

            print("Listening... (Speak clearly and pause when finished)")
            try:
                # Listen until a pause is detected
                # timeout: max seconds of silence before considering phrase complete
                # phrase_time_limit: max seconds to record before stopping
                audio = recognizer.listen(source, timeout=5, phrase_time_limit=15)
            except sr.WaitTimeoutError:
                print("No speech detected within timeout.", file=sys.stderr)
                return None
            except Exception as e:
                 print(f"Error during listening: {e}", file=sys.stderr)
                 return None

        print("Transcribing...")
        try:
            # Use recognize_vosk for local transcription
            # *** IMPORTANT: Provide the model_path to recognize_vosk ***
            text_result = json.loads(recognizer.recognize_vosk(audio, model_path=model_path))
            text = text_result.get("text", "") # Extract text field from Vosk JSON result
            if not text:
                print("Vosk transcription returned empty text.", file=sys.stderr)
                return None
            print(f"Heard: '{text}'")
            return text
        except sr.UnknownValueError:
            print("Vosk could not understand audio", file=sys.stderr)
            return None
        except sr.RequestError as e:
            # This might indicate issues loading the model or with the Vosk installation
            print(f"Vosk error: {e}", file=sys.stderr)
            return None
        except json.JSONDecodeError:
            print("Error: Vosk returned non-JSON result.", file=sys.stderr)
            return None
        except Exception as e:
            # Catch other potential errors during transcription
            print(f"An unexpected error occurred during transcription: {e}", file=sys.stderr)
            return None

    except AttributeError:
        print("[Error]: SpeechRecognition or Microphone object not initialized correctly.", file=sys.stderr)
        return None
    except OSError as e:
         print(f"[Error]: Microphone OS error: {e}. Is a microphone connected and configured?", file=sys.stderr)
         vosk_available = False # Modify global if mic fails persistently
         return None
    except Exception as e:
        print(f"[Error]: An unexpected error occurred in voice handling: {e}", file=sys.stderr)
        return None


# --- Modified Main Loop ---
def main_loop(server_url: str, vosk_model_path: str):
    """Runs the main interactive command-line loop with voice option."""
    # *** ADD global DECLARATION HERE ***
    global vosk_available

    print(f"Connecting to Agent API at: {server_url}")
    print("Type your prompt and press Enter.")
    if vosk_available: # Reads the global variable
        print("Type '/v' or '/voice' then Enter to use voice input.")
    print("Type 'exit' or 'quit' to quit.")

    recognizer = None
    microphone = None
    if vosk_available: # Reads the global variable
        try:
            recognizer = sr.Recognizer()
            # Optional: Adjust energy threshold if needed
            # recognizer.energy_threshold = 4000
            microphone = sr.Microphone()
            print("Microphone initialized.") # Add confirmation
        except AttributeError:
             print("\n[WARN]: Failed to initialize SpeechRecognition/Microphone. Voice input unavailable.", file=sys.stderr)
             vosk_available = False # Modifies the global variable
        except OSError as e:
             print(f"\n[WARN]: Failed to access Microphone ({e}). Voice input unavailable.", file=sys.stderr)
             vosk_available = False # Modifies the global variable
        except Exception as e:
             print(f"\n[WARN]: Unknown error initializing voice components ({e}). Voice input unavailable.", file=sys.stderr)
             vosk_available = False # Modifies the global variable


    while True:
        user_input_trigger = ""
        try:
            user_input_trigger = input("\n> ")
        except EOFError:
            print("\nExiting (EOF received).")
            break # Exit loop on Ctrl+D
        except KeyboardInterrupt:
             print("\nExiting (Interrupted by user).")
             break

        user_input_final = ""

        if user_input_trigger.lower() in ["exit", "quit"]:
            print("Exiting.")
            break
        elif user_input_trigger.lower() in ["/v", "/voice"]:
            if vosk_available and recognizer and microphone: # Reads global + checks local vars
                transcribed_text = handle_voice_input(recognizer, microphone, vosk_model_path)
                if transcribed_text:
                    user_input_final = transcribed_text
                else:
                    print("(Voice input failed or cancelled)")
                    continue # Skip sending empty prompt
            else:
                print("[Error]: Voice input is not available or not initialized.", file=sys.stderr)
                continue
        elif not user_input_trigger.strip():
             continue # Ignore empty text input
        else:
            user_input_final = user_input_trigger # Use typed text

        if not user_input_final.strip():
            continue # If voice failed and resulted in empty, skip

        # Send the final input (either text or transcribed voice)
        agent_response = send_prompt_to_agent(server_url, user_input_final)
        print(f"\nAgent: {agent_response}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="CLI Client for the C++ Agent API with Voice Input.")
    parser.add_argument(
        "--url",
        type=str,
        default=DEFAULT_SERVER_URL,
        help=f"Base URL of the agent API server (default: {DEFAULT_SERVER_URL})"
    )
    parser.add_argument(
        "--vosk-model",
        type=str,
        default=DEFAULT_VOSK_MODEL_PATH,
        help=f"Path to the Vosk language model directory (default: {DEFAULT_VOSK_MODEL_PATH})"
    )
    args = parser.parse_args()

    # Check Vosk availability again before starting main loop if voice is intended
    # This reads the global vosk_available which might have been set False during import
    if not vosk_available:
        print("\n--- VOICE INPUT DISABLED (Import Failed) ---", file=sys.stderr)
        print("Install libraries ('pip install SpeechRecognition vosk') and ensure microphone access.", file=sys.stderr)
        # Optional: Exit if voice is strictly required, but better to allow text fallback
        # sys.exit(1)


    main_loop(args.url, args.vosk_model)
```

### File: voice-client/style.css
```css
/* --- style.css for Agent Keyword Mode --- */

/* Basic Reset & Font */
:root {
    --user-bg: #dcf8c6; /* WhatsApp-like green */
    --agent-bg: #f1f0f0; /* Light grey */
    --error-bg: #f8d7da;
    --error-text: #721c24;
    --border-color: #ddd;
    --button-bg: #007bff; /* Standard blue */
    --button-hover-bg: #0056b3;
    --keyword-button-idle-bg: #28a745; /* Green for Start */
    --keyword-button-idle-hover-bg: #218838;
    --keyword-button-listening-bg: #ffc107; /* Yellow for Listening */
    --keyword-button-listening-text: #333;
    --keyword-button-recording-bg: #dc3545; /* Red for Recording */
    --keyword-button-processing-bg: #6c757d; /* Grey for Processing */
    --text-color: #333;
    --secondary-text: #666;
    --bg-color: #f4f4f4; /* Page background */
    --chat-bg: #ffffff; /* Chat area background */
    --input-focus-border: #80bdff;
    --input-focus-shadow: rgba(0, 123, 255, 0.25);
}

html { box-sizing: border-box; height: 100%; }
*, *:before, *:after { box-sizing: inherit; }

body {
    font-family: system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Oxygen, Ubuntu, Cantarell, "Open Sans", "Helvetica Neue", sans-serif;
    display: flex;
    flex-direction: column;
    height: 100vh; /* Full viewport height */
    margin: 0;
    background-color: var(--bg-color);
    color: var(--text-color);
}

header {
    background-color: var(--chat-bg);
    padding: 10px 15px;
    text-align: center;
    border-bottom: 1px solid var(--border-color);
    flex-shrink: 0; /* Prevent header from shrinking */
    box-shadow: 0 1px 3px rgba(0,0,0,0.1);
    z-index: 10; /* Keep header above chat */
}
h1 { margin: 0; font-size: 1.2em; color: var(--text-color); font-weight: 500; }

#keywordStatus {
    padding: 8px 15px;
    font-size: 0.9em;
    color: var(--secondary-text);
    min-height: 1.5em;
    text-align: center;
    background-color: #e9ecef; /* Light status background */
    border-bottom: 1px solid var(--border-color);
    flex-shrink: 0; /* Prevent shrinking */
    font-style: italic;
}

#chat-container {
    flex-grow: 1; /* Take remaining vertical space */
    padding: 15px;
    overflow-y: auto; /* Allow scrolling */
    background-color: var(--chat-bg);
    display: flex;
    flex-direction: column;
    gap: 12px; /* Space between messages */
    scroll-behavior: smooth;
}

.message {
    padding: 10px 15px;
    border-radius: 18px;
    line-height: 1.45;
    max-width: 75%; /* Max width of message bubble */
    word-wrap: break-word;
    box-shadow: 0 1px 1px rgba(0,0,0,0.08);
    font-size: 0.95em;
}
.user-message {
    background-color: var(--user-bg);
    margin-left: auto; /* Align right */
    border-bottom-right-radius: 5px; /* Slightly flatten corner */
    align-self: flex-end;
}
.agent-message {
    background-color: var(--agent-bg);
    margin-right: auto; /* Align left */
    border-bottom-left-radius: 5px; /* Slightly flatten corner */
    align-self: flex-start;
}
.system-error-message {
    background-color: var(--error-bg);
    color: var(--error-text);
    border: 1px solid #f5c6cb;
    border-radius: 8px;
    align-self: stretch; /* Take full width */
    margin: 5px auto;
    max-width: 95%;
    font-size: 0.9em;
    text-align: center;
}

#status {
    padding: 5px 15px;
    font-size: 0.85em;
    color: var(--secondary-text);
    min-height: 1.5em;
    text-align: center;
    font-style: italic;
    flex-shrink: 0;
    background-color: var(--bg-color);
}

#input-area {
    display: flex;
    padding: 10px 15px;
    border-top: 1px solid var(--border-color);
    background-color: var(--bg-color);
    flex-shrink: 0;
    gap: 10px; /* Space between elements */
    align-items: center; /* Vertically align */
}

#userInput {
    flex-grow: 1; /* Take available space */
    padding: 10px 15px;
    border: 1px solid var(--border-color);
    border-radius: 20px; /* Rounded input */
    font-size: 1em;
    background-color: var(--chat-bg);
    outline: none;
    transition: border-color 0.2s ease, box-shadow 0.2s ease;
}
#userInput:focus {
    border-color: var(--input-focus-border);
    box-shadow: 0 0 0 3px var(--input-focus-shadow);
}
#userInput:disabled {
    background-color: #e9ecef; /* Indicate disabled */
    cursor: not-allowed;
}

/* General Button Styling */
.chat-button {
    padding: 10px 15px;
    min-width: 40px;
    height: 40px;
    border: none;
    color: white;
    border-radius: 20px; /* Rounded buttons */
    cursor: pointer;
    font-size: 0.95em;
    font-weight: 500;
    transition: background-color 0.2s ease, transform 0.1s ease;
    flex-shrink: 0;
    display: flex;
    align-items: center;
    justify-content: center;
    white-space: nowrap; /* Prevent text wrapping */
}
.chat-button:active:not(:disabled) {
     transform: scale(0.95); /* Click feedback */
}
.chat-button:disabled {
    opacity: 0.6;
    cursor: not-allowed;
    transform: none;
}

/* Specific Button Styles */
#sendButton {
    background-color: var(--button-bg);
}
#sendButton:hover:not(:disabled) { background-color: var(--button-hover-bg); }

#keywordButton {
    background-color: var(--keyword-button-idle-bg);
}
#keywordButton:hover:not(:disabled) { background-color: var(--keyword-button-idle-hover-bg); }
#keywordButton.listening {
    background-color: var(--keyword-button-listening-bg);
    color: var(--keyword-button-listening-text);
}
#keywordButton.recording {
    background-color: var(--keyword-button-recording-bg);
    animation: pulse 1.5s infinite ease-in-out;
}
#keywordButton:disabled { /* Use specific color when disabled (e.g., during processing) */
     background-color: var(--keyword-button-processing-bg);
     opacity: 0.6;
}

/* Mic Pulse Animation (for recording state) */
@keyframes pulse {
    0% { box-shadow: 0 0 0 0 rgba(220, 53, 69, 0.7); }
    70% { box-shadow: 0 0 0 10px rgba(220, 53, 69, 0); }
    100% { box-shadow: 0 0 0 0 rgba(220, 53, 69, 0); }
}
```
