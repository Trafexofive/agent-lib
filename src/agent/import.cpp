#include "../../inc/Agent.hpp"
#include "../../inc/Tool.hpp"
#include "../../inc/Utils.hpp"       // For executeScriptTool
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

// Helper to expand environment variables like ${VAR_NAME} or $VAR_NAME
// (Simplified version, a more robust one might be needed for complex cases)
std::string expandEnvironmentVariables(const std::string& inputStr) {
    std::string result = inputStr;
    size_t pos = 0;
    while ((pos = result.find('$', pos)) != std::string::npos) {
        if (pos + 1 < result.length()) {
            size_t endPos;
            std::string varName;
            if (result[pos + 1] == '{') { // ${VAR_NAME}
                endPos = result.find('}', pos + 2);
                if (endPos != std::string::npos) {
                    varName = result.substr(pos + 2, endPos - (pos + 2));
                } else {
                    pos += 2; continue; // Malformed
                }
            } else { // $VAR_NAME (simple variable name)
                endPos = pos + 1;
                while (endPos < result.length() && (std::isalnum(result[endPos]) || result[endPos] == '_')) {
                    endPos++;
                }
                varName = result.substr(pos + 1, endPos - (pos + 1));
                endPos--; // Adjust endPos to point to the last char of varName
            }

            if (!varName.empty()) {
                const char* envVal = std::getenv(varName.c_str());
                if (envVal) {
                    result.replace(pos, (endPos +1) - pos, envVal);
                    pos += strlen(envVal); // Move past the replaced string
                } else {
                    logMessage(LogLevel::WARN, "Environment variable not found for expansion: " + varName);
                    result.replace(pos, (endPos +1) - pos, ""); // Replace with empty if not found
                    // Or keep placeholder: pos = endPos + 1;
                }
            } else {
                pos++; // Just a '$', not a variable
            }
        } else {
            break; // '$' at the end of the string
        }
    }
    return result;
}


// Basic path safety check: ensures targetPath is within or at baseDir
// and does not use ".." to escape upwards significantly.
// THIS IS A SIMPLIFIED CHECK AND SHOULD BE MADE MORE ROBUST FOR PRODUCTION.
// bool isPathConsideredSafe(const fs::path& targetPath, const fs::path& baseDir, const std::string& contextAgentName, const std::string& toolName) {
//     fs::path canonicalTarget;
//     fs::path canonicalBase;
//     std::error_code ec;
//
//     // Canonicalize paths to resolve symlinks and ".."
//     canonicalTarget = fs::weakly_canonical(targetPath, ec);
//     if (ec) {
//         logMessage(LogLevel::ERROR, "Agent '" + contextAgentName + "', Tool '" + toolName + "': Error canonicalizing target path", targetPath.string() + " - " + ec.message());
//         return false; // Cannot determine safety if path is invalid
//     }
//     canonicalBase = fs::weakly_canonical(baseDir, ec);
//      if (ec) {
//         logMessage(LogLevel::ERROR, "Agent '" + contextAgentName + "', Tool '" + toolName + "': Error canonicalizing base path", baseDir.string() + " - " + ec.message());
//         return false;
//     }
//
//     // Check if the canonical target path starts with the canonical base path
//     std::string targetStr = canonicalTarget.string();
//     std::string baseStr = canonicalBase.string();
//
//     if (targetStr.rfind(baseStr, 0) == 0) { // starts_with check
//         // Further check to ensure it's not just the base, but within a sub-directory
//         // or if it's intended to be a script directly in the base (e.g. baseDir/script.sh)
//         // This part depends on your desired strictness.
//         // For now, starting with base is considered "within".
//         // A more robust check would analyze path components to disallow "baseDir/../../something_else"
//         // even if `weakly_canonical` handled some of it.
//         return true;
//     }
//
//     logMessage(LogLevel::ERROR, "Agent '" + contextAgentName + "', Tool '" + toolName + "': SECURITY VIOLATION - Path attempts to escape allowed directory.",
//                "Base: " + baseDir.string() + ", Target: " + targetPath.string() + ", ResolvedTarget: " + canonicalTarget.string());
//     return false;
// }


