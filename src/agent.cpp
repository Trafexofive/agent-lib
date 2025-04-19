#include "../inc/Agent.hpp"
#include "../inc/MiniGemini.hpp"
#include "../inc/Tool.hpp" // Header-only Tool #include <algorithm>       // For std::transform
#include <cctype>  // For std::toupper
#include <ctime>   // For time functions
#include <iomanip> // For formatting time
#include <iostream>
#include <json/json.h>
#include <sstream>
#include <stdexcept>

// Simple colored logging function
void logMessage(LogLevel level, const std::string &message,
                const std::string &details = "") {
  std::time_t now = std::time(nullptr);
  std::tm *tm = std::localtime(&now);
  char time_buffer[20]; // HH:MM:SS
  std::strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", tm);

  std::string prefix;
  std::string color_start = "";
  std::string color_end = "\033[0m"; // Reset color

  // if (level == LogLevel::DEBUG)
  //   return;
  if (level == LogLevel::PROMPT)
    return;
  // level = LogLevel::INFO; // Default to INFO for now
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
    // Indent details slightly
    out << "  " << details << std::endl;
  }
}

// --- Agent Implementation ---

Agent::Agent(MiniGemini &api)
    : m_api(api), m_systemPrompt("You are a helpful assistant."), iteration(0),
      iterationCap(120), skipFlowIteration(false), _name("orchestrator") {
  // Initialize random seed (though std::rand is generally discouraged for
  // serious use)
  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  // Add the internal 'help' tool description automatically
  m_internalToolDescriptions["help"] =
      "Provides descriptions of available tools. Takes an optional 'tool_name' "
      "parameter to get help for a specific tool.";
  m_internalToolDescriptions["skip"] =
      "Skips the current workflow iteration. Almost always used when "
      "multi-steps are needed to achieve said task or when toolcall results "
      "need to be evaluated beforehand. Or if there is an issue of course. "
      "Intakes a skip (bool) and is almost always used last (after "
      "tools/workflow is done.)";
  m_internalToolDescriptions["promptAgent"] =
      "Prompts another agent with a given input. Takes 'name' (string) and "
      "'prompt' (string) parameters. It is common courtesy to tell the agent "
      "your name, and such.\n";
  m_internalToolDescriptions["summerize"] =
      "Summarizes the conversation history. Takes 'name' (string) and "
      "'prompt' (string) parameters. It is common courtesy to tell the agent "
      "your name, and such.\n";
  // m_internalToolDescriptions["generate"] =
  //     "Generates content based on the provided parameters. Takes 'path' "
  //     "(string) and 'mode' (string) parameters. It is common courtesy to tell
  //     " "the agent your name, and such.\n";
  m_internalToolDescriptions["summarizeHistory"] =
      "Summarizes the conversation history. Takes 'name' (string) and "
      "'prompt' (string) parameters. It is common courtesy to tell the agent "
      "your name, and such.\n";
  m_internalToolDescriptions["getWeather"] =
      "Fetches the current weather for a given location. Takes 'location' "
      "(string) parameter. It is common courtesy to tell the agent your name, "
      "and such.\n";

  // Add other internal tools as needed
}

void Agent::setSystemPrompt(const std::string &prompt) {
  m_systemPrompt = prompt;
}

void Agent::addTool(Tool *tool) {
  if (tool) {
    // Check for name collisions, including with internal tools
    if (m_tools.count(tool->getName()) ||
        m_internalToolDescriptions.count(tool->getName())) {
      logMessage(LogLevel::WARN,
                 "Attempted to add tool with duplicate name: '" +
                     tool->getName() + "'. Ignoring.");
    } else {
      m_tools.insert(std::make_pair(tool->getName(), tool));
      logMessage(LogLevel::INFO, "Added tool: '" + tool->getName() + "'");
    }
  } else {
    logMessage(LogLevel::WARN, "Attempted to add a null tool.");
  }
}

