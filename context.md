# AI Project Analysis - externals
- Generated on: Wed May  7 07:26:05 AM +01 2025
- System: Linux 6.12.26-1-lts x86_64
- Arch Linux: 1692 packages installed
- Directory: /home/mlamkadm/ai-repos/agents/agent-lib

## Directory Structure
```
../agent-lib/externals
├── bash.cpp
├── cal-events.cpp
├── file.cpp
├── general.cpp
├── sway.cpp
└── write.cpp
```

## Project Statistics
- Total Files: 112
- Total Lines of Code: 23279
- Languages: .cpp(6)

## Project Files

### File: externals/bash.cpp
```cpp
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
```

### File: externals/cal-events.cpp
```cpp

#include <algorithm>  // For std::sort, std::find_if
#include <chrono>     // For generating basic IDs
#include <filesystem> // Requires C++17
#include <fstream>
#include <iomanip> // For formatting time in IDs
#include <iostream>
#include <mutex> // For protecting file access
#include <sstream>
#include <string>
#include <vector>

#include "json/json.h" // Already used by the agent/server
// #include "../inc/Tool.hpp" // Tool definition - included via server.cpp
// indirectly

// --- Data Structure for Events ---
struct CalendarEvent {
  std::string id;   // Unique identifier for the event
  std::string date; // Format: YYYY-MM-DD
  std::string time; // Format: HH:MM (or empty for all-day)
  std::string description;

  // For sorting events
  bool operator<(const CalendarEvent &other) const {
    if (date != other.date) {
      return date < other.date;
    }
    // Treat empty time as earlier than any specific time for sorting purposes
    if (time.empty() && !other.time.empty())
      return true;
    if (!time.empty() && other.time.empty())
      return false;
    // If both have times or both are empty, compare times lexicographically
    return time < other.time;
  }
};

// --- Globals for this Tool (Protected by Mutex) ---
const std::string CALENDAR_DATA_FILE = ".calendar_data.json";
std::mutex calendarMutex; // Mutex to protect file I/O and in-memory data if
                          // needed concurrently

// --- Helper Functions ---

// Generate a reasonably unique ID (Timestamp + simple counter/random part)
std::string generateEventId() {
  auto now = std::chrono::system_clock::now();
  auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
  auto epoch = now_ms.time_since_epoch();
  long long timestamp_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();

  // Add a small random element to reduce collision chance if called rapidly
  static int counter = 0;
  std::stringstream ss;
  ss << timestamp_ms << "-" << (counter++);
  return ss.str();
}

// Load events from the JSON file
std::vector<CalendarEvent> loadEventsFromFile() {
  std::vector<CalendarEvent> events;
  std::ifstream file(CALENDAR_DATA_FILE);

  if (!file.is_open()) {
    // File might not exist yet, which is fine on first run
    if (std::filesystem::exists(CALENDAR_DATA_FILE)) {
      std::cerr << "[WARN] calendarTool: Could not open '" << CALENDAR_DATA_FILE
                << "' for reading." << std::endl;
    } else {
      std::cout << "[INFO] calendarTool: Data file '" << CALENDAR_DATA_FILE
                << "' not found. Starting fresh." << std::endl;
    }
    return events; // Return empty vector
  }

  Json::Value root;
  Json::Reader reader;
  if (!reader.parse(file, root)) {
    std::cerr << "[ERROR] calendarTool: Failed to parse JSON from '"
              << CALENDAR_DATA_FILE
              << "'. Error: " << reader.getFormattedErrorMessages()
              << std::endl;
    file.close();
    // Consider returning an error or throwing, but returning empty might be
    // safer for agent flow
    return events; // Return empty vector on parse error
  }
  file.close();

  if (!root.isArray()) {
    std::cerr << "[ERROR] calendarTool: JSON root in '" << CALENDAR_DATA_FILE
              << "' is not an array." << std::endl;
    return events;
  }

  for (const auto &item : root) {
    try {
      CalendarEvent event;
      event.id = item.get("id", generateEventId())
                     .asString(); // Add ID generation if missing
      event.date = item["date"].asString();
      event.time = item.get("time", "").asString(); // Optional time
      event.description = item["description"].asString();
      // Basic validation could happen here (e.g., date format)
      events.push_back(event);
    } catch (const Json::Exception &e) {
      std::cerr
          << "[WARN] calendarTool: Skipping invalid event item in JSON. Error: "
          << e.what() << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "[WARN] calendarTool: Skipping event due to std exception: "
                << e.what() << std::endl;
    }
  }

  // Sort events after loading
  std::sort(events.begin(), events.end());

  return events;
}

// Save events to the JSON file
bool saveEventsToFile(const std::vector<CalendarEvent> &events) {
  Json::Value root(Json::arrayValue); // Create a JSON array

  for (const auto &event : events) {
    Json::Value item;
    item["id"] = event.id;
    item["date"] = event.date;
    if (!event.time.empty()) { // Only include time if it's set
      item["time"] = event.time;
    }
    item["description"] = event.description;
    root.append(item);
  }

  std::ofstream file(CALENDAR_DATA_FILE, std::ios::trunc); // Overwrite the file
  if (!file.is_open()) {
    std::cerr << "[ERROR] calendarTool: Could not open '" << CALENDAR_DATA_FILE
              << "' for writing." << std::endl;
    return false;
  }

  // Use StyledWriter for readability, FastWriter for compactness
  // Json::StyledWriter writer;
  Json::FastWriter writer;
  file << writer.write(root);

  if (file.fail() || file.bad()) {
    std::cerr << "[ERROR] calendarTool: Failed writing events to '"
              << CALENDAR_DATA_FILE << "'." << std::endl;
    file.close();
    return false;
  }

  file.close();
  return true;
}

// Basic date format validation (YYYY-MM-DD)
bool isValidDate(const std::string &date) {
  if (date.length() != 10)
    return false;
  // Very basic check - doesn't validate day/month ranges or leap years
  return date[4] == '-' && date[7] == '-' && isdigit(date[0]) &&
         isdigit(date[1]) && isdigit(date[2]) && isdigit(date[3]) &&
         isdigit(date[5]) && isdigit(date[6]) && isdigit(date[8]) &&
         isdigit(date[9]);
}

// Basic time format validation (HH:MM)
bool isValidTime(const std::string &time) {
  if (time.empty())
    return true; // Empty time is valid (all-day)
  if (time.length() != 5)
    return false;
  // Basic check - doesn't validate hour/minute ranges
  return time[2] == ':' && isdigit(time[0]) && isdigit(time[1]) &&
         isdigit(time[3]) && isdigit(time[4]);
}

// --- Tool Implementation ---

// Tool Function: Manages calendar events (add, list).
// Input: JSON object with "action" ("add" or "list") and corresponding
// parameters. Output: Success or error message string.
std::string calendarTool(const Json::Value &params) {
  std::lock_guard<std::mutex> lock(calendarMutex); // Protect file access

  // 1. Validate overall parameters
  if (params == Json::nullValue || params.empty() || !params.isObject()) {
    return "Error: calendarTool requires a JSON object with parameters.";
  }
  if (!params.isMember("action") || !params["action"].isString()) {
    return "Error: Missing or invalid required string parameter 'action' "
           "('add' or 'list').";
  }

  std::string action = params["action"].asString();
  std::vector<CalendarEvent> events =
      loadEventsFromFile(); // Load current events

  // 2. Dispatch based on action
  if (action == "add") {
    // --- Add Event Action ---
    if (!params.isMember("date") || !params["date"].isString()) {
      return "Error [add action]: Missing or invalid required string parameter "
             "'date' (YYYY-MM-DD).";
    }
    if (!params.isMember("description") || !params["description"].isString() ||
        params["description"].asString().empty()) {
      return "Error [add action]: Missing or invalid non-empty string "
             "parameter 'description'.";
    }

    CalendarEvent newEvent;
    newEvent.date = params["date"].asString();
    newEvent.description = params["description"].asString();
    newEvent.time = params.get("time", "").asString(); // Optional time

    // Validate inputs
    if (!isValidDate(newEvent.date)) {
      return "Error [add action]: Invalid date format. Expected YYYY-MM-DD. "
             "Received: '" +
             newEvent.date + "'";
    }
    if (!isValidTime(newEvent.time)) {
      return "Error [add action]: Invalid time format. Expected HH:MM or "
             "empty. Received: '" +
             newEvent.time + "'";
    }

    newEvent.id = generateEventId(); // Assign unique ID

    events.push_back(newEvent);
    std::sort(events.begin(), events.end()); // Keep sorted

    if (saveEventsToFile(events)) {
      std::stringstream successMsg;
      successMsg << "Success: Event added with ID '" << newEvent.id << "'.\n";
      successMsg << "Date: " << newEvent.date;
      if (!newEvent.time.empty())
        successMsg << " Time: " << newEvent.time;
      successMsg << "\nDescription: " << newEvent.description;
      return successMsg.str();
    } else {
      return "Error [add action]: Failed to save event to file.";
    }

  } else if (action == "list") {
    // --- List Events Action ---
    std::string filterDate = params.get("date", "").asString();
    if (!filterDate.empty() && !isValidDate(filterDate)) {
      return "Error [list action]: Invalid date format for filtering. Expected "
             "YYYY-MM-DD. Received: '" +
             filterDate + "'";
    }

    std::stringstream output;
    int count = 0;
    if (filterDate.empty()) {
      output << "All upcoming or current events:\n"; // Or just "All events"
    } else {
      output << "Events for date " << filterDate << ":\n";
    }

    for (const auto &event : events) {
      bool match = false;
      if (filterDate.empty()) {
        match = true; // List all if no date filter
      } else {
        match = (event.date == filterDate);
      }

      if (match) {
        count++;
        output << "- ID: " << event.id << " | Date: " << event.date;
        if (!event.time.empty()) {
          output << " Time: " << event.time;
        }
        output << " | Desc: " << event.description << "\n";
      }
    }

    if (count == 0) {
      if (filterDate.empty()) {
        output << "(No events scheduled)\n";
      } else {
        output << "(No events found for this date)\n";
      }
    }

    std::string result = output.str();
    if (!result.empty() && result.back() == '\n') { // Trim trailing newline
      result.pop_back();
    }
    return result;

  } else {
    // --- Unknown Action ---
    return "Error: Unknown action '" + action +
           "'. Supported actions: 'add', 'list'.";
  }
}
```

### File: externals/file.cpp
```cpp
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
```

### File: externals/general.cpp
```cpp

#include "json/json.h" // For parsing the JSON response
#include <cstdlib>     // For std::getenv
#include <curl/curl.h> // For making HTTP requests
#include <iostream>
#include <memory> // For unique_ptr
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Gets current time (params ignored)
std::string getTime(const Json::Value &params) {
  (void)params; // Unused
  time_t now = time(0);
  char buf[80];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
  return buf;
}

// Rolls a die (uses params)
std::string rollDice(const Json::Value &params) {
  int sides = 6; // Default
  int rolls = 1; // Default
  if (params.isMember("sides") && params["sides"].isInt()) {
    int requestedSides = params["sides"].asInt();
    if (requestedSides > 0) {
      sides = requestedSides;
    }
  }
  std::ostringstream oss;
  if (params.isMember("rolls") && params["rolls"].isInt()) {
    rolls = params["rolls"].asInt();
    while (rolls > 0) {
      int roll = (rand() % sides) + 1;
      oss << "Rolled a " << roll << " (d" << sides << ")";
      rolls--;
    }
  }
  return oss.str();
}

std::string calculate(const Json::Value &params) {
  if (params == Json::nullValue) {
    return "No parameters provided.";
  } else if (params.isMember("expression")) {
    std::string expression = params["expression"].asString();
    // Simple parsing and evaluation logic (for demonstration)
    // In a real scenario, you would use a proper expression parser
    std::istringstream iss(expression);
    double a, b;
    char op;
    iss >> a >> op >> b;
    double result = 0.0;
    switch (op) {
    case '+':
      result = a + b;
      break;
    case '-':
      result = a - b;
      break;
    case '*':
      result = a * b;
      break;
    case '/':
      result = (b != 0) ? a / b : 0;
      break;
    default:
      return "Invalid operator.";
    }
    return "Result: " + std::to_string(result);
  }
  return "Invalid parameters.";
}
```

### File: externals/sway.cpp
```cpp
// externals/sway_control.cpp

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
```

### File: externals/write.cpp
```cpp

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
```
# AI Project Analysis - inc
- Generated on: Wed May  7 07:26:05 AM +01 2025
- System: Linux 6.12.26-1-lts x86_64
- Arch Linux: 1692 packages installed
- Directory: /home/mlamkadm/ai-repos/agents/agent-lib

## Directory Structure
```
../agent-lib/inc
├── Agent.hpp
├── File.hpp
├── Groq.hpp
├── Import.hpp
├── Json.hpp
├── MiniGemini.hpp
├── modelApi.hpp
├── notes.hpp
├── Tool.hpp
├── Utils.hpp
└── variables.hpp
```

## Project Statistics
- Total Files: 113
- Total Lines of Code: 24625
- Languages: .hpp(11)

## Project Files

### File: inc/Agent.hpp
```cpp
#pragma once

#include "File.hpp" // Forward declare if only pointers/refs used, include if values/size needed
#include "MiniGemini.hpp" // Forward declare if only pointers/refs used
#include "Tool.hpp"       // Forward declare if only pointers/refs used

#include <cerrno>
#include <chrono>  // For timestamp generation
#include <iomanip> // For timestamp formatting
#include <map>
#include <stdexcept> // For std::runtime_error
#include <string>
#include <vector>

// Needed for ActionInfo struct and some method parameters
#include <json/json.h> // Includes json/value.h
                       //

// Forward declarations (if full definitions aren't needed in this header)
namespace Json {
class Value;
}
class MiniGemini;
class Tool;
class File;

// Typedefs
typedef std::vector<File> fileList;
typedef std::vector<std::pair<std::string, std::string>> StrkeyValuePair;

// Logging Enum
enum class LogLevel {
  DEBUG,
  INFO,
  WARN,
  ERROR,
  TOOL_CALL,
  TOOL_RESULT,
  PROMPT
};

// Logging Function Prototype (Implementation likely in agent.cpp or logging
// util)
void logMessage(LogLevel level, const std::string &message,
                const std::string &details = "");

// --- Struct Definitions for LLM Interaction (NEW/UPDATED) ---

struct StructuredThought {
  std::string type;    // e.g., PLAN, OBSERVATION, QUESTION, NORM, etc.
  std::string content; // The text of the thought
};

struct ActionInfo {
  std::string action; // Identifier (tool name, function name, script name)
  std::string type;   // Category (tool, script, internal_function, output,
                      // workflow_control)
  Json::Value
      params; // Parameters for the action (ensure json/json.h is included)
  double confidence = 1.0;           // Optional confidence score
  std::vector<std::string> warnings; // Optional warnings

  // Default constructor might be needed depending on usage
  ActionInfo() = default;
};

// --- Struct Definition for Directive (PUBLIC) ---
struct AgentDirective {
  enum Type { BRAINSTORMING, AUTONOMOUS, NORMAL, EXECUTE, REPORT };
  Type type = NORMAL;
  std::string description;
  std::string format;
};

namespace types {

// dealing with runtime data context
typedef enum {
  FILE,
  FOLDER,
  EXECUTABLE, // could be  script
  STRING,
} VarType;

} // namespace types

#define FILE_EXT_ALLOWED                                                       \
  {".txt", ".csv", ".json", ".xml", ".yaml", ".yml", ".md", ".log"}

class CONTEXT_ITEM {
private:
  unsigned long id; // will just index them for now, imma look into how to
                    // porperly implement uuid  later.

  std::string name; //  in this case it is refering to the varname inside the
                    //  yaml file
  types::VarType type;
  std::string path;
  std::string content;

  std::string runtime; // for executable type only.

  bool loaded;
};

// --- Agent Class Definition ---
class Agent {
public:
  // Public alias for the directive struct
  using DIRECTIVE = AgentDirective;

  // Struct for OLD tool call format (potentially deprecated)
  struct ToolCallInfo {
    std::string toolName;
    Json::Value params;
  };

  // --- Constructor ---
  Agent(MiniGemini &api);

  // --- Static Factory (Optional) ---
  // static std::unique_ptr<Agent> loadFromYaml(const std::string& yamlPath,
  // MiniGemini& api);

  // --- Configuration (Public Setters - Now including Directive/Task/Command)
  // ---
  void setSystemPrompt(const std::string &prompt);
  void setName(const std::string &name);
  void setDescription(const std::string &description);
  void setIterationCap(int cap);
  void setDirective(const DIRECTIVE &dir);            // MADE PUBLIC
  void addTask(const std::string &task);              // MADE PUBLIC
  void addInitialCommand(const std::string &command); // MADE PUBLIC
  void setSchema(const std::string &schema);
  void setExample(const std::string &example) { this->example = example; }

  // --- Tool Management ---
  void addTool(Tool *tool);
  void removeTool(const std::string &toolName);
  Tool *getTool(const std::string &toolName) const;
  Tool *getRegisteredTool(const std::string &toolName) const;
  std::string getInternalToolDescription(const std::string &toolName) const;

  // finds a tool by name then add it as a built-in
  void addBuiltin(const std::string &toolName);

  // --- Core Agent Loop ---
  void reset();
  std::string prompt(const std::string &userInput);
  void run();

  // --- Agent Interaction (Orchestration) ---
  void addAgent(Agent *agent);
  std::string
  agentInstance(const std::string &name); // Gets sub-agent name (simple check)

  // --- Manual Operations ---
  std::string manualToolCall(const std::string &toolName,
                             const Json::Value &params);

  // --- Public Getters (Ensure const correctness and existence) ---
  const std::string getName() const;
  const std::string getDescription() const;
  MiniGemini &getApi(); // Cannot be const if MiniGemini methods aren't const
  fileList getFiles() const;
  const std::vector<std::pair<std::string, std::string>> &getHistory() const;

  // --- Getters needed for saveAgentProfile (Added/Made Public/Const) ---
  const std::string &getSystemPrompt() const;
  int getIterationCap() const;
  const std::vector<std::pair<std::string, std::string>> &getEnv() const;
  const std::vector<std::string> &getExtraPrompts() const;
  const std::vector<std::string> &getTasks() const;
  const DIRECTIVE &getDirective() const; // MADE PUBLIC and CONST

  // --- Memory & State (Public Modifiers) ---
  void addMemory(const std::string &role, const std::string &content);
  void removeMemory(const std::string &role, const std::string &content);
  void addEnvVar(const std::string &key, const std::string &value);
  void importEnvFile(const std::string &filePath);
  void addPrompt(const std::string &prompt); // Adds to extraPrompts

  // --- Utilities ---
  std::string generateStamp(void);
  void addToHistory(const std::string &role, const std::string &content);
  std::string wrapText(const std::string &text);
  std::string wrapXmlLine(const std::string &content, const std::string &tag);
  std::string wrapXmlBlock(const std::string &content, const std::string &tag);
  void addBlockToSystemPrompt(const std::string &content,
                              const std::string &tag);

  void listAllAgentInfo() {
    std::cout << "Agent Name: " << name << "\n"
              << "Description: " << description << "\n"
              << "System Prompt: " << systemPrompt << "\n"
              << "Iteration Cap: " << iterationCap << "\n"
              << "Current Iteration: " << iteration << "\n"
              << "Schema: " << schema << "\n"
              << "Tasks: ";
    for (const auto &task : tasks) {
      std::cout << task << ", ";
    }
    std::cout << "\n";
  }

private:
  // --- Private Members (Clean Naming Scheme) ---
  MiniGemini &api;
  std::map<std::string, Tool *> tools;
  std::map<std::string, std::string> internalToolDescriptions;

  std::string systemPrompt;
  std::vector<std::pair<std::string, std::string>> history;

  int iteration = 0;
  int iterationCap = 10;
  bool skipFlowIteration = false;

  std::vector<std::pair<std::string, std::string>> env;

  fileList files; // Assuming File class definition exists
  std::string name;
  std::string description;

  std::vector<std::string> extraPrompts;

  std::vector<std::pair<std::string, Agent *>> subAgents;

  StrkeyValuePair scratchpad;
  StrkeyValuePair shortTermMemory;
  StrkeyValuePair longTermMemory;

  std::vector<std::string> tasks;
  std::vector<std::string> initialCommands;

  DIRECTIVE directive; // Private member instance192.168.1.101

  std::string schema;
  std::string example;

  // std::vector<std::string, CONTEXT_ITEM> context;
  std::map<std::string, CONTEXT_ITEM> contextMap;
  // --- Private Helper Methods ---
  std::string buildFullPrompt() const;
  // --- UPDATED parseStructuredLLMResponse declaration ---
  bool parseStructuredLLMResponse(
      const std::string &jsonString,
      std::string &status,                      // Changed
      std::vector<StructuredThought> &thoughts, // Changed
      std::vector<ActionInfo> &actions,         // Changed
      std::string &finalResponse);

  // --- ADDED declarations for new private helpers ---
  std::string processActions(const std::vector<ActionInfo> &actions);
  std::string processAction(const ActionInfo &actionInfo);
  std::string directiveTypeToString(
      Agent::DIRECTIVE::Type type) const; // Helper for buildFullPrompt

  // --- Potentially Deprecated Fallback/Old Methods ---
  std::vector<ToolCallInfo>
  extractToolCalls(const std::string &response); // Deprecated?
  std::string
  processToolCalls(const std::vector<ToolCallInfo> &toolCalls); // Deprecated?
  std::string handleToolExecution(const ToolCallInfo &call);    // Deprecated?

  std::string executeApiCall(const std::string &fullPrompt);
  void setSkipFlowIteration(bool skip);

  // --- Internal Tool Implementations (Remain Private) ---
  std::string getHelp(const Json::Value &params);
  std::string skip(const Json::Value &params);
  std::string promptAgentTool(const Json::Value &params);
  std::string summerizeTool(const Json::Value &params); // Typo preserved
  std::string summarizeHistory(const Json::Value &params);
  std::string getWeather(const Json::Value &params);
  std::string reportTool(const Json::Value &params);
  std::string generateCode(const Json::Value &params);

  // --- Task Execution Helpers (Remain Private) ---
  std::string
  executeTask(const std::string &task,
              const Json::Value &format); // Needs Json::Value variant if used
  std::string executeTask(const std::string &task);
  std::string executeTask(const std::string &task, const std::string &format);
  std::string auditResponse(const std::string &response);
};
```

