#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream> // For potential logging/debugging

// Tool Function: Writes content to a file specified by the path on the first line of input.
// Input: A string where the first line is the file path, and subsequent lines are the content.
// Output: A success message or an error message.
std::string writeFileTool(const std::string& inputData) {
    std::string filePath;
    std::string content;

    size_t firstNewline = inputData.find('\n');

    if (firstNewline == std::string::npos) {
        // Assume input is just the path, write empty content
        filePath = inputData;
        content = "";
        // Alternative: return "Error: writeFileTool input format error. Requires path on first line and content after newline.";
    } else {
        filePath = inputData.substr(0, firstNewline);
        content = inputData.substr(firstNewline + 1); // Content is everything after the first newline
    }

    // Trim path
    filePath.erase(0, filePath.find_first_not_of(" \t\r\n"));
    filePath.erase(filePath.find_last_not_of(" \t\r\n") + 1);

    if (filePath.empty()) {
        return "Error: writeFileTool requires a non-empty file path on the first line.";
    }

    // Security check (basic example - NEEDS TO BE MUCH MORE ROBUST)
    if (filePath.find("..") != std::string::npos || filePath[0] == '/') {
        return "Error: Invalid or potentially unsafe file path specified for writeFileTool.";
    }

    std::cout << "[TOOL_DEBUG] writeFileTool attempting to write " << content.length() << " bytes to: '" << filePath << "'" << std::endl;

    // Open file for writing (creates if not exists, truncates/overwrites if exists)
    std::ofstream fileStream(filePath, std::ios::trunc);
    if (!fileStream.is_open()) {
        return "Error: Could not open file for writing: '" + filePath + "'. Check path and permissions.";
    }

    try {
        fileStream << content; // Write the content
        fileStream.flush(); // Ensure content is written to OS buffer
    } catch (const std::exception& e) {
        fileStream.close();
        return "Error: Exception while writing to file '" + filePath + "': " + e.what();
    } catch (...) {
        fileStream.close();
        return "Error: Unknown exception while writing to file '" + filePath + "'.";
    }


    if (fileStream.fail() || fileStream.bad()) { // Check stream state after writing/flushing
         std::string errorMsg = "Error: Failed to write content to file '" + filePath + "'. Disk full or permissions issue?";
         fileStream.close(); // Attempt to close even on failure
         return errorMsg;
    }

    fileStream.close(); // Close the file

    std::cout << "[TOOL_DEBUG] writeFileTool successfully wrote to '" << filePath << "'" << std::endl;
    return "Success: Content written to file: '" + filePath + "'";
}