void Agent::addTextTool(Tool *tool) {
  if (tool) {
    // Check for name collisions, including with internal tools
    if (m_textTools.count(tool->getName()) ||
        m_internalToolDescriptions.count(tool->getName())) {
      logMessage(LogLevel::WARN,
                 "Attempted to add text tool with duplicate name: '" +
                     tool->getName() + "'. Ignoring.");
    } else {
      m_textTools.insert(std::make_pair(tool->getName(), tool));
      logMessage(LogLevel::INFO, "Added text tool: '" + tool->getName() + "'");
    }
  } else {
    logMessage(LogLevel::WARN, "Attempted to add a null text tool.");
  }
}

void Agent::reset() {
  m_history.clear();
  iteration = 0;
  skipFlowIteration = false;
  logMessage(LogLevel::INFO, "Agent reset.");
}

int executeCommand(const std::string &command, std::string &result) {
  FILE *pipe = popen(command.c_str(), "r");
  if (!pipe) {
    return -1; // Error opening pipe
  }
  char buffer[128];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    result += buffer;
  }
  return pclose(pipe);
}

std::string Agent::buildFullPrompt() const {
  std::ostringstream oss;
  oss << "System Instructions: " << m_systemPrompt << "\n";

  if (this->extraPrompts.size() > 0) {
    oss << "Extra Instructions:\n";
    for (const auto &extraPrompt : this->extraPrompts) {
      oss << extraPrompt << "\n";
    }
    oss << "\n";
  }
  // System information
  oss << "System Information:\n";
  oss << "- Agent Name: " << _name << "\n";
  // execute tree cmd
  // print available agent tools
  oss << "- Available  Agents (agents you can use; can be called using the promptAgent built-in tool): " << std::endl;
  for (const auto &pair : agents) {
    oss << "-Name: " << pair.first << " -Description: "<< pair.second->getDescription() << "\n";
  }

  std::string command = "tree";
  std::string result;

  // executing all startup scripts and automated bash commands
  if (initialCommands.size() > 0) {
    oss << "Executing Startup Commands:\n";
  }
  for (auto &command : initialCommands) {
    std::string result;
    int status = executeCommand(command, result);
    oss << "- Executed: " << command << "\n";
    if (status == -1) {
      logMessage(LogLevel::ERROR, "Failed to execute command: " + command);
    } else {
      oss << "- Command Output: " << result << "\n";
    }
  }
  // executeCommand(command, result);
  // oss << "- System Tree: \n";
  // oss << result << "\n";

  // list env vars if available
  if (this->_env.size() > 0) {
    oss << "Environment Variables:\n";
    for (const auto &pair : this->_env) {
      oss << "- " << pair.first << ": " << pair.second << "\n";
    }
    oss << "\n";
  }

  // Available Tools Section
  bool hasTools = !m_tools.empty() || !m_internalToolDescriptions.empty();
  if (hasTools) {
    oss << "Available tools:\n";
    // Internal tools first
    for (const auto &pair : m_internalToolDescriptions) {
      oss << "- " << pair.first << ": " << pair.second << "\n";
    }
    // External tools
    for (const auto &pair : m_tools) {
      oss << "- " << pair.second->getName() << ": "
          << pair.second->getDescription() << "\n";
    }
    for (const auto &pair : m_textTools) {
      oss << "- " << pair.first << ": " << pair.second->getDescription()
          << "\n";
    }
    oss << "\n";
  }

  // Conversation History Section
  if (!m_history.empty()) {
    oss << "Conversation history:\n";
    for (const auto &entry : m_history) {
      std::string role = entry.first;
      if (!role.empty()) {
        // Capitalize first letter
        role[0] = std::toupper(static_cast<unsigned char>(role[0]));
      }
      oss << role << ": " << entry.second << "\n";
    }
    oss << "\n";
  }

  // Indicate where the assistant should respond
  oss << "Assistant:"; // Let the LLM continue from here
  logMessage(LogLevel::DEBUG, "Full prompt built:", oss.str());
  return oss.str();
}

