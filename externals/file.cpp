// externals/file.cpp
// Rewritten for better security and robustness.

#include <chrono>     // For time formatting in 'info'
#include <cstdlib>    // For std::getenv
#include <filesystem> // Requires C++17
#include <fstream>
#include <iomanip>  // For time formatting in 'info'
#include <iostream> // For cerr, cout (debugging)
#include <optional> // For optional path return
#include <sstream>
#include <string>
#include <system_error> // For std::error_code
#include <vector>

#include "json/json.h" // Provided JSON library

// Include Agent.hpp for LogLevel enum and logMessage function declaration
#include "../inc/Agent.hpp"

namespace fs = std::filesystem;

namespace { // Use an anonymous namespace for internal helpers

const char *AGENT_WORKSPACE_ENV_VAR = "AGENT_WORKSPACE";
const char *DEFAULT_WORKSPACE =
    "./agent_workspace"; // Default if ENV_VAR not set

// --- Helper: Get the configured workspace path ---
fs::path getWorkspacePath() {
  const char *envPath = std::getenv(AGENT_WORKSPACE_ENV_VAR);
  fs::path workspace;
  if (envPath && envPath[0] != '\0') {
    workspace = envPath;
  } else {
    workspace = DEFAULT_WORKSPACE;
    // Optional: Log that default is being used if ENV not set
    // logMessage(LogLevel::DEBUG, "AGENT_WORKSPACE env var not set, using
    // default:", workspace.string());
  }

  // Ensure the workspace directory exists, create if not
  std::error_code ec;
  if (!fs::exists(workspace, ec)) {
    if (!fs::create_directories(workspace, ec) && ec) {
      logMessage(LogLevel::ERROR,
                 "Failed to create default workspace directory",
                 workspace.string() + " | Error: " + ec.message());
      // Throw or return an empty path to indicate critical failure? Let's
      // throw.
      throw std::runtime_error("Failed to create agent workspace: " +
                               workspace.string());
    } else if (!ec) {
      logMessage(LogLevel::INFO,
                 "Created agent workspace directory:", workspace.string());
    }
  } else if (ec) {
    logMessage(LogLevel::ERROR, "Error checking workspace directory existence",
               workspace.string() + " | Error: " + ec.message());
    throw std::runtime_error("Error checking agent workspace: " +
                             workspace.string());
  } else if (!fs::is_directory(workspace, ec)) {
    logMessage(LogLevel::ERROR, "Workspace path exists but is not a directory:",
               workspace.string());
    throw std::runtime_error("Agent workspace path is not a directory: " +
                             workspace.string());
  } else if (ec) {
    logMessage(LogLevel::ERROR,
               "Error checking if workspace path is a directory",
               workspace.string() + " | Error: " + ec.message());
    throw std::runtime_error("Error checking agent workspace type: " +
                             workspace.string());
  }

  // Return the canonical workspace path for consistent comparisons
  return fs::weakly_canonical(workspace,
                              ec); // Use weakly_canonical as it handles
                                   // non-existent paths during creation check
}

// --- Helper: Resolve relative path within workspace and ensure safety ---
std::optional<fs::path>
resolvePathAndCheckSafety(const std::string &relativePathStr,
                          const fs::path &workspacePath) {
  if (relativePathStr.empty()) {
    logMessage(LogLevel::WARN, "[fileTool] Received empty relative path.");
    return std::nullopt; // Reject empty paths
  }

  // Basic initial checks on the input string
  if (relativePathStr.find("..") != std::string::npos) {
    logMessage(LogLevel::WARN, "[fileTool] Path contains '..'. Disallowed.",
               relativePathStr);
    return std::nullopt; // Disallow '..' explicitly
  }
  if (relativePathStr.find_first_of("|;&`$<>\\") != std::string::npos) {
    logMessage(LogLevel::WARN,
               "[fileTool] Path contains potentially unsafe characters.",
               relativePathStr);
    return std::nullopt;
  }

  fs::path relativePath(relativePathStr);
  if (relativePath.is_absolute()) {
    logMessage(LogLevel::WARN, "[fileTool] Absolute paths are disallowed.",
               relativePathStr);
    return std::nullopt; // Disallow absolute paths
  }

  // Combine with workspace and normalize
  fs::path combinedPath = workspacePath / relativePath;
  std::error_code ec;
  fs::path canonicalPath = fs::weakly_canonical(
      combinedPath, ec); // Normalizes '.', avoids resolving symlinks
                         // immediately if target might not exist yet

  if (ec) {
    // This might happen if parts of the path don't exist yet, which is ok for
    // write/mkdir But log it for debugging potential issues later if needed.
    // logMessage(LogLevel::DEBUG, "[fileTool] Path canonicalization resulted in
    // error (might be ok if target doesn't exist yet)", combinedPath.string() +
    // " | Error: " + ec.message()); We still need to check the *intended* path
    // for safety based on string comparison before canonicalization potentially
    // fails
    canonicalPath =
        combinedPath
            .lexically_normal(); // Use lexical normalization as fallback
  }

  // *** Security Check: Ensure the final path is WITHIN the workspace ***
  auto workspaceStr = workspacePath.string();
  auto canonicalStr = canonicalPath.string();

  // Simple string prefix check (works reliably if paths are canonical/normal)
  // Add trailing separator to workspace path if not present for robust prefix
  // check
  if (!workspaceStr.empty() &&
      workspaceStr.back() != fs::path::preferred_separator) {
    workspaceStr += fs::path::preferred_separator;
  }
  if (!canonicalStr.empty() &&
      canonicalStr.back() != fs::path::preferred_separator &&
      fs::is_directory(canonicalPath, ec)) {
    // If it's intended to be a directory, ensure check compares directory
    // prefixes correctly
    // canonicalStr += fs::path::preferred_separator; // Temporarily add for
    // comparison if needed, depends on exact check logic
  }

  if (canonicalStr.rfind(workspaceStr, 0) !=
      0) { // Check if canonicalStr starts with workspaceStr
    logMessage(LogLevel::ERROR,
               "[fileTool] Security Violation: Path escapes workspace!",
               "Workspace: " + workspacePath.string() +
                   " | Attempted Path: " + relativePathStr +
                   " | Resolved To: " + canonicalPath.string());
    return std::nullopt; // Path is outside the workspace!
  }

  logMessage(LogLevel::DEBUG,
             "[fileTool] Path resolved safely within workspace",
             canonicalPath.string());
  return canonicalPath; // Return the safe, canonical path
}

// --- Helper: Format file time ---
std::string formatFileTime(fs::file_time_type ftime) {
  try {
    // Convert file_time_type to system_clock::time_point
    auto sctp =
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() +
            std::chrono::system_clock::now());
    std::time_t ctime = std::chrono::system_clock::to_time_t(sctp);
    std::tm timeinfo =
        *std::localtime(&ctime); // Use localtime for local time zone display
    char time_buf[100];          // Increased buffer size
    // ISO 8601-like format (YYYY-MM-DDTHH:MM:SS)
    if (std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%S",
                      &timeinfo)) {
      return std::string(time_buf);
    } else {
      logMessage(LogLevel::WARN,
                 "[fileTool] Failed to format file time with strftime.");
      return "(Error formatting time)";
    }
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR,
               "[fileTool] Exception during file time formatting", e.what());
    return "(Time conversion/formatting error)";
  }
}

} // end anonymous namespace

