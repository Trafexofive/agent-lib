// externals/sway_control.cpp
#pragma once

#include <string>
#include <cstdio>       // For popen, pclose, fgets
#include <sstream>
#include <stdexcept>
#include <sys/wait.h>   // For WEXITSTATUS
#include <iostream>     // For logging/debugging

#include "json/json.h"
// Assuming LogLevel enum and logMessage function are accessible globally or via includes
// If not, include the necessary header or pass a logger instance.
enum class LogLevel { DEBUG, INFO, WARN, ERROR }; // Placeholder if not included
void logMessage(LogLevel level, const std::string &message, const std::string &details = ""); // Placeholder

// Helper to execute command and capture output (similar to agent.cpp's version)
int executeSwayCommand(const std::string &sway_args, std::string &output) {
    output.clear();
    // Construct the full command
    std::string full_command = "swaymsg " + sway_args + " 2>&1"; // Redirect stderr to stdout
    logMessage(LogLevel::DEBUG, "[swayControlTool] Executing:", full_command);

    FILE *pipe = popen(full_command.c_str(), "r");
    if (!pipe) {
        logMessage(LogLevel::ERROR, "[swayControlTool] popen() failed for command:", full_command);
        output = "Error: Failed to execute swaymsg command (popen failed).";
        return -1; // Indicate pipe creation failure
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }

    int status = pclose(pipe);
    logMessage(LogLevel::DEBUG, "[swayControlTool] Command finished. Exit Status (raw):", std::to_string(status));

    if (status == -1) {
         logMessage(LogLevel::ERROR, "[swayControlTool] pclose() failed.");
         // Output might contain partial data, keep it.
         output += "\nError: pclose failed after running command.";
         return -1; // Indicate pclose failure
    } else {
        // Return the actual exit status of the swaymsg command
        return WEXITSTATUS(status);
    }
}


// Tool Function: Executes a swaymsg command.
// Input: JSON object like: {"command": "swaymsg arguments here"}
// Output: Success/error message string including swaymsg output.
std::string swayControlTool(const Json::Value& params) {
    // 1. Parameter Validation
    if (params == Json::nullValue || !params.isObject()) {
        return "Error [swayControlTool]: Requires a JSON object with parameters.";
    }
    if (!params.isMember("command") || !params["command"].isString()) {
        return "Error [swayControlTool]: Missing or invalid required string parameter 'command'.";
    }
    std::string swayArgs = params["command"].asString();
    if (swayArgs.empty()) {
        return "Error [swayControlTool]: Parameter 'command' cannot be empty.";
    }

    // Basic sanity check - avoid obviously dangerous characters if desired, though
    // swaymsg commands themselves can be complex. This is minimal.
    // if (swayArgs.find_first_of(";&|`$<>") != std::string::npos) {
    //     return "Error [swayControlTool]: Command contains potentially unsafe shell metacharacters.";
    // }

    // 2. Execute Command
    std::string output;
    int exitStatus = executeSwayCommand(swayArgs, output);

    // 3. Format Response
    std::stringstream result;
    result << "--- Sway Control Result ---\n";
    result << "Executed: swaymsg " << swayArgs << "\n";
    result << "Exit Status: " << exitStatus << "\n";

    if (exitStatus == 0) {
        result << "Status: Success\n";
        logMessage(LogLevel::INFO, "[swayControlTool] Command succeeded:", swayArgs);
    } else {
        result << "Status: Failed\n";
         logMessage(LogLevel::WARN, "[swayControlTool] Command failed:", "Command: swaymsg " + swayArgs + " | Exit Status: " + std::to_string(exitStatus));
    }

    result << "Output:\n" << (!output.empty() ? output : "(No output)");

    return result.str();
}

// Placeholder implementations if LogLevel/logMessage aren't globally available
// enum class LogLevel { DEBUG, INFO, WARN, ERROR };
// void logMessage(LogLevel level, const std::string &message, const std::string &details = "") {
//     const char* levelStr[] = {"DEBUG", "INFO", "WARN", "ERROR"};
//     std::cerr << "[" << levelStr[static_cast<int>(level)] << "] " << message;
//     if (!details.empty()) std::cerr << " | Details: " << details;
//     std::cerr << std::endl;
// }