std::vector<Agent::ToolCallInfo>
Agent::extractToolCalls(const std::string &response) {
  std::vector<ToolCallInfo> calls;
  const std::string startMarker = "```tool:";
  const std::string endMarker = "```";
  std::string::size_type currentPos = 0;

  while ((currentPos = response.find(startMarker, currentPos)) !=
         std::string::npos) {
    std::string::size_type toolNameStart = currentPos + startMarker.length();
    std::string::size_type toolNameEnd = response.find('\n', toolNameStart);
    if (toolNameEnd == std::string::npos) {
      logMessage(LogLevel::WARN,
                 "Malformed tool call: Could not find newline after tool name.",
                 response.substr(currentPos, 50) + "...");
      currentPos = toolNameStart; // Move past marker to avoid infinite loop
      continue;
    }

    std::string toolName =
        response.substr(toolNameStart, toolNameEnd - toolNameStart);

    // Basic trim
    toolName.erase(0, toolName.find_first_not_of(" \t\r\n"));
    toolName.erase(toolName.find_last_not_of(" \t\r\n") + 1);

    if (toolName.empty()) {
      logMessage(LogLevel::WARN,
                 "Malformed tool call: Empty tool name detected.");
      currentPos = toolNameEnd + 1; // Move past newline
      continue;
    }

    auto textBasedTools = m_textTools;

    if (textBasedTools.find(toolName) != textBasedTools.end()) {
      // Handle text-based tool
      std::string::size_type contentStart = toolNameEnd + 1;
      std::string::size_type contentEnd =
          response.find(endMarker, contentStart);

      if (contentEnd == std::string::npos) {
        logMessage(LogLevel::WARN,
                   "Malformed text-based tool call: Could not find end marker "
                   "'```' for tool '" +
                       toolName + "'.",
                   response.substr(contentStart, 50) + "...");
        currentPos = contentStart;
        continue;
      }

      std::string rawContent =
          response.substr(contentStart, contentEnd - contentStart);

      // Trim the raw content
      rawContent.erase(0, rawContent.find_first_not_of(" \t\r\n"));
      rawContent.erase(rawContent.find_last_not_of(" \t\r\n") + 1);

      logMessage(LogLevel::DEBUG,
                 "Detected text-based tool call '" + toolName +
                     "' with content: " + rawContent.substr(0, 50) +
                     (rawContent.length() > 50 ? "..." : ""));

      ToolCallInfo info;
      info.toolName = toolName;
      info.optionalDataStr = rawContent;
      info.isTextBasedTool = true; // Mark as text-based tool
      calls.push_back(info);

      currentPos = contentEnd + endMarker.length();
      continue;
    }

    // Handle JSON-based tool (existing code)
    std::string::size_type paramsStart = toolNameEnd + 1;
    std::string::size_type paramsEnd = response.find(endMarker, paramsStart);
    if (paramsEnd == std::string::npos) {
      logMessage(
          LogLevel::WARN,
          "Malformed tool call: Could not find end marker '```' for tool '" +
              toolName + "'.",
          response.substr(paramsStart, 50) + "...");
      currentPos = paramsStart; // Move past where params should have started
      continue;
    }

    std::string paramsJsonStr =
        response.substr(paramsStart, paramsEnd - paramsStart);

    Json::Value paramsJson;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(paramsJsonStr, paramsJson);

    if (!parsingSuccessful) {
      logMessage(LogLevel::WARN,
                 "Failed to parse JSON params for tool '" + toolName + "'.",
                 "JSON String: " + paramsJsonStr +
                     "\nError: " + reader.getFormattedErrorMessages());
      // Decide how to handle: skip this call, or add with empty params? Let's
      // skip.
      currentPos = paramsEnd + endMarker.length();
      continue;
    }

    ToolCallInfo info;
    info.toolName = toolName;
    info.params = paramsJson;
    info.isTextBasedTool = false; // Mark as JSON-based tool
    calls.push_back(info);
    logMessage(LogLevel::DEBUG, "Extracted tool call",
               "Tool: " + toolName +
                   ", Params: " + paramsJson.toStyledString());

    currentPos =
        paramsEnd + endMarker.length(); // Continue search after this block
  }
  return calls;
}