// --- Main Tool Function ---

// Tool Function: Performs file system operations (read, write, list, info,
// delete, mkdir)
//                confined within a designated workspace directory.
// Input: JSON object like:
// {
//   "action": "read|write|list|info|delete|mkdir", // Required
//   "path": "relative/path/to/file_or_dir",        // Required (Relative ONLY)
//   "content": "text content"                     // Required ONLY for "write"
//   action
// }
// Output: Operation result (content, listing, info) or success/error message
// string.
std::string fileTool(const Json::Value &params) {
  fs::path workspacePath;
  try {
    workspacePath = getWorkspacePath();
    logMessage(LogLevel::DEBUG,
               "[fileTool] Using workspace:", workspacePath.string());
  } catch (const std::exception &e) {
    return "Error [fileTool]: Critical failure getting or creating "
           "workspace: " +
           std::string(e.what());
  }

  // 1. Parameter Validation
  if (params == Json::nullValue || !params.isObject()) {
    return "Error [fileTool]: Requires a JSON object with parameters.";
  }
  if (!params.isMember("action") || !params["action"].isString()) {
    return "Error [fileTool]: Missing or invalid required string parameter "
           "'action'. "
           "Valid actions: read, write, list, info, delete, mkdir.";
  }
  std::string action = params["action"].asString();

  if (!params.isMember("path") || !params["path"].isString()) {
    return "Error [fileTool]: Missing or invalid required string parameter "
           "'path'. Path must be relative to the agent workspace.";
  }
  std::string relativePathStr = params["path"].asString();

  std::string content = ""; // Needed only for write
  if (action == "write") {
    if (!params.isMember("content") || !params["content"].isString()) {
      return "Error [fileTool:write]: Missing or invalid string parameter "
             "'content' when action is 'write'.";
    }
    content = params["content"].asString();
  }

  // 2. Path Resolution and Safety Check
  std::optional<fs::path> safePathOpt =
      resolvePathAndCheckSafety(relativePathStr, workspacePath);
  if (!safePathOpt) {
    return "Error [fileTool]: Invalid or unsafe path specified: '" +
           relativePathStr +
           "'. Path must be relative and within the workspace.";
  }
  fs::path safePath = *safePathOpt;

  logMessage(LogLevel::DEBUG, "[fileTool] Processing action '" + action +
                                  "' on safe path '" + safePath.string() + "'");

  // 3. Action Dispatching
  try {
    std::error_code ec;

    // === READ ===
    if (action == "read") {
      // Check existence and type
      if (!fs::exists(safePath, ec)) {
        return "Error [fileTool:read]: Path not found: '" + relativePathStr +
               "'";
      }
      if (ec)
        return "Error [fileTool:read]: Cannot check existence: " + ec.message();
      if (!fs::is_regular_file(safePath, ec)) {
        return "Error [fileTool:read]: Path is not a regular file: '" +
               relativePathStr + "'";
      }
      if (ec)
        return "Error [fileTool:read]: Cannot check type: " + ec.message();

      // Open and read
      std::ifstream fileStream(
          safePath, std::ios::binary); // Use binary to read bytes accurately
      if (!fileStream.is_open()) {
        return "Error [fileTool:read]: Could not open file '" +
               relativePathStr + "' (permissions?).";
      }
      std::stringstream buffer;
      buffer << fileStream.rdbuf();
      if (fileStream.bad()) {
        fileStream.close();
        return "Error [fileTool:read]: I/O error reading file '" +
               relativePathStr + "'.";
      }
      fileStream.close();
      logMessage(LogLevel::INFO, "[fileTool:read] Successfully read file",
                 relativePathStr);
      return "Success [fileTool:read]: Content of '" + relativePathStr +
             "':\n" + buffer.str();
    }

    // === WRITE ===
    else if (action == "write") {
      fs::path parentDir = safePath.parent_path();
      ec.clear();
      // Ensure parent directory exists (create if needed, within workspace
      // check already done)
      if (!parentDir.empty() && !fs::exists(parentDir, ec)) {
        if (!fs::create_directories(parentDir, ec) &&
            ec) { // Check error code *after* attempt
          return "Error [fileTool:write]: Failed to create parent directory '" +
                 parentDir.lexically_relative(workspacePath).string() +
                 "': " + ec.message();
        } else if (!ec) {
          logMessage(LogLevel::INFO,
                     "[fileTool:write] Created parent directory",
                     parentDir.lexically_relative(workspacePath).string());
        }
      } else if (ec) {
        return "Error [fileTool:write]: Cannot check parent existence: " +
               ec.message();
      }

      // Check if path exists and is a directory (cannot overwrite dir with
      // file)
      ec.clear();
      if (fs::exists(safePath, ec) && !ec && fs::is_directory(safePath, ec) &&
          !ec) {
        return "Error [fileTool:write]: Path '" + relativePathStr +
               "' exists and is a directory.";
      }

      // Open and write (overwrite/truncate)
      std::ofstream fileStream(safePath, std::ios::trunc | std::ios::binary);
      if (!fileStream.is_open()) {
        return "Error [fileTool:write]: Could not open file '" +
               relativePathStr + "' for writing (permissions?).";
      }
      fileStream << content;
      fileStream.flush(); // Ensure data is flushed
      if (fileStream.fail() || fileStream.bad()) {
        fileStream.close();
        return "Error [fileTool:write]: Failed writing content to '" +
               relativePathStr + "' (I/O error? Disk full?).";
      }
      fileStream.close();
      logMessage(LogLevel::INFO, "[fileTool:write] Successfully wrote to file",
                 relativePathStr);
      return "Success [fileTool:write]: Content written to file '" +
             relativePathStr + "'";
    }

    // === LIST ===
    else if (action == "list") {
      // Check existence and type
      if (!fs::exists(safePath, ec)) {
        return "Error [fileTool:list]: Path not found: '" + relativePathStr +
               "'";
      }
      if (ec)
        return "Error [fileTool:list]: Cannot check existence: " + ec.message();
      if (!fs::is_directory(safePath, ec)) {
        return "Error [fileTool:list]: Path is not a directory: '" +
               relativePathStr + "'";
      }
      if (ec)
        return "Error [fileTool:list]: Cannot check type: " + ec.message();

      std::stringstream listing;
      listing << "Listing of directory '" << relativePathStr << "':\n";
      int count = 0;
      const int max_list_entries = 200; // Limit output size
      ec.clear();
      // Iterate through directory, skipping permission errors
      for (const auto &entry : fs::directory_iterator(
               safePath, fs::directory_options::skip_permission_denied, ec)) {
        if (ec) {
          listing << "[Warning: Error reading an entry: " << ec.message()
                  << "]\n";
          ec.clear(); // Clear error to continue listing other entries
          continue;
        }
        if (count >= max_list_entries) {
          listing << "[Info: Reached maximum entry limit (" << max_list_entries
                  << ")]\n";
          break;
        }
        try {
          std::string entryName = entry.path().filename().string();
          listing << "- " << entryName;
          std::error_code entry_ec; // Local ec for checking entry type
          if (entry.is_directory(entry_ec) && !entry_ec)
            listing << "/";
          else if (entry.is_symlink(entry_ec) && !entry_ec)
            listing << "@"; // Indicate symlink
          else if (entry_ec)
            listing << " [Error checking type: " << entry_ec.message() << "]";
        } catch (const std::exception &e) {
          listing << "- [Error accessing entry properties: " << e.what() << "]";
        }
        listing << "\n";
        count++;
      }
      if (ec)
        listing
            << "[Warning: Stopped listing due to directory iteration error: "
            << ec.message() << "]\n";
      if (count == 0 && !ec)
        listing << "(Directory is empty or inaccessible)\n";

      std::string result = listing.str();
      if (!result.empty() && result.back() == '\n')
        result.pop_back(); // Trim trailing newline
      logMessage(LogLevel::INFO,
                 "[fileTool:list] Successfully listed directory",
                 relativePathStr);
      return result;
    }

    // === INFO ===
    else if (action == "info") {
      if (!fs::exists(safePath, ec)) {
        return "Error [fileTool:info]: Path not found: '" + relativePathStr +
               "'";
      }
      if (ec)
        return "Error [fileTool:info]: Cannot check existence: " + ec.message();

      std::stringstream info;
      info << "Information for path '" << relativePathStr << "':\n";
      ec.clear();
      auto status = fs::symlink_status(
          safePath, ec); // Use symlink_status to identify symlinks
      if (ec)
        return "Error [fileTool:info]: Could not get status for '" +
               relativePathStr + "': " + ec.message();

      info << "- Type: ";
      if (fs::is_regular_file(status))
        info << "File";
      else if (fs::is_directory(status))
        info << "Directory";
      else if (fs::is_symlink(status)) {
        info << "Symbolic Link";
        ec.clear();
        fs::path target = fs::read_symlink(safePath, ec);
        if (!ec)
          info << " -> '" << target.string() << "'";
        else
          info << " [Error reading target]";
      } else if (fs::is_block_file(status))
        info << "Block Device";
      else if (fs::is_character_file(status))
        info << "Character Device";
      else if (fs::is_fifo(status))
        info << "FIFO/Pipe";
      else if (fs::is_socket(status))
        info << "Socket";
      else
        info << "Other/Unknown";
      info << "\n";

      // Get size only for regular files (following symlinks)
      ec.clear();
      if (fs::is_regular_file(safePath, ec) &&
          !ec) { // Checks the target if it's a symlink
        uintmax_t size = fs::file_size(safePath, ec);
        if (ec)
          info << "- Size: N/A [Error: " << ec.message() << "]\n";
        else
          info << "- Size: " << size << " bytes\n";
      } else if (ec) {
        info << "- Size: N/A [Error checking type: " << ec.message() << "]\n";
      }

      ec.clear();
      auto ftime = fs::last_write_time(safePath, ec);
      if (ec)
        info << "- Last Modified: N/A [Error: " << ec.message() << "]\n";
      else
        info << "- Last Modified: " << formatFileTime(ftime) << "\n";

      // Permissions (basic example)
      // fs::perms p = status.permissions();
      // info << "- Permissions: " << std::oct << static_cast<int>(p) <<
      // std::dec << "\n";

      std::string result = info.str();
      if (!result.empty() && result.back() == '\n')
        result.pop_back();
      logMessage(LogLevel::INFO,
                 "[fileTool:info] Successfully retrieved info for",
                 relativePathStr);
      return result;
    }

    // === DELETE ===
    else if (action == "delete") {
      // Check existence first
      if (!fs::exists(safePath, ec)) {
        return "Success [fileTool:delete]: Path '" + relativePathStr +
               "' did not exist.";
      }
      if (ec)
        return "Error [fileTool:delete]: Cannot check existence: " +
               ec.message();

      ec.clear();
      bool is_dir = fs::is_directory(safePath, ec);
      if (ec)
        return "Error [fileTool:delete]: Cannot determine type of '" +
               relativePathStr + "': " + ec.message();

      // Refuse to delete non-empty directories for safety
      if (is_dir) {
        ec.clear();
        bool empty = fs::is_empty(safePath, ec);
        if (ec)
          return "Error [fileTool:delete]: Cannot check if directory '" +
                 relativePathStr + "' is empty: " + ec.message();
        if (!empty)
          return "Error [fileTool:delete]: Directory '" + relativePathStr +
                 "' is not empty. Cannot delete non-empty directories.";
      }

      ec.clear();
      bool removed = fs::remove(
          safePath, ec); // fs::remove handles both files and empty directories
      if (ec)
        return "Error [fileTool:delete]: Failed to delete '" + relativePathStr +
               "': " + ec.message();
      else if (!removed &&
               fs::exists(
                   safePath)) { // Double check if removal failed silently
        return "Error [fileTool:delete]: Delete reported no error for '" +
               relativePathStr + "', but path still exists (permissions?).";
      } else {
        logMessage(LogLevel::INFO, "[fileTool:delete] Successfully deleted",
                   relativePathStr);
        return "Success [fileTool:delete]: Path '" + relativePathStr +
               "' deleted.";
      }
    }

    // === MKDIR ===
    else if (action == "mkdir") {
      if (fs::exists(safePath, ec)) {
        return "Error [fileTool:mkdir]: Path '" + relativePathStr +
               "' already exists.";
      }
      if (ec)
        return "Error [fileTool:mkdir]: Cannot check existence: " +
               ec.message();

      fs::path parentDir = safePath.parent_path();
      // Ensure parent directory exists (create if needed) - workspace check
      // already done
      ec.clear();
      if (!parentDir.empty() && parentDir != workspacePath &&
          !fs::exists(parentDir, ec)) {
        if (!fs::create_directories(parentDir, ec) && ec) {
          return "Error [fileTool:mkdir]: Failed to create parent directory '" +
                 parentDir.lexically_relative(workspacePath).string() +
                 "': " + ec.message();
        } else if (!ec) {
          logMessage(LogLevel::INFO,
                     "[fileTool:mkdir] Created parent directory",
                     parentDir.lexically_relative(workspacePath).string());
        }
      } else if (ec) {
        return "Error [fileTool:mkdir]: Cannot check parent existence: " +
               ec.message();
      }

      ec.clear();
      if (fs::create_directory(safePath, ec)) {
        logMessage(LogLevel::INFO,
                   "[fileTool:mkdir] Successfully created directory",
                   relativePathStr);
        return "Success [fileTool:mkdir]: Directory '" + relativePathStr +
               "' created.";
      } else {
        return "Error [fileTool:mkdir]: Failed to create directory '" +
               relativePathStr + "'" + (ec ? ": " + ec.message() : ".");
      }
    }

    // === Unknown Action ===
    else {
      logMessage(LogLevel::WARN, "[fileTool] Received unknown action", action);
      return "Error [fileTool]: Unknown action '" + action +
             "'. Valid actions: read, write, list, info, delete, mkdir.";
    }

  } catch (const fs::filesystem_error &e) {
    // Log filesystem errors specifically
    std::string path1_info =
        e.path1().empty() ? "" : " | Path1: '" + e.path1().string() + "'";
    std::string path2_info =
        e.path2().empty() ? "" : " | Path2: '" + e.path2().string() + "'";
    logMessage(LogLevel::ERROR,
               "[fileTool] Filesystem Error for action '" + action +
                   "' on path '" + relativePathStr + "'",
               std::string(e.what()) + path1_info + path2_info +
                   " | Code: " + std::to_string(e.code().value()));
    return "Error [fileTool:" + action + "] Filesystem error processing '" +
           relativePathStr + "': " + std::string(e.what());
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR,
               "[fileTool] Standard Exception for action '" + action +
                   "' on path '" + relativePathStr + "'",
               e.what());
    return "Error [fileTool:" + action + "] Exception processing '" +
           relativePathStr + "': " + std::string(e.what());
  } catch (...) {
    logMessage(LogLevel::ERROR,
               "[fileTool] Unknown internal error occurred for action '" +
                   action + "' on path '" + relativePathStr + "'");
    return "Error [fileTool:" + action +
           "] Unknown internal error occurred processing '" + relativePathStr +
           "'.";
  }
}

// --- Helper Function Implementation ---
// (Already defined within the anonymous namespace above)
// std::string formatFileTime(fs::file_time_type ftime) { ... }