bool loadAgentProfile(Agent &agentToConfigure, const std::string &yamlPath) {
    logMessage(LogLevel::INFO, "Loading agent profile: " + yamlPath);
    YAML::Node config;
    fs::path agentYamlFsPath(yamlPath);
    fs::path agentYamlDir = agentYamlFsPath.parent_path();

    // Define allowed root directories for script execution and tool imports (relative to project root or absolute)
    // THIS IS A CRITICAL SECURITY CONFIGURATION POINT.
    // These paths should be canonicalized at application startup.
    const fs::path projectRootDir = fs::current_path(); // Or a more fixed anchor
    const fs::path allowedScriptsBaseDir = projectRootDir / "config"; // Example: scripts must be under agent-lib/config/
    const fs::path allowedToolImportBaseDir = projectRootDir / "config/tools"; // Example

    try {
        std::ifstream f(yamlPath); // Use fs::path directly
        if (!f.good()) {
            logMessage(LogLevel::ERROR, "Agent profile file not found", yamlPath);
            return false;
        }
        config = YAML::Load(f); // Load from stream
        f.close();

        // --- Load Core Identity & Configuration ---
        if (config["name"] && config["name"].IsScalar()) {
            agentToConfigure.setName(config["name"].as<std::string>());
        } else {
            logMessage(LogLevel::WARN, "Agent profile missing 'name'. Using default or previous.", yamlPath);
        }
        // ... (load description, system_prompt (with .md file loading), schema, example, iteration_cap as before) ...
        if (config["description"] && config["description"].IsScalar()) {
            agentToConfigure.setDescription(expandEnvironmentVariables(config["description"].as<std::string>()));
        }
        if (config["system_prompt"] && config["system_prompt"].IsScalar()) {
            std::string systemPromptStr = config["system_prompt"].as<std::string>();
            if (systemPromptStr.size() > 3 && systemPromptStr.substr(systemPromptStr.size() - 3) == ".md") {
                fs::path promptFilePath = agentYamlDir / systemPromptStr;
                promptFilePath = fs::weakly_canonical(promptFilePath);
                // SAFETY CHECK for prompt file path (e.g., ensure it's within config/sysprompts)
                 // if (!isPathConsideredSafe(promptFilePath, projectRootDir / "config", agentToConfigure.getName(), "system_prompt_file")) {
                 //     logMessage(LogLevel::ERROR, "System prompt file path is unsafe: " + promptFilePath.string() + ". Skipping.");
                 // } else {
                    std::ifstream promptFile(promptFilePath);
                    if (promptFile.good()) {
                        std::string content((std::istreambuf_iterator<char>(promptFile)), std::istreambuf_iterator<char>());
                        agentToConfigure.setSystemPrompt(expandEnvironmentVariables(content));
                    } else {
                        logMessage(LogLevel::ERROR, "System prompt file not found or not readable: " + promptFilePath.string());
                    }
                 // }
            } else {
                agentToConfigure.setSystemPrompt(expandEnvironmentVariables(systemPromptStr));
            }
        }
         if (config["schema"] && config["schema"].IsScalar()) {
            agentToConfigure.setSchema(expandEnvironmentVariables(config["schema"].as<std::string>()));
        }
        if (config["example"] && config["example"].IsScalar()) {
            agentToConfigure.setExample(expandEnvironmentVariables(config["example"].as<std::string>()));
        }
        if (config["iteration_cap"] && config["iteration_cap"].IsScalar()) {
            try {
                agentToConfigure.setIterationCap(config["iteration_cap"].as<int>());
            } catch (const YAML::BadConversion& e) { /* ... log ... */ }
        }
        if (config["environment"] && config["environment"].IsMap()) {
            for (const auto& env_it : config["environment"]) {
                agentToConfigure.addEnvironmentVariable(env_it.first.as<std::string>(), expandEnvironmentVariables(env_it.second.as<std::string>()));
            }
        }
         if (config["extra_prompts"] && config["extra_prompts"].IsSequence()) {
            for (const auto& item : config["extra_prompts"]) {
                if(item.IsScalar()) agentToConfigure.addExtraSystemPrompt(expandEnvironmentVariables(item.as<std::string>()));
            }
        }
        if (config["tasks"] && config["tasks"].IsSequence()) {
            for (const auto& item : config["tasks"]) {
                 if(item.IsScalar()) agentToConfigure.addTask(expandEnvironmentVariables(item.as<std::string>()));
            }
        }
        if (config["directive"] && config["directive"].IsMap()) {
            Agent::AgentDirective directive;
            // ... (load directive fields, expandEnvironmentVariables for strings) ...
            agentToConfigure.setDirective(directive);
        }


        std::map<std::string, Tool*> resolvedTools;

        // --- 1. Import Tools ---
        if (config["import"] && config["import"].IsMap() &&
            config["import"]["tools"] && config["import"]["tools"].IsSequence()) {
            logMessage(LogLevel::DEBUG, "Agent '" + agentToConfigure.getName() + "': Processing tool imports...");
            for (const auto& importPathNode : config["import"]["tools"]) {
                if (importPathNode.IsScalar()) {
                    std::string relativeToolYamlPathStr = expandEnvironmentVariables(importPathNode.as<std::string>());
                    fs::path fullToolYamlPath = agentYamlDir / relativeToolYamlPathStr; // Path relative to agent's YAML
                    fullToolYamlPath = fs::weakly_canonical(fullToolYamlPath);

                    // **SECURITY CHECK for imported tool definition file path**
                    // if (!isPathConsideredSafe(fullToolYamlPath, allowedToolImportBaseDir, agentToConfigure.getName(), "tool_import:" + relativeToolYamlPathStr)) {
                    //     logMessage(LogLevel::ERROR, "Agent '" + agentToConfigure.getName() + "': Tool import path '" + fullToolYamlPath.string() + "' is outside allowed tool directories. Skipping import.");
                    //     continue;
                    // }
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

        // --- 2. Load Inline Tools (Overrides imported tools with the same 'name') ---
        if (config["tools"] && config["tools"].IsMap()) {
            logMessage(LogLevel::DEBUG, "Agent '" + agentToConfigure.getName() + "': Processing inline tools...");
            for (const auto& toolNodePair : config["tools"]) {
                std::string yamlToolKey = toolNodePair.first.as<std::string>(); // e.g., "BashExecutorConfig"
                YAML::Node toolDef = toolNodePair.second;

                if (!toolDef.IsMap() || !toolDef["name"] || !toolDef["name"].IsScalar() ||
                    !toolDef["description"] || !toolDef["description"].IsScalar() ||
                    !toolDef["type"] || !toolDef["type"].IsScalar()) {
                    logMessage(LogLevel::WARN, "Agent '" + agentToConfigure.getName() +
                                               "': Skipping malformed inline tool definition under YAML key '" + yamlToolKey + "'.");
                    continue;
                }

                std::string toolName = toolDef["name"].as<std::string>();
                std::string toolDescription = expandEnvironmentVariables(toolDef["description"].as<std::string>());
                std::string toolType = toolDef["type"].as<std::string>();

                Tool* newTool = new Tool(toolName, toolDescription);
                FunctionalToolCallback callback = nullptr;

                if (toolType == "script") {
                    if (!toolDef["runtime"] || !toolDef["runtime"].IsScalar()) {
                        logMessage(LogLevel::WARN, "Agent '" + agentToConfigure.getName() + "': Inline script tool '" + toolName + "' missing 'runtime'. Skipping.");
                        delete newTool; continue;
                    }
                    std::string runtime = toolDef["runtime"].as<std::string>();
                    std::string scriptSourceLocation;
                    bool isInline = false;

                    if (toolDef["code"] && toolDef["code"].IsScalar()) {
                        scriptSourceLocation = expandEnvironmentVariables(toolDef["code"].as<std::string>());
                        isInline = true;
                    } else if (toolDef["path"] && toolDef["path"].IsScalar()) {
                        std::string scriptPathStr = expandEnvironmentVariables(toolDef["path"].as<std::string>());
                        fs::path scriptFullPath = agentYamlDir / scriptPathStr; // Relative to agent's YAML file
                        scriptFullPath = fs::weakly_canonical(scriptFullPath);

                        // **SECURITY CHECK for inline script path**
                        // if (!isPathConsideredSafe(scriptFullPath, allowedScriptsBaseDir, agentToConfigure.getName(), toolName + "(inline_script_path)")) {
                        //      logMessage(LogLevel::ERROR, "Agent '" + agentToConfigure.getName() + "': Inline script tool path '" + scriptFullPath.string() + "' is outside allowed script directories. Skipping tool '" + toolName + "'.");
                        //      delete newTool; continue;
                        // }
                         if (!fs::exists(scriptFullPath) && !isInline) { // Check existence only if not inline code
                            logMessage(LogLevel::ERROR, "Agent '" + agentToConfigure.getName() + "': Script file for inline tool '" + toolName + "' not found: " + scriptFullPath.string() + ". Skipping.");
                            delete newTool; continue;
                        }

                        scriptSourceLocation = scriptFullPath.string();
                        isInline = false; // It's a path, not inline code
                    } else {
                        logMessage(LogLevel::WARN, "Agent '" + agentToConfigure.getName() + "': Inline script tool '" + toolName + "' missing 'path' or 'code'. Skipping.");
                        delete newTool; continue;
                    }

                    callback = [runtime, scriptSourceLocation, isInline, toolName](const Json::Value& params) -> std::string {
                        // logMessage(LogLevel::DEBUG, "Executing inline-defined script tool via lambda: " + toolName);
                        try {
                            return executeScriptTool(scriptSourceLocation, runtime, params, isInline);
                        } catch (const std::exception& e) {
                            logMessage(LogLevel::ERROR, "Exception in inline script tool '" + toolName + "'", e.what());
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
            }
        }

        // --- 3. Add all resolved tools to the agent ---
        for (auto const& [name, toolPtr] : resolvedTools) {
            agentToConfigure.addTool(toolPtr); // Agent takes ownership
        }

        // ... (load extra_prompts, tasks, directive as before, applying expandEnvironmentVariables to string fields) ...

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

// Parses a dedicated tool definition YAML file.
// toolFileBaseDir is the directory of the tool.yaml file, used for resolving its relative script paths.
std::map<std::string, Tool*> loadToolsFromFile(const std::string& toolYamlPath, Agent& agentForLoggingContext, const fs::path& toolFileBaseDir) {
    std::map<std::string, Tool*> loadedTools;
    YAML::Node toolsRootNode;
    fs::path projectRootDir = fs::current_path(); // Or a more fixed anchor for script safety checks
    fs::path allowedScriptsBaseDir = projectRootDir / "config/scripts"; // Example global safe script dir

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
                           "': Skipping malformed tool definition in '" + toolYamlPath + "' under YAML key '" + yamlToolKey + "'.");
                continue;
            }

            std::string toolName = toolDef["name"].as<std::string>();
            std::string toolDescription = expandEnvironmentVariables(toolDef["description"].as<std::string>());
            std::string toolType = toolDef["type"].as<std::string>();

            Tool* newTool = new Tool(toolName, toolDescription);
            FunctionalToolCallback callback = nullptr;

            if (toolType == "script") {
                if (!toolDef["runtime"] || !toolDef["runtime"].IsScalar()) {
                    logMessage(LogLevel::WARN, "Agent '" + agentForLoggingContext.getName() + "': Script tool '" + toolName + "' in '" + toolYamlPath + "' missing 'runtime'. Skipping.");
                    delete newTool; continue;
                }
                std::string runtime = toolDef["runtime"].as<std::string>();
                std::string scriptSourceLocation;
                bool isInline = false;

                if (toolDef["code"] && toolDef["code"].IsScalar()) {
                    scriptSourceLocation = expandEnvironmentVariables(toolDef["code"].as<std::string>());
                    isInline = true;
                } else if (toolDef["path"] && toolDef["path"].IsScalar()) {
                    std::string scriptPathStr = expandEnvironmentVariables(toolDef["path"].as<std::string>());
                    fs::path scriptFullPath = toolFileBaseDir / scriptPathStr; // Path relative to the *tool definition file*
                    scriptFullPath = fs::weakly_canonical(scriptFullPath);

                    // **SECURITY CHECK for script path from tool file**
                    // if (!isPathConsideredSafe(scriptFullPath, allowedScriptsBaseDir, agentForLoggingContext.getName(), toolName + "(script_from_tool_file)")) {
                    //      logMessage(LogLevel::ERROR, "Agent '" + agentForLoggingContext.getName() + "': Script path for tool '" + toolName + "' from '" + toolYamlPath + "' (" + scriptFullPath.string() + ") is outside allowed script directories. Skipping tool.");
                    //      delete newTool; continue;
                    // }
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

                callback = [runtime, scriptSourceLocation, isInline, toolName, agentName = agentForLoggingContext.getName()](const Json::Value& params) -> std::string {
                    // logMessage(LogLevel::DEBUG, "Agent '" + agentName + "': Executing script tool '" + toolName + "' (defined in separate tool file).");
                    try {
                        return executeScriptTool(scriptSourceLocation, runtime, params, isInline);
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
        }
    }
    logMessage(LogLevel::INFO, "Agent '" + agentForLoggingContext.getName() + "': Finished importing " + std::to_string(loadedTools.size()) + " tool definitions from " + toolYamlPath);
    return loadedTools;
}