// Executes a single tool call, handling both JSON-based and text-based tools
std::string Agent::handleToolExecution(const ToolCallInfo &call) {
  if (call.isTextBasedTool) {
    logMessage(LogLevel::TOOL_CALL,
               "Executing text-based tool '" + call.toolName + "'",
               "Content: " + call.optionalDataStr.substr(0, 100) +
                   (call.optionalDataStr.length() > 100 ? "..." : ""));
  } else {
    logMessage(LogLevel::TOOL_CALL,
               "Executing JSON-based tool '" + call.toolName + "'",
               "Params: " + call.params.toStyledString());
  }

  std::string result;
  try {
    if (call.toolName == "help") {
      result = this->getHelp(call.params);
    } else if (call.toolName == "skip") {
      result = skip(call.params);
    } else if (call.toolName == "promptAgent") {
      result = promptAgentTool(call.params);
    } else if (call.toolName == "summerize") {
      result = summerizeTool(call.params);
    } else if (call.toolName == "summarizeHistory") {
      result = summarizeHistory(call.params);
    } else if (call.toolName == "weather") {
      result = getWeather(call.params);
    } else if (call.toolName == "write") {
      // Handle text-based tool for writing
      auto toolIt = m_textTools.find(call.toolName);
      if (toolIt != m_textTools.end()) {
        // Call the text-based tool handler
        result = toolIt->second->execute(call.optionalDataStr);
      } else {
        logMessage(LogLevel::WARN,
                   "Attempted to execute unknown text-based tool: '" +
                       call.toolName + "'");
        result = "Error: Unknown text-based tool '" + call.toolName + "'";
      }

    } else if (call.isTextBasedTool) {
      // Handle text-based tools
      auto toolIt = m_textTools.find(call.toolName);
      if (toolIt != m_textTools.end()) {
        // Call the text-based tool handler
        result = toolIt->second->execute(call.optionalDataStr);
      } else {
        // Fallback for built-in text tools like "talk"
        if (call.toolName == "talk" || call.toolName == "message") {
          // For "talk" tool, the result could just be an acknowledgment
          // or you could implement specific behavior
          result = "[INFO] Tool '" + call.toolName + "' executed successfully.";
          logMessage(LogLevel::INFO, "Message from agent " + this->_name +
                                         ": " + call.optionalDataStr);

          // You might want to implement special handling here
          // such as sending the message somewhere, storing it, etc.
        } else {
          logMessage(LogLevel::WARN,
                     "Attempted to execute unknown text-based tool: '" +
                         call.toolName + "'");
          result = "Error: Unknown text-based tool '" + call.toolName + "'";
        }
      }
    } else {
      // Handle JSON-based tools (existing code)
      auto toolIt = m_tools.find(call.toolName);
      if (toolIt != m_tools.end()) {
        result = toolIt->second->execute(call.params);
      } else {
        logMessage(LogLevel::WARN, "Attempted to execute unknown tool: '" +
                                       call.toolName + "'");
        result = "Error: Unknown tool '" + call.toolName + "'";
      }
    }
    logMessage(LogLevel::TOOL_RESULT,
               "Tool '" + call.toolName + "' executed successfully.",
               "Result: " + result);
  } catch (const std::exception &e) {
    result = "Error executing tool '" + call.toolName + "': " + e.what();
    logMessage(LogLevel::ERROR,
               "Exception during tool execution for '" + call.toolName + "'.",
               "Error: " + std::string(e.what()));
  } catch (...) {
    result = "Error: Unknown exception occurred while executing tool '" +
             call.toolName + "'";
    logMessage(LogLevel::ERROR,
               "Unknown exception during tool execution for '" + call.toolName +
                   "'.");
  }
  return result;
}

