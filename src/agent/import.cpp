#include "../../inc/Agent.hpp" // Your Agent class header
#include <string>
#include <vector>
#include <fstream>
#include <iostream> // For basic error output in this example
#include <stdexcept>

// Include yaml-cpp headers
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/emitter.h> // For exporting

// --- Placeholder Logging ---
// Replace with your actual logMessage function or desired logging mechanism
// void logMessage(LogLevel level, const std::string &message, const std::string &details = "") {
//     std::cerr << "[" << static_cast<int>(level) << "] " << message;
//     if (!details.empty()) {
//         std::cerr << " (" << details << ")";
//     }
//     std::cerr << std::endl;
// }

// --- Import Function ---
// Loads configuration FROM a YAML file INTO an existing Agent object.
// Overwrites relevant configuration fields on the agent object.
// Returns true on success, false on failure.
bool loadAgentProfile(Agent& agentToConfigure, const std::string& yamlPath) {
    logMessage(LogLevel::INFO, "Attempting to load agent profile", yamlPath);
    YAML::Node config;
    try {
        // Check if file exists first (LoadFile throws if not found, but good practice)
        std::ifstream f(yamlPath.c_str());
        if (!f.good()) {
             logMessage(LogLevel::ERROR, "Agent profile file not found", yamlPath);
             return false;
        }
        f.close();

        config = YAML::LoadFile(yamlPath);

        // --- Load Core Identity & Configuration ---
        if (config["name"] && config["name"].IsScalar()) {
            agentToConfigure.setName(config["name"].as<std::string>());
        }
        if (config["description"] && config["description"].IsScalar()) {
            agentToConfigure.setDescription(config["description"].as<std::string>());
        }
        if (config["system_prompt"] && config["system_prompt"].IsScalar()) {
            agentToConfigure.setSystemPrompt(config["system_prompt"].as<std::string>());
        }
        if (config["iteration_cap"] && config["iteration_cap"].IsScalar()) {
            try {
                agentToConfigure.setIterationCap(config["iteration_cap"].as<int>());
            } catch (const YAML::TypedBadConversion& e) {
                logMessage(LogLevel::WARN, "Invalid iteration_cap value in profile, using agent's default", yamlPath);
            }
        }

        // --- Load Environment ---
        // Assuming Agent might have a clearEnvVars() or similar if overwrite is desired,
        // otherwise this just adds/updates variables.
        if (config["environment"] && config["environment"].IsMap()) {
            for (const auto& it : config["environment"]) {
                std::string key = it.first.as<std::string>();
                std::string value = it.second.as<std::string>();
                agentToConfigure.addEnvVar(key, value);
            }
        }

        // --- Load Optional Sections ---
        // Assuming Agent might have clear methods if overwrite is desired.
        if (config["extra_prompts"] && config["extra_prompts"].IsSequence()) {
            // agentToConfigure.clearExtraPrompts(); // Example if overwrite needed
            for (const auto& item : config["extra_prompts"]) {
                if (item.IsScalar()) {
                    agentToConfigure.addPrompt(item.as<std::string>());
                }
            }
        }
        if (config["tasks"] && config["tasks"].IsSequence()) {
            // agentToConfigure.clearTasks(); // Example if overwrite needed
            for (const auto& item : config["tasks"]) {
                if (item.IsScalar()) {
                    agentToConfigure.addTask(item.as<std::string>());
                }
            }
        }

        // --- Load Directive ---
        if (config["directive"] && config["directive"].IsMap()) {
            Agent::DIRECTIVE dir; // Assuming Agent::DIRECTIVE is accessible or has public members/setters
            const auto& dirNode = config["directive"];
            if (dirNode["type"] && dirNode["type"].IsScalar()) {
                 std::string typeStr = dirNode["type"].as<std::string>();
                 // Map string to enum (example logic, adjust based on your Agent::DIRECTIVE)
                 if (typeStr == "BRAINSTORMING") dir.type = Agent::DIRECTIVE::Type::BRAINSTORMING;
                 else if (typeStr == "AUTONOMOUS") dir.type = Agent::DIRECTIVE::Type::AUTONOMOUS;
                 else if (typeStr == "EXECUTE") dir.type = Agent::DIRECTIVE::Type::EXECUTE;
                 else if (typeStr == "REPORT") dir.type = Agent::DIRECTIVE::Type::REPORT;
                 else dir.type = Agent::DIRECTIVE::Type::NORMAL; // Default
            }
            if (dirNode["description"] && dirNode["description"].IsScalar()) {
                dir.description = dirNode["description"].as<std::string>();
            }
             if (dirNode["format"] && dirNode["format"].IsScalar()) {
                dir.format = dirNode["format"].as<std::string>();
            }
            agentToConfigure.setDirective(dir); // Assuming setDirective takes the struct
        }

        // --- What is NOT loaded from profile ---
        // - Runtime state (iteration, skipFlowIteration, scratchpad)
        // - History
        // - Memory (long/short term - unless explicitly defined in YAML and loader handles it)
        // - Registered Tool* pointers (profile might *list* expected tools, but instances aren't loaded)
        // - Registered Agent* pointers (sub-agents)

        logMessage(LogLevel::INFO, "Successfully loaded agent profile", yamlPath);
        return true;

    } catch (const YAML::Exception& e) {
        logMessage(LogLevel::ERROR, "Failed to load or parse agent profile YAML", yamlPath + ": " + e.what());
        return false;
    } catch (const std::exception& e) {
        logMessage(LogLevel::ERROR, "Generic error loading agent profile", yamlPath + ": " + e.what());
        return false;
    }
}


