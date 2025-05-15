#include "../../inc/Agent.hpp"
#include "../../inc/Tool.hpp"
#include "../../inc/Utils.hpp"       // For executeScriptTool, logMessage
#include "../../inc/ToolRegistry.hpp" // For ToolRegistry
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <filesystem> // For path manipulation (C++17)

namespace fs = std::filesystem;

// Forward declaration for the new helper
std::map<std::string, Tool*> loadToolsFromFile(const std::string& toolYamlPath, Agent& agentForLoggingContext, const fs::path& toolFileBaseDir);

// Helper to expand environment variables
std::string expandEnvironmentVariables(const std::string& inputStr, const Agent& agentContext) {
    std::string result = inputStr;
    size_t pos = 0;

    // 1. Expand agent's environment variables first: ${VAR_NAME} or $VAR_NAME from agent.getEnvironmentVariables()
    const auto& agentEnvVars = agentContext.getEnvironmentVariables();
    for(const auto& pair : agentEnvVars) {
        std::string placeholder = "${" + pair.first + "}";
        size_t N = placeholder.length();
        for (pos = result.find(placeholder); pos != std::string::npos; pos = result.find(placeholder, pos)) {
            result.replace(pos, N, pair.second);
            pos += pair.second.length(); // Adjust pos to continue search after the replacement
        }
        // Simple $VAR version (less specific, might conflict with system env vars if names overlap)
        // For now, prioritizing ${VAR_NAME} for agent vars to be explicit.
    }


    // 2. Expand system environment variables: $SYS_VAR or ${SYS_VAR}
    pos = 0;
    while ((pos = result.find('$', pos)) != std::string::npos) {
        if (pos + 1 < result.length()) {
            size_t endPos;
            std::string varName;
            bool isBracketed = (result[pos + 1] == '{');

            if (isBracketed) { // ${SYS_VAR}
                endPos = result.find('}', pos + 2);
                if (endPos != std::string::npos) {
                    varName = result.substr(pos + 2, endPos - (pos + 2));
                } else {
                    pos += 2; continue; // Malformed
                }
            } else { // $SYS_VAR (simple variable name)
                endPos = pos + 1;
                while (endPos < result.length() && (std::isalnum(result[endPos]) || result[endPos] == '_')) {
                    endPos++;
                }
                varName = result.substr(pos + 1, endPos - (pos + 1));
                endPos--; // Adjust endPos to point to the last char of varName
            }

            if (!varName.empty()) {
                // Check if it's an agent variable first (already handled if using ${...})
                bool agentVarFound = false;
                if (!isBracketed) { // For $VAR_NAME style, check agent env again
                    for(const auto& pair : agentEnvVars) {
                        if (pair.first == varName) {
                            result.replace(pos, (endPos +1) - pos, pair.second);
                            pos += pair.second.length();
                            agentVarFound = true;
                            break;
                        }
                    }
                }

                if (!agentVarFound) {
                    const char* envVal = std::getenv(varName.c_str());
                    if (envVal) {
                        result.replace(pos, (endPos +1) - pos, envVal);
                        pos += strlen(envVal); 
                    } else {
                        logMessage(LogLevel::WARN, "Environment variable not found for expansion: " + varName, "Context: " + agentContext.getName());
                        result.replace(pos, (endPos +1) - pos, ""); 
                    }
                }
            } else {
                pos++; 
            }
        } else {
            break; 
        }
    }
    return result;
}


