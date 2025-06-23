#include "../../inc/Agent.hpp" // Your Agent class header

static void saveStringToFile(const std::string &filename, const std::string &content) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << content;
        file.close();
    } else {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
    }
}

static void saveThoughtsToFile(const std::vector<StructuredThought> &thoughts, const std::string &filename) 
{
    for (const auto &thought : thoughts) {
        // append each thought to the filename
        std::ofstream file(filename, std::ios::app);
        if (file.is_open()) {
            file << "Thought Type: " << thought.type << "\n";
            file << "Content: " << thought.content << "\n";
            file << "------------------------\n";
            file.close();
        } else {
            std::cerr << "Error opening file for writing: " << filename << std::endl;
        }
    }
}

static void saveJsonToFile(const Json::Value &jsonData, const std::string &filename) { 
    // will create the file if it does not exist, will append if it does, needs to be clean and error free.
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        Json::StreamWriterBuilder writer;
        std::string output = Json::writeString(writer, jsonData);
        file << output << std::endl; // Append a newline for readability
        file.close();
    } else {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
    }
}



// std::vector<StructuredThought> Agent::thinkMore(std::vector<StructuredThought> thoughts)
// {
//     std::vector<StructuredThought> ExpandedThoughts;
//
//     for (const auto &thought : thoughts) {
//         if (thought.type == "expand") {
//             // Here you would implement the logic to expand the thought
//             // For now, we just copy it to the expanded thoughts
//             ExpandedThoughts.push_back(thought);
//         } else if (thought.type == "refine") {
//             // Implement refinement logic here
//             // For now, we just copy it to the expanded thoughts
//             ExpandedThoughts.push_back(thought);
//         } else {
//             // If it's not an expand or refine type, we can just keep it as is
//             ExpandedThoughts.push_back(thought);
//         }
//     }

// std::vector<StructuredThought> Agent::ThinkMore(std::vector<StructuredThought> thoughts, size_t scale) // scaled version
// {
//     std::vector<StructuredThought> expandedThoughts;
//     // use scale to gen more thoughts in parallel
//
//     std::string promptText = "<instruction>Expand and refine the following thoughts.</instruction>\n";
//
//         for (const auto &thought : thoughts) {
//         promptText += "<thought>\n";
//         promptText += "<type>" + thought.type + "</type>\n";
//         promptText += "<content>" + thought.content + "</content>\n";
//         promptText += "</thought>\n";
//     } 
//



std::string Agent::prompt(const std::string &userInput) {

    if (!userInput.empty()) {
        addToHistory("user", userInput);
    }

    currentIteration = 0;
    std::string finalAgentResponseToUser = "";

    while (currentIteration < iterationLimit) {
        currentIteration++;
        logMessage(LogLevel::INFO, "Agent '" + agentName + "' Iteration " +
                                   std::to_string(currentIteration) + "/" +
                                   std::to_string(iterationLimit));

        std::string fullPromptText = buildFullPrompt();
        std::string llmRawResponse = executeApiCall(fullPromptText);

        std::string trimmedLlmResponse = llmRawResponse;
        trimLLMResponse(trimmedLlmResponse);

        ParsedLLMResponse parsedData = parseStructuredLLMResponse(trimmedLlmResponse);
        addToHistory("model", parsedData.rawTrimmedJson);

        if (!parsedData.success) {
            logMessage(LogLevel::ERROR,
                       "Agent '" + agentName +
                       "': Critical failure parsing LLM response. Internal "
                       "parser status: " + parsedData.status,
                       "Raw trimmed JSON: " +
                       parsedData.rawTrimmedJson.substr(0, 500));

            // Check if raw response contains valid error JSON
            Json::Value rawJsonCheck;
            Json::CharReaderBuilder rBuilder;
            std::unique_ptr<Json::CharReader> r(rBuilder.newCharReader());
            std::string err_parse_raw;

            if (r->parse(parsedData.rawTrimmedJson.c_str(),
                         parsedData.rawTrimmedJson.c_str() +
                         parsedData.rawTrimmedJson.length(),
                         &rawJsonCheck, &err_parse_raw) &&
                rawJsonCheck.isObject() && rawJsonCheck.isMember("error")) {
                finalAgentResponseToUser = parsedData.rawTrimmedJson;
            } else {
                finalAgentResponseToUser =
                    "Agent '" + agentName +
                    "' encountered an issue processing the response from the language "
                    "model. Parser status: " + parsedData.status +
                    ". Raw: " + parsedData.rawTrimmedJson.substr(0, 200);
            }
            break; // Exit on parse failure
        }

        // Log thoughts for debugging
        for (const auto &thought : parsedData.thoughts) {
            std::cout << "Thought (" << thought.type << "): " << thought.content
                      << std::endl;
        }

        logMessage(LogLevel::DEBUG,
                   "Agent '" + agentName + "': LLM Status: " + parsedData.status,
                   parsedData.rawTrimmedJson);

        // Process actions if any exist
        if (!parsedData.actions.empty()) {
            logMessage(LogLevel::INFO, "Agent '" + agentName +
                                       "': LLM requires action(s). Processing " +
                                       std::to_string(parsedData.actions.size()) +
                                       " action(s).");

            std::string actionResultsText = processActions(parsedData.actions);
            addToHistory("action_results", actionResultsText);
        }

        // Check if we should stop processing
        if (parsedData.stop) {
            if (!parsedData.finalResponseField.empty()) {
                finalAgentResponseToUser = parsedData.finalResponseField;
            }
            break; // Stop processing when LLM indicates completion
        }

        // If we've hit iteration limit without stop signal
        if (currentIteration >= iterationLimit) {
            logMessage(LogLevel::WARN, "Agent '" + agentName +
                                       "' reached iteration limit (" +
                                       std::to_string(iterationLimit) + ").");
            if (finalAgentResponseToUser.empty()) {
                finalAgentResponseToUser =
                    "Agent '" + agentName + "' has processed the maximum iterations (" +
                    std::to_string(iterationLimit) +
                    ") for this request. Please try rephrasing or "
                    "breaking down the request.";
            }
            break;
        }
    }

#define RESET "\033[0m"
#define RED "\033[31m"

    std::cout << "\n"
              << RED << agentName << ": " << RESET << finalAgentResponseToUser
              << std::endl;

    return finalAgentResponseToUser;
}
