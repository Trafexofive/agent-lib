// #include "../../inc/Agent.hpp"
// #include "../../inc/MiniGemini.hpp"
// #include "../../inc/Tool.hpp"
// #include <algorithm> // For std::find_if, std::remove_if
// #include <cctype>    // For std::toupper
// #include <chrono>    // For timestamp
// #include <cstdio>    // For FILE, fgets
// #include <cstdlib>   // For popen, pclose, system
// #include <ctime>
// #include <fstream> // For file operations
// #include <iomanip> // For formatting time
// #include <iostream>
// #include <json/json.h>
// #include <sstream>
// #include <stdexcept>
//
// std::string Agent::handleToolExecution(const ToolCallInfo &call) {
//   logMessage(LogLevel::TOOL_CALL, "Executing tool '" + call.toolName + "'",
//              "Params: " + call.params.toStyledString());
//   std::string result;
//   try {
//     // Internal Tools
//     if (call.toolName == "help")
//       result = this->getHelp(call.params);
//     else if (call.toolName == "skip")
//       result = skip(call.params);
//     else if (call.toolName == "promptAgent")
//       result = promptAgentTool(call.params);
//     else if (call.toolName == "summarizeTool")
//       result = summerizeTool(call.params);
//     else if (call.toolName == "summarizeHistory")
//       result = summarizeHistory(call.params);
//     else if (call.toolName == "getWeather")
//       result = getWeather(call.params);
//     // External Tools
//     else {
//       Tool *tool = getTool(call.toolName);
//       if (tool) {
//         result = tool->execute(call.params);
//       } else {
//         logMessage(LogLevel::WARN, "Attempted to execute unknown tool: '" +
//                                        call.toolName + "'");
//         result = "Error: Unknown tool '" + call.toolName + "'.";
//       }
//     }
//     logMessage(LogLevel::TOOL_RESULT, "Tool '" + call.toolName + "' executed.",
//                "Result: " + result);
//   } catch (const std::exception &e) {
//     result = "Error executing tool '" + call.toolName + "': " + e.what();
//     logMessage(LogLevel::ERROR,
//                "Exception during tool execution for '" + call.toolName + "'.",
//                "Error: " + std::string(e.what()));
//   } catch (...) {
//     result = "Error: Unknown exception executing tool '" + call.toolName + "'";
//     logMessage(LogLevel::ERROR,
//                "Unknown exception during tool execution for '" + call.toolName +
//                    "'.");
//   }
//   return result;
// }
//
// std::string
// Agent::processToolCalls(const std::vector<ToolCallInfo> &toolCalls) {
//   if (toolCalls.empty())
//     return "";
//   std::ostringstream toolResultsOss;
//   toolResultsOss << "Tool Execution Results:\n";
//   for (const auto &call : toolCalls) {
//     std::string result = handleToolExecution(call);
//     toolResultsOss << "--- Tool: " << call.toolName << " ---\n"
//                    << "Params: " << call.params.toStyledString()
//                    << "Result: " << result << "\n";
//   }
//   toolResultsOss << "--- End Tool Results ---";
//   return toolResultsOss.str();
// }
//