bool loadAgentProfile(Agent &agentToConfigure, const std::string &yamlPath) {
    logMessage(LogLevel::INFO, "Loading agent profile: " + yamlPath, "Agent: " + agentToConfigure.getName());
    YAML::Node config;
    fs::path agentYamlFsPath(yamlPath);
    fs::path agentYamlDir = agentYamlFsPath.parent_path();
    fs::path projectRootDir = fs::current_path(); 
    fs::path allowedScriptsBaseDir = projectRootDir / "config" / "scripts"; // More specific
    fs::path allowedToolImportBaseDir = projectRootDir / "config" / "tools";

    try {
        std::ifstream f(agentYamlFsPath.string()); 
        if (!f.good()) {
            logMessage(LogLevel::ERROR, "Agent profile file not found", yamlPath);
            return false;
        }
        config = YAML::Load(f); 
        f.close();

        if (config["name"] && config["name"].IsScalar()) {
            agentToConfigure.setName(config["name"].as<std::string>());
        } else {
            logMessage(LogLevel::WARN, "Agent profile missing 'name'. Using default or previous.", yamlPath);
        }
        
        if (config["description"] && config["description"].IsScalar()) {
            agentToConfigure.setDescription(expandEnvironmentVariables(config["description"].as<std::string>(), agentToConfigure));
        }
        if (config["system_prompt"] && config["system_prompt"].IsScalar()) {
            std::string systemPromptStr = config["system_prompt"].as<std::string>();
            if (systemPromptStr.size() > 3 && systemPromptStr.substr(systemPromptStr.size() - 3) == ".md") {
                fs::path promptFilePath = agentYamlDir / systemPromptStr;
                // logMessage(LogLevel::DEBUG, "Attempting to load system prompt from .md file", promptFilePath.string());
                
                std::error_code ec;
                promptFilePath = fs::weakly_canonical(promptFilePath, ec);
                 if (ec) {
                    logMessage(LogLevel::ERROR, "Error canonicalizing system prompt path: " + promptFilePath.string(), ec.message());
                } else {
                    std::ifstream promptFile(promptFilePath);
                    if (promptFile.good()) {
                        std::string content((std::istreambuf_iterator<char>(promptFile)), std::istreambuf_iterator<char>());
                        agentToConfigure.setSystemPrompt(expandEnvironmentVariables(content, agentToConfigure));
                    } else {
                        logMessage(LogLevel::ERROR, "System prompt file not found or not readable: " + promptFilePath.string());
                    }
                }
            } else {
                agentToConfigure.setSystemPrompt(expandEnvironmentVariables(systemPromptStr, agentToConfigure));
            }
        }
         if (config["schema"] && config["schema"].IsScalar()) {
            agentToConfigure.setSchema(expandEnvironmentVariables(config["schema"].as<std::string>(), agentToConfigure));
        }
        if (config["example"] && config["example"].IsScalar()) {
            agentToConfigure.setExample(expandEnvironmentVariables(config["example"].as<std::string>(), agentToConfigure));
        }
        if (config["iteration_cap"] && config["iteration_cap"].IsScalar()) {
            try {
                int cap = config["iteration_cap"].as<int>();
                agentToConfigure.setIterationCap(cap);
                 logMessage(LogLevel::DEBUG, "Agent '" + agentToConfigure.getName() + "' iteration_cap set to: " + std::to_string(cap));
            } catch (const YAML::BadConversion& e) { 
                 logMessage(LogLevel::WARN, "Failed to parse 'iteration_cap' for agent " + agentToConfigure.getName(), e.what());
            }
        }
        if (config["environment"] && config["environment"].IsMap()) {
            for (const auto& env_it : config["environment"]) {
                std::string key = env_it.first.as<std::string>();
                std::string value = expandEnvironmentVariables(env_it.second.as<std::string>(), agentToConfigure);
                agentToConfigure.addEnvironmentVariable(key, value);
            }
        }
         if (config["extra_prompts"] && config["extra_prompts"].IsSequence()) {
            for (const auto& item : config["extra_prompts"]) {
                if(item.IsScalar()) agentToConfigure.addExtraSystemPrompt(expandEnvironmentVariables(item.as<std::string>(), agentToConfigure));
            }
        }
        // Tasks are conceptual, not currently used for direct execution logic beyond prompting
        // if (config["tasks"] && config["tasks"].IsSequence()) {
        //     for (const auto& item : config["tasks"]) {
        //          if(item.IsScalar()) agentToConfigure.addTask(expandEnvironmentVariables(item.as<std::string>(), agentToConfigure));
        //     }
        // }
        if (config["directive"] && config["directive"].IsMap()) {
            Agent::AgentDirective directive;
            if (config["directive"]["type"] && config["directive"]["type"].IsScalar()){
                std::string typeStr = config["directive"]["type"].as<std::string>();
                if (typeStr == "BRAINSTORMING") directive.type = Agent::AgentDirective::Type::BRAINSTORMING;
                else if (typeStr == "AUTONOMOUS") directive.type = Agent::AgentDirective::Type::AUTONOMOUS;
                else if (typeStr == "EXECUTE") directive.type = Agent::AgentDirective::Type::EXECUTE;
                else if (typeStr == "REPORT") directive.type = Agent::AgentDirective::Type::REPORT;
                else directive.type = Agent::AgentDirective::Type::NORMAL; // Default
            }
            if (config["directive"]["description"] && config["directive"]["description"].IsScalar()){
                directive.description = expandEnvironmentVariables(config["directive"]["description"].as<std::string>(), agentToConfigure);
            }
            if (config["directive"]["format"] && config["directive"]["format"].IsScalar()){
                directive.format = expandEnvironmentVariables(config["directive"]["format"].as<std::string>(), agentToConfigure);
            }
            agentToConfigure.setDirective(directive);
        }


        std::map<std::string, Tool*> resolvedTools;

        if (config["import"] && config["import"].IsMap() &&
            config["import"]["tools"] && config["import"]["tools"].IsSequence()) {
            logMessage(LogLevel::DEBUG, "Agent '" + agentToConfigure.getName() + "': Processing tool imports...");
            for (const auto& importPathNode : config["import"]["tools"]) {
                if (importPathNode.IsScalar()) {
                    std::string relativeToolYamlPathStr = expandEnvironmentVariables(importPathNode.as<std::string>(), agentToConfigure);
                    fs::path fullToolYamlPath = agentYamlDir / relativeToolYamlPathStr; 
                    
                    std::error_code ec;
                    fullToolYamlPath = fs::weakly_canonical(fullToolYamlPath, ec);
                    if (ec) {
                        logMessage(LogLevel::ERROR, "Error canonicalizing tool import path: " + fullToolYamlPath.string(), ec.message());
                        continue;
                    }

                    if (!fs::exists(fullToolYamlPath)) {
                        logMessage(LogLevel::ERROR, "Agent '" + agentToConfigure.getName() + "': Tool import file not found: " + fullToolYamlPath.string() + ". Skipping import.");
                        continue;
                    }

                    std::map<std::string, Tool*> toolsFromFile = loadToolsFromFile(fullToolYamlPath.string(), agentToConfigure, fullToolYamlPath.parent_path());
                    for (auto const& [name, toolPtr] : toolsFromFile) {
                        if (resolvedTools.count(name)) {
                            logMessage(LogLevel::WARN, "Agent '" + agentToConfigure.getName() +
                                                       "': Tool '" + name + "' from '" + fullToolYamlPath.string() +
                                                       "' (import) is being overwritten by a subsequent import or inline definition.");
                            delete resolvedTools[name];
                        }
                        resolvedTools[name] = toolPtr;
                    }
                }
            }
        }

        if (config["tools"] && config["tools"].IsMap()) {
            logMessage(LogLevel::DEBUG, "Agent '" + agentToConfigure.getName() + "': Processing inline tools...");
            for (const auto& toolNodePair : config["tools"]) {
                std::string yamlToolKey = toolNodePair.first.as<std::string>(); 
                YAML::Node toolDef = toolNodePair.second;

                if (!toolDef.IsMap() || !toolDef["name"] || !toolDef["name"].IsScalar() ||
                    !toolDef["description"] || !toolDef["description"].IsScalar() ||
                    !toolDef["type"] || !toolDef["type"].IsScalar()) {
                    logMessage(LogLevel::WARN, "Agent '" + agentToConfigure.getName() +
                                               "': Skipping malformed inline tool definition under YAML key '" + yamlToolKey + "'. Missing required fields (name, description, type).");
                    continue;
                }

                std::string toolName = toolDef["name"].as<std::string>();
                std::string toolDescription = expandEnvironmentVariables(toolDef["description"].as<std::string>(), agentToConfigure);
                std::string toolType = toolDef["type"].as<std::string>();

                Tool* newTool = new Tool(toolName, toolDescription);
                FunctionalToolCallback callback = nullptr;

                if (toolType == "script") {
                    if (!toolDef["runtime"] || !toolDef["runtime"].IsScalar()) {
                        logMessage(LogLevel::WARN, "Agent '" + agentToConfigure.getName() + "': Inline script tool '" + toolName + "' missing 'runtime'. Skipping.");
                        delete newTool; continue;
                    }
                    std::string runtime = toolDef["runtime"].as<std::string>(); // Captured by lambda
                    std::string scriptSourceLocation;                           // Captured by lambda
                    bool isInline = false;                                      // Captured by lambda

                    if (toolDef["code"] && toolDef["code"].IsScalar()) {
                        scriptSourceLocation = expandEnvironmentVariables(toolDef["code"].as<std::string>(), agentToConfigure);
                        isInline = true;
                    } else if (toolDef["path"] && toolDef["path"].IsScalar()) {
                        std::string scriptPathStr = expandEnvironmentVariables(toolDef["path"].as<std::string>(), agentToConfigure);
                        fs::path scriptFullPath = agentYamlDir / scriptPathStr; 
                        
                        std::error_code ec;
                        scriptFullPath = fs::weakly_canonical(scriptFullPath, ec);
                        if (ec) {
                             logMessage(LogLevel::ERROR, "Error canonicalizing inline script path: " + scriptFullPath.string(), ec.message() + " for tool " + toolName);
                             delete newTool; continue;
                        }

                         if (!fs::exists(scriptFullPath)) { 
                            logMessage(LogLevel::ERROR, "Agent '" + agentToConfigure.getName() + "': Script file for inline tool '" + toolName + "' not found: " + scriptFullPath.string() + ". Skipping.");
                            delete newTool; continue;
                        }
                        scriptSourceLocation = scriptFullPath.string();
                        isInline = false; 
                    } else {
                        logMessage(LogLevel::WARN, "Agent '" + agentToConfigure.getName() + "': Inline script tool '" + toolName + "' missing 'path' or 'code'. Skipping.");
                        delete newTool; continue;
                    }
                    
                    // The callback captures necessary info to call executeScriptTool
                    callback = [runtime, scriptSourceLocation, isInline, toolName, agentName = agentToConfigure.getName()](const Json::Value& scriptParams) -> std::string {
                        // logMessage(LogLevel::DEBUG, "Agent '" + agentName + "' executing inline-defined script tool via lambda: " + toolName);
                        try {
                            // scriptParams are the parameters specifically for the *target script*,
                            // extracted by the LLM and placed in actionInfo.params.
                            return executeScriptTool(scriptSourceLocation, runtime, scriptParams, isInline);
                        } catch (const std::exception& e) {
                            logMessage(LogLevel::ERROR, "Exception in inline script tool '" + toolName + "' for agent '" + agentName + "'", e.what());
                            return "Error executing script '" + toolName + "': " + e.what();
                        }
                    };

                } else if (toolType == "internal_function") {
                    if (!toolDef["function_identifier"] || !toolDef["function_identifier"].IsScalar()) {
                        logMessage(LogLevel::WARN, "Agent '" + agentToConfigure.getName() + "': Inline internal function tool '" + toolName + "' missing 'function_identifier'. Skipping.");
                        delete newTool; continue;
                    }
                    std::string funcId = toolDef["function_identifier"].as<std::string>();
                    callback = ToolRegistry::getInstance().getFunction(funcId);
                    if (!callback) {
                        logMessage(LogLevel::ERROR, "Agent '" + agentToConfigure.getName() + "': Internal function '" + funcId + "' for inline tool '" + toolName + "' not found in registry. Skipping.");
                        delete newTool; continue;
                    }
                } else {
                    logMessage(LogLevel::WARN, "Agent '" + agentToConfigure.getName() + "': Unknown inline tool type '" + toolType + "' for tool '" + toolName + "'. Skipping.");
                    delete newTool; continue;
                }

                newTool->setCallback(callback);
                if (resolvedTools.count(toolName)) {
                    logMessage(LogLevel::WARN, "Agent '" + agentToConfigure.getName() + "': Inline tool '" + toolName + "' is overwriting an imported tool definition.");
                    delete resolvedTools[toolName];
                }
                resolvedTools[toolName] = newTool;
                 logMessage(LogLevel::DEBUG, "Agent '" + agentToConfigure.getName() + "': Loaded inline tool '" + toolName + "' with type '" + toolType + "'.");
            }
        }

        for (auto const& [name, toolPtr] : resolvedTools) {
            agentToConfigure.addTool(toolPtr); 
        }

        logMessage(LogLevel::INFO, "Successfully loaded agent profile: " + agentToConfigure.getName(), yamlPath);
        return true;

    } catch (const YAML::Exception& e) {
        logMessage(LogLevel::ERROR, "YAML parsing error in agent profile: " + yamlPath, e.what());
        return false;
    } catch (const fs::filesystem_error& e) {
        logMessage(LogLevel::ERROR, "Filesystem error loading agent profile: " + yamlPath, e.what());
        return false;
    } catch (const std::exception& e) {
        logMessage(LogLevel::ERROR, "Generic error loading agent profile: " + yamlPath, e.what());
        return false;
    }
}

