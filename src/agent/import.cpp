#include "../../inc/Agent.hpp"
#include "../../inc/Tool.hpp"  // Include the simplified Tool.hpp
#include "../../inc/Utils.hpp" // For executeScriptTool
#include <fstream>
#include <iostream>
#include <memory> // For std::make_unique if Agent stores ToolPtr
#include <stdexcept>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>


// ascii json graph tool

// return a buffer for a simple json graph in ascii pillars
std::string simpleJsonGraph(const Json::Value &jsonValue) {
  std::string graph;
  for (const auto &key : jsonValue.getMemberNames()) {
    graph += key + ": " + jsonValue[key].asString() + "\n";
  }
  return graph;
}

// display any json table infinite collumns and rows
std::string jsonTable(const Json::Value &jsonValue) {
  std::string table;
  for (const auto &key : jsonValue.getMemberNames()) {
    table += key + ": " + jsonValue[key].asString() + "\n";
  }
  return table;
}


// Utility
// file expansion utility function, can extract the file name from a path
std::string getFileName(const std::string &filePath) {
  size_t lastSlash = filePath.find_last_of("/\\");
  if (lastSlash == std::string::npos) {
    return filePath; // No path, return the file name
  }
  return filePath.substr(lastSlash + 1);
}
// relative path utility function, can extract the relative path from a full path
std::string getRelativePath(const std::string &filePath) {
  size_t lastSlash = filePath.find_last_of("/\\");
  if (lastSlash == std::string::npos) {
    return ""; // No path, return empty string
  }
  return filePath.substr(0, lastSlash);
}

// get file content relative mode
std::string getFileContent(const std::string &filePath) {
  std::ifstream file(filePath);
  if (!file.is_open()) {
    logMessage(LogLevel::ERROR, "Failed to open file", filePath);
    return "";
  }
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();
  return content;
}

// takes string + std::vector<string, string> env expands the string
std::string expandStringWithEnv(const std::string &input,
                                         const StringKeyValuePair &env) {
  std::string expanded = input;
  for (const auto &pair : env) {
    std::string placeholder = "{" + pair.first + "}";
    size_t pos = expanded.find(placeholder);
    if (pos != std::string::npos) {
      expanded.replace(pos, placeholder.length(), pair.second);
    }
  }
  return expanded;
}

// should be accessible via the state namespace
// class runtimeEnv { // reference to the runtime environment/context
//     private:
//         std::string id;
//         std::vector<std::string, std::string> env;
//         std::vector<std::string, std::string> variables;
//     public:
//
//
// };

// expand paths in $(PATH) format with xml bindings
const std::string expandPathWithEnv(const std::string &input,
                                         const StringKeyValuePair &env) {
  std::string expanded = input;
  for (const auto &pair : env) {
    std::string placeholder = "$(" + pair.first + ")";
    size_t pos = expanded.find(placeholder);
    if (pos != std::string::npos) {
      expanded.replace(pos, placeholder.length(), pair.second);
    }
  }
  return expanded;
}
// one without env, will expand any/all $(PATH) format 
// const std::string expandPath(const std::string &input) {


