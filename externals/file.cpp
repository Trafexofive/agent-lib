// externals/file.cpp
#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>   // Requires C++17
#include <system_error> // For std::error_code
#include <iostream>     // For cerr/cout debugging/logging
#include <iomanip>      // For time formatting in 'info'
#include <chrono>       // For time formatting in 'info'
#include <cstdlib>      // For std::getenv (though no longer used for workspace)

#include "json/json.h" // Provided JSON library

namespace fs = std::filesystem;

// --- Helper Function ---
std::string formatFileTimeHelper(fs::file_time_type ftime); // Assuming implementation exists above

// --- Main Tool Function ---

// Tool Function: Performs file system operations (read, write, list, info, delete, mkdir).
// Input: JSON object like:
// {
//   "action": "read|write|list|info|delete|mkdir", // Required
//   "path": "path/to/file_or_dir",                 // Required (Relative or Absolute)
//   "content": "text content"                      // Required ONLY for "write" action
// }
// Output: Operation result (content, listing, info) or success/error message string.
//
// **************************************************************************
// * WARNING: THIS VERSION ALLOWS ABSOLUTE PATHS WITHOUT WORKSPACE CHECKS. *
// * Granting an AI agent unrestricted filesystem access via absolute paths *
// * carries significant security risks. Use with extreme caution and only *
// * in controlled environments where process permissions provide the      *
// * necessary boundaries.                                                *
// **************************************************************************
std::string fileTool(const Json::Value& params) {

    // 1. Parameter Validation (same as before)
    if (params == Json::nullValue || !params.isObject()) return "Error [fileTool]: Requires a JSON object.";
    if (!params.isMember("action") || !params["action"].isString()) return "Error [fileTool]: Missing/invalid string 'action'.";
    std::string action = params["action"].asString();
    if (!params.isMember("path") || !params["path"].isString()) return "Error [fileTool]: Missing/invalid string 'path'.";
    std::string path_str = params["path"].asString();
    if (path_str.empty()) return "Error [fileTool]: 'path' cannot be empty.";
    std::string content = "";
    if (action == "write") {
        if (!params.isMember("content") || !params["content"].isString()) return "Error [fileTool:write]: Missing/invalid string 'content'.";
        content = params["content"].asString();
    }

    // 2. Security Checks & Path Handling (Absolute Paths Allowed)
    fs::path targetPath;
    try {
        // Check for directory traversal attempts BEFORE creating fs::path
        // This remains important even with absolute paths allowed,
        // as ".." could still be used maliciously within relative segments
        // or combined with absolute paths in confusing ways.
        if (path_str.find("..") != std::string::npos) {
            // More nuanced check: Allow ".." only if normalization doesn't escape a known root?
            // For now, keeping the strict "no .." check is simplest, though potentially limiting
            // for some legitimate relative paths involving "..".
            // If allowing "..", robust canonicalization and comparison against CWD/roots is needed.
            // Let's keep the strict check for now as a minimal safety rail.
            logMessage(LogLevel::WARN, "[fileTool] Path contains '..', disallowing for safety.", path_str);
            return "Error [fileTool]: Invalid file path. Directory traversal ('..') is disallowed for safety, even with absolute paths enabled.";
        }

        targetPath = path_str; // Convert to filesystem path

        // Check for potentially unsafe characters (still relevant)
        if (path_str.find_first_of("|;&`$<>\\") != std::string::npos) {
             return "Error [fileTool]: Invalid file path. Path contains potentially unsafe characters.";
        }

        // Log whether path is absolute or relative
        if (targetPath.is_absolute()) {
            logMessage(LogLevel::DEBUG, "[fileTool] Processing ABSOLUTE path:", targetPath.string());
        } else {
            logMessage(LogLevel::DEBUG, "[fileTool] Processing RELATIVE path:", targetPath.string());
            // Optional: Log the fully resolved path for clarity
            // try {
            //     logMessage(LogLevel::DEBUG, "[fileTool] Relative path resolves to:", fs::absolute(targetPath).string());
            // } catch(...) { /* ignore errors resolving absolute path for logging */ }
        }

    } catch (const std::exception& e) {
        return "Error [fileTool]: Invalid path format or validation failed for '" + path_str + "': " + e.what();
    }

    logMessage(LogLevel::DEBUG, "[fileTool] Processing action '" + action + "' on path '" + targetPath.string() + "'");


    // 3. Action Dispatching (Logic remains the same as the previous robust version, using targetPath)
    try {
        std::error_code ec;

        // === READ ===
        if (action == "read") {
            if (!fs::exists(targetPath, ec)) return "Error [fileTool:read]: Path not found: '" + targetPath.string() + "'"; if(ec) return "Error [fileTool:read]: Cannot check existence: " + ec.message();
            if (!fs::is_regular_file(targetPath, ec)) return "Error [fileTool:read]: Path is not a regular file: '" + targetPath.string() + "'"; if(ec) return "Error [fileTool:read]: Cannot check type: " + ec.message();
            std::ifstream fileStream(targetPath);
            if (!fileStream.is_open()) return "Error [fileTool:read]: Could not open file '" + targetPath.string() + "' (permissions?).";
            std::stringstream buffer; buffer << fileStream.rdbuf();
            if (fileStream.bad()) { fileStream.close(); return "Error [fileTool:read]: I/O error reading file '" + targetPath.string() + "'."; }
            fileStream.close();
            return "Success [fileTool:read]: Content of '" + targetPath.string() + "':\n" + buffer.str();
        }

        // === WRITE ===
        else if (action == "write") {
            fs::path parentDir = targetPath.parent_path();
            // Check parent existence only if it's not the root path itself
            if (!parentDir.empty() && parentDir != targetPath) {
                 if (!fs::exists(parentDir, ec)) return "Error [fileTool:write]: Parent directory '" + parentDir.string() + "' does not exist."; if(ec) return "Error [fileTool:write]: Cannot check parent existence: " + ec.message();
                 if (!fs::is_directory(parentDir, ec)) return "Error [fileTool:write]: Parent path '" + parentDir.string() + "' is not a directory."; if(ec) return "Error [fileTool:write]: Cannot check parent type: " + ec.message();
            }
            if(fs::exists(targetPath, ec) && !ec && fs::is_directory(targetPath, ec) && !ec) return "Error [fileTool:write]: Path '" + targetPath.string() + "' exists and is a directory.";
            std::ofstream fileStream(targetPath, std::ios::trunc);
            if (!fileStream.is_open()) return "Error [fileTool:write]: Could not open file '" + targetPath.string() + "' for writing (permissions?).";
            fileStream << content; fileStream.flush();
            if (fileStream.fail() || fileStream.bad()) { fileStream.close(); return "Error [fileTool:write]: Failed writing content to '" + targetPath.string() + "' (I/O error?)."; }
            fileStream.close();
            return "Success [fileTool:write]: Content written to file '" + targetPath.string() + "'";
        }

        // === LIST ===
        else if (action == "list") {
            if (!fs::exists(targetPath, ec)) return "Error [fileTool:list]: Path not found: '" + targetPath.string() + "'"; if(ec) return "Error [fileTool:list]: Cannot check existence: " + ec.message();
            if (!fs::is_directory(targetPath, ec)) return "Error [fileTool:list]: Path is not a directory: '" + targetPath.string() + "'"; if(ec) return "Error [fileTool:list]: Cannot check type: " + ec.message();
            std::stringstream listing; listing << "Listing of directory '" << targetPath.string() << "':\n";
            int count = 0; ec.clear();
            for (const auto& entry : fs::directory_iterator(targetPath, fs::directory_options::skip_permission_denied, ec)) {
                  if (ec) { listing << "[Warning: Error reading entry: " << ec.message() << "]\n"; ec.clear(); continue; }
                 try {
                     std::string entryName = entry.path().filename().string(); listing << "- " << entryName;
                     std::error_code entry_ec; if (entry.is_directory(entry_ec) && !entry_ec) listing << "/"; else if (entry_ec) listing << " (type error)";
                 } catch (const std::exception& e) { listing << "- [Error accessing entry: " << e.what() << "]"; }
                 listing << "\n"; count++;
             }
              if (ec) listing << "[Stopped listing due to error: " << ec.message() << "]\n";
             if (count == 0 && !ec) listing << "(Directory is empty or inaccessible)\n";
             std::string result = listing.str(); if (!result.empty() && result.back() == '\n') result.pop_back();
             return result;
        }

        // === INFO ===
        else if (action == "info") {
             if (!fs::exists(targetPath, ec)) return "Error [fileTool:info]: Path not found: '" + targetPath.string() + "'"; if(ec) return "Error [fileTool:info]: Cannot check existence: " + ec.message();
             std::stringstream info; info << "Information for path '" << targetPath.string() << "':\n";
             auto status = fs::status(targetPath, ec); if (ec) return "Error [fileTool:info]: Could not get status: " + ec.message();
             info << "- Type: ";
             if (fs::is_regular_file(status)) info << "File"; else if (fs::is_directory(status)) info << "Directory"; else if (fs::is_symlink(status)) info << "Symbolic Link"; else info << "Other/Unknown"; info << "\n";
             if (fs::is_regular_file(status)) {
                 uintmax_t size = fs::file_size(targetPath, ec); if (ec) info << "- Size: N/A (Error: " << ec.message() << ")\n"; else info << "- Size: " << size << " bytes\n";
             }
             auto ftime = fs::last_write_time(targetPath, ec); if (ec) info << "- Last Modified: N/A (Error: " << ec.message() << ")\n"; else info << "- Last Modified: " << formatFileTimeHelper(ftime) << "\n";
             std::string result = info.str(); if (!result.empty() && result.back() == '\n') result.pop_back();
             return result;
        }

        // === DELETE ===
        else if (action == "delete") {
             if (!fs::exists(targetPath, ec)) return "Success [fileTool:delete]: Path '" + targetPath.string() + "' did not exist."; if(ec) return "Error [fileTool:delete]: Cannot check existence: " + ec.message();
             bool is_dir = fs::is_directory(targetPath, ec); if (ec) return "Error [fileTool:delete]: Cannot determine type: " + ec.message();
             if (is_dir) {
                bool empty = fs::is_empty(targetPath, ec); if (ec) return "Error [fileTool:delete]: Cannot check if directory empty: " + ec.message();
                if (!empty) return "Error [fileTool:delete]: Directory '" + targetPath.string() + "' is not empty.";
             }
             bool removed = fs::remove(targetPath, ec);
             if (ec) return "Error [fileTool:delete]: Failed to delete '" + targetPath.string() + "': " + ec.message();
             else if (!removed && fs::exists(targetPath)) return "Error [fileTool:delete]: Delete reported no error for '" + targetPath.string() + "', but path still exists.";
             else return "Success [fileTool:delete]: Path '" + targetPath.string() + "' deleted.";
        }

        // === MKDIR ===
        else if (action == "mkdir") {
            if (fs::exists(targetPath, ec)) return "Error [fileTool:mkdir]: Path '" + targetPath.string() + "' already exists."; if(ec) return "Error [fileTool:mkdir]: Cannot check existence: " + ec.message();
            fs::path parentDir = targetPath.parent_path();
            // Check parent existence only if it's not the root path itself
            if (!parentDir.empty() && parentDir != targetPath) {
                 if (!fs::exists(parentDir, ec)) return "Error [fileTool:mkdir]: Parent directory '" + parentDir.string() + "' does not exist."; if(ec) return "Error [fileTool:mkdir]: Cannot check parent existence: " + ec.message();
                 if (!fs::is_directory(parentDir, ec)) return "Error [fileTool:mkdir]: Parent path '" + parentDir.string() + "' is not a directory."; if(ec) return "Error [fileTool:mkdir]: Cannot check parent type: " + ec.message();
            }
             if (fs::create_directory(targetPath, ec)) return "Success [fileTool:mkdir]: Directory '" + targetPath.string() + "' created.";
             else return "Error [fileTool:mkdir]: Failed to create directory '" + targetPath.string() + "'" + (ec ? ": " + ec.message() : ".");
        }

        // === Unknown Action ===
        else {
            return "Error [fileTool]: Unknown action '" + action + "'.";
        }

    } catch (const fs::filesystem_error& e) {
        std::string path1_info = e.path1().empty() ? "" : " [Path1: '" + e.path1().string() + "']";
        std::string path2_info = e.path2().empty() ? "" : " [Path2: '" + e.path2().string() + "']";
        logMessage(LogLevel::ERROR, "[fileTool] Filesystem Error:", std::string(e.what()) + path1_info + path2_info + " | Code: " + std::to_string(e.code().value()));
        return "Error [fileTool:" + action + "] Filesystem error: " + std::string(e.what()) + path1_info + path2_info;
    } catch (const std::exception& e) {
        logMessage(LogLevel::ERROR, "[fileTool] Standard Exception:", e.what());
        return "Error [fileTool:" + action + "] Exception: " + std::string(e.what());
    } catch (...) {
        logMessage(LogLevel::ERROR, "[fileTool] Unknown internal error occurred.");
        return "Error [fileTool:" + action + "] Unknown internal error occurred.";
    }
}

// --- Helper Function Implementation ---
std::string formatFileTimeHelper(fs::file_time_type ftime) {
     try {
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
        );
        std::time_t ctime = std::chrono::system_clock::to_time_t(sctp);
        std::tm timeinfo = *std::localtime(&ctime); // Check thread safety if needed
        char time_buf[100];
        if (std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", &timeinfo)) {
            return std::string(time_buf);
        } else {
            return "(Error formatting time)";
        }
     } catch (const std::exception& e) {
         std::cerr << "[ERROR] fileTool time formatting exception: " << e.what() << std::endl;
         return "(Time conversion/formatting error)";
     }
}