std::map<std::string, Tool*> loadToolsFromFile(const std::string& toolYamlPath, Agent& agentForLoggingContext, const fs::path& toolFileBaseDir) {
    std::map<std::string, Tool*> loadedTools;
    YAML::Node toolsRootNode;
    fs::path projectRootDir = fs::current_path(); 
    fs::path allowedScriptsBaseDir = projectRootDir / "config" / "scripts"; 

    logMessage(LogLevel::DEBUG, "Agent '" + agentForLoggingContext.getName() + "': Importing tool definitions from: " + toolYamlPath);

    try {
        std::ifstream f(toolYamlPath);
        if (!f.good()) {
             logMessage(LogLevel::ERROR, "Agent '" + agentForLoggingContext.getName() + "': Tool definition file not found: " + toolYamlPath);
            return loadedTools;
        }
        toolsRootNode = YAML::Load(f);
        f.close();
    } catch (const YAML::Exception& e) {
        logMessage(LogLevel::ERROR, "Agent '" + agentForLoggingContext.getName() + "': Failed to parse tool YAML file: " + toolYamlPath, e.what());
        return loadedTools;
    }

    if (!toolsRootNode.IsMap()) {
        logMessage(LogLevel::ERROR, "Agent '" + agentForLoggingContext.getName() + "': Root of tool file '" + toolYamlPath + "' is not a map. Skipping.");
        return loadedTools;
    }

    for (const auto& categoryNodePair : toolsRootNode) {
        std::string categoryKey = categoryNodePair.first.as<std::string>();
        YAML::Node categoryToolsMap = categoryNodePair.second;

        if (!categoryToolsMap.IsMap()) {
            logMessage(LogLevel::WARN, "Agent '" + agentForLoggingContext.getName() + "': Expected a map of tools under category '" + categoryKey + "' in " + toolYamlPath + ". Skipping category.");
            continue;
        }

        for (const auto& toolNodePair : categoryToolsMap) {
            std::string yamlToolKey = toolNodePair.first.as<std::string>();
            YAML::Node toolDef = toolNodePair.second;

            if (!toolDef.IsMap() || !toolDef["name"] || !toolDef["name"].IsScalar() ||
                !toolDef["description"] || !toolDef["description"].IsScalar() ||
                !toolDef["type"] || !toolDef["type"].IsScalar()) {
                logMessage(LogLevel::WARN, "Agent '" + agentForLoggingContext.getName() +
                           "': Skipping malformed tool definition in '" + toolYamlPath + "' under YAML key '" + yamlToolKey + "'. Missing required fields (name, description, type).");
                continue;
            }

            std::string toolName = toolDef["name"].as<std::string>();
            std::string toolDescription = expandEnvironmentVariables(toolDef["description"].as<std::string>(), agentForLoggingContext);
            std::string toolType = toolDef["type"].as<std::string>();

            Tool* newTool = new Tool(toolName, toolDescription);
            FunctionalToolCallback callback = nullptr;

            if (toolType == "script") {
                if (!toolDef["runtime"] || !toolDef["runtime"].IsScalar()) {
                    logMessage(LogLevel::WARN, "Agent '" + agentForLoggingContext.getName() + "': Script tool '" + toolName + "' in '" + toolYamlPath + "' missing 'runtime'. Skipping.");
                    delete newTool; continue;
                }
                std::string runtime = toolDef["runtime"].as<std::string>(); // Captured
                std::string scriptSourceLocation;                           // Captured
                bool isInline = false;                                      // Captured

                if (toolDef["code"] && toolDef["code"].IsScalar()) {
                    scriptSourceLocation = expandEnvironmentVariables(toolDef["code"].as<std::string>(), agentForLoggingContext);
                    isInline = true;
                } else if (toolDef["path"] && toolDef["path"].IsScalar()) {
                    std::string scriptPathStr = expandEnvironmentVariables(toolDef["path"].as<std::string>(), agentForLoggingContext);
                    fs::path scriptFullPath = toolFileBaseDir / scriptPathStr; 
                    
                    std::error_code ec;
                    scriptFullPath = fs::weakly_canonical(scriptFullPath, ec);
                     if (ec) {
                        logMessage(LogLevel::ERROR, "Error canonicalizing script path from tool file: " + scriptFullPath.string(), ec.message() + " for tool " + toolName);
                        delete newTool; continue;
                    }

                    if (!fs::exists(scriptFullPath)) {
                        logMessage(LogLevel::ERROR, "Agent '" + agentForLoggingContext.getName() + "': Script file for tool '" + toolName + "' from '" + toolYamlPath + "' not found: " + scriptFullPath.string() + ". Skipping.");
                        delete newTool; continue;
                    }
                    scriptSourceLocation = scriptFullPath.string();
                    isInline = false;
                } else {
                    logMessage(LogLevel::WARN, "Agent '" + agentForLoggingContext.getName() + "': Script tool '" + toolName + "' in '" + toolYamlPath + "' missing 'path' or 'code'. Skipping.");
                    delete newTool; continue;
                }

                callback = [runtime, scriptSourceLocation, isInline, toolName, agentName = agentForLoggingContext.getName()](const Json::Value& scriptParams) -> std::string {
                    // logMessage(LogLevel::DEBUG, "Agent '" + agentName + "': Executing script tool '" + toolName + "' (defined in separate tool file).");
                    try {
                        return executeScriptTool(scriptSourceLocation, runtime, scriptParams, isInline);
                    } catch (const std::exception& e) {
                        logMessage(LogLevel::ERROR, "Agent '" + agentName + "': Exception in script tool '" + toolName + "' (defined in separate tool file).", e.what());
                        return "Error executing script '" + toolName + "': " + e.what();
                    }
                };

            } else if (toolType == "internal_function") {
                if (!toolDef["function_identifier"] || !toolDef["function_identifier"].IsScalar()) {
                    logMessage(LogLevel::WARN, "Agent '" + agentForLoggingContext.getName() + "': Internal function tool '" + toolName + "' in '" + toolYamlPath + "' missing 'function_identifier'. Skipping.");
                    delete newTool; continue;
                }
                std::string funcId = toolDef["function_identifier"].as<std::string>();
                callback = ToolRegistry::getInstance().getFunction(funcId);
                if (!callback) {
                    logMessage(LogLevel::ERROR, "Agent '" + agentForLoggingContext.getName() + "': Internal function '" + funcId + "' for tool '" + toolName + "' from '" + toolYamlPath + "' not found in registry. Skipping.");
                    delete newTool; continue;
                }
            } else {
                logMessage(LogLevel::WARN, "Agent '" + agentForLoggingContext.getName() + "': Unknown tool type '" + toolType + "' for tool '" + toolName + "' in '" + toolYamlPath + "'. Skipping.");
                delete newTool; continue;
            }

            newTool->setCallback(callback);
            if (loadedTools.count(toolName)) {
                logMessage(LogLevel::WARN, "Agent '" + agentForLoggingContext.getName() + "': Duplicate tool name '" + toolName + "' within the same tool definition file '" + toolYamlPath + "'. Overwriting.");
                delete loadedTools[toolName];
            }
            loadedTools[toolName] = newTool;
            logMessage(LogLevel::DEBUG, "Agent '" + agentForLoggingContext.getName() + "': Loaded tool '" + toolName + "' from file '" + toolYamlPath + "' with type '" + toolType + "'.");
        }
    }
    logMessage(LogLevel::INFO, "Agent '" + agentForLoggingContext.getName() + "': Finished importing " + std::to_string(loadedTools.size()) + " tool definitions from " + toolYamlPath);
    return loadedTools;
}
