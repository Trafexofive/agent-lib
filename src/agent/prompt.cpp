// #include "../../inc/Agent.hpp" // Assuming this is the correct path
//
// std::string Agent::buildFullPrompt() const {
//   std::stringstream promptSs;
//   if (!systemPrompt.empty()) {
//     promptSs << "<system_prompt>\n" << systemPrompt << "\n</system_prompt>\n\n";
//   }
//   if (!llmResponseSchema.empty()) {
//     promptSs << "<agent_reply_schema>\n"
//              << llmResponseSchema << "\n</agent_reply_schema>\n\n";
//   }
//   if (!llmResponseExample.empty()) {
//     promptSs << "<agent_reply_example>\n"
//              << llmResponseExample << "\n</agent_reply_example>\n\n";
//   }
//
//   promptSs << "<agent_context>\n";
//   promptSs << "\t<name>" << agentName << "</name>\n";
//   if (!agentDescription.empty())
//     promptSs << "\t<description>" << agentDescription << "</description>\n";
//   promptSs << "</agent_context>\n\n";
//
//   if (!environmentVariables.empty()) {
//     promptSs << "<environment_variables>\n";
//     for (const auto &pair : environmentVariables) {
//       promptSs << "\t<variable name=\"" << pair.first << "\">" << pair.second
//                << "</variable>\n";
//     }
//     promptSs << "</environment_variables>\n\n";
//   }
//
//   std::map<std::string, std::string> allAvailableActions =
//       internalFunctionDescriptions;
//   for (const auto &pair : registeredTools) {
//     if (pair.second)
//       allAvailableActions[pair.first] = pair.second->getDescription();
//   }
//   if (!allAvailableActions.empty()) {
//     promptSs << "<available_actions>\n";
//     for (const auto &pair : allAvailableActions) {
//       promptSs << "\t<action name=\"" << pair.first << "\">\n";
//       promptSs << "\t\t<description>" << pair.second << "</description>\n";
//       // Could add param schema here in future
//       promptSs << "\t</action>\n";
//     }
//     promptSs << "</available_actions>\n\n";
//   }
//
//   if (currentDirective.type != AgentDirective::Type::NORMAL ||
//       !currentDirective.description.empty()) {
//     promptSs << "<current_directive>\n";
//     promptSs << "\t<type>" << directiveTypeToString(currentDirective.type)
//              << "</type>\n";
//     if (!currentDirective.description.empty())
//       promptSs << "\t<description>" << currentDirective.description
//                << "</description>\n";
//     if (!currentDirective.format.empty())
//       promptSs << "\t<expected_format>" << currentDirective.format
//                << "</expected_format>\n";
//     promptSs << "</current_directive>\n\n";
//   }
//
//   if (!extraSystemPrompts.empty()) {
//     promptSs << "<additional_instructions>\n";
//     for (const auto &p : extraSystemPrompts)
//       promptSs << "\t" << p << "\n";
//     promptSs << "</additional_instructions>\n\n";
//   }
//
//   // Memory context
//   std::string memoryBlock;
//   if (!scratchpad.empty()) {
//     memoryBlock += "\t<scratchpad>\n";
//     for (const auto &item : scratchpad)
//       memoryBlock +=
//           "\t\t<item key=\"" + item.first + "\">" + item.second + "</item>\n";
//     memoryBlock += "\t</scratchpad>\n";
//   }
//   if (!shortTermMemory.empty()) {
//     memoryBlock += "\t<short_term_memory>\n";
//     for (const auto &item : shortTermMemory)
//       memoryBlock += "\t\t<entry role=\"" + item.first + "\">" + item.second +
//                      "</entry>\n";
//     memoryBlock += "\t</short_term_memory>\n";
//   }
//   if (!longTermMemory
//            .empty()) { // Assuming long term memory might be more substantial,
//                        // could be summarized or selectively included
//     memoryBlock += "\t<long_term_memory_summary>\n"; // Placeholder for now
//     for (const auto &item : longTermMemory)
//       memoryBlock += "\t\t<entry role=\"" + item.first + "\">" +
//                      item.second.substr(0, 100) + "...</entry>\n";
//     memoryBlock += "\t</long_term_memory_summary>\n";
//   }
//   if (!memoryBlock.empty()) {
//     promptSs << "<memory_context>\n" << memoryBlock << "</memory_context>\n\n";
//   }
//
//   if (!conversationHistory.empty()) {
//     promptSs << "<conversation_history>\n";
//     for (const auto &entry : conversationHistory) {
//       promptSs << "\t<turn role=\"" << entry.first << "\">\n";
//       // Basic XML escaping for content - just & and < for this example
//       std::string escapedContent = entry.second;
//       size_t pos = 0;
//       while ((pos = escapedContent.find('&', pos)) != std::string::npos) {
//         escapedContent.replace(pos, 1, "&");
//         pos += 5;
//       }
//       pos = 0;
//       while ((pos = escapedContent.find('<', pos)) != std::string::npos) {
//         escapedContent.replace(pos, 1, "<");
//         pos += 4;
//       }
//
//       std::stringstream contentSs(escapedContent); // Use escaped content
//       std::string line;
//       while (std::getline(contentSs, line)) {
//         promptSs << "\t\t" << line << "\n";
//       }
//       promptSs << "\t</turn>\n";
//     }
//     promptSs << "</conversation_history>\n\n";
//   }
//
//   // schema && Example listing
//   if (!llmResponseSchema.empty()) {
//     promptSs << "<schema>\n" << llmResponseSchema << "\n</schema>\n\n";
//   }
//   if (!llmResponseExample.empty()) {
//     promptSs << "<example>\n" << llmResponseExample << "\n</example>\n\n";
//   }
//
//   promptSs << "CONTEXT END\n\n";
//   // promptSs << "RESPONSE INSTRUCTIONS: Based on the entire context above, "
//   //             "generate your response strictly as a single JSON object. "
//   //          << "This JSON object MUST contain 'status' (string), 'thoughts' "
//   //             "(array of objects with 'type' and 'content'), "
//   //          << "'actions' (array of objects with 'action', 'type', 'params'; "
//   //             "use '[]' if no actions), "
//   //          << "and 'final_response' (string or null; typically null if
//   //          actions "
//   //             "are present).";
//
//   // logMessage(LogLevel::PROMPT, "Full prompt for agent " + agentName + ":",
//   // promptSs.str()); // Can be very verbose
//   return promptSs.str();
// }
