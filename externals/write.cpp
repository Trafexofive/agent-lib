#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>     // For cerr, cout (debugging)
#include <filesystem>   // Requires C++17
#include <system_error> // For std::error_code
#include "json/json.h"  // Provided JSON library

namespace fs = std::filesystem;

// Tool Function: Writes content to a file specified by path.
// Input: JSON object like: {"path": "relative/path/to/file.txt", "content": "File content here"}
// Output: A success message or an error message string.
std::string writeFileTool(const Json::Value& params) {
    // 1. Parameter Validation
    if (params == Json::nullValue || !params.isObject()) {
        return "Error [writeFile]: Requires a JSON object with parameters.";
    }
    if (!params.isMember("path") || !params["path"].isString()) {
        return "Error [writeFile]: Missing or invalid required string parameter 'path'.";
    }
    std::string path_str = params["path"].asString();
    if (path_str.empty()) {
        return "Error [writeFile]: Parameter 'path' cannot be empty.";
    }

    // Content is optional, allow writing empty files
    std::string content = "";
    if (params.isMember("content")) {
         if (!params["content"].isString()) {
             return "Error [writeFile]: Optional parameter 'content' must be a string if provided.";
         }
         content = params["content"].asString();
    } else {
         std::cout << "[TOOL_DEBUG] writeFileTool: No 'content' provided for path '" << path_str << "'. Writing empty file." << std::endl;
    }


    // 2. Security Checks (Crucial!) - Reusing checks similar to file.cpp
    if (path_str.find("..") != std::string::npos) {
        return "Error [writeFile]: Invalid file path. Directory traversal ('..') is not allowed.";
    }
    fs::path filePath;
    try {
        filePath = path_str; // Convert to filesystem path
    } catch (const std::exception& e) {
         return "Error [writeFile]: Invalid path format '" + path_str + "': " + e.what();
    }

    if (filePath.is_absolute()) {
       return "Error [writeFile]: Invalid file path. Absolute paths are not allowed.";
    }
    // Basic check for potentially unsafe characters (adjust as needed)
    if (path_str.find_first_of("|;&`$<>\\") != std::string::npos) {
         return "Error [writeFile]: Invalid file path. Path contains potentially unsafe characters.";
    }

    std::cout << "[TOOL_DEBUG] writeFileTool attempting to write " << content.length() << " bytes to: '" << path_str << "'" << std::endl;

    // 3. Ensure parent directory exists and target is not a directory
    try {
        fs::path parentDir = filePath.parent_path();
        std::error_code ec;

        // Create parent directories if they don't exist
        if (!parentDir.empty() && !fs::exists(parentDir, ec)) {
             if (!fs::create_directories(parentDir, ec) && ec) { // Check error code after attempt
                  return "Error [writeFile]: Failed to create parent directory '" + parentDir.string() + "': " + ec.message();
             }
             std::cout << "[TOOL_DEBUG] writeFileTool: Created parent directory: " << parentDir.string() << std::endl;
        } else if (ec) {
             return "Error [writeFile]: Failed to check existence of parent directory '" + parentDir.string() + "': " + ec.message();
        }

         // Check if the target path itself exists and is a directory
         ec.clear(); // Reset error code
         if (fs::exists(filePath, ec) && !ec && fs::is_directory(filePath, ec) && !ec) {
             return "Error [writeFile]: Path '" + path_str + "' already exists and is a directory. Cannot overwrite a directory with a file.";
         } else if (ec) {
              // Non-critical error if checking the target path fails initially, file open will likely catch it too.
              std::cerr << "[WARN] writeFileTool: Could not definitively check target path status '" << path_str << "': " << ec.message() << std::endl;
         }

    } catch (const fs::filesystem_error& e) {
         return "Error [writeFile]: Filesystem error checking/creating directories for '" + path_str + "': " + e.what();
    } catch (const std::exception& e) {
         return "Error [writeFile]: Unexpected error during directory setup for '" + path_str + "': " + e.what();
    }


    // 4. Open file for writing (creates if not exists, truncates/overwrites if exists)
    // Use binary mode? For text, default is usually fine. Use std::ios::binary if needed.
    std::ofstream fileStream(filePath, std::ios::trunc);
    if (!fileStream.is_open()) {
        // Provide more context if possible
        return "Error [writeFile]: Could not open file for writing: '" + path_str + "'. Check path validity and permissions.";
    }

    // 5. Write content and handle errors
    try {
        fileStream << content;
        fileStream.flush(); // Ensure content is written to OS buffer

        if (fileStream.fail() || fileStream.bad()) {
            // Check stream state after writing/flushing
             std::string errorMsg = "Error [writeFile]: Failed to write content to file '" + path_str + "'. Disk full, permissions issue, or other I/O error?";
             fileStream.close(); // Attempt to close even on failure
             return errorMsg;
        }

    } catch (const std::ios_base::failure& e) { // Catch specific stream exceptions
        fileStream.close();
        return "Error [writeFile]: I/O exception while writing to file '" + path_str + "': " + e.what();
    } catch (const std::exception& e) {
        fileStream.close();
        return "Error [writeFile]: General exception while writing to file '" + path_str + "': " + e.what();
    } catch (...) {
        fileStream.close();
        return "Error [writeFile]: Unknown exception while writing to file '" + path_str + "'.";
    }

    fileStream.close(); // Close the file on success

    // Optional: Verify write by checking file size or existence again, but usually overkill
    std::cout << "[TOOL_DEBUG] writeFileTool successfully wrote to '" << path_str << "'" << std::endl;
    return "Success [writeFile]: Content successfully written to file '" + path_str + "'";
}
