#include "../../inc/Agent.hpp" // Your Agent class header
#include <fstream>
#include <iostream> // For basic error output in this example
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

// Include yaml-cpp headers
#include <yaml-cpp/emitter.h> // For exporting
#include <yaml-cpp/yaml.h>

// --- Utility Function ---

// implicite extraction, strings and scalars are treated as strings. the issue
// is that a string can contain a file/folder/executable those will be handled

// Loads configuration FROM a YAML file INTO an existing Agent object.
// Overwrites relevant configuration fields on the agent object.
// Returns true on success, false on failure.
bool loadAgentProfile(Agent &agentToConfigure, const std::string &yamlPath) {
  logMessage(LogLevel::INFO, "Attempting to load agent profile", yamlPath);
  YAML::Node config;
  try {
    // Check if file exists first (LoadFile throws if not found, but good
    // practice)
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
      agentToConfigure.setSystemPrompt(
          config["system_prompt"].as<std::string>());
    }
    if (config["schema"] && config["schema"].IsScalar()) {
      agentToConfigure.setSchema(config["schema"].as<std::string>());
    }
    if (config["example"] && config["example"].IsScalar()) {
      agentToConfigure.setExample(config["example"].as<std::string>());
    }
    if (config["iteration_cap"] && config["iteration_cap"].IsScalar()) {
      try {
        agentToConfigure.setIterationCap(config["iteration_cap"].as<int>());
      } catch (const YAML::BadConversion &e) { // <-- CHANGE IS HERE
        logMessage(LogLevel::WARN,
                   "Invalid iteration_cap value in profile (bad conversion), "
                   "using agent's default",
                   yamlPath + ": " + e.what());
      }
    }

    if (config["extra_prompts"] && config["extra_prompts"].IsSequence()) {
      // agentToConfigure.clearExtraPrompts(); // Example if overwrite needed
      for (const auto &item : config["extra_prompts"]) {
        if (item.IsScalar()) {
          agentToConfigure.addPrompt(item.as<std::string>());
        }
      }
    }

    // if (config["variables"] && config["variables"].IsMap()) {
    //   for (const auto &it : config["variables"]) {
    //     std::string key = it.first.as<std::string>();
    //     std::string value = it.second.as<std::string>();
    //
    //     if (extractType(key) == "file") {
    //
    //       // } else if (extractType(key) == "tool") {
    //     } else if (extractType(key) == "folder") { // should use the
    //       extractType methoe in order to recursivaly import the files
    //           apropriatly depending on the type
    //     } else if (extractType(key) == "executable") {
    //     } else if (extractType(key) == "string") {
    //     }
    //
    //     agentToConfigure.addEnvVar(key, value);
    //   }
    //   m
    // }

    // --- Load Environment ---
    // Assuming Agent might have a clearEnvVars() or similar if overwrite is
    // desired, otherwise this just adds/updates variables.
    if (config["environment"] && config["environment"].IsMap()) {
      for (const auto &it : config["environment"]) {
        std::string key = it.first.as<std::string>();
        std::string value = it.second.as<std::string>();
        agentToConfigure.addEnvVar(key, value);
      }
    }

    // --- Load Optional Sections ---
    // Assuming Agent might have clear methods if overwrite is desired.
    if (config["extra_prompts"] && config["extra_prompts"].IsSequence()) {
      // agentToConfigure.clearExtraPrompts(); // Example if overwrite needed
      for (const auto &item : config["extra_prompts"]) {
        if (item.IsScalar()) {
          agentToConfigure.addPrompt(item.as<std::string>());
        }
      }
    }
    if (config["tasks"] && config["tasks"].IsSequence()) {
      // agentToConfigure.clearTasks(); // Example if overwrite needed
      for (const auto &item : config["tasks"]) {
        if (item.IsScalar()) {
          agentToConfigure.addTask(item.as<std::string>());
        }
      }
    }

    // --- Load Directive ---
    if (config["directive"] && config["directive"].IsMap()) {
      Agent::DIRECTIVE dir; // Assuming Agent::DIRECTIVE is accessible or has
                            // public members/setters
      const auto &dirNode = config["directive"];
      if (dirNode["type"] && dirNode["type"].IsScalar()) {
        std::string typeStr = dirNode["type"].as<std::string>();
        // Map string to enum (example logic, adjust based on your
        // Agent::DIRECTIVE)
        if (typeStr == "BRAINSTORMING")
          dir.type = Agent::DIRECTIVE::Type::BRAINSTORMING;
        else if (typeStr == "AUTONOMOUS")
          dir.type = Agent::DIRECTIVE::Type::AUTONOMOUS;
        else if (typeStr == "EXECUTE")
          dir.type = Agent::DIRECTIVE::Type::EXECUTE;
        else if (typeStr == "REPORT")
          dir.type = Agent::DIRECTIVE::Type::REPORT;
        else
          dir.type = Agent::DIRECTIVE::Type::NORMAL; // Default
      }
      if (dirNode["description"] && dirNode["description"].IsScalar()) {
        dir.description = dirNode["description"].as<std::string>();
      }
      if (dirNode["format"] && dirNode["format"].IsScalar()) {
        dir.format = dirNode["format"].as<std::string>();
      }
      agentToConfigure.setDirective(
          dir); // Assuming setDirective takes the struct
    }

    // --- What is NOT loaded from profile ---
    // - Runtime state (iteration, skipFlowIteration, scratchpad)
    // - History
    // - Memory (long/short term - unless explicitly defined in YAML and
    // loader handles it)
    // - Registered Tool* pointers (profile might *list* expected tools, but
    // instances aren't loaded)
    // - Registered Agent* pointers (sub-agents)

    logMessage(LogLevel::INFO, "Successfully loaded agent profile", yamlPath);
    return true;

  } catch (const YAML::Exception &e) {
    logMessage(LogLevel::ERROR, "Failed to load or parse agent profile YAML",
               yamlPath + ": " + e.what());
    return false;
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR, "Generic error loading agent profile",
               yamlPath + ": " + e.what());
    return false;
  }
}
