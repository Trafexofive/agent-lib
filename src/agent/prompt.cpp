
#include "../../inc/Agent.hpp"
#include "../../inc/Tool.hpp"
#include "../../inc/ToolRegistry.hpp" // For ToolRegistry
#include "../../inc/Utils.hpp"        // For executeScriptTool

// --- Private Helper Methods (Implementations) ---
std::string Agent::buildFullPrompt() const {
  // Implementation from src/agent/prompt.cpp
  // (This was already quite complete, ensure it's consistent with Agent.hpp
  // state)
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
  
  // std::map<std::string, std::string> allAvailableActions =
  //     internalFunctionDescriptions;
  // if (!allAvailableActions.empty()) {
  //   promptSs << "<available_actions_reference>\n";
  //   for (const auto &pair : allAvailableActions) { // pair.first is tool name,
  //                                                  // pair.second is description
  //     Tool *toolPtr = getTool(
  //         pair.first); // Check if it's a registered (script/external) tool
  //     std::string toolTypeStr = "unknown";
  //     if (toolPtr) { // It's a Tool object, likely script or complex
  //       // We need a way for Tool class to store its definition type (script,
  //       // C++ internal via registry, etc.) For now, assume tools loaded from
  //       // YAML with 'code' are 'script' type for LLM. If it was loaded via
  //       // ToolRegistry, it's effectively an internal_function in behavior. This
  //       // part needs refinement in how tools store their 'type' as defined in
  //       // YAML. Let's assume for tools loaded from YAML that are 'script', we
  //       // can tell the LLM 'script'. And internal C++ functions in
  //       // internalFunctionDescriptions are 'internal_function'.
  //       toolTypeStr = "script"; // Default assumption for tools in
  //                               // registeredTools map for now
  //     } else if (internalFunctionDescriptions.count(pair.first)) {
  //       toolTypeStr = "internal_function";
  //     }
  //
  //     promptSs << "\t<action_definition name=\"" << pair.first << "\" type=\""
  //              << toolTypeStr << "\">\n"; // <<< ADDED TYPE ATTRIBUTE
  //     promptSs << "\t\t<description_text>" << pair.second
  //              << "</description_text>\n";
  //     // Optionally, add params_schema here too if available
  //     promptSs << "\t</action_definition>\n";
  //   }
  //   promptSs << "</available_actions_reference>\n\n";
  // }

  // if (!tasks.empty()) {
  //   promptSs << "<tasks>\n";
  //   for (const auto &task : tasks) {
  //     promptSs << "\t<task>" << task << "</task>\n";
  //   }
  //   promptSs << "</tasks>\n\n";
  // }

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

  if (!conversationHistory.empty()) {
    promptSs << "<conversation_history_log>\n"; // Renamed
    for (const auto &entry : conversationHistory) {
      promptSs << "\t<turn role=\"" << entry.first
               << "\">\n\t\t<content><![CDATA[" << entry.second
               << "]]></content>\n\t</turn>\n";
    }
    promptSs << "</conversation_history_log>\n\n";
  }

  // The final instruction about JSON format is critical.
  // promptSs
  //     << "RESPONSE_FORMATTING_INSTRUCTIONS: You MUST respond with a single, "
  //        "valid JSON object. This JSON object must strictly adhere to the "
  //        "'response_schema_definition' provided above if present, otherwise "
  //        "use the 'response_example' as a structural guide. Key fields "
  //        "expected are 'status', 'thoughts' (array of objects), 'actions' "
  //        "(array of objects or null), and 'final_response' (string or
  //        null).";
  return promptSs.str();
}