### File: inc/File.hpp
```cpp
#ifndef FILE_HPP // Include guard start
#define FILE_HPP

#include <algorithm> // For std::remove, std::find
#include <cstddef>   // For size_t
#include <fstream>   // For std::ifstream, std::ofstream
#include <iostream>  // For std::ostream
#include <sstream>   // For std::ostringstream
#include <stdexcept> // For std::runtime_error
#include <string>
#include <vector>

// Forward declaration for the friend function
class File;
std::ostream &operator<<(std::ostream &os, const File &f);

class File {
public:
  // Default constructor (C++98 style)
  // Initializes members to reasonable defaults.
  File() : path_(""), content_(""), description_("") {
    // tags_ is default-constructed to an empty vector
  }

  // Constructor to load from a file path (C++98 style)
  // Use explicit to prevent unintentional conversions
  explicit File(const std::string &filePath)
      : path_(filePath), content_(""), description_("") {
    // tags_ is default-constructed to an empty vector

    // C++98 std::ifstream constructor often preferred const char*
    std::ifstream fileStream(path_.c_str());
    if (!fileStream.is_open()) {
      throw std::runtime_error("Could not open file for reading: " + path_);
    }

    // Read the entire file content efficiently using stream buffer
    std::ostringstream ss;
    ss << fileStream.rdbuf();
    content_ = ss.str();

    // fileStream is closed automatically when it goes out of scope (RAII)
  }

  // Destructor (C++98 style)
  // Default behavior is sufficient as members manage their own resources.
  ~File() {
    // No explicit cleanup needed for std::string or std::vector
  }

  // --- Getters (const methods) ---
  const std::string &getPath() const { return path_; }
  const std::string &getContent() const { return content_; }
  const std::string &getDescription() const { return description_; }
  const std::vector<std::string> &getTags() const { return tags_; }

  // --- Setters / Modifiers ---
  void setContent(const std::string &content) { content_ = content; }
  void setDescription(const std::string &desc) { description_ = desc; }
  void setTags(const std::vector<std::string> &tags) { tags_ = tags; }

  void addTag(const std::string &tag) {
    // Optional: Avoid adding duplicate tags using std::find (C++98 compatible)
    if (std::find(tags_.begin(), tags_.end(), tag) == tags_.end()) {
      tags_.push_back(tag);
    }
  }

  void removeTag(const std::string &tag) {
    // Erase-remove idiom (C++98 compatible)
    // Need to explicitly state the iterator type (no 'auto')
    std::vector<std::string>::iterator new_end =
        std::remove(tags_.begin(), tags_.end(), tag);
    tags_.erase(new_end, tags_.end());
  }

  // --- File Operations ---

  // Save content back to the original path
  // Throws if path_ is empty.
  void save() {
    if (path_.empty()) {
      throw std::logic_error("Cannot save file: Path is not set. Use saveAs() "
                             "or load from a file first.");
    }
    saveAs(path_); // Delegate to saveAs
  }

  // Save content to a *new* path (and update the internal path_)
  // Note: Marked non-const here because although it doesn't change *members*
  // other than path_,
  //       it has a significant side effect (filesystem modification) and
  //       updates the path. If save() were const, calling saveAs from it would
  //       be problematic. If strict const-correctness regarding members is
  //       needed, saveAs could return void and a separate setPath method could
  //       be used, or save could be non-const. Making saveAs non-const as it
  //       modifies path_ is a reasonable C++98 approach.
  void saveAs(const std::string &newPath) {
    // C++98 std::ofstream constructor often preferred const char*
    std::ofstream outFile(newPath.c_str());
    if (!outFile.is_open()) {
      throw std::runtime_error("Could not open file for writing: " + newPath);
    }
    outFile << content_; // Write content

    // outFile is closed automatically when it goes out of scope (RAII)

    // Update the internal path only after successful write attempt
    path_ = newPath;
  }

  // --- Operator Overloads ---

  // Friend declaration needed within the class
  friend std::ostream &operator<<(std::ostream &os, const File &f);

private:
  // --- Member Variables ---
  std::string path_;              // Path of the file
  std::string content_;           // Content loaded from the file
  std::string description_;       // User-defined description
  std::vector<std::string> tags_; // User-defined tags

  // --- Private Copy Control (Optional, C++98 style) ---
  // Uncomment these lines to prevent copying/assignment if shallow copies
  // are undesirable or if managing resources requires deeper copies.
  // The default compiler-generated ones perform member-wise copy/assignment.
  // File(const File&);            // Disallow copy constructor
  // File& operator=(const File&); // Disallow assignment operator
};

// --- Operator Overloads Implementation (outside class) ---

// Overload << to print metadata summary (C++98 style loop)
inline std::ostream &operator<<(std::ostream &os, const File &f) {
  os << "File(path: \"" << f.getPath() << "\""; // Use getter for consistency
  if (!f.getDescription().empty()) {
    os << ", description: \"" << f.getDescription() << "\"";
  }
  const std::vector<std::string> &tags = f.getTags(); // Use getter
  if (!tags.empty()) {
    os << ", tags: [";
    // C++98 compatible loop (using index)
    for (std::size_t i = 0; i < tags.size(); ++i) {
      os << "\"" << tags[i] << "\"";
      if (i < tags.size() - 1) { // Check if not the last element
        os << ", ";
      }
    }
    os << "]";
  }
  os << ", content_size: " << f.getContent().length()
     << " bytes)"; // Use getter
  return os;
}

#endif // FILE_HPP // Include guard end
```

### File: inc/Groq.hpp
```cpp
#ifndef GROQ_CLIENT_HPP
#define GROQ_CLIENT_HPP

#include "modelApi.hpp" // Include the base class definition
#include <string>
#include <json/json.h> // Or your preferred JSON library

class GroqClient : public LLMClient {
public:
    // Constructor: API key is required (can be empty string to try ENV var)
    GroqClient(const std::string& apiKey = "");

    // Override the pure virtual generate function from the base class
    std::string generate(const std::string& prompt) override;

    // --- Groq-Specific Configuration ---
    void setApiKey(const std::string& apiKey);
    void setModel(const std::string& model) override;
    void setTemperature(double temperature) override;
    void setMaxTokens(int maxTokens) override;
    void setBaseUrl(const std::string& baseUrl); // Default: https://api.groq.com/openai/v1

private:
    std::string m_apiKey;
    std::string m_model;
    double m_temperature;
    int m_maxTokens;
    std::string m_baseUrl;

    // Internal helper for HTTP POST request (specific to Groq/OpenAI structure)
    std::string performHttpRequest(const std::string& url, const std::string& payload);
    // Internal helper to parse Groq/OpenAI JSON response structure
    std::string parseJsonResponse(const std::string& jsonResponse) const;

    // Static helper for curl callback
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif // GROQ_CLIENT_HPP
```

### File: inc/Import.hpp
```cpp

#pragma once
#include "Agent.hpp"

class Agent;

bool loadAgentProfile(Agent &agentToConfigure, const std::string &yamlPath) ;
```

### File: inc/Json.hpp
```cpp
#ifndef JSON_H_
#define JSON_H_

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cstdlib>
#include <stdexcept>
#include <cassert>

namespace json {

// Forward declarations
class Value;
typedef std::map<std::string, Value> Object;
typedef std::vector<Value> Array;

// JSON value types
enum ValueType {
    NULL_VALUE,
    BOOL_VALUE,
    NUMBER_VALUE,
    STRING_VALUE,
    ARRAY_VALUE,
    OBJECT_VALUE
};

class Value {
private:
    ValueType type_;
    bool bool_value_;
    double number_value_;
    std::string string_value_;
    Array array_value_;
    Object object_value_;

public:
    // Constructors
    Value() : type_(NULL_VALUE), bool_value_(false), number_value_(0) {}
    
    // Type-specific constructors
    Value(bool value) : type_(BOOL_VALUE), bool_value_(value), number_value_(0) {}
    Value(int value) : type_(NUMBER_VALUE), bool_value_(false), number_value_(value) {}
    Value(double value) : type_(NUMBER_VALUE), bool_value_(false), number_value_(value) {}
    Value(const char* value) : type_(STRING_VALUE), bool_value_(false), number_value_(0), string_value_(value) {}
    Value(const std::string& value) : type_(STRING_VALUE), bool_value_(false), number_value_(0), string_value_(value) {}
    Value(const Array& value) : type_(ARRAY_VALUE), bool_value_(false), number_value_(0), array_value_(value) {}
    Value(const Object& value) : type_(OBJECT_VALUE), bool_value_(false), number_value_(0), object_value_(value) {}

    // Type checking
    bool isNull() const { return type_ == NULL_VALUE; }
    bool isBool() const { return type_ == BOOL_VALUE; }
    bool isNumber() const { return type_ == NUMBER_VALUE; }
    bool isString() const { return type_ == STRING_VALUE; }
    bool isArray() const { return type_ == ARRAY_VALUE; }
    bool isObject() const { return type_ == OBJECT_VALUE; }
    ValueType type() const { return type_; }

    // Value retrieval (with type checking)
    bool asBool() const {
        if (!isBool()) throw std::runtime_error("Value is not a boolean");
        return bool_value_;
    }
    
    double asNumber() const {
        if (!isNumber()) throw std::runtime_error("Value is not a number");
        return number_value_;
    }
    
    int asInt() const {
        if (!isNumber()) throw std::runtime_error("Value is not a number");
        return static_cast<int>(number_value_);
    }
    
    const std::string& asString() const {
        if (!isString()) throw std::runtime_error("Value is not a string");
        return string_value_;
    }
    
    const Array& asArray() const {
        if (!isArray()) throw std::runtime_error("Value is not an array");
        return array_value_;
    }
    
    const Object& asObject() const {
        if (!isObject()) throw std::runtime_error("Value is not an object");
        return object_value_;
    }
    
    // Mutable accessors
    Array& asArray() {
        if (!isArray()) throw std::runtime_error("Value is not an array");
        return array_value_;
    }
    
    Object& asObject() {
        if (!isObject()) throw std::runtime_error("Value is not an object");
        return object_value_;
    }

    // Array/Object element access
    const Value& operator[](size_t index) const {
        if (!isArray()) throw std::runtime_error("Value is not an array");
        if (index >= array_value_.size()) throw std::out_of_range("Array index out of range");
        return array_value_[index];
    }
    
    Value& operator[](size_t index) {
        if (type_ != ARRAY_VALUE) {
            type_ = ARRAY_VALUE;
            array_value_.clear();
        }
        if (index >= array_value_.size()) 
            array_value_.resize(index + 1);
        return array_value_[index];
    }
    
    const Value& operator[](const std::string& key) const {
        if (!isObject()) throw std::runtime_error("Value is not an object");
        Object::const_iterator it = object_value_.find(key);
        if (it == object_value_.end()) throw std::out_of_range("Object key not found");
        return it->second;
    }
    
    Value& operator[](const std::string& key) {
        if (type_ != OBJECT_VALUE) {
            type_ = OBJECT_VALUE;
            object_value_.clear();
        }
        return object_value_[key];
    }

    // Helper methods for object access
    bool has(const std::string& key) const {
        if (!isObject()) return false;
        return object_value_.find(key) != object_value_.end();
    }
    
    // Size information
    size_t size() const {
        if (isArray()) return array_value_.size();
        if (isObject()) return object_value_.size();
        return 0;
    }

    // String representation
    std::string toString(bool pretty = false, int indent = 0) const {
        std::ostringstream os;
        
        switch (type_) {
            case NULL_VALUE:
                os << "null";
                break;
                
            case BOOL_VALUE:
                os << (bool_value_ ? "true" : "false");
                break;
                
            case NUMBER_VALUE:
                os << number_value_;
                break;
                
            case STRING_VALUE:
                os << '"';
                for (size_t i = 0; i < string_value_.length(); ++i) {
                    char c = string_value_[i];
                    switch (c) {
                        case '"':  os << "\\\""; break;
                        case '\\': os << "\\\\"; break;
                        case '\b': os << "\\b"; break;
                        case '\f': os << "\\f"; break;
                        case '\n': os << "\\n"; break;
                        case '\r': os << "\\r"; break;
                        case '\t': os << "\\t"; break;
                        default:
                            if (c < 32) {
                                os << "\\u" 
                                   << std::hex << std::uppercase
                                   << static_cast<int>(c)
                                   << std::dec << std::nouppercase;
                            } else {
                                os << c;
                            }
                    }
                }
                os << '"';
                break;
                
            case ARRAY_VALUE: {
                os << '[';
                
                if (pretty && !array_value_.empty()) {
                    os << '\n';
                }
                
                for (size_t i = 0; i < array_value_.size(); ++i) {
                    if (pretty) {
                        for (int j = 0; j < indent + 2; ++j) os << ' ';
                    }
                    
                    os << array_value_[i].toString(pretty, indent + 2);
                    
                    if (i < array_value_.size() - 1) {
                        os << ',';
                    }
                    
                    if (pretty) {
                        os << '\n';
                    }
                }
                
                if (pretty && !array_value_.empty()) {
                    for (int j = 0; j < indent; ++j) os << ' ';
                }
                
                os << ']';
                break;
            }
            
            case OBJECT_VALUE: {
                os << '{';
                
                if (pretty && !object_value_.empty()) {
                    os << '\n';
                }
                
                Object::const_iterator it = object_value_.begin();
                for (size_t i = 0; it != object_value_.end(); ++it, ++i) {
                    if (pretty) {
                        for (int j = 0; j < indent + 2; ++j) os << ' ';
                    }
                    
                    os << '"' << it->first << "\":";
                    
                    if (pretty) {
                        os << ' ';
                    }
                    
                    os << it->second.toString(pretty, indent + 2);
                    
                    if (i < object_value_.size() - 1) {
                        os << ',';
                    }
                    
                    if (pretty) {
                        os << '\n';
                    }
                }
                
                if (pretty && !object_value_.empty()) {
                    for (int j = 0; j < indent; ++j) os << ' ';
                }
                
                os << '}';
                break;
            }
        }
        
        return os.str();
    }
};

// Parsing functions
namespace parser {
    // Simple tokenizer for JSON parsing
    class Tokenizer {
    private:
        const std::string& input_;
        size_t pos_;

        void skipWhitespace() {
            while (pos_ < input_.length() && isspace(input_[pos_]))
                ++pos_;
        }

    public:
        Tokenizer(const std::string& input) : input_(input), pos_(0) {}

        bool hasMore() const {
            return pos_ < input_.length();
        }

        char peek() {
            skipWhitespace();
            if (!hasMore()) return '\0';
            return input_[pos_];
        }

        char next() {
            skipWhitespace();
            if (!hasMore()) return '\0';
            return input_[pos_++];
        }

        std::string readString() {
            // Assume the opening quote has been consumed
            std::string result;
            bool escape = false;

            while (hasMore()) {
                char c = input_[pos_++];
                
                if (escape) {
                    escape = false;
                    switch (c) {
                        case '"': result += '"'; break;
                        case '\\': result += '\\'; break;
                        case '/': result += '/'; break;
                        case 'b': result += '\b'; break;
                        case 'f': result += '\f'; break;
                        case 'n': result += '\n'; break;
                        case 'r': result += '\r'; break;
                        case 't': result += '\t'; break;
                        case 'u': {
                            // Handle \uXXXX escape sequences
                            if (pos_ + 4 > input_.length()) {
                                throw std::runtime_error("Invalid \\u escape sequence");
                            }
                            std::string hex = input_.substr(pos_, 4);
                            pos_ += 4;
                            
                            // Convert hex to int and then to char
                            int value = 0;
                            for (int i = 0; i < 4; ++i) {
                                value = value * 16;
                                char h = hex[i];
                                if (h >= '0' && h <= '9') value += (h - '0');
                                else if (h >= 'A' && h <= 'F') value += (h - 'A' + 10);
                                else if (h >= 'a' && h <= 'f') value += (h - 'a' + 10);
                                else throw std::runtime_error("Invalid hex character in \\u escape");
                            }
                            
                            // This is a simplification - proper UTF-8 handling would be more complex
                            if (value < 128) {
                                result += static_cast<char>(value);
                            } else {
                                // Just for basic handling, more comprehensive UTF-8 would be needed
                                result += '?';
                            }
                            break;
                        }
                        default:
                            result += c;
                    }
                } else if (c == '\\') {
                    escape = true;
                } else if (c == '"') {
                    // End of string
                    break;
                } else {
                    result += c;
                }
            }
            
            return result;
        }

        std::string readNumber() {
            size_t start = pos_ - 1;  // Include the first digit
            
            // Read digits, dot, exponent, etc.
            while (hasMore() && (
                   isdigit(input_[pos_]) || 
                   input_[pos_] == '.' || 
                   input_[pos_] == 'e' || 
                   input_[pos_] == 'E' || 
                   input_[pos_] == '+' || 
                   input_[pos_] == '-')) {
                ++pos_;
            }
            
            return input_.substr(start, pos_ - start);
        }

        std::string readToken() {
            size_t start = pos_ - 1;
            while (hasMore() && isalpha(input_[pos_])) {
                ++pos_;
            }
            return input_.substr(start, pos_ - start);
        }
        
        void expect(char c) {
            if (next() != c) {
                std::ostringstream msg;
                msg << "Expected '" << c << "' at position " << pos_;
                throw std::runtime_error(msg.str());
            }
        }
    };

    Value parseValue(Tokenizer& tokenizer);

    Value parseObject(Tokenizer& tokenizer) {
        Object obj;
        
        // Empty object?
        if (tokenizer.peek() == '}') {
            tokenizer.next();  // Consume '}'
            return Value(obj);
        }
        
        while (true) {
            // Parse key
            if (tokenizer.next() != '"') {
                throw std::runtime_error("Expected string as object key");
            }
            std::string key = tokenizer.readString();
            
            // Parse colon
            if (tokenizer.next() != ':') {
                throw std::runtime_error("Expected ':' after object key");
            }
            
            // Parse value
            obj[key] = parseValue(tokenizer);
            
            // More items?
            char c = tokenizer.next();
            if (c == '}') {
                break;  // End of object
            }
            else if (c != ',') {
                throw std::runtime_error("Expected ',' or '}' after object value");
            }
        }
        
        return Value(obj);
    }

    Value parseArray(Tokenizer& tokenizer) {
        Array arr;
        
        // Empty array?
        if (tokenizer.peek() == ']') {
            tokenizer.next();  // Consume ']'
            return Value(arr);
        }
        
        while (true) {
            // Parse value
            arr.push_back(parseValue(tokenizer));
            
            // More items?
            char c = tokenizer.next();
            if (c == ']') {
                break;  // End of array
            }
            else if (c != ',') {
                throw std::runtime_error("Expected ',' or ']' after array value");
            }
        }
        
        return Value(arr);
    }

    Value parseValue(Tokenizer& tokenizer) {
        char c = tokenizer.peek();
        
        switch (c) {
            case 'n':
                // null
                tokenizer.next();  // Consume 'n'
                if (tokenizer.readToken() == "ull") {
                    return Value();
                }
                throw std::runtime_error("Invalid token: expected 'null'");
                
            case 't':
                // true
                tokenizer.next();  // Consume 't'
                if (tokenizer.readToken() == "rue") {
                    return Value(true);
                }
                throw std::runtime_error("Invalid token: expected 'true'");
                
            case 'f':
                // false
                tokenizer.next();  // Consume 'f'
                if (tokenizer.readToken() == "alse") {  
                    return Value(false);
                }
                throw std::runtime_error("Invalid token: expected 'false'");
                
            case '"':
                // string
                tokenizer.next();  // Consume '"'
                return Value(tokenizer.readString());
                
            case '{':
                // object
                tokenizer.next();  // Consume '{'
                return parseObject(tokenizer);
                
            case '[':
                // array
                tokenizer.next();  // Consume '['
                return parseArray(tokenizer);
                
            case '-':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                // number
                tokenizer.next();  // Consume first digit/sign
                return Value(atof(tokenizer.readNumber().c_str()));
                
            default:
                throw std::runtime_error("Unexpected character in JSON");
        }
    }
    
    Value parse(const std::string& input) {
        Tokenizer tokenizer(input);
        Value result = parseValue(tokenizer);
        
        // Check for trailing data
        if (tokenizer.hasMore()) {
            char c = tokenizer.peek();
            if (!isspace(c)) {
                throw std::runtime_error("Unexpected trailing data in JSON");
            }
        }
        
        return result;
    }
}  // namespace parser

// Helper functions
inline Value parse(const std::string& input) {
    return parser::parse(input);
}

inline Value parseFile(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for reading");
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return parse(buffer.str());
}

}  // namespace json

#endif  // JSON_H_
```