// Processes all detected tool calls and aggregates their results.
std::string
Agent::processToolCalls(const std::vector<ToolCallInfo> &toolCalls) {
  if (toolCalls.empty()) {
    return ""; // No tools to process
  }

  std::ostringstream toolResultsOss;
  toolResultsOss
      << "Tool Results:\n"; // Start the tool results section for the LLM

  for (const auto &call : toolCalls) {
    std::string result = handleToolExecution(call);
    // Format for the LLM: Tool name and its result/error
    toolResultsOss << "Tool: " << call.toolName << "\nResult: " << result
                   << "\n";
  }
  return toolResultsOss.str();
}

// Makes a call to the underlying LLM API.
std::string Agent::executeApiCall(const std::string &fullPrompt) {
  logMessage(LogLevel::PROMPT, "Sending prompt to API:", fullPrompt);
  try {
    std::string response = m_api.generate(fullPrompt);
    logMessage(LogLevel::DEBUG, "Received API response:", response);
    return response;
  } catch (const ApiError &e) {
    logMessage(LogLevel::ERROR, "API Error occurred:", e.what());
    // Re-throw as a runtime_error or handle differently if needed
    throw std::runtime_error("API Error: " + std::string(e.what()));
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR,
               "Standard exception during API call:", e.what());
    throw std::runtime_error("Error during API call: " + std::string(e.what()));
  } catch (...) {
    logMessage(LogLevel::ERROR, "Unknown exception during API call.");
    throw std::runtime_error("Unknown error during API call");
  }
}

// Main interaction logic for a single user turn.
std::string Agent::prompt(const std::string &userInput) {
  // 1. Add user input to history
  m_history.push_back({"Master/User", userInput});
  logMessage(LogLevel::DEBUG, "User Input:", userInput);

  // --- Iteration 1: Initial API Call ---

  // 2. Build prompt based on current history and system instructions
  std::string currentPrompt = buildFullPrompt();

  // 3. Get initial response from API
  std::string assistantResponse1 = executeApiCall(currentPrompt);

  // 4. Add the raw assistant response (which might contain tool calls) to
  // history
  // m_history.push_back({this->_name, assistantResponse1});

  // 5. Check for tool calls in the response
  std::vector<ToolCallInfo> toolCalls = extractToolCalls(assistantResponse1);

  // --- Iteration 2: Tool Execution and Final Response (if needed) ---

  if (!toolCalls.empty()) {
    logMessage(LogLevel::INFO,
               "Tool calls detected in response. Executing tools...");

    // 6. Execute tools and gather results
    std::string toolResultsString = processToolCalls(toolCalls);

    // 7. Add tool results to history
    // Use a distinct role like "tool" or "system" for results
    m_history.push_back({"tool", toolResultsString});
    logMessage(LogLevel::DEBUG,
               "Added tool results to history:", toolResultsString);

    // 8. Build the prompt for the second API call (now includes tool results)
    std::string promptWithToolResults =
        buildFullPrompt(); // Rebuild with updated history

    // 8a. Handle skipFlowIteration (if applicable)
    // 8b. (Optional) Add guidance for the LLM after tool execution
    promptWithToolResults +=
        "\n[POST-TOOL-EXECUTION] Please synthesize the tool results.";

    // 9. Call API again with tool results included
    std::string finalResponse = executeApiCall(promptWithToolResults);

    std::vector<ToolCallInfo> postToolCalls = extractToolCalls(finalResponse);
    std::string toolResults = processToolCalls(postToolCalls);

    m_history.push_back({"POST-TOOL-EXECUTION", toolResults});

    if (skipFlowIteration) {
      std::string notice =
          "\n[SYSTEM] Workflow iteration skipped as per request.";
      notice += " Iteration: " + std::to_string(iteration) + "/" +
                std::to_string(iterationCap) + ".";
      notice += " Please review the tool results above and provide the final "
                "response to the user or call further tools if necessary.";
    notice +=
        "\n[IMPORTANT] If the task is not solved";
      promptWithToolResults += notice; // Append the notice for the LLM
      logMessage(LogLevel::INFO, "Skipping flow iteration as requested.");
      skipFlowIteration = false; // Reset the flag
    }
    // 10. Add final assistant response to history and return it
    m_history.push_back({this->_name, finalResponse});
    logMessage(LogLevel::DEBUG,
               "Final Assistant Response (after tools):", finalResponse);
    return finalResponse;

  } else {
    // No tool calls, the first response is the final one for this turn
    return assistantResponse1;
  }
}

