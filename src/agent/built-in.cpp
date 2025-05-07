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