### File: inc/MiniGemini.hpp
```cpp
#ifndef GEMINI_CLIENT_HPP
#define GEMINI_CLIENT_HPP

#include "modelApi.hpp" // Include the base class definition
#include <string>
#include <json/json.h> // For Json::Value (or your preferred JSON library)

const std::string MODELS[] = {
    // Main models
    "gemini-2.0-flash",         // Latest Flash model
    "gemini-2.0-flash-lite",    // Cost-efficient Flash variant
    "gemini-1.5-flash",         // Previous generation Flash model
    "gemini-1.5-flash-8b",      // Lightweight 8B parameter variant
    "gemini-2.5-pro-exp-03-25", // Experimental Pro model

    // Special purpose models
    "gemini-embedding-exp",      // Text embedding model
    "models/text-embedding-004", // Basic text embedding model
    "imagen-3.0-generate-002",   // Image generation model
    "models/embedding-001"       // Legacy embedding model
};

class MiniGemini : public LLMClient {
public:
    // Constructor: API key is required (can be empty string to try ENV var)
    MiniGemini(const std::string& apiKey = "");

    // Override the pure virtual generate function from the base class
    std::string generate(const std::string& prompt) override;

    // --- Gemini-Specific Configuration ---
    void setApiKey(const std::string& apiKey);
    void setModel(const std::string& model) override;
    void setTemperature(double temperature) override;
    void setMaxTokens(int maxTokens) override;
    void setBaseUrl(const std::string& baseUrl); // Allow changing the base URL if needed

private:
    std::string m_apiKey;
    std::string m_model;
    double m_temperature;
    int m_maxTokens;
    std::string m_baseUrl;

    // Internal helper for HTTP POST request (specific to Gemini structure)
    std::string performHttpRequest(const std::string& url, const std::string& payload);
    // Internal helper to parse Gemini's JSON response structure
    std::string parseJsonResponse(const std::string& jsonResponse) const;

    // Static helper for curl callback (can be shared or made global if needed)
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif // GEMINI_CLIENT_HPP
```

### File: inc/modelApi.hpp
```cpp
#pragma once
#include <stdexcept>
#include <string>

// Custom exception for API errors (can be shared by all clients)
class ApiError : public std::runtime_error {
public:
    ApiError(const std::string& message) : std::runtime_error(message) {}
};

// Abstract base class for LLM clients
class LLMClient {
public:
    // Virtual destructor is crucial for base classes with virtual functions
    virtual ~LLMClient() = default;

    // Pure virtual function that all derived clients MUST implement
    // Takes a prompt and returns the generated text or throws ApiError
    virtual std::string generate(const std::string& prompt) = 0;

    // Optional: Add common configuration setters if desired,
    // but they might be better handled in derived classes if APIs differ significantly.
    virtual void setModel(const std::string& model) = 0;
    virtual void setTemperature(double temperature) = 0;
    virtual void setMaxTokens(int maxTokens) = 0;
};

```

### File: inc/notes.hpp
```cpp
#ifndef NOTES_HPP
#define NOTES_HPP

#include <fstream>
#include <string>
#include <vector>

class note {
public:
  note(const std::string &text) : _text(text) {}
  note(const std::string &text, const std::string &path)
      : _text(text), _path(path) {
    // if (_path.empty()) {
    //     _path = "./";
    //     }
    // open file and write text to it
    std::ofstream file(_path);

    if (!file.is_open()) {
      throw std::runtime_error("Could not open file: " + _path);
    } else {
      file << text;
    }
  }
  ~note() {
    // close file
    std::ofstream file(_path, std::ios::app);
    if (!file.is_open()) {
      throw std::runtime_error("Could not open file: " + _path);
    } else {
      file.close();
    }
  }
  std::string getText() const;
  void setText(const std::string &text);

  std::string getPath() const;

private:
  std::string _text;
  std::string _path;
};

typedef std::vector<note> notes;

#endif
```

### File: inc/Tool.hpp
```cpp
#pragma once

#include <json/json.h> // For Json::Value used in callback signature
#include <map>         // For m_use_cases, m_memory_stack
#include <stdexcept>   // For std::runtime_error
#include <string>

// Forward declaration (optional, good practice)
class Agent;
namespace Json {
class Value;
}

// Callback function signatures: Takes JSON parameters, returns string result
typedef std::string (*ToolCallback)(const Json::Value &params);
typedef std::string (*ToolCallbackWithAgent)(const Json::Value &params,
                                             Agent &agent);

class Tool {
public:
  // --- Constructors ---
  // Constructor for tools needing agent context (built-in style)
  Tool(const std::string &name, const std::string &description,
       ToolCallbackWithAgent callback, Agent &agent)
      : m_name(name), m_description(description), m_callback(nullptr),
        m_builtin_callback(callback), m_agent(&agent) {}

  // Constructor for standard external tools
  Tool(const std::string &name, const std::string &description,
       ToolCallback callback)
      : m_name(name), m_description(description), m_callback(callback),
        m_builtin_callback(nullptr), m_agent(nullptr) {}

  // Default constructor
  Tool()
      : m_name(""), m_description(""), m_callback(nullptr),
        m_builtin_callback(nullptr), m_agent(nullptr) {}

  // Constructor with just a callback (name/desc can be set later)
  Tool(ToolCallback callback)
      : m_name(""), m_description(""), m_callback(callback),
        m_builtin_callback(nullptr), m_agent(nullptr) {}

  // --- Getters / Setters ---
  std::string getName() const { return m_name; }
  void setName(const std::string &name) { m_name = name; }

  std::string getDescription() const { return m_description; }
  void setDescription(const std::string &description) {
    m_description = description;
  }

  // --- Execution ---
  // Execute the tool's function with given JSON parameters
  std::string execute(const Json::Value &params) {
    if (m_builtin_callback &&
        m_agent) { // Prioritize built-in if agent context is available
      return m_builtin_callback(params, *m_agent);
    } else if (m_callback) { // Fallback to standard callback
      return m_callback(params);
    } else {
      throw std::runtime_error("No valid callback function set for tool '" +
                               m_name + "'");
    }
  }
  // execute(const std::string& params) overload removed

  // --- Callback Configuration ---
  void setCallback(ToolCallback callback) {
    if (!callback) {
      throw std::invalid_argument("Tool callback cannot be NULL");
    }
    m_callback = callback;
    m_builtin_callback = nullptr; // Ensure only one callback type is active
  }

  void setBuiltinCallback(ToolCallbackWithAgent callback, Agent &agent) {
    if (!callback) {
      throw std::invalid_argument("Builtin tool callback cannot be NULL");
    }
    m_builtin_callback = callback;
    m_agent = &agent;
    m_callback = nullptr; // Ensure only one callback type is active
  }

  // --- Use Cases ---
  void addUseCase(const std::string &use_case, const std::string &example) {
    m_use_cases[use_case] = example;
  }

  std::string getUseCase(const std::string &use_case) const {
    auto it = m_use_cases.find(use_case);
    if (it != m_use_cases.end()) {
      return it->second;
    } else {
      // Return empty string or throw? Returning empty might be safer.
      return "";
      // throw std::runtime_error("Use case not found: " + use_case);
    }
  }
  std::string getAllUseCases() const {
    std::string all_use_cases;
    if (m_use_cases.empty())
      return "";

    all_use_cases += "Example Use Cases for tool '" + m_name + "':\n";
    for (const auto &pair : m_use_cases) {
      all_use_cases += "- Purpose: " + pair.first +
                       "\n  Example JSON Params: " + pair.second + "\n";
    }
    return all_use_cases;
  }
  std::string getAllUseCaseCap(size_t cap) const {
    std::string all_use_cases;
    if (m_use_cases.empty() || cap == 0)
      return "";

    all_use_cases += "Example Use Cases for tool '" + m_name + "' (limit " +
                     std::to_string(cap) + "):\n";
    size_t count = 0;
    for (const auto &pair : m_use_cases) {
      if (count >= cap)
        break;
      all_use_cases += "- Purpose: " + pair.first +
                       "\n  Example JSON Params: " + pair.second + "\n";
      ++count;
    }
    return all_use_cases;
  }

private:
  std::string m_name;
  std::string m_description;

  ToolCallback m_callback = nullptr; // Standard callback (expects JSON)
  ToolCallbackWithAgent m_builtin_callback =
      nullptr; // Callback needing agent context (expects JSON)
  enum {
    // Tool types can be defined here if needed
    // e.g., EXTERNAL, BUILTIN, etc.
    TOOL_TYPE_EXTERNAL,
    TOOL_TYPE_BUILTIN
  } type = TOOL_TYPE_EXTERNAL;

  std::map<std::string, std::string>
      m_use_cases; // Map of purpose -> example JSON params string

  Agent *m_agent = nullptr; // Pointer to the agent using this tool (for builtin
                            // callbacks) PureTextToolCallback removed
};
```

### File: inc/Utils.hpp
```cpp

#pragma once

#include <string>

int executeCommand(const std::string &command, std::string &output) ;
```

### File: inc/variables.hpp
```cpp
#include <iostream>
#include <string>
#include <filesystem>
#include <map>
#include <vector>
#include <sstream>
// std::any is C++17 and later
#include <any>

template <typename T>
class Variable {
private:
    std::string key;
    T value;

public:
    Variable(const std::string& k, const T& val) : key(k), value(val) {}

    std::string getKey() const { return key; }
    T getValue() const { return value; }
};


class NamespaceVariables {
private:
    std::map<std::string, NamespaceVariables*> namespaces;
    std::map<std::string, Variable<std::any>> variables;  // Use std::any to store any type

public:
    // Access a namespace (creates it if it doesn't exist)
    NamespaceVariables& operator[](const std::string& ns) {
        if (namespaces.find(ns) == namespaces.end()) {
            namespaces[ns] = new NamespaceVariables();
        }
        return *namespaces[ns];
    }

    // Set a variable within the current namespace
    template <typename T>
    void set(const std::string& key, const T& value) {
        variables[key] = Variable<std::any>(key, value);
    }

    // Get a variable within the current namespace
    template <typename T>
    T get(const std::string& key) const {
        try {
            return std::any_cast<T>(variables.at(key).getValue());
        } catch (const std::bad_any_cast& e) {
            std::cerr << "Error: Invalid type cast for key: " << key << std::endl;
            throw; // Re-throw the exception or handle it differently
        }
    }


    // Helper function to print all variables in a namespace (and sub-namespaces)
    void print(const std::string& prefix = "") const {
        for (const auto& var : variables) {
            std::cout << prefix << var.first << ": ";
            try { // Attempt to print common types
                std::cout << std::any_cast<std::string>(var.second.getValue()) << std::endl;
            } catch(...) {
                try { std::cout << std::any_cast<int>(var.second.getValue()) << std::endl; }
                catch(...) {
                    try { std::cout << std::any_cast<bool>(var.second.getValue()) << std::endl; }
                    catch(...) {
                        try { std::cout << std::any_cast<std::filesystem::path>(var.second.getValue()) << std::endl; }
                        catch(...) { std::cout << "[Unprintable Type]" << std::endl; }
                    }
                }
            }
        }
        for (const auto& ns : namespaces) {
            std::cout << prefix << ns.first << ":" << std::endl;
            ns.second->print(prefix + "  ");
        }
    }


};




int main() {
    NamespaceVariables vars;

    vars["global"]["configs"]["agents"]["standard"].set("name", "Standard Agent");
    vars["agent-1"]["state"]["input"].set("value", 123);
    vars["global"]["configs"]["port"].set("number", 8080);
    vars["global"]["configs"]["path"].set<std::filesystem::path>("value", "data/config.json");


    vars.print();


    std::cout << "Agent name: " << vars["global"]["configs"]["agents"]["standard"].get<std::string>("name") << std::endl;
    std::cout << "Port number: " << vars["global"]["configs"]["port"].get<int>("number") << std::endl;

    // Example demonstrating error handling:
    try {
        std::string portNumber = vars["global"]["configs"]["port"].get<std::string>("number"); // Incorrect type
    } catch (const std::bad_any_cast& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }


    return 0;
}
```
# AI Project Analysis - src
- Generated on: Wed May  7 07:26:05 AM +01 2025
- System: Linux 6.12.26-1-lts x86_64
- Arch Linux: 1692 packages installed
- Directory: /home/mlamkadm/ai-repos/agents/agent-lib

## Directory Structure
```
../agent-lib/src
├── agent
│   ├── built-in.cpp
│   ├── core.cpp
│   ├── export.cpp
│   ├── import.cpp
│   ├── parse.cpp
│   ├── prompt.cpp
│   ├── runtime.cpp
│   └── tool.cpp
├── agent.cpp
├── groqClient.cpp
├── logging
├── memory
│   ├── local
│   └── note
├── MiniGemini.cpp
├── tools
└── utils
    └── global.cpp
```

## Project Statistics
- Total Files: 114
- Total Lines of Code: 26127
- Languages: .cpp(12)

## Project Files

