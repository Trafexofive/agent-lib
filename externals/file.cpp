// externals/file.cpp
#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>   // Requires C++17
#include <chrono>       // For file times
#include <iomanip>      // For formatting time
#include <iostream>     // For cerr debug/errors if needed
#include <system_error> // For std::error_code

#include "json/json.h" // Provided JSON library

namespace fs = std::filesystem;

// --- Helper Function ---

// Helper to format file time (consistent with info action)
std::string formatFileTimeHelper(const fs::file_time_type& ftime) {
     try {
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
        std::time_t ctime = std::chrono::system_clock::to_time_t(sctp);
        char time_buf[100];
        if (std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S %Z", std::localtime(&ctime))) {
            return std::string(time_buf);
        } else {
            return "(Error formatting time)";
        }
     } catch (const std::exception& e) {
         return "(Time conversion/formatting error)";
     }
}


// --- Main Tool Function ---

// Tool Function: Performs file operations (read, write, list, info, delete, mkdir).
// Input: JSON object like:
// {
//   "action": "read|write|list|info|delete|mkdir", // Required
//   "path": "relative/path/to/file_or_dir",      // Required
//   "content": "text content"                   // Required only for "write" action
//   // "recursive": bool (optional, future extension for mkdir/delete)
// }
// Output: Operation result (content, listing, info) or success/error message string.
std::string fileTool(const Json::Value& params) {

    // 1. Parameter Validation
    if (params == Json::nullValue || !params.isObject()) {
        return "Error: fileTool requires a JSON object with parameters.";
    }
    if (!params.isMember("action") || !params["action"].isString()) {
        return "Error: Missing or invalid required string parameter 'action' (e.g., 'read', 'write', 'list', 'info', 'delete', 'mkdir').";
    }
    std::string action = params["action"].asString();

    if (!params.isMember("path") || !params["path"].isString()) {
        return "Error: Missing or invalid required string parameter 'path'.";
    }
    std::string path_str = params["path"].asString();
    if (path_str.empty()) {
        return "Error: Parameter 'path' cannot be empty.";
    }

    // 2. Security Checks (Crucial!)
    if (path_str.find("..") != std::string::npos) {
        return "Error: Invalid file path. Directory traversal ('..') is not allowed.";
    }
    fs::path filePath = path_str; // Convert to filesystem path *after* '..' check
    if (filePath.is_absolute()) {
       return "Error: Invalid file path. Absolute paths are not allowed.";
    }
    if (path_str.find_first_of("|;&`$<>\\") != std::string::npos) {
         return "Error: Invalid file path. Path contains potentially unsafe characters.";
    }

    // 3. Action Dispatching
    try {
        // === READ ===
        if (action == "read") {
            std::error_code exists_ec;
            if (!fs::exists(filePath, exists_ec) || exists_ec) {
                 return "Error [read]: Path not found or inaccessible: '" + path_str + "'" + (exists_ec ? " (" + exists_ec.message() + ")" : "");
            }
            std::error_code file_ec;
            if (!fs::is_regular_file(filePath, file_ec) || file_ec) {
                return "Error [read]: Path is not a regular file: '" + path_str + "'" + (file_ec ? " (" + file_ec.message() + ")" : "");
            }
            std::ifstream fileStream(filePath);
            if (!fileStream.is_open()) {
                return "Error [read]: Could not open file '" + path_str + "' for reading (check permissions).";
            }
            std::stringstream buffer;
            buffer << fileStream.rdbuf();
            fileStream.close();
            return "Content of '" + path_str + "':\n" + buffer.str();

        // === WRITE ===
        } else if (action == "write") {
            if (!params.isMember("content") || !params["content"].isString()) {
                return "Error [write]: Missing or invalid string parameter 'content'.";
            }
            std::string content = params["content"].asString();
            fs::path parentDir = filePath.parent_path();
            if (!parentDir.empty()) {
                 std::error_code parent_ec;
                 if (!fs::exists(parentDir, parent_ec) || parent_ec) {
                     return "Error [write]: Parent directory '" + parentDir.string() + "' does not exist or is inaccessible" + (parent_ec ? " (" + parent_ec.message() + ")" : "");
                 }
                 if (!fs::is_directory(parentDir, parent_ec) || parent_ec) {
                      return "Error [write]: Parent path '" + parentDir.string() + "' is not a directory" + (parent_ec ? " (" + parent_ec.message() + ")" : "");
                 }
            }
            // Ensure target path is not a directory itself
            std::error_code target_ec;
             if(fs::exists(filePath, target_ec) && !target_ec && fs::is_directory(filePath, target_ec) && !target_ec) {
                return "Error [write]: Path '" + path_str + "' already exists and is a directory. Cannot write file content to a directory.";
             }

            std::ofstream fileStream(filePath, std::ios::trunc);
            if (!fileStream.is_open()) {
                 return "Error [write]: Could not open file '" + path_str + "' for writing (check permissions or if path is a directory).";
            }
            fileStream << content;
            fileStream.flush();
            if (fileStream.fail() || fileStream.bad()) {
                fileStream.close();
                return "Error [write]: Failed writing content to '" + path_str + "' (disk full, permissions, or other I/O error?).";
            }
            fileStream.close();
            return "Success [write]: Content successfully written to file '" + path_str + "'";

        // === LIST ===
        } else if (action == "list") {
            std::error_code exists_ec;
             if (!fs::exists(filePath, exists_ec) || exists_ec) {
                 return "Error [list]: Path not found or inaccessible: '" + path_str + "'" + (exists_ec ? " (" + exists_ec.message() + ")" : "");
             }
             std::error_code dir_ec;
             if (!fs::is_directory(filePath, dir_ec) || dir_ec) {
                 return "Error [list]: Path is not a directory: '" + path_str + "'" + (dir_ec ? " (" + dir_ec.message() + ")" : "");
             }
             std::stringstream listing;
             listing << "Listing of directory '" << path_str << "':\n";
             int count = 0;
             std::error_code iter_ec;
             for (const auto& entry : fs::directory_iterator(filePath, fs::directory_options::skip_permission_denied, iter_ec)) {
                  if (iter_ec) {
                     listing << "[Warning: Error reading directory entry: " << iter_ec.message() << "]\n";
                     iter_ec.clear();
                     continue;
                  }
                 try {
                     listing << "- " << entry.path().filename().string();
                     if (entry.is_directory()) listing << "/";
                 } catch (const std::exception& e) {
                     listing << "- [Error accessing entry name/type: " << e.what() << "]";
                 }
                 listing << "\n";
                 count++;
             }
              if (iter_ec) {
                     listing << "[Stopped listing due to error: " << iter_ec.message() << "]\n";
              }
             if (count == 0 && !iter_ec) {
                 listing << "(Directory is empty or contains only inaccessible items)\n";
             }
             std::string result = listing.str();
             if (!result.empty() && result.back() == '\n') result.pop_back();
             return result;

        // === INFO ===
        } else if (action == "info") {
             std::error_code exists_ec;
             if (!fs::exists(filePath, exists_ec) || exists_ec) {
                 return "Error [info]: Path not found or inaccessible: '" + path_str + "'" + (exists_ec ? " (" + exists_ec.message() + ")" : "");
             }
             std::stringstream info;
             info << "Information for path '" << path_str << "':\n";
             std::error_code status_ec;
             auto status = fs::status(filePath, status_ec);
             if (status_ec) {
                 return "Error [info]: Could not get status for '" + path_str + "': " + status_ec.message();
             }
             info << "- Type: ";
             if (fs::is_regular_file(status)) info << "File";
             else if (fs::is_directory(status)) info << "Directory";
             else if (fs::is_symlink(status)) info << "Symbolic Link";
             else if (fs::is_block_file(status)) info << "Block Device";
             else if (fs::is_character_file(status)) info << "Character Device";
             else if (fs::is_fifo(status)) info << "FIFO/Pipe";
             else if (fs::is_socket(status)) info << "Socket";
             else info << "Other/Unknown";
             info << "\n";
             if (fs::is_regular_file(status)) {
                 std::error_code size_ec;
                 uintmax_t size = fs::file_size(filePath, size_ec);
                 if (size_ec) info << "- Size: N/A (Error: " << size_ec.message() << ")\n";
                 else info << "- Size: " << size << " bytes\n";
             }
             std::error_code time_ec;
             auto ftime = fs::last_write_time(filePath, time_ec);
             if (time_ec) info << "- Last Modified: N/A (Error: " << time_ec.message() << ")\n";
             else info << "- Last Modified: " << formatFileTimeHelper(ftime) << "\n";
             std::string result = info.str();
             if (!result.empty() && result.back() == '\n') result.pop_back();
             return result;

        // === DELETE ===
        } else if (action == "delete") {
             std::error_code exists_ec;
             if (!fs::exists(filePath, exists_ec)) {
                 return "Error [delete]: Path not found: '" + path_str + "'";
             }
             if(exists_ec) {
                  return "Error [delete]: Cannot access path '" + path_str + "' to check existence: " + exists_ec.message();
             }
             std::error_code type_ec;
             bool is_dir = fs::is_directory(filePath, type_ec);
             if (type_ec) return "Error [delete]: Cannot determine type of '" + path_str + "': " + type_ec.message();

             if (is_dir) {
                std::error_code empty_ec;
                bool empty = fs::is_empty(filePath, empty_ec);
                 if (empty_ec) {
                    return "Error [delete]: Cannot check if directory '" + path_str + "' is empty: " + empty_ec.message();
                 }
                 if (!empty) {
                      // *** More Specific Error Message ***
                      return "Error [delete]: Directory '" + path_str + "' is not empty. This tool can only delete files or *empty* directories. Recursive deletion is not supported.";
                 }
             }
             std::error_code remove_ec;
             bool removed = fs::remove(filePath, remove_ec);
             if (remove_ec) {
                 return "Error [delete]: Failed to delete '" + path_str + "': " + remove_ec.message();
             } else if (!removed) {
                 return "Error [delete]: Attempted to delete '" + path_str + "' but remove operation failed without error code (check permissions/locking?).";
             } else {
                  std::error_code final_check_ec;
                  if (!fs::exists(filePath, final_check_ec) && !final_check_ec) {
                     return "Success [delete]: Path '" + path_str + "' successfully deleted.";
                  } else {
                     return "Error [delete]: Delete command successful for '" + path_str + "' but it still seems to exist (potential filesystem inconsistency?).";
                  }
             }

        // === MKDIR === (New Action)
        } else if (action == "mkdir") {
            std::error_code exists_ec;
            if (fs::exists(filePath, exists_ec) && !exists_ec) {
                 return "Error [mkdir]: Path '" + path_str + "' already exists.";
            } else if (exists_ec) {
                 return "Error [mkdir]: Cannot check existence of path '" + path_str + "': " + exists_ec.message();
            }

            // Check parent directory exists and is a directory
             fs::path parentDir = filePath.parent_path();
            if (!parentDir.empty()) {
                 std::error_code parent_ec;
                 if (!fs::exists(parentDir, parent_ec) || parent_ec) {
                     return "Error [mkdir]: Parent directory '" + parentDir.string() + "' does not exist or is inaccessible" + (parent_ec ? " (" + parent_ec.message() + ")" : "");
                 }
                 if (!fs::is_directory(parentDir, parent_ec) || parent_ec) {
                      return "Error [mkdir]: Parent path '" + parentDir.string() + "' is not a directory" + (parent_ec ? " (" + parent_ec.message() + ")" : "");
                 }
            }
             // Use fs::create_directory (does not create parent dirs automatically)
             // Use fs::create_directories for recursive creation if desired, but be cautious.
             std::error_code create_ec;
             if (fs::create_directory(filePath, create_ec)) {
                 return "Success [mkdir]: Directory '" + path_str + "' created.";
             } else {
                 return "Error [mkdir]: Failed to create directory '" + path_str + "': " + create_ec.message();
             }

        // === Unknown Action ===
        } else {
            return "Error: Unknown fileTool action '" + action + "'. Supported actions: 'read', 'write', 'list', 'info', 'delete', 'mkdir'.";
        }

    } catch (const fs::filesystem_error& e) {
        std::string path1_info = e.path1().empty() ? "" : " [Path1: '" + e.path1().string() + "']";
        std::string path2_info = e.path2().empty() ? "" : " [Path2: '" + e.path2().string() + "']";
        return "Error (filesystem): " + std::string(e.what()) + path1_info + path2_info;
    } catch (const std::exception& e) {
        return "Error (standard exception): " + std::string(e.what());
    } catch (...) {
        return "Unknown internal error occurred during file operation.";
    }
}
