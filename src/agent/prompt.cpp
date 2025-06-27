#include "../../inc/Agent.hpp"
#include "../../inc/Tool.hpp"
#include "../../inc/ToolRegistry.hpp" // For ToolRegistry
#include "../../inc/Utils.hpp"        // For executeScriptTool
#include <ctime>
#include <iomanip>
#include <sstream>

static std::string xmlWrapHelper(const std::string &content,
                                 const std::string &tagName,
                                 size_t indent = 0) {
  std::string indentStr(indent, '\t');
  return indentStr + "<" + tagName + ">\n" + content + "\n" + indentStr + "</" +
         tagName + ">\n";
}

static std::string getFormattedDateTime() {
  std::time_t now = std::time(0);
  std::tm *timeinfo = std::localtime(&now);

  std::stringstream ss;
  ss << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S %Z");
  return ss.str();
}

// static void injectSysPrompt()
// --- Private Helper Methods (Implementations) ---
std::string Agent::buildFullPrompt() const {
  size_t indentLevel = 1;
  std::stringstream promptSs;

  // live updated metadata, time,date and weather, etc.
  // in the future we could have external tools here.
  promptSs << "<live_metadata>\n";
  promptSs << "\t<current_datetime>" << getFormattedDateTime()
           << "</current_datetime>\n";
  // TODO: Add weather, system stats, etc. when available
  promptSs << "</live_metadata>\n\n";

  // TODO: add a tool that will register some tools with predefined input, it
  // will execute every time and append to the system prompt

  if (!systemPrompt.empty()) {
    promptSs << "<system_prompt>\n" << systemPrompt << "\n</system_prompt>\n\n";
  }

  // Add schema and example if they exist
  if (!llmResponseSchema.empty()) {
    promptSs << "<response_schema_definition>\n"
             << llmResponseSchema << "\n</response_schema_definition>\n\n";
  }

  if (!llmResponseExample.empty()) {
    promptSs << "<response_example>\n"
             << llmResponseExample << "\n</response_example>\n\n";
  }

  promptSs << "<agent_identity>\n";
  promptSs << "\t<name>" << agentName << "</name>\n";
  if (!agentDescription.empty())
    promptSs << "\t<description>" << agentDescription << "</description>\n";
  promptSs << "</agent_identity>\n\n";

  if (!environmentVariables.empty()) {
    promptSs << "<environment_variables>\n";
    for (const auto &pair : environmentVariables) {
      promptSs << "\t<variable name=\"" << pair.first << "\">" << pair.second
               << "</variable>\n";
    }
    promptSs << "</environment_variables>\n\n";
  }

  if (!subAgents.empty()) {
    promptSs << "<sub_agents_online>\n";
    for (const auto &pair : subAgents) {
      promptSs << "\t<sub_agent name=\"" << pair.first << "\"/>\n";
      promptSs << "\t<sub_agent_description>" << pair.second->getDescription()
               << "</sub_agent_description>\n";

      for (const auto &action: pair.second->registeredTools) {
          // TODO: if action.description is already in allAvailableActions, display name only
        if (action.second )
            promptSs << "\t<action_definition name=\"" << action.first << "\"";
                     // << "\">" << pair.second->getDescription()
                     // << "</action_definition>\n";
      }
    }
    promptSs << "</sub_agents_online>\n\n";
  }

  std::map<std::string, std::string> allAvailableActions =
      internalFunctionDescriptions;
  for (const auto &pair : registeredTools) {
    if (pair.second)
      allAvailableActions[pair.first] = pair.second->getDescription();
  }

  if (!allAvailableActions.empty()) {
    promptSs << "<available_actions_reference>\n";
    for (const auto &pair : allAvailableActions) {
      promptSs << "\t<action_definition name=\"" << pair.first << "\">\n";
      promptSs << "\t\t<description_text>" << pair.second
               << "</description_text>\n";
      promptSs << "\t</action_definition>\n";
    }
    promptSs << "</available_actions_reference>\n\n";
  }

  if (!extraSystemPrompts.empty()) {
    promptSs << "<additional_guidance>\n"; // Renamed
    for (const auto &p : extraSystemPrompts)
      promptSs << "\t<instruction>" << p << "</instruction>\n";
    promptSs << "</additional_guidance>\n\n";
  }

  const size_t MAX_PAST_HISTORY = 100; // items
  if (!conversationHistory.empty()) {
    promptSs << "<conversation_history>\n";
    for (size_t i = 0; i < conversationHistory.size() && i < MAX_PAST_HISTORY;
         ++i) {
      const auto &item = conversationHistory[i];
      promptSs << "\t<past_conversation_item>\n";
      promptSs << "\t\t<role>" << item.first << "</role>\n";
      promptSs << "\t\t<content>" << item.second << "</content>\n";
      promptSs << "\t</past_conversation_item>\n";
    }
    promptSs << "</conversation_history>\n\n";
  }

  return promptSs.str();
}