### File: src/agent/built-in.cpp
```cpp
//
// #include "../../inc/Agent.hpp"
// #include "../../inc/MiniGemini.hpp"
// #include "../../inc/Tool.hpp"
// #include <algorithm> // for std::find_if, std::remove_if
// #include <cctype>    // for std::toupper
// #include <chrono>    // for timestamp
// #include <cstdio>    // for file, fgets
// #include <cstdlib>   // for popen, pclose, system
// #include <ctime>
// #include <fstream> // for file operations
// #include <iomanip> // for formatting time
// #include <iostream> #include <json/json.h>
// #include <sstream>
// #include <stdexcept>
//
// std::string Agent::summerizeTool(const Json::Value &params) {
//   if (!params.isMember("content") || !params["content"].isString()) {
//     return "Error [summarizeTool]: Missing or invalid string parameter "
//            "'content'.";
//   }
//   std::string content = params["content"].asString();
//   if (content.length() < 50) {
//     return "Content is too short to summarize effectively.";
//   }
//   logMessage(LogLevel::DEBUG, "Summarizing content (length: " +
//                                   std::to_string(content.length()) + ")");
//   try {
//     std::string task =
//         "Provide a concise summary of the following text:\n\n" + content;
//     std::string format = "{\"summary\": \"string (concise summary)\"}";
//     std::string llmResponse = executeTask(task, format);
//
//     Json::Value summaryJson;
//     Json::CharReaderBuilder readerBuilder;
//     std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
//     std::string errs;
//     if (reader->parse(llmResponse.c_str(),
//                       llmResponse.c_str() + llmResponse.length(), &summaryJson,
//                       &errs) &&
//         summaryJson.isObject() && summaryJson.isMember("summary") &&
//         summaryJson["summary"].isString()) {
//       return summaryJson["summary"].asString();
//     } else {
//       logMessage(
//           LogLevel::WARN,
//           "Failed to parse summary JSON from LLM, returning raw response.",
//           "LLM Response: " + llmResponse);
//       return llmResponse;
//     }
//   } catch (const std::exception &e) {
//     logMessage(LogLevel::ERROR, "Error during summarization task execution",
//                e.what());
//     return "Error [summarizeTool]: Exception during summarization: " +
//            std::string(e.what());
//   }
// }
//
// std::string Agent::summarizeHistory(const Json::Value &params) {
//   (void)params;
//   if (history.empty())
//     return "Conversation history is empty.";
//   std::string historyText = "Conversation History:\n";
//   for (const auto &entry : history)
//     historyText += entry.first + ": " + entry.second + "\n";
//   Json::Value summarizeParams;
//   summarizeParams["content"] = historyText;
//   return summerizeTool(summarizeParams);
// }
//
// std::string Agent::getWeather(const Json::Value &params) {
//   if (!params.isMember("location") || !params["location"].isString()) {
//     return "Error [getWeather]: Missing or invalid string parameter "
//            "'location'.";
//   }
//   std::string location = params["location"].asString();
//   std::string originalLocation = location;
//   std::replace(location.begin(), location.end(), ' ', '+');
//   std::string command =
//       "curl -s -L \"https://wttr.in/" + location + "?format=3\"";
//   std::string weatherResult;
//   int status = executeCommand(command, weatherResult);
//   weatherResult.erase(0, weatherResult.find_first_not_of(" \t\r\n"));
//   weatherResult.erase(weatherResult.find_last_not_of(" \t\r\n") + 1);
//   if (status != 0 || weatherResult.empty() ||
//       weatherResult.find("Unknown location") != std::string::npos ||
//       weatherResult.find("ERROR") != std::string::npos ||
//       weatherResult.find("Sorry") != std::string::npos) {
//     logMessage(LogLevel::WARN, "Failed to get weather using wttr.in",
//                "Command: " + command + ", Status: " + std::to_string(status) +
//                    ", Output: " + weatherResult);
//     return "Error [getWeather]: Could not retrieve weather for '" +
//            originalLocation + "'.";
//   }
//   return "Current weather for " + originalLocation + ": " + weatherResult;
// }
//
// std::string Agent::skip(const Json::Value &params) {
//   bool doSkip = false;
//   if (params.isMember("skip") && params["skip"].isBool()) {
//     doSkip = params["skip"].asBool();
//   } else {
//     return "Error [skip]: Missing or invalid boolean parameter 'skip'. "
//            "Example: {\"skip\": true}";
//   }
//
//   if (doSkip) {
//     this->setSkipFlowIteration(true);
//     return "Success [skip]: Final response generation for this turn will be "
//            "skipped.";
//   } else {
//     this->setSkipFlowIteration(false);
//     return "Success [skip]: Final response generation will proceed normally.";
//   }
// }
//
// std::string Agent::promptAgentTool(const Json::Value &params) {
//   if (!params.isMember("name") || !params["name"].isString() ||
//       !params.isMember("prompt") || !params["prompt"].isString()) {
//     return "Error [promptAgent]: Requires string parameters 'name' (target "
//            "agent) and 'prompt'.";
//   }
//   std::string agentName = params["name"].asString();
//   std::string userInput = params["prompt"].asString();
//
//   logMessage(LogLevel::INFO,
//              "Agent '" + name + "' is prompting agent '" + agentName + "'");
//
//   Agent *targetAgent = nullptr;
//   for (auto &agentPair : subAgents) {
//     if (agentPair.first == agentName) {
//       targetAgent = agentPair.second;
//       break;
//     }
//   }
//
//   if (targetAgent) {
//     try {
//       std::string contextualPrompt =
//           "Received prompt from Agent '" + name + "':\n" + userInput;
//       std::string response = targetAgent->prompt(contextualPrompt);
//       logMessage(LogLevel::INFO,
//                  "Received response from agent '" + agentName + "'");
//       return "Response from Agent '" + agentName + "':\n---\n" + response +
//              "\n---";
//     } catch (const std::exception &e) {
//       logMessage(LogLevel::ERROR, "Error prompting agent '" + agentName + "'",
//                  e.what());
//       return "Error [promptAgent]: Exception occurred while prompting agent '" +
//              agentName + "': " + e.what();
//     }
//   } else {
//     logMessage(LogLevel::WARN,
//                "Agent '" + agentName + "' not found for prompting.");
//     return "Error [promptAgent]: Agent '" + agentName + "' not found.";
//   }
// }
//
// // Provides help/descriptions for available tools.
// std::string Agent::getHelp(const Json::Value &params) {
//   std::ostringstream helpOss;
//   std::string specificTool;
//
//   if (params.isMember("tool_name") && params["tool_name"].isString()) {
//     specificTool = params["tool_name"].asString();
//   }
//
//   if (!specificTool.empty()) {
//     helpOss << "Help for tool '" << specificTool << "':\n";
//     bool found = false;
//     auto internalIt = internalToolDescriptions.find(specificTool);
//     if (internalIt != internalToolDescriptions.end()) {
//       helpOss << "- Type: Internal\n";
//       helpOss << "- Description & Params: " << internalIt->second;
//       found = true;
//     }
//     Tool *tool = getTool(specificTool);
//     if (tool) {
//       if (found)
//         helpOss << "\n---\n";
//       helpOss << "- Type: External\n";
//       helpOss << "- Description: " << tool->getDescription();
//       helpOss << "\n" << tool->getAllUseCaseCap(2); // Show examples
//       found = true;
//     }
//     if (!found) {
//       helpOss
//           << "Tool '" << specificTool
//           << "' not found. Use 'help' with no parameters to list all tools.";
//     }
//   } else {
//     helpOss << "Available Tools:\n";
//     helpOss << "--- Internal Tools ---\n";
//     for (const auto &pair : internalToolDescriptions) {
//       helpOss << "- " << pair.first << ": " << pair.second << "\n";
//     }
//     helpOss << "\n--- External Tools ---\n";
//     if (tools.empty()) {
//       helpOss << "(No external tools registered)\n";
//     } else {
//       for (const auto &pair : tools) {
//         helpOss << "- " << pair.second->getName() << ": "
//                 << pair.second->getDescription() << "\n";
//       }
//     }
//     helpOss << "\nUse help with {\"tool_name\": \"<tool_name>\"} for details.";
//   }
//   return helpOss.str();
// }
//
// std::string Agent::generateStamp(void) {
//   auto now = std::chrono::system_clock::now();
//   auto now_c = std::chrono::system_clock::to_time_t(now);
//   std::tm now_tm = *std::localtime(&now_c);
//   // *** FIX: Declare buffer as a char array ***
//   char buffer[80]; // Increased size to safely hold the timestamp
//   // Use strftime correctly with the buffer array
//   if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S%Z", &now_tm)) {
//     // *** FIX: Construct string from the null-terminated buffer ***
//     return std::string(buffer);
//   } else {
//     // Handle error if strftime fails
//     return "[Timestamp Error]";
//   }
// }
//
```

### File: src/agent/core.cpp
```cpp
#include "../../inc/Agent.hpp"
#include "../../inc/MiniGemini.hpp"
#include "../../inc/Tool.hpp"
#include <algorithm> // For std::find_if, std::remove_if
#include <cctype>    // For std::toupper
#include <chrono>    // For timestamp
#include <cstdio>    // For FILE, fgets
#include <cstdlib>   // For popen, pclose, system
#include <ctime>
#include <fstream> // For file operations
#include <iomanip> // For formatting time
#include <iostream>
#include <json/json.h>
#include <sstream>
#include <stdexcept>

// --- JSON Parsing (NEW Structure) ---
bool Agent::parseStructuredLLMResponse(const std::string &jsonString,
                                       std::string &status,
                                       std::vector<StructuredThought> &thoughts,
                                       std::vector<ActionInfo> &actions,
                                       std::string &finalResponse) {
  status = "ERROR_INTERNAL";
  thoughts.clear();
  actions.clear();
  finalResponse = "";
  Json::Value root;
  Json::Reader reader;
  std::stringstream ss(jsonString);

  if (!reader.parse(ss, root, false)) {
    logMessage(LogLevel::ERROR,
               "Failed to parse LLM response as JSON for agent '" + name + "'",
               reader.getFormattedErrorMessages());
    finalResponse = jsonString;
    return false;
  }

  if (root.isMember("status") && root["status"].isString()) {
    status = root["status"].asString();
  } else {
    logMessage(LogLevel::ERROR,
               "LLM JSON missing required 'status' field for agent '" + name +
                   "'.",
               jsonString);
  }

  if (root.isMember("thoughts") && root["thoughts"].isArray()) {
    for (const auto &tv : root["thoughts"]) {
      if (tv.isObject() && tv.isMember("type") && tv["type"].isString() &&
          tv.isMember("content") && tv["content"].isString()) {
        thoughts.push_back({tv["type"].asString(), tv["content"].asString()});
      } else {
        logMessage(LogLevel::WARN,
                   "Malformed thought object in LLM JSON for agent '" + name +
                       "'. Skipping.",
                   tv.toStyledString());
      }
    }
  } else {
    logMessage(LogLevel::ERROR,
               "LLM JSON missing required 'thoughts' array for agent '" + name +
                   "'.",
               jsonString);
  }

  if (root.isMember("actions") && root["actions"].isArray()) {
    for (const auto &av : root["actions"]) {
      if (av.isObject() && av.isMember("action") && av["action"].isString() &&
          av.isMember("type") && av["type"].isString() &&
          av.isMember("params") && av["params"].isObject()) {
        ActionInfo ai;
        ai.action = av["action"].asString();
        ai.type = av["type"].asString();
        ai.params = av["params"];
        if (av.isMember("confidence") && av["confidence"].isNumeric()) {
          ai.confidence = av["confidence"].asDouble();
        }
        if (av.isMember("warnings") && av["warnings"].isArray()) {
          for (const auto &w : av["warnings"]) {
            if (w.isString())
              ai.warnings.push_back(w.asString());
          }
        }
        actions.push_back(ai);
      } else {
        logMessage(LogLevel::WARN,
                   "Malformed action object in LLM JSON for agent '" + name +
                       "'. Skipping.",
                   av.toStyledString());
      }
    }
  } else {
    logMessage(LogLevel::ERROR,
               "LLM JSON missing required 'actions' array for agent '" + name +
                   "'.",
               jsonString);
  }

  if (root.isMember("final_response") && root["final_response"].isString()) {
    finalResponse = root["final_response"].asString();
  } else if (root.isMember("final_response") &&
             root["final_response"].isNull()) {
    finalResponse = "";
  } else {
    logMessage(LogLevel::DEBUG, "LLM JSON missing 'final_response' field or "
                                "not string/null for agent '" +
                                    name + "'.");
  }

  if (status == "SUCCESS_FINAL" && !actions.empty()) {
    logMessage(LogLevel::WARN,
               "LLM JSON: Status SUCCESS_FINAL but actions array not empty for "
               "agent '" +
                   name + "'.",
               jsonString);
  }
  if ((status == "REQUIRES_ACTION" || status == "REQUIRES_CLARIFICATION") &&
      actions.empty()) {
    logMessage(LogLevel::WARN,
               "LLM JSON: Status requires action/clarification but actions "
               "array empty for agent '" +
                   name + "'.",
               jsonString);
  }

  return true;
}

// --- Action Processing (NEW Structure) ---
std::string Agent::processActions(const std::vector<ActionInfo> &actions) {
  std::stringstream resultsStream;
  resultsStream << "<action_results>\n";
  for (const auto &actionInfo : actions) {
    logMessage(LogLevel::TOOL_CALL, "Processing action '" + actionInfo.action +
                                        "' (type: " + actionInfo.type + ")");
    if (!actionInfo.warnings.empty()) {
      for (const auto &warn : actionInfo.warnings)
        logMessage(LogLevel::WARN, "Action Warning:", warn);
    }
    if (actionInfo.confidence < 0.95)
      logMessage(LogLevel::DEBUG,
                 "Action Confidence:", std::to_string(actionInfo.confidence));
    std::string result = processAction(actionInfo);
    logMessage(LogLevel::TOOL_RESULT,
               "Action result for '" + actionInfo.action + "'", result);
    resultsStream << "\t<action_result action=\"" << actionInfo.action
                  << "\" type=\"" << actionInfo.type << "\">\n";
    std::stringstream ss_result(result);
    std::string line;
    while (std::getline(ss_result, line)) {
      resultsStream << "\t\t" << line << "\n";
    }
    resultsStream << "\t</action_result>\n";
  }
  resultsStream << "</action_results>";
  return resultsStream.str();
}

std::string Agent::processAction(const ActionInfo &actionInfo) {
  if (actionInfo.type == "tool") {
    Tool *tool = getRegisteredTool(actionInfo.action);
    if (tool) {
      try {
        return tool->execute(actionInfo.params);
      } catch (const std::exception &e) {
        logMessage(LogLevel::ERROR, "Tool Exception: " + actionInfo.action,
                   e.what());
        return "Error: Tool Exception: " + std::string(e.what());
      } catch (...) {
        logMessage(LogLevel::ERROR, "Unknown Tool Exception",
                   actionInfo.action);
        return "Error: Unknown Tool Exception.";
      }
    } else {
      logMessage(LogLevel::ERROR, "Tool not found", actionInfo.action);
      return "Error: Tool '" + actionInfo.action + "' not registered.";
    }
  } else if (actionInfo.type == "internal_function") {
    if (actionInfo.action == "help")
      return getHelp(actionInfo.params);
    if (actionInfo.action == "skip")
      return skip(actionInfo.params);
    if (actionInfo.action == "promptAgent")
      return promptAgentTool(actionInfo.params);
    if (actionInfo.action == "summarizeTool")
      return summerizeTool(actionInfo.params); // Typo kept
    if (actionInfo.action == "summarizeHistory")
      return summarizeHistory(actionInfo.params);
    if (actionInfo.action == "getWeather")
      return getWeather(actionInfo.params);
    logMessage(LogLevel::ERROR, "Unknown internal function", actionInfo.action);
    return "Error: Unknown internal function '" + actionInfo.action + "'.";
  } else if (actionInfo.type == "script") {
    logMessage(LogLevel::WARN, "Script execution not implemented",
               actionInfo.action);
    return "Error: Script execution not supported.";
  } else if (actionInfo.type == "http_request") {
    logMessage(LogLevel::WARN, "HTTP request execution not implemented",
               actionInfo.action);
    return "Error: Direct HTTP requests not supported.";
  } else if (actionInfo.type == "output") {
    if (actionInfo.action == "send_response") {
      if (actionInfo.params.isMember("text") &&
          actionInfo.params["text"].isString()) {
        logMessage(LogLevel::INFO, "Executing send_response (text)",
                   actionInfo.params["text"].asString());
        return actionInfo.params["text"].asString();
      } // Add file_path, variable handling here (FUTURE)
      else {
        logMessage(LogLevel::ERROR, "Invalid params for send_response",
                   actionInfo.params.toStyledString());
        return "Error: Invalid params for send_response.";
      }
    } else {
      logMessage(LogLevel::ERROR, "Unknown output action", actionInfo.action);
      return "Error: Unknown output action '" + actionInfo.action + "'.";
    }
  } else if (actionInfo.type == "workflow_control") {
    logMessage(LogLevel::DEBUG, "Workflow control action handled by main loop",
               actionInfo.action);
    return "Action '" + actionInfo.action + "' acknowledged.";
  } else {
    logMessage(LogLevel::ERROR, "Unsupported action type",
               actionInfo.type + " for " + actionInfo.action);
    return "Error: Unsupported action type '" + actionInfo.type + "'.";
  }
}
```

