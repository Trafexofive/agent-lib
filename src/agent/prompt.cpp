#include "../../inc/Agent.hpp"
#include "../../inc/Tool.hpp"
#include "../../inc/ToolRegistry.hpp" // For ToolRegistry
#include "../../inc/Utils.hpp"        // For executeScriptTool

static std::string xmlWrapHelper(const std::string &content,
                                 const std::string &tagName,
                                 size_t indent = 0) {
  std::string indentStr(indent, '\t');

  return indentStr + "<" + tagName + ">\n" + indentStr + "</" + tagName + ">\n";
}

// --- Private Helper Methods (Implementations) ---
std::string Agent::buildFullPrompt() const {

  size_t indentLevel = 1;

  std::stringstream promptSs;
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
    promptSs << "<sub_agents online>\n";
    for (const auto &pair : subAgents) {
      promptSs << "\t<sub_agent name=\"" << pair.first << "\"/>\n";
    }
    promptSs << "</sub_agents online>\n\n";
  }

  std::map<std::string, std::string> allAvailableActions =
      internalFunctionDescriptions;
  for (const auto &pair : registeredTools) {
    if (pair.second)
      allAvailableActions[pair.first] = pair.second->getDescription();
  }

  if (!allAvailableActions.empty()) {
    promptSs << "<available_actions_reference>\n"; // Renamed for clarity
    for (const auto &pair : allAvailableActions) {
      promptSs << "\t<action_definition name=\"" << pair.first
               << "\">\n"; // Renamed
      promptSs << "\t\t<description_text>" << pair.second
               << "</description_text>\n"; // Renamed
      promptSs << "\t</action_definition>\n";
    }
    promptSs << "</available_actions_reference>\n\n";
  }

  if (currentDirective.type != AgentDirective::Type::NORMAL ||
      !currentDirective.description.empty() ||
      !currentDirective.format.empty()) {
    promptSs << "<current_operational_directive>\n"; // Renamed
    promptSs << "\t<directive_type>"
             << directiveTypeToString(currentDirective.type)
             << "</directive_type>\n";
    if (!currentDirective.description.empty())
      promptSs << "\t<directive_description>" << currentDirective.description
               << "</directive_description>\n";
    if (!currentDirective.format.empty())
      promptSs << "\t<directive_expected_output_format>"
               << currentDirective.format
               << "</directive_expected_output_format>\n";
    promptSs << "</current_operational_directive>\n\n";
  }

  if (!extraSystemPrompts.empty()) {
    promptSs << "<additional_guidance>\n"; // Renamed
    for (const auto &p : extraSystemPrompts)
      promptSs << "\t<instruction>" << p << "</instruction>\n";
    promptSs << "</additional_guidance>\n\n";
  }

  std::string memoryContextBlock;
  if (!scratchpad.empty()) {
    memoryContextBlock += "\t<scratchpad_contents>\n"; // Renamed
    for (const auto &item : scratchpad)
      memoryContextBlock += "\t\t<item key=\"" + item.first + "\"><![CDATA[" +
                            item.second + "]]></item>\n";
    memoryContextBlock += "\t</scratchpad_contents>\n";
  }
  // Add ShortTermMemory and LongTermMemory similarly if needed, potentially
  // summarized for LongTerm.
  if (!memoryContextBlock.empty()) {
    promptSs << "<internal_memory_context>\n"
             << memoryContextBlock
             << "</internal_memory_context>\n\n"; // Renamed
  }

  const size_t MAX_PAST_HISTORY = 100; // items

  if (!conversationHistory.empty()) {
      // include only the last 100 items

      for (size_t i = 0; i < conversationHistory.size() &&
                      i < MAX_PAST_HISTORY; ++i) {
        const auto &item = conversationHistory[conversationHistory.size() - 1 - i];
        promptSs << "<past_conversation_item>\n";
        promptSs << "\t<role>" << item.first<< "</role>\n";
        promptSs << "\t<content><![CDATA[" << item.second << "]]></content>\n";
        promptSs << "</past_conversation_item>\n";

      }
  }

  return promptSs.str();
}
