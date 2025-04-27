#include "../../inc/Agent.hpp"
#include "../../inc/MiniGemini.hpp"
#include "../../inc/Tool.hpp"
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

#include "../../inc/modelApi.hpp" // For ApiError

std::string Agent::buildFullPrompt() const {
  std::ostringstream oss;
  // --- Context Management ---
  // TODO (Phase 2+): Implement History Summarization & Memory Retrieval

  oss << systemPrompt << "\n\n";

  if (!extraPrompts.empty()) {
    oss << "Additional Instructions:\n";
    for (const auto &extraPrompt : extraPrompts)
      oss << "- " << extraPrompt << "\n";
    oss << "\n";
  }

  oss << "Agent Information:\n- Name: " << name << "\n";
  if (!description.empty())
    oss << "- Description: " << description << "\n";
  oss << "\n";

  if (!env.empty()) {
    oss << "Environment Variables:\n";
    for (const auto &pair : env)
      oss << "- " << pair.first << "=" << pair.second << "\n";
    oss << "\n";
  }

  if (!subAgents.empty()) {
    oss << "Available Sub-Agents (Interact using 'promptAgent' tool):\n";
    for (const auto &pair : subAgents)
      oss << "- Name: " << pair.first
          << " (Description: " << pair.second->getDescription() << ")\n";
    oss << "\n";
  }

  bool hasTools = !tools.empty() || !internalToolDescriptions.empty();
  if (hasTools) {
    oss << "Available Tools (Use STRICT JSON format: {\"tool_name\": \"...\", "
           "\"params\": {...}} within 'tool_calls'):\n";
    for (const auto &pair : internalToolDescriptions)
      oss << "- " << pair.first << ": " << pair.second << "\n";
    for (const auto &pair : tools)
      oss << "- " << pair.second->getName() << ": "
          << pair.second->getDescription() << "\n";
    oss << "\n";
  }

  if (!history.empty()) {
    oss << "Conversation History (Recent relevant context):\n";
    // TODO (Phase 2+): Only include relevant/summarized history
    for (const auto &entry : history) {
      std::string role = entry.first;
      if (!role.empty())
        role[0] = std::toupper(static_cast<unsigned char>(role[0]));
      oss << role << ": " << entry.second << "\n";
    }
    oss << "\n";
  }

  oss << "Assistant Response (Provide JSON with 'thought', 'tool_calls', "
         "'final_response'):";

  logMessage(LogLevel::PROMPT, "Full prompt built for Agent '" + name + "'");
  return oss.str();
}