### File: src/agent.cpp
```cpp
#include "../inc/Agent.hpp"      // Make sure this path is correct
#include "../inc/MiniGemini.hpp" // Make sure this path is correct
#include "../inc/Tool.hpp"       // Make sure this path is correct
#include "../inc/modelApi.hpp"   // For ApiError (ensure this exists)

#include <algorithm> // For std::find_if, std::remove_if
#include <chrono>    // For timestamp
#include <cstdlib>   // For system, getenv
#include <ctime>
#include <fstream> // For file operations
#include <iomanip> // For formatting time
#include <iostream>
#include <json/json.h> // Includes reader, writer, value
#include <map>
#include <memory> // For unique_ptr if used elsewhere
#include <sstream>
#include <stdexcept>
#include <vector>

// Simple colored logging function (assuming it's defined correctly, e.g., like
// previous versions) void logMessage(LogLevel level, const std::string
// &message, const std::string &details = "");
// --- Logging Function Placeholder ---
// Replace with your actual logging implementation
void logMessage(LogLevel level, const std::string &message,
                const std::string &details) {
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::localtime(&now_c);
  char time_buffer[20];
  std::strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", &now_tm);
  std::string prefix;
  std::string color_start = "";
  std::string color_end = "\033[0m";
  switch (level) {
  case LogLevel::DEBUG:
    prefix = "[DEBUG] ";
    color_start = "\033[36m";
    break;
  case LogLevel::INFO:
    prefix = "[INFO]  ";
    color_start = "\033[32m";
    break;
  case LogLevel::WARN:
    prefix = "[WARN]  ";
    color_start = "\033[33m";
    break;
  case LogLevel::ERROR:
    prefix = "[ERROR] ";
    color_start = "\033[1;31m";
    break;
  case LogLevel::TOOL_CALL:
    prefix = "[TOOL CALL] ";
    color_start = "\033[1;35m";
    break;
  case LogLevel::TOOL_RESULT:
    prefix = "[TOOL RESULT] ";
    color_start = "\033[35m";
    break;
  case LogLevel::PROMPT:
    prefix = "[PROMPT] ";
    color_start = "\033[34m";
    break;
  }
  std::ostream &out = (level == LogLevel::ERROR || level == LogLevel::WARN)
                          ? std::cerr
                          : std::cout;
  out << color_start << std::string(time_buffer) << " " << prefix << message
      << color_end << std::endl;
  if (!details.empty()) {
    const size_t max_detail_len = 500;
    std::string truncated_details = details.substr(0, max_detail_len);
    if (details.length() > max_detail_len)
      truncated_details += "... (truncated)";
    out << color_start << "  " << truncated_details << color_end << std::endl;
  }
}
// --- End Logging Function ---

// --- Agent Implementation ---

Agent::Agent(MiniGemini &apiRef)
    : api(apiRef), systemPrompt(R"(SYSTEM PROMPT: Base Agent

**Role:** Process user requests, utilize tools via actions, and provide responses.

**Interaction Model:**
You MUST respond with a single JSON object containing the following fields:
1.  `status`: (String - REQUIRED) Outcome hint: 'SUCCESS_FINAL', 'REQUIRES_ACTION', 'REQUIRES_CLARIFICATION', 'ERROR_INTERNAL'.
2.  `thoughts`: (Array of Objects - REQUIRED) Your structured reasoning, each object as `{"type": "TYPE", "content": "..."}`. Can be `[]`.
3.  `actions`: (Array of Objects - REQUIRED) Actions to execute now, each as `{"action": "...", "type": "...", "params": { ... }}`. MUST be `[]` if status is 'SUCCESS_FINAL'.
4.  `final_response`: (String | null - DEPRECATED?) Primarily use `send_response` action. Use this field ONLY if status is 'SUCCESS_FINAL' and 'actions' is `[]`. Set to `null` or `""` otherwise.

Adhere strictly to this JSON format (LLM Output Schema v0.3).
)"),
      iteration(0), iterationCap(10), skipFlowIteration(false),
      name("default_agent") {
  // Initialize internal tool descriptions
  internalToolDescriptions["help"] =
      "Provides descriptions of available tools/actions. Parameters: "
      "{\"action_name\": \"string\" (optional)}";
  internalToolDescriptions["skip"] =
      "Skips the final response generation for the current turn. Parameters: "
      "null (action implies skipping)";
  internalToolDescriptions["promptAgent"] =
      "Sends a prompt to another registered agent. Parameters: "
      "{\"agent_name\": \"string\", \"prompt\": \"string\"}";
  internalToolDescriptions["summarizeTool"] =
      "Summarizes provided text content. Parameters: {\"text\": \"string\"}";
  internalToolDescriptions["summarizeHistory"] =
      "Summarizes the current conversation history. Parameters: {}";
  internalToolDescriptions["getWeather"] =
      "Fetches current weather. Parameters: {\"location\": \"string\"}";
  // Add others as needed...
  logMessage(LogLevel::DEBUG, "Agent instance created", "Name: " + name);
}

// --- Configuration Setters ---
void Agent::setSystemPrompt(const std::string &prompt) {
  systemPrompt = prompt;
  // Optional: Add reminder check
  //   if (systemPrompt.find("\"status\"") == std::string::npos ||
  //       systemPrompt.find("\"thoughts\"") == std::string::npos ||
  //       systemPrompt.find("\"actions\"") == std::string::npos) {
  //     systemPrompt += R"(
  //
  // // Reminder: ALWAYS respond with a single JSON object matching the required
  // schema (status, thoughts, actions, final_response).
  // )";
  //   }
}
void Agent::setName(const std::string &newName) { name = newName; }
void Agent::setDescription(const std::string &newDescription) {
  description = newDescription;
}
void Agent::setIterationCap(int cap) { iterationCap = (cap > 0) ? cap : 1; }
void Agent::setDirective(const DIRECTIVE &dir) { directive = dir; }
void Agent::addTask(const std::string &task) { tasks.push_back(task); }
void Agent::addInitialCommand(const std::string &command) {
  initialCommands.push_back(command);
}

// --- Public Getters ---
const std::string Agent::getName() const { return name; }
const std::string Agent::getDescription() const { return description; }
MiniGemini &Agent::getApi() { return api; }
fileList Agent::getFiles() const { return files; }
const std::vector<std::pair<std::string, std::string>> &
Agent::getHistory() const {
  return history;
}
const std::string &Agent::getSystemPrompt() const { return systemPrompt; }
int Agent::getIterationCap() const { return iterationCap; }
const std::vector<std::pair<std::string, std::string>> &Agent::getEnv() const {
  return env;
}
const std::vector<std::string> &Agent::getExtraPrompts() const {
  return extraPrompts;
}
const std::vector<std::string> &Agent::getTasks() const { return tasks; }
const Agent::DIRECTIVE &Agent::getDirective() const { return directive; }

// --- Tool Management ---
void Agent::addTool(Tool *tool) {
  if (!tool) {
    logMessage(LogLevel::WARN, "Attempted to add a null tool.");
    return;
  }
  const std::string &toolName = tool->getName();
  if (toolName.empty()) {
    logMessage(LogLevel::WARN, "Attempted to add a tool with an empty name.");
    return;
  }
  if (tools.count(toolName) || internalToolDescriptions.count(toolName)) {
    logMessage(LogLevel::WARN,
               "Attempted to add tool/action with duplicate name: '" +
                   toolName + "'. Ignoring.");
  } else {
    tools[toolName] = tool; // Use map assignment
    internalToolDescriptions[toolName] =
        tool->getDescription(); // Store description
    logMessage(LogLevel::INFO,
               "Agent '" + name + "' added tool: '" + toolName + "'");
  }
}

void Agent::removeTool(const std::string &toolName) {
  if (tools.erase(toolName)) {
    internalToolDescriptions.erase(toolName);
    logMessage(LogLevel::INFO,
               "Agent '" + name + "' removed tool: '" + toolName + "'");
  } else {
    logMessage(LogLevel::WARN,
               "Agent '" + name + "' attempted to remove non-existent tool: '" +
                   toolName + "'");
  }
}

Tool *Agent::getTool(const std::string &toolName) const {
  auto it = tools.find(toolName);
  return (it != tools.end()) ? it->second : nullptr;
}

Tool *Agent::getRegisteredTool(const std::string &toolName) const {
  auto it = tools.find(toolName);
  return (it != tools.end()) ? it->second : nullptr;
}

std::string
Agent::getInternalToolDescription(const std::string &toolName) const {
  auto it_internal = internalToolDescriptions.find(toolName);
  if (it_internal != internalToolDescriptions.end()) {
    return it_internal->second;
  }
  auto it_external = tools.find(toolName);
  if (it_external != tools.end() && it_external->second) {
    return it_external->second->getDescription();
  }
  return "";
}

// --- Core Agent Logic ---
void Agent::reset() {
  history.clear();
  scratchpad.clear();
  iteration = 0;
  skipFlowIteration = false;
  logMessage(LogLevel::INFO, "Agent '" + name + "' reset.");
}

void Agent::setSchema(const std::string &schema) { this->schema = schema; }

std::string Agent::executeApiCall(const std::string &fullPrompt) {
  logMessage(LogLevel::PROMPT,
             "Sending prompt to API for agent '" + name + "'");
  // logMessage(LogLevel::DEBUG, "Full Prompt:", fullPrompt); // Uncomment for
  // debugging
  try {
    std::string response = api.generate(fullPrompt);
    logMessage(LogLevel::DEBUG,
               "Received API response for agent '" + name + "'");
    return response;
  } catch (const ApiError &e) {
    logMessage(LogLevel::ERROR,
               "API Error occurred for agent '" + name + "':", e.what());
    throw;
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR,
               "Standard exception during API call for agent '" + name + "':",
               e.what());
    throw std::runtime_error("Error during API call: " + std::string(e.what()));
  } catch (...) {
    logMessage(LogLevel::ERROR,
               "Unknown exception during API call for agent '" + name + "'.");
    throw std::runtime_error("Unknown error during API call");
  }
}

// --- Deprecated / Fallback Methods ---
std::vector<Agent::ToolCallInfo>
Agent::extractToolCalls(const std::string &response) {
  logMessage(LogLevel::WARN,
             "Using fallback simple tool call extraction for agent '" + name +
                 "'.");
  std::vector<ToolCallInfo> calls;
  std::string startDelimiter = "```tool:";
  std::string endDelimiter = "```";
  size_t startPos = response.find(startDelimiter);
  while (startPos != std::string::npos) {
    size_t endPos =
        response.find(endDelimiter, startPos + startDelimiter.length());
    if (endPos == std::string::npos)
      break;
    std::string toolBlock =
        response.substr(startPos + startDelimiter.length(),
                        endPos - (startPos + startDelimiter.length()));
    // Basic parsing - assumes toolName\n{JSON_PARAMS}
    size_t firstNewline = toolBlock.find('\n');
    if (firstNewline == std::string::npos) {
      startPos = response.find(startDelimiter, endPos);
      continue;
    }
    ToolCallInfo callInfo;
    callInfo.toolName = toolBlock.substr(0, firstNewline);
    std::string jsonParamsStr = toolBlock.substr(firstNewline + 1);
    Json::Value paramsJson;
    Json::Reader reader;
    if (reader.parse(jsonParamsStr, paramsJson)) {
      callInfo.params = paramsJson;
      calls.push_back(callInfo);
    } // Fixed placeholder
    else {
      logMessage(LogLevel::ERROR,
                 "Failed to parse JSON params in tool block (fallback)",
                 reader.getFormattedErrorMessages());
    }
    startPos = response.find(startDelimiter, endPos);
  }
  return calls;
}

std::string
Agent::processToolCalls(const std::vector<ToolCallInfo> &toolCalls) {
  logMessage(LogLevel::WARN,
             "Using deprecated processToolCalls for agent '" + name + "'.");
  std::stringstream resultsStream;
  resultsStream << "<tool_results>\n";
  for (const auto &call : toolCalls) {
    logMessage(LogLevel::TOOL_CALL, "Processing OLD tool call format",
               call.toolName);
    std::string result = handleToolExecution(call); // Delegate to old handler
    logMessage(LogLevel::TOOL_RESULT, "OLD Tool result",
               call.toolName + ": " + result);
    resultsStream << "\t<tool_result tool_name=\"" << call.toolName << "\">\n";
    std::stringstream ss_result(result);
    std::string line;
    while (std::getline(ss_result, line)) {
      resultsStream << "\t\t" << line << "\n";
    }
    resultsStream << "\t</tool_result>\n";
  }
  resultsStream << "</tool_results>";
  return resultsStream.str();
}

std::string Agent::handleToolExecution(const ToolCallInfo &call) {
  logMessage(LogLevel::WARN,
             "Using deprecated handleToolExecution for agent '" + name + "'.",
             call.toolName);
  ActionInfo actionInfo;
  actionInfo.action = call.toolName;
  actionInfo.params = call.params;
  if (tools.count(call.toolName)) {
    actionInfo.type = "tool";
  } else if (internalToolDescriptions.count(call.toolName)) {
    actionInfo.type = "internal_function";
  } else {
    actionInfo.type = "unknown";
    logMessage(LogLevel::WARN, "Cannot determine type for fallback tool call",
               call.toolName);
  }
  return processAction(actionInfo); // Delegate to new processor
}

// --- Internal Tool Implementations (Fixing signatures) ---

std::string Agent::getHelp(const Json::Value &params) { // Added &
  std::string targetAction = "";
  if (params.isMember("action_name") && params["action_name"].isString()) {
    targetAction = params["action_name"].asString();
  }
  std::stringstream help;
  if (!targetAction.empty()) {
    std::string desc = getInternalToolDescription(targetAction);
    if (!desc.empty()) {
      help << "Help for action '" << targetAction << "':\n" << desc;
    } else {
      help << "Error: No help found for action '" << targetAction << "'.";
    }
  } else {
    help << "Available actions/tools:\n";
    std::map<std::string, std::string> availableActions =
        internalToolDescriptions;
    for (const auto &pair : tools) {
      if (pair.second) {
        availableActions[pair.first] = pair.second->getDescription();
      }
    }
    if (availableActions.empty()) {
      help << "- No actions/tools currently registered.\n";
    } else {
      for (const auto &pair : availableActions) {
        help << "- " << pair.first << ": " << pair.second << "\n";
      }
    }
  }
  return help.str();
}

std::string Agent::skip(const Json::Value &params) { // Added &
  (void)params;
  logMessage(LogLevel::INFO,
             "Skip action called for agent '" + name + "'. Ending turn.");
  setSkipFlowIteration(true);
  return "Skipping remainder of turn.";
}

std::string Agent::promptAgentTool(const Json::Value &params) { // Added &
  if (!params.isMember("agent_name") || !params["agent_name"].isString() ||
      !params.isMember("prompt") || !params["prompt"].isString()) {
    return "Error: promptAgent action requires 'agent_name' (string) and "
           "'prompt' (string) parameters.";
  }
  std::string agentName = params["agent_name"].asString();
  std::string userPrompt = params["prompt"].asString();
  Agent *subAgent = nullptr;
  for (const auto &pair : subAgents) {
    if (pair.first == agentName) {
      subAgent = pair.second;
      break;
    }
  }
  if (subAgent) {
    logMessage(LogLevel::INFO,
               "Agent '" + name + "' prompting sub-agent '" + agentName + "'");
    return subAgent->prompt(userPrompt);
  } else {
    return "Error: Sub-agent '" + agentName + "' not found.";
  }
}

std::string
Agent::summerizeTool(const Json::Value &params) { // Added & - Typo preserved
  if (!params.isMember("text") || !params["text"].isString()) {
    return "Error: summarizeTool action requires 'text' (string) parameter.";
  }
  std::string textToSummarize = params["text"].asString();
  logMessage(LogLevel::INFO, "Summarize tool called by agent '" + name + "'.");
  std::string summarizationPrompt =
      "Please summarize the following text concisely:\n\n" + textToSummarize;
  try {
    logMessage(LogLevel::WARN,
               "Summarization uses placeholder implementation.");
    return "Summary of: " + textToSummarize.substr(0, 30) +
           "... (Placeholder implementation)";
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR, "Summarization API call failed", e.what());
    return "Error during summarization API call: " + std::string(e.what());
  }
}

std::string
Agent::summarizeHistory(const Json::Value &params) { // Added & and fixed name
  (void)params;
  logMessage(LogLevel::INFO,
             "Summarize history tool called by agent '" + name + "'.");
  std::stringstream historyStream;
  for (const auto &entry : history) {
    historyStream << entry.first << ": " << entry.second << "\n---\n";
  }
  std::string historyStr = historyStream.str();
  if (historyStr.length() < 100) {
    return "History is too short to summarize meaningfully.";
  }
  std::string summarizationPrompt = "Please summarize the key points of the "
                                    "following conversation history:\n\n" +
                                    historyStr;
  try {
    logMessage(LogLevel::WARN,
               "History Summarization uses placeholder implementation.");
    return "Summary of conversation history... (Placeholder implementation)";
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR, "History summarization API call failed",
               e.what());
    return "Error during history summarization API call: " +
           std::string(e.what());
  }
}

std::string
Agent::getWeather(const Json::Value &params) { // Added & and fixed name
  if (!params.isMember("location") || !params["location"].isString()) {
    return "Error: getWeather action requires 'location' (string) parameter.";
  }
  std::string location = params["location"].asString();
  logMessage(LogLevel::INFO,
             "Get weather tool called by agent '" + name + "' for", location);
  logMessage(LogLevel::WARN, "Get weather uses placeholder implementation.");
  return "Weather in " + location +
         " is sunny, 25°C (Placeholder implementation).";
}

// --- Utility Methods (add*, remove*, import*, wrap*, run) remain largely the
// same, using clean names ---
// ... (Implementations for addToHistory, addMemory, removeMemory, addEnvVar,
// importEnvFile, addPrompt, addAgent, agentInstance, setSkipFlowIteration, run)
// ... (Ensure they use clean names like history, longTermMemory, env,
// extraPrompts, subAgents internally) Example addToHistory already shown above
// uses clean names.

