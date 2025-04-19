# AI Project Analysis - externals
- Generated on: Mon Apr 14 05:06:36 PM +01 2025
- System: Linux 6.12.21-1-lts x86_64
- Arch Linux: 1639 packages installed
- Directory: /home/mlamkadm/ai-repos/agents/cpp-agent-mk1

## Directory Structure
```
../cpp-agent-mk1/externals
├── bash.cpp
├── cal-events.cpp
├── ddg-search.cpp
├── file.cpp
├── general.cpp
├── search.cpp
└── write.cpp
```

## Project Statistics
- Total Files: 57
- Total Lines of Code: 5187
- Languages: .cpp(7)

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
#pragma once // Prevent multiple inclusions

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

### File: externals/ddg-search.cpp
```cpp
#pragma once // Prevent multiple inclusions

#include <iostream>
#include <memory> // For unique_ptr
#include <regex>  // For basic HTML entity decoding
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "json/json.h" // For parsing the JSON input parameters
#include <curl/curl.h> // For making HTTP requests

// --- Helper: libcurl Write Callback (Can likely be shared if put in a common
// header) ---
static size_t ddgWriteCallback(void *contents, size_t size, size_t nmemb,
                               void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

// --- Helper: URL Encode (Can likely be shared) ---
std::string ddgUrlEncode(const std::string &value) {
  CURL *curl = curl_easy_init();
  if (!curl)
    return "";
  char *output = curl_easy_escape(curl, value.c_str(), static_cast<int>(value.length()));
  if (!output) {
    curl_easy_cleanup(curl);
    return "";
  }
  std::string result(output);
  curl_free(output);
  curl_easy_cleanup(curl);
  return result;
}

// --- Helper: Basic HTML Entity Decode ---
// Very basic, only handles a few common entities. A proper library is better.
std::string basicHtmlDecode(std::string text) {
  // Order matters for things like &lt;
  text = std::regex_replace(text, std::regex("&"), "&");
  text = std::regex_replace(text, std::regex("<"), "<");
  text = std::regex_replace(text, std::regex(">"), ">");
    text = std::regex_replace(text, std::regex("\""), "\"");
    text = std::regex_replace(text, std::regex("'"), "'");
    // Add more entities if needed
    return text;
}

// --- Helper: Extract text between tags (VERY basic) ---
std::string extractText(const std::string &html, size_t startPos) {
  size_t tagEnd = html.find('>', startPos);
  if (tagEnd == std::string::npos)
    return "";
  size_t textStart = tagEnd + 1;
  size_t textEnd = html.find('<', textStart);
  if (textEnd == std::string::npos)
    return ""; // Malformed?
  return basicHtmlDecode(html.substr(textStart, textEnd - textStart));
}

// --- Tool Implementation ---

// Tool Function: Performs a web search using DuckDuckGo HTML endpoint scraping.
// Input: JSON object like: {"query": "search terms" [, "num_results": N
// (optional, best effort)]} Output: Formatted string with search results
// (title, url, snippet) or an error message. WARNING: Relies on HTML scraping,
// which is fragile and may break without notice.
std::string duckduckgoSearchTool(const Json::Value &params) {

  // 1. Validate Parameters
  if (!params.isObject()) {
    return "Error: duckduckgoSearchTool requires a JSON object as input.";
  }
  if (!params.isMember("query") || !params["query"].isString() ||
      params["query"].asString().empty()) {
    return "Error: Missing or invalid required non-empty string parameter "
           "'query'.";
  }

  std::string query = params["query"].asString();
  int max_results = 5; // Default max results to aim for
  if (params.isMember("num_results") && params["num_results"].isInt()) {
    max_results = params["num_results"].asInt();
    if (max_results <= 0 ||
        max_results > 10) { // Keep requested count reasonable
      max_results = 5;
    }
  }
  // NOTE: We may get more/less results from the HTML page regardless of this
  // param.

  std::cout << "[TOOL_DEBUG] duckduckgoSearchTool: Query='" << query
            << "', Aiming for max " << max_results << " results." << std::endl;

  // 2. Prepare Request
  std::string encodedQuery = ddgUrlEncode(query);
  if (encodedQuery.empty() && !query.empty()) {
    return "Error: Failed to URL-encode the search query.";
  }

  // Use the non-JavaScript HTML endpoint
  std::string url = "https://html.duckduckgo.com/html/?q=" + encodedQuery;

  CURL *curl = curl_easy_init();
  if (!curl) {
    return "Error: Failed to initialize libcurl for DuckDuckGo search.";
  }
  std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl_handle(
      curl, curl_easy_cleanup);

  std::string readBuffer;
  long http_code = 0;
  // Use a common browser User-Agent
  const char *userAgent =
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, "
      "like Gecko) Chrome/91.0.4472.124 Safari/537.36";

  try {
    // Set CURL Options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ddgWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);       // 15 second timeout
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L); // Disable progress meter
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING,
                     ""); // Allow curl to handle encoding (like gzip)

    // 3. Perform API Call (Fetch HTML)
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      return "Error: DuckDuckGo search network request failed: " +
             std::string(curl_easy_strerror(res));
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code != 200) {
      std::stringstream errMsg;
      errMsg << "Error: DuckDuckGo search returned HTTP status " << http_code
             << ".";
      if (!readBuffer.empty()) {
        errMsg << " Response snippet: " << readBuffer.substr(0, 200) << "...";
      }
      return errMsg.str();
    }

  } catch (const std::exception &e) {
    return "Error: Exception during DuckDuckGo network operation: " +
           std::string(e.what());
  }
  // curl handle cleaned up automatically by unique_ptr

  // 4. Parse HTML (Basic and Fragile String Searching)
  std::stringstream output;
  output << "DuckDuckGo Search Results for '" << query << "':\n";
  int results_found = 0;
  size_t current_pos = 0;

  // Markers often used by DDG HTML results (these WILL change over time)
  const std::string result_block_start =
      "result--web"; // Often a class on a containing div
  const std::string link_marker = "result__a"; // Class for the main result link
  const std::string snippet_marker =
      "result__snippet"; // Class for the description snippet
  const std::string href_marker = "href=\"";

  while (results_found < max_results) {
    // Find the start of the next result block
    size_t block_start = readBuffer.find(result_block_start, current_pos);
    if (block_start == std::string::npos) {
      break; // No more result blocks found
    }

    // Find the link within this block
    size_t link_start = readBuffer.find(link_marker, block_start);
    if (link_start == std::string::npos) {
      current_pos =
          block_start + result_block_start.length(); // Move past this block
      continue;
    }

    // Extract URL
    size_t href_start = readBuffer.find(href_marker, link_start);
    std::string url = "N/A";
    if (href_start != std::string::npos) {
      href_start += href_marker.length();
      size_t href_end = readBuffer.find('"', href_start);
      if (href_end != std::string::npos) {
        // DDG URLs might be relative redirects, need cleaning
        url = readBuffer.substr(href_start, href_end - href_start);
        size_t uddg_param = url.find("uddg=");
        if (uddg_param != std::string::npos) {
          std::string encoded_target =
              url.substr(uddg_param + 5); // Length of "uddg="
          // Basic URL decode might be needed here, but often works without it
          url = ddgUrlEncode(
              encoded_target); // Re-use encode fn name, but it does decode %XX
          // Basic decode of %XX hex codes
          std::string decoded_url;
          CURL *temp_curl = curl_easy_init();
          int outlen;
          char *decoded = curl_easy_unescape(temp_curl, encoded_target.c_str(),
                                             encoded_target.length(), &outlen);
          if (decoded) {
            decoded_url.assign(decoded, outlen);
            curl_free(decoded);
            url = decoded_url;
          }
          curl_easy_cleanup(temp_curl);
        }
        url = basicHtmlDecode(url); // Decode entities like &
      }
    }

    // Extract Title (usually the link text)
    std::string title = extractText(readBuffer, link_start);
    if (title.empty())
      title = "N/A";

    // Find and Extract Snippet
    size_t snippet_start =
        readBuffer.find(snippet_marker, link_start); // Search after link
    std::string snippet = "N/A";
    if (snippet_start != std::string::npos) {
      // Find the actual text content after the snippet marker tag closes
      size_t tag_end = readBuffer.find('>', snippet_start);
      if (tag_end != std::string::npos) {
        size_t text_start = tag_end + 1;
        size_t text_end =
            readBuffer.find('<', text_start); // Find start of next tag
        if (text_end != std::string::npos) {
          snippet = readBuffer.substr(text_start, text_end - text_start);
          // Trim leading/trailing whitespace often present
          snippet.erase(0, snippet.find_first_not_of(" \t\n\r"));
          snippet.erase(snippet.find_last_not_of(" \t\n\r") + 1);
          snippet = basicHtmlDecode(snippet);
        }
      }
    }

    // Append formatted result
    output << "\n---\n";
    output << "Title: " << title << "\n";
    output << "URL: " << url << "\n";
    output << "Snippet: " << snippet << "\n";
    results_found++;

    // Move position past the current snippet to find the next block
    current_pos = (snippet_start != std::string::npos)
                      ? (snippet_start + snippet_marker.length())
                      : (link_start + link_marker.length());

  } // End while loop

  if (results_found == 0) {
    output << "\n(No results found or failed to parse HTML)\n";
  }

  std::cout << "[TOOL_DEBUG] duckduckgoSearchTool: Found and formatted "
            << results_found << " results." << std::endl;
  return output.str();
}
```

