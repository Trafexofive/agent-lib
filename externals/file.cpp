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

// --- Main Tool Function (Returns JSON String) ---
std::string fileTool(const Json::Value& params) {
    Json::Value response; // JSON object for the response

    // 1. Parameter Validation
    if (params == Json::nullValue || !params.isObject()) {
        response["status"] = "error";
        response["result"] = "Error: fileTool requires a JSON object with parameters.";
        return response.toStyledString();
    }
    if (!params.isMember("action") || !params["action"].isString()) {
        response["status"] = "error";
        response["result"] = "Error: Missing or invalid required string parameter 'action' (e.g., 'read', 'write', 'list', 'info', 'delete', 'mkdir').";
        return response.toStyledString();
    }
    std::string action = params["action"].asString();

    if (!params.isMember("path") || !params["path"].isString()) {
        response["status"] = "error";
        response["result"] = "Error: Missing or invalid required string parameter 'path'.";
        return response.toStyledString();
    }
    std::string path_str = params["path"].asString();
    if (path_str.empty()) {
        response["status"] = "error";
        response["result"] = "Error: Parameter 'path' cannot be empty.";
        return response.toStyledString();
    }

    // 2. Security Checks
    if (path_str.find("..") != std::string::npos) {
        response["status"] = "error";
        response["result"] = "Error: Invalid file path. Directory traversal ('..') is not allowed.";
        return response.toStyledString();
    }
    fs::path filePath = path_str;
    if (filePath.is_absolute()) {
       response["status"] = "error";
       response["result"] = "Error: Invalid file path. Absolute paths are not allowed.";
       return response.toStyledString();
    }
    // Basic check for potentially unsafe characters
    if (path_str.find_first_of("|;&`$<>") != std::string::npos) {
        response["status"] = "error";
        response["result"] = "Error: Invalid file path. Path contains potentially unsafe characters.";
        return response.toStyledString();
    }

    // 3. Action Dispatching
    try {
        // === READ ===
        if (action == "read") {
            std::error_code exists_ec;
            if (!fs::exists(filePath, exists_ec) || exists_ec) {
                 response["status"] = "error";
                 response["result"] = "Error [read]: Path not found or inaccessible: '" + path_str + "'" + (exists_ec ? " (" + exists_ec.message() + ")" : "");
                 return response.toStyledString();
            }
            std::error_code file_ec;
            if (!fs::is_regular_file(filePath, file_ec) || file_ec) {
                response["status"] = "error";
                response["result"] = "Error [read]: Path is not a regular file: '" + path_str + "'" + (file_ec ? " (" + file_ec.message() + ")" : "");
                return response.toStyledString();
            }
            std::ifstream fileStream(filePath);
            if (!fileStream.is_open()) {
                response["status"] = "error";
                response["result"] = "Error [read]: Could not open file '" + path_str + "' for reading (check permissions).";
                return response.toStyledString();
            }
            std::stringstream buffer;
            buffer << fileStream.rdbuf();
            fileStream.close();
            response["status"] = "success";
            response["result"] = buffer.str(); // Return only the content
            return response.toStyledString();

        // === WRITE ===
        } else if (action == "write") {
            if (!params.isMember("content") || !params["content"].isString()) {
                response["status"] = "error";
                response["result"] = "Error [write]: Missing or invalid string parameter 'content'.";
                return response.toStyledString();
            }
            std::string content = params["content"].asString();
            fs::path parentDir = filePath.parent_path();
            if (!parentDir.empty()) {
                 std::error_code parent_ec;
                 if (!fs::exists(parentDir, parent_ec) || parent_ec) {
                     response["status"] = "error";
                     response["result"] = "Error [write]: Parent directory '" + parentDir.string() + "' does not exist or is inaccessible" + (parent_ec ? " (" + parent_ec.message() + ")" : "");
                     return response.toStyledString();
                 }
                 if (!fs::is_directory(parentDir, parent_ec) || parent_ec) {
                      response["status"] = "error";
                      response["result"] = "Error [write]: Parent path '" + parentDir.string() + "' is not a directory" + (parent_ec ? " (" + parent_ec.message() + ")" : "");
                      return response.toStyledString();
                 }
            }
            std::error_code target_ec;
             if(fs::exists(filePath, target_ec) && !target_ec && fs::is_directory(filePath, target_ec) && !target_ec) {
                response["status"] = "error";
                response["result"] = "Error [write]: Path '" + path_str + "' already exists and is a directory.";
                return response.toStyledString();
             }

            std::ofstream fileStream(filePath, std::ios::trunc);
            if (!fileStream.is_open()) {
                 response["status"] = "error";
                 response["result"] = "Error [write]: Could not open file '" + path_str + "' for writing (check permissions or if path is a directory).";
                 return response.toStyledString();
            }
            fileStream << content;
            fileStream.flush();
            if (fileStream.fail() || fileStream.bad()) {
                fileStream.close();
                response["status"] = "error";
                response["result"] = "Error [write]: Failed writing content to '" + path_str + "' (disk full, permissions, or other I/O error?).";
                return response.toStyledString();
            }
            fileStream.close();
            response["status"] = "success";
            response["result"] = "Content successfully written to file '" + path_str + "'.";
            return response.toStyledString();

        // === LIST ===
        } else if (action == "list") {
            std::error_code exists_ec;
             if (!fs::exists(filePath, exists_ec) || exists_ec) {
                 response["status"] = "error";
                 response["result"] = "Error [list]: Path not found or inaccessible: '" + path_str + "'" + (exists_ec ? " (" + exists_ec.message() + ")" : "");
                 return response.toStyledString();
             }
             std::error_code dir_ec;
             if (!fs::is_directory(filePath, dir_ec) || dir_ec) {
                 response["status"] = "error";
                 response["result"] = "Error [list]: Path is not a directory: '" + path_str + "'" + (dir_ec ? " (" + dir_ec.message() + ")" : "");
                 return response.toStyledString();
             }
             std::stringstream listing;
             listing << "Listing of directory '" << path_str << "':
";
             int count = 0;
             std::error_code iter_ec;
             for (const auto& entry : fs::directory_iterator(filePath, fs::directory_options::skip_permission_denied, iter_ec)) {
                  if (iter_ec) {
                     listing << "[Warning: Error reading directory entry: " << iter_ec.message() << "]
";
                     iter_ec.clear();
                     continue;
                  }
                 try {
                     listing << "- " << entry.path().filename().string();
                     if (entry.is_directory()) listing << "/";
                 } catch (const std::exception& e) {
                     listing << "- [Error accessing entry name/type: " << e.what() << "]";
                 }
                 listing << "
";
                 count++;
             }
              if (iter_ec) {
                     listing << "[Stopped listing due to error: " << iter_ec.message() << "]
";
              }
             if (count == 0 && !iter_ec) {
                 listing << "(Directory is empty or contains only inaccessible items)
";
             }
             std::string result_str = listing.str();
             if (!result_str.empty() && result_str.back() == '
') result_str.pop_back();
             response["status"] = "success";
             response["result"] = result_str;
             return response.toStyledString();

        // === INFO ===
        } else if (action == "info") {
             std::error_code exists_ec;
             if (!fs::exists(filePath, exists_ec) || exists_ec) {
                 response["status"] = "error";
                 response["result"] = "Error [info]: Path not found or inaccessible: '" + path_str + "'" + (exists_ec ? " (" + exists_ec.message() + ")" : "");
                 return response.toStyledString();
             }
             std::stringstream info;
             info << "Information for path '" << path_str << "':
";
             std::error_code status_ec;
             auto status = fs::status(filePath, status_ec);
             if (status_ec) {
                 response["status"] = "error";
                 response["result"] = "Error [info]: Could not get status for '" + path_str + "': " + status_ec.message();
                 return response.toStyledString();
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
             info << "
";
             if (fs::is_regular_file(status)) {
                 std::error_code size_ec;
                 uintmax_t size = fs::file_size(filePath, size_ec);
                 if (size_ec) info << "- Size: N/A (Error: " << size_ec.message() << ")
";
                 else info << "- Size: " << size << " bytes
";
             }
             std::error_code time_ec;
             auto ftime = fs::last_write_time(filePath, time_ec);
             if (time_ec) info << "- Last Modified: N/A (Error: " << time_ec.message() << ")
";
             else info << "- Last Modified: " << formatFileTimeHelper(ftime) << "
";
             std::string result_str = info.str();
             if (!result_str.empty() && result_str.back() == '
') result_str.pop_back();
             response["status"] = "success";
             response["result"] = result_str;
             return response.toStyledString();

        // === DELETE ===
        } else if (action == "delete") {
             std::error_code exists_ec;
             if (!fs::exists(filePath, exists_ec)) {
                response["status"] = "error";
                response["result"] = "Error [delete]: Path not found: '" + path_str + "'";
                return response.toStyledString();
             }
             if(exists_ec) {
                  response["status"] = "error";
                  response["result"] = "Error [delete]: Cannot access path '" + path_str + "' to check existence: " + exists_ec.message();
                  return response.toStyledString();
             }
             std::error_code type_ec;
             bool is_dir = fs::is_directory(filePath, type_ec);
             if (type_ec) {
                 response["status"] = "error";
                 response["result"] = "Error [delete]: Cannot determine type of '" + path_str + "': " + type_ec.message();
                 return response.toStyledString();
             }

             if (is_dir) {
                std::error_code empty_ec;
                bool empty = fs::is_empty(filePath, empty_ec);
                 if (empty_ec) {
                    response["status"] = "error";
                    response["result"] = "Error [delete]: Cannot check if directory '" + path_str + "' is empty: " + empty_ec.message();
                    return response.toStyledString();
                 }
                 if (!empty) {
                      response["status"] = "error";
                      response["result"] = "Error [delete]: Directory '" + path_str + "' is not empty. Cannot delete non-empty directories.";
                      return response.toStyledString();
                 }
             }
             std::error_code remove_ec;
             bool removed = fs::remove(filePath, remove_ec);
             if (remove_ec) {
                 response["status"] = "error";
                 response["result"] = "Error [delete]: Failed to delete '" + path_str + "': " + remove_ec.message();
             } else if (!removed) {
                 response["status"] = "error";
                 response["result"] = "Error [delete]: Delete operation failed for '" + path_str + "' (check permissions/locking?).";
             } else {
                  std::error_code final_check_ec;
                  if (!fs::exists(filePath, final_check_ec) && !final_check_ec) {
                     response["status"] = "success";
                     response["result"] = "Path '" + path_str + "' successfully deleted.";
                  } else {
                     response["status"] = "warning"; // Changed status to warning as delete was reported success
                     response["result"] = "Warning [delete]: Delete command successful for '" + path_str + "' but it might still exist.";
                  }
             }
              return response.toStyledString();

        // === MKDIR ===
        } else if (action == "mkdir") {
            std::error_code exists_ec;
            if (fs::exists(filePath, exists_ec) && !exists_ec) {
                 response["status"] = "error";
                 response["result"] = "Error [mkdir]: Path '" + path_str + "' already exists.";
                 return response.toStyledString();
            } else if (exists_ec) {
                 response["status"] = "error";
                 response["result"] = "Error [mkdir]: Cannot check existence of path '" + path_str + "': " + exists_ec.message();
                 return response.toStyledString();
            }

             fs::path parentDir = filePath.parent_path();
            if (!parentDir.empty()) {
                 std::error_code parent_ec;
                 if (!fs::exists(parentDir, parent_ec) || parent_ec) {
                     response["status"] = "error";
                     response["result"] = "Error [mkdir]: Parent directory '" + parentDir.string() + "' does not exist or is inaccessible" + (parent_ec ? " (" + parent_ec.message() + ")" : "");
                     return response.toStyledString();
                 }
                 if (!fs::is_directory(parentDir, parent_ec) || parent_ec) {
                      response["status"] = "error";
                      response["result"] = "Error [mkdir]: Parent path '" + parentDir.string() + "' is not a directory" + (parent_ec ? " (" + parent_ec.message() + ")" : "");
                      return response.toStyledString();
                 }
            }
             std::error_code create_ec;
             if (fs::create_directory(filePath, create_ec)) {
                 response["status"] = "success";
                 response["result"] = "Directory '" + path_str + "' created.";
             } else {
                 response["status"] = "error";
                 response["result"] = "Error [mkdir]: Failed to create directory '" + path_str + "': " + create_ec.message();
             }
             return response.toStyledString();

        // === Unknown Action ===
        } else {
            response["status"] = "error";
            response["result"] = "Error: Unknown fileTool action '" + action + "'. Supported: 'read', 'write', 'list', 'info', 'delete', 'mkdir'.";
            return response.toStyledString();
        }

    } catch (const fs::filesystem_error& e) {
        std::string path1_info = e.path1().empty() ? "" : " [Path1: '" + e.path1().string() + "']";
        std::string path2_info = e.path2().empty() ? "" : " [Path2: '" + e.path2().string() + "']";
        response["status"] = "error";
        response["result"] = "Error (filesystem): " + std::string(e.what()) + path1_info + path2_info;
        return response.toStyledString();
    } catch (const std::exception& e) {
        response["status"] = "error";
        response["result"] = "Error (standard exception): " + std::string(e.what());
        return response.toStyledString();
    } catch (...) {
        response["status"] = "error";
        response["result"] = "Unknown internal error occurred during file operation.";
        return response.toStyledString();
    }
}