void Agent::addToHistory(const std::string &role, const std::string &content) {
  const size_t max_history_content_len = 2500;
  std::string processed_content = content.substr(0, max_history_content_len);
  bool truncated = (content.length() > max_history_content_len);
  if (truncated)
    processed_content += "... (truncated)";
  history.push_back({role, processed_content});
  logMessage(LogLevel::DEBUG, "Added to history for agent '" + name + "'",
             "Role: " + role + (truncated ? " (Content Truncated)" : ""));
}
void Agent::addMemory(const std::string &role, const std::string &content) {
  longTermMemory.push_back({role, content});
  logMessage(LogLevel::DEBUG,
             "Added to LongTermMemory for agent '" + name + "'",
             "Role: " + role);
}
void Agent::removeMemory(const std::string &role, const std::string &content) {
  auto it = std::remove_if(longTermMemory.begin(), longTermMemory.end(),
                           [&role, &content](const auto &p) {
                             return p.first == role && p.second == content;
                           });
  if (it != longTermMemory.end()) {
    longTermMemory.erase(it, longTermMemory.end());
    logMessage(LogLevel::DEBUG,
               "Removed from LongTermMemory for agent '" + name + "'",
               "Role: " + role);
  }
}
void Agent::addEnvVar(const std::string &key, const std::string &value) {
  bool found = false;
  for (auto &pair : env) {
    if (pair.first == key) {
      pair.second = value;
      found = true;
      logMessage(LogLevel::DEBUG, "Updated env var for agent '" + name + "'",
                 key);
      break;
    }
  }
  if (!found) {
    env.push_back({key, value});
    logMessage(LogLevel::DEBUG, "Set env var for agent '" + name + "'", key);
  }
}
void Agent::importEnvFile(const std::string &filePath) {
  std::ifstream file(filePath);
  if (!file.is_open()) {
    logMessage(LogLevel::ERROR,
               "Agent '" + name + "' could not open env file:", filePath);
    return;
  }
  std::string line;
  int count = 0;
  while (std::getline(file, line)) {
    line.erase(0, line.find_first_not_of(" \t"));
    if (line.empty() || line[0] == '#')
      continue;
    size_t pos = line.find('=');
    if (pos != std::string::npos) {
      std::string key = line.substr(0, pos);
      key.erase(key.find_last_not_of(" \t") + 1);
      std::string value = line.substr(pos + 1);
      value.erase(0, value.find_first_not_of(" \t"));
      value.erase(value.find_last_not_of(" \t") + 1);
      if (value.length() >= 2 &&
          ((value.front() == '"' && value.back() == '"') ||
           (value.front() == '\'' && value.back() == '\''))) {
        value = value.substr(1, value.length() - 2);
      }
      if (!key.empty()) {
        addEnvVar(key, value);
        count++;
      }
    }
  }
  file.close();
  logMessage(LogLevel::INFO,
             "Agent '" + name + "' imported " + std::to_string(count) +
                 " env vars from:",
             filePath);
}
void Agent::addPrompt(const std::string &prompt) {
  extraPrompts.push_back(prompt);
  logMessage(LogLevel::DEBUG, "Added extra prompt for agent '" + name + "'.");
}
void Agent::addAgent(Agent *agent) {
  if (!agent) {
    logMessage(LogLevel::WARN, "Attempted to add null agent.");
    return;
  }
  if (agent == this) {
    logMessage(LogLevel::WARN, "Agent '" + name + "' cannot add self.");
    return;
  }
  for (const auto &pair : subAgents) {
    if (pair.first == agent->getName()) {
      logMessage(LogLevel::WARN, "Agent '" + name +
                                     "' duplicate sub-agent name: '" +
                                     agent->getName() + "'. Ignoring.");
      return;
    }
  }
  subAgents.push_back({agent->getName(), agent});
  logMessage(LogLevel::INFO, "Agent '" + name + "' registered sub-agent: '" +
                                 agent->getName() + "'");
}
std::string Agent::agentInstance(const std::string &agentName) {
  for (const auto &pair : subAgents) {
    if (pair.first == agentName)
      return pair.second->getName();
  }
  return "";
}
void Agent::setSkipFlowIteration(bool skip) { skipFlowIteration = skip; }
void Agent::run() {
  logMessage(LogLevel::INFO, "Agent '" + name + "' starting interactive loop.");
  logMessage(LogLevel::INFO,
             "Type 'exit' or 'quit' to stop, 'reset' to clear history.");
  std::string userInput;
  if (!initialCommands.empty()) {
    logMessage(LogLevel::INFO,
               "Executing initial commands for agent '" + name + "'...");
    Json::Value bashParams;
    std::vector<std::string> commandsToRun = initialCommands;
    initialCommands.clear();
    for (const auto &command : commandsToRun) {
      bashParams["command"] = command;
      logMessage(LogLevel::INFO, "Running initial command:", command);
      try {
        std::string result = manualToolCall("bash", bashParams);
        logMessage(LogLevel::INFO, "Initial command result:", result);
      } catch (const std::exception &e) {
        logMessage(
            LogLevel::ERROR,
            "Error running initial command '" + command + "':", e.what());
      } catch (...) {
        logMessage(LogLevel::ERROR,
                   "Unknown error running initial command '" + command + "'");
      }
    }
  }
  while (true) {
    std::cout << "\nUser (" << name << ") > ";
    if (!std::getline(std::cin, userInput)) {
      logMessage(LogLevel::INFO,
                 "Input stream closed. Exiting agent '" + name + "'.");
      break;
    }
    userInput.erase(0, userInput.find_first_not_of(" \t\r\n"));
    userInput.erase(userInput.find_last_not_of(" \t\r\n") + 1);
    if (userInput == "exit" || userInput == "quit") {
      logMessage(LogLevel::INFO, "Exit command received. Goodbye!");
      break;
    } else if (userInput == "reset") {
      reset();
      continue;
    } else if (userInput.empty()) {
      continue;
    }
    try {
      std::string response = prompt(userInput);
      std::cout << "\n" << name << ":\n" << response << std::endl;
      std::cout << "-----------------------------------------" << std::endl;
    } catch (const ApiError &e) {
      logMessage(LogLevel::ERROR, "API Error:", e.what());
      std::cout << "\n[Agent Error - API]: " << e.what() << std::endl;
      std::cout << "-----------------------------------------" << std::endl;
    } catch (const std::exception &e) {
      logMessage(LogLevel::ERROR, "General Error:", e.what());
      std::cout << "\n[Agent Error - General]: " << e.what() << std::endl;
      std::cout << "-----------------------------------------" << std::endl;
    } catch (...) {
      logMessage(LogLevel::ERROR, "Unknown Error.");
      std::cout << "\n[Agent Error - Unknown]" << std::endl;
      std::cout << "-----------------------------------------" << std::endl;
    }
  }
  logMessage(LogLevel::INFO, "Agent '" + name + "' interactive loop finished.");
}

// --- executeTask method ---
std::string Agent::executeTask(const std::string &task,
                               const std::string &format) {
  logMessage(LogLevel::DEBUG,
             "Executing task with specific format for agent '" + name + "'",
             "Task: " + task);
  std::string taskPrompt = systemPrompt;
  taskPrompt += "\n\n<current_task>\n";
  taskPrompt += "\t<description>" + task + "</description>\n";
  taskPrompt += "\t<required_format>" + format + "</required_format>\n";
  taskPrompt += "</current_task>\n\n";
  taskPrompt += "RESPONSE INSTRUCTIONS: Execute the task and respond ONLY with "
                "the result in the specified format within 'final_response', "
                "status='SUCCESS_FINAL', actions='[]'.";
  try {
    std::string llmResponseJson = executeApiCall(taskPrompt);
    std::string status, finalResponse;
    std::vector<StructuredThought> thoughts;
    std::vector<ActionInfo> actions;
    if (parseStructuredLLMResponse(llmResponseJson, status, thoughts, actions,
                                   finalResponse) &&
        (status == "SUCCESS_FINAL" || !finalResponse.empty())) {
      return finalResponse;
    } else {
      logMessage(LogLevel::ERROR,
                 "Task execution failed or yielded invalid response",
                 llmResponseJson);
      return "Error: Task execution failed to produce expected result.";
    }
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR, "Failed to execute task via API", e.what());
    return "Error: Failed to execute task: " + std::string(e.what());
  }
}
// Overloads for executeTask (assuming Json::Value version might be needed by
// tools)
// std::string Agent::executeTask(const std::string &task) {
//   return executeTask(task, "Provide the result directly as plain text.");
// } // Default format
std::string Agent::executeTask(const std::string &task,
                               const Json::Value &format) {
  return executeTask(task, format.toStyledString());
} // Convert JSON format to string

// --- manualToolCall (signature corrected) ---
std::string
Agent::manualToolCall(const std::string &toolName,
                      const Json::Value &params) { // Added & and fixed name
  logMessage(LogLevel::INFO, "Manually calling tool '" + toolName +
                                 "' for agent '" + name + "'");
  ActionInfo actionInfo;
  actionInfo.action = toolName;
  if (!params.isObject()) {
    logMessage(LogLevel::ERROR,
               "Manual tool call params not JSON object for agent '" + name +
                   "'.",
               params.toStyledString());
    return "Error: Manual tool call params must be JSON object.";
  }
  actionInfo.params = params;
  if (tools.count(toolName)) {
    actionInfo.type = "tool";
  } else if (internalToolDescriptions.count(toolName)) {
    actionInfo.type = "internal_function";
  } else {
    logMessage(LogLevel::WARN,
               "Cannot determine type for manual tool call. Assuming 'tool'.",
               toolName);
    actionInfo.type = "tool";
  }
  return processAction(actionInfo);
}

// --- Other Wrappers / Placeholders ---
std::string Agent::wrapText(const std::string &text) {
  return "```\n" + text + "\n```";
}
std::string Agent::wrapXmlLine(const std::string &content,
                               const std::string &tag) {
  return "<" + tag + ">" + content + "</" + tag + ">\n";
}
std::string Agent::wrapXmlBlock(const std::string &content,
                                const std::string &tag) {
  return "<" + tag + ">\n\t" + content + "\n</" + tag + ">\n";
}
void Agent::addBlockToSystemPrompt(const std::string &content,
                                   const std::string &tag) {
  systemPrompt += "\n" + wrapXmlBlock(content, tag);
}
std::string Agent::reportTool(const Json::Value &params) {
  (void)params;
  logMessage(LogLevel::WARN,
             "'reportTool' not implemented for agent '" + name + "'.");
  return "Error: reportTool not implemented.";
}
std::string Agent::generateCode(const Json::Value &params) {
  (void)params;
  logMessage(LogLevel::WARN,
             "'generateCode' not implemented for agent '" + name + "'.");
  return "Error: generateCode not implemented.";
}
std::string Agent::auditResponse(const std::string &response) {
  (void)response;
  logMessage(LogLevel::WARN,
             "'auditResponse' not implemented for agent '" + name + "'.");
  return response;
}

// --- Directive Type to String Helper ---
std::string Agent::directiveTypeToString(Agent::DIRECTIVE::Type type) const {
  switch (type) {
  case DIRECTIVE::Type::BRAINSTORMING:
    return "BRAINSTORMING";
  case DIRECTIVE::Type::AUTONOMOUS:
    return "AUTONOMOUS";
  case DIRECTIVE::Type::NORMAL:
    return "NORMAL";
  case DIRECTIVE::Type::EXECUTE:
    return "EXECUTE";
  case DIRECTIVE::Type::REPORT:
    return "REPORT";
  default:
    logMessage(LogLevel::WARN, "Unknown directive type encountered");
    return "UNKNOWN";
  }
}

// --- Timestamp Generation Utility ---
std::string Agent::generateStamp(void) {
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  // Use std::localtime for local time formatting
  ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
  return ss.str();
}
```

### File: src/agent/export.cpp
```cpp
#include "../../inc/Agent.hpp" // Your Agent class header
#include <string>
#include <vector>
#include <fstream>
#include <iostream> // For basic error output in this example
#include <stdexcept>

// Include yaml-cpp headers
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/emitter.h> // For exporting
                              
// --- Export Function ---
// Saves the current configuration OF an Agent object TO a YAML file.
// Only saves configurable profile aspects, not full runtime state.
// Returns true on success, false on failure.
bool saveAgentProfile(const Agent& agentToSave, const std::string& yamlPath) {
    logMessage(LogLevel::INFO, "Attempting to save agent profile", yamlPath);
    YAML::Emitter emitter;
    try {
        emitter << YAML::BeginMap;

        // Add a version marker (good practice)
        emitter << YAML::Key << "version" << YAML::Value << "agent-1.1"; // Example version

        // --- Save Core Identity & Configuration ---
        emitter << YAML::Key << "name" << YAML::Value << agentToSave.getName();
        emitter << YAML::Key << "description" << YAML::Value << YAML::Literal << agentToSave.getDescription(); // Use Literal for multi-line
        emitter << YAML::Key << "system_prompt" << YAML::Value << YAML::Literal << agentToSave.getSystemPrompt(); // Use Literal for multi-line
        emitter << YAML::Key << "iteration_cap" << YAML::Value << agentToSave.getIterationCap(); // Assuming getter exists

        // --- Save Environment ---
        // Assuming Agent has a way to get environment variables, e.g., getEnv() returning a map or vector<pair>
        const auto& envVars = agentToSave.getEnv(); // *** ASSUMPTION: getEnv() exists and returns suitable type ***
        if (!envVars.empty()) {
            emitter << YAML::Key << "environment" << YAML::Value << YAML::BeginMap;
            for (const auto& pair : envVars) {
                emitter << YAML::Key << pair.first << YAML::Value << pair.second;
            }
            emitter << YAML::EndMap;
        }

        // --- Save Optional Sections ---
        // Assuming Agent has getters for these, e.g., getExtraPrompts(), getTasks()
        const auto& extraPrompts = agentToSave.getExtraPrompts(); // *** ASSUMPTION: getExtraPrompts() exists ***
        if (!extraPrompts.empty()) {
            emitter << YAML::Key << "extra_prompts" << YAML::Value << YAML::BeginSeq;
            for (const auto& prompt : extraPrompts) {
                emitter << prompt;
            }
            emitter << YAML::EndSeq;
        }
        const auto& tasks = agentToSave.getTasks(); // *** ASSUMPTION: getTasks() exists ***
        if (!tasks.empty()) {
            emitter << YAML::Key << "tasks" << YAML::Value << YAML::BeginSeq;
            for (const auto& task : tasks) {
                emitter << task;
            }
            emitter << YAML::EndSeq;
        }

        // --- Save Directive ---
        const auto& directive = agentToSave.getDirective(); // *** ASSUMPTION: getDirective() exists ***
        emitter << YAML::Key << "directive" << YAML::Value << YAML::BeginMap;
        // Convert enum back to string
        std::string typeStr = "NORMAL"; // Default
        switch (directive.type) {
            case Agent::DIRECTIVE::Type::BRAINSTORMING: typeStr = "BRAINSTORMING"; break;
            case Agent::DIRECTIVE::Type::AUTONOMOUS: typeStr = "AUTONOMOUS"; break;
            case Agent::DIRECTIVE::Type::EXECUTE: typeStr = "EXECUTE"; break;
            case Agent::DIRECTIVE::Type::REPORT: typeStr = "REPORT"; break;
            case Agent::DIRECTIVE::Type::NORMAL: // Fallthrough
            default: typeStr = "NORMAL"; break;
        }
        emitter << YAML::Key << "type" << YAML::Value << typeStr;
        emitter << YAML::Key << "description" << YAML::Value << directive.description;
        emitter << YAML::Key << "format" << YAML::Value << directive.format;
        emitter << YAML::EndMap;

        // --- What is NOT saved ---
        // - Runtime state (iteration, skipFlowIteration, scratchpad)
        // - History
        // - Memory state (long/short term)
        // - Tool definitions/instances (might save *list* of expected tool *names* if needed)
        // - Sub-agent references/definitions

        emitter << YAML::EndMap;

        // Write to file
        std::ofstream fout(yamlPath);
        if (!fout.is_open()) {
            logMessage(LogLevel::ERROR, "Failed to open file for saving agent profile", yamlPath);
            return false;
        }
        fout << emitter.c_str();
        fout.close();

        if (!emitter.good()) {
             logMessage(LogLevel::ERROR, "YAML emitter error after saving agent profile", emitter.GetLastError());
             return false; // Indicate potential issue even if file write succeeded superficially
        }

        logMessage(LogLevel::INFO, "Successfully saved agent profile", yamlPath);
        return true;

    } catch (const YAML::Exception& e) {
        logMessage(LogLevel::ERROR, "YAML emitter error during agent profile save", yamlPath + ": " + e.what());
        return false;
    } catch (const std::exception& e) {
        logMessage(LogLevel::ERROR, "Generic error saving agent profile", yamlPath + ": " + e.what());
        return false;
    }
}


```

### File: src/agent/import.cpp
```cpp
#include "../../inc/Agent.hpp" // Your Agent class header
#include <fstream>
#include <iostream> // For basic error output in this example
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

// Include yaml-cpp headers
#include <yaml-cpp/emitter.h> // For exporting
#include <yaml-cpp/yaml.h>

// --- Utility Function ---

// implicite extraction, strings and scalars are treated as strings. the issue
// is that a string can contain a file/folder/executable those will be handled

