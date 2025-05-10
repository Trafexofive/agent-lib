#pragma once

#include <string>
#include <json/json.h> // For Json::Value

// Function to execute a simple command and get its output
int executeCommand(const std::string &command, std::string &output);

// Function to execute a script (inline or from file) with specified runtime and parameters
std::string executeScriptTool(const std::string& scriptPathOrContent,
                              const std::string& runtime,
                              const Json::Value& params,
                              bool isContentInline);
