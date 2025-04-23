
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