// Loads configuration FROM a YAML file INTO an existing Agent object.
// Overwrites relevant configuration fields on the agent object.
// Returns true on success, false on failure.
bool loadAgentProfile(Agent &agentToConfigure, const std::string &yamlPath) {
  logMessage(LogLevel::INFO, "Attempting to load agent profile", yamlPath);
  YAML::Node config;
  try {
    // Check if file exists first (LoadFile throws if not found, but good
    // practice)
    std::ifstream f(yamlPath.c_str());
    if (!f.good()) {
      logMessage(LogLevel::ERROR, "Agent profile file not found", yamlPath);
      return false;
    }
    f.close();

    config = YAML::LoadFile(yamlPath);

    // --- Load Core Identity & Configuration ---
    if (config["name"] && config["name"].IsScalar()) {
      agentToConfigure.setName(config["name"].as<std::string>());
    }
    if (config["description"] && config["description"].IsScalar()) {
      agentToConfigure.setDescription(config["description"].as<std::string>());
    }
    if (config["system_prompt"] && config["system_prompt"].IsScalar()) {
      agentToConfigure.setSystemPrompt(
          config["system_prompt"].as<std::string>());
    }
    if (config["schema"] && config["schema"].IsScalar()) {
      agentToConfigure.setSchema(config["schema"].as<std::string>());
    }
    if (config["example"] && config["example"].IsScalar()) {
      agentToConfigure.setExample(config["example"].as<std::string>());
    }
    if (config["iteration_cap"] && config["iteration_cap"].IsScalar()) {
      try {
        agentToConfigure.setIterationCap(config["iteration_cap"].as<int>());
      } catch (const YAML::BadConversion &e) { // <-- CHANGE IS HERE
        logMessage(LogLevel::WARN,
                   "Invalid iteration_cap value in profile (bad conversion), "
                   "using agent's default",
                   yamlPath + ": " + e.what());
      }
    }

    if (config["extra_prompts"] && config["extra_prompts"].IsSequence()) {
      // agentToConfigure.clearExtraPrompts(); // Example if overwrite needed
      for (const auto &item : config["extra_prompts"]) {
        if (item.IsScalar()) {
          agentToConfigure.addPrompt(item.as<std::string>());
        }
      }
    }

    // if (config["variables"] && config["variables"].IsMap()) {
    //   for (const auto &it : config["variables"]) {
    //     std::string key = it.first.as<std::string>();
    //     std::string value = it.second.as<std::string>();
    //
    //     if (extractType(key) == "file") {
    //
    //       // } else if (extractType(key) == "tool") {
    //     } else if (extractType(key) == "folder") { // should use the
    //       extractType methoe in order to recursivaly import the files
    //           apropriatly depending on the type
    //     } else if (extractType(key) == "executable") {
    //     } else if (extractType(key) == "string") {
    //     }
    //
    //     agentToConfigure.addEnvVar(key, value);
    //   }
    //   m
    // }

    // --- Load Environment ---
    // Assuming Agent might have a clearEnvVars() or similar if overwrite is
    // desired, otherwise this just adds/updates variables.
    if (config["environment"] && config["environment"].IsMap()) {
      for (const auto &it : config["environment"]) {
        std::string key = it.first.as<std::string>();
        std::string value = it.second.as<std::string>();
        agentToConfigure.addEnvVar(key, value);
      }
    }

    // --- Load Optional Sections ---
    // Assuming Agent might have clear methods if overwrite is desired.
    if (config["extra_prompts"] && config["extra_prompts"].IsSequence()) {
      // agentToConfigure.clearExtraPrompts(); // Example if overwrite needed
      for (const auto &item : config["extra_prompts"]) {
        if (item.IsScalar()) {
          agentToConfigure.addPrompt(item.as<std::string>());
        }
      }
    }
    if (config["tasks"] && config["tasks"].IsSequence()) {
      // agentToConfigure.clearTasks(); // Example if overwrite needed
      for (const auto &item : config["tasks"]) {
        if (item.IsScalar()) {
          agentToConfigure.addTask(item.as<std::string>());
        }
      }
    }

    // --- Load Directive ---
    if (config["directive"] && config["directive"].IsMap()) {
      Agent::DIRECTIVE dir; // Assuming Agent::DIRECTIVE is accessible or has
                            // public members/setters
      const auto &dirNode = config["directive"];
      if (dirNode["type"] && dirNode["type"].IsScalar()) {
        std::string typeStr = dirNode["type"].as<std::string>();
        // Map string to enum (example logic, adjust based on your
        // Agent::DIRECTIVE)
        if (typeStr == "BRAINSTORMING")
          dir.type = Agent::DIRECTIVE::Type::BRAINSTORMING;
        else if (typeStr == "AUTONOMOUS")
          dir.type = Agent::DIRECTIVE::Type::AUTONOMOUS;
        else if (typeStr == "EXECUTE")
          dir.type = Agent::DIRECTIVE::Type::EXECUTE;
        else if (typeStr == "REPORT")
          dir.type = Agent::DIRECTIVE::Type::REPORT;
        else
          dir.type = Agent::DIRECTIVE::Type::NORMAL; // Default
      }
      if (dirNode["description"] && dirNode["description"].IsScalar()) {
        dir.description = dirNode["description"].as<std::string>();
      }
      if (dirNode["format"] && dirNode["format"].IsScalar()) {
        dir.format = dirNode["format"].as<std::string>();
      }
      agentToConfigure.setDirective(
          dir); // Assuming setDirective takes the struct
    }

    // --- What is NOT loaded from profile ---
    // - Runtime state (iteration, skipFlowIteration, scratchpad)
    // - History
    // - Memory (long/short term - unless explicitly defined in YAML and
    // loader handles it)
    // - Registered Tool* pointers (profile might *list* expected tools, but
    // instances aren't loaded)
    // - Registered Agent* pointers (sub-agents)

    logMessage(LogLevel::INFO, "Successfully loaded agent profile", yamlPath);
    return true;

  } catch (const YAML::Exception &e) {
    logMessage(LogLevel::ERROR, "Failed to load or parse agent profile YAML",
               yamlPath + ": " + e.what());
    return false;
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR, "Generic error loading agent profile",
               yamlPath + ": " + e.what());
    return false;
  }
}
```

### File: src/agent/parse.cpp
```cpp
// #include "../../inc/Agent.hpp"
// #include "../../inc/MiniGemini.hpp"
// #include "../../inc/Tool.hpp"
// #include <algorithm> // For std::find_if, std::remove_if
// #include <cctype>    // For std::toupper
// #include <chrono>    // For timestamp
// #include <cstdio>    // For FILE, fgets
// #include <cstdlib>   // For popen, pclose, system
// #include <ctime>
// #include <fstream> // For file operations
// #include <iomanip> // For formatting time
// #include <iostream>
// #include <json/json.h>
// #include <sstream>
// #include <stdexcept>
//
// #include <string>
// #include <sys/stat.h> // For file operations
// #include <unistd.h>   // For getcwd
//                       
// // Helper function to parse the structured LLM response
// bool Agent::parseStructuredLLMResponse(const std::string &jsonString,
//                                        std::string &thought,
//                                        std::vector<ToolCallInfo> &toolCalls,
//                                        std::string &finalResponse) {
//   Json::Value root;
//   Json::CharReaderBuilder readerBuilder;
//   std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
//   std::string errs;
//
//   toolCalls.clear();
//   finalResponse = "";
//   thought = "";
//
//   if (!reader->parse(jsonString.c_str(),
//                      jsonString.c_str() + jsonString.length(), &root, &errs)) {
//     // Log the specific parsing error and the problematic string
//     logMessage(LogLevel::ERROR, "JSON Parsing failed for LLM Response",
//                "Errors: " + errs + "\nInput: " + jsonString);
//     return false;
//   }
//
//   if (!root.isObject()) {
//     logMessage(LogLevel::ERROR, "LLM Response root is not a JSON object",
//                jsonString);
//     return false;
//   }
//
//   // Extract fields safely
//   if (root.isMember("thought") && root["thought"].isString()) {
//     thought = root["thought"].asString();
//   } else {
//     logMessage(LogLevel::WARN,
//                "LLM Response missing or invalid 'thought' field.");
//   }
//
//   if (root.isMember("tool_calls")) {
//     if (root["tool_calls"].isNull()) { /* Explicit null is fine */
//     } else if (root["tool_calls"].isArray()) {
//       for (const auto &callJson : root["tool_calls"]) {
//         if (callJson.isObject() && callJson.isMember("tool_name") &&
//             callJson["tool_name"].isString() && callJson.isMember("params") &&
//             callJson["params"].isObject()) {
//           ToolCallInfo info;
//           info.toolName = callJson["tool_name"].asString();
//           if (info.toolName.empty()) {
//             logMessage(LogLevel::WARN,
//                        "Empty 'tool_name' found in tool_calls object.",
//                        callJson.toStyledString());
//             continue;
//           }
//           info.params = callJson["params"];
//           toolCalls.push_back(info);
//         } else {
//           logMessage(LogLevel::WARN,
//                      "Malformed tool_call object in LLM response",
//                      callJson.toStyledString());
//         }
//       }
//     } else {
//       logMessage(LogLevel::WARN,
//                  "'tool_calls' field exists but is not an array or null.",
//                  root["tool_calls"].toStyledString());
//     }
//   } else {
//     logMessage(LogLevel::WARN, "LLM Response missing 'tool_calls' field.");
//   }
//
//   if (root.isMember("final_response")) {
//     if (root["final_response"].isNull()) { /* Explicit null is fine */
//     } else if (root["final_response"].isString()) {
//       finalResponse = root["final_response"].asString();
//     } else {
//       logMessage(LogLevel::WARN,
//                  "'final_response' field exists but is not a string or null.",
//                  root["final_response"].toStyledString());
//     }
//   } else {
//     logMessage(LogLevel::WARN, "LLM Response missing 'final_response' field.");
//   }
//
//   // Enforce: Cannot have both final_response and tool_calls
//   if (!finalResponse.empty() && !toolCalls.empty()) {
//     logMessage(LogLevel::WARN, "LLM provided both final_response and "
//                                "tool_calls. Ignoring tool_calls.");
//     toolCalls.clear();
//   }
//
//   return true;
// }
```

### File: src/agent/prompt.cpp
```cpp
#include "../../inc/Agent.hpp"
#include "../../inc/MiniGemini.hpp"
#include "../../inc/Tool.hpp"
#include <algorithm> // For std::find_if, std::remove_if
#include <cctype>    // For std::toupper
#include <chrono>    // For timestamp
#include <cstdio>    // For FILE, fgets
#include <cstdlib>   // For popen, pclose, system
#include <ctime>
#include <fstream> // For file operations
#include <iomanip> // For formatting time
#include <iostream>
#include <json/json.h>
#include <sstream>
#include <stdexcept>

#include <string>
#include <sys/stat.h> // For file operations
#include <unistd.h>   // For getcwd


// helper to trim markdown code blocks if present
void trimCodeBlock(std::string &str)
{
    std::string startMarker = "```json";
    std::string endMarker = "```";
    size_t startPos = str.find(startMarker);
    if (startPos != std::string::npos)
    {
        // Found start marker, look for actual JSON start '{'
        size_t jsonStart = str.find('{', startPos + startMarker.length());
        if (jsonStart != std::string::npos)
        {
            // Found '{', now find end marker '```' after it
            size_t endPos = str.find(endMarker, jsonStart);
            if (endPos != std::string::npos)
            {
                // Found end marker, find last '}' before it
                size_t jsonEnd = str.rfind('}', endPos);
                if (jsonEnd != std::string::npos && jsonEnd >= jsonStart)
                {
                    // Extract the likely JSON object content
                    str = str.substr(jsonStart, jsonEnd - jsonStart + 1);
                }
            }
        }
    }
    else
    {
        // No code block found, just trim whitespace
        str.erase(str.find_last_not_of(" \t\r\n") + 1);
    }
    // Trim whitespace from the string we intend to parse
    str.erase(0, str.find_first_not_of(" \t\r\n"));
    str.erase(str.find_last_not_of(" \t\r\n") + 1);
    
    // Check if the string is empty after trimming
    if (str.empty())
    {
        throw std::runtime_error("Parsed JSON string is empty after trimming.");
    }

}
    



// Main prompt processing loop (using new JSON structure)
std::string Agent::prompt(const std::string &userInput) {
  if (!userInput.empty()) {
    addToHistory("user", userInput);
  }
  iteration = 0;
  skipFlowIteration = false;
  std::string finalAgentResponse = "";

  while (iteration < iterationCap && !skipFlowIteration) {
    iteration++;
    logMessage(LogLevel::INFO, "Agent '" + name + "' Iteration " +
                                   std::to_string(iteration) + "/" +
                                   std::to_string(iterationCap));

    std::string fullPrompt = buildFullPrompt();
    std::string llmResponseJson = executeApiCall(fullPrompt);
    try {
      trimCodeBlock(llmResponseJson); // Trim code block if present
    } catch (const std::exception &e) {
      logMessage(LogLevel::ERROR,
                 "Error trimming code block from LLM response for agent '" +
                     name + "': " + e.what());
      finalAgentResponse = llmResponseJson;
      setSkipFlowIteration(true);
      continue;
    }
    addToHistory("Agent: " + this->getName(), llmResponseJson);

    std::string status;
    std::vector<StructuredThought> thoughts;
    std::vector<ActionInfo> actions;
    std::string llmFinalResponseField;

    // Call the *newly declared* parser
    bool parsedSuccessfully = parseStructuredLLMResponse(
        llmResponseJson, status, thoughts, actions, llmFinalResponseField);

    if (!parsedSuccessfully) {
      logMessage(LogLevel::ERROR,
                 "Failed to parse structured LLM response for agent '" + name +
                     "'. Treating raw response as final.",
                 llmResponseJson);
      finalAgentResponse = llmResponseJson;
      setSkipFlowIteration(true);
      continue;
    }

    for (const auto &thought : thoughts) {
      logMessage(LogLevel::DEBUG,
                 "LLM Thought (" + thought.type + "):", thought.content);
    }

    if (status == "REQUIRES_CLARIFICATION") {
      logMessage(LogLevel::INFO,
                 "LLM requires clarification for agent '" + name + "'");
      finalAgentResponse = "Agent needs clarification."; // Default
      for (const auto &action : actions) {
        if (action.action == "request_user_input" &&
            action.params.isMember("query_to_user") &&
            action.params["query_to_user"].isString()) {
          finalAgentResponse = action.params["query_to_user"].asString();
          break;
        }
      }
      setSkipFlowIteration(true);
    } else if (status == "REQUIRES_ACTION" && !actions.empty()) {
      logMessage(LogLevel::INFO,
                 "LLM requested actions for agent '" + name + "'");
      std::string actionResults =
          processActions(actions);         // Use the new function
      addToHistory("user", actionResults); // Feed results back
    } else if (status == "SUCCESS_FINAL") {
      logMessage(LogLevel::INFO,
                 "LLM provided final response for agent '" + name + "'");
      finalAgentResponse =
          llmFinalResponseField; // Default to final_response field
      // Allow first 'send_response' action to override if present (and
      // final_response was empty/null)
      for (const auto &action : actions) {
        if (action.action == "send_response") {
          if (finalAgentResponse.empty()) { // Only override if final_response
                                            // wasn't explicitly set
            finalAgentResponse =
                processAction(action); // Process and use this as final
          } else {
            logMessage(LogLevel::WARN,
                       "LLM has status SUCCESS_FINAL and non-empty "
                       "final_response, ignoring send_response action.",
                       action.params.toStyledString());
          }
          break; // Only handle first send_response as final here
        }
      }
      setSkipFlowIteration(true);
    } else if (status == "ERROR_INTERNAL") {
      logMessage(LogLevel::ERROR,
                 "LLM indicated an internal error for agent '" + name + "'");
      finalAgentResponse = llmFinalResponseField.empty()
                               ? "LLM reported an internal error."
                               : llmFinalResponseField;
      setSkipFlowIteration(true);
    } else {
      logMessage(LogLevel::WARN,
                 "LLM response has unclear status/action state for agent '" +
                     name + "'. Treating as final.",
                 "Status: " + status);
      finalAgentResponse = llmFinalResponseField.empty()
                               ? llmResponseJson
                               : llmFinalResponseField; // Use raw if empty
      setSkipFlowIteration(true);
    }
  } // End while loop

  if (iteration >= iterationCap &&
      !skipFlowIteration) { // Check skipFlowIteration flag
    logMessage(LogLevel::WARN,
               "Iteration limit reached for agent '" + name + "'",
               std::to_string(iterationCap));
    if (finalAgentResponse.empty()) {
      // Try to get last sensible model response from history
      finalAgentResponse = "Agent reached iteration limit (" +
                           std::to_string(iterationCap) +
                           ") without a conclusive response.";
      for (auto it = history.rbegin(); it != history.rend(); ++it) {
        if (it->first == "model") {
          if (it->second.find("<action_results>") == std::string::npos &&
              it->second.find("\"actions\": [") == std::string::npos) {
            finalAgentResponse = it->second; // Use last model message not
                                             // containing results/actions
            break;
          }
        }
      }
    }
  }

  setSkipFlowIteration(false);
  logMessage(LogLevel::INFO,
             "Final response for agent '" + name + "':", finalAgentResponse);
  return finalAgentResponse;
}

// --- Prompt Building ---
std::string Agent::buildFullPrompt() const {
  std::stringstream promptStream;
  // 1. System Prompt
  if (!systemPrompt.empty()) {
    promptStream << "<system_prompt>\n";
    promptStream << systemPrompt << "\n";
    promptStream << "</system_prompt>\n\n";
  }

  // agent schema
  if (!schema.empty()) {
    promptStream << "<agent_reply_schema>\n";
    promptStream << "\t" << schema << "\n";
    promptStream << "</agent_reply_schema>\n\n";
  }
  if (!example.empty()) {
    promptStream << "<agent_reply_example>\n";
    promptStream << "\t" << example << "\n";
    promptStream << "</agent_reply_example>\n\n";
  }
  // 2. Agent Info
  promptStream << "<agent_context>\n";
  if (!name.empty()) {
    promptStream << "\t<name>" << name << "</name>\n";
  }
  if (!description.empty()) {
    promptStream << "\t<description>" << description << "</description>\n";
  }
  promptStream << "</agent_context>\n\n";
  // 3. Environment Variables
  if (!env.empty()) {
    promptStream << "<environment_variables>\n";
    for (const auto &pair : env) {
      promptStream << "\t<variable name=\"" << pair.first << "\">"
                   << pair.second << "</variable>\n";
    }
    promptStream << "</environment_variables>\n\n";
  }
  // 4. Available Tools/Actions
  std::map<std::string, std::string> availableActions =
      internalToolDescriptions;
  for (const auto &pair : tools) {
    if (pair.second) {
      availableActions[pair.first] = pair.second->getDescription();
    }
  }
  if (!availableActions.empty()) {
    promptStream << "<available_actions>\n";
    for (const auto &pair : availableActions) {
      promptStream << "\t<action name=\"" << pair.first << "\">\n";
      promptStream << "\t\t<description>" << pair.second << "</description>\n";
      promptStream << "\t</action>\n";
    }
    promptStream << "</available_actions>\n\n";
  }
  // 5. Directive
  if (directive.type != DIRECTIVE::NORMAL || !directive.description.empty()) {
    promptStream << "<current_directive>\n";
    promptStream << "\t<type>" << directiveTypeToString(directive.type)
                 << "</type>\n";
    if (!directive.description.empty())
      promptStream << "\t<description>" << directive.description
                   << "</description>\n";
    if (!directive.format.empty())
      promptStream << "\t<expected_format>" << directive.format
                   << "</expected_format>\n";
    promptStream << "</current_directive>\n\n";
  }
  // 6. Extra Prompts
  if (!extraPrompts.empty()) {
    promptStream << "<additional_instructions>\n";
    for (const auto &p : extraPrompts) {
      promptStream << "\t" << p << "\n";
    }
    promptStream << "</additional_instructions>\n\n";
  }
  // 7. Memory
  std::string memoryContent;
  if (!shortTermMemory.empty()) {
    memoryContent += "\t<short_term>\n";
    for (const auto &pair : shortTermMemory)
      memoryContent +=
          "\t\t<item key=\"" + pair.first + "\">" + pair.second + "</item>\n";
    memoryContent += "\t</short_term>\n";
  }
  if (!longTermMemory.empty()) {
    memoryContent += "\t<long_term>\n";
    for (const auto &pair : longTermMemory)
      memoryContent +=
          "\t\t<item role=\"" + pair.first + "\">" + pair.second + "</item>\n";
    memoryContent += "\t</long_term>\n";
  }
  if (!scratchpad.empty()) {
    memoryContent += "\t<scratchpad>\n";
    for (const auto &pair : scratchpad)
      memoryContent +=
          "\t\t<item key=\"" + pair.first + "\">" + pair.second + "</item>\n";
    memoryContent += "\t</scratchpad>\n";
  }
  if (!memoryContent.empty()) {
    promptStream << "<memory_context>\n"
                 << memoryContent << "</memory_context>\n\n";
  }
  // 8. Conversation History
  if (!history.empty()) {
    promptStream << "<conversation_history>\n";
    for (const auto &entry : history) {
      promptStream << "\t<turn role=\"" << entry.first << "\">\n";
      std::stringstream ss_content(entry.second);
      std::string line;
      while (std::getline(ss_content, line)) {
        promptStream << "\t\t" << line << "\n";
      }
      promptStream << "\t</turn>\n";
    }
    promptStream << "</conversation_history>\n\n";
  }
  promptStream << "CONTEXT END\n\n";
  // 9. Final Instruction
  promptStream
      << "RESPONSE INSTRUCTIONS: Generate your response strictly as a single "
         "JSON object containing 'status', 'thoughts' (structured array), "
         "'actions' (array, use '[]' if none), and potentially "
         "'final_response' (usually null/empty if actions are present). Base "
         "your response on the entire context provided above.";
  std::cout << "DEBUG: ++++++++++++++++++++++++++++ \n" + promptStream.str()
            << std::endl;
  return promptStream.str();
}
```

### File: src/agent/runtime.cpp
```cpp

#include "../../inc/Agent.hpp" // Your Agent class header