### File: externals/file.cpp
```cpp
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

### File: externals/search.cpp
```cpp
#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include "json/json.h"

// --- Helper: libcurl Write Callback ---
static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

// --- Helper: URL Encode ---
std::string urlEncode(const std::string &value) {
    CURL *curl = curl_easy_init();
    if (!curl) return "";
    char *output = curl_easy_escape(curl, value.c_str(), static_cast<int>(value.length()));
    if (!output) {
        curl_easy_cleanup(curl);
        return "";
    }
    std::string result(output);
    curl_free(output);
    curl_easy_cleanup(curl);
    return result;
}

// --- Tool Implementation ---
std::string webSearchTool(const Json::Value &params) {
    // 1. Validate Parameters
    if (!params.isObject()) {
        return "Error: webSearchTool requires a JSON object as input.";
    }
    if (!params.isMember("query") || !params["query"].isString() || params["query"].asString().empty()) {
        return "Error: Missing or invalid required non-empty string parameter 'query'.";
    }

    std::string query = params["query"].asString();
    int num_results = 3; // Default
    if (params.isMember("num_results") && params["num_results"].isInt()) {
        num_results = params["num_results"].asInt();
        if (num_results <= 0) {
            num_results = 3;
        }
    }
    std::string search_engine = "google";
    if (params.isMember("search_engine") && params["search_engine"].isString()) {
        search_engine = params["search_engine"].asString();
    }
    std::string url;
    if (search_engine == "google") {
        url = "https://www.google.com/search?q=" + urlEncode(query);
    } else if (search_engine == "duckduckgo") {
        url = "https://html.duckduckgo.com/html/?q=" + urlEncode(query);
    } else {
        return "Error: Invalid search engine: " + search_engine + ". Supported engines: google, duckduckgo";
    }
    CURL *curl = curl_easy_init();
    if (!curl) {
        return "Error: Failed to initialize libcurl.";
    }
    std::string readBuffer;

    try {
        // Set CURL options
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            curl_easy_cleanup(curl);
            return "Error: Web search failed: " + std::string(curl_easy_strerror(res));
        }

        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 200) {
            curl_easy_cleanup(curl);
            return "Error: Web search returned HTTP " + std::to_string(http_code);
        }

        curl_easy_cleanup(curl);

        // Basic result parsing (This will need improvement!)
        std::stringstream output;
        output << "Web Search Results for '" << query << "':\n";
        output << "Retrieved from: " << url << "\n";
        output << "Raw response (first 200 chars):\n" << readBuffer.substr(0, 200) << "...\n"; // For debugging
        return output.str();

    } catch (const std::exception &e) {
        curl_easy_cleanup(curl);
        return "Error: Exception during web search: " + std::string(e.what());
    }
}```

### File: externals/write.cpp
```cpp
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
```
# AI Project Analysis - inc
- Generated on: Mon Apr 14 05:06:36 PM +01 2025
- System: Linux 6.12.21-1-lts x86_64
- Arch Linux: 1639 packages installed
- Directory: /home/mlamkadm/ai-repos/agents/cpp-agent-mk1

## Directory Structure
```
../cpp-agent-mk1/inc
├── Agent.hpp
├── File.hpp
├── Groq.hpp
├── Json.hpp
├── MiniGemini.hpp
├── modelApi.hpp
├── notes.hpp
└── Tool.hpp
```

## Project Statistics
- Total Files: 58
- Total Lines of Code: 6413
- Languages: .hpp(8)

## Project Files

### File: inc/Agent.hpp
```cpp
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Agent.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlamkadm <mlamkadm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/05 15:09:32 by mlamkadm          #+#    #+#             */
/*   Updated: 2025/04/12 18:07:46 by mlamkadm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "File.hpp"
#include "MiniGemini.hpp" // Include MiniGemini header
#include "Tool.hpp"       // Include Tool header
#include <json/json.h>    // For Json::Value
#include <json/value.h>
#include <map>
#include <string>
#include <vector>
// main.cpp includes
#include <cstdlib>
#include <ctime>
#include <curl/curl.h>
#include <iostream>
#include <json/json.h>
#include <sstream> // Required for rollDice
#include <stdexcept>
#include <string>
// remove_if
#include "Tool.hpp"
#include <algorithm> // For std::remove_if
#include <fstream>   // For file operations

#include <algorithm> // For std::remove, std::find
#include <cstddef>   // For size_t
#include <fstream>   // For std::ifstream, std::ofstream
#include <iostream>  // For std::ostream
#include <sstream>   // For std::ostringstream
#include <stdexcept> // For std::runtime_error
#include <string>
#include <vector>

typedef std::vector<File> fileList;
typedef std::vector<std::pair<std::string, std::string>> StrkeyValuePair;
class Tool;        // Forward declaration of Tool class
class Agent;       // Forward declaration of Agent class
class BuiltInTool; // Forward declaration of BuiltInTool class
struct ToolParams {
  const Json::Value &params; // Parameters for the tool
  Agent &agent;              // Reference to the agent
};

// Basic Logger (can be expanded into a class later if needed)
enum class LogLevel {
  DEBUG,
  INFO,
  WARN,
  ERROR,
  TOOL_CALL,
  TOOL_RESULT,
  PROMPT
};

class Agent {
public:
  // Structure to hold parsed tool call information

  struct ToolCallInfo {
    std::string toolName;
    Json::Value params;
    std::string optionalDataStr; // For text-based tools
    bool isTextBasedTool =
        false; // Flag to distinguish between JSON and text tools
  };
  Agent(MiniGemini &api); // Takes API client by reference

  void setSystemPrompt(const std::string &prompt);
  void addTool(Tool *tool); // Manages Tool pointers (caller owns the Tool)
  void addTextTool(Tool *tool);
  void reset();
  std::string prompt(const std::string &userInput);

  void run(); // Starts the interactive loop

  void addAgent(Agent *agent) {
    agents.push_back(std::make_pair(agent->getName(), agent));
  } // Adds an agent to the list
  std::string agentInstance(const std::string &name) {
    auto it = std::find_if(agents.begin(), agents.end(),
                           [&name](const std::pair<std::string, Agent *> &p) {
                             return p.first == name;
                           });
    if (it != agents.end()) {
      return it->second->getName();
    }
    return "Agent name not found.";
  } // Returns the agent instance by name
    // Potentially add methods to manage iteration cap, skip flag etc. if needed
    // externally
    // =========================================================
    // ================================[ Basic Memory Management

  std::string manualToolCall(const std::string &toolName,
                             const Json::Value &params) {
    ToolCallInfo call;
    call.toolName = toolName;
    call.params = params;
    return handleToolExecution(call);
  } // Manually calls a tool with the given parameters
  const std::string getName() const {
    return this->_name;
  } // Returns the name of the agent
  void addMemory(const std::string &role, const std::string &content) {
    this->LongTermMemory.push_back(std::make_pair(role, content));
  }
  void removeMemory(const std::string &role, const std::string &content) {
    auto it = std::remove_if(
        this->LongTermMemory.begin(), this->LongTermMemory.end(),
        [&role, &content](const std::pair<std::string, std::string> &p) {
          return p.first == role && p.second == content;
        });
    this->LongTermMemory.erase(it, this->LongTermMemory.end());
  }

  MiniGemini &getApi() { return m_api; } // Returns the API client reference

  fileList getFiles() { return _files; } // Returns the list of files
  void setName(const std::string &name) { _name = name; }

  void addPrompt(const std::string &prompt) { extraPrompts.push_back(prompt); }
  void addEnvVar(const std::string &key, const std::string &value) {
    this->_env.push_back(std::make_pair(key, value));
  } // Adds to the environment variables

  void importEnvFile(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
      throw std::runtime_error("Could not open file: " + filePath);
    }
    std::string line;
    while (std::getline(file, line)) {
      size_t pos = line.find('=');
      if (pos != std::string::npos) {
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        addEnvVar(key, value);
      }
    }
    file.close();
  } // Imports environment variables from a file

  void setDescription(const std::string &description) {
    _description = description;
  } // Sets the description
  const std::string getDescription() const {
    return _description;
  } // Returns the description
  void loadFromYaml(const std::string &filePath);
  void saveToYaml(const std::string &filePath);

  std::string generateStamp(void) {
    std::time_t now = std::time(nullptr);
    std::tm *tm = std::localtime(&now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);

    std::string result = "[" + std::string(buffer) + "]";

    return result;
  } // Generates a timestamp

  // std::string generateHistoryMetadata(voi
  void addToHistory(const std::string &role, const std::string &content) {
    std::string prefixStamps = generateStamp();

    std::string result = prefixStamps + " :\n" + content;

    m_history.push_back(std::make_pair(role, result));
  }

  std::string wrapText(const std::string &text) {
    std::string wrappedText = "```" + text + "```";
    return wrappedText;
  } // Wraps text in code block
  std::string wrapXmlLine(const std::string &content, const std::string &tag) {
    std::string wrappedText = "<" + tag + ">" + content + "</" + tag + ">\n";
    return wrappedText;
  } // Wraps text in XML tags
    //
  std::string wrapXmlBlock(const std::string &content, const std::string &tag) {
    std::string wrappedText =
        "<" + tag + ">\n\t" + content + "\n</" + tag + ">\n";
    return wrappedText;
  } // Wraps text in XML tags

  void removeTool(const std::string &toolName);
  Tool *getTool(const std::string &toolName) const;
  // --- Built-in Tools ---
  void addBuiltInTool(Tool *tool);
  Tool *getBuiltInTool(const std::string &toolName) const;

  void addBlockToSystemPrompt(const std::string &content,
                              const std::string &tag) {
    std::string wrappedText = wrapXmlBlock(content, tag);
    m_systemPrompt += wrappedText;
  } // Adds a block to the system prompt

private:
  // --- Core Components ---
  MiniGemini &m_api;                     // Reference to the API client
  std::map<std::string, Tool *> m_tools; // Map of available external tools
  std::map<std::string, std::string>
      m_internalToolDescriptions; // Descriptions for internal tools like
                                  // 'help'

  // --- State ---
  std::string m_systemPrompt;
  std::vector<std::pair<std::string, std::string>>
      m_history; // Conversation history (role, content)
  std::vector<std::pair<std::string, std::string>>
      conversationHistory; // Persistent conversation history
  int iteration;
  int iterationCap;
  bool skipFlowIteration; // Flag to skip the final LLM call after tool
                          // execution
  std::vector<std::pair<std::string, std::string>>
      _env;          // Store results of tool calls for later use
  fileList _files;   // Store files for later use
  std::string _name; // Store name for later use
  // std::string _job;
  std::vector<std::string> extraPrompts;

  std::vector<std::pair<std::string, Agent *>>
      agents; // Orchestrator mode/cooperation mode
  // behavior later
  StrkeyValuePair Scratchpad;
  StrkeyValuePair ShortTermMemory;
  StrkeyValuePair LongTermMemory;

  std::map<std::string, Tool *> m_builtInTools; // Map of built-in tools
  // Add to your Agent class header
  std::map<std::string, Tool *> m_textTools;

  std::vector<std::string> tasks;
  void addTask(const std::string &task) { tasks.push_back(task); }

  std::vector<std::string> initialCommands;
  void addInitialCommand(const std::string &command) {
    initialCommands.push_back(command);
  } // Adds to the initial commands

  struct DIRECTIVE { // Referring to the conversation flow or the underlying
                     // intonation/stance that the agent is in/takes.
    enum Type { BRAINSTORMING, AUTONOMOUS, NORMAL, EXECUTE, REPORT };
    Type type;
    std::string description;
    std::string format;
  };

  DIRECTIVE directive;
  DIRECTIVE &getDirective() { return directive; } // Returns the directive
  void setDirective(const DIRECTIVE &dir) {
    directive = dir;
  } // Sets the directive
  std::string _description;

  // helpers

  // better to have xml helpers for building anyprompt
  std::string executeTask(const std::string &task, const Json::Value &format) {

    std::string prompt = "SYSTEM:\nYou must reply in valid json only. Strictly "
                         "following the below FORMAT.\n";
    prompt += "TASK:\n" + task + "\n";
    prompt += "FORMAT: " + format.toStyledString() + "\n";

    std::string res = m_api.generate(prompt);

    // checker/auditor for the response
    return res;
  }

  std::string executeTask(const std::string &task) {
    std::string prompt = "SYSTEM:\nYou must reply in valid json only. Strictly "
                         "following the below FORMAT.\n";
    prompt += "TASK:\n" + task + "\n";
    prompt += "FORMAT: {\"response\": \"string\"}\n";
    std::string res = m_api.generate(prompt);
    // checker/auditor for the response
    return res;
  }

  std::string executeTask(const std::string &task, const std::string &format) {
    std::string prompt = "SYSTEM:\nYou must reply strictly "
                         "following the provided FORMAT below.\n";
    prompt += "TASK:\n" + task + "\n";
    prompt += "FORMAT: " + format + "\n";
    std::string res = m_api.generate(prompt);
    // checker/auditor for the response
    return res;
  }

  std::string generateCode(const Json::Value &params) {
    if (params.isMember("prompt") && !params.isMember("content"))
      return executeTask(
          params["prompt"].asString(),
          std::string("multi-files are allowed. Format: "
                      "```file_name\ngenerated code content\n```"));
    if (params.isMember("content") && params["content"].isString()) {
      std::string content = params["content"].asString();
      executeTask("Generate code from the content:" + content,
                  std::string("multi-files are allowed. Format: "
                              "```file_name\ngenerated code content\n```"));
      return "Generated code from content: " + content;
    }
    return "Invalid parameters for generateCode. Expected 'prompt' (string) "
           "or 'content' (string).";
  }

  std::string getWeather(const Json::Value &params) {
    Json::Value bashParams;

    bashParams["command"] = "curl -L https://www.wttr.in/khouribga\?format=3";

    return this->manualToolCall("bash", params);
  }

  std::string summerizeTool(const Json::Value &params) {
    // Implement your summarization logic here
    // For example, summarize the content of a file
    if (params.isMember("content") && params["content"].isString()) {
      std::string content = params["content"].asString();
      executeTask(
          "Summarize the content:" + content,
          std::string("markdown. You must have at least 3 sections of "));
      return "Summary of content: " + content;
    }
    return "Invalid parameters for summerizeTool. Expected 'content' "
           "(string).";
  }

  std::string summarizeHistory(const Json::Value &params) {
    (void)params; // Unused parameter

    std::string summary =
        "You Must Summarize the following Conversation History:\n";
    for (const auto &entry : m_history) {
      summary += entry.first + ": " + entry.second + "\n";
    }

    std::string result = executeTask(summary);
    return result;
  }

  std::string reportTool(const Json::Value &params) {
    if (params.isMember("content") && params["content"].isString()) {
      std::string content = params["content"].asString();
      executeTask(
          "Report the content:" + content,
          std::string("markdown. You must have at least 3 sections of "));
      return "Report of content: " + content;
    }
    return "Invalid parameters for reportTool. Expected 'content' (string).";
  }

  std::string auditResponse(const std::string &response) {
    // Implement your auditing logic here
    // For example, check if the response is valid JSON
    Json::CharReaderBuilder reader;
    Json::Value jsonResponse;
    std::istringstream iss(response);
    std::string errs;

    if (!Json::parseFromStream(reader, iss, &jsonResponse, &errs)) {
      return "Invalid JSON response: " + errs;
    }
    return "Valid JSON response.";
  }

  // --- Helper Methods ---
  std::string buildFullPrompt() const;
  std::vector<ToolCallInfo> extractToolCalls(const std::string &response);
  std::string processToolCalls(const std::vector<ToolCallInfo> &toolCalls);
  std::string handleToolExecution(const ToolCallInfo &call);
  std::string executeApiCall(const std::string &fullPrompt);
  std::string
  getHelp(const Json::Value &params); // Internal help tool implementation
                                      //
  void setSkipFlowIteration(bool skip) {
    this->skipFlowIteration = skip;
  } // Set the skip flag

  // built-in tools
  std::string getToolDescription(const std::string &toolName) const;
  std::string skip(const Json::Value &params) {
    if (params.isMember("skip") and params["skip"].isBool()) {
      this->setSkipFlowIteration(params["skip"].asBool());
      return "Skipping the next iteration.";
    }
    return "Invalid parameters for skip tool. Expected a boolean.";
  }

  std::string promptAgent(Agent &agentToCall, const std::string &userInput) {
    // Call the agent's prompt method and return the response
    return agentToCall.prompt(userInput);
  }

  std::string promptAgent(const std::string &name,
                          const std::string &userInput) {
    // Find the agent by name and call its prompt method
    auto it = std::find_if(agents.begin(), agents.end(),
                           [&name](const std::pair<std::string, Agent *> &p) {
                             return p.first == name;
                           });
    if (it != agents.end()) {
      return it->second->prompt(userInput);
    }
    return "Agent name not found.";
  }

  std::string promptAgentTool(const Json::Value &params) {
    // Call the agent's prompt method and return the response
    if (params.isMember("name") && params["name"].isString()) {
      std::string agentName = params["name"].asString();
      std::string userInput = params["prompt"].asString();
      return promptAgent(agentName, userInput);
    }
    return "Invalid parameters for promptAgent tool. Expected agent name "
           "'name' (string) and 'prompt'.";
  }

  std::string generateTool(const Json::Value &params) {
    // Call the agent's prompt method and return the response
    // could get an optional 'path' (string) should output the result to the
    // file, an extra 'mode' (string) either 'in' or 'out' to specify whether
    // we will dump the generated content into a file or use a files content
    // as context to fuel the prompt the ai

    if (params.isMember("path") && params["path"].isString()) {
      std::string path = params["path"].asString();
      std::string mode = params["mode"].asString();
      if (mode == "in") {
        // Load the file content and use it as context
        std::ifstream file(path);
        if (!file.is_open()) {
          return "Error: Could not open file for reading: " + path;
        }
        std::string fileContent((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
        file.close();
        return prompt(fileContent);
      } else if (mode == "out") {
        // Generate content and save it to the specified file
        std::string generatedContent = prompt(params["prompt"].asString());
        std::ofstream outFile(path);
        if (!outFile.is_open()) {
          return "Error: Could not open file for writing: " + path;
        }
        outFile << generatedContent;
        outFile.close();
        return "Generated content saved to: " + path;
      } else {
        return "Error: Invalid mode. Expected 'in' or 'out'.";
      }
    }
    return "Invalid parameters for generateTool. Expected 'path' (string) "
           "and "
           "'mode' (string).";
  }
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
#include <stdexcept>   // For std::runtime_error
#include <string>

#include "Agent.hpp"
// Forward declaration (optional, good practice)
class Agent;
namespace Json {
class Value;
}

// Callback function signature: Takes JSON parameters, returns string result
// This is: a typedef for a function pointer that takes a Json::Value and
// returns a string
typedef std::string (*ToolCallback)(const Json::Value &params);
typedef std::string (*ToolCallbackWithAgent)(const Json::Value &params,
                                             Agent &agent);
typedef std::string (*PureTextToolCallback)(const std::string &params);

class Tool {
public:
  Tool(const std::string &name, const std::string &description,
       ToolCallback callback, Agent &agent)
      : m_name(name), m_description(description), m_callback(callback),
        m_agent(&agent) {}
  Tool(const std::string &name, const std::string &description,
       ToolCallbackWithAgent callback, Agent &agent)
      : m_name(name), m_description(description), m_builtin_callback(callback),
        m_agent(&agent) {}
  Tool(const std::string &name, const std::string &description,
       ToolCallback callback)
      : m_name(name), m_description(description), m_callback(callback),
        m_agent(nullptr) {}
  Tool(const std::string &name, const std::string &description,
       PureTextToolCallback callback)
      : m_name(name), m_description(description), m_agent(nullptr),
        m_text_callback(callback) {}
  Tool()
      : m_name(""), m_description(""), m_callback(nullptr), m_agent(nullptr) {}
  Tool(ToolCallback callback)
      : m_name(""), m_description(""), m_callback(callback), m_agent(nullptr) {}
  // Constructor: Takes name, description, and the callback function

  // Get the tool's name
  std::string getName() const { return m_name; }
  void setName(const std::string &name) { m_name = name; }

  // Get the tool's description (used in prompts)
  std::string getDescription() const { return m_description; }
  void setDescription(const std::string &description) {
    m_description = description;
  }

  // Execute the tool's function with given parameters
  std::string execute(const Json::Value &params) {
    if (m_agent)
      return m_builtin_callback(params, *m_agent);
    else if (m_callback) {
      return m_callback(params);
    } else {
      throw std::runtime_error("No callback function set for this tool");
    }
  }
  std::string execute(const std::string &params) {
    if (m_text_callback) {
      return m_text_callback(params);
    } else {
      throw std::runtime_error("No callback function set for this tool");
    }
  }

  void setCallback(ToolCallback callback) {
    if (!callback) {
      throw std::invalid_argument("Tool callback cannot be NULL");
    }
    m_callback = callback;
  }

  void setBuiltin(ToolCallbackWithAgent callback) {
    m_builtin_callback = callback;
  }

  void addUseCase(const std::string &use_case, const std::string &description) {
    m_use_cases[use_case] = description;
  }
  std::string getUseCase(const std::string &use_case) const {
    auto it = m_use_cases.find(use_case);
    if (it != m_use_cases.end()) {
      return it->second;
    } else {
      throw std::runtime_error("Use case not found");
    }
  }
  std::string getAllUseCases() const {
    std::string all_use_cases;

    all_use_cases += "All Use Cases for tool: " + m_name + "\n";
    for (const auto &pair : m_use_cases) {
      all_use_cases +=
          "Purpose: " + pair.first + "| example case: " + pair.second + "\n";
    }
    return all_use_cases;
  }
  std::string getAllUseCaseCap(size_t cap) const {
    std::string all_use_cases;
    all_use_cases += "All Use Cases for tool: " + m_name + "\n";
    size_t count = 0;
    for (const auto &pair : m_use_cases) {
      if (count >= cap) {
        break;
      }
      all_use_cases +=
          "Purpose: " + pair.first + "| example case: " + pair.second + "\n";
      ++count;
    }
    return all_use_cases;
  }
  // Memory and Storage Management
  void addMemory(const std::string &key, const std::string &value) {
    m_memory_stack[key] = value;
  }
  std::string getMemory(const std::string &key) const {
    auto it = m_memory_stack.find(key);
    if (it != m_memory_stack.end()) {
      return it->second;
    } else {
      throw std::runtime_error("Memory key not found");
    }
  }

private:
  std::string m_name;
  std::string m_description;

  ToolCallback m_callback;
  ToolCallbackWithAgent m_builtin_callback;

  std::map<std::string, std::string> m_use_cases; // for use cases
  // Memory and Storage Management:
  std::map<std::string, std::string> m_memory_stack; // for general use
  // Memory for storing tool state
  Agent *m_agent; // Pointer to the agent using this tool
  PureTextToolCallback m_text_callback;
};
```
# AI Project Analysis - src
- Generated on: Mon Apr 14 05:06:36 PM +01 2025
- System: Linux 6.12.21-1-lts x86_64
- Arch Linux: 1639 packages installed
- Directory: /home/mlamkadm/ai-repos/agents/cpp-agent-mk1

## Directory Structure
```
../cpp-agent-mk1/src
├── agent
├── agent.cpp
├── behavior.cpp
├── groqClient.cpp
├── logging
├── memory
│   ├── local
│   └── note
├── MiniGemini.cpp
├── tools
├── tools.cpp
├── utils
└── utils.cpp
```

## Project Statistics
- Total Files: 59
- Total Lines of Code: 7961
- Languages: .cpp(6)

## Project Files

### File: src/agent.cpp
```cpp
#include "../inc/Agent.hpp"
#include "../inc/MiniGemini.hpp"
#include "../inc/Tool.hpp" // Header-only Tool #include <algorithm>       // For std::transform
#include <cctype>  // For std::toupper
#include <ctime>   // For time functions
#include <iomanip> // For formatting time
#include <iostream>
#include <json/json.h>
#include <sstream>
#include <stdexcept>

// Simple colored logging function
void logMessage(LogLevel level, const std::string &message,
                const std::string &details = "") {
  std::time_t now = std::time(nullptr);
  std::tm *tm = std::localtime(&now);
  char time_buffer[20]; // HH:MM:SS
  std::strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", tm);

  std::string prefix;
  std::string color_start = "";
  std::string color_end = "\033[0m"; // Reset color

  // if (level == LogLevel::DEBUG)
  //   return;
  if (level == LogLevel::PROMPT)
    return;
  // level = LogLevel::INFO; // Default to INFO for now
  switch (level) {
  case LogLevel::DEBUG:
    prefix = "[DEBUG] ";
    color_start = "\033[36m";
    break; // Cyan
  case LogLevel::INFO:
    prefix = "[INFO]  ";
    color_start = "\033[32m";
    break; // Green
  case LogLevel::WARN:
    prefix = "[WARN]  ";
    color_start = "\033[33m";
    break; // Yellow
  case LogLevel::ERROR:
    prefix = "[ERROR] ";
    color_start = "\033[1;31m";
    break; // Bold Red
  case LogLevel::TOOL_CALL:
    prefix = "[TOOL CALL] ";
    color_start = "\033[1;35m";
    break; // Bold Magenta
  case LogLevel::TOOL_RESULT:
    prefix = "[TOOL RESULT] ";
    color_start = "\033[35m";
    break; // Magenta
  case LogLevel::PROMPT:
    prefix = "[PROMPT] ";
    color_start = "\033[34m";
    break; // Blue
  }

  // Use cerr for errors and warnings, cout for others
  std::ostream &out = (level == LogLevel::ERROR || level == LogLevel::WARN)
                          ? std::cerr
                          : std::cout;

  out << color_start << std::string(time_buffer) << " " << prefix << message
      << color_end << std::endl;
  if (!details.empty()) {
    // Indent details slightly
    out << "  " << details << std::endl;
  }
}

// --- Agent Implementation ---

Agent::Agent(MiniGemini &api)
    : m_api(api), m_systemPrompt("You are a helpful assistant."), iteration(0),
      iterationCap(120), skipFlowIteration(false), _name("orchestrator") {
  // Initialize random seed (though std::rand is generally discouraged for
  // serious use)
  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  // Add the internal 'help' tool description automatically
  m_internalToolDescriptions["help"] =
      "Provides descriptions of available tools. Takes an optional 'tool_name' "
      "parameter to get help for a specific tool.";
  m_internalToolDescriptions["skip"] =
      "Skips the current workflow iteration. Almost always used when "
      "multi-steps are needed to achieve said task or when toolcall results "
      "need to be evaluated beforehand. Or if there is an issue of course. "
      "Intakes a skip (bool) and is almost always used last (after "
      "tools/workflow is done.)";
  m_internalToolDescriptions["promptAgent"] =
      "Prompts another agent with a given input. Takes 'name' (string) and "
      "'prompt' (string) parameters. It is common courtesy to tell the agent "
      "your name, and such.\n";
  m_internalToolDescriptions["summerize"] =
      "Summarizes the conversation history. Takes 'name' (string) and "
      "'prompt' (string) parameters. It is common courtesy to tell the agent "
      "your name, and such.\n";
  // m_internalToolDescriptions["generate"] =
  //     "Generates content based on the provided parameters. Takes 'path' "
  //     "(string) and 'mode' (string) parameters. It is common courtesy to tell
  //     " "the agent your name, and such.\n";
  m_internalToolDescriptions["summarizeHistory"] =
      "Summarizes the conversation history. Takes 'name' (string) and "
      "'prompt' (string) parameters. It is common courtesy to tell the agent "
      "your name, and such.\n";
  m_internalToolDescriptions["getWeather"] =
      "Fetches the current weather for a given location. Takes 'location' "
      "(string) parameter. It is common courtesy to tell the agent your name, "
      "and such.\n";

  // Add other internal tools as needed
}

void Agent::setSystemPrompt(const std::string &prompt) {
  m_systemPrompt = prompt;
}

void Agent::addTool(Tool *tool) {
  if (tool) {
    // Check for name collisions, including with internal tools
    if (m_tools.count(tool->getName()) ||
        m_internalToolDescriptions.count(tool->getName())) {
      logMessage(LogLevel::WARN,
                 "Attempted to add tool with duplicate name: '" +
                     tool->getName() + "'. Ignoring.");
    } else {
      m_tools.insert(std::make_pair(tool->getName(), tool));
      logMessage(LogLevel::INFO, "Added tool: '" + tool->getName() + "'");
    }
  } else {
    logMessage(LogLevel::WARN, "Attempted to add a null tool.");
  }
}

void Agent::addTextTool(Tool *tool) {
  if (tool) {
    // Check for name collisions, including with internal tools
    if (m_textTools.count(tool->getName()) ||
        m_internalToolDescriptions.count(tool->getName())) {
      logMessage(LogLevel::WARN,
                 "Attempted to add text tool with duplicate name: '" +
                     tool->getName() + "'. Ignoring.");
    } else {
      m_textTools.insert(std::make_pair(tool->getName(), tool));
      logMessage(LogLevel::INFO, "Added text tool: '" + tool->getName() + "'");
    }
  } else {
    logMessage(LogLevel::WARN, "Attempted to add a null text tool.");
  }
}

void Agent::reset() {
  m_history.clear();
  iteration = 0;
  skipFlowIteration = false;
  logMessage(LogLevel::INFO, "Agent reset.");
}

int executeCommand(const std::string &command, std::string &result) {
  FILE *pipe = popen(command.c_str(), "r");
  if (!pipe) {
    return -1; // Error opening pipe
  }
  char buffer[128];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    result += buffer;
  }
  return pclose(pipe);
}

std::string Agent::buildFullPrompt() const {
  std::ostringstream oss;
  oss << "System Instructions: " << m_systemPrompt << "\n";

  if (this->extraPrompts.size() > 0) {
    oss << "Extra Instructions:\n";
    for (const auto &extraPrompt : this->extraPrompts) {
      oss << extraPrompt << "\n";
    }
    oss << "\n";
  }
  // System information
  oss << "System Information:\n";
  oss << "- Agent Name: " << _name << "\n";
  // execute tree cmd
  // print available agent tools
  oss << "- Available  Agents (agents you can use; can be called using the promptAgent built-in tool): " << std::endl;
  for (const auto &pair : agents) {
    oss << "-Name: " << pair.first << " -Description: "<< pair.second->getDescription() << "\n";
  }

  std::string command = "tree";
  std::string result;

  // executing all startup scripts and automated bash commands
  if (initialCommands.size() > 0) {
    oss << "Executing Startup Commands:\n";
  }
  for (auto &command : initialCommands) {
    std::string result;
    int status = executeCommand(command, result);
    oss << "- Executed: " << command << "\n";
    if (status == -1) {
      logMessage(LogLevel::ERROR, "Failed to execute command: " + command);
    } else {
      oss << "- Command Output: " << result << "\n";
    }
  }
  // executeCommand(command, result);
  // oss << "- System Tree: \n";
  // oss << result << "\n";

  // list env vars if available
  if (this->_env.size() > 0) {
    oss << "Environment Variables:\n";
    for (const auto &pair : this->_env) {
      oss << "- " << pair.first << ": " << pair.second << "\n";
    }
    oss << "\n";
  }

  // Available Tools Section
  bool hasTools = !m_tools.empty() || !m_internalToolDescriptions.empty();
  if (hasTools) {
    oss << "Available tools:\n";
    // Internal tools first
    for (const auto &pair : m_internalToolDescriptions) {
      oss << "- " << pair.first << ": " << pair.second << "\n";
    }
    // External tools
    for (const auto &pair : m_tools) {
      oss << "- " << pair.second->getName() << ": "
          << pair.second->getDescription() << "\n";
    }
    for (const auto &pair : m_textTools) {
      oss << "- " << pair.first << ": " << pair.second->getDescription()
          << "\n";
    }
    oss << "\n";
  }

  // Conversation History Section
  if (!m_history.empty()) {
    oss << "Conversation history:\n";
    for (const auto &entry : m_history) {
      std::string role = entry.first;
      if (!role.empty()) {
        // Capitalize first letter
        role[0] = std::toupper(static_cast<unsigned char>(role[0]));
      }
      oss << role << ": " << entry.second << "\n";
    }
    oss << "\n";
  }

  // Indicate where the assistant should respond
  oss << "Assistant:"; // Let the LLM continue from here
  logMessage(LogLevel::DEBUG, "Full prompt built:", oss.str());
  return oss.str();
}

std::vector<Agent::ToolCallInfo>
Agent::extractToolCalls(const std::string &response) {
  std::vector<ToolCallInfo> calls;
  const std::string startMarker = "```tool:";
  const std::string endMarker = "```";
  std::string::size_type currentPos = 0;

  while ((currentPos = response.find(startMarker, currentPos)) !=
         std::string::npos) {
    std::string::size_type toolNameStart = currentPos + startMarker.length();
    std::string::size_type toolNameEnd = response.find('\n', toolNameStart);
    if (toolNameEnd == std::string::npos) {
      logMessage(LogLevel::WARN,
                 "Malformed tool call: Could not find newline after tool name.",
                 response.substr(currentPos, 50) + "...");
      currentPos = toolNameStart; // Move past marker to avoid infinite loop
      continue;
    }

    std::string toolName =
        response.substr(toolNameStart, toolNameEnd - toolNameStart);

    // Basic trim
    toolName.erase(0, toolName.find_first_not_of(" \t\r\n"));
    toolName.erase(toolName.find_last_not_of(" \t\r\n") + 1);

    if (toolName.empty()) {
      logMessage(LogLevel::WARN,
                 "Malformed tool call: Empty tool name detected.");
      currentPos = toolNameEnd + 1; // Move past newline
      continue;
    }

    auto textBasedTools = m_textTools;

    if (textBasedTools.find(toolName) != textBasedTools.end()) {
      // Handle text-based tool
      std::string::size_type contentStart = toolNameEnd + 1;
      std::string::size_type contentEnd =
          response.find(endMarker, contentStart);

      if (contentEnd == std::string::npos) {
        logMessage(LogLevel::WARN,
                   "Malformed text-based tool call: Could not find end marker "
                   "'```' for tool '" +
                       toolName + "'.",
                   response.substr(contentStart, 50) + "...");
        currentPos = contentStart;
        continue;
      }

      std::string rawContent =
          response.substr(contentStart, contentEnd - contentStart);

      // Trim the raw content
      rawContent.erase(0, rawContent.find_first_not_of(" \t\r\n"));
      rawContent.erase(rawContent.find_last_not_of(" \t\r\n") + 1);

      logMessage(LogLevel::DEBUG,
                 "Detected text-based tool call '" + toolName +
                     "' with content: " + rawContent.substr(0, 50) +
                     (rawContent.length() > 50 ? "..." : ""));

      ToolCallInfo info;
      info.toolName = toolName;
      info.optionalDataStr = rawContent;
      info.isTextBasedTool = true; // Mark as text-based tool
      calls.push_back(info);

      currentPos = contentEnd + endMarker.length();
      continue;
    }

    // Handle JSON-based tool (existing code)
    std::string::size_type paramsStart = toolNameEnd + 1;
    std::string::size_type paramsEnd = response.find(endMarker, paramsStart);
    if (paramsEnd == std::string::npos) {
      logMessage(
          LogLevel::WARN,
          "Malformed tool call: Could not find end marker '```' for tool '" +
              toolName + "'.",
          response.substr(paramsStart, 50) + "...");
      currentPos = paramsStart; // Move past where params should have started
      continue;
    }

    std::string paramsJsonStr =
        response.substr(paramsStart, paramsEnd - paramsStart);

    Json::Value paramsJson;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(paramsJsonStr, paramsJson);

    if (!parsingSuccessful) {
      logMessage(LogLevel::WARN,
                 "Failed to parse JSON params for tool '" + toolName + "'.",
                 "JSON String: " + paramsJsonStr +
                     "\nError: " + reader.getFormattedErrorMessages());
      // Decide how to handle: skip this call, or add with empty params? Let's
      // skip.
      currentPos = paramsEnd + endMarker.length();
      continue;
    }

    ToolCallInfo info;
    info.toolName = toolName;
    info.params = paramsJson;
    info.isTextBasedTool = false; // Mark as JSON-based tool
    calls.push_back(info);
    logMessage(LogLevel::DEBUG, "Extracted tool call",
               "Tool: " + toolName +
                   ", Params: " + paramsJson.toStyledString());

    currentPos =
        paramsEnd + endMarker.length(); // Continue search after this block
  }
  return calls;
}

// Executes a single tool call, handling both JSON-based and text-based tools
std::string Agent::handleToolExecution(const ToolCallInfo &call) {
  if (call.isTextBasedTool) {
    logMessage(LogLevel::TOOL_CALL,
               "Executing text-based tool '" + call.toolName + "'",
               "Content: " + call.optionalDataStr.substr(0, 100) +
                   (call.optionalDataStr.length() > 100 ? "..." : ""));
  } else {
    logMessage(LogLevel::TOOL_CALL,
               "Executing JSON-based tool '" + call.toolName + "'",
               "Params: " + call.params.toStyledString());
  }

  std::string result;
  try {
    if (call.toolName == "help") {
      result = this->getHelp(call.params);
    } else if (call.toolName == "skip") {
      result = skip(call.params);
    } else if (call.toolName == "promptAgent") {
      result = promptAgentTool(call.params);
    } else if (call.toolName == "summerize") {
      result = summerizeTool(call.params);
    } else if (call.toolName == "summarizeHistory") {
      result = summarizeHistory(call.params);
    } else if (call.toolName == "weather") {
      result = getWeather(call.params);
    } else if (call.toolName == "write") {
      // Handle text-based tool for writing
      auto toolIt = m_textTools.find(call.toolName);
      if (toolIt != m_textTools.end()) {
        // Call the text-based tool handler
        result = toolIt->second->execute(call.optionalDataStr);
      } else {
        logMessage(LogLevel::WARN,
                   "Attempted to execute unknown text-based tool: '" +
                       call.toolName + "'");
        result = "Error: Unknown text-based tool '" + call.toolName + "'";
      }

    } else if (call.isTextBasedTool) {
      // Handle text-based tools
      auto toolIt = m_textTools.find(call.toolName);
      if (toolIt != m_textTools.end()) {
        // Call the text-based tool handler
        result = toolIt->second->execute(call.optionalDataStr);
      } else {
        // Fallback for built-in text tools like "talk"
        if (call.toolName == "talk" || call.toolName == "message") {
          // For "talk" tool, the result could just be an acknowledgment
          // or you could implement specific behavior
          result = "[INFO] Tool '" + call.toolName + "' executed successfully.";
          logMessage(LogLevel::INFO, "Message from agent " + this->_name +
                                         ": " + call.optionalDataStr);

          // You might want to implement special handling here
          // such as sending the message somewhere, storing it, etc.
        } else {
          logMessage(LogLevel::WARN,
                     "Attempted to execute unknown text-based tool: '" +
                         call.toolName + "'");
          result = "Error: Unknown text-based tool '" + call.toolName + "'";
        }
      }
    } else {
      // Handle JSON-based tools (existing code)
      auto toolIt = m_tools.find(call.toolName);
      if (toolIt != m_tools.end()) {
        result = toolIt->second->execute(call.params);
      } else {
        logMessage(LogLevel::WARN, "Attempted to execute unknown tool: '" +
                                       call.toolName + "'");
        result = "Error: Unknown tool '" + call.toolName + "'";
      }
    }
    logMessage(LogLevel::TOOL_RESULT,
               "Tool '" + call.toolName + "' executed successfully.",
               "Result: " + result);
  } catch (const std::exception &e) {
    result = "Error executing tool '" + call.toolName + "': " + e.what();
    logMessage(LogLevel::ERROR,
               "Exception during tool execution for '" + call.toolName + "'.",
               "Error: " + std::string(e.what()));
  } catch (...) {
    result = "Error: Unknown exception occurred while executing tool '" +
             call.toolName + "'";
    logMessage(LogLevel::ERROR,
               "Unknown exception during tool execution for '" + call.toolName +
                   "'.");
  }
  return result;
}

// Processes all detected tool calls and aggregates their results.
std::string
Agent::processToolCalls(const std::vector<ToolCallInfo> &toolCalls) {
  if (toolCalls.empty()) {
    return ""; // No tools to process
  }

  std::ostringstream toolResultsOss;
  toolResultsOss
      << "Tool Results:\n"; // Start the tool results section for the LLM

  for (const auto &call : toolCalls) {
    std::string result = handleToolExecution(call);
    // Format for the LLM: Tool name and its result/error
    toolResultsOss << "Tool: " << call.toolName << "\nResult: " << result
                   << "\n";
  }
  return toolResultsOss.str();
}

// Makes a call to the underlying LLM API.
std::string Agent::executeApiCall(const std::string &fullPrompt) {
  logMessage(LogLevel::PROMPT, "Sending prompt to API:", fullPrompt);
  try {
    std::string response = m_api.generate(fullPrompt);
    logMessage(LogLevel::DEBUG, "Received API response:", response);
    return response;
  } catch (const ApiError &e) {
    logMessage(LogLevel::ERROR, "API Error occurred:", e.what());
    // Re-throw as a runtime_error or handle differently if needed
    throw std::runtime_error("API Error: " + std::string(e.what()));
  } catch (const std::exception &e) {
    logMessage(LogLevel::ERROR,
               "Standard exception during API call:", e.what());
    throw std::runtime_error("Error during API call: " + std::string(e.what()));
  } catch (...) {
    logMessage(LogLevel::ERROR, "Unknown exception during API call.");
    throw std::runtime_error("Unknown error during API call");
  }
}

// Main interaction logic for a single user turn.
std::string Agent::prompt(const std::string &userInput) {
  // 1. Add user input to history
  m_history.push_back({"Master/User", userInput});
  logMessage(LogLevel::DEBUG, "User Input:", userInput);

  // --- Iteration 1: Initial API Call ---

  // 2. Build prompt based on current history and system instructions
  std::string currentPrompt = buildFullPrompt();

  // 3. Get initial response from API
  std::string assistantResponse1 = executeApiCall(currentPrompt);

  // 4. Add the raw assistant response (which might contain tool calls) to
  // history
  // m_history.push_back({this->_name, assistantResponse1});

  // 5. Check for tool calls in the response
  std::vector<ToolCallInfo> toolCalls = extractToolCalls(assistantResponse1);

  // --- Iteration 2: Tool Execution and Final Response (if needed) ---

  if (!toolCalls.empty()) {
    logMessage(LogLevel::INFO,
               "Tool calls detected in response. Executing tools...");

    // 6. Execute tools and gather results
    std::string toolResultsString = processToolCalls(toolCalls);

    // 7. Add tool results to history
    // Use a distinct role like "tool" or "system" for results
    m_history.push_back({"tool", toolResultsString});
    logMessage(LogLevel::DEBUG,
               "Added tool results to history:", toolResultsString);

    // 8. Build the prompt for the second API call (now includes tool results)
    std::string promptWithToolResults =
        buildFullPrompt(); // Rebuild with updated history

    // 8a. Handle skipFlowIteration (if applicable)
    // 8b. (Optional) Add guidance for the LLM after tool execution
    promptWithToolResults +=
        "\n[POST-TOOL-EXECUTION] Please synthesize the tool results.";

    // 9. Call API again with tool results included
    std::string finalResponse = executeApiCall(promptWithToolResults);

    std::vector<ToolCallInfo> postToolCalls = extractToolCalls(finalResponse);
    std::string toolResults = processToolCalls(postToolCalls);

    m_history.push_back({"POST-TOOL-EXECUTION", toolResults});

    if (skipFlowIteration) {
      std::string notice =
          "\n[SYSTEM] Workflow iteration skipped as per request.";
      notice += " Iteration: " + std::to_string(iteration) + "/" +
                std::to_string(iterationCap) + ".";
      notice += " Please review the tool results above and provide the final "
                "response to the user or call further tools if necessary.";
    notice +=
        "\n[IMPORTANT] If the task is not solved";
      promptWithToolResults += notice; // Append the notice for the LLM
      logMessage(LogLevel::INFO, "Skipping flow iteration as requested.");
      skipFlowIteration = false; // Reset the flag
    }
    // 10. Add final assistant response to history and return it
    m_history.push_back({this->_name, finalResponse});
    logMessage(LogLevel::DEBUG,
               "Final Assistant Response (after tools):", finalResponse);
    return finalResponse;

  } else {
    // No tool calls, the first response is the final one for this turn
    return assistantResponse1;
  }
}

// Provides help/descriptions for available tools.
std::string Agent::getHelp(const Json::Value &params) {
  std::ostringstream helpOss;
  std::string specificTool;

  if (params.isMember("tool_name") && params["tool_name"].isString()) {
    specificTool = params["tool_name"].asString();
  }

  if (!specificTool.empty()) {
    // Help for a specific tool
    helpOss << "Help for tool '" << specificTool << "':\n";
    bool found = false;
    // Check internal tools
    auto internalIt = m_internalToolDescriptions.find(specificTool);
    if (internalIt != m_internalToolDescriptions.end()) {
      helpOss << "- Description: " << internalIt->second;
      // Potentially add parameter schema info here in the future
      found = true;
    }
    // Check external tools
    auto externalIt = m_tools.find(specificTool);
    if (externalIt != m_tools.end()) {
      helpOss << "- Description: " << externalIt->second->getDescription();
      // Potentially add externalIt->second->getParameterSchema() or similar
      found = true;
    }
    auto textIt = m_textTools.find(specificTool);
    if (textIt != m_textTools.end()) {
      helpOss << "- Description: " << textIt->second->getDescription();
      // Potentially add textIt->second->getParameterSchema() or similar
      found = true;
    }

    if (!found) {
      helpOss << "Tool '" << specificTool << "' not found.";
    }
  } else {
    // General help - list all tools
    helpOss << "Available Tools:\n";
    // Internal tools first
    for (const auto &pair : m_internalToolDescriptions) {
      helpOss << "- " << pair.first << ": " << pair.second << "\n";
    }
    // External tools
    for (const auto &pair : m_tools) {
      helpOss << "- " << pair.second->getName() << ": "
              << pair.second->getDescription() << "\n";
    }
    helpOss << "\nUse help with 'tool_name' parameter for details on a "
               "specific tool (e.g., {\"tool_name\": \"calculator\"}).";
  }
  return helpOss.str();
}

// Interactive loop
void Agent::run() {
  logMessage(
      LogLevel::INFO,
      "Agent ready. Type 'exit' or 'quit' to quit, 'reset' to clear history.");
  std::string userInput;

  // temp agent arr

  while (true) {
    std::cout << "\n> ";
    if (!std::getline(std::cin, userInput)) {
      logMessage(LogLevel::INFO, "Input stream closed (EOF). Exiting.");
      break; // Handle EOF
    }
    if (userInput == "exit" || userInput == "quit") {
      logMessage(LogLevel::INFO, "Exit command received. Goodbye!");
      break;
    } else if (userInput == "reset") {
      reset(); // Already logs the reset
      continue;
    } else if (userInput.empty()) {
      continue; // Ignore empty input
    }

    try {
      std::string response = prompt(userInput);
      // The final response is already logged within prompt()
      // Optional: Print a separator or the final response here if desired
      std::cout << "\n-----------------------------------------"
                << std::endl; // Separator
    } catch (const std::exception &e) {
      // Errors from API calls or tool execution are caught here
      logMessage(LogLevel::ERROR,
                 "An error occurred during processing:", e.what());
      // Inform the user
      std::cout << "\n[Agent Error]: " << e.what() << std::endl;
      std::cout << "-----------------------------------------" << std::endl;
    }
    // Note: iteration count logic wasn't fully used in the original,
    // but you could add checks against iterationCap here or within prompt()
    // if needed for flow control.
    iteration++; // Increment iteration count per user turn
    if (iteration >= iterationCap) {
      logMessage(LogLevel::WARN, "Iteration cap reached.",
                 "Limit: " + std::to_string(iterationCap));
      // Optionally reset or take other action
    }
  }
}
```

### File: src/behavior.cpp
```cpp
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
    m_model("gemini-1.5-flash-latest"), // Use a common, stable model
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

### File: src/tools.cpp
```cpp
```

### File: src/utils.cpp
```cpp
```
