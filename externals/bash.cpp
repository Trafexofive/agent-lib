#include <cstdio>
#include <string>
#include <sstream>
#include <stdexcept>
#include <array>
#include <iostream>
#include "json/json.h"
#include "../inc/Tool.hpp" // Include your Tool.hpp


std::string executeBashCommandReal(const Json::Value& params) {
    // 1. Parameter Validation
    if (params == Json::nullValue || params.empty()) {
        return "Error: No parameters provided. Please provide a command parameter.";
    }
    if (!params.isMember("command")) {
        return "Error: Missing required parameter 'command'.";
    }
    if (!params["command"].isString()) {
        return "Error: Parameter 'command' must be a string.";
    }

    std::string command = params["command"].asString();
    if (command.empty()) {
        return "Error: 'command' parameter cannot be empty.";
    }

    std::cout << "Executing command: " << command << std::endl;

    // --- Command Execution Logic ---
    std::string full_command = command + " 2>&1"; // Redirect stderr to stdout
    std::array<char, 128> buffer;
    std::string result; // Use string directly for efficiency
    FILE* pipe = nullptr;

    // Use popen to execute the command and open a pipe to read its output
    pipe = popen(full_command.c_str(), "r");
    if (!pipe) {
        return "Error: popen() failed to execute command: " + command;
    }

    // Read the output from the pipe line by line
    try {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data(); // Append directly to the string
        }
    } catch (const std::exception& e) { // Catch specific exceptions
        pclose(pipe);
        return "Error: Exception caught while reading command output: " + std::string(e.what());
    } catch (...) { // Catch any other exceptions
        pclose(pipe);
        return "Error: Unknown exception caught while reading command output.";
    }


    int exit_status = pclose(pipe);


    result += "\n--- Exit Status: " + std::to_string(WEXITSTATUS(exit_status)) + " ---";

    return result;
}