```

### File: src/agent/tool.cpp
```cpp
// #include "../../inc/Agent.hpp"
// #include "../../inc/MiniGemini.hpp"
// #include "../../inc/Tool.hpp"
// #include <algorithm> // For std::find_if, std::remove_if
// #include <cctype>    // For std::toupper
// #include <chrono>    // For timestamp
// #include <cstdio>    // For FILE, fgets
// #include <cstdlib>   // For popen, pclose, system
// #include <ctime>
// #include <fstream> // For file operations
// #include <iomanip> // For formatting time
// #include <iostream>
// #include <json/json.h>
// #include <sstream>
// #include <stdexcept>
//
// std::string Agent::handleToolExecution(const ToolCallInfo &call) {
//   logMessage(LogLevel::TOOL_CALL, "Executing tool '" + call.toolName + "'",
//              "Params: " + call.params.toStyledString());
//   std::string result;
//   try {
//     // Internal Tools
//     if (call.toolName == "help")
//       result = this->getHelp(call.params);
//     else if (call.toolName == "skip")
//       result = skip(call.params);
//     else if (call.toolName == "promptAgent")
//       result = promptAgentTool(call.params);
//     else if (call.toolName == "summarizeTool")
//       result = summerizeTool(call.params);
//     else if (call.toolName == "summarizeHistory")
//       result = summarizeHistory(call.params);
//     else if (call.toolName == "getWeather")
//       result = getWeather(call.params);
//     // External Tools
//     else {
//       Tool *tool = getTool(call.toolName);
//       if (tool) {
//         result = tool->execute(call.params);
//       } else {
//         logMessage(LogLevel::WARN, "Attempted to execute unknown tool: '" +
//                                        call.toolName + "'");
//         result = "Error: Unknown tool '" + call.toolName + "'.";
//       }
//     }
//     logMessage(LogLevel::TOOL_RESULT, "Tool '" + call.toolName + "' executed.",
//                "Result: " + result);
//   } catch (const std::exception &e) {
//     result = "Error executing tool '" + call.toolName + "': " + e.what();
//     logMessage(LogLevel::ERROR,
//                "Exception during tool execution for '" + call.toolName + "'.",
//                "Error: " + std::string(e.what()));
//   } catch (...) {
//     result = "Error: Unknown exception executing tool '" + call.toolName + "'";
//     logMessage(LogLevel::ERROR,
//                "Unknown exception during tool execution for '" + call.toolName +
//                    "'.");
//   }
//   return result;
// }
//
// std::string
// Agent::processToolCalls(const std::vector<ToolCallInfo> &toolCalls) {
//   if (toolCalls.empty())
//     return "";
//   std::ostringstream toolResultsOss;
//   toolResultsOss << "Tool Execution Results:\n";
//   for (const auto &call : toolCalls) {
//     std::string result = handleToolExecution(call);
//     toolResultsOss << "--- Tool: " << call.toolName << " ---\n"
//                    << "Params: " << call.params.toStyledString()
//                    << "Result: " << result << "\n";
//   }
//   toolResultsOss << "--- End Tool Results ---";
//   return toolResultsOss.str();
// }
//
```

### File: src/groqClient.cpp
```cpp
#include "../inc/Groq.hpp"
#include <curl/curl.h>
#include <json/json.h>
#include <sstream>
#include <cstdlib> // For getenv
#include <stdexcept>
#include <iostream> // For potential debug logging
#include <memory>   // For unique_ptr

// Libcurl write callback
size_t GroqClient::writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Constructor implementation
GroqClient::GroqClient(const std::string& apiKey) :
    // Sensible defaults for Groq
    m_model("llama3-8b-8192"), // Example Groq model
    m_temperature(0.5),
    m_maxTokens(4096),
    m_baseUrl("https://api.groq.com/openai/v1") // OpenAI-compatible endpoint base
{
    if (!apiKey.empty()) {
        m_apiKey = apiKey;
    } else {
        const char* envKey = std::getenv("GROQ_API_KEY");
        if (envKey != nullptr && envKey[0] != '\0') {
            m_apiKey = envKey;
             std::cout << "[INFO] GroqClient: Using API key from GROQ_API_KEY environment variable." << std::endl;
        } else {
            std::cerr << "[WARN] GroqClient: API key not provided via constructor or GROQ_API_KEY env var. API calls will likely fail." << std::endl;
            // throw std::invalid_argument("Groq API key required: Provide via constructor or GROQ_API_KEY env var");
        }
    }
    // curl_global_init in main()
}

// Configuration Setters
void GroqClient::setApiKey(const std::string& apiKey) { m_apiKey = apiKey; }
void GroqClient::setModel(const std::string& model) { m_model = model; }
void GroqClient::setTemperature(double temperature) { m_temperature = temperature; }
void GroqClient::setMaxTokens(int maxTokens) { m_maxTokens = maxTokens; }
void GroqClient::setBaseUrl(const std::string& baseUrl) { m_baseUrl = baseUrl; }

// Generate implementation (Overrides base class)
std::string GroqClient::generate(const std::string& prompt) {
     if (m_apiKey.empty()) {
        throw ApiError("Groq API key is not set.");
    }
    // Groq uses the chat completions endpoint
    std::string url = m_baseUrl + "/chat/completions";

    // Build JSON payload specific to Groq/OpenAI Chat Completions API
    Json::Value root;
    Json::Value message;
    Json::Value messagesArray(Json::arrayValue); // Array for messages

    // Simple structure: only one user message
    message["role"] = "user";
    message["content"] = prompt;
    messagesArray.append(message);

    root["messages"] = messagesArray;
    root["model"] = m_model;
    root["temperature"] = m_temperature;
    root["max_tokens"] = m_maxTokens;
    // Add other Groq/OpenAI-specific parameters if needed (e.g., top_p, stream)

    Json::StreamWriterBuilder writerBuilder;
    writerBuilder["indentation"] = ""; // Compact output
    std::string payload = Json::writeString(writerBuilder, root);

    // std::cout << "[DEBUG] GroqClient Request URL: " << url << std::endl;
    // std::cout << "[DEBUG] GroqClient Request Payload: " << payload << std::endl;

    try {
        std::string responseBody = performHttpRequest(url, payload);
        // std::cout << "[DEBUG] GroqClient Response Body: " << responseBody << std::endl;
        return parseJsonResponse(responseBody);
    } catch (const ApiError& e) {
        std::cerr << "[ERROR] GroqClient API Error: " << e.what() << std::endl;
        throw; // Re-throw API specific errors
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] GroqClient Request Failed: " << e.what() << std::endl;
        throw ApiError(std::string("Groq request failed: ") + e.what()); // Wrap other errors
    } catch (...) {
         std::cerr << "[ERROR] GroqClient Unknown request failure." << std::endl;
         throw ApiError("Unknown error during Groq request.");
    }
}


// HTTP request helper (Note the Authorization header)
std::string GroqClient::performHttpRequest(const std::string& url, const std::string& payload) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to initialize libcurl");

    std::string readBuffer;
    long http_code = 0;
    struct curl_slist* headers = nullptr;

    // Groq/OpenAI typically uses Bearer token authorization
    std::string authHeader = "Authorization: Bearer " + m_apiKey;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::unique_ptr<struct curl_slist, decltype(&curl_slist_free_all)> header_list(headers, curl_slist_free_all);


    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload.length());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list.get());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

    CURLcode res = curl_easy_perform(curl);
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl_handle(curl, curl_easy_cleanup);

    if (res != CURLE_OK) {
        throw ApiError("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code < 200 || http_code >= 300) {
        std::ostringstream errMsg;
        errMsg << "HTTP Error: " << http_code;
        errMsg << " | Response: " << readBuffer.substr(0, 500);
        if (readBuffer.length() > 500) errMsg << "...";
        throw ApiError(errMsg.str());
    }

    return readBuffer;
}

// JSON parsing helper (Adapted for Groq/OpenAI Chat Completions response)
std::string GroqClient::parseJsonResponse(const std::string& jsonResponse) const {
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
    std::string errors;

    bool parsingSuccessful = reader->parse(jsonResponse.c_str(), jsonResponse.c_str() + jsonResponse.length(), &root, &errors);

    if (!parsingSuccessful) {
        throw ApiError("Failed to parse Groq JSON response: " + errors);
    }

    // Check for OpenAI-style error structure first
    if (root.isMember("error") && root["error"].isObject()) {
         std::string errorMsg = "API Error: ";
         if (root["error"].isMember("message") && root["error"]["message"].isString()) {
            errorMsg += root["error"]["message"].asString();
         } else if (root["error"].isMember("type") && root["error"]["type"].isString()){
             errorMsg += root["error"]["type"].asString(); // Include error type if message is missing
         } else {
             errorMsg += Json::writeString(Json::StreamWriterBuilder(), root["error"]);
         }
         throw ApiError(errorMsg);
    }
     // Also handle top-level 'detail' which Groq sometimes uses for errors
     if (root.isMember("detail") && root["detail"].isString()){
        throw ApiError("API Error Detail: " + root["detail"].asString());
     }


    // Navigate the expected Groq/OpenAI success structure
    try {
        // Structure: root -> choices[0] -> message -> content
        if (root.isMember("choices") && root["choices"].isArray() && !root["choices"].empty()) {
            const Json::Value& firstChoice = root["choices"][0u];
            if (firstChoice.isMember("message") && firstChoice["message"].isObject()) {
                const Json::Value& message = firstChoice["message"];
                if (message.isMember("content") && message["content"].isString()) {
                    return message["content"].asString();
                }
            }
             // Check finish reason if content is missing/null
             if (firstChoice.isMember("finish_reason") && firstChoice["finish_reason"].asString() != "stop") {
                 throw ApiError("Content generation finished unexpectedly. Reason: " + firstChoice["finish_reason"].asString());
             }
        }
        // If structure wasn't as expected
         throw ApiError("Could not extract content from Groq API response structure. Response: " + jsonResponse.substr(0, 500) + "...");

    } catch (const Json::Exception& e) {
        throw ApiError(std::string("JSON access error while parsing Groq response: ") + e.what());
     } catch (const ApiError& e) {
        throw;
    } catch (const std::exception& e) {
         throw ApiError(std::string("Standard exception while parsing Groq response: ") + e.what());
    }
}
```

### File: src/MiniGemini.cpp
```cpp
#include "../inc/MiniGemini.hpp"
#include <curl/curl.h>
#include <json/json.h>
#include <sstream>
#include <cstdlib> // For getenv
#include <stdexcept>
#include <iostream> // For potential debug logging

// Libcurl write callback (Implementation remains the same)
size_t MiniGemini::writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Constructor implementation
MiniGemini::MiniGemini(const std::string& apiKey) :
    // Sensible defaults for Gemini
    m_model("gemini-2.0-flash"), // Use a common, stable model
    // m_model("gemini-2.5-pro-exp-03-25"),
    m_temperature(0.5), // Adjusted default temperature
    m_maxTokens(4096),
    m_baseUrl("https://generativelanguage.googleapis.com/v1beta/models")
{
    if (!apiKey.empty()) {
        m_apiKey = apiKey;
    } else {
        const char* envKey = std::getenv("GEMINI_API_KEY");
        if (envKey != nullptr && envKey[0] != '\0') {
            m_apiKey = envKey;
            std::cout << "[INFO] GeminiClient: Using API key from GEMINI_API_KEY environment variable." << std::endl;
        } else {
            std::cerr << "[WARN] GeminiClient: API key not provided via constructor or GEMINI_API_KEY env var. API calls will likely fail." << std::endl;
            // Consider throwing here if the key is absolutely mandatory for initialization
            // throw std::invalid_argument("Gemini API key required: Provide via constructor or GEMINI_API_KEY env var");
        }
    }
     // Note: curl_global_init should ideally be called once in main()
}

// Configuration Setters
void MiniGemini::setApiKey(const std::string& apiKey) { m_apiKey = apiKey; }
void MiniGemini::setModel(const std::string& model) { m_model = model; }
void MiniGemini::setTemperature(double temperature) { m_temperature = temperature; }
void MiniGemini::setMaxTokens(int maxTokens) { m_maxTokens = maxTokens; }
void MiniGemini::setBaseUrl(const std::string& baseUrl) { m_baseUrl = baseUrl; }


// Generate implementation (Overrides base class)
std::string MiniGemini::generate(const std::string& prompt) {
     if (m_apiKey.empty()) {
        throw ApiError("Gemini API key is not set.");
    }
    std::string url = m_baseUrl + "/" + m_model + ":generateContent?key=" + m_apiKey;

    // Build JSON payload specific to Gemini API
    Json::Value root;
    Json::Value content;
    Json::Value part;
    Json::Value genConfig;

    part["text"] = prompt;
    content["parts"].append(part);
    // Gemini API structure often uses a 'contents' array
    root["contents"].append(content);

    // Add generation config
    genConfig["temperature"] = m_temperature;
    genConfig["maxOutputTokens"] = m_maxTokens;
    // Add other Gemini-specific configs if needed (e.g., stop sequences, top_k, top_p)
    root["generationConfig"] = genConfig;

    Json::StreamWriterBuilder writerBuilder; // Use StreamWriterBuilder for more control
    writerBuilder["indentation"] = ""; // Use FastWriter equivalent for compact output
    std::string payload = Json::writeString(writerBuilder, root);

    // std::cout << "[DEBUG] GeminiClient Request URL: " << url << std::endl;
    // std::cout << "[DEBUG] GeminiClient Request Payload: " << payload << std::endl;

    try {
        std::string responseBody = performHttpRequest(url, payload);
        // std::cout << "[DEBUG] GeminiClient Response Body: " << responseBody << std::endl;
        return parseJsonResponse(responseBody);
    } catch (const ApiError& e) {
        std::cerr << "[ERROR] GeminiClient API Error: " << e.what() << std::endl;
        throw; // Re-throw API specific errors
    } catch (const std::exception& e) {
         std::cerr << "[ERROR] GeminiClient Request Failed: " << e.what() << std::endl;
        throw ApiError(std::string("Gemini request failed: ") + e.what()); // Wrap other errors
    } catch (...) {
        std::cerr << "[ERROR] GeminiClient Unknown request failure." << std::endl;
        throw ApiError("Unknown error during Gemini request.");
    }
}


// HTTP request helper (Remains mostly the same, ensure correct headers)
std::string MiniGemini::performHttpRequest(const std::string& url, const std::string& payload) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to initialize libcurl");

    std::string readBuffer;
    long http_code = 0;
    struct curl_slist* headers = nullptr;
    // Ensure correct Content-Type for Gemini
    headers = curl_slist_append(headers, "Content-Type: application/json");
    // Wrap slist in unique_ptr for RAII
    std::unique_ptr<struct curl_slist, decltype(&curl_slist_free_all)> header_list(headers, curl_slist_free_all);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload.length());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list.get());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L); // Increased timeout slightly
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L); // Disable progress meter
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""); // Allow curl to handle encoding


    CURLcode res = curl_easy_perform(curl);
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl_handle(curl, curl_easy_cleanup); // RAII cleanup


    if (res != CURLE_OK) {
        throw ApiError("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code < 200 || http_code >= 300) {
        std::ostringstream errMsg;
        errMsg << "HTTP Error: " << http_code;
        errMsg << " | Response: " << readBuffer.substr(0, 500); // Include more response context
         if (readBuffer.length() > 500) errMsg << "...";
        throw ApiError(errMsg.str());
    }

    return readBuffer;
}

// JSON parsing helper (Adapted for Gemini's typical response structure)
std::string MiniGemini::parseJsonResponse(const std::string& jsonResponse) const {
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
    std::string errors;

    bool parsingSuccessful = reader->parse(jsonResponse.c_str(), jsonResponse.c_str() + jsonResponse.length(), &root, &errors);

    if (!parsingSuccessful) {
        throw ApiError("Failed to parse Gemini JSON response: " + errors);
    }

    // Check for Gemini-specific error structure first
    if (root.isMember("error") && root["error"].isObject()) {
        std::string errorMsg = "API Error: ";
        if (root["error"].isMember("message") && root["error"]["message"].isString()) {
            errorMsg += root["error"]["message"].asString();
        } else {
             errorMsg += Json::writeString(Json::StreamWriterBuilder(), root["error"]); // Dump error object if message missing
        }
         return errorMsg; // Return error message instead of throwing? Or throw ApiError(errorMsg)? Let's throw.
         throw ApiError(errorMsg);
    }

    // Navigate the expected Gemini success structure
    try {
        // Gemini structure: root -> candidates[0] -> content -> parts[0] -> text
        if (root.isMember("candidates") && root["candidates"].isArray() && !root["candidates"].empty()) {
            const Json::Value& firstCandidate = root["candidates"][0u];
            if (firstCandidate.isMember("content") && firstCandidate["content"].isObject()) {
                 const Json::Value& content = firstCandidate["content"];
                 if (content.isMember("parts") && content["parts"].isArray() && !content["parts"].empty()) {
                     const Json::Value& firstPart = content["parts"][0u];
                     if (firstPart.isMember("text") && firstPart["text"].isString()) {
                         return firstPart["text"].asString();
                     }
                 }
            }
             // Handle cases where content might be blocked (safety ratings)
             if (firstCandidate.isMember("finishReason") && firstCandidate["finishReason"].asString() != "STOP") {
                 std::string reason = firstCandidate["finishReason"].asString();
                 std::string safetyInfo = "";
                 if (firstCandidate.isMember("safetyRatings")) {
                    safetyInfo = Json::writeString(Json::StreamWriterBuilder(), firstCandidate["safetyRatings"]);
                 }
                 throw ApiError("Content generation stopped due to safety settings or other reason: " + reason + ". Safety Ratings: " + safetyInfo);
             }
        }
        // If structure wasn't as expected
        throw ApiError("Could not extract text from Gemini API response structure. Response: " + jsonResponse.substr(0, 500) + "...");

    } catch (const Json::Exception& e) { // Catch JSON access errors
        throw ApiError(std::string("JSON access error while parsing Gemini response: ") + e.what());
    } catch (const ApiError& e) { // Re-throw our specific errors
        throw;
    } catch (const std::exception& e) { // Catch other potential errors
         throw ApiError(std::string("Standard exception while parsing Gemini response: ") + e.what());
    }
}
```

### File: src/utils/global.cpp
```cpp

#include "../../inc/Agent.hpp"
#include "../../inc/MiniGemini.hpp"
#include "../../inc/Tool.hpp"
#include <algorithm> // For std::find_if, std::remove_if
#include <cctype>    // For std::toupper
#include <chrono>    // For timestamp
#include <cstdio>    // For FILE, fgets
#include <cstdlib>   // For popen, pclose, system
#include <ctime>
#include <fstream> // For file operations
#include <iomanip> // For formatting time
#include <iostream>
#include <json/json.h>
#include <sstream>
#include <stdexcept>

// Collection of utility functions and global helpers

// Helper to execute external commands
int executeCommand(const std::string &command, std::string &output) {
  output.clear();
  std::string cmd_with_stderr = command + " 2>&1";
  FILE *pipe = popen(cmd_with_stderr.c_str(), "r");
  if (!pipe) {
    logMessage(LogLevel::ERROR, "popen() failed for command:", command);
    return -1;
  }
  char buffer[256];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    output += buffer;
  }
  int status = pclose(pipe);
  return WEXITSTATUS(status);
}

```