bool loadAgentProfile(Agent &agentToConfigure, const std::string &yamlPath) {

  logMessage(LogLevel::INFO,
             "Attempting to load agent profile (B-line Tool Import)", yamlPath);
  YAML::Node config;

  try {
    std::ifstream f(yamlPath.c_str());
    if (!f.good()) {
      logMessage(LogLevel::ERROR, "Agent profile file not found", yamlPath);
      return false;
    }
    f.close();
    config = YAML::LoadFile(yamlPath);

    // --- Load Core Identity & Configuration (Name, Description, System Prompt,
    // etc.) ---
    if (config["name"] && config["name"].IsScalar()) {
      agentToConfigure.setName(config["name"].as<std::string>());
    }
    if (config["description"] && config["description"].IsScalar()) {
      agentToConfigure.setDescription(config["description"].as<std::string>());
    }
    // check if system_prompt is a scalar or markdown file relative to the yamlPath i.e config/agents/agent.yaml | config/agents/sysprompts/sysprompt.md
    if (config["system_prompt"] && config["system_prompt"].IsScalar()) {
      std::string systemPrompt = config["system_prompt"].as<std::string>();
      if (systemPrompt.find(".md") != std::string::npos) {
        // Load the markdown file
        std::ifstream promptFile(yamlPath.substr(0, yamlPath.find_last_of("/")) +
                                  "/" + systemPrompt);
        if (promptFile.good()) {
          std::string line;
          std::string fullPrompt;
          while (std::getline(promptFile, line)) {
            fullPrompt += line + "\n";
          }
          agentToConfigure.setSystemPrompt(fullPrompt);
        } else {
          logMessage(LogLevel::ERROR,
                     "System prompt file not found", systemPrompt);
        }
      } else {
        agentToConfigure.setSystemPrompt(systemPrompt);
      }
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
      } catch (const YAML::BadConversion &e) {
        logMessage(
            LogLevel::WARN,
            "Invalid iteration_cap value in profile, using agent's default",
            yamlPath + ": " + e.what());
      }
    }
    // ... (load environment, extra_prompts, tasks, directive as before) ... 
    if (config["environment"] && config["environment"].IsMap()) { // ability to add .env and env from files in general
      for (const auto &it : config["environment"]) {
        std::string key = it.first.as<std::string>();
        std::string value = it.second.as<std::string>();
        agentToConfigure.addEnvironmentVariable(key, value);
      }
    }
    if (config["extra_prompts"] && config["extra_prompts"].IsSequence()) {
      for (const auto &item : config["extra_prompts"]) {
        if (item.IsScalar()) {
          agentToConfigure.addExtraSystemPrompt(item.as<std::string>());
        }
      }
    }
    // ... (rest of the non-tool loading logic) ...

    // --- Load Tools ---
    if (config["tools"] && config["tools"].IsMap()) {
      for (const auto &toolConfigEntryNode : config["tools"]) {
        std::string logicalToolKey =
            toolConfigEntryNode.first.as<std::string>(); // e.g., "BashExecutor"
        YAML::Node toolDef = toolConfigEntryNode.second;

        if (!toolDef["name"] || !toolDef["name"].IsScalar() ||
            !toolDef["description"] || !toolDef["description"].IsScalar() ||
            !toolDef["type"] || !toolDef["type"].IsScalar()) {
          logMessage(LogLevel::WARN,
                     "Skipping malformed tool definition in YAML (missing "
                     "name, desc, or type)",
                     logicalToolKey);
          continue;
        }

        std::string actualToolName = toolDef["name"].as<std::string>();
        std::string description = toolDef["description"].as<std::string>();
        std::string type = toolDef["type"].as<std::string>();
        std::string runtime;

        if (toolDef["runtime"] && toolDef["runtime"].IsScalar()) {
          runtime = toolDef["runtime"].as<std::string>();
        } else if (type == "code" || type == "file") {
          logMessage(LogLevel::WARN,
                     "Skipping tool due to missing 'runtime' for type 'code' "
                     "or 'file'",
                     actualToolName);
          continue;
        }

        // Create a new Tool instance.
        // Agent will own this pointer. If using unique_ptr, adjust
        // agent.addTool.
        Tool *newTool = new Tool(actualToolName, description);

        FunctionalToolCallback toolLambdaCallback;

        if (type == "code") {
          if (!toolDef["code"] || !toolDef["code"].IsScalar()) {
            logMessage(
                LogLevel::WARN,
                "Skipping 'code' tool with missing or invalid 'code' block",
                actualToolName);
            delete newTool;
            continue;
          }
          std::string scriptContent = toolDef["code"].as<std::string>();

          toolLambdaCallback = [scriptContent, runtime, actualToolName](
                                   const Json::Value &params) -> std::string {
            // Capture necessary variables by value for the lambda
            return executeScriptTool(scriptContent, runtime, params,
                                     true /*isContentInline*/);
          };

        } else if (type == "file") {
          if (!toolDef["path"] || !toolDef["path"].IsScalar()) {
            logMessage(LogLevel::WARN,
                       "Skipping 'file' tool with missing or invalid 'path'",
                       actualToolName);
            delete newTool;
            continue;
          }
          std::string scriptPath = toolDef["path"].as<std::string>();

          toolLambdaCallback = [scriptPath, runtime, actualToolName](
                                   const Json::Value &params) -> std::string {
            // Capture necessary variables by value
            return executeScriptTool(scriptPath, runtime, params,
                                     false /*isContentInline*/);
          };
        } else {
          logMessage(LogLevel::WARN, "Unsupported tool type in YAML",
                     type + " for tool " + actualToolName);
          delete newTool;
          continue;
        }

        newTool->setCallback(toolLambdaCallback);
        agentToConfigure.addTool(
            newTool); // Agent takes ownership if storing raw Tool*
                      // If Agent stores unique_ptr<Tool>, use:
                      // agentToConfigure.addTool(std::unique_ptr<Tool>(newTool));
        logMessage(LogLevel::INFO, "Loaded and registered tool from YAML",
                   actualToolName + " (Type: " + type + ")");
      }
    }

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