// --- Export Function ---
// Saves the current configuration OF an Agent object TO a YAML file.
// Only saves configurable profile aspects, not full runtime state.
// Returns true on success, false on failure.
bool saveAgentProfile(const Agent& agentToSave, const std::string& yamlPath) {
    logMessage(LogLevel::INFO, "Attempting to save agent profile", yamlPath);
    YAML::Emitter emitter;
    try {
        emitter << YAML::BeginMap;

        // Add a version marker (good practice)
        emitter << YAML::Key << "version" << YAML::Value << "agent-1.1"; // Example version

        // --- Save Core Identity & Configuration ---
        emitter << YAML::Key << "name" << YAML::Value << agentToSave.getName();
        emitter << YAML::Key << "description" << YAML::Value << YAML::Literal << agentToSave.getDescription(); // Use Literal for multi-line
        emitter << YAML::Key << "system_prompt" << YAML::Value << YAML::Literal << agentToSave.getSystemPrompt(); // Use Literal for multi-line
        emitter << YAML::Key << "iteration_cap" << YAML::Value << agentToSave.getIterationCap(); // Assuming getter exists

        // --- Save Environment ---
        // Assuming Agent has a way to get environment variables, e.g., getEnv() returning a map or vector<pair>
        const auto& envVars = agentToSave.getEnv(); // *** ASSUMPTION: getEnv() exists and returns suitable type ***
        if (!envVars.empty()) {
            emitter << YAML::Key << "environment" << YAML::Value << YAML::BeginMap;
            for (const auto& pair : envVars) {
                emitter << YAML::Key << pair.first << YAML::Value << pair.second;
            }
            emitter << YAML::EndMap;
        }

        // --- Save Optional Sections ---
        // Assuming Agent has getters for these, e.g., getExtraPrompts(), getTasks()
        const auto& extraPrompts = agentToSave.getExtraPrompts(); // *** ASSUMPTION: getExtraPrompts() exists ***
        if (!extraPrompts.empty()) {
            emitter << YAML::Key << "extra_prompts" << YAML::Value << YAML::BeginSeq;
            for (const auto& prompt : extraPrompts) {
                emitter << prompt;
            }
            emitter << YAML::EndSeq;
        }
        const auto& tasks = agentToSave.getTasks(); // *** ASSUMPTION: getTasks() exists ***
        if (!tasks.empty()) {
            emitter << YAML::Key << "tasks" << YAML::Value << YAML::BeginSeq;
            for (const auto& task : tasks) {
                emitter << task;
            }
            emitter << YAML::EndSeq;
        }

        // --- Save Directive ---
        const auto& directive = agentToSave.getDirective(); // *** ASSUMPTION: getDirective() exists ***
        emitter << YAML::Key << "directive" << YAML::Value << YAML::BeginMap;
        // Convert enum back to string
        std::string typeStr = "NORMAL"; // Default
        switch (directive.type) {
            case Agent::DIRECTIVE::Type::BRAINSTORMING: typeStr = "BRAINSTORMING"; break;
            case Agent::DIRECTIVE::Type::AUTONOMOUS: typeStr = "AUTONOMOUS"; break;
            case Agent::DIRECTIVE::Type::EXECUTE: typeStr = "EXECUTE"; break;
            case Agent::DIRECTIVE::Type::REPORT: typeStr = "REPORT"; break;
            case Agent::DIRECTIVE::Type::NORMAL: // Fallthrough
            default: typeStr = "NORMAL"; break;
        }
        emitter << YAML::Key << "type" << YAML::Value << typeStr;
        emitter << YAML::Key << "description" << YAML::Value << directive.description;
        emitter << YAML::Key << "format" << YAML::Value << directive.format;
        emitter << YAML::EndMap;

        // --- What is NOT saved ---
        // - Runtime state (iteration, skipFlowIteration, scratchpad)
        // - History
        // - Memory state (long/short term)
        // - Tool definitions/instances (might save *list* of expected tool *names* if needed)
        // - Sub-agent references/definitions

        emitter << YAML::EndMap;

        // Write to file
        std::ofstream fout(yamlPath);
        if (!fout.is_open()) {
            logMessage(LogLevel::ERROR, "Failed to open file for saving agent profile", yamlPath);
            return false;
        }
        fout << emitter.c_str();
        fout.close();

        if (!emitter.good()) {
             logMessage(LogLevel::ERROR, "YAML emitter error after saving agent profile", emitter.GetLastError());
             return false; // Indicate potential issue even if file write succeeded superficially
        }

        logMessage(LogLevel::INFO, "Successfully saved agent profile", yamlPath);
        return true;

    } catch (const YAML::Exception& e) {
        logMessage(LogLevel::ERROR, "YAML emitter error during agent profile save", yamlPath + ": " + e.what());
        return false;
    } catch (const std::exception& e) {
        logMessage(LogLevel::ERROR, "Generic error saving agent profile", yamlPath + ": " + e.what());
        return false;
    }
}

// --- Example Usage ---
/*
int main() {
    // Assume myApi is an initialized MiniGemini instance
    MiniGemini myApi;
    Agent agent1(myApi);

    // Load configuration into agent1
    if (loadAgentProfile(agent1, "./agents/order_processor.yaml")) {
        std::cout << "Agent Name after load: " << agent1.getName() << std::endl;

        // Modify the agent slightly
        agent1.addEnvVar("SESSION_ID", "xyz789");
        agent1.addTask("Final review step");

        // Save the modified configuration (profile aspects only)
        if (saveAgentProfile(agent1, "./agents/order_processor_modified.yaml")) {
            std::cout << "Modified profile saved." << std::endl;
        } else {
            std::cerr << "Failed to save modified profile." << std::endl;
        }

    } else {
        std::cerr << "Failed to load agent profile." << std::endl;
    }

    return 0;
}
*/