// Provides help/descriptions for available tools.
std::string Agent::getHelp(const Json::Value &params) {
  std::ostringstream helpOss;
  std::string specificTool;

  if (params.isMember("tool_name") && params["tool_name"].isString()) {
    specificTool = params["tool_name"].asString();
  }

  if (!specificTool.empty()) {
    // Help for a specific tool
    helpOss << "Help for tool '" << specificTool << "':\n";
    bool found = false;
    // Check internal tools
    auto internalIt = m_internalToolDescriptions.find(specificTool);
    if (internalIt != m_internalToolDescriptions.end()) {
      helpOss << "- Description: " << internalIt->second;
      // Potentially add parameter schema info here in the future
      found = true;
    }
    // Check external tools
    auto externalIt = m_tools.find(specificTool);
    if (externalIt != m_tools.end()) {
      helpOss << "- Description: " << externalIt->second->getDescription();
      // Potentially add externalIt->second->getParameterSchema() or similar
      found = true;
    }
    auto textIt = m_textTools.find(specificTool);
    if (textIt != m_textTools.end()) {
      helpOss << "- Description: " << textIt->second->getDescription();
      // Potentially add textIt->second->getParameterSchema() or similar
      found = true;
    }

    if (!found) {
      helpOss << "Tool '" << specificTool << "' not found.";
    }
  } else {
    // General help - list all tools
    helpOss << "Available Tools:\n";
    // Internal tools first
    for (const auto &pair : m_internalToolDescriptions) {
      helpOss << "- " << pair.first << ": " << pair.second << "\n";
    }
    // External tools
    for (const auto &pair : m_tools) {
      helpOss << "- " << pair.second->getName() << ": "
              << pair.second->getDescription() << "\n";
    }
    helpOss << "\nUse help with 'tool_name' parameter for details on a "
               "specific tool (e.g., {\"tool_name\": \"calculator\"}).";
  }
  return helpOss.str();
}

// Interactive loop
void Agent::run() {
  logMessage(
      LogLevel::INFO,
      "Agent ready. Type 'exit' or 'quit' to quit, 'reset' to clear history.");
  std::string userInput;

  // temp agent arr

  while (true) {
    std::cout << "\n> ";
    if (!std::getline(std::cin, userInput)) {
      logMessage(LogLevel::INFO, "Input stream closed (EOF). Exiting.");
      break; // Handle EOF
    }
    if (userInput == "exit" || userInput == "quit") {
      logMessage(LogLevel::INFO, "Exit command received. Goodbye!");
      break;
    } else if (userInput == "reset") {
      reset(); // Already logs the reset
      continue;
    } else if (userInput.empty()) {
      continue; // Ignore empty input
    }

    try {
      std::string response = prompt(userInput);
      // The final response is already logged within prompt()
      // Optional: Print a separator or the final response here if desired
      std::cout << "\n-----------------------------------------"
                << std::endl; // Separator
    } catch (const std::exception &e) {
      // Errors from API calls or tool execution are caught here
      logMessage(LogLevel::ERROR,
                 "An error occurred during processing:", e.what());
      // Inform the user
      std::cout << "\n[Agent Error]: " << e.what() << std::endl;
      std::cout << "-----------------------------------------" << std::endl;
    }
    // Note: iteration count logic wasn't fully used in the original,
    // but you could add checks against iterationCap here or within prompt()
    // if needed for flow control.
    iteration++; // Increment iteration count per user turn
    if (iteration >= iterationCap) {
      logMessage(LogLevel::WARN, "Iteration cap reached.",
                 "Limit: " + std::to_string(iterationCap));
      // Optionally reset or take other action
    }
  }
}
