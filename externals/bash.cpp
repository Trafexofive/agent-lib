#include <cstdio>
#include <string>
#include <sstream>
#include <stdexcept>
#include <array>
#include <iostream>
#include "json/json.h"
#include "../inc/Tool.hpp" // Include your Tool.hpp

std::string executeBashCommandReal(const Json::Value& params) {
    Json::Value response;
    // 1. Parameter Validation
    if (params == Json::nullValue || params.empty()) {
        response["status"] = "error";
        response["result"] = "Error: No parameters provided. Please provide a command parameter.";
        return response.toStyledString();
    }

    if (!params.isMember("command")) {
        response["status"] = "error";
        response["result"] = "Error: Missing required parameter 'command'.";
        return response.toStyledString();
    }

    if (!params["command"].isString()) {
        response["status"] = "error";
        response["result"] = "Error: Parameter 'command' must be a string.";
        return response.toStyledString();
    }

    std::string command = params["command"].asString();
    if (command.empty()) {
        response["status"] = "error";
        response["result"] = "Error: 'command' parameter cannot be empty.";
        return response.toStyledString();
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
        response["status"] = "error";
        response["result"] = "Error: popen() failed to execute command: " + command;
        return response.toStyledString();
    }

    // Read the output from the pipe line by line
    try {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data(); // Append directly to the string
        }
    } catch (const std::exception& e) { // Catch specific exceptions
        pclose(pipe);
        response["status"] = "error";
        response["result"] = "Error: Exception caught while reading command output: " + std::string(e.what());
        return response.toStyledString();
    } catch (...) { // Catch any other exceptions
        pclose(pipe);
        response["status"] = "error";
        response["result"] = "Error: Unknown exception caught while reading command output.";
        return response.toStyledString();
    }


    int exit_status = pclose(pipe);

    result += "
--- Exit Status: " + std::to_string(WEXITSTATUS(exit_status)) + " ---";

    response["status"] = "success";
    response["result"] = result;
    return response.toStyledString();
}
